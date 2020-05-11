module Styles = {
  open Css;
  let pageTitle =
    style([fontSize(px(32)), textAlign(center), marginBottom(px(32))]);
};

[@react.component]
let make = (~title) => {
  <div className=Styles.pageTitle> {React.string(title)} </div>;
};