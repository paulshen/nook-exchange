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
  let%Repromise.Js _responseResult =
    makeAuthenticatedPostRequest(
      ~url=
        Constants.bapiUrl
        ++ "/@me4/items/"
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
  Promise.resolved();
};

let setItemNote = (~userId, ~sessionId, ~itemId, ~variant, ~note) => {
  let%Repromise.Js _responseResult =
    makeAuthenticatedPostRequest(
      ~url=
        Constants.bapiUrl
        ++ "/@me4/items/"
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
  Promise.resolved();
};

let setItemStatusBatch = (~userId, ~sessionId, ~itemId, ~variants, ~status) => {
  let%Repromise.Js _responseResult =
    makeAuthenticatedPostRequest(
      ~url=
        Constants.bapiUrl
        ++ "/@me4/items/"
        ++ string_of_int(itemId)
        ++ "/batch/status",
      ~bodyJson=[
        ("status", Json.Encode.int(User.itemStatusToJs(status))),
        ("variants", Json.Encode.array(Json.Encode.int, variants)),
        ("userId", Json.Encode.string(userId)),
      ],
      ~sessionId,
    );
  Promise.resolved();
};

let removeItem = (~userId, ~sessionId, ~itemId, ~variant) => {
  let url =
    Constants.bapiUrl
    ++ "/@me3/items/"
    ++ string_of_int(itemId)
    ++ "/"
    ++ string_of_int(variant);
  let%Repromise.Js _responseResult =
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
  Promise.resolved();
};

let updateProfileText = (~userId, ~sessionId, ~profileText) => {
  let%Repromise.Js _responseResult =
    makeAuthenticatedPostRequest(
      ~url=Constants.bapiUrl ++ "/@me/profileText",
      ~bodyJson=[
        ("text", Json.Encode.string(profileText)),
        ("userId", Json.Encode.string(userId)),
      ],
      ~sessionId,
    );
  Promise.resolved();
};

let updateSetting = (~userId, ~sessionId, ~settingKey, ~settingValue) => {
  let url = Constants.bapiUrl ++ "/@me/settings";
  let%Repromise.Js _responseResult =
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
  Promise.resolved();
};

let patchMe =
    (~userId, ~sessionId, ~username, ~newPassword, ~email, ~oldPassword) => {
  open Belt;
  let url = Constants.bapiUrl ++ "/@me";
  let%Repromise.Js _responseResult =
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
  Promise.resolved();
};