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
  let catalogCheckbox =
    style([
      backgroundColor(Colors.white),
      height(px(20)),
      width(px(20)),
      borderRadius(px(3)),
      border(px(2), solid, hex("c0c0c0")),
      padding(zero),
      transition(~duration=200, "all"),
      margin(zero),
      cursor(pointer),
      outlineStyle(none),
      opacity(0.),
      transition(~duration=200, "all"),
      media("(hover: none)", [opacity(0.5)]),
      hover([borderColor(hex("808080"))]),
    ]);
  [@bs.module "./assets/check.png"] external checkImage: string = "default";
  let catalogCheckboxChecked =
    style([
      borderColor(Colors.white),
      backgroundImage(url(checkImage)),
      backgroundSize(size(px(16), px(16))),
      backgroundRepeat(noRepeat),
      backgroundPosition(center),
      opacity(0.5),
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
      boxSizing(borderBox),
      width(px(256)),
      transition(~duration=200, "all"),
      hover([
        selector(
          "& ." ++ statusButton,
          [backgroundColor(hex("3aa56320")), color(Colors.green)],
        ),
        selector(
          "& ." ++ catalogCheckbox,
          [
            opacity(1.),
            media("(hover: none)", [borderColor(hex("c0c0c0"))]),
          ],
        ),
      ]),
      media(
        "(max-width: 600px)",
        [
          marginRight(px(16)),
          marginBottom(px(16)),
          width(Calc.(pct(50.) - px(16))),
        ],
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
    style([
      display(flexBox),
      flexWrap(wrap),
      justifyContent(center),
      marginBottom(px(8)),
    ]);
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
  let topRightIcons =
    style([position(absolute), top(px(10)), right(px(10))]);
  let bottomBar = style([fontSize(px(12))]);
  let bottomBarStatus = style([alignSelf(flexStart), paddingTop(px(4))]);
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
  let layer = style([padding2(~v=px(4), ~h=px(4))]);
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

  [@bs.module "./assets/recipe_icon.png"]
  external recipeIcon: string = "default";

  [@react.component]
  let make = (~recipe: Item.recipe) => {
    <ReactAtmosphere.Tooltip
      text={<RecipeLayer recipe />}
      options={Obj.magic({"placement": "bottom-start"})}>
      {({onMouseEnter, onMouseLeave, ref}) =>
         <img
           src=recipeIcon
           className=MetaIconStyles.icon
           onMouseEnter
           onMouseLeave
           ref={ReactDOMRe.Ref.domRef(ref)}
         />}
    </ReactAtmosphere.Tooltip>;
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

  [@bs.module "./assets/shop_icon.png"]
  external orderableIcon: string = "default";

  [@react.component]
  let make = () => {
    <ReactAtmosphere.Tooltip
      text={<OrderableLayer />}
      options={Obj.magic({"placement": "bottom-start"})}>
      {({onMouseEnter, onMouseLeave, ref}) =>
         <img
           src=orderableIcon
           className=MetaIconStyles.icon
           onMouseEnter
           onMouseLeave
           ref={ReactDOMRe.Ref.domRef(ref)}
         />}
    </ReactAtmosphere.Tooltip>;
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
        UserStore.setItemStatus(~itemId, ~variation, ~status);
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
      | InCatalog => raise(Constants.Uhoh)
      }
    }>
    {React.string(
       switch (status) {
       | Wishlist => {j|+ Wishlist|j}
       | ForTrade => {j|+ For Trade|j}
       | CanCraft => {j|+ Can Craft|j}
       | InCatalog => raise(Constants.Uhoh)
       },
     )}
  </button>;
};

[@react.component]
let make = (~item: Item.t, ~showCatalogCheckbox, ~showLogin) => {
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
               src={Constants.cdnUrl ++ "/images/DIYRecipe.png"}
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
              let image =
                <img
                  src={Item.getImageUrl(~item, ~variant=v)}
                  className={Cn.make([
                    Styles.variationImage,
                    Cn.ifTrue(Styles.variationImageSelected, v == variation),
                  ])}
                />;
              children
              |> Js.Array.push(
                   switch (Item.getVariantName(~item, ~variant=v)) {
                   | Some(variantName) =>
                     <ReactAtmosphere.Tooltip
                       text={React.string(variantName)}
                       key={string_of_int(v)}>
                       {(
                          ({onMouseEnter, onMouseLeave, onFocus, onBlur, ref}) =>
                            <div
                              onClick={_ => {setVariation(_ => v)}}
                              onMouseEnter
                              onMouseLeave
                              onFocus
                              onBlur
                              ref={ReactDOMRe.Ref.domRef(ref)}>
                              image
                            </div>
                        )}
                     </ReactAtmosphere.Tooltip>
                   | None =>
                     <div
                       onClick={_ => {setVariation(_ => v)}}
                       key={string_of_int(v)}>
                       image
                     </div>
                   },
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
      {showCatalogCheckbox
         ? <div className=Styles.topRightIcons>
             <ReactAtmosphere.Tooltip
               options={
                 placement: Some("top"),
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
               text={React.string(
                 switch (userItem->Option.map(userItem => userItem.status)) {
                 | None => "Not in catalog"
                 | Some(Wishlist) => "Move to catalog from Wishlist"
                 | Some(ForTrade)
                 | Some(CanCraft)
                 | Some(InCatalog) => "In your catalog"
                 },
               )}>
               {({onMouseEnter, onMouseLeave, ref}) =>
                  <button
                    className={Cn.make([
                      Styles.catalogCheckbox,
                      Cn.ifTrue(
                        Styles.catalogCheckboxChecked,
                        switch (userItem) {
                        | Some(userItem) => userItem.status !== Wishlist
                        | None => false
                        },
                      ),
                    ])}
                    onClick={e => {
                      let status =
                        switch (
                          Option.map(userItem, userItem => userItem.status)
                        ) {
                        | None
                        | Some(Wishlist) => Some(User.InCatalog)
                        | Some(CanCraft)
                        | Some(ForTrade)
                        | Some(InCatalog) => None
                        };
                      switch (status) {
                      | Some(status) =>
                        let updateItem = () => {
                          UserStore.setItemStatus(
                            ~itemId=item.id,
                            ~variation,
                            ~status,
                          );
                        };
                        if (Option.map(userItem, userItem => userItem.status)
                            == Some(Wishlist)) {
                          WishlistToCatalog.confirm(~onConfirm=updateItem);
                        } else {
                          updateItem();
                        };
                      | None =>
                        if (Belt.Option.getExn(userItem).status == InCatalog) {
                          UserStore.removeItem(~itemId=item.id, ~variation);
                        } else {
                          DeleteFromCatalog.confirm(~onConfirm=() =>
                            UserStore.removeItem(~itemId=item.id, ~variation)
                          );
                        }
                      };
                    }}
                    onMouseEnter
                    onMouseLeave
                    ref={ReactDOMRe.Ref.domRef(ref)}
                  />}
             </ReactAtmosphere.Tooltip>
           </div>
         : React.null}
    </div>
    {switch (userItem, userItem->Option.map(userItem => userItem.status)) {
     | (Some(userItem), Some(Wishlist))
     | (Some(userItem), Some(ForTrade))
     | (Some(userItem), Some(CanCraft)) =>
       <>
         <UserItemNote
           itemId={item.id}
           variation
           userItem
           key={string_of_int(variation)}
         />
         <div className={Cn.make([Styles.bottomBar, Styles.bottomBarStatus])}>
           {React.string(
              {
                switch (userItem.status) {
                | Wishlist => {j|🙏 In your Wishlist|j}
                | ForTrade => {j|🤝 In your For Trade list|j}
                | CanCraft => {j|🔨 In your Can Craft list|j}
                | _ => raise(Constants.Uhoh)
                };
              },
            )}
         </div>
         <button
           className=Styles.removeButton
           title="Remove"
           onClick={_ => {
             let status =
               switch (showCatalogCheckbox, userItem.status) {
               | (_, InCatalog) => raise(Constants.Uhoh)
               | (false, _) => None
               | (true, CanCraft)
               | (true, ForTrade) => Some(User.InCatalog)
               | (true, Wishlist) => None
               };
             switch (status) {
             | Some(status) =>
               UserStore.setItemStatus(~itemId=item.id, ~variation, ~status)
             | None => UserStore.removeItem(~itemId=item.id, ~variation)
             };
           }}>
           {React.string({j|❌|j})}
         </button>
       </>
     | _ =>
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
         {!item.isRecipe && item.recipe !== None
            ? renderStatusButton(
                ~itemId=item.id,
                ~variation,
                ~status=CanCraft,
                ~userItem,
                ~showLogin,
                (),
              )
            : React.null}
       </div>
     }}
  </div>;
};