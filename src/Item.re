type recipeItem = (string, int);
type recipe = array(recipeItem);

type t = {
  id: string,
  name: string,
  image: string,
  numVariations: option(int),
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
  "Wallpaper",
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
  "Songs",
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
  "Wallpaper",
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

let otherCategories = [|"Photos", "Posters", "Fencing", "Tools", "Songs"|];

[@bs.module] external itemsJson: Js.Json.t = "./items.json";

exception UnexpectedType(string);

let spaceRegex = [%bs.re "/\\s/g"];

let jsonToItem = (json: Js.Json.t) => {
  open Json.Decode;
  let name = json |> field("name", string);
  let id =
    name |> Js.String.toLowerCase |> Js.String.replaceByRe(spaceRegex, "-");
  {
    id,
    name,
    image: id,
    numVariations: json |> optional(field("num_variants", int)),
    sellPrice: json |> optional(field("sell", int)),
    buyPrice: json |> optional(field("buy", int)),
    recipe:
      json
      |> optional(
           field(
             "recipe",
             array(json =>
               (
                 json |> field("itemName", string),
                 json |> field("count", int),
               )
             ),
           ),
         ),
    orderable:
      (json |> optional(field("catalog", bool)))
      ->Belt.Option.getWithDefault(false),
    customizable:
      (json |> optional(field("customize", bool)))
      ->Belt.Option.getWithDefault(false),
    category: json |> field("category", string),
  };
};

let all = itemsJson |> Json.Decode.array(jsonToItem);
let getItem = (~itemId) =>
  all->Belt.Array.getByU((. item) => item.id == itemId)->Belt.Option.getExn;