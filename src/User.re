[@bs.deriving jsConverter]
type itemStatus =
  | [@bs.as 1] Want
  | ForTrade;
type item = {
  status: itemStatus,
  note: string,
};

let itemToJson = (item: item) => {
  Json.Encode.(
    object_([
      ("status", int(itemStatusToJs(item.status))),
      ("note", string(item.note)),
    ])
  );
};

let itemFromJson = json => {
  Json.Decode.{
    status:
      (json |> field("status", int))->itemStatusFromJs->Belt.Option.getExn,
    note: json |> field("note", string),
  };
};

let getItemKey = (~itemId: string, ~variation: int) => {
  itemId ++ "//" ++ string_of_int(variation);
};

let fromItemKey = (~key: string) => {
  let [|itemId, variation|] = key |> Js.String.split("//");
  (itemId, int_of_string(variation));
};

type t = {
  id: string,
  items: Js.Dict.t(item),
  profileText: string,
};

let fromAPI = (json: Js.Json.t) => {
  Json.Decode.{
    id: json |> field("userId", string),
    items: json |> field("items", dict(itemFromJson)),
    profileText:
      (
        json
        |> field("metadata", json =>
             json |> optional(field("profileText", string))
           )
      )
      ->Belt.Option.getWithDefault(""),
  };
};