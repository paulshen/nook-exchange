type recipeItem = (string, int);
type recipe = array(recipeItem);

type t = {
  id: string,
  name: string,
  recipe: option(recipe),
  orderable: bool,
  category: string,
};

[@bs.module] external itemsJson: Js.Json.t = "./items.json";

exception UnexpectedType(string);
let jsonToItem = (json: Js.Json.t) => {
  Json.Decode.{
    id: json |> field("id", string),
    name: json |> field("name", string),
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