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
      marginRight(px(6)),
      outlineStyle(none),
      padding3(~top=px(5), ~bottom=px(3), ~h=px(4)),
      transition(~duration=200, "all"),
      cursor(pointer),
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
        boxShadow(Shadow.box(~spread=px(6), hex("88c9a180"))),
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
  let mainImage =
    style([
      display(block),
      height(px(128)),
      width(px(128)),
      marginBottom(px(8)),
    ]);
  let variations =
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
  let bottomBar = style([alignSelf(flexStart), fontSize(px(12))]);
  let bottomBarStatus = style([paddingTop(px(8))]);
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

module RecipeIcon = {
  module Styles = {
    open Css;
    let recipeIcon =
      style([
        display(block),
        width(px(24)),
        height(px(24)),
        marginRight(px(8)),
      ]);
    let recipeLayer =
      style([
        backgroundColor(hex("404040f8")),
        color(Colors.white),
        borderRadius(px(8)),
        padding2(~v=px(12), ~h=px(16)),
      ]);
  };

  module RecipeLayer = {
    [@react.component]
    let make = (~recipe) => {
      <div className=Styles.recipeLayer>
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
    let (showRecipe, setShowRecipe) = React.useState(() => false);
    let iconRef = React.useRef(Js.Nullable.null);
    <>
      <img
        src=recipeIcon
        className=Styles.recipeIcon
        onMouseEnter={_ => setShowRecipe(_ => true)}
        onMouseLeave={_ => setShowRecipe(_ => false)}
        ref={ReactDOMRe.Ref.domRef(iconRef)}
      />
      {showRecipe
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
  module Styles = {
    open Css;
    let orderableIcon =
      style([
        display(block),
        width(px(24)),
        height(px(24)),
        marginRight(px(8)),
      ]);
  };

  [@bs.module] external orderableIcon: string = "./assets/shop_icon.png";

  [@react.component]
  let make = () => {
    <img
      src=orderableIcon
      title="Orderable from Catalog"
      className=Styles.orderableIcon
    />;
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
      | Want => "Add to Wishlist"
      | ForTrade => "Add to For Trade list"
      }
    }>
    {React.string(
       switch (status) {
       | Want => {j|+ Wishlist|j}
       | ForTrade => {j|+ For Trade|j}
       },
     )}
  </button>;
};

[@react.component]
let make = (~item: Item.t, ~showLogin) => {
  let (variation, setVariation) = React.useState(() => 0);
  let userItem = UserStore.useItem(~itemId=item.id, ~variation);

  <div
    className={Cn.make([
      Styles.card,
      Cn.ifSome(Styles.cardSelected, userItem),
    ])}>
    <div className=Styles.body>
      <div className=Styles.name> {React.string(item.name)} </div>
      <img
        src={
          Constants.cdnUrl
          ++ "/items/"
          ++ item.image
          ++ "__"
          ++ string_of_int(variation)
          ++ ".png"
        }
        className=Styles.mainImage
      />
      {switch (item.numVariations) {
       | Some(numVariations) =>
         <div className=Styles.variations>
           {let children = [||];
            for (v in 0 to numVariations - 1) {
              children
              |> Js.Array.push(
                   <div
                     onClick={_ => {setVariation(_ => v)}}
                     key={string_of_int(v)}>
                     <img
                       src={
                         Constants.cdnUrl
                         ++ "/items/"
                         ++ item.image
                         ++ "__"
                         ++ string_of_int(v)
                         ++ ".png"
                       }
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
       | None => React.null
       }}
      <div className=Styles.metaIcons>
        {switch (item.recipe) {
         | Some(recipe) => <RecipeIcon recipe />
         | None => React.null
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
                | Want => {j|🙏 In your Wishlist|j}
                | ForTrade => {j|✅ In your For Trade list|j}
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
            ~status=Want,
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
       </div>
     }}
  </div>;
};