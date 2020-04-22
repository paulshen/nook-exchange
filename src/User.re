[@bs.deriving jsConverter]
type itemStatus =
  | [@bs.as 1] Wishlist
  | ForTrade
  | CanCraft;
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
  open Json.Decode;
  let status = (json |> field("status", int))->itemStatusFromJs;
  Belt.Option.map(status, status =>
    {
      status,
      note:
        (json |> optional(field("note", string)))
        ->Belt.Option.getWithDefault(""),
    }
  );
};

exception InvalidRecipeItemKey(string, int);
let getItemKey = (~itemId: string, ~variation: int) => {
  if (Item.getItemIdForRecipeId(~recipeId=itemId) !== None && variation != 0) {
    raise(InvalidRecipeItemKey(itemId, variation));
  };
  itemId ++ "@@" ++ string_of_int(variation);
};

let fromItemKey = (~key: string) => {
  let [|itemId, variation|] = key |> Js.String.split("@@");
  let variation = int_of_string(variation);
  if (Item.getItemIdForRecipeId(~recipeId=itemId) !== None && variation != 0) {
    raise(InvalidRecipeItemKey(itemId, variation));
  };
  (itemId, variation);
};

type t = {
  id: string,
  username: string,
  items: Js.Dict.t(item),
  profileText: string,
};

let fromAPI = (json: Js.Json.t) => {
  Json.Decode.{
    id: json |> field("uuid", string),
    username: json |> field("username", string),
    items:
      (json |> field("items", dict(itemFromJson)))
      ->Js.Dict.entries
      ->Belt.Array.keepMap(((itemKey, value)) => {
          Belt.Option.flatMap(
            value,
            value => {
              let (itemId, _) = fromItemKey(~key=itemKey);
              Item.hasItem(~itemId) ? Some((itemKey, value)) : None;
            },
          )
        })
      ->Js.Dict.fromArray,
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