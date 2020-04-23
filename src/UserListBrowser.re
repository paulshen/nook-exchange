module Styles = {
  open Css;
  let root =
    style([
      padding(px(32)),
      borderRadius(px(16)),
      backgroundColor(hex("88c9a1a0")),
      margin3(~top=px(16), ~bottom=zero, ~h=auto),
      media("(min-width: 640px)", [width(px(560))]),
      media("(min-width: 860px)", [width(px(752))]),
      media("(min-width: 1040px)", [width(px(944))]),
      media("(min-width: 1240px)", [width(px(1136))]),
      media("(min-width: 1440px)", [width(px(1328))]),
      position(relative),
      media(
        "(max-width: 640px)",
        [width(auto), padding(px(16)), borderRadius(zero)],
      ),
    ]);
  let listLinks =
    style([
      display(flexBox),
      justifyContent(center),
      marginTop(px(48)),
      marginBottom(px(16)),
    ]);
  let listLinkEmoji = style([display(inlineBlock), marginRight(px(6))]);
  let listLinkName = "listLinkName";
  let listLinkSelectedName =
    style([borderBottom(px(2), solid, Colors.green)]);
  let listLink =
    style([
      display(inlineBlock),
      fontSize(px(16)),
      textDecoration(none),
      lineHeight(px(16)),
      marginRight(px(24)),
      marginBottom(px(16)),
      media("(max-width: 400px)", [marginRight(px(16))]),
      lastChild([marginRight(zero)]),
      hover([
        selector(
          "& ." ++ listLinkName,
          [borderBottom(px(2), solid, Colors.green)],
        ),
      ]),
    ]);
  let rootMini = style([important(backgroundColor(hex("fffffff0")))]);
  let filterBar = style([marginBottom(zero)]);
  let sectionToggles =
    style([
      position(absolute),
      right(px(32)),
      top(px(-32)),
      display(flexBox),
      media(
        "(max-width: 860px)",
        [position(static), textAlign(center), marginTop(px(8))],
      ),
      media("(max-width: 470px)", [marginBottom(px(16))]),
    ]);
  let showRecipesLabelDisabled = style([opacity(0.5)]);
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

let userItemsHasOneWithStatus = (~userItems, ~status) => {
  userItems
  ->Js.Dict.values
  ->Belt.Array.someU((. item: User.item) => item.status == status);
};

[@react.component]
let make = (~user: User.t, ~list: string, ~url: ReasonReactRouter.url) => {
  let viewportWidth = Utils.useViewportWidth();
  let numResultsPerPage = getNumResultsPerPage(~viewportWidth);
  let listStatus = User.urlToItemStatus(list);
  let hasForTrade =
    React.useMemo1(
      () =>
        userItemsHasOneWithStatus(~userItems=user.items, ~status=ForTrade),
      [|user|],
    );
  let hasCanCraft =
    React.useMemo1(
      () =>
        userItemsHasOneWithStatus(~userItems=user.items, ~status=CanCraft),
      [|user|],
    );
  let hasWishlist =
    React.useMemo1(
      () =>
        userItemsHasOneWithStatus(~userItems=user.items, ~status=Wishlist),
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
  let userItemIds =
    userItems->Belt.Array.mapU((. ((itemId, _variant), _)) => itemId);

  let (showMini, setShowMini) = React.useState(() => false);
  let (showRecipes, setShowRecipes) = React.useState(() => false);
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
        let sortFn = ItemFilters.getSort(~sort=filters.sort);
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
        className=Styles.listLink>
        <span className=Styles.listLinkEmoji>
          {React.string(User.itemStatusToEmoji(status))}
        </span>
        <span
          className={Cn.make([
            Styles.listLinkName,
            Cn.ifTrue(Styles.listLinkSelectedName, status == listStatus),
          ])}>
          {React.string(User.itemStatusToString(status))}
        </span>
      </Link>;
    } else {
      React.null;
    };

  React.useEffect0(() => {
    Webapi.Dom.(window |> Window.scrollTo(0., 0.));
    None;
  });

  <>
    <div className=Styles.listLinks>
      {renderListLink(ForTrade)}
      {renderListLink(CanCraft)}
      {renderListLink(Wishlist)}
    </div>
    <div
      className={Cn.make([
        Styles.root,
        Cn.ifTrue(Styles.rootMini, showMini),
      ])}>
      <ItemFilters.CategoryButtons
        userItemIds
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
      <div className=Styles.sectionToggles>
        {userItems->Array.length > 16
           ? <div>
               <label
                 htmlFor="show-mini"
                 className=UserItemBrowser.Styles.showRecipesLabel>
                 {React.string("Miniature")}
               </label>
               <input
                 id="show-mini"
                 type_="checkbox"
                 checked=showMini
                 onChange={e => {
                   let checked = ReactEvent.Form.target(e)##checked;
                   Analytics.Amplitude.logEventWithProperties(
                     ~eventName="Miniature Mode Clicked",
                     ~eventProperties={
                       "checked": checked,
                       "status": listStatus,
                     },
                   );
                   setShowMini(_ => checked);
                 }}
                 className=UserItemBrowser.Styles.showRecipesCheckbox
               />
             </div>
           : React.null}
        <div className=UserItemBrowser.Styles.showRecipesBox>
          <label
            htmlFor="craftShowRecipe"
            className={Cn.make([
              UserItemBrowser.Styles.showRecipesLabel,
              Cn.ifTrue(Styles.showRecipesLabelDisabled, showMini),
            ])}>
            {React.string("Recipes")}
          </label>
          <input
            id="craftShowRecipe"
            type_="checkbox"
            checked={showRecipes && !showMini}
            onChange={e => {
              let checked = ReactEvent.Form.target(e)##checked;
              Analytics.Amplitude.logEventWithProperties(
                ~eventName="Show Recipes Clicked",
                ~eventProperties={"checked": checked},
              );
              setShowRecipes(_ => checked);
            }}
            disabled=showMini
            className=UserItemBrowser.Styles.showRecipesCheckbox
          />
        </div>
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
                   showRecipe=showRecipes
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
    </div>
  </>;
};