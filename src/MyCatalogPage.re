module Styles = {
  open Css;
  let title =
    style([fontSize(px(36)), textAlign(center), marginBottom(px(16))]);
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
  let userBodyParagraph =
    style([
      marginBottom(px(12)),
      whiteSpace(`preLine),
      media("(max-width: 450px)", [whiteSpace(normal)]),
    ]);
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
    let filterBar = style([marginBottom(zero)]);
    let cards =
      style([
        paddingTop(px(16)),
        marginRight(px(-16)),
        media("(max-width: 470px)", [paddingTop(zero)]),
      ]);
  };

  let getNumResultsPerPage = (~viewportWidth) =>
    if (viewportWidth >= 1620) {
      40;
    } else if (viewportWidth >= 1400) {
      35;
    } else if (viewportWidth >= 1200) {
      30;
    } else if (viewportWidth >= 1000) {
      25;
    } else if (viewportWidth >= 800) {
      24;
    } else if (viewportWidth >= 600) {
      21;
    } else {
      20;
    };

  [@react.component]
  let make = (~userItems: array(((string, int), User.item))) => {
    let viewportWidth = Utils.useViewportWidth();
    let numResultsPerPage = getNumResultsPerPage(~viewportWidth);
    let (filters, setFilters) =
      React.useState(() =>
        (
          {
            text: "",
            mask: None,
            category: None,
            exclude: [||],
            sort: UserDefault,
          }: ItemFilters.t
        )
      );
    let (pageOffset, setPageOffset) = React.useState(() => 0);
    let filteredItems =
      React.useMemo2(
        () => {
          let sortFn = ItemFilters.getUserItemSort(~sort=filters.sort);
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
    let userItemIds =
      userItems->Belt.Array.mapU((. ((itemId, _variant), _)) => itemId);
    let numResults = filteredItems->Belt.Array.length;

    <div className={Cn.make([Styles.root])}>
      <div
        className={Cn.make([ItemBrowser.Styles.filterBar, Styles.filterBar])}>
        <ItemFilters
          filters
          onChange={filters => {
            setFilters(_ => filters);
            setPageOffset(_ => 0);
          }}
          userItemIds
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
             <UserItemCard
               itemId
               variation
               listStatus=User.CatalogOnly
               userItem
               editable=true
               showRecipe=false
               onCatalogPage=true
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

module Loaded = {
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
      <div className=Styles.title> {React.string("My Catalog")} </div>
      <div className=Styles.userBody>
        <div className=Styles.userBodyParagraph>
          {React.string(
             {j|You are getting a sneak preview of your catalog 🤫. This is a list of items in your For Trade and Can Craft. You can also have catalog-only items.|j},
           )}
        </div>
        {user.enableCatalogCheckbox
           ? <div className=Styles.userBodyParagraph>
               {React.string(
                  "Catalog checkboxes are enabled! Add items in the ",
                )}
               <Link path="/"> {React.string("item browser")} </Link>
               {React.string(".\nDon't want to see checkmarks anymore? ")}
               <a
                 href="#"
                 onClick={e => {
                   ReactEvent.Mouse.preventDefault(e);
                   UserStore.toggleCatalogCheckboxSetting(~enabled=false);
                   Analytics.Amplitude.logEventWithProperties(
                     ~eventName="Catalog Checkbox Setting Toggled",
                     ~eventProperties={"enabled": false},
                   );
                 }}>
                 {React.string("Hide catalog checkboxes")}
               </a>
               {React.string(".")}
             </div>
           : <div className=Styles.userBodyParagraph>
               {React.string("To add catalog-only items, ")}
               <a
                 href="#"
                 onClick={e => {
                   ReactEvent.Mouse.preventDefault(e);
                   UserStore.toggleCatalogCheckboxSetting(~enabled=true);
                   Analytics.Amplitude.logEventWithProperties(
                     ~eventName="Catalog Checkbox Setting Toggled",
                     ~eventProperties={"enabled": true},
                   );
                 }}>
                 {React.string("enable catalog checkboxes")}
               </a>
               {React.string(".")}
             </div>}
        <div>
          {React.string("Do you like it? How can it be better? ")}
          <a href="https://twitter.com/nookexchange" target="_blank">
            {React.string("Let us know!")}
          </a>
        </div>
      </div>
      <Catalog userItems=catalog />
    </div>;
  };
};

[@react.component]
let make = () => {
  let me = UserStore.useMe();
  switch (me) {
  | Some(me) => <Loaded user=me />
  | None => React.null
  };
};