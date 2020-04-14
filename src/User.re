[@bs.deriving jsConverter]
type itemStatus =
  | [@bs.as 1] Want
  | WillTrade;
type item = {
  itemId: string,
  variation: option(string),
  status: itemStatus,
};

let itemToJson = (item: item) => {
  Json.Encode.(
    object_([
      ("itemId", string(item.itemId)),
      ("variation", nullable(string, item.variation)),
      ("status", int(itemStatusToJs(item.status))),
    ])
  );
};

let itemFromJson = json => {
  Json.Decode.{
    itemId: json |> field("itemId", string),
    variation:
      json |> field("variation", nullable(string)) |> Js.Null.toOption,
    status:
      (json |> field("status", int))->itemStatusFromJs->Belt.Option.getExn,
  };
};

type t = {
  id: string,
  items: array(item),
};

let fromAPI = (json: Js.Json.t) => {
  Json.Decode.{
    id: json |> field("userId", string),
    items:
      json
      |> field("data", json =>
           (json |> optional(field("items", array(itemFromJson))))
           ->Belt.Option.getWithDefault([||])
         ),
  };
};

let toAPI = (user: t) => {
  Json.Encode.(
    object_([
      ("userId", string(user.id)),
      ("data", object_([("items", array(itemToJson, user.items))])),
    ])
  );
};