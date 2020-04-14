module Styles = {
  open Css;
  let root = style([paddingBottom(px(64))]);
};

[@react.component]
let make = () => {
  let url = ReasonReactRouter.useUrl();
  let (showLogin, setShowLogin) = React.useState(() => false);
  <div className=Styles.root>
    <HeaderBar onLogin={_ => setShowLogin(_ => true)} />
    {switch (url.path) {
     | ["u", userId] => <UserPage userId />
     | _ => <ItemBrowser showLogin={() => setShowLogin(_ => true)} />
     }}
    {showLogin
       ? <LoginOverlay onClose={() => setShowLogin(_ => false)} /> : React.null}
  </div>;
};