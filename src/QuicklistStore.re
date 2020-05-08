type t = {
  id: option(string),
  userId: option(string),
  itemIds: array((int, int)),
};

type action =
  | StartList
  | AddItem(int, int)
  | RemoveItem(int, int)
  | SaveList(string)
  | RemoveList;

open Belt;

let api =
  Restorative.createStore(None, (state, action) => {
    switch (action) {
    | StartList => Some({id: None, userId: None, itemIds: [||]})
    | AddItem(itemId, variant) =>
      state->Option.map(state =>
        {
          ...state,
          itemIds: state.itemIds->Array.concat([|(itemId, variant)|]),
        }
      )
    | RemoveItem(itemId, variant) =>
      state->Option.map(state =>
        {
          ...state,
          itemIds:
            state.itemIds
            ->Array.keep(((a, b)) => a != itemId || b != variant),
        }
      )
    | SaveList(id) => state->Option.map(state => {...state, id: Some(id)})
    | RemoveList => None
    }
  });

let useHasQuicklist = () => {
  api.useStoreWithSelector(state => state != None, ());
};
let useQuicklist = () => {
  api.useStore();
};

let useItemState = (~itemId, ~variant) => {
  api.useStoreWithSelector(
    state =>
      switch (state) {
      | Some(state) =>
        state.itemIds->Array.getBy(((a, b)) => a == itemId && b == variant)
        !== None
      | None => false
      },
    (),
  );
};

let startList = () => {
  api.dispatch(StartList);
  Analytics.Amplitude.logEventWithProperties(
    ~eventName="Item List Started",
    ~eventProperties={
      "path": Webapi.Dom.(location |> Location.pathname),
      "isLoggedIn": UserStore.isLoggedIn(),
      "isViewingSelf":
        switch (UserStore.getUserOption()) {
        | Some(user) =>
          let url = ReasonReactRouter.dangerouslyGetInitialUrl();
          switch (url.path) {
          | ["u", username, ..._] =>
            Js.String.toLowerCase(username)
            == Js.String.toLowerCase(user.username)
          | _ => false
          };
        | None => false
        },
    },
  );
};

let hasLoggedItemAdd = ref(false);
let addItem = (~itemId, ~variant) => {
  api.dispatch(AddItem(itemId, variant));
  if (! hasLoggedItemAdd^) {
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Item List Item Added",
      ~eventProperties={"itemId": itemId, "variant": variant},
    );
    hasLoggedItemAdd := true;
  };
};

let hasLoggedItemRemove = ref(false);
let removeItem = (~itemId, ~variant) => {
  api.dispatch(RemoveItem(itemId, variant));
  if (! hasLoggedItemRemove^) {
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Item List Item Removed",
      ~eventProperties={"itemId": itemId, "variant": variant},
    );
    hasLoggedItemRemove := true;
  };
};

let removeList = () => {
  api.dispatch(RemoveList);
};

let saveList = () => {
  let list = api.getState()->Belt.Option.getExn;
  let itemIds = list.itemIds;
  let%Repromise responseResult =
    BAPI.createItemList(~sessionId=UserStore.sessionId^, ~items=itemIds);
  UserStore.handleServerResponse("/item-lists", responseResult);
  let response = Belt.Result.getExn(responseResult);
  let%Repromise.JsExn json = Fetch.Response.json(response);
  let listId = json |> Json.Decode.(field("id", string));
  Analytics.Amplitude.logEventWithProperties(
    ~eventName="Item List Created",
    ~eventProperties={
      "listId": listId,
      "numItems": Js.Array.length(itemIds),
    },
  );
  // api.dispatch(SaveList(listId));
  Promise.resolved(listId);
};