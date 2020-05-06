module Styles = {
  open Css;
  let button =
    style([
      backgroundColor(Colors.faintGray),
      borderRadius(pct(50.)),
      borderWidth(zero),
      width(px(24)),
      height(px(24)),
    ]);
  [@bs.module "./assets/check.png"] external checkImage: string = "default";
  let buttonSelected =
    style([
      backgroundImage(url(checkImage)),
      backgroundSize(size(px(16), px(16))),
      backgroundRepeat(noRepeat),
      backgroundPosition(center),
      opacity(0.5),
    ]);
};

[@react.component]
let make = (~itemId, ~variant, ~className) => {
  let isInQuicklist = QuicklistStore.useItemState(~itemId, ~variant);
  <button
    className={Cn.make([
      Styles.button,
      Cn.ifTrue(Styles.buttonSelected, isInQuicklist),
      className,
    ])}>
    React.null
  </button>;
};