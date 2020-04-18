module Styles = {
  open Css;
  let root =
    style([display(flexBox), flexDirection(column), minHeight(vh(100.))]);
  let body = style([flexGrow(1.)]);
};

[@bs.val] [@bs.scope "window"]
external gtag: option((. string, string, {. "page_path": string}) => unit) =
  "gtag";

[@react.component]
let make = () => {
  let url = ReasonReactRouter.useUrl();
  let (showLogin, setShowLogin) = React.useState(() => false);

  React.useEffect1(
    () => {
      let pagePath =
        "/" ++ Js.Array.joinWith("/", Belt.List.toArray(url.path));
      Analytics.Amplitude.logEventWithProperties(
        ~eventName="Page Viewed",
        ~eventProperties={"path": pagePath},
      );
      switch (gtag) {
      | Some(gtag) =>
        gtag(. "config", Constants.gtagId, {"page_path": pagePath})
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
       | ["u", username] => <UserPage username />
       | _ => <ItemBrowser showLogin={() => setShowLogin(_ => true)} />
       }}
    </div>
    <Footer />
    {showLogin
       ? <LoginOverlay onClose={() => setShowLogin(_ => false)} /> : React.null}
    <ReactAtmosphere.LayerContainer />
  </div>;
};