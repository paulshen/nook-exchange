module Styles = {
  open Css;
  let emptyProfile =
    style([
      maxWidth(px(768)),
      margin3(~top=px(32), ~bottom=px(128), ~h=auto),
      textAlign(center),
    ]);
  let username =
    style([
      fontSize(px(36)),
      textAlign(center),
      marginTop(px(32)),
      marginBottom(px(32)),
    ]);
  let userBody =
    style([
      backgroundColor(hex("ffffffc0")),
      boxSizing(borderBox),
      lineHeight(px(20)),
      margin2(~v=px(16), ~h=auto),
      maxWidth(px(512)),
      padding2(~v=px(16), ~h=px(24)),
      borderRadius(px(8)),
      media("(max-width: 512px)", [borderRadius(zero), padding(px(16))]),
    ]);
  let bodyText = style([fontSize(px(18))]);
};

module Catalog = {
  module Styles = {
    open Css;
    let root =
      style([
        width(px(368)),
        margin3(~top=px(32), ~bottom=zero, ~h=auto),
        media("(min-width: 600px)", [width(px(560))]),
        media("(min-width: 800px)", [width(px(752))]),
        media("(min-width: 1000px)", [width(px(944))]),
        media("(min-width: 1200px)", [width(px(1136))]),
        media("(min-width: 1400px)", [width(px(1328))]),
        media("(min-width: 1620px)", [width(px(1520))]),
        padding2(~v=zero, ~h=px(16)),
        position(relative),
        media(
          "(max-width: 470px)",
          [width(auto), padding(px(16)), borderRadius(zero)],
        ),
      ]);
    let cards =
      style([
        paddingTop(px(16)),
        marginRight(px(-16)),
        media("(max-width: 470px)", [paddingTop(zero)]),
      ]);
  };

  let numResultsPerPage = 60;

  [@react.component]
  let make = (~userItems: array(((string, int), User.item))) => {
    let (filters, setFilters) =
      React.useState(() =>
        (
          {text: "", mask: None, category: None, sort: SellPriceDesc}: ItemFilters.t
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

    <div className={Cn.make([Styles.root])}>
      <ItemFilters.CategoryButtons
        filters
        onChange={filters => {
          setFilters(_ => filters);
          setPageOffset(_ => 0);
        }}
      />
      <div className=ItemBrowser.Styles.filterBar>
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
      <div className={Cn.make([ItemBrowser.Styles.cards, Styles.cards])}>
        {filteredItems
         ->(
             Belt.Array.slice(
               ~offset=pageOffset * numResultsPerPage,
               ~len=numResultsPerPage,
             )
           )
         ->Belt.Array.mapU((. ((itemId, variation), userItem)) => {
             <UserItemBrowser.UserItemCard
               itemId
               variation
               userItem
               editable=true
               showRecipe=false
               key={itemId ++ string_of_int(variation)}
             />
           })
         ->React.array}
      </div>
      <div className=ItemBrowser.Styles.bottomFilterBar>
        <ItemFilters.Pager
          numResults
          pageOffset
          numResultsPerPage
          setPageOffset
        />
      </div>
    </div>;
  };
};

[@react.component]
let make = (~user: User.t) => {
  let userItems =
    React.useMemo1(
      () =>
        user.items
        ->Js.Dict.entries
        ->Belt.Array.mapU((. (itemKey, item)) =>
            (User.fromItemKey(~key=itemKey), item)
          ),
      [|user.items|],
    );
  let catalog =
    userItems->Belt.Array.keepU((. (_, item: User.item)) =>
      item.status != Wishlist
    );

  <div>
    <div className=Styles.username> {React.string("My Catalog")} </div>
    <div className=Styles.userBody>
      <div>
        {React.string(
           "This is the list of all items in your catalog, including your For Trade and Can Craft items.",
         )}
      </div>
    </div>
    <Catalog userItems=catalog />
  </div>;
};