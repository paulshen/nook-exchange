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
        selector("& ." ++ ItemImage.Styles.variantButton, [opacity(0.5)]),
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
      width(pct(100.)),
    ]);
  let name =
    style([
      fontSize(px(20)),
      marginBottom(px(8)),
      padding2(~v=zero, ~h=px(16)),
      textAlign(center),
    ]);
  let itemImage =
    style([unsafe("alignSelf", "stretch"), marginBottom(px(8))]);
  let variation =
    style([
      display(flexBox),
      flexWrap(wrap),
      justifyContent(center),
      marginBottom(px(8)),
    ]);
  let variationBatch = style([borderRadius(px(4)), overflow(hidden)]);
  let variationImage =
    style([
      display(block),
      cursor(pointer),
      width(px(32)),
      height(px(32)),
      borderRadius(px(4)),
      hover([backgroundColor(hex("00000010"))]),
    ]);
  let variationImageBatch =
    style([backgroundColor(hex("3aa56320")), borderRadius(zero)]);
  let variationImageSelected = style([backgroundColor(hex("3aa56320"))]);
  let metaIcons = style([position(absolute), top(px(8)), left(px(6))]);
  let topRightIcons =
    style([position(absolute), top(px(10)), right(px(10))]);
  let bottomBar = style([fontSize(px(12))]);
  let bottomBarStatus = style([alignSelf(flexStart), paddingTop(px(4))]);
  let statusButtons = style([display(flexBox), alignItems(center)]);
  let batchIndicator =
    style([
      backgroundColor(Colors.green),
      color(Colors.white),
      fontSize(px(12)),
      padding3(~top=px(5), ~bottom=px(3), ~h=px(4)),
      borderRadius(px(4)),
      textTransform(uppercase),
      marginRight(px(4)),
    ]);
  let statusButtonSelected =
    style([backgroundColor(Colors.green), color(Colors.white)]);
  let removeButton =
    style([
      position(absolute),
      bottom(px(10)),
      right(px(10)),
      selector("." ++ card ++ ":hover &", [opacity(0.8)]),
    ]);
  let wishlistEllipsisButton =
    style([
      position(absolute),
      bottom(px(8)),
      right(px(8)),
      selector("." ++ card ++ ":hover &", [opacity(0.8)]),
    ]);
};

module MetaIconStyles = {
  open Css;
  let icon = style([display(block), width(px(24)), height(px(24))]);
  let iconClickable = style([cursor(pointer)]);
  let layer = style([padding2(~v=px(4), ~h=px(4))]);
  let clickNote =
    style([
      borderTop(px(1), solid, hex("ffffff40")),
      marginTop(px(6)),
      marginLeft(px(-8)),
      marginRight(px(-8)),
      marginBottom(px(-4)),
      paddingTop(px(6)),
      paddingLeft(px(8)),
      paddingRight(px(8)),
      paddingBottom(px(4)),
    ]);
};

module RecipeIcon = {
  module RecipeIconStyles = {
    open Css;
    let tooltip =
      style([
        backgroundColor(Colors.darkLayerBackground),
        borderRadius(px(4)),
        color(Colors.white),
        fontSize(px(14)),
        padding3(~top=px(9), ~bottom=px(7), ~h=px(14)),
        position(relative),
        whiteSpace(`preLine),
        Colors.darkLayerShadow,
      ]);
  };

  module RecipeLayer = {
    [@react.component]
    let make = (~recipe, ~isRecipe, ~onClickAlternate) => {
      <div
        className={Cn.make([MetaIconStyles.layer, RecipeIconStyles.tooltip])}>
        {recipe
         ->Array.map(((itemId, quantity)) =>
             <div key=itemId>
               {React.string(
                  Item.getMaterialName(itemId)
                  ++ " x "
                  ++ string_of_int(quantity),
                )}
             </div>
           )
         ->React.array}
        {switch (isRecipe, onClickAlternate) {
         | (Some(isRecipe), Some(onClickAlternate)) =>
           <div
             onClick={_ => onClickAlternate()}
             onTouchStart={_ => onClickAlternate()}
             className=MetaIconStyles.clickNote>
             {React.string("Click to see " ++ (isRecipe ? "item" : "DIY"))}
           </div>
         | _ => React.null
         }}
      </div>;
    };
  };

  [@bs.module "./assets/recipe_icon.png"]
  external recipeIcon: string = "default";

  [@bs.get]
  external mediaQueryListMatches: Webapi.Dom.Window.mediaQueryList => bool =
    "matches";
  let browserSupportsHover = {
    Webapi.Dom.(window |> Window.matchMedia("(hover: hover)"))
    ->mediaQueryListMatches;
  };

  [@react.component]
  let make = (~recipe: Item.recipe, ~isRecipe=?, ~onClick=?, ()) => {
    let (showLayer, setShowLayer) = React.useState(() => false);
    let iconRef = React.useRef(Js.Nullable.null);
    let isMountedRef = React.useRef(true);
    React.useEffect0(() => {
      Some(() => React.Ref.setCurrent(isMountedRef, false))
    });
    <>
      <img
        src=recipeIcon
        className={Cn.make([
          MetaIconStyles.icon,
          Cn.ifTrue(MetaIconStyles.iconClickable, onClick !== None),
        ])}
        onMouseEnter={_ => {
          Js.Global.setTimeout(() => setShowLayer(_ => true), 10) |> ignore
        }}
        onMouseLeave={_ => {
          Js.Global.setTimeout(
            () =>
              if (React.Ref.current(isMountedRef)) {
                setShowLayer(_ => false);
              },
            10,
          )
          |> ignore
        }}
        onClick=?{
          Belt.Option.map(onClick, (onClick, e) =>
            if (browserSupportsHover) {
              onClick();
            }
          )
        }
        ref={ReactDOMRe.Ref.domRef(iconRef)}
      />
      {showLayer
         ? <ReactAtmosphere.PopperLayer
             reference=iconRef
             render={_ =>
               <RecipeLayer recipe isRecipe onClickAlternate=onClick />
             }
             options={Obj.magic({
               "placement": "bottom-start",
               "modifiers":
                 Some([|
                   {
                     "name": "offset",
                     "options": {
                       "offset": [|0, 2|],
                     },
                   },
                 |]),
             })}
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
      ~useBatchMode,
      ~numVariations,
      (),
    ) => {
  let userItemStatus = Option.map(userItem, userItem => userItem.status);
  <button
    onClick={_ =>
      if (UserStore.isLoggedIn()) {
        if (useBatchMode && numVariations > 1) {
          let item = Item.getItem(~itemId);
          UserStore.setItemStatusBatch(
            ~itemId,
            ~variations=Item.getCollapsedVariants(~item),
            ~status,
          );
        } else {
          UserStore.setItemStatus(~itemId, ~variation, ~status);
        };
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
      | CatalogOnly => raise(Constants.Uhoh)
      }
    }>
    {React.string(
       switch (status) {
       | Wishlist => "+ Wishlist"
       | ForTrade => "+ For Trade"
       | CanCraft => "+ Can Craft"
       | CatalogOnly => raise(Constants.Uhoh)
       },
     )}
  </button>;
};

[@react.component]
let make = (~item: Item.t, ~showCatalogCheckbox, ~showLogin) => {
  let (showRecipeAlternate, setShowRecipeAlternate) =
    React.useState(() => false);
  let item =
    if (showRecipeAlternate) {
      if (item.isRecipe) {
        Item.getItem(
          ~itemId=
            Item.getItemIdForRecipeId(~recipeId=item.id)->Belt.Option.getExn,
        );
      } else {
        Item.getItem(~itemId=Item.getRecipeIdForItemId(~itemId=item.id));
      };
    } else {
      item;
    };

  let (variation, setVariation) = React.useState(() => 0);
  let (useBatchMode, setUseBatchMode) = React.useState(() => false);
  let numVariations = Item.getNumVariations(~item);
  let numVariationsRef = React.useRef(numVariations);
  React.useEffect1(
    () => {
      React.Ref.setCurrent(numVariationsRef, numVariations);
      None;
    },
    [|numVariations|],
  );
  if (variation > numVariations) {
    setVariation(_ => 0);
  };
  let variation = Js.Math.min_int(variation, numVariations - 1);
  let userItem = UserStore.useItem(~itemId=item.id, ~variation);

  if (useBatchMode
      && (
        numVariations === 1
        || (
          switch (userItem->Belt.Option.map(userItem => userItem.status)) {
          | Some(ForTrade)
          | Some(CanCraft)
          | Some(Wishlist) => true
          | Some(CatalogOnly)
          | None => false
          }
        )
      )) {
    setUseBatchMode(_ => false);
  };

  React.useEffect0(() => {
    open Webapi.Dom;
    let onKeyDown = e =>
      if (KeyboardEvent.key(e) == "Shift") {
        (); //   setUseBatchMode(_ => true);
 // if (React.Ref.current(numVariationsRef) > 1) {
          // };
      };
    let onKeyUp = e =>
      if (KeyboardEvent.key(e) == "Shift") {
        setUseBatchMode(_ => false);
      };
    window |> Window.addKeyDownEventListener(onKeyDown);
    window |> Window.addKeyUpEventListener(onKeyUp);
    Some(
      () => {
        window |> Window.removeKeyDownEventListener(onKeyDown);
        window |> Window.removeKeyUpEventListener(onKeyUp);
      },
    );
  });

  <div
    className={Cn.make([
      Styles.card,
      Cn.ifSome(Styles.cardSelected, userItem),
    ])}>
    <div className=Styles.body>
      <div className=Styles.name> {React.string(Item.getName(item))} </div>
      <ItemImage
        item
        variant=variation
        className=Styles.itemImage
        key={item.id}
      />
      {let collapsedVariants = Item.getCollapsedVariants(~item);
       if (Js.Array.length(collapsedVariants) === 1) {
         React.null;
       } else {
         <div
           className={Cn.make([
             Styles.variation,
             Cn.ifTrue(Styles.variationBatch, useBatchMode),
           ])}>
           {collapsedVariants
            ->Belt.Array.map(v => {
                let image =
                  <img
                    src={Item.getImageUrl(~item, ~variant=v)}
                    className={Cn.make([
                      Styles.variationImage,
                      Cn.ifTrue(Styles.variationImageBatch, useBatchMode),
                      Cn.ifTrue(
                        Styles.variationImageSelected,
                        v == variation,
                      ),
                    ])}
                  />;
                switch (
                  Item.getVariantName(
                    ~item,
                    ~variant=v,
                    ~hidePattern=true,
                    (),
                  )
                ) {
                | Some(variantName) =>
                  <ReactAtmosphere.Tooltip
                    text={React.string(variantName)} key={string_of_int(v)}>
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
                };
              })
            ->React.array}
         </div>;
       }}
      <div className=Styles.metaIcons>
        {switch (item.recipe) {
         | Some(recipe) =>
           <RecipeIcon
             recipe
             isRecipe={item.isRecipe}
             onClick={_ => {setShowRecipeAlternate(show => !show)}}
             key={item.id}
           />
         | None => React.null
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
                 | Some(CatalogOnly) => "In your catalog"
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
                        | Some(Wishlist) => Some(User.CatalogOnly)
                        | Some(CanCraft)
                        | Some(ForTrade)
                        | Some(CatalogOnly) => None
                        };
                      switch (status) {
                      | Some(status) =>
                        let updateItem = () =>
                          if (useBatchMode && numVariations > 1) {
                            UserStore.setItemStatusBatch(
                              ~itemId=item.id,
                              ~variations=Item.getCollapsedVariants(~item),
                              ~status,
                            );
                          } else {
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
                        if (Belt.Option.getExn(userItem).status == CatalogOnly) {
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
    {let userItemStatus = userItem->Option.map(userItem => userItem.status);
     switch (userItem, userItemStatus) {
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
         {userItemStatus == Some(Wishlist)
            ? <WishlistEllipsisButton
                item
                variation
                className=Styles.wishlistEllipsisButton
              />
            : <RemoveButton
                className=Styles.removeButton
                onClick={_ => {
                  let status =
                    switch (showCatalogCheckbox, userItem.status) {
                    | (_, CatalogOnly) => raise(Constants.Uhoh)
                    | (false, _) => None
                    | (true, CanCraft)
                    | (true, ForTrade) => Some(User.CatalogOnly)
                    | (true, Wishlist) => None
                    };
                  switch (status) {
                  | Some(status) =>
                    UserStore.setItemStatus(
                      ~itemId=item.id,
                      ~variation,
                      ~status,
                    )
                  | None => UserStore.removeItem(~itemId=item.id, ~variation)
                  };
                }}
              />}
       </>
     | _ =>
       <div className={Cn.make([Styles.bottomBar, Styles.statusButtons])}>
         {useBatchMode && numVariations > 1
            ? <div className=Styles.batchIndicator>
                {React.string("All")}
              </div>
            : React.null}
         {renderStatusButton(
            ~itemId=item.id,
            ~variation,
            ~status=Wishlist,
            ~userItem,
            ~showLogin,
            ~useBatchMode,
            ~numVariations,
            (),
          )}
         {renderStatusButton(
            ~itemId=item.id,
            ~variation,
            ~status=ForTrade,
            ~userItem,
            ~showLogin,
            ~useBatchMode,
            ~numVariations,
            (),
          )}
         {!item.isRecipe && item.recipe !== None
            ? renderStatusButton(
                ~itemId=item.id,
                ~variation,
                ~status=CanCraft,
                ~userItem,
                ~showLogin,
                ~useBatchMode,
                ~numVariations,
                (),
              )
            : React.null}
       </div>
     }}
  </div>;
};