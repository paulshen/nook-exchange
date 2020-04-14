type recipeItem = (string, int);
type recipe = array(recipeItem);

type t = {
  id: string,
  name: string,
  image: string,
  variations: option(array(string)),
  recipe: option(recipe),
  orderable: bool,
  customizable: bool,
  category: string,
};

[@bs.module] external itemsJson: Js.Json.t = "./items.json";

exception UnexpectedType(string);

let spaceRegex = [%bs.re "/\\s/g"];

let jsonToItem = (json: Js.Json.t) => {
  open Json.Decode;
  let name = json |> field("name", string);
  {
    id:
      name |> Js.String.toLowerCase |> Js.String.replaceByRe(spaceRegex, "-"),
    name,
    image:
      (json |> optional(field("image", string)))
      ->Belt.Option.getWithDefault(""),
    variations: json |> optional(field("variants", array(string))),
    recipe:
      json
      |> optional(
           field("recipe", json => {
             let recipeDict = json |> dict(int);
             Js.Dict.entries(recipeDict);
           }),
         ),
    orderable:
      (json |> optional(field("reorder", bool)))
      ->Belt.Option.getWithDefault(false),
    customizable:
      (json |> optional(field("customize", bool)))
      ->Belt.Option.getWithDefault(false),
    category: json |> field("category", string),
  };
};

let all = itemsJson |> Json.Decode.array(jsonToItem);