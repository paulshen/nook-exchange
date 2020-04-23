module Styles = {
  open Css;
  let footer =
    style([
      display(flexBox),
      flexDirection(column),
      fontSize(px(12)),
      alignItems(center),
      textAlign(center),
      margin3(~top=px(96), ~bottom=px(48), ~h=zero),
    ]);
  let contents =
    style([
      color(Colors.gray),
      maxWidth(px(512)),
      padding2(~v=zero, ~h=px(8)),
    ]);
  let disclaimer = style([marginTop(px(8))]);
};

[@react.component]
let make = () => {
  <div className=Styles.footer>
    <div className=Styles.contents>
      <div>
        <a href="https://twitter.com/nookexchange" target="_blank">
          {React.string("Twitter")}
        </a>
        {React.string(" | ")}
        <a
          href="https://docs.google.com/spreadsheets/d/1Hxrdp7oxtK-J5x9u1-rzChUpLtkv3t0_kNGdS6dtyWI/edit#gid=562907750"
          target="_blank">
          {React.string("Data Source")}
        </a>
        {React.string(" | ")}
        <a
          href="https://drive.google.com/open?id=1Ywj6etKAZe26niXDkXc-5ahjIyNdKUMB"
          target="_blank">
          {React.string("Images")}
        </a>
        {React.string(" | Thanks for visiting!")}
      </div>
      <div className=Styles.disclaimer>
        {React.string(
           "Animal Crossing is a registered trademark of Nintendo. Nook Exchange in no way claims ownership of any intellectual property associated with Animal Crossing.",
         )}
      </div>
    </div>
  </div>;
};