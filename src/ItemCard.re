open Belt;

module Styles = {
  open Css;
  let statusButton =
    style([
      backgroundColor(transparent),
      color(hex("3aa563c0")),
      borderWidth(zero),
      borderRadius(px(4)),
      fontSize(px(12)),
      marginRight(px(8)),
      outlineStyle(none),
      padding3(~top=px(5), ~bottom=px(3), ~h=px(4)),
      transition(~duration=200, "all"),
      cursor(pointer),
      lastChild([marginRight(zero)]),
      hover([
        important(backgroundColor(Colors.green)),
        important(color(Colors.white)),
      ]),
      media("(max-width: 430px)", [fontSize(px(14))]),
    ]);
  let card =
    style([
      backgroundColor(hex("fffffff0")),
      borderRadius(px(8)),
      display(flexBox),
      flexDirection(column),
      alignItems(center),
      marginRight(px(32)),
      marginBottom(px(32)),
      padding3(~top=px(24), ~bottom=px(8), ~h=px(8)),
      position(relative),
      width(px(240)),
      transition(~duration=200, "all"),
      hover([
        selector(
          "& ." ++ statusButton,
          [backgroundColor(hex("3aa56320")), color(Colors.green)],
        ),
      ]),
      media(
        "(max-width: 600px)",
        [marginRight(px(16)), marginBottom(px(16)), width(px(160))],
      ),
      media("(max-width: 430px)", [width(pct(100.))]),
    ]);
  let cardSelected = style([backgroundColor(hex("ffffff"))]);
  let body =
    style([
      flexGrow(1.),
      display(flexBox),
      flexDirection(column),
      alignItems(center),
      marginBottom(px(16)),
    ]);
  let name =
    style([
      fontSize(px(20)),
      marginBottom(px(8)),
      padding2(~v=zero, ~h=px(16)),
      textAlign(center),
    ]);
  let mainImageWrapper = style([marginBottom(px(8)), position(relative)]);
  let mainImage =
    style([display(block), height(px(128)), width(px(128))]);
  let recipeIcon =
    style([
      display(block),
      height(px(64)),
      width(px(64)),
      position(absolute),
      right(px(-16)),
      bottom(px(-16)),
      opacity(0.95),
      transition(~duration=200, "all"),
      hover([opacity(1.)]),
    ]);
  let variation =
    style([display(flexBox), flexWrap(wrap), justifyContent(center)]);
  let variationImage =
    style([
      display(block),
      cursor(pointer),
      width(px(32)),
      height(px(32)),
      borderRadius(px(4)),
      hover([backgroundColor(hex("00000010"))]),
    ]);
  let variationImageSelected = style([backgroundColor(hex("3aa56320"))]);
  let metaIcons = style([position(absolute), top(px(8)), left(px(6))]);
  let bottomBar = style([fontSize(px(12))]);
  let bottomBarStatus = style([alignSelf(flexStart), paddingTop(px(8))]);
  let statusButtons = style([]);
  let statusButtonSelected =
    style([backgroundColor(Colors.green), color(Colors.white)]);
  let removeButton =
    style([
      position(absolute),
      bottom(px(6)),
      right(px(2)),
      backgroundColor(transparent),
      borderWidth(zero),
      fontSize(px(12)),
      opacity(0.5),
      transition(~duration=200, "all"),
      cursor(pointer),
      hover([opacity(1.)]),
    ]);
};

module MetaIconStyles = {
  open Css;
  let icon =
    style([
      display(block),
      width(px(24)),
      height(px(24)),
      marginRight(px(8)),
    ]);
  let layer =
    style([
      backgroundColor(hex("404040f8")),
      color(Colors.white),
      borderRadius(px(8)),
      padding2(~v=px(12), ~h=px(16)),
    ]);
};

module RecipeIcon = {
  module RecipeLayer = {
    [@react.component]
    let make = (~recipe) => {
      <div className=MetaIconStyles.layer>
        {recipe
         ->Array.map(((itemId, quantity)) =>
             <div key=itemId>
               {React.string(itemId ++ " x " ++ string_of_int(quantity))}
             </div>
           )
         ->React.array}
      </div>;
    };
  };

  [@bs.module] external recipeIcon: string = "./assets/recipe_icon.png";

  [@react.component]
  let make = (~recipe: Item.recipe) => {
    let (showLayer, setShowLayer) = React.useState(() => false);
    let iconRef = React.useRef(Js.Nullable.null);
    <>
      <img
        src=recipeIcon
        className=MetaIconStyles.icon
        onMouseEnter={_ => setShowLayer(_ => true)}
        onMouseLeave={_ => setShowLayer(_ => false)}
        ref={ReactDOMRe.Ref.domRef(iconRef)}
      />
      {showLayer
         ? <ReactAtmosphere.PopperLayer
             reference=iconRef
             render={_ => <RecipeLayer recipe />}
             options={
               placement: Some("bottom-start"),
               modifiers:
                 Some([|
                   {
                     "name": "offset",
                     "options": {
                       "offset": [|0, 4|],
                     },
                   },
                 |]),
             }
           />
         : React.null}
    </>;
  };
};

module OrderableIcon = {
  module OrderableLayer = {
    [@react.component]
    let make = () => {
      <div className=MetaIconStyles.layer>
        <div> {React.string("Orderable from catalog")} </div>
        <div> {React.string("Touch tradeable")} </div>
      </div>;
    };
  };

  [@bs.module] external orderableIcon: string = "./assets/shop_icon.png";

  [@react.component]
  let make = () => {
    let (showLayer, setShowLayer) = React.useState(() => false);
    let iconRef = React.useRef(Js.Nullable.null);
    <>
      <img
        src=orderableIcon
        className=MetaIconStyles.icon
        onMouseEnter={_ => setShowLayer(_ => true)}
        onMouseLeave={_ => setShowLayer(_ => false)}
        ref={ReactDOMRe.Ref.domRef(iconRef)}
      />
      {showLayer
         ? <ReactAtmosphere.PopperLayer
             reference=iconRef
             render={_ => <OrderableLayer />}
             options={
               placement: Some("bottom-start"),
               modifiers:
                 Some([|
                   {
                     "name": "offset",
                     "options": {
                       "offset": [|0, 4|],
                     },
                   },
                 |]),
             }
           />
         : React.null}
    </>;
  };
};

let renderStatusButton =
    (
      ~itemId,
      ~variation,
      ~status,
      ~userItem: option(User.item),
      ~showLogin=?,
      (),
    ) => {
  let userItemStatus = Option.map(userItem, userItem => userItem.status);
  <button
    onClick={_ =>
      if (UserStore.isLoggedIn()) {
        UserStore.setItem(
          ~itemId,
          ~variation,
          ~item={
            status,
            note:
              switch (userItem) {
              | Some(userItem) => userItem.note
              | None => ""
              },
          },
        );
      } else {
        Option.map(showLogin, showLogin => showLogin()) |> ignore;
      }
    }
    className={Cn.make([
      Styles.statusButton,
      Cn.ifTrue(Styles.statusButtonSelected, userItemStatus == Some(status)),
    ])}
    title={
      switch (status) {
      | Wishlist => "Add to Wishlist"
      | ForTrade => "Add to For Trade list"
      | CanCraft => "Add to Can Craft list"
      }
    }>
    {React.string(
       switch (status) {
       | Wishlist => {j|+ Wishlist|j}
       | ForTrade => {j|+ For Trade|j}
       | CanCraft => {j|+ Can Craft|j}
       },
     )}
  </button>;
};

[@react.component]
let make = (~item: Item.t, ~showLogin) => {
  let (variation, setVariation) = React.useState(() => 0);
  let userItem = UserStore.useItem(~itemId=item.id, ~variation);

  let numVariations = Item.getNumVariations(~item);
  <div
    className={Cn.make([
      Styles.card,
      Cn.ifSome(Styles.cardSelected, userItem),
    ])}>
    <div className=Styles.body>
      <div className=Styles.name> {React.string(item.name)} </div>
      <div className=Styles.mainImageWrapper>
        <img
          src={Item.getImageUrl(~item, ~variant=variation)}
          className=Styles.mainImage
        />
        {item.isRecipe
           ? <img
               src={Constants.imageUrl ++ "/DIYRecipe.png"}
               className=Styles.recipeIcon
             />
           : React.null}
      </div>
      {switch (numVariations) {
       | 1 => React.null
       | numVariations =>
         <div className=Styles.variation>
           {let children = [||];
            for (v in 0 to numVariations - 1) {
              children
              |> Js.Array.push(
                   <div
                     onClick={_ => {setVariation(_ => v)}}
                     key={string_of_int(v)}>
                     <img
                       src={Item.getImageUrl(~item, ~variant=v)}
                       className={Cn.make([
                         Styles.variationImage,
                         Cn.ifTrue(
                           Styles.variationImageSelected,
                           v == variation,
                         ),
                       ])}
                     />
                   </div>,
                 )
              |> ignore;
            };
            children->React.array}
         </div>
       }}
      <div className=Styles.metaIcons>
        {switch (item.isRecipe, item.recipe) {
         | (false, Some(recipe)) => <RecipeIcon recipe />
         | _ => React.null
         }}
        {if (item.orderable) {
           <OrderableIcon />;
         } else {
           React.null;
         }}
      </div>
    </div>
    {switch (userItem) {
     | Some(userItem) =>
       <>
         <UserItemNote itemId={item.id} variation userItem />
         <div className={Cn.make([Styles.bottomBar, Styles.bottomBarStatus])}>
           {React.string(
              {
                switch (userItem.status) {
                | Wishlist => {j|🙏 In your Wishlist|j}
                | ForTrade => {j|✅ In your For Trade list|j}
                | CanCraft => {j|🔨 In your Can Craft list|j}
                };
              },
            )}
         </div>
         <button
           className=Styles.removeButton
           title="Remove"
           onClick={_ => {UserStore.removeItem(~itemId=item.id, ~variation)}}>
           {React.string({j|❌|j})}
         </button>
       </>
     | None =>
       <div className={Cn.make([Styles.bottomBar, Styles.statusButtons])}>
         {renderStatusButton(
            ~itemId=item.id,
            ~variation,
            ~status=Wishlist,
            ~userItem,
            ~showLogin,
            (),
          )}
         {renderStatusButton(
            ~itemId=item.id,
            ~variation,
            ~status=ForTrade,
            ~userItem,
            ~showLogin,
            (),
          )}
         {renderStatusButton(
            ~itemId=item.id,
            ~variation,
            ~status=CanCraft,
            ~userItem,
            ~showLogin,
            (),
          )}
       </div>
     }}
  </div>;
};