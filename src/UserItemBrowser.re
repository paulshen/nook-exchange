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
  let anchor = style([position(absolute), top(px(-100))]);
  let rootForTrade = style([backgroundColor(hex("8FCDE0a0"))]);
  let rootCanCraft = style([backgroundColor(hex("f1e26fa0"))]);
  let rootMini = style([backgroundColor(hex("fffffff0"))]);
  let sectionTitle =
    style([fontSize(px(24)), marginBottom(px(16)), textAlign(center)]);
  let filterBar = style([marginTop(px(32)), marginBottom(zero)]);
  let cards =
    style([
      paddingTop(px(16)),
      marginRight(px(-16)),
      media("(max-width: 470px)", [paddingTop(zero)]),
    ]);
  let cardsMini =
    style([
      justifyContent(flexStart),
      paddingTop(px(32)),
      marginLeft(px(-8)),
      marginRight(px(-8)),
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
  let cardMiniImage =
    style([display(block), width(px(64)), height(px(64))]);
  let sectionToggles =
    style([
      position(absolute),
      right(px(32)),
      top(px(36)),
      display(flexBox),
      flexDirection(column),
      alignItems(flexEnd),
      media(
        "(max-width: 600px)",
        [position(static), textAlign(center), flexDirection(row)],
      ),
      media("(max-width: 470px)", [marginBottom(px(16))]),
    ]);
  let showRecipesBox = style([marginLeft(px(16))]);
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
          src={Item.getImageUrl(~item, ~variant=variation)}
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

module UserItemCardMini = {
  [@react.component]
  let make = (~itemId, ~variation) => {
    let item = Item.getItem(~itemId);
    <img
      src={Item.getImageUrl(~item, ~variant=variation)}
      title={item.name}
      className=Styles.cardMiniImage
    />;
  };
};

module Section = {
  let numResultsPerPage = 60;

  let randomString = () => Js.Math.random()->Js.Float.toString;

  [@react.component]
  let make =
      (
        ~status: User.itemStatus,
        ~userItems: array(((string, int), User.item)),
        ~editable,
      ) => {
    let id = React.useMemo0(() => randomString());
    let (showRecipes, setShowRecipes) = React.useState(() => false);
    let (showMini, setShowMini) = React.useState(() => false);
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
    let showFilters = userItems->Belt.Array.length > numResultsPerPage;

    let anchorId =
      switch (status) {
      | ForTrade => "for-trade"
      | CanCraft => "can-craft"
      | Wishlist => "wishlist"
      };
    let anchorRef = React.useRef(Js.Nullable.null);
    React.useEffect0(() => {
      let url = ReasonReactRouter.dangerouslyGetInitialUrl();
      if (url.hash == anchorId) {
        open Webapi.Dom;
        let anchorElement = Utils.getElementForDomRef(anchorRef);
        anchorElement->Element.scrollIntoView;
      };
      None;
    });

    <div
      className={Cn.make([
        Styles.root,
        Cn.ifTrue(Styles.rootForTrade, status == ForTrade),
        Cn.ifTrue(Styles.rootCanCraft, status == CanCraft),
        Cn.ifTrue(Styles.rootMini, showMini),
      ])}>
      <div
        className=Styles.anchor
        id=anchorId
        ref={ReactDOMRe.Ref.domRef(anchorRef)}
      />
      <div className=Styles.sectionTitle>
        {React.string(
           switch (status) {
           | Wishlist => {j|🙏 Wishlist|j}
           | ForTrade => {j|✅ For Trade|j}
           | CanCraft => {j|🔨 Can Craft|j}
           },
         )}
      </div>
      <div className=Styles.sectionToggles>
        {userItems->Array.length > 16
           ? <div>
               <label htmlFor=id className=Styles.showRecipesLabel>
                 {React.string("Miniature")}
               </label>
               <input
                 id
                 type_="checkbox"
                 checked=showMini
                 onChange={e => {
                   let checked = ReactEvent.Form.target(e)##checked;
                   Analytics.Amplitude.logEventWithProperties(
                     ~eventName="Miniature Mode Clicked",
                     ~eventProperties={"checked": checked, "status": status},
                   );
                   setShowMini(_ => checked);
                 }}
                 className=Styles.showRecipesCheckbox
               />
             </div>
           : React.null}
        {if (status == CanCraft && !showMini) {
           <div className=Styles.showRecipesBox>
             <label
               htmlFor="craftShowRecipe" className=Styles.showRecipesLabel>
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
      </div>
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
      <div
        className={Cn.make([
          ItemBrowser.Styles.cards,
          Styles.cards,
          Cn.ifTrue(Styles.cardsMini, showMini),
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
               ? <UserItemCardMini
                   itemId
                   variation
                   key={itemId ++ string_of_int(variation)}
                 />
               : <UserItemCard
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
      {showFilters && !showMini
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

module ListLinks = {
  module Styles = {
    open Css;
    let root = style([textAlign(center)]);
    let link = style([marginLeft(px(8))]);
  };

  [@react.component]
  let make = (~hasForTrade, ~hasCanCraft, ~hasWishlist) => {
    <div className=Styles.root>
      {React.string("Lists:")}
      {hasForTrade
         ? <a href="#for-trade" className=Styles.link>
             {React.string("For Trade")}
           </a>
         : React.null}
      {hasCanCraft
         ? <a href="#can-craft" className=Styles.link>
             {React.string("Can Craft")}
           </a>
         : React.null}
      {hasWishlist
         ? <a href="#wishlist" className=Styles.link>
             {React.string("Wishlist")}
           </a>
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
  let hasForTrade = forTradeList->Array.length > 0;
  let hasCanCraft = canCraftList->Array.length > 0;
  let hasWishlist = wishlist->Array.length > 0;
  let showListLinks =
    forTradeList->Array.length > 16
    && hasCanCraft
    || forTradeList->Array.length
    + canCraftList->Array.length > 16
    && hasWishlist;
  <div>
    {showListLinks
       ? <ListLinks hasForTrade hasCanCraft hasWishlist /> : React.null}
    {if (hasForTrade) {
       <Section status=ForTrade userItems=forTradeList editable />;
     } else {
       React.null;
     }}
    {if (hasCanCraft) {
       <Section status=CanCraft userItems=canCraftList editable />;
     } else {
       React.null;
     }}
    {if (hasWishlist) {
       <Section status=Wishlist userItems=wishlist editable />;
     } else {
       React.null;
     }}
  </div>;
};