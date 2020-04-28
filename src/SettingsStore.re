[@bs.deriving jsConverter]
type locale = [
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

[@bs.val] [@bs.scope "navigator"]
external browserLanguage: string = "language";

let browserLocale =
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

type state = {locale};
type action =
  | SetLocale(locale);

let localStorageKey = "locale";
let localStorageLocale =
  Dom.Storage.localStorage |> Dom.Storage.getItem(localStorageKey);

let api =
  Restorative.createStore(
    {
      locale:
        localStorageLocale
        ->Belt.Option.flatMap(localeFromJs)
        ->Belt.Option.getWithDefault(`English),
    },
    (state, action) => {
    switch (action) {
    | SetLocale(locale) => {locale: locale}
    }
  });

let useLocale = () => {
  api.useStoreWithSelector(state => state.locale, ());
};

let setLocale = (~locale) =>
  if (locale == browserLocale) {
    Dom.Storage.localStorage |> Dom.Storage.removeItem(localStorageKey);
  } else {
    Dom.Storage.localStorage
    |> Dom.Storage.setItem(localStorageKey, localeToJs(locale));
  };