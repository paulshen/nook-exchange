let setItemStatus = (~userId, ~sessionId, ~itemId, ~variant, ~status) => {
  let url =
    Constants.bapiUrl
    ++ "/@me4/items/"
    ++ itemId
    ++ "/"
    ++ string_of_int(variant)
    ++ "/status";
  let%Repromise.Js _responseResult =
    Fetch.fetchWithInit(
      url,
      Fetch.RequestInit.make(
        ~method_=Post,
        ~body=
          Fetch.BodyInit.make(
            Js.Json.stringify(
              Json.Encode.object_([
                ("status", Json.Encode.int(User.itemStatusToJs(status))),
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

let setItemNote = (~userId, ~sessionId, ~itemId, ~variant, ~note) => {
  let url =
    Constants.bapiUrl
    ++ "/@me4/items/"
    ++ itemId
    ++ "/"
    ++ string_of_int(variant)
    ++ "/note";
  let%Repromise.Js _responseResult =
    Fetch.fetchWithInit(
      url,
      Fetch.RequestInit.make(
        ~method_=Post,
        ~body=
          Fetch.BodyInit.make(
            Js.Json.stringify(
              Json.Encode.object_([
                ("note", Json.Encode.string(note)),
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

let setItemStatusBatch = (~userId, ~sessionId, ~itemId, ~variants, ~status) => {
  let url = Constants.bapiUrl ++ "/@me4/items/" ++ itemId ++ "/batch/status";
  let%Repromise.Js _responseResult =
    Fetch.fetchWithInit(
      url,
      Fetch.RequestInit.make(
        ~method_=Post,
        ~body=
          Fetch.BodyInit.make(
            Js.Json.stringify(
              Json.Encode.object_([
                ("status", Json.Encode.int(User.itemStatusToJs(status))),
                ("variants", Json.Encode.array(Json.Encode.int, variants)),
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

let removeItem = (~userId, ~sessionId, ~itemId, ~variant) => {
  let url =
    Constants.bapiUrl
    ++ "/@me3/items/"
    ++ itemId
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