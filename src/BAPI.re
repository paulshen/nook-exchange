let makeAuthenticatedPostRequest = (~url, ~bodyJson, ~sessionId) => {
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
          "Authorization":
            "Bearer " ++ Belt.Option.getWithDefault(sessionId, ""),
        }),
      ~credentials=Include,
      ~mode=CORS,
      (),
    ),
  );
};

let setItemStatus = (~userId, ~sessionId, ~itemId, ~variant, ~status) => {
  let%Repromise.Js responseResult =
    makeAuthenticatedPostRequest(
      ~url=
        Constants.bapiUrl
        ++ "/@me/items/"
        ++ string_of_int(itemId)
        ++ "/"
        ++ string_of_int(variant)
        ++ "/status",
      ~bodyJson=[
        ("status", Json.Encode.int(User.itemStatusToJs(status))),
        ("userId", Json.Encode.string(userId)),
      ],
      ~sessionId,
    );
  Promise.resolved(responseResult);
};

let setItemNote = (~userId, ~sessionId, ~itemId, ~variant, ~note) => {
  let%Repromise.Js responseResult =
    makeAuthenticatedPostRequest(
      ~url=
        Constants.bapiUrl
        ++ "/@me/items/"
        ++ string_of_int(itemId)
        ++ "/"
        ++ string_of_int(variant)
        ++ "/note",
      ~bodyJson=[
        ("note", Json.Encode.string(note)),
        ("userId", Json.Encode.string(userId)),
      ],
      ~sessionId,
    );
  Promise.resolved(responseResult);
};

let setItemPriority = (~sessionId, ~itemId, ~variant, ~isPriority) => {
  let%Repromise.Js responseResult =
    makeAuthenticatedPostRequest(
      ~url=
        Constants.bapiUrl
        ++ "/@me/items/"
        ++ string_of_int(itemId)
        ++ "/"
        ++ string_of_int(variant)
        ++ "/priority",
      ~bodyJson=[("isPriority", Json.Encode.bool(isPriority))],
      ~sessionId,
    );
  Promise.resolved(responseResult);
};

let removeItem = (~userId, ~sessionId, ~itemId, ~variant) => {
  let url =
    Constants.bapiUrl
    ++ "/@me/items/"
    ++ string_of_int(itemId)
    ++ "/"
    ++ string_of_int(variant);
  let%Repromise.Js responseResult =
    Fetch.fetchWithInit(
      url,
      Fetch.RequestInit.make(
        ~method_=Delete,
        ~body=
          Fetch.BodyInit.make(
            Js.Json.stringify(
              Json.Encode.object_([("userId", Json.Encode.string(userId))]),
            ),
          ),
        ~headers=
          Fetch.HeadersInit.make({
            "X-Client-Version": Constants.gitCommitRef,
            "Content-Type": "application/json",
            "Authorization":
              "Bearer " ++ Belt.Option.getWithDefault(sessionId, ""),
          }),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  Promise.resolved(responseResult);
};

let updateProfileText = (~userId, ~sessionId, ~profileText) => {
  let%Repromise.Js responseResult =
    makeAuthenticatedPostRequest(
      ~url=Constants.bapiUrl ++ "/@me/profileText",
      ~bodyJson=[
        ("text", Json.Encode.string(profileText)),
        ("userId", Json.Encode.string(userId)),
      ],
      ~sessionId,
    );
  Promise.resolved(responseResult);
};

let updateSetting = (~userId, ~sessionId, ~settingKey, ~settingValue) => {
  let url = Constants.bapiUrl ++ "/@me/settings";
  let%Repromise.Js responseResult =
    Fetch.fetchWithInit(
      url,
      Fetch.RequestInit.make(
        ~method_=Patch,
        ~body=
          Fetch.BodyInit.make(
            Js.Json.stringify(
              Json.Encode.object_([
                ("key", Json.Encode.string(settingKey)),
                ("value", settingValue),
                ("userId", Json.Encode.string(userId)),
              ]),
            ),
          ),
        ~headers=
          Fetch.HeadersInit.make({
            "X-Client-Version": Constants.gitCommitRef,
            "Content-Type": "application/json",
            "Authorization":
              "Bearer " ++ Belt.Option.getWithDefault(sessionId, ""),
          }),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  Promise.resolved(responseResult);
};

let patchMe =
    (~userId, ~sessionId, ~username, ~newPassword, ~email, ~oldPassword) => {
  open Belt;
  let url = Constants.bapiUrl ++ "/@me";
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
                  Array.keepMap(
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
                      Some(("userId", Js.Json.string(userId))),
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
              "Bearer " ++ Belt.Option.getWithDefault(sessionId, ""),
          }),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  Promise.resolved(response);
};

let createItemList = (~sessionId, ~items: array((int, int))) => {
  let url = Constants.bapiUrl ++ "/item-lists";
  let%Repromise.Js responseResult =
    Fetch.fetchWithInit(
      url,
      Fetch.RequestInit.make(
        ~method_=Post,
        ~body=
          Fetch.BodyInit.make(
            Js.Json.stringify(
              Json.Encode.(
                object_([("items", array(tuple2(int, int), items))])
              ),
            ),
          ),
        ~headers=
          Fetch.HeadersInit.make({
            "X-Client-Version": Constants.gitCommitRef,
            "Content-Type": "application/json",
            "Authorization":
              "Bearer " ++ Belt.Option.getWithDefault(sessionId, ""),
          }),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  Promise.resolved(responseResult);
};