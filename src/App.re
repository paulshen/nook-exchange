module Styles = {
  open Css;
  let root =
    style([display(flexBox), flexDirection(column), minHeight(vh(100.))]);
  let body = style([flexGrow(1.)]);
  let tooltip =
    style([
      backgroundColor(Colors.charcoal),
      borderRadius(px(4)),
      color(Colors.white),
      fontSize(px(12)),
      padding2(~v=px(4), ~h=px(10)),
      position(relative),
      before([
        unsafe("content", ""),
        width(px(3)),
        height(px(3)),
        position(absolute),
        transform(rotateZ(deg(45.))),
        borderColor(Colors.charcoal),
        borderStyle(solid),
        borderTopWidth(zero),
        borderRightWidth(px(3)),
        borderBottomWidth(px(3)),
        borderLeftWidth(zero),
        bottom(px(-3)),
        left(pct(50.)),
        marginLeft(px(-4)),
      ]),
    ]);
};

[@bs.val] [@bs.scope "window"]
external gtag: option((. string, string, {. "page_path": string}) => unit) =
  "gtag";

module TooltipConfigContextProvider = {
  type tooltipModifiers =
    array({
      .
      "name": string,
      "options": {. "offset": array(int)},
    });
  let makeProps = (~value, ~children, ()) => {
    "value": value,
    "children": children,
  };
  let make =
    React.Context.provider(
      ReactAtmosphere.Tooltip.configContext:
                                             React.Context.t(
                                               ReactAtmosphere.Tooltip.configContext(
                                                 tooltipModifiers,
                                               ),
                                             ),
    );
};

let tooltipConfig:
  ReactAtmosphere.Tooltip.configContext(
    TooltipConfigContextProvider.tooltipModifiers,
  ) = {
  renderTooltip: (tooltip, _) =>
    <div className=Styles.tooltip> tooltip </div>,
  options:
    Some({
      placement: Some("top"),
      modifiers:
        Some([|{
                 "name": "offset",
                 "options": {
                   "offset": [|0, 4|],
                 },
               }|]),
    }),
};

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
    <TooltipConfigContextProvider value=tooltipConfig>
      <HeaderBar onLogin={_ => setShowLogin(_ => true)} />
      <div className=Styles.body>
        {switch (url.path) {
         | ["me"] => <MyPage url />
         | ["u", username] => <UserPage username />
         | _ => <ItemBrowser showLogin={() => setShowLogin(_ => true)} url />
         }}
      </div>
      <Footer />
      {showLogin
         ? <LoginOverlay onClose={() => setShowLogin(_ => false)} />
         : React.null}
    </TooltipConfigContextProvider>
    <ReactAtmosphere.LayerContainer />
  </div>;
};