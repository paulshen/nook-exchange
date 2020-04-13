open Belt;

type state = option(User.t);
type action =
  | Login(User.t)
  | UpdateUser(User.t)
  | Logout;

let api =
  Restorative.createStore(None, (state, action) => {
    switch (action) {
    | Login(user) => Some(user)
    | UpdateUser(user) => Some(user)
    | Logout => None
    }
  });

let useMe = () => api.useStore();
let useItem = (~itemId, ~variation) => {
  let selector =
    React.useCallback2(
      (state: state) => {
        Option.flatMap(state, user =>
          user.items
          ->Array.getBy(item =>
              item.itemId == itemId && item.variation == variation
            )
        )
      },
      (itemId, variation),
    );
  api.useStoreWithSelector(selector, ());
};

let postUpdatedUser = (~user) => {
  {
    Fetch.fetchWithInit(
      Constants.apiUrl ++ "/me",
      Fetch.RequestInit.make(
        ~method_=Post,
        ~body=Fetch.BodyInit.make(Js.Json.stringify(User.toAPI(user))),
        ~headers=Fetch.HeadersInit.make({"Content-Type": "application/json"}),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  }
  |> ignore;
};

let setItem = (~item: User.item) => {
  let user = Option.getExn(api.getState());
  let updatedUser = {
    ...user,
    items:
      switch (
        user.items
        ->Array.getIndexBy(iter => {
            iter.itemId == item.itemId && iter.variation == item.variation
          })
      ) {
      | Some(index) =>
        Array.concatMany([|
          user.items->Array.slice(~offset=0, ~len=index),
          [|item|],
          user.items->Array.sliceToEnd(index + 1),
        |])
      | None => Array.concat(user.items, [|item|])
      },
  };
  api.dispatch(UpdateUser(updatedUser));
  postUpdatedUser(~user=updatedUser);
};

let removeItem = (~itemId, ~variation) => {
  let user = Option.getExn(api.getState());
  switch (
    user.items
    ->Array.getIndexBy(iter => {
        iter.itemId == itemId && iter.variation == variation
      })
  ) {
  | Some(index) =>
    let updatedUser = {
      ...user,
      items:
        Array.concatMany([|
          user.items->Array.slice(~offset=0, ~len=index),
          user.items->Array.sliceToEnd(index + 1),
        |]),
    };
    api.dispatch(UpdateUser(updatedUser));
    postUpdatedUser(~user=updatedUser);
  | None => ()
  };
};

let login = (~userId, ~password) => {
  let%Repromise.JsExn response =
    Fetch.fetchWithInit(
      Constants.apiUrl ++ "/login",
      Fetch.RequestInit.make(
        ~method_=Post,
        ~body=
          Fetch.BodyInit.make(
            Js.Json.stringify(
              Js.Json.object_(
                Js.Dict.fromArray([|
                  ("userId", Js.Json.string(userId)),
                  ("password", Js.Json.string(password)),
                |]),
              ),
            ),
          ),
        ~headers=Fetch.HeadersInit.make({"Content-Type": "application/json"}),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  switch (Fetch.Response.status(response)) {
  | 200 =>
    let%Repromise.JsExn json = Fetch.Response.json(response);
    let user = User.fromAPI(json);
    api.dispatch(Login(user));
    Promise.resolved(Ok(user));
  | _ => Promise.resolved(Error())
  };
};
let logout = () => {
  api.dispatch(Logout);
  let%Repromise.JsExn response =
    Fetch.fetchWithInit(
      Constants.apiUrl ++ "/logout",
      Fetch.RequestInit.make(
        ~method_=Post,
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  Promise.resolved();
};

let init = () => {
  {
    let%Repromise.JsExn response =
      Fetch.fetchWithInit(
        Constants.apiUrl ++ "/me",
        Fetch.RequestInit.make(
          ~method_=Get,
          ~credentials=Include,
          ~mode=CORS,
          (),
        ),
      );
    switch (Fetch.Response.status(response)) {
    | 200 =>
      let%Repromise.JsExn json = Fetch.Response.json(response);
      let user = User.fromAPI(json);
      api.dispatch(Login(user));
      Promise.resolved();
    | _ => Promise.resolved()
    };
  }
  |> ignore;
};