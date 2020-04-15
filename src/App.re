module Styles = {
  open Css;
  let root =
    style([display(flexBox), flexDirection(column), minHeight(vh(100.))]);
  let body = style([flexGrow(1.)]);
};

[@bs.val] [@bs.scope "window"]
external gtag: option((. string, string, {. "path_path": string}) => unit) =
  "gtag";

[@react.component]
let make = () => {
  let url = ReasonReactRouter.useUrl();
  let (showLogin, setShowLogin) = React.useState(() => false);

  React.useEffect1(
    () => {
      switch (gtag) {
      | Some(gtag) =>
        gtag(.
          "config",
          Constants.gtagId,
          {
            "path_path": Js.Array.joinWith("/", Belt.List.toArray(url.path)),
          },
        )
      | None => ()
      };

      None;
    },
    [|url.path|],
  );

  <div className=Styles.root>
    <HeaderBar onLogin={_ => setShowLogin(_ => true)} />
    <div className=Styles.body>
      {switch (url.path) {
       | ["u", userId] => <UserPage userId />
       | _ => <ItemBrowser showLogin={() => setShowLogin(_ => true)} />
       }}
    </div>
    <Footer />
    {showLogin
       ? <LoginOverlay onClose={() => setShowLogin(_ => false)} /> : React.null}
    <ReactAtmosphere.LayerContainer />
  </div>;
};