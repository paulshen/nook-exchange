module Styles = {
  open Css;
  let button =
    style([
      backgroundColor(hex("3aa56320")),
      borderRadius(pct(50.)),
      border(px(2), solid, Colors.green),
      cursor(pointer),
      width(px(24)),
      height(px(24)),
      outlineStyle(none),
      transition(~duration=200, "all"),
      media(
        "(hover: hover)",
        [
          hover([
            backgroundColor(hex("3aa56380")),
            borderColor(Colors.green),
          ]),
        ],
      ),
      important(opacity(1.)),
    ]);
  [@bs.module "./assets/check_white.png"]
  external checkImage: string = "default";
  let buttonSelected =
    style([
      important(backgroundColor(Colors.green)),
      backgroundImage(url(checkImage)),
      backgroundSize(size(px(16), px(16))),
      backgroundRepeat(noRepeat),
      backgroundPosition(center),
      borderColor(Colors.green),
    ]);
};

[@react.component]
let make = (~itemId, ~variant, ~selected, ~className) => {
  <button
    onClick={_ =>
      if (selected) {
        QuicklistStore.removeItem(~itemId, ~variant);
      } else {
        QuicklistStore.addItem(~itemId, ~variant);
      }
    }
    className={Cn.make([
      Styles.button,
      Cn.ifTrue(Styles.buttonSelected, selected),
      className,
    ])}>
    React.null
  </button>;
};