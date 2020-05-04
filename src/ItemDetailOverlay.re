let smallThreshold = 500;

module Div100VH = {
  [@bs.module "react-div-100vh"] [@react.component]
  external make:
    (~children: React.element, ~className: string) => React.element =
    "default";
};

module Styles = {
  open Css;
  let smallThresholdMediaQuery = styles =>
    media("(max-width: " ++ string_of_int(smallThreshold) ++ "px)", styles);
  let backdrop =
    style([
      position(absolute),
      top(zero),
      bottom(zero),
      left(zero),
      right(zero),
      opacity(0.),
      backgroundColor(hex("80808080")),
      transition(~duration=200, "all"),
    ]);
  let root100VH =
    style([
      display(flexBox),
      alignItems(center),
      justifyContent(center),
      width(Calc.(vw(100.) - px(16))),
    ]);
  let root =
    style([
      backgroundColor(hex("ffffff")),
      borderRadius(px(8)),
      position(relative),
      maxWidth(px(640)),
      boxSizing(borderBox),
      boxShadow(Shadow.box(~blur=px(32), rgba(0, 0, 0, 0.2))),
      overflow(auto),
      maxHeight(pct(100.)),
      minHeight(px(256)),
      opacity(0.),
      transforms([scale(0.85, 0.85), translate3d(zero, zero, zero)]),
      media("(max-width: 400px)", [maxHeight(pct(90.)), minWidth(zero)]),
      transition(
        ~duration=200,
        ~timingFunction=cubicBezier(0.48, 1.38, 0.71, 0.93),
        "all",
      ),
      smallThresholdMediaQuery([width(pct(100.))]),
    ]);
  let body =
    style([
      display(flexBox),
      justifyContent(center),
      padding2(~v=px(32), ~h=px(32)),
      smallThresholdMediaQuery([justifyContent(flexStart)]),
    ]);
  let bodyContent =
    style([
      display(flexBox),
      flexDirection(column),
      smallThresholdMediaQuery([width(pct(100.))]),
    ]);
  let itemBodyTop = style([fontSize(px(16)), paddingRight(px(8))]);
  let itemImageUnit =
    style([
      width(px(128)),
      marginRight(px(48)),
      smallThresholdMediaQuery([
        display(flexBox),
        marginTop(px(16)),
        marginRight(zero),
        width(auto),
        justifyContent(center),
      ]),
    ]);
  let itemImageUnitWithRecipe =
    style([
      smallThresholdMediaQuery([
        display(flexBox),
        marginTop(px(16)),
        marginRight(zero),
        width(auto),
        justifyContent(spaceBetween),
      ]),
    ]);
  let itemImage =
    style([display(block), width(px(128)), height(px(128))]);
  let itemImageUnitBody =
    style([display(flexBox), flexDirection(column), alignItems(center)]);
  let itemImageLabel =
    style([fontSize(px(16)), textAlign(center), marginTop(px(8))]);
  let itemRecipe =
    style([
      marginTop(px(16)),
      smallThresholdMediaQuery([width(pct(50.)), marginTop(zero)]),
    ]);
  let itemRecipeTitle = style([marginBottom(px(2))]);
  [@bs.module "./assets/recipe_icon.png"]
  external recipeIcon: string = "default";
  let recipeIcon =
    style([
      display(inlineBlock),
      backgroundImage(url(recipeIcon)),
      width(px(16)),
      height(px(16)),
      backgroundSize(cover),
      marginLeft(px(4)),
      verticalAlign(`top),
    ]);
  let recipeRow =
    style([
      color(Colors.gray),
      display(flexBox),
      borderTop(px(1), dashed, Colors.faintGray),
      padding2(~v=px(2), ~h=zero),
    ]);
  let recipeRowMaterial = style([flexGrow(1.)]);
  let recipeRowQuantity =
    style([width(px(24)), color(Colors.lightGray), textAlign(`right)]);
  let itemName =
    style([
      fontSize(px(24)),
      marginBottom(px(8)),
      paddingRight(px(16)),
      smallThresholdMediaQuery([paddingRight(px(8))]),
    ]);
  let itemCategory = style([marginBottom(px(4))]);
  let itemCategoryLink =
    style([
      color(Colors.gray),
      textDecoration(none),
      hover([textDecoration(underline)]),
    ]);
  let itemSourceFrom = style([color(Colors.lightGray)]);
  let itemTags = style([marginBottom(px(4))]);
  [@bs.module "./assets/tag.png"] external tagIcon: string = "default";
  let itemTagIcon =
    style([
      display(inlineBlock),
      backgroundImage(url(tagIcon)),
      width(px(16)),
      height(px(16)),
      backgroundSize(cover),
      marginRight(px(6)),
      verticalAlign(`top),
      position(relative),
      top(px(1)),
    ]);
  let itemTag =
    style([
      color(Colors.gray),
      display(inlineBlock),
      textDecoration(none),
      hover([textDecoration(underline)]),
      marginRight(px(8)),
    ]);
  let itemPriceIcon =
    style([marginRight(px(6)), verticalAlign(`top), top(px(1))]);
  let itemPrices = style([display(flexBox), marginBottom(px(4))]);
  let itemPrice = style([marginRight(px(16)), whiteSpace(nowrap)]);
  let itemPriceLabel = style([color(Colors.gray), marginRight(px(4))]);
  let itemPriceValue = style([fontWeight(`num(700))]);
  let itemCustomizeCost = style([marginBottom(px(4))]);
  [@bs.module "./assets/remake.png"] external remakeIcon: string = "default";
  let itemRemakeIcon =
    style([
      display(inlineBlock),
      backgroundImage(url(remakeIcon)),
      width(px(16)),
      height(px(16)),
      backgroundSize(cover),
      marginRight(px(6)),
      verticalAlign(`top),
      position(relative),
      top(px(1)),
    ]);
  let variantSection =
    style([
      marginTop(px(24)),
      marginLeft(px(-16)),
      marginRight(px(-16)),
      paddingLeft(px(16)),
      paddingRight(px(32)),
      overflowX(auto),
      overflowY(hidden),
      smallThresholdMediaQuery([
        marginLeft(px(-32)),
        marginRight(px(-32)),
      ]),
    ]);
  let variantTable =
    style([display(flexBox), flexWrap(wrap), maxWidth(px(288))]);
  let variantRow =
    style([
      display(flexBox),
      alignItems(center),
      marginRight(px(8)),
      marginBottom(px(4)),
    ]);
  let variantRowLabel = style([alignItems(flexStart)]);
  let variantLabel =
    style([width(px(86)), paddingRight(px(6)), flexShrink(0.)]);
  let variantCell = style([fontSize(zero)]);
  let variantImage =
    style([
      borderRadius(px(4)),
      border(px(1), solid, transparent),
      display(inlineBlock),
      width(px(32)),
      height(px(32)),
      flexShrink(0.),
      cursor(pointer),
    ]);
  let variantImageCell =
    style([
      marginRight(px(4)),
      hover([borderColor(Colors.veryLightGray)]),
    ]);
  let variantImageWrap = style([marginRight(px(8))]);
  let variantImageSelected =
    style([important(borderColor(Colors.lightGray))]);
  let singleVariantUnit =
    style([
      borderRadius(px(4)),
      border(px(1), solid, transparent),
      cursor(pointer),
      hover([
        selector(
          "& ." ++ variantImage,
          [borderColor(Colors.veryLightGray)],
        ),
      ]),
    ]);
  let singleVariantUnitSelected =
    style([
      selector(
        "& ." ++ variantImage,
        [important(borderColor(Colors.lightGray))],
      ),
    ]);
  let patternCell = style([marginRight(px(4))]);
  let patternLabel =
    style([
      width(px(34)),
      transforms([translateX(px(22)), rotate(deg(45.))]),
      transformOrigin(zero, zero),
      whiteSpace(nowrap),
    ]);
  let transitionIn =
    style([
      selector("& ." ++ backdrop, [opacity(1.)]),
      selector(
        "& ." ++ root,
        [
          opacity(1.),
          transforms([scale(1., 1.), translate3d(zero, zero, zero)]),
        ],
      ),
    ]);
};

let getItemDetailUrl = (~itemId, ~variant) => {
  let url = ReasonReactRouter.dangerouslyGetInitialUrl();
  "/"
  ++ Js.Array.joinWith("/", Belt.List.toArray(url.path))
  ++ (
    switch (url.search) {
    | "" => ""
    | search => "?" ++ search
    }
  )
  ++ "#i"
  ++ string_of_int(itemId)
  ++ (
    switch (variant) {
    | Some(variant) => ":" ++ string_of_int(variant)
    | None => ""
    }
  );
};

module VariantWithLabel = {
  [@react.component]
  let make = (~item: Item.t, ~variant, ~selected) => {
    <div
      onClick={_ => {
        Webapi.Dom.(
          location->Location.setHash(
            "i" ++ string_of_int(item.id) ++ ":" ++ string_of_int(variant),
          )
        )
      }}
      className={Cn.make([
        Styles.variantRow,
        Styles.singleVariantUnit,
        Cn.ifTrue(Styles.singleVariantUnitSelected, selected),
      ])}>
      <img
        src={Item.getImageUrl(~item, ~variant)}
        className={Cn.make([Styles.variantImage, Styles.variantImageWrap])}
      />
      <div className=Styles.variantLabel>
        {switch (Item.getVariantName(~item, ~variant, ())) {
         | Some(bodyName) => React.string(bodyName)
         | None => React.null
         }}
      </div>
    </div>;
  };
};

module OneDimensionVariants = {
  [@react.component]
  let make = (~item, ~a, ~variant) => {
    <div className={Cn.make([Styles.variantSection, Styles.variantTable])}>
      {Belt.Array.make(a, None)
       ->Belt.Array.mapWithIndex((i, _) => {
           <VariantWithLabel
             item
             variant
             selected={i == variant}
             key={string_of_int(i)}
           />
         })
       ->React.array}
    </div>;
  };
};

module TwoDimensionVariants = {
  [@react.component]
  let make = (~item, ~a, ~b, ~variant) => {
    <div
      className={Cn.make([
        Styles.variantSection,
        Cn.ifTrue(Styles.variantTable, b == 1),
      ])}>
      {Belt.Array.make(a, None)
       ->Belt.Array.mapWithIndex((i, _) =>
           if (b > 1) {
             <div className=Styles.variantRow key={string_of_int(i)}>
               {a > 1
                  ? <div className=Styles.variantLabel>
                      {switch (
                         Item.getVariantName(
                           ~item,
                           ~variant={
                             i * b;
                           },
                           ~hidePattern=true,
                           (),
                         )
                       ) {
                       | Some(bodyName) => React.string(bodyName)
                       | None => React.null
                       }}
                    </div>
                  : React.null}
               {Belt.Array.make(b, None)
                ->Belt.Array.mapWithIndex((j, _) => {
                    let v = i * b + j;
                    <img
                      src={Item.getImageUrl(~item, ~variant=v)}
                      onClick={_ => {
                        Webapi.Dom.(
                          location->Location.setHash(
                            "i"
                            ++ string_of_int(item.id)
                            ++ ":"
                            ++ string_of_int(v),
                          )
                        )
                      }}
                      className={Cn.make([
                        Styles.variantImage,
                        Styles.variantImageCell,
                        Cn.ifTrue(Styles.variantImageSelected, variant == v),
                      ])}
                      key={string_of_int(j)}
                    />;
                  })
                ->React.array}
             </div>;
           } else {
             <VariantWithLabel
               item
               variant=i
               selected={i == variant}
               key={string_of_int(i)}
             />;
           }
         )
       ->React.array}
      {b > 1
         ? <div
             className={Cn.make([Styles.variantRow, Styles.variantRowLabel])}>
             {a > 1 ? <div className=Styles.variantLabel /> : React.null}
             {Belt.Array.make(b, None)
              ->Belt.Array.mapWithIndex((j, _) => {
                  <div className=Styles.patternCell key={string_of_int(j)}>
                    {switch (
                       Item.getVariantName(
                         ~item,
                         ~variant=j,
                         ~hideBody=true,
                         (),
                       )
                     ) {
                     | Some(patternName) =>
                       <div
                         className=Styles.patternLabel
                         style={ReactDOMRe.Style.make(
                           ~height=
                             string_of_int(Js.String.length(patternName) * 6)
                             ++ "px",
                           (),
                         )}>
                         {React.string(patternName)}
                       </div>
                     | None => React.null
                     }}
                  </div>
                })
              ->React.array}
           </div>
         : React.null}
    </div>;
  };
};

module ItemRecipe = {
  [@react.component]
  let make = (~recipe) => {
    <div className=Styles.itemRecipe>
      <div className=Styles.itemRecipeTitle>
        {React.string("DIY")}
        <span className=Styles.recipeIcon />
      </div>
      {recipe
       ->Belt.Array.map(((itemId, quantity)) =>
           <div className=Styles.recipeRow key=itemId>
             <div className=Styles.recipeRowMaterial>
               {React.string(Item.getMaterialName(itemId))}
             </div>
             <div className=Styles.recipeRowQuantity>
               {React.string({j|×|j} ++ string_of_int(quantity))}
             </div>
           </div>
         )
       ->React.array}
    </div>;
  };
};

[@react.component]
let make = (~item: Item.t, ~variant, ~isInitialLoad) => {
  let onClose = () => {
    let url = ReasonReactRouter.dangerouslyGetInitialUrl();
    ReasonReactRouter.push(
      "/"
      ++ Js.Array.joinWith("/", Belt.List.toArray(url.path))
      ++ (
        switch (url.search) {
        | "" => ""
        | search => "?" ++ search
        }
      ),
    );
  };
  let variant = variant->Belt.Option.getWithDefault(0);
  let (transitionIn, setTransitionIn) = React.useState(() => isInitialLoad);
  React.useEffect0(() => {
    Js.Global.setTimeout(() => {setTransitionIn(_ => true)}, 20) |> ignore;
    None;
  });

  let viewportWidth = Utils.useViewportWidth();
  let itemImage =
    <div
      className={Cn.make([
        Styles.itemImageUnit,
        Cn.ifTrue(Styles.itemImageUnitWithRecipe, item.recipe != None),
      ])}>
      <div className=Styles.itemImageUnitBody>
        <img
          src={Item.getImageUrl(~item, ~variant)}
          className=Styles.itemImage
        />
        {if (Item.getNumVariations(~item) > 1) {
           switch (Item.getVariantName(~item, ~variant, ())) {
           | Some(variantName) =>
             <div className=Styles.itemImageLabel>
               {React.string(variantName)}
             </div>
           | None => React.null
           };
         } else {
           React.null;
         }}
      </div>
      {switch (item.recipe) {
       | Some(recipe) => <ItemRecipe recipe />
       | None => React.null
       }}
    </div>;

  <div
    className={Cn.make([
      LoginOverlay.Styles.overlay,
      Cn.ifTrue(Styles.transitionIn, transitionIn),
    ])}>
    <div className=Styles.backdrop onClick={_ => onClose()} />
    <Div100VH className=Styles.root100VH>
      <div className=Styles.root>
        <div className=Styles.body>
          {viewportWidth > 500 ? itemImage : React.null}
          <div className=Styles.bodyContent>
            <div className=Styles.itemBodyTop>
              <div className=Styles.itemName>
                {React.string(Item.getName(item))}
              </div>
              <div className=Styles.itemCategory>
                <Link
                  path={"/?c=" ++ item.category}
                  className=Styles.itemCategoryLink>
                  {React.string(Utils.capitalizeFirstLetter(item.category))}
                </Link>
                {switch (item.source) {
                 | Some(itemSource) =>
                   <>
                     <span className=Styles.itemSourceFrom>
                       {React.string(" from ")}
                     </span>
                     {React.string(itemSource)}
                   </>
                 | None => React.null
                 }}
              </div>
              {if (item.tags->Js.Array.length > 0) {
                 <div className=Styles.itemTags>
                   <span className=Styles.itemTagIcon />
                   {item.tags
                    ->Belt.Array.map(tag =>
                        <Link
                          path={"/?q=" ++ tag}
                          className=Styles.itemTag
                          key=tag>
                          {React.string(Utils.capitalizeFirstLetter(tag))}
                        </Link>
                      )
                    ->React.array}
                 </div>;
               } else {
                 React.null;
               }}
              {if (item.buyPrice != None && item.sellPrice != None) {
                 <div className=Styles.itemPrices>
                   {switch (item.buyPrice) {
                    | Some(buyPrice) =>
                      <div className=Styles.itemPrice>
                        <span
                          className={Cn.make([
                            Emoji.Styles.bell,
                            Styles.itemPriceIcon,
                          ])}
                        />
                        <label className=Styles.itemPriceLabel>
                          {React.string("Buy")}
                        </label>
                        <span className=Styles.itemPriceValue>
                          {React.string(string_of_int(buyPrice))}
                        </span>
                      </div>
                    | None => React.null
                    }}
                   {switch (item.sellPrice) {
                    | Some(sellPrice) =>
                      <div className=Styles.itemPrice>
                        <span
                          className={Cn.make([
                            Emoji.Styles.bell,
                            Styles.itemPriceIcon,
                          ])}
                        />
                        <label className=Styles.itemPriceLabel>
                          {React.string("Sell")}
                        </label>
                        <span className=Styles.itemPriceValue>
                          {React.string(string_of_int(sellPrice))}
                        </span>
                      </div>
                    | None => React.null
                    }}
                 </div>;
               } else {
                 React.null;
               }}
              {switch (item.customizeCost) {
               | Some(customizeCost) =>
                 <div className=Styles.itemCustomizeCost>
                   <span className=Styles.itemRemakeIcon />
                   {React.string(
                      switch (item.bodyCustomizable, item.patternCustomizable) {
                      | (true, true) => "Body and Pattern Customizable "
                      | (false, true) => "Pattern Customizable "
                      | (true, false) => "Body Customizable "
                      | _ => ""
                      },
                    )}
                   {React.string(
                      {j|(×|j} ++ string_of_int(customizeCost) ++ ")",
                    )}
                 </div>
               | None => React.null
               }}
            </div>
            {viewportWidth <= 500 ? itemImage : React.null}
            {switch (item.type_, item.variations) {
             | (Recipe(_), _) => React.null
             | (_, Single) => React.null
             | (_, OneDimension(a)) => <OneDimensionVariants item a variant />
             | (_, TwoDimensions(a, b)) =>
               <TwoDimensionVariants item a b variant />
             }}
          </div>
        </div>
        <button
          onClick={_ => onClose()}
          className=LoginOverlay.Styles.closeButton
        />
      </div>
    </Div100VH>
  </div>;
};