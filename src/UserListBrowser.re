module Styles = {
  open Css;
  let root =
    style([
      borderRadius(px(16)),
      margin2(~v=zero, ~h=auto),
      overflow(hidden),
      media("(min-width: 640px)", [width(px(624))]),
      media("(min-width: 860px)", [width(px(816))]),
      media("(min-width: 1040px)", [width(px(1008))]),
      media("(min-width: 1240px)", [width(px(1200))]),
      media("(min-width: 1440px)", [width(px(1392))]),
      media("(max-width: 640px)", [width(auto), borderRadius(zero)]),
    ]);
  let body =
    style([
      padding(px(32)),
      backgroundColor(hex("b0dec1c0")),
      position(relative),
      media("(max-width: 640px)", [padding(px(16))]),
    ]);
  let listLinks =
    style([
      display(flexBox),
      alignItems(center),
      backgroundColor(hex("ffffff80")),
      padding3(~top=px(18), ~bottom=px(16), ~h=px(32)),
      media(
        "(max-width: 640px)",
        [
          paddingLeft(px(16)),
          paddingRight(px(16)),
          whiteSpace(nowrap),
          overflowX(auto),
        ],
      ),
    ]);
  let profileLink =
    style([
      color(Colors.charcoal),
      display(inlineBlock),
      fontSize(px(20)),
      textDecoration(none),
      opacity(0.8),
      media("(max-width: 640px)", [fontSize(px(16))]),
      hover([opacity(1.)]),
    ]);
  let profileLinkIcon =
    style([
      flexShrink(0.),
      margin2(~v=zero, ~h=px(16)),
      top(zero),
      media("(max-width: 640px)", [margin2(~v=zero, ~h=px(8))]),
      media("(max-width: 400px)", [margin2(~v=zero, ~h=px(4))]),
    ]);
  let listLinkEmoji =
    style([
      display(inlineBlock),
      lineHeight(zero),
      marginRight(px(6)),
      position(relative),
      top(px(2)),
      media("(max-width: 460px)", [display(none)]),
    ]);
  let listLinkName = "listLinkName";
  let listLinkSelectedName =
    style([borderBottom(px(2), solid, Colors.charcoal)]);
  let listLink =
    style([
      color(Colors.charcoal),
      display(inlineBlock),
      fontSize(px(20)),
      textDecoration(none),
      marginRight(px(28)),
      media("(max-width: 640px)", [fontSize(px(16))]),
      media("(max-width: 400px)", [marginRight(px(16))]),
      lastChild([marginRight(zero)]),
      hover([
        selector(
          "& ." ++ listLinkName,
          [borderBottom(px(2), solid, Colors.charcoal)],
        ),
      ]),
    ]);
  let rootMini = style([important(backgroundColor(hex("fffffff0")))]);
  let filterBar = style([marginBottom(zero)]);
  let sectionToggles =
    style([
      position(absolute),
      right(px(32)),
      top(px(-40)),
      display(flexBox),
      media(
        "(max-width: 860px)",
        [position(static), textAlign(center), marginTop(px(8))],
      ),
      media("(max-width: 470px)", [marginBottom(px(16))]),
    ]);
};

let getUrl =
    (
      ~url: ReasonReactRouter.url,
      ~urlSearchParams: Webapi.Url.URLSearchParams.t,
    ) => {
  Utils.getPath(~url)
  ++ (
    switch (Webapi.Url.URLSearchParams.toString(urlSearchParams)) {
    | "" => ""
    | search => "?" ++ search
    }
  );
};

let getNumResultsPerPage = (~viewportWidth) =>
  if (viewportWidth >= 1440) {
    49;
  } else if (viewportWidth >= 1240) {
    42;
  } else if (viewportWidth >= 1040) {
    35;
  } else if (viewportWidth >= 860) {
    28;
  } else if (viewportWidth >= 640) {
    21;
  } else {
    20;
  };

let userItemsHasOneWithStatus = (~userItems, ~status) => {
  userItems
  ->Js.Dict.values
  ->Belt.Array.someU((. item: User.item) => item.status == status);
};

[@react.component]
let make =
    (
      ~user: User.t,
      ~list: ViewingList.t,
      ~url: ReasonReactRouter.url,
      ~me=false,
      (),
    ) => {
  let viewportWidth = Utils.useViewportWidth();
  let numResultsPerPage = getNumResultsPerPage(~viewportWidth);
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
            ViewingList.doesUserItemStatusMatchViewingList(item.status, list)
              ? User.fromItemKey(~key=itemKey)
                ->Belt.Option.map(x => (x, item))
              : None
          ),
      (user, list),
    );
  let userItemIds =
    userItems->Belt.Array.mapU((. ((itemId, _variant), _)) => itemId);

  let url = ReasonReactRouter.useUrl();
  let showMini =
    Webapi.Url.URLSearchParams.(make(url.search) |> has("thumbnails"));
  let (showRecipes, setShowRecipes) = React.useState(() => false);
  let (filters, pageOffset) =
    React.useMemo1(
      () =>
        ItemFilters.fromUrlSearch(
          ~urlSearch=url.search,
          ~defaultSort=UserDefault,
        ),
      [|url.search|],
    );
  let numFiltersChangeLogged = React.useRef(0);
  let setFilters = (filters: ItemFilters.t) => {
    let urlSearchParams =
      Webapi.Url.URLSearchParams.makeWithArray(
        ItemFilters.serialize(
          ~filters,
          ~defaultSort=UserDefault,
          ~pageOffset=0,
        ),
      );
    if (showMini) {
      urlSearchParams |> Webapi.Url.URLSearchParams.append("thumbnails", "");
    };
    let url = ReasonReactRouter.dangerouslyGetInitialUrl();
    let newUrl = getUrl(~url, ~urlSearchParams);
    ReasonReactRouter.push(newUrl);
    if (React.Ref.current(numFiltersChangeLogged) < 2) {
      Analytics.Amplitude.logEventWithProperties(
        ~eventName="Filters Changed",
        ~eventProperties={
          "filterText": filters.text,
          "filterMask": filters.mask,
          "filterCategory": filters.category,
          "filterSort": filters.sort,
        },
      );
      React.Ref.setCurrent(
        numFiltersChangeLogged,
        React.Ref.current(numFiltersChangeLogged) + 1,
      );
    };
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
        let sortFn =
          ItemFilters.getUserItemSort(
            ~prioritizeViewerStatuses=?
              !me
                ? Some(
                    switch (list) {
                    | Wishlist => [|User.ForTrade, User.CanCraft|]
                    | ForTrade
                    | CanCraft
                    | Catalog => [|User.Wishlist|]
                    },
                  )
                : None,
            ~sort=filters.sort,
          );
        userItems->Belt.Array.keep((((itemId, _), _)) =>
          ItemFilters.doesItemMatchFilters(
            ~item=Item.getItem(~itemId),
            ~filters,
          )
        )
        |> Js.Array.sortInPlaceWith((aUserItem, bUserItem) =>
             sortFn(aUserItem, bUserItem)
           );
      },
      (userItems, filters),
    );
  let numResults = filteredItems->Belt.Array.length;

  let renderListLink = (linkList: ViewingList.t) =>
    if (switch (linkList) {
        | Wishlist => hasWishlist
        | CanCraft => hasCanCraft
        | ForTrade => hasForTrade
        }) {
      <Link
        path={
          "/u/"
          ++ user.username
          ++ "/"
          ++ ViewingList.viewingListToUrl(linkList)
        }
        className=Styles.listLink>
        <span className=Styles.listLinkEmoji>
          {React.string(ViewingList.viewingListToEmoji(linkList))}
        </span>
        <span
          className={Cn.make([
            Styles.listLinkName,
            Cn.ifTrue(Styles.listLinkSelectedName, linkList == list),
          ])}>
          {React.string(ViewingList.viewingListToString(linkList))}
        </span>
      </Link>;
    } else {
      React.null;
    };

  let rootRef = React.useRef(Js.Nullable.null);
  React.useEffect0(() => {
    if (TemporaryState.state^ == Some(FromProfileBrowser)) {
      TemporaryState.state := None;
      let rootElement = Utils.getElementForDomRef(rootRef);
      open Webapi.Dom;
      let boundingRect = Element.getBoundingClientRect(rootElement);
      window |> Window.scrollBy(0., DomRect.top(boundingRect) -. 32.);
    } else {
      Webapi.Dom.(window |> Window.scrollTo(0., 0.));
    };
    None;
  });

  <div className=Styles.root ref={ReactDOMRe.Ref.domRef(rootRef)}>
    <div className=Styles.listLinks>
      <Link path={"/u/" ++ user.username} className=Styles.profileLink>
        {React.string(user.username)}
      </Link>
      <span
        className={Cn.make([
          UserProfileBrowser.Styles.sectionTitleLinkIcon,
          Styles.profileLinkIcon,
        ])}
      />
      {renderListLink(ForTrade)}
      {renderListLink(CanCraft)}
      {renderListLink(Wishlist)}
    </div>
    <div
      className={Cn.make([
        Styles.body,
        Cn.ifTrue(Styles.rootMini, showMini),
      ])}>
      {Js.Array.length(userItems) > 8
         ? <div
             className={Cn.make([
               ItemBrowser.Styles.filterBar,
               Styles.filterBar,
             ])}>
             <ItemFilters
               userItemIds
               filters
               isViewingSelf=me
               onChange={filters => setFilters(filters)}
             />
             {!showMini
                ? <ItemFilters.Pager
                    numResults
                    pageOffset
                    numResultsPerPage
                    setPageOffset
                  />
                : React.null}
           </div>
         : React.null}
      <div className=Styles.sectionToggles>
        <div className=UserProfileBrowser.Styles.showRecipesBox>
          <label
            htmlFor="show-mini"
            className=UserProfileBrowser.Styles.showRecipesLabel>
            {React.string("Thumbnails")}
          </label>
          <input
            id="show-mini"
            type_="checkbox"
            checked=showMini
            onChange={e => {
              let checked = ReactEvent.Form.target(e)##checked;
              Analytics.Amplitude.logEventWithProperties(
                ~eventName="Miniature Mode Clicked",
                ~eventProperties={"checked": checked, "status": list},
              );
              let urlSearchParams =
                Webapi.Url.URLSearchParams.makeWithArray(
                  ItemFilters.serialize(
                    ~filters,
                    ~defaultSort=UserDefault,
                    ~pageOffset=0,
                  ),
                );
              if (checked) {
                urlSearchParams
                |> Webapi.Url.URLSearchParams.append("thumbnails", "");
              };
              let url = ReasonReactRouter.dangerouslyGetInitialUrl();
              let newUrl = getUrl(~url, ~urlSearchParams);
              ReasonReactRouter.replace(newUrl);
            }}
            className=UserProfileBrowser.Styles.showRecipesCheckbox
          />
        </div>
        <div className=UserProfileBrowser.Styles.showRecipesBox>
          <label
            htmlFor="craftShowRecipe"
            className={Cn.make([
              UserProfileBrowser.Styles.showRecipesLabel,
              Cn.ifTrue(
                UserProfileBrowser.Styles.showRecipesLabelDisabled,
                showMini,
              ),
            ])}>
            {React.string("Show Recipes")}
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
            className=UserProfileBrowser.Styles.showRecipesCheckbox
          />
        </div>
      </div>
      <div
        className={Cn.make([
          ItemBrowser.Styles.cards,
          UserProfileBrowser.Styles.cards,
          Cn.ifTrue(UserProfileBrowser.Styles.cardsMini, showMini),
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
               ? <UserProfileBrowser.UserItemCardMini
                   itemId
                   variation
                   key={string_of_int(itemId) ++ string_of_int(variation)}
                 />
               : <UserItemCard
                   itemId
                   variation
                   userItem
                   list
                   editable=me
                   showRecipe=showRecipes
                   key={string_of_int(itemId) ++ string_of_int(variation)}
                 />
           })
         ->React.array}
      </div>
      {!showMini && Js.Array.length(filteredItems) > numResultsPerPage
         ? <div className=ItemBrowser.Styles.bottomFilterBar>
             <ItemFilters.Pager
               numResults
               pageOffset
               numResultsPerPage
               setPageOffset={f => {
                 setPageOffset(f);
                 let rootElement = Utils.getElementForDomRef(rootRef);
                 open Webapi.Dom;
                 let boundingRect =
                   Element.getBoundingClientRect(rootElement);
                 window |> Window.scrollBy(0., DomRect.top(boundingRect));
               }}
             />
           </div>
         : React.null}
    </div>
  </div>;
};