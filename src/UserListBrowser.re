module Styles = {
  open Css;
  let root =
    style([
      padding2(~v=zero, ~h=px(16)),
      margin3(~top=px(16), ~bottom=zero, ~h=auto),
      media("(min-width: 640px)", [width(px(560))]),
      media("(min-width: 860px)", [width(px(752))]),
      media("(min-width: 1040px)", [width(px(944))]),
      media("(min-width: 1240px)", [width(px(1136))]),
      media("(min-width: 1440px)", [width(px(1328))]),
      position(relative),
    ]);
  let listLinks =
    style([
      display(flexBox),
      justifyContent(center),
      margin3(~top=zero, ~bottom=px(32), ~h=px(-16)),
    ]);
  let listLink =
    style([
      border(px(2), solid, transparent),
      borderRadius(px(4)),
      display(inlineBlock),
      fontSize(px(16)),
      textDecoration(none),
      lineHeight(px(16)),
      marginRight(px(16)),
      marginBottom(px(8)),
      padding3(~top=px(9), ~bottom=px(7), ~h=px(10)),
      transition(~duration=200, "all"),
      media("(max-width: 400px)", [marginRight(px(8))]),
      lastChild([marginRight(zero)]),
      hover([borderColor(hex("3aa56380"))]),
    ]);
  let listLinkSelected =
    style([borderColor(Colors.green), hover([borderColor(Colors.green)])]);
  let rootMini = style([backgroundColor(hex("fffffff0"))]);
  let filterBar = style([marginBottom(zero)]);
};

let getUrl =
    (
      ~url: ReasonReactRouter.url,
      ~urlSearchParams: Webapi.Url.URLSearchParams.t,
    ) => {
  "/"
  ++ Js.Array.joinWith("/", Belt.List.toArray(url.path))
  ++ (
    switch (Webapi.Url.URLSearchParams.toString(urlSearchParams)) {
    | "" => ""
    | search => "?" ++ search
    }
  );
};

// TODO
let numResultsPerPage = 20;
let getNumResultsPerPage = (~viewportWidth) =>
  if (viewportWidth >= 1440) {
    28;
  } else if (viewportWidth >= 1240) {
    24;
  } else if (viewportWidth >= 1040) {
    25;
  } else if (viewportWidth >= 860) {
    20;
  } else if (viewportWidth >= 640) {
    18;
  } else {
    12;
  };

[@react.component]
let make = (~user: User.t, ~list: string, ~url: ReasonReactRouter.url) => {
  let viewportWidth = Utils.useViewportWidth();
  let numResultsPerPage = getNumResultsPerPage(~viewportWidth);
  let listStatus = User.urlToItemStatus(list);
  let hasForTrade =
    React.useMemo1(
      () =>
        user.items
        ->Js.Dict.values
        ->Belt.Array.someU((. item: User.item) => item.status == ForTrade),
      [|user|],
    );
  let hasCanCraft =
    React.useMemo1(
      () =>
        user.items
        ->Js.Dict.values
        ->Belt.Array.someU((. item: User.item) => item.status == CanCraft),
      [|user|],
    );
  let hasWishlist =
    React.useMemo1(
      () =>
        user.items
        ->Js.Dict.values
        ->Belt.Array.someU((. item: User.item) => item.status == Wishlist),
      [|user|],
    );
  let userItems =
    React.useMemo2(
      () =>
        user.items
        ->Js.Dict.entries
        ->Belt.Array.keepMapU((. (itemKey, item: User.item)) =>
            item.status == listStatus
              ? Some((User.fromItemKey(~key=itemKey), item)) : None
          ),
      (user, list),
    );

  let (showMini, setShowMini) = React.useState(() => false);
  let (filters, pageOffset) =
    React.useMemo1(
      () =>
        ItemFilters.fromUrlSearch(
          ~urlSearch=url.search,
          ~defaultSort=SellPriceDesc,
        ),
      [|url.search|],
    );
  let setFilters = (filters: ItemFilters.t) => {
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Filters Changed",
      ~eventProperties={
        "filterText": filters.text,
        "filterMask": filters.mask,
        "filterCategory": filters.category,
        "filterSort": filters.sort,
      },
    );
    let urlSearchParams =
      Webapi.Url.URLSearchParams.makeWithArray(
        ItemFilters.serialize(
          ~filters,
          ~defaultSort=SellPriceDesc,
          ~pageOffset=0,
        ),
      );
    ReasonReactRouter.push(getUrl(~url, ~urlSearchParams));
  };
  let setPageOffset = f => {
    let nextPageOffset = f(pageOffset);
    let urlSearchParams =
      Webapi.Url.URLSearchParams.makeWithArray(
        ItemFilters.serialize(
          ~filters,
          ~defaultSort=SellPriceDesc,
          ~pageOffset=nextPageOffset,
        ),
      );
    ReasonReactRouter.push(getUrl(~url, ~urlSearchParams));
  };
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

  let renderListLink = (status: User.itemStatus) =>
    if (switch (status) {
        | Wishlist => hasWishlist
        | CanCraft => hasCanCraft
        | ForTrade => hasForTrade
        }) {
      <Link
        path={"/u/" ++ user.username ++ "/" ++ User.itemStatusToUrl(status)}
        className={Cn.make([
          Styles.listLink,
          Cn.ifTrue(Styles.listLinkSelected, status == listStatus),
        ])}>
        {React.string(User.itemStatusToString(status))}
      </Link>;
    } else {
      React.null;
    };

  <div className=Styles.root>
    <div className=Styles.listLinks>
      {renderListLink(ForTrade)}
      {renderListLink(CanCraft)}
      {renderListLink(Wishlist)}
    </div>
    <ItemFilters.CategoryButtons
      filters
      onChange={filters => setFilters(filters)}
    />
    <div
      className={Cn.make([ItemBrowser.Styles.filterBar, Styles.filterBar])}>
      <ItemFilters
        filters
        onChange={filters => setFilters(filters)}
        showCategorySort=true
      />
      <ItemFilters.Pager
        numResults
        pageOffset
        numResultsPerPage
        setPageOffset
      />
    </div>
    <div
      className={Cn.make([
        ItemBrowser.Styles.cards,
        UserItemBrowser.Styles.cards,
        Cn.ifTrue(UserItemBrowser.Styles.cardsMini, showMini),
      ])}>
      {filteredItems
       ->(
           showMini
             ? x => x
             : Belt.Array.slice(
                 ~offset=pageOffset * numResultsPerPage,
                 ~len=numResultsPerPage,
               )
         )
       ->Belt.Array.mapU((. ((itemId, variation), userItem)) => {
           showMini
             ? <UserItemBrowser.UserItemCardMini
                 itemId
                 variation
                 key={itemId ++ string_of_int(variation)}
               />
             : <UserItemBrowser.UserItemCard
                 itemId
                 variation
                 userItem
                 editable=false
                 showRecipe=false
                 key={itemId ++ string_of_int(variation)}
               />
         })
       ->React.array}
    </div>
    {!showMini
       ? <div className=ItemBrowser.Styles.bottomFilterBar>
           <ItemFilters.Pager
             numResults
             pageOffset
             numResultsPerPage
             setPageOffset={f => {
               setPageOffset(f);
               Webapi.Dom.(window |> Window.scrollTo(0., 0.));
             }}
           />
         </div>
       : React.null}
  </div>;
};