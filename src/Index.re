[@bs.module "@sentry/browser"]
external initSentry: {. "dsn": string} => unit = "init";

initSentry({"dsn": Constants.sentryId});
UserStore.init();
Analytics.Amplitude.init();

ReactDOMRe.renderToElementWithId(<App />, "container");