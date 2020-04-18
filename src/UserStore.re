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

let sessionId =
  ref(Dom.Storage.localStorage |> Dom.Storage.getItem("sessionId"));
let updateSessionId = newValue => {
  sessionId := newValue;
  Dom.Storage.(
    switch (newValue) {
    | Some(sessionId) => localStorage |> setItem("sessionId", sessionId)
    | None => localStorage |> removeItem("sessionId")
    }
  );
};

let handleServerResponse = responseResult =>
  if (switch (responseResult) {
      | Error(_) => true
      | Ok(response) => Fetch.Response.status(response) != 200
      }) {
    Error.showPopup(
      ~message=
        "Something went wrong. Sorry!\nRefresh your browser and try again.\nIf you are running into issues, please email hi@nook.exchange",
    );
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Error Dialog Shown",
      ~eventProperties={
        "error_response":
          Js.Json.stringifyAny(
            switch (responseResult) {
            | Ok(response) => {
                "status": Fetch.Response.status(response),
                "statusText": Fetch.Response.statusText(response),
              }
            | Error(error) => Obj.magic(error)
            },
          )
          ->Option.getExn,
      },
    );
  };

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
  let userItemJson = User.itemToJson(item);
  Analytics.Amplitude.logEventWithProperties(
    ~eventName="Item Updated",
    ~eventProperties={
      "itemId": itemId,
      "variation": variation,
      "data": userItemJson,
    },
  );
  {
    let%Repromise.Js responseResult =
      Fetch.fetchWithInit(
        Constants.apiUrl
        ++ "/@me/items/"
        ++ itemId
        ++ "/"
        ++ string_of_int(variation),
        Fetch.RequestInit.make(
          ~method_=Post,
          ~body=Fetch.BodyInit.make(Js.Json.stringify(userItemJson)),
          ~headers=
            Fetch.HeadersInit.make({
              "Content-Type": "application/json",
              "Authorization":
                "Bearer " ++ Option.getWithDefault(sessionId^, ""),
            }),
          ~credentials=Include,
          ~mode=CORS,
          (),
        ),
      );
    handleServerResponse(responseResult);
    Promise.resolved();
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
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Item Removed",
      ~eventProperties={"itemId": itemId, "variation": variation},
    );
    {
      let%Repromise.Js responseResult =
        Fetch.fetchWithInit(
          Constants.apiUrl
          ++ "/@me/items/"
          ++ itemId
          ++ "/"
          ++ string_of_int(variation),
          Fetch.RequestInit.make(
            ~method_=Delete,
            ~headers=?
              Option.map(sessionId^, sessionId =>
                Fetch.HeadersInit.make({
                  "Authorization": "Bearer " ++ sessionId,
                })
              ),
            ~credentials=Include,
            ~mode=CORS,
            (),
          ),
        );
      handleServerResponse(responseResult);
      Promise.resolved();
    }
    |> ignore;
  };
};

let updateProfileText = (~profileText) => {
  let user = Option.getExn(api.getState());
  let updatedUser = {...user, profileText};
  api.dispatch(UpdateUser(updatedUser));
  Analytics.Amplitude.logEventWithProperties(
    ~eventName="Profile Text Updated",
    ~eventProperties={"text": profileText},
  );
  {
    let%Repromise.Js responseResult =
      Fetch.fetchWithInit(
        Constants.apiUrl ++ "/@me/profileText",
        Fetch.RequestInit.make(
          ~method_=Post,
          ~body=
            Fetch.BodyInit.make(
              Js.Json.stringify(
                Js.Json.object_(
                  Js.Dict.fromArray([|
                    ("text", Js.Json.string(profileText)),
                  |]),
                ),
              ),
            ),
          ~headers=
            Fetch.HeadersInit.make({
              "Content-Type": "application/json",
              "Authorization":
                "Bearer " ++ Option.getWithDefault(sessionId^, ""),
            }),
          ~credentials=Include,
          ~mode=CORS,
          (),
        ),
      );
    handleServerResponse(responseResult);
    Promise.resolved();
  }
  |> ignore;
};

let errorQuotationMarksRegex = [%bs.re {|/^"(.*)"$/|}];
let register = (~username, ~email, ~password) => {
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
                  ("username", Js.Json.string(username)),
                  ("email", Js.Json.string(email)),
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
    updateSessionId(
      json |> Json.Decode.(optional(field("sessionId", string))),
    );
    let user = User.fromAPI(json);
    api.dispatch(Login(user));
    Analytics.Amplitude.setUserId(~userId=Some(user.id));
    Analytics.Amplitude.setUsername(~username);
    Promise.resolved(Ok(user));
  | _ =>
    let%Repromise.JsExn text = Fetch.Response.text(response);
    let result = text |> Js.Re.exec_(errorQuotationMarksRegex);
    let text =
      switch (result) {
      | Some(match) =>
        let captures = Js.Re.captures(match);
        captures[1]->Option.getExn->Js.Nullable.toOption->Option.getExn;
      | None => text
      };
    Promise.resolved(Error(text));
  };
};

let login = (~username, ~password) => {
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
                  ("username", Js.Json.string(username)),
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
    updateSessionId(
      json |> Json.Decode.(optional(field("sessionId", string))),
    );
    let user = User.fromAPI(json);
    api.dispatch(Login(user));
    Analytics.Amplitude.setUserId(~userId=Some(user.id));
    Promise.resolved(Ok(user));
  | _ => Promise.resolved(Error())
  };
};

let logout = () => {
  api.dispatch(Logout);
  Dom.Storage.localStorage |> Dom.Storage.removeItem("sessionId");
  Analytics.Amplitude.setUserId(~userId=None);
  updateSessionId(None);
  let%Repromise.JsExn response =
    Fetch.fetchWithInit(
      Constants.apiUrl ++ "/logout",
      Fetch.RequestInit.make(
        ~method_=Post,
        ~headers=?
          Option.map(sessionId^, sessionId =>
            Fetch.HeadersInit.make({"Authorization": "Bearer " ++ sessionId})
          ),
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
          ~headers=?
            Option.map(sessionId^, sessionId =>
              Fetch.HeadersInit.make({
                "Authorization": "Bearer " ++ sessionId,
              })
            ),
          ~credentials=Include,
          ~mode=CORS,
          (),
        ),
      );
    switch (Fetch.Response.status(response)) {
    | 200 =>
      let%Repromise.JsExn json = Fetch.Response.json(response);
      updateSessionId(
        json |> Json.Decode.(optional(field("sessionId", string))),
      );
      let user = User.fromAPI(json);
      api.dispatch(Login(user));
      Analytics.Amplitude.setUserId(~userId=Some(user.id));
      Analytics.Amplitude.setUsername(~username=user.username);
      Promise.resolved();
    | _ => Promise.resolved()
    };
  }
  |> ignore;
};