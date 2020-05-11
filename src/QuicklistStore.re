type t = {
  id: option(string),
  userId: option(string),
  itemIds: array((int, int)),
};

type action =
  | StartList
  | LoadList(string, array((int, int)))
  | AddItem(int, int)
  | RemoveItem(int, int)
  | SaveList(string)
  | RemoveList;

open Belt;

let api =
  Restorative.createStore(None, (state, action) => {
    switch (action) {
    | StartList => Some({id: None, userId: None, itemIds: [||]})
    | LoadList(id, itemIds) => Some({id: Some(id), userId: None, itemIds})
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

let removeList = () => {
  api.dispatch(RemoveList);
};

let hasLoggedItemListUpdate = ref(false);
let saveList = () => {
  let list = api.getState()->Belt.Option.getExn;
  let itemIds = list.itemIds;
  switch (list.id) {
  | Some(listId) =>
    let%Repromise responseResult =
      BAPI.updateItemList(
        ~sessionId=Belt.Option.getExn(UserStore.sessionId^),
        ~listId,
        ~items=itemIds,
      );
    UserStore.handleServerResponse("/item-lists/patch", responseResult);
    if (! hasLoggedItemListUpdate^) {
      Analytics.Amplitude.logEventWithProperties(
        ~eventName="Item List Updated",
        ~eventProperties={
          "listId": listId,
          "numItems": Js.Array.length(itemIds),
        },
      );
      hasLoggedItemListUpdate := true;
    };
    Promise.resolved(listId);
  | None =>
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
    Promise.resolved(listId);
  };
};

let hasLoggedItemAdd = ref(false);
let addItem = (~itemId, ~variant) => {
  api.dispatch(AddItem(itemId, variant));
  switch (api.getState()) {
  | Some(quicklist) =>
    switch (quicklist.id) {
    | Some(_listId) => saveList() |> ignore
    | None =>
      if (! hasLoggedItemAdd^) {
        Analytics.Amplitude.logEventWithProperties(
          ~eventName="Item List Item Added",
          ~eventProperties={"itemId": itemId, "variant": variant},
        );
        hasLoggedItemAdd := true;
      }
    }
  | None => ()
  };
};

let hasLoggedItemRemove = ref(false);
let removeItem = (~itemId, ~variant) => {
  api.dispatch(RemoveItem(itemId, variant));
  switch (api.getState()) {
  | Some(quicklist) =>
    switch (quicklist.id) {
    | Some(_listId) => saveList() |> ignore
    | None =>
      if (! hasLoggedItemRemove^) {
        Analytics.Amplitude.logEventWithProperties(
          ~eventName="Item List Item Removed",
          ~eventProperties={"itemId": itemId, "variant": variant},
        );
        hasLoggedItemRemove := true;
      }
    }
  | None => ()
  };
};

let loadList = (~listId, ~listItems) => {
  api.dispatch(LoadList(listId, listItems));
};