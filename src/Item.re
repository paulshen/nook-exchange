type t = {
  id: string,
  name: string,
  category: string,
};

[@bs.module] external itemsJson: Js.Json.t = "./items.json";

let jsonToItem = (json: Js.Json.t) => {
  Json.Decode.{
    id: json |> field("id", string),
    name: json |> field("name", string),
    category: json |> field("category", string),
  };
};

let all = itemsJson |> Json.Decode.array(jsonToItem);