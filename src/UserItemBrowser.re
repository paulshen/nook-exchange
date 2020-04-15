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
      backgroundColor(hex("88c9a1a0")),
      position(relative),
      media(
        "(max-width: 470px)",
        [width(auto), padding(px(16)), borderRadius(zero)],
      ),
    ]);
  let rootForTrade = style([backgroundColor(hex("8FCDE0a0"))]);
  let rootCanCraft = style([backgroundColor(hex("f1e26fa0"))]);
  let sectionTitle =
    style([fontSize(px(24)), marginBottom(px(16)), textAlign(center)]);
  let filterBar = style([marginTop(px(32)), marginBottom(zero)]);
  let cards =
    style([
      paddingTop(px(16)),
      marginRight(px(-16)),
      media("(max-width: 470px)", [paddingTop(zero)]),
    ]);
  let metaIcons = style([opacity(0.), transition(~duration=200, "all")]);
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
      media("(max-width: 430px)", [width(pct(100.))]),
      hover([selector("& ." ++ metaIcons, [opacity(1.)])]),
    ]);
  let name =
    style([fontSize(px(16)), marginBottom(px(4)), textAlign(center)]);
  let userNote =
    style([
      borderTop(px(1), solid, hex("f0f0f0")),
      unsafe("alignSelf", "stretch"),
      lineHeight(px(18)),
      marginTop(px(-8)),
      padding3(~top=px(8), ~bottom=zero, ~h=px(4)),
    ]);
  let removeButton = style([top(px(9)), bottom(initial)]);
  let showRecipesBox =
    style([
      position(absolute),
      right(px(32)),
      top(px(36)),
      media("(max-width: 600px)", [position(static), textAlign(center)]),
      media("(max-width: 470px)", [marginBottom(px(16))]),
    ]);
  let showRecipesLabel = style([fontSize(px(16)), marginRight(px(8))]);
  let showRecipesCheckbox =
    style([
      fontSize(px(24)),
      margin(zero),
      position(relative),
      top(px(-2)),
    ]);
  let recipe =
    style([marginTop(px(6)), textAlign(center), fontSize(px(12))]);
};

open Belt;

module UserItemCard = {
  [@react.component]
  let make =
      (~itemId, ~variation, ~userItem: User.item, ~editable, ~showRecipe) => {
    let item = Item.getItem(~itemId);
    <div className={Cn.make([Styles.card])}>
      <div className=ItemCard.Styles.body>
        <img
          src={
            Constants.cdnUrl
            ++ "/items/"
            ++ item.image
            ++ "__"
            ++ string_of_int(variation)
            ++ ".png"
          }
          className=ItemCard.Styles.mainImage
        />
        <div className=Styles.name> {React.string(item.name)} </div>
        {switch (showRecipe, item.recipe) {
         | (true, Some(recipe)) =>
           <div className=Styles.recipe>
             {recipe
              ->Array.map(((itemId, quantity)) =>
                  <div key=itemId>
                    {React.string(itemId ++ " x " ++ string_of_int(quantity))}
                  </div>
                )
              ->React.array}
           </div>
         | _ => React.null
         }}
      </div>
      <div className={Cn.make([ItemCard.Styles.metaIcons, Styles.metaIcons])}>
        {switch (item.recipe) {
         | Some(recipe) => <ItemCard.RecipeIcon recipe />
         | None => React.null
         }}
        {if (item.orderable) {
           <ItemCard.OrderableIcon />;
         } else {
           React.null;
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
               {Emoji.parseText(userItem.note)}
             </div>;
           } else {
             React.null;
           }
         )}
    </div>;
  };
};

module Section = {
  let numResultsPerPage = 60;

  [@react.component]
  let make =
      (
        ~status: User.itemStatus,
        ~userItems: array(((string, int), User.item)),
        ~editable,
      ) => {
    let (showRecipes, setShowRecipes) = React.useState(() => false);
    let (filters, setFilters) =
      React.useState(() =>
        (
          {
            text: "",
            orderable: None,
            hasRecipe: None,
            category: None,
            sort: SellPriceDesc,
          }: ItemFilters.t
        )
      );
    let (pageOffset, setPageOffset) = React.useState(() => 0);
    let filteredItems =
      React.useMemo2(
        () => {
          let sortFn = ItemFilters.getSort(~filters);
          userItems->Belt.Array.keep((((itemId, _), _)) =>
            ItemFilters.doesItemMatchFilters(
              ~item=Item.getItem(~itemId),
              ~filters,
            )
          )
          |> Js.Array.sortInPlaceWith((((aId, _), _), ((bId, _), _)) =>
               sortFn(Item.getItem(~itemId=aId), Item.getItem(~itemId=bId))
             );
        },
        (userItems, filters),
      );
    let numResults = filteredItems->Belt.Array.length;
    let showFilters = numResults > numResultsPerPage;

    <div
      className={Cn.make([
        Styles.root,
        Cn.ifTrue(Styles.rootForTrade, status == ForTrade),
        Cn.ifTrue(Styles.rootCanCraft, status == CanCraft),
      ])}>
      <div className=Styles.sectionTitle>
        {React.string(
           switch (status) {
           | Wishlist => {j|🙏 Wishlist|j}
           | ForTrade => {j|✅ For Trade|j}
           | CanCraft => {j|🔨 Can Craft|j}
           },
         )}
      </div>
      {if (status == CanCraft) {
         <div className=Styles.showRecipesBox>
           <label htmlFor="craftShowRecipe" className=Styles.showRecipesLabel>
             {React.string("Show Recipes")}
           </label>
           <input
             id="craftShowRecipe"
             type_="checkbox"
             checked=showRecipes
             onChange={e => {
               let checked = ReactEvent.Form.target(e)##checked;
               Analytics.Amplitude.logEventWithProperties(
                 ~eventName="Show Recipes Clicked",
                 ~eventProperties={"checked": checked},
               );
               setShowRecipes(_ => checked);
             }}
             className=Styles.showRecipesCheckbox
           />
         </div>;
       } else {
         React.null;
       }}
      {showFilters
         ? <div
             className={Cn.make([
               ItemBrowser.Styles.filterBar,
               Styles.filterBar,
             ])}>
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
               showRecipe=showRecipes
               key={itemId ++ string_of_int(variation)}
             />
           })
         ->React.array}
      </div>
      {showFilters
         ? <div className=ItemBrowser.Styles.bottomFilterBar>
             <ItemFilters.Pager
               numResults
               pageOffset
               numResultsPerPage
               setPageOffset
             />
           </div>
         : React.null}
    </div>;
  };
};

[@react.component]
let make = (~userItems: array(((string, int), User.item)), ~editable) => {
  let wishlist =
    userItems->Array.keepU((. (_, item: User.item)) =>
      item.status == Wishlist
    );
  let forTradeList =
    userItems->Array.keepU((. (_, item: User.item)) =>
      item.status == ForTrade
    );
  let canCraftList =
    userItems->Array.keepU((. (_, item: User.item)) =>
      item.status == CanCraft
    );
  <div>
    {if (forTradeList->Array.length > 0) {
       <Section status=ForTrade userItems=forTradeList editable />;
     } else {
       React.null;
     }}
    {if (canCraftList->Array.length > 0) {
       <Section status=CanCraft userItems=canCraftList editable />;
     } else {
       React.null;
     }}
    {if (wishlist->Array.length > 0) {
       <Section status=Wishlist userItems=wishlist editable />;
     } else {
       React.null;
     }}
  </div>;
};