module Styles = {
  open Css;
  let button =
    style([
      backgroundColor(Colors.green),
      borderWidth(zero),
      borderRadius(px(4)),
      color(Colors.white),
      cursor(pointer),
      padding2(~v=px(10), ~h=px(16)),
      fontSize(px(14)),
      transition(~duration=200, "all"),
      disabled([opacity(0.5)]),
    ]);
};

[@react.component]
let make = (~onClick=?, ~children, ~className=?, ()) => {
  <button
    className={Cn.make([Styles.button, Cn.unpack(className)])} ?onClick>
    children
  </button>;
};