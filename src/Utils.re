[@bs.val] [@bs.scope "Object"]
external objectAssign: (Js.Dict.t('a), Js.Dict.t('a)) => unit = "assign";

[@bs.val] [@bs.scope "Object"] [@bs.variadic]
external objectAssignMany: array(Js.Dict.t('a)) => unit = "assign";

let cloneJsDict = dict => {
  let clone = Js.Dict.empty();
  objectAssign(clone, dict);
  clone;
};

let combineJsDict = (dictA, dictB) => {
  let combined = Js.Dict.empty();
  objectAssignMany([|combined, dictA, dictB|]);
  combined;
};

type any;
let _internalDeleteJsDictKey: (any, string) => unit = [%bs.raw
  "function(dict, key) { delete dict[key]; }"
];
external convertToAny: Js.Dict.t('a) => any = "%identity";

let deleteJsDictKey = (dict, key) =>
  _internalDeleteJsDictKey(convertToAny(dict), key);

let getElementForDomRef = domRef => {
  domRef->React.Ref.current->Js.Nullable.toOption->Belt.Option.getExn;
};