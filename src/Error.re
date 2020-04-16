module ErrorPopup = {
  module Styles = {
    open Css;
    let root =
      style([
        position(fixed),
        top(px(32)),
        right(px(32)),
        backgroundColor(Colors.red),
        color(Colors.white),
        padding(px(24)),
        minWidth(px(256)),
        borderRadius(px(8)),
        boxShadow(Shadow.box(~blur=px(16), rgba(0, 0, 0, 0.2))),
        whiteSpace(`preLine),
      ]);
  };

  [@react.component]
  let make = (~message) => {
    <div className=Styles.root> {React.string(message)} </div>;
  };
};

let showPopup = (~message) => {
  ReactAtmosphere.API.pushLayer(~render=_ => {<ErrorPopup message />})
  |> ignore;
};