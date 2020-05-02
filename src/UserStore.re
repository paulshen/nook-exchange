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
      | Ok(response) => Fetch.Response.status(response) != 200
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

exception NotCanonicalVariant(string, int);
let numItemUpdatesLogged = ref(0);
let setItemStatus =
    (~itemId: string, ~variation: int, ~status: User.itemStatus) => {
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
    | None => {status, note: "", timeUpdated}
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
  if (status == CatalogOnly
      || numItemUpdatesLogged^ < 5
      || updatedUser.items->Js.Dict.keys->Js.Array.length < 10) {
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Item Status Updated",
      ~eventProperties={
        "itemId": item.id,
        "variant": variation,
        "status": User.itemStatusToJs(status),
      },
    );
    numItemUpdatesLogged := numItemUpdatesLogged^ + 1;
  };
  Analytics.Amplitude.setItemCount(
    ~itemCount=Js.Dict.keys(updatedUser.items)->Js.Array.length,
  );
  {
    let url =
      Constants.apiUrl
      ++ "/@me4/items/"
      ++ item.id
      ++ "/"
      ++ string_of_int(variation)
      ++ "/status";
    let%Repromise.Js responseResult =
      Fetch.fetchWithInit(
        url,
        Fetch.RequestInit.make(
          ~method_=Post,
          ~body=
            Fetch.BodyInit.make(
              Js.Json.stringify(
                Json.Encode.object_([
                  ("status", Json.Encode.int(User.itemStatusToJs(status))),
                ]),
              ),
            ),
          ~headers=
            Fetch.HeadersInit.make({
              "X-Client-Version": Constants.gitCommitRef,
              "Content-Type": "application/json",
              "Authorization":
                "Bearer " ++ Option.getWithDefault(sessionId^, ""),
            }),
          ~credentials=Include,
          ~mode=CORS,
          (),
        ),
      );
    handleServerResponse(url, responseResult);
    BAPI.setItemStatus(
      ~userId=user.id,
      ~sessionId=sessionId^,
      ~itemId,
      ~variant=variation,
      ~status,
    )
    |> ignore;
    Promise.resolved();
  }
  |> ignore;
};

let setItemStatusBatch =
    (~itemId: string, ~variations: array(int), ~status: User.itemStatus) => {
  let user = getUser();
  let updatedItems = {
    let clone = Utils.cloneJsDict(user.items);
    variations
    |> Js.Array.forEach(variant => {
         let item = Item.getItem(~itemId);
         if (Item.getCanonicalVariant(~item, ~variant) != variant) {
           raise(NotCanonicalVariant(itemId, variant));
         };
         let itemKey = User.getItemKey(~itemId, ~variation=variant);
         let timeUpdated = Some(Js.Date.now() /. 1000.);
         let userItem =
           switch (user.items->Js.Dict.get(itemKey)) {
           | Some(item) => {...item, status, timeUpdated}
           | None => {status, note: "", timeUpdated}
           };
         clone->Js.Dict.set(itemKey, userItem);
       });
    clone;
  };
  let updatedUser = {...user, items: updatedItems};
  api.dispatch(UpdateUser(updatedUser));
  let item = Item.getItem(~itemId);
  if (numItemUpdatesLogged^ < 5
      || updatedUser.items->Js.Dict.keys->Js.Array.length < 10) {
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Item Status Updated",
      ~eventProperties={
        "itemId": item.id,
        "variants": variations,
        "status": User.itemStatusToJs(status),
      },
    );
    numItemUpdatesLogged := numItemUpdatesLogged^ + 1;
  };
  Analytics.Amplitude.setItemCount(
    ~itemCount=Js.Dict.keys(updatedUser.items)->Js.Array.length,
  );
  {
    let url = Constants.apiUrl ++ "/@me4/items/" ++ item.id ++ "/batch/status";
    let%Repromise.Js responseResult =
      Fetch.fetchWithInit(
        url,
        Fetch.RequestInit.make(
          ~method_=Post,
          ~body=
            Fetch.BodyInit.make(
              Js.Json.stringify(
                Json.Encode.object_([
                  ("status", Json.Encode.int(User.itemStatusToJs(status))),
                  (
                    "variants",
                    Json.Encode.array(
                      Json.Encode.string,
                      variations->Belt.Array.map(string_of_int),
                    ),
                  ),
                ]),
              ),
            ),
          ~headers=
            Fetch.HeadersInit.make({
              "X-Client-Version": Constants.gitCommitRef,
              "Content-Type": "application/json",
              "Authorization":
                "Bearer " ++ Option.getWithDefault(sessionId^, ""),
            }),
          ~credentials=Include,
          ~mode=CORS,
          (),
        ),
      );
    handleServerResponse(url, responseResult);
    BAPI.setItemStatusBatch(
      ~userId=user.id,
      ~sessionId=sessionId^,
      ~itemId,
      ~variants=variations,
      ~status,
    )
    |> ignore;
    Promise.resolved();
  }
  |> ignore;
};

let setItemNote = (~itemId: string, ~variation: int, ~note: string) => {
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
  if (numItemUpdatesLogged^ < 5
      || updatedUser.items->Js.Dict.keys->Js.Array.length < 10) {
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
  {
    let url =
      Constants.apiUrl
      ++ "/@me4/items/"
      ++ item.id
      ++ "/"
      ++ string_of_int(variation)
      ++ "/note";
    let%Repromise.Js responseResult =
      Fetch.fetchWithInit(
        url,
        Fetch.RequestInit.make(
          ~method_=Post,
          ~body=
            Fetch.BodyInit.make(
              Js.Json.stringify(
                Json.Encode.object_([("note", Json.Encode.string(note))]),
              ),
            ),
          ~headers=
            Fetch.HeadersInit.make({
              "X-Client-Version": Constants.gitCommitRef,
              "Content-Type": "application/json",
              "Authorization":
                "Bearer " ++ Option.getWithDefault(sessionId^, ""),
            }),
          ~credentials=Include,
          ~mode=CORS,
          (),
        ),
      );
    handleServerResponse(url, responseResult);
    BAPI.setItemNote(
      ~userId=user.id,
      ~sessionId=sessionId^,
      ~itemId,
      ~variant=variation,
      ~note,
    )
    |> ignore;
    Promise.resolved();
  }
  |> ignore;
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
    Analytics.Amplitude.setItemCount(
      ~itemCount=Js.Dict.keys(updatedUser.items)->Js.Array.length,
    );
    {
      let url =
        Constants.apiUrl
        ++ "/@me3/items/"
        ++ item.id
        ++ "/"
        ++ string_of_int(variation);
      let%Repromise.Js responseResult =
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
      handleServerResponse(url, responseResult);
      BAPI.removeItem(
        ~userId=user.id,
        ~sessionId=sessionId^,
        ~itemId,
        ~variant=variation,
      )
      |> ignore;
      Promise.resolved();
    }
    |> ignore;
  };
};

let updateProfileText = (~profileText) => {
  let user = getUser();
  let updatedUser = {...user, profileText};
  api.dispatch(UpdateUser(updatedUser));
  Analytics.Amplitude.logEventWithProperties(
    ~eventName="Profile Text Updated",
    ~eventProperties={"text": profileText},
  );
  {
    let url = Constants.apiUrl ++ "/@me/profileText";
    let%Repromise.Js responseResult =
      Fetch.fetchWithInit(
        url,
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
              "X-Client-Version": Constants.gitCommitRef,
              "Content-Type": "application/json",
              "Authorization":
                "Bearer " ++ Option.getWithDefault(sessionId^, ""),
            }),
          ~credentials=Include,
          ~mode=CORS,
          (),
        ),
      );
    handleServerResponse(url, responseResult);
    BAPI.updateProfileText(
      ~userId=user.id,
      ~sessionId=sessionId^,
      ~profileText,
    )
    |> ignore;
    Promise.resolved();
  }
  |> ignore;
};

let patchMe = (~username=?, ~newPassword=?, ~email=?, ~oldPassword, ()) => {
  let url = Constants.apiUrl ++ "/@me";
  let%Repromise.JsExn response =
    Fetch.fetchWithInit(
      url,
      Fetch.RequestInit.make(
        ~method_=Patch,
        ~body=
          Fetch.BodyInit.make(
            Js.Json.stringify(
              Js.Json.object_(
                Js.Dict.fromArray(
                  Belt.Array.keepMap(
                    [|
                      Option.map(username, username =>
                        ("username", Js.Json.string(username))
                      ),
                      Option.map(newPassword, newPassword =>
                        ("password", Js.Json.string(newPassword))
                      ),
                      Option.map(email, email =>
                        ("email", Js.Json.string(email))
                      ),
                      Some(("oldPassword", Js.Json.string(oldPassword))),
                    |],
                    x =>
                    x
                  ),
                ),
              ),
            ),
          ),
        ~headers=
          Fetch.HeadersInit.make({
            "X-Client-Version": Constants.gitCommitRef,
            "Content-Type": "application/json",
            "Authorization":
              "Bearer " ++ Option.getWithDefault(sessionId^, ""),
          }),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  switch (Fetch.Response.status(response)) {
  | 200 =>
    let%Repromise.JsExn json = Fetch.Response.json(response);
    let user = User.fromAPI(json);
    api.dispatch(UpdateUser(user));
    Analytics.Amplitude.logEventWithProperties(
      ~eventName="Account Update Succeeded",
      ~eventProperties={
        "username": username,
        "password": newPassword !== None,
        "email": email,
      },
    );
    BAPI.patchMe(
      ~userId=user.id,
      ~sessionId=sessionId^,
      ~username,
      ~newPassword,
      ~email,
      ~oldPassword,
    )
    |> ignore;
    Promise.resolved(Ok());
  | _ =>
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
  Analytics.Amplitude.logEventWithProperties(
    ~eventName="Catalog Checkbox Setting Toggled",
    ~eventProperties={"enabled": enabled},
  );
  {
    let url = Constants.apiUrl ++ "/@me/toggleCatalogCheckboxSetting";
    let%Repromise.Js responseResult =
      Fetch.fetchWithInit(
        url,
        Fetch.RequestInit.make(
          ~method_=Post,
          ~body=
            Fetch.BodyInit.make(
              Js.Json.stringify(
                Js.Json.object_(
                  Js.Dict.fromArray([|
                    ("enabled", Js.Json.boolean(enabled)),
                  |]),
                ),
              ),
            ),
          ~headers=
            Fetch.HeadersInit.make({
              "X-Client-Version": Constants.gitCommitRef,
              "Content-Type": "application/json",
              "Authorization":
                "Bearer " ++ Option.getWithDefault(sessionId^, ""),
            }),
          ~credentials=Include,
          ~mode=CORS,
          (),
        ),
      );
    handleServerResponse(url, responseResult);
    BAPI.updateSetting(
      ~userId=user.id,
      ~sessionId=sessionId^,
      ~settingKey="enableCatalog",
      ~settingValue=Js.Json.boolean(enabled),
    )
    |> ignore;
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
  switch (Fetch.Response.status(response)) {
  | 200 =>
    let%Repromise.JsExn json = Fetch.Response.json(response);
    updateSessionId(
      json |> Json.Decode.(optional(field("sessionId", string))),
    );
    let user = User.fromAPI(json);
    api.dispatch(Login(user));
    Analytics.Amplitude.setUserId(~userId=Some(user.id));
    Analytics.Amplitude.setUsername(~username, ~email);
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
      Constants.apiUrl ++ "/login2",
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
  ReasonReactRouter.push("/");
  let%Repromise.JsExn response =
    Fetch.fetchWithInit(
      Constants.apiUrl ++ "/logout",
      Fetch.RequestInit.make(
        ~method_=Post,
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
};

let init = () => {
  {
    let%Repromise.JsExn response =
      Fetch.fetchWithInit(
        Constants.apiUrl ++ "/@me2",
        Fetch.RequestInit.make(
          ~method_=Get,
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
    switch (Fetch.Response.status(response)) {
    | 200 =>
      let%Repromise.JsExn json = Fetch.Response.json(response);
      updateSessionId(
        json |> Json.Decode.(optional(field("sessionId", string))),
      );
      let user = User.fromAPI(json);
      api.dispatch(Login(user));
      Analytics.Amplitude.setUserId(~userId=Some(user.id));
      Promise.resolved();
    | _ =>
      api.dispatch(FetchMeFailed);
      Promise.resolved();
    };
  }
  |> ignore;
};