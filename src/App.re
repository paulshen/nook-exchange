module Styles = {
  open Css;
  let root =
    style([display(flexBox), flexDirection(column), minHeight(vh(100.))]);
  let body = style([flexGrow(1.)]);
  let tooltip =
    style([
      backgroundColor(Colors.darkLayerBackground),
      borderRadius(px(4)),
      color(Colors.white),
      fontSize(px(14)),
      padding3(~top=px(5), ~bottom=px(3), ~h=px(10)),
      position(relative),
      whiteSpace(`preLine),
      Colors.darkLayerShadow,
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
                   "offset": [|0, 2|],
                 },
               }|]),
    }),
};

[@react.component]
let make = () => {
  let url = ReasonReactRouter.useUrl();
  let (showLogin, setShowLogin) = React.useState(() => false);

  let pathString =
    "/" ++ Js.Array.joinWith("/", Belt.List.toArray(url.path));
  React.useEffect1(
    () => {
      Analytics.Amplitude.logEventWithProperties(
        ~eventName="Page Viewed",
        ~eventProperties={"path": pathString},
      );
      switch (gtag) {
      | Some(gtag) =>
        gtag(. "config", Constants.gtagId, {"page_path": pathString})
      | None => ()
      };

      None;
    },
    [|pathString|],
  );
  let (_, forceUpdate) = React.useState(() => 1);

  let locale = SettingsStore.useLocale();
  let (isLocaleLoaded, setIsLocaleLoaded) =
    React.useState(() => locale === `English);
  if (locale === `English && !isLocaleLoaded) {
    setIsLocaleLoaded(_ => true);
  };
  React.useEffect1(
    () => {
      if (locale !== `English) {
        setIsLocaleLoaded(_ => false);
        Item.loadTranslation(
          SettingsStore.localeToJs(locale),
          json => {
            setIsLocaleLoaded(_ => true);
            Item.setTranslations(json);
            forceUpdate(x => x + 1);
          },
        );
      };
      None;
    },
    [|locale|],
  );

  React.useEffect0(() => {
    Item.loadVariants(json => {
      Item.setVariantNames(json);
      forceUpdate(x => x + 1);
    });
    None;
  });

  <div className=Styles.root>
    <TooltipConfigContextProvider value=tooltipConfig>
      <HeaderBar onLogin={_ => setShowLogin(_ => true)} />
      {isLocaleLoaded
         ? <div className=Styles.body>
             {switch (url.path) {
              | ["catalog"] => <MyCatalogPage />
              | ["u", username, ...urlRest] =>
                <UserPage
                  username
                  urlRest
                  url
                  showLogin={() => setShowLogin(_ => true)}
                  key=username
                />
              | ["privacy"] => <TextPages.PrivacyPolicy />
              | ["terms"] => <TextPages.TermsOfService />
              | _ =>
                <ItemBrowser showLogin={() => setShowLogin(_ => true)} url />
              }}
           </div>
         : React.null}
      <Footer />
      {showLogin
         ? <LoginOverlay onClose={() => setShowLogin(_ => false)} />
         : React.null}
    </TooltipConfigContextProvider>
    <ReactAtmosphere.LayerContainer />
  </div>;
};