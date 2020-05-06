type t = {
  id: option(string),
  itemIds: array((int, int)),
};

type action =
  | StartList
  | AddItem(int, int)
  | RemoveItem(int, int)
  | SaveList(string);

open Belt;

let api =
  Restorative.createStore(Some({id: None, itemIds: [||]}), (state, action) => {
    switch (action) {
    | StartList => Some({id: None, itemIds: [||]})
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
    }
  });

let useHasQuicklist = () => {
  api.useStoreWithSelector(state => state != None, ());
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

let addItem = (~itemId, ~variant) => {
  api.dispatch(AddItem(itemId, variant));
};

let removeItem = (~itemId, ~variant) => {
  api.dispatch(RemoveItem(itemId, variant));
};