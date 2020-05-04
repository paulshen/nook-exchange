open Belt;

type state =
  | Loading
  | NotLoggedIn
  | LoggedIn(User.t);
type action =
  | Login(User.t)
  | UpdateUser(User.t)
  | Logout
  | FetchMeFailed;

let api =
  Restorative.createStore(Loading, (state, action) => {
    switch (action) {
    | Login(user) => LoggedIn(user)
    | UpdateUser(user) => LoggedIn(user)
    | Logout => NotLoggedIn
    | FetchMeFailed => NotLoggedIn
    }
  });

let useStore = api.useStore;
let useMe = () =>
  switch (api.useStore()) {
  | LoggedIn(user) => Some(user)
  | _ => None
  };
let useItem = (~itemId, ~variation) => {
  let selector =
    React.useCallback2(
      (state: state) => {
        switch (state) {
        | LoggedIn(user) =>
          user.items->Js.Dict.get(User.getItemKey(~itemId, ~variation))
        | _ => None
        }
      },
      (itemId, variation),
    );
  api.useStoreWithSelector(selector, ());
};
let useIsLoggedIn = () => {
  api.useStoreWithSelector(
    state =>
      switch (state) {
      | LoggedIn(_) => true
      | _ => false
      },
    (),
  );
};
let useEnableCatalogCheckbox = () => {
  api.useStoreWithSelector(
    state =>
      switch (state) {
      | LoggedIn(user) => user.enableCatalogCheckbox
      | _ => false
      },
    (),
  );
};
exception ExpectedUser;
let getUser = () => {
  switch (api.getState()) {
  | LoggedIn(user) => user
  | _ => raise(ExpectedUser)
  };
};
let getItem = (~itemId, ~variation) => {
  switch (api.getState()) {
  | LoggedIn(user) =>
    user.items->Js.Dict.get(User.getItemKey(~itemId, ~variation))
  | _ => None
  };
};

let isLoggedIn = () =>
  switch (api.getState()) {
  | LoggedIn(_) => true
  | _ => false
  };

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

let handleServerResponse = (url, responseResult) =>
  if (switch (responseResult) {
      | Error(_) => true
      | Ok(response) => Fetch.Response.status(response) >= 400
      }) {
    Error.showPopup(
      ~message=
        "Something went wrong. Sorry!\nRefresh your browser and try again.\nIf you are running into issues, please email hi@nook.exchange",
    );
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Error Dialog Shown",
      ~eventProperties={
        "url": url,
        "errorResponse":
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

let makeAuthenticatedPostRequest = (~url, ~bodyJson) => {
  Fetch.fetchWithInit(
    url,
    Fetch.RequestInit.make(
      ~method_=Post,
      ~body=
        Fetch.BodyInit.make(
          Js.Json.stringify(Json.Encode.object_(bodyJson)),
        ),
      ~headers=
        Fetch.HeadersInit.make({
          "X-Client-Version": Constants.gitCommitRef,
          "Content-Type": "application/json",
          "Authorization": "Bearer " ++ Option.getWithDefault(sessionId^, ""),
        }),
      ~credentials=Include,
      ~mode=CORS,
      (),
    ),
  );
};

exception NotCanonicalVariant(int, int);
let numItemUpdatesLogged = ref(0);
let setItemStatus = (~itemId: int, ~variation: int, ~status: User.itemStatus) => {
  let item = Item.getItem(~itemId);
  if (Item.getCanonicalVariant(~item, ~variant=variation) != variation) {
    raise(NotCanonicalVariant(itemId, variation));
  };
  let user = getUser();
  let itemKey = User.getItemKey(~itemId, ~variation);
  let timeUpdated = Some(Js.Date.now() /. 1000.);
  let userItem =
    switch (user.items->Js.Dict.get(itemKey)) {
    | Some(item) => {...item, status, timeUpdated}
    | None => {status, note: "", priorityTimestamp: None, timeUpdated}
    };
  let updatedUser = {
    ...user,
    items: {
      let clone = Utils.cloneJsDict(user.items);
      clone->Js.Dict.set(itemKey, userItem);
      clone;
    },
  };
  api.dispatch(UpdateUser(updatedUser));
  {
    let%Repromise responseResult =
      BAPI.setItemStatus(
        ~userId=user.id,
        ~sessionId=sessionId^,
        ~itemId,
        ~variant=variation,
        ~status,
      );
    handleServerResponse("/@me/items/status", responseResult);
    Promise.resolved();
  }
  |> ignore;
  {
    let%Repromise.Js _responseResult =
      makeAuthenticatedPostRequest(
        ~url=
          Constants.apiUrl
          ++ "/@me5/items/"
          ++ string_of_int(item.id)
          ++ "/"
          ++ string_of_int(variation)
          ++ "/status",
        ~bodyJson=[
          ("status", Json.Encode.int(User.itemStatusToJs(status))),
        ],
      );
    Promise.resolved();
  }
  |> ignore;
  if (numItemUpdatesLogged^ < 2
      || updatedUser.items->Js.Dict.keys->Js.Array.length < 4) {
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Item Status Updated",
      ~eventProperties={
        "itemId": itemId,
        "variant": variation,
        "status": User.itemStatusToJs(status),
      },
    );
    numItemUpdatesLogged := numItemUpdatesLogged^ + 1;
  };
  Analytics.Amplitude.setItemCount(
    ~itemCount=Js.Dict.keys(updatedUser.items)->Js.Array.length,
  );
};

let setItemNote = (~itemId: int, ~variation: int, ~note: string) => {
  let item = Item.getItem(~itemId);
  if (Item.getCanonicalVariant(~item, ~variant=variation) != variation) {
    raise(NotCanonicalVariant(itemId, variation));
  };
  let user = getUser();
  let itemKey = User.getItemKey(~itemId, ~variation);
  let userItem = {
    ...Belt.Option.getExn(user.items->Js.Dict.get(itemKey)),
    note,
  };
  let updatedUser = {
    ...user,
    items: {
      let clone = Utils.cloneJsDict(user.items);
      clone->Js.Dict.set(itemKey, userItem);
      clone;
    },
  };
  api.dispatch(UpdateUser(updatedUser));
  let item = Item.getItem(~itemId);
  {
    let%Repromise responseResult =
      BAPI.setItemNote(
        ~userId=user.id,
        ~sessionId=sessionId^,
        ~itemId,
        ~variant=variation,
        ~note,
      );
    handleServerResponse("/@me/items/note", responseResult);
    Promise.resolved();
  }
  |> ignore;
  {
    let%Repromise.Js _responseResult =
      makeAuthenticatedPostRequest(
        ~url=
          Constants.apiUrl
          ++ "/@me5/items/"
          ++ string_of_int(item.id)
          ++ "/"
          ++ string_of_int(variation)
          ++ "/note",
        ~bodyJson=[("note", Json.Encode.string(note))],
      );
    Promise.resolved();
  }
  |> ignore;
  if (numItemUpdatesLogged^ < 2
      || updatedUser.items->Js.Dict.keys->Js.Array.length < 4) {
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Item Note Updated",
      ~eventProperties={
        "itemId": item.id,
        "variant": variation,
        "note": note,
      },
    );
    numItemUpdatesLogged := numItemUpdatesLogged^ + 1;
  };
};

let setItemPriorityTimestamp =
    (~itemId: int, ~variation: int, ~priorityTimestamp: option(float)) => {
  ();
    //paul will do
};

let numItemRemovesLogged = ref(0);
let removeItem = (~itemId, ~variation) => {
  let user = getUser();
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
    let item = Item.getItem(~itemId);
    if (numItemRemovesLogged^ < 3) {
      Analytics.Amplitude.logEventWithProperties(
        ~eventName="Item Removed",
        ~eventProperties={"itemId": item.id, "variant": variation},
      );
      numItemRemovesLogged := numItemRemovesLogged^ + 1;
    };
    {
      let%Repromise responseResult =
        BAPI.removeItem(
          ~userId=user.id,
          ~sessionId=sessionId^,
          ~itemId,
          ~variant=variation,
        );
      handleServerResponse("/@me/items/remove", responseResult);
      Promise.resolved();
    }
    |> ignore;
    {
      let url =
        Constants.apiUrl
        ++ "/@me3/items/"
        ++ string_of_int(item.id)
        ++ "/"
        ++ string_of_int(variation);
      let%Repromise.Js _responseResult =
        Fetch.fetchWithInit(
          url,
          Fetch.RequestInit.make(
            ~method_=Delete,
            ~headers=?
              Option.map(sessionId^, sessionId =>
                Fetch.HeadersInit.make({
                  "X-Client-Version": Constants.gitCommitRef,
                  "Authorization": "Bearer " ++ sessionId,
                })
              ),
            ~credentials=Include,
            ~mode=CORS,
            (),
          ),
        );
      Promise.resolved();
    }
    |> ignore;
    Analytics.Amplitude.setItemCount(
      ~itemCount=Js.Dict.keys(updatedUser.items)->Js.Array.length,
    );
  };
};

let updateProfileText = (~profileText) => {
  let user = getUser();
  let updatedUser = {...user, profileText};
  api.dispatch(UpdateUser(updatedUser));
  {
    let%Repromise responseResult =
      BAPI.updateProfileText(
        ~userId=user.id,
        ~sessionId=sessionId^,
        ~profileText,
      );
    handleServerResponse("/@me/profileText", responseResult);
    Promise.resolved();
  }
  |> ignore;
  Analytics.Amplitude.logEventWithProperties(
    ~eventName="Profile Text Updated",
    ~eventProperties={"text": profileText},
  );
};

let patchMe = (~username=?, ~newPassword=?, ~email=?, ~oldPassword, ()) => {
  let user = getUser();
  let%Repromise response =
    BAPI.patchMe(
      ~userId=user.id,
      ~sessionId=sessionId^,
      ~username,
      ~newPassword,
      ~email,
      ~oldPassword,
    );
  if (Fetch.Response.status(response) < 300) {
    let user = getUser();
    let updatedUser = {
      ...user,
      username: username->Belt.Option.getWithDefault(user.username),
      email:
        switch (email) {
        | Some(email) => email != "" ? Some(email) : None
        | None => user.email
        },
    };
    api.dispatch(UpdateUser(updatedUser));
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Account Update Succeeded",
      ~eventProperties={
        "username": username,
        "password": newPassword !== None,
        "email": email,
      },
    );
    Promise.resolved(Ok());
  } else {
    let%Repromise.JsExn error = Fetch.Response.text(response);
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Account Update Failed",
      ~eventProperties={"error": error},
    );
    Promise.resolved(Error(error));
  };
};

let toggleCatalogCheckboxSetting = (~enabled) => {
  let user = getUser();
  let updatedUser = {...user, enableCatalogCheckbox: enabled};
  api.dispatch(UpdateUser(updatedUser));
  {
    let%Repromise responseResult =
      BAPI.updateSetting(
        ~userId=user.id,
        ~sessionId=sessionId^,
        ~settingKey="enableCatalog",
        ~settingValue=Js.Json.boolean(enabled),
      );
    handleServerResponse("/@me/toggleCatalogCheckboxSetting", responseResult);
    Promise.resolved();
  }
  |> ignore;
  Analytics.Amplitude.logEventWithProperties(
    ~eventName="Catalog Checkbox Setting Toggled",
    ~eventProperties={"enabled": enabled},
  );
};

let errorQuotationMarksRegex = [%bs.re {|/^"(.*)"$/|}];
let register = (~username, ~email, ~password) => {
  let%Repromise.JsExn response =
    Fetch.fetchWithInit(
      Constants.bapiUrl ++ "/register2",
      Fetch.RequestInit.make(
        ~method_=Post,
        ~body=
          Fetch.BodyInit.make(
            Js.Json.stringify(
              Json.Encode.object_([
                ("username", Js.Json.string(username)),
                ("email", Js.Json.string(email)),
                ("password", Js.Json.string(password)),
              ]),
            ),
          ),
        ~headers=
          Fetch.HeadersInit.make({
            "X-Client-Version": Constants.gitCommitRef,
            "Content-Type": "application/json",
          }),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  if (Fetch.Response.status(response) < 300) {
    let%Repromise.JsExn json = Fetch.Response.json(response);
    updateSessionId(
      json |> Json.Decode.(optional(field("sessionId", string))),
    );
    let user = User.fromAPI(json);
    api.dispatch(Login(user));
    Analytics.Amplitude.setUserId(~userId=Some(user.id));
    Analytics.Amplitude.setUsername(~username, ~email);
    Promise.resolved(Ok(user));
  } else {
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
      Constants.bapiUrl ++ "/login",
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
        ~headers=
          Fetch.HeadersInit.make({
            "X-Client-Version": Constants.gitCommitRef,
            "Content-Type": "application/json",
          }),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  if (Fetch.Response.status(response) < 300) {
    let%Repromise.JsExn json = Fetch.Response.json(response);
    updateSessionId(
      json |> Json.Decode.(optional(field("sessionId", string))),
    );
    let user = User.fromAPI(json);
    api.dispatch(Login(user));
    Analytics.Amplitude.setUserId(~userId=Some(user.id));
    Promise.resolved(Ok(user));
  } else {
    Promise.resolved(Error());
  };
};

let logout = () => {
  api.dispatch(Logout);
  Dom.Storage.localStorage |> Dom.Storage.removeItem("sessionId");
  Analytics.Amplitude.setUserId(~userId=None);
  ReasonReactRouter.push("/");
  let sessionId = sessionId^;
  updateSessionId(None);
  let%Repromise.JsExn response =
    Fetch.fetchWithInit(
      Constants.bapiUrl ++ "/logout",
      Fetch.RequestInit.make(
        ~method_=Post,
        ~headers=?
          Option.map(sessionId, sessionId =>
            Fetch.HeadersInit.make({
              "X-Client-Version": Constants.gitCommitRef,
              "Authorization": "Bearer " ++ sessionId,
            })
          ),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  Promise.resolved();
};

let init = () => {
  switch (sessionId^) {
  | Some(sessionId) =>
    {
      let%Repromise.JsExn response =
        Fetch.fetchWithInit(
          Constants.bapiUrl ++ "/@me",
          Fetch.RequestInit.make(
            ~method_=Get,
            ~headers=
              Fetch.HeadersInit.make({
                "X-Client-Version": Constants.gitCommitRef,
                "Authorization": "Bearer " ++ sessionId,
              }),
            ~credentials=Include,
            ~mode=CORS,
            (),
          ),
        );
      if (Fetch.Response.status(response) < 400) {
        let%Repromise.JsExn json = Fetch.Response.json(response);
        updateSessionId(
          json |> Json.Decode.(optional(field("sessionId", string))),
        );
        let user = User.fromAPI(json);
        api.dispatch(Login(user));
        Analytics.Amplitude.setUserId(~userId=Some(user.id));
        Promise.resolved();
      } else {
        api.dispatch(FetchMeFailed);
        Promise.resolved();
      };
    }
    |> ignore
  | None => ()
  };
};