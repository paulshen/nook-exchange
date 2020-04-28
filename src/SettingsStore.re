[@bs.deriving jsConverter]
type language = [
  | [@bs.as "de"] `German
  | [@bs.as "es"] `Spanish
  | [@bs.as "fr"] `French
  | [@bs.as "it"] `Italian
  | [@bs.as "ja"] `Japanese
  | [@bs.as "ko"] `Korean
  | [@bs.as "nl"] `Dutch
  | [@bs.as "zh-cn"] `ChineseSimplified
  | [@bs.as "zh-tw"] `ChineseTraditional
  | [@bs.as "en"] `English
];

let languages: array(language) = [|
  `English,
  `Spanish,
  `French,
  `German,
  `Italian,
  `Dutch,
  `Japanese,
  `Korean,
  `ChineseTraditional,
  `ChineseSimplified,
|];

let languageToString = (language: language) => {
  switch (language) {
  | `German => {j|Deutsch|j}
  | `Spanish => {j|Español|j}
  | `French => {j|Français|j}
  | `Italian => {j|Italiano|j}
  | `Japanese => {j|日本語|j}
  | `Korean => {j|한국어|j}
  | `Dutch => {j|Dansk|j}
  | `ChineseSimplified => {j|中文|j}
  | `ChineseTraditional => {j|繁體中文|j}
  | `English => {j|English|j}
  };
};

[@bs.val] [@bs.scope "navigator"]
external browserLanguage: string = "language";

let browserLanguage =
  if (browserLanguage == "de") {
    `German;
  } else if (browserLanguage |> Js.String.startsWith("es")) {
    `Spanish;
  } else if (browserLanguage |> Js.String.startsWith("fr")) {
    `French;
  } else if (browserLanguage |> Js.String.startsWith("it")) {
    `Italian;
  } else if (browserLanguage == "ja") {
    `Japanese;
  } else if (browserLanguage == "ko") {
    `Korean;
  } else if (browserLanguage == "nl") {
    `Dutch;
  } else if (Js.String.toLowerCase(browserLanguage) == "zh-cn") {
    `ChineseSimplified;
  } else if (browserLanguage |> Js.String.startsWith("zh")) {
    `ChineseTraditional;
  } else {
    `English;
  };

type state = {language};
type action =
  | SetLanguage(language);

let localStorageKey = "language";
let localStorageLanguage =
  Dom.Storage.localStorage |> Dom.Storage.getItem(localStorageKey);

let api =
  Restorative.createStore(
    {
      language:
        localStorageLanguage
        ->Belt.Option.flatMap(languageFromJs)
        ->Belt.Option.getWithDefault(`English),
    },
    (state, action) => {
    switch (action) {
    | SetLanguage(language) => {language: language}
    }
  });

let useLanguage = () => {
  api.useStoreWithSelector(state => state.language, ());
};

let setLanguage = (~language) => {
  api.dispatch(SetLanguage(language));
  if (language == browserLanguage) {
    Dom.Storage.localStorage |> Dom.Storage.removeItem(localStorageKey);
  } else {
    Dom.Storage.localStorage
    |> Dom.Storage.setItem(localStorageKey, languageToJs(language));
  };
  Analytics.Amplitude.logEventWithProperties(
    ~eventName="Language Changed",
    ~eventProperties={"language": language},
  );
};