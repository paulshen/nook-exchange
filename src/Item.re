type recipeItem = (string, int);
type recipe = array(recipeItem);

type t = {
  id: string,
  name: string,
  variations: option(array(string)),
  recipe: option(recipe),
  orderable: bool,
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
    variations: json |> optional(field("variations", array(string))),
    recipe:
      json
      |> optional(
           field("recipe", json => {
             let recipeDict = json |> dict(int);
             Js.Dict.entries(recipeDict);
           }),
         ),
    orderable:
      (json |> optional(field("orderable", bool)))
      ->Belt.Option.getWithDefault(false),
    category: json |> field("category", string),
  };
};

let all = itemsJson |> Json.Decode.array(jsonToItem);