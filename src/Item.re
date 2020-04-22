type recipeMaterial = (string, int);
type recipe = array(recipeMaterial);

type variations =
  | Single
  | OneDimension(int)
  | TwoDimensions(int, int);

type image =
  | Base(string)
  | Array(array(string));
type t = {
  id: string,
  name: string,
  image,
  variations,
  sellPrice: option(int),
  buyPrice: option(int),
  isRecipe: bool,
  recipe: option(recipe),
  orderable: bool,
  customizable: bool,
  category: string,
};

let recipeIdRegex = [%bs.re {|/^(\d+)r$/|}];
let getRecipeIdForItemId = (~itemId) => {
  itemId ++ "r";
};
let getItemIdForRecipeId = (~recipeId) => {
  let result = recipeId |> Js.Re.exec_(recipeIdRegex);
  Belt.Option.map(
    result,
    result => {
      let matches = Js.Re.captures(result);
      matches[1]->Js.Nullable.toOption->Belt.Option.getExn;
    },
  );
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
  "photos",
  "posters",
  "fencing",
  "tools",
  "music",
  "other",
  // "Recipes",
  // "Bugs - North",
  // "Fish - North",
  // "Fossils",
  // "Construction",
  // "Nook Miles",
  // "Other",
|];

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

let otherCategories = [|"photos", "posters", "fencing", "tools", "music"|];

[@bs.val] [@bs.scope "window"] external itemsJson: Js.Json.t = "items";

exception UnexpectedType(string);

let spaceRegex = [%bs.re "/\\s/g"];

exception Unexpected;
let jsonToItem = (json: Js.Json.t) => {
  open Json.Decode;
  let flags = json |> field("flags", int);
  {
    id: json |> field("id", int) |> string_of_int,
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
    isRecipe: false,
    recipe:
      json
      |> optional(
           field(
             "recipe",
             array(json => {
               let (quantity, itemName) = json |> tuple2(int, string);
               (itemName, quantity);
             }),
           ),
         ),
    orderable: flags land 2 !== 0,
    customizable: flags land (4 lor 8) !== 0,
    category: json |> field("category", string),
  };
};

let all = {
  let allFromJson = itemsJson |> Json.Decode.array(jsonToItem);
  let recipeItems =
    allFromJson->Belt.Array.keepMap(item =>
      item.recipe
      ->Belt.Option.map(recipe =>
          {
            ...item,
            id: getRecipeIdForItemId(~itemId=item.id),
            name: item.name ++ " DIY",
            sellPrice: None,
            buyPrice: None,
            isRecipe: true,
            orderable: false,
            customizable: false,
          }
        )
    );
  allFromJson->Belt.Array.concat(recipeItems);
};
let hasItem = (~itemId) =>
  all->Belt.Array.someU((. item) => item.id == itemId);
let getItem = (~itemId) =>
  all->Belt.Array.getByU((. item) => item.id == itemId)->Belt.Option.getExn;

let getImageUrl = (~item, ~variant) => {
  Constants.imageUrl
  ++ "/"
  ++ item.category
  ++ "/"
  ++ (
    switch (item.image) {
    | Base(base) =>
      base
      ++ (
        switch (item.variations) {
        | Single => ""
        | OneDimension(_) => string_of_int(variant)
        | TwoDimensions(_a, b) =>
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

let getNumVariations = (~item) =>
  if (item.isRecipe) {
    1;
  } else {
    switch (item.variations) {
    | Single => 1
    | OneDimension(a) => a
    | TwoDimensions(a, b) => a * b
    };
  };