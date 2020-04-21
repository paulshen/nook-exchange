type recipeItem = (string, int);
type recipe = array(recipeItem);

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
  recipe: option(recipe),
  orderable: bool,
  customizable: bool,
  category: string,
};

let categories = [|
  "Housewares",
  "Miscellaneous",
  "Wall-mounted",
  "Wallpapers",
  "Floors",
  "Rugs",
  "Tops",
  "Bottoms",
  "Dresses",
  "Headwear",
  "Accessories",
  "Socks",
  "Shoes",
  "Bags",
  "Umbrellas",
  "Photos",
  "Posters",
  "Fencing",
  "Tools",
  "Music",
  // "Recipes",
  // "Bugs - North",
  // "Fish - North",
  // "Fossils",
  // "Construction",
  // "Nook Miles",
  // "Other",
|];

let furnitureCategories = [|
  "Housewares",
  "Miscellaneous",
  "Wall-mounted",
  "Wallpapers",
  "Floors",
  "Rugs",
|];

let clothingCategories = [|
  "Tops",
  "Bottoms",
  "Dresses",
  "Headwear",
  "Accessories",
  "Socks",
  "Shoes",
  "Bags",
  "Umbrellas",
|];

let otherCategories = [|"Photos", "Posters", "Fencing", "Tools", "Music"|];

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

let all = itemsJson |> Json.Decode.array(jsonToItem);
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

let getNumVariations = (~item) => {
  switch (item.variations) {
  | Single => 1
  | OneDimension(a) => a
  | TwoDimensions(a, b) => a * b
  };
};