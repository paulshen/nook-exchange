module Styles = {
  open Css;
  let root =
    style([
      width(px(368)),
      margin3(~top=px(32), ~bottom=zero, ~h=auto),
      media("(min-width: 600px)", [width(px(560))]),
      media("(min-width: 940px)", [width(px(752))]),
      media("(min-width: 1200px)", [width(px(944))]),
      media("(min-width: 1460px)", [width(px(1136))]),
      padding(px(32)),
      backgroundColor(Colors.green),
      borderRadius(px(16)),
      backgroundColor(hex("ffbfb9a0")),
    ]);
  let rootAvailable = style([backgroundColor(hex("8FCDE0a0"))]);
  let sectionTitle = style([fontSize(px(24)), marginBottom(px(16))]);
  let cards = style([paddingTop(px(16)), marginRight(px(-16))]);
  let card =
    style([
      backgroundColor(hex("fffffff0")),
      borderRadius(px(8)),
      display(flexBox),
      flexDirection(column),
      alignItems(center),
      marginRight(px(16)),
      marginBottom(px(16)),
      padding3(~top=px(24), ~bottom=px(8), ~h=px(8)),
      position(relative),
      width(px(160)),
      transition(~duration=200, "all"),
    ]);
  let name = style([fontSize(px(16)), marginBottom(px(4))]);
  let userNote =
    style([
      borderTop(px(1), solid, hex("f0f0f0")),
      unsafe("alignSelf", "stretch"),
      marginTop(px(-8)),
      padding3(~top=px(8), ~bottom=zero, ~h=px(4)),
    ]);
  let removeButton = style([top(px(9)), bottom(initial)]);
};

open Belt;

module UserItemCard = {
  [@react.component]
  let make = (~itemId, ~variation, ~userItem: User.item, ~editable) => {
    let item = Item.getItem(~itemId);
    <div className={Cn.make([Styles.card])}>
      <div className=ItemCard.Styles.body>
        <img
          src={
            "/images/"
            ++ item.image
            ++ "__"
            ++ string_of_int(variation)
            ++ ".png"
          }
          className=ItemCard.Styles.mainImage
        />
        <div className=Styles.name> {React.string(item.name)} </div>
      </div>
      <div className=ItemCard.Styles.metaIcons>
        {switch (item.recipe) {
         | Some(recipe) => <ItemCard.RecipeIcon recipe />
         | None => React.null
         }}
      </div>
      {editable
         ? <>
             <UserItemNote itemId={item.id} variation userItem />
             <button
               className={Cn.make([
                 ItemCard.Styles.removeButton,
                 Styles.removeButton,
               ])}
               title="Remove"
               onClick={_ => {
                 UserStore.removeItem(~itemId=item.id, ~variation)
               }}>
               {React.string({j|❌|j})}
             </button>
           </>
         : (
           if (userItem.note->Js.String.length > 0) {
             <div className=Styles.userNote>
               {React.string(userItem.note)}
             </div>;
           } else {
             React.null;
           }
         )}
    </div>;
  };
};

module Section = {
  let numResultsPerPage = 12;

  [@react.component]
  let make =
      (
        ~status: User.itemStatus,
        ~userItems: array(((string, int), User.item)),
        ~editable,
      ) => {
    let (filters, setFilters) =
      React.useState(() =>
        (
          {text: "", orderable: None, hasRecipe: None, category: None}: ItemFilters.t
        )
      );
    let (pageOffset, setPageOffset) = React.useState(() => 0);
    let filteredItems =
      React.useMemo2(
        () =>
          userItems->Belt.Array.keep((((itemId, _), _)) =>
            ItemFilters.doesItemMatchFilters(
              ~item=Item.getItem(~itemId),
              ~filters,
            )
          ),
        (userItems, filters),
      );
    let numResults = filteredItems->Belt.Array.length;

    <div
      className={Cn.make([
        Styles.root,
        Cn.ifTrue(Styles.rootAvailable, status == WillTrade),
      ])}>
      <div className=Styles.sectionTitle>
        {React.string(
           switch (status) {
           | Want => {j|🙏 Wishlist|j}
           | WillTrade => {j|✅ Available|j}
           },
         )}
      </div>
      {userItems->Belt.Array.length > 8
         ? <div className=ItemBrowser.Styles.filterBar>
             <ItemFilters
               filters
               onChange={filters => {
                 setFilters(_ => filters);
                 setPageOffset(_ => 0);
               }}
             />
             <ItemFilters.Pager
               numResults
               pageOffset
               numResultsPerPage
               setPageOffset
             />
           </div>
         : React.null}
      <div className={Cn.make([ItemBrowser.Styles.cards, Styles.cards])}>
        {filteredItems
         ->Belt.Array.slice(
             ~offset=pageOffset * numResultsPerPage,
             ~len=numResultsPerPage,
           )
         ->Belt.Array.mapU((. ((itemId, variation), userItem)) => {
             <UserItemCard
               itemId
               variation
               userItem
               editable
               key={itemId ++ string_of_int(variation)}
             />
           })
         ->React.array}
      </div>
    </div>;
  };
};

[@react.component]
let make = (~userItems: array(((string, int), User.item)), ~editable) => {
  let (wants, willTrades) =
    userItems->Array.partitionU((. (_, item)) => item.status == Want);
  <div>
    {if (wants->Array.length > 0) {
       <Section status=Want userItems=wants editable />;
     } else {
       React.null;
     }}
    {if (willTrades->Array.length > 0) {
       <Section status=WillTrade userItems=willTrades editable />;
     } else {
       React.null;
     }}
  </div>;
};