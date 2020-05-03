type recipeMaterial = (string, int);
type recipe = array(recipeMaterial);

type variations =
  | Single
  | OneDimension(int)
  | TwoDimensions(int, int);

type image =
  | Base(string)
  | Array(array(string));
type type_ =
  | Item(option(int))
  | Recipe(int);
type t = {
  id: int,
  type_,
  name: string,
  image,
  variations,
  sellPrice: option(int),
  buyPrice: option(int),
  recipe: option(recipe),
  orderable: bool,
  bodyCustomizable: bool,
  patternCustomizable: bool,
  category: string,
  version: option(string),
  tags: array(string),
};

let categories = [|
  "housewares",
  "miscellaneous",
  "wall-mounted",
  "wallpapers",
  "floors",
  "rugs",
  "tops",
  "bottoms",
  "dresses",
  "headwear",
  "accessories",
  "socks",
  "shoes",
  "bags",
  "umbrellas",
  "fossils",
  "photos",
  "posters",
  "fencing",
  "tools",
  "music",
  "other",
|];
let validCategoryStrings =
  [|"furniture", "clothing", "recipes"|]->Belt.Array.concat(categories);

let furnitureCategories = [|
  "housewares",
  "miscellaneous",
  "wall-mounted",
  "wallpapers",
  "floors",
  "rugs",
|];

let clothingCategories = [|
  "tops",
  "bottoms",
  "dresses",
  "headwear",
  "accessories",
  "socks",
  "shoes",
  "bags",
  "umbrellas",
|];

let otherCategories = [|
  "art",
  "fossils",
  "photos",
  "posters",
  "fencing",
  "tools",
  "music",
|];

[@bs.val] [@bs.scope "window"] external itemsJson: Js.Json.t = "items";

let loadTranslation: (string, Js.Json.t => unit) => unit = [%raw
  {|function(language, callback) {
    import(/* webpackChunkName */ './translations/' + language + '.json').then(j => callback(j.default))
  }|}
];

exception UnexpectedType(string);

let spaceRegex = [%bs.re "/\\s/g"];

exception Unexpected;
let jsonToItem = (json: Js.Json.t) => {
  open Json.Decode;
  let flags = json |> field("flags", int);
  let recipeInfo =
    json
    |> optional(
         field("recipe", json => {
           let jsonArray = Js.Json.decodeArray(json)->Belt.Option.getExn;
           (
             - int(jsonArray[0]),
             jsonArray
             |> Js.Array.sliceFrom(1)
             |> Js.Array.map(json => {
                  let (quantity, itemName) = json |> tuple2(int, string);
                  (itemName, quantity);
                }),
           );
         }),
       );
  {
    id: json |> field("id", int),
    type_: Item(recipeInfo->Belt.Option.map(((recipeId, _)) => recipeId)),
    name: json |> field("name", string),
    image:
      json
      |> field(
           "image",
           oneOf([
             json => Base(string(json)),
             json => Array(array(string, json)),
           ]),
         ),
    variations: {
      switch (json |> field("variants", array(int))) {
      | [||] => Single
      | [|a|] => OneDimension(a)
      | [|a, b|] => TwoDimensions(a, b)
      | _ => raise(Unexpected)
      };
    },
    sellPrice: json |> optional(field("sell", int)),
    buyPrice: json |> optional(field("buy", int)),
    recipe: recipeInfo->Belt.Option.map(((_, recipe)) => recipe),
    orderable: flags land 2 !== 0,
    bodyCustomizable: flags land 4 != 0,
    patternCustomizable: flags land 8 != 0,
    category: json |> field("category", string),
    version: json |> optional(field("v", string)),
    tags:
      (json |> optional(field("tags", array(string))))
      ->Belt.Option.getWithDefault([||]),
  };
};

let all = {
  let allFromJson = itemsJson |> Json.Decode.array(jsonToItem);
  let recipeItems =
    allFromJson->Belt.Array.keepMap(item =>
      (
        switch (item.type_) {
        | Item(recipeId) => recipeId
        | Recipe(_) => raise(Constants.Uhoh)
        }
      )
      ->Belt.Option.map(recipeId =>
          {
            ...item,
            id: recipeId,
            type_: Recipe(item.id),
            name: item.name ++ " DIY",
            sellPrice: None,
            buyPrice: None,
            orderable: false,
            bodyCustomizable: false,
            patternCustomizable: false,
          }
        )
    );
  allFromJson->Belt.Array.concat(recipeItems);
};
let itemMap = {
  let itemMap = Js.Dict.empty();
  all->Belt.Array.forEach(item => {
    itemMap->Js.Dict.set(string_of_int(item.id), item)
  });
  itemMap;
};
let getItem = (~itemId) =>
  itemMap->Js.Dict.unsafeGet(string_of_int(itemId));

exception UnexpectedVersion(string);
let getImageUrl = (~item, ~variant) => {
  Constants.cdnUrl
  ++ "/items/"
  ++ (
    switch (item.image) {
    | Base(base) =>
      base
      ++ (
        switch (item.category, item.variations) {
        | ("art", _) => ""
        | (_, Single) => ""
        | (_, OneDimension(_)) => string_of_int(variant)
        | (_, TwoDimensions(_a, b)) =>
          "_"
          ++ string_of_int(variant / b)
          ++ "_"
          ++ string_of_int(variant mod b)
        }
      )
    | Array(variantImages) => variantImages[variant]
    }
  )
  ++ ".png";
};

let isRecipe = (~item: t) => {
  switch (item.type_) {
  | Item(_) => false
  | Recipe(_) => true
  };
};

let getRecipeIdForItem = (~item: t) => {
  switch (item.type_) {
  | Item(recipeId) => recipeId
  | Recipe(_) => raise(Constants.Uhoh)
  };
};

let getItemIdForRecipe = (~recipe: t) => {
  switch (recipe.type_) {
  | Recipe(itemId) => itemId
  | Item(_) => raise(Constants.Uhoh)
  };
};

let getNumVariations = (~item) =>
  if (isRecipe(~item)) {
    1;
  } else {
    switch (item.variations) {
    | Single => 1
    | OneDimension(a) => a
    | TwoDimensions(a, b) => a * b
    };
  };

let getCollapsedVariants = (~item: t) => {
  switch (item.type_, item.variations) {
  | (Recipe(_), _)
  | (_, Single) => [|0|]
  | (_, OneDimension(a)) =>
    Array.make(a, None)->Belt.Array.mapWithIndex((i, _) => i)
  | (_, TwoDimensions(a, b)) =>
    if (item.bodyCustomizable) {
      [|0|];
    } else {
      Array.make(a, None)->Belt.Array.mapWithIndex((i, _) => i * b);
    }
  };
};

let getCanonicalVariant = (~item, ~variant) => {
  switch (item.variations) {
  | Single => 0
  | OneDimension(_a) => variant
  | TwoDimensions(_a, b) =>
    if (item.bodyCustomizable) {
      0;
    } else {
      variant / b * b;
    }
  };
};

let loadVariants: (Js.Json.t => unit) => unit = [%raw
  {|function(callback) {
    import(/* webpackChunkName variants */ './variants.json').then(j => callback(j.default))
  }|}
];
type variantNames =
  | NameOneDimension(array(string))
  | NameTwoDimensions((array(string), array(string)));
let variantNames: ref(option(Js.Dict.t(variantNames))) = ref(None);
let setVariantNames = json => {
  Json.Decode.(
    variantNames :=
      Some(
        json
        |> dict(
             oneOf([
               json =>
                 NameTwoDimensions(
                   json |> tuple2(array(string), array(string)),
                 ),
               json => NameOneDimension(json |> array(string)),
             ]),
           ),
      )
  );
};

let loadTranslation: (string, Js.Json.t => unit) => unit = [%raw
  {|function(language, callback) {
    import(/* webpackChunkName */ './translations/' + language + '.json').then(j => callback(j.default))
  }|}
];
type translationItem = {
  name: string,
  variants: option(variantNames),
};
type translations = {
  items: Js.Dict.t(translationItem),
  materials: Js.Dict.t(string),
};
let translations: ref(option(translations)) = ref(None);
let setTranslations = json => {
  Json.Decode.(
    translations :=
      Some({
        items:
          json
          |> field(
               "items",
               dict(json => {
                 let row = Js.Json.decodeArray(json)->Belt.Option.getExn;
                 {
                   name: string(row[0]),
                   variants:
                     Belt.Option.map(Belt.Array.get(row, 1), json => {
                       json
                       |> oneOf([
                            json =>
                              NameTwoDimensions(
                                json |> tuple2(array(string), array(string)),
                              ),
                            json => NameOneDimension(json |> array(string)),
                          ])
                     }),
                 };
               }),
             ),
        materials: json |> field("materials", dict(string)),
      })
  );
};
let clearTranslations = () => {
  translations := None;
};

let getName = (item: t) =>
  switch (item.type_) {
  | Recipe(itemId) =>
    Belt.(
      (translations^)
      ->Option.flatMap(translations =>
          Js.Dict.get(translations.items, string_of_int(itemId))
        )
      ->Option.map(translation => translation.name ++ " DIY")
      ->Option.getWithDefault(item.name)
    )
  | Item(_) =>
    Belt.(
      (translations^)
      ->Option.flatMap(translations =>
          Js.Dict.get(translations.items, string_of_int(item.id))
        )
      ->Option.map(translation => translation.name)
      ->Option.getWithDefault(item.name)
    )
  };

let getVariantName = (~item: t, ~variant: int, ~hidePattern=false, ()) => {
  Belt.(
    switch (item.variations) {
    | Single => None
    | OneDimension(_) =>
      (
        switch (
          (translations^)
          ->Option.flatMap(translations =>
              Js.Dict.get(translations.items, string_of_int(item.id))
            )
          ->Option.flatMap(translationItem => translationItem.variants)
        ) {
        | Some(value) => Some(value)
        | None =>
          (variantNames^)
          ->Option.flatMap(Js.Dict.get(_, string_of_int(item.id)))
        }
      )
      ->Option.flatMap(value =>
          switch (value) {
          | NameOneDimension(names) => Some(Option.getExn(names[variant]))
          | _ => None
          }
        )
    | TwoDimensions(_a, b) =>
      (
        switch (
          (translations^)
          ->Option.flatMap(translations =>
              Js.Dict.get(translations.items, string_of_int(item.id))
            )
          ->Option.flatMap(translationItem => translationItem.variants)
        ) {
        | Some(value) => Some(value)
        | None =>
          (variantNames^)
          ->Option.flatMap(Js.Dict.get(_, string_of_int(item.id)))
        }
      )
      ->Belt.Option.flatMap(value =>
          switch (value) {
          | NameTwoDimensions((nameA, nameB)) =>
            Some(
              Option.getExn(nameA[variant / b])
              ++ (
                if (!hidePattern && b > 1) {
                  " x " ++ Option.getExn(nameB[variant mod b]);
                } else {
                  "";
                }
              ),
            )
          | _ => None
          }
        )
    }
  );
};

let getMaterialName = (material: string) =>
  Belt.(
    (translations^)
    ->Option.flatMap(translations =>
        Js.Dict.get(translations.materials, material)
      )
    ->Option.getWithDefault(material)
  );