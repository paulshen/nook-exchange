[@bs.deriving jsConverter]
type itemStatus =
  | [@bs.as 1] Wishlist
  | ForTrade
  | CanCraft;
let itemStatusToUrl = itemStatus =>
  switch (itemStatus) {
  | Wishlist => "wishlist"
  | ForTrade => "for-trade"
  | CanCraft => "can-craft"
  };
let urlToItemStatus = url =>
  switch (url) {
  | "for-trade" => ForTrade
  | "can-craft" => CanCraft
  | "wishlist"
  | _ => Wishlist
  };
let itemStatusToEmoji = itemStatus => {
  switch (itemStatus) {
  | Wishlist => {j|🙏|j}
  | ForTrade => {j|🤝|j}
  | CanCraft => {j|🔨|j}
  };
};
let itemStatusToString = itemStatus =>
  switch (itemStatus) {
  | Wishlist => "Wishlist"
  | ForTrade => "For Trade"
  | CanCraft => "Can Craft"
  };
type item = {
  status: itemStatus,
  note: string,
};

let itemToJson = (item: item) => {
  Json.Encode.(
    object_(
      [
        Some(("status", int(itemStatusToJs(item.status)))),
        item.note != "" ? Some(("note", string(item.note))) : None,
      ]
      ->Belt.List.keepMap(x => x),
    )
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