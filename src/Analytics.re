module Amplitude = {
  module API = {
    type t;

    [@bs.module "amplitude-js"]
    external getInstance: unit => t = "getInstance";

    [@bs.send] external init: (t, string) => unit = "init";
    [@bs.send] external logEvent: (t, string) => unit = "logEvent";
    [@bs.send]
    external logEventWithProperties: (t, string, 'a) => unit = "logEvent";
    [@bs.send] external setUserId: (t, option(string)) => unit = "setUserId";
  };

  let instanceRef = ref(None);
  let getInstance = () => {
    switch (instanceRef^) {
    | Some(instance) => instance
    | None =>
      let instance = API.getInstance();
      instance->API.init(Constants.amplitudeApiKey);
      instanceRef := Some(instance);
      instance;
    };
  };

  let init = () => {
    getInstance() |> ignore;
  };

  let logEvent = (~eventName) => {
    getInstance()->API.logEvent(eventName);
  };

  let logEventWithProperties = (~eventName, ~eventProperties) => {
    getInstance()->API.logEventWithProperties(eventName, eventProperties);
  };

  let setUserId = (~userId) => {
    getInstance()->API.setUserId(userId);
  };
};