module Styles = {
  open Css;
  let root =
    style([display(flexBox), alignItems(stretch), justifyContent(center)]);
  let mainImageWrapper = style([position(relative)]);
  let mainImageWrapperRecipe = style([marginBottom(px(16))]);
  let image = style([display(block), height(px(128)), width(px(128))]);
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
  let variantButton =
    style([
      background(transparent),
      borderWidth(zero),
      padding(zero),
      cursor(pointer),
      flexGrow(1.),
      display(flexBox),
      alignItems(center),
      justifyContent(center),
      transition(~duration=200, "all"),
      outlineStyle(none),
      opacity(0.2),
      media("(hover: hover)", [hover([important(opacity(1.))])]),
      disabled([important(opacity(0.1)), cursor(`default)]),
      unsafe("touchAction", "manipulation"),
    ]);
  let variantButtonLeft = "variant-button-left";
  let variantButtonRight = "variant-button-right";
  [@bs.module "./assets/variant_left.png"]
  external variantLeftIcon: string = "default";
  let variantArrow =
    style([
      display(inlineBlock),
      width(px(32)),
      height(px(32)),
      backgroundSize(cover),
    ]);
  let variantArrowLeft = style([backgroundImage(url(variantLeftIcon))]);
  [@bs.module "./assets/variant_right.png"]
  external variantRightIcon: string = "default";
  let variantArrowRight = style([backgroundImage(url(variantRightIcon))]);
  let variantButtonNarrow =
    style([
      selector(
        "&." ++ variantButtonLeft,
        [position(relative), left(px(4))],
      ),
      selector(
        "&." ++ variantButtonRight,
        [position(relative), left(px(-4))],
      ),
      selector("& ." ++ variantArrow, [width(px(16)), height(px(16))]),
    ]);
};

[@react.component]
let make =
    (
      ~item: Item.t,
      ~variant,
      ~narrow=false,
      ~forceTooltip=false,
      ~className,
      (),
    ) => {
  let numCollapsedVariants =
    switch (item.variations) {
    | Single
    | OneDimension(_) => 1
    | TwoDimensions(a, b) =>
      if (item.bodyCustomizable) {
        a * b;
      } else {
        b;
      }
    };
  let (offset, setOffset) = React.useState(() => 0);
  <div className={Cn.make([Styles.root, className])}>
    {numCollapsedVariants > 1
       ? <button
           disabled={offset <= 0}
           onClick={_ => {setOffset(offset => offset - 1)}}
           className={Cn.make([
             Styles.variantButton,
             Styles.variantButtonLeft,
             Cn.ifTrue(Styles.variantButtonNarrow, narrow),
           ])}>
           <span
             className={Cn.make([
               Styles.variantArrow,
               Styles.variantArrowLeft,
             ])}
           />
         </button>
       : React.null}
    <div
      className={Cn.make([
        Styles.mainImageWrapper,
        Cn.ifTrue(Styles.mainImageWrapperRecipe, item.isRecipe),
        className,
      ])}>
      {let image =
         <img
           src={Item.getImageUrl(~item, ~variant=variant + offset)}
           className=Styles.image
         />;
       let variantName =
         if (numCollapsedVariants > 1 || forceTooltip) {
           Item.getVariantName(~item, ~variant=variant + offset, ());
         } else {
           None;
         };
       switch (variantName) {
       | Some(variantName) =>
         <ReactAtmosphere.Tooltip text={React.string(variantName)}>
           {(
              ({onMouseEnter, onMouseLeave, onFocus, onBlur, ref}) =>
                <div
                  onMouseEnter
                  onMouseLeave
                  onFocus
                  onBlur
                  ref={ReactDOMRe.Ref.domRef(ref)}>
                  image
                </div>
            )}
         </ReactAtmosphere.Tooltip>
       | None => image
       }}
      {item.isRecipe
         ? <img
             src={Constants.cdnUrl ++ "/images/DIYRecipe.png"}
             className=Styles.recipeIcon
           />
         : React.null}
    </div>
    {numCollapsedVariants > 1
       ? <button
           disabled={offset >= numCollapsedVariants - 1}
           onClick={_ => {setOffset(offset => offset + 1)}}
           className={Cn.make([
             Styles.variantButton,
             Styles.variantButtonRight,
             Cn.ifTrue(Styles.variantButtonNarrow, narrow),
           ])}>
           <span
             className={Cn.make([
               Styles.variantArrow,
               Styles.variantArrowRight,
             ])}
           />
         </button>
       : React.null}
  </div>;
};