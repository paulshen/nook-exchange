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
          user.items->Js.Dict.get(User.getItemKey(~itemId, ~variation))
        )
      },
      (itemId, variation),
    );
  api.useStoreWithSelector(selector, ());
};

let isLoggedIn = () => api.getState() != None;

let setItem = (~itemId: string, ~variation: int, ~item: User.item) => {
  let user = Option.getExn(api.getState());
  let updatedUser = {
    ...user,
    items: {
      let clone = Utils.cloneJsDict(user.items);
      clone->Js.Dict.set(User.getItemKey(~itemId, ~variation), item);
      clone;
    },
  };
  api.dispatch(UpdateUser(updatedUser));
  {
    Fetch.fetchWithInit(
      Constants.apiUrl
      ++ "/@me/items/"
      ++ itemId
      ++ "/"
      ++ string_of_int(variation),
      Fetch.RequestInit.make(
        ~method_=Post,
        ~body=Fetch.BodyInit.make(Js.Json.stringify(User.itemToJson(item))),
        ~headers=Fetch.HeadersInit.make({"Content-Type": "application/json"}),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  }
  |> ignore;
};

let removeItem = (~itemId, ~variation) => {
  let user = Option.getExn(api.getState());
  let key = User.getItemKey(~itemId, ~variation);
  if (user.items->Js.Dict.get(key)->Option.isSome) {
    let updatedUser = {
      ...user,
      items: {
        let clone = Utils.cloneJsDict(user.items);
        Utils.deleteJsDictKey(clone, key);
        clone;
      },
    };
    api.dispatch(UpdateUser(updatedUser));
    {
      Fetch.fetchWithInit(
        Constants.apiUrl
        ++ "/@me/items/"
        ++ itemId
        ++ "/"
        ++ string_of_int(variation),
        Fetch.RequestInit.make(
          ~method_=Delete,
          ~credentials=Include,
          ~mode=CORS,
          (),
        ),
      );
    }
    |> ignore;
  };
};

let register = (~userId, ~password) => {
  let%Repromise.JsExn response =
    Fetch.fetchWithInit(
      Constants.apiUrl ++ "/register",
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
  | _ =>
    let%Repromise.JsExn text = Fetch.Response.text(response);
    Promise.resolved(Error(text));
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
        Constants.apiUrl ++ "/@me",
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