module Styles = {
  open Css;
  let emptyProfile =
    style([
      maxWidth(px(768)),
      margin3(~top=zero, ~bottom=px(128), ~h=auto),
      textAlign(center),
    ]);
  let username =
    style([fontSize(px(36)), textAlign(center), marginBottom(px(16))]);
  let userBodySpacer = style([height(px(16))]);
  let userBody =
    style([
      backgroundColor(hex("ffffffc0")),
      boxSizing(borderBox),
      lineHeight(px(20)),
      margin3(~top=zero, ~bottom=px(48), ~h=auto),
      maxWidth(px(512)),
      padding2(~v=px(16), ~h=px(24)),
      borderRadius(px(8)),
      whiteSpace(`preLine),
      media(
        "(max-width: 512px)",
        [
          borderRadius(zero),
          padding(px(16)),
          marginBottom(zero),
          borderBottom(px(1), solid, Colors.faintGray),
        ],
      ),
    ]);
  let bodyText = style([fontSize(px(18))]);
  let followBlock =
    style([
      backgroundColor(hex("ffffffc0")),
      boxSizing(borderBox),
      maxWidth(px(512)),
      padding2(~v=px(32), ~h=px(24)),
      margin3(~top=px(64), ~bottom=px(48), ~h=auto),
      borderRadius(px(8)),
      textAlign(center),
    ]);
};

module FollowLink = {
  type status =
    | Success
    | Error;

  [@react.component]
  let make = (~user: User.t, ~showLogin) => {
    let (status, setStatus) = React.useState(() => None);

    // TODO: get and render follow status

    switch (status) {
    | Some(Success) => <div> {React.string("Success!")} </div>
    | _ =>
      <div>
        <a
          href="#"
          onClick={e => {
            ReactEvent.Mouse.preventDefault(e);
            if (UserStore.isLoggedIn()) {
              {
                let%Repromise success =
                  BAPI.followUser(
                    ~userId=user.id,
                    ~sessionId=Belt.Option.getExn(UserStore.sessionId^),
                  );
                setStatus(_ => Some(success ? Success : Error));
                Promise.resolved();
              }
              |> ignore;
            } else {
              showLogin();
            };
          }}>
          {React.string("Add " ++ user.username ++ " to friends")}
        </a>
        {status == Some(Error)
           ? <div>
               {React.string(
                  "Oh no! Something wrong happened. Please try again.",
                )}
             </div>
           : React.null}
      </div>
    };
  };
};

[@react.component]
let make = (~username, ~urlRest, ~url: ReasonReactRouter.url, ~showLogin) => {
  let list =
    switch (urlRest) {
    | [url] => ViewingList.urlToViewingList(url)
    | _ => None
    };
  let (user, setUser) = React.useState(() => None);
  let isMountedRef = React.useRef(true);
  React.useEffect0(() => {
    open Webapi.Dom;
    window |> Window.scrollTo(0., 0.);
    Some(() => {React.Ref.setCurrent(isMountedRef, false)});
  });
  React.useEffect1(
    () => {
      {
        let%Repromise.JsExn response =
          Fetch.fetchWithInit(
            Constants.bapiUrl ++ "/users/" ++ username,
            Fetch.RequestInit.make(
              ~headers=
                Fetch.HeadersInit.make({
                  "X-Client-Version": Constants.gitCommitRef,
                }),
              (),
            ),
          );
        switch (Fetch.Response.status(response)) {
        | 200 =>
          let%Repromise.JsExn json = Fetch.Response.json(response);
          if (React.Ref.current(isMountedRef)) {
            setUser(_ => Some(User.fromAPI(json)));
          };
          Promise.resolved();
        | _ => Promise.resolved()
        };
      }
      |> ignore;
      None;
    },
    [|username|],
  );
  React.useEffect0(() => {
    switch (url.hash) {
    | "for-trade"
    | "can-craft"
    | "wishlist" =>
      if (urlRest == []) {
        ReasonReactRouter.replace(
          "/"
          ++ Js.Array.joinWith("/", Belt.List.toArray(url.path))
          ++ "/"
          ++ url.hash,
        );
      }
    | _ => ()
    };
    None;
  });
  <div>
    <div className=Styles.username> {React.string(username)} </div>
    {switch (user) {
     | Some(user) =>
       <div>
         {switch (user.profileText) {
          | "" => <div className=Styles.userBodySpacer />
          | profileText =>
            <div className=Styles.userBody>
              {Emoji.parseText(profileText)}
            </div>
          }}
         {switch (list) {
          | Some(list) => <UserListBrowser user list url />
          | None =>
            if (user.items->Js.Dict.keys->Js.Array.length > 0) {
              <UserProfileBrowser
                username
                userItems={
                  user.items
                  ->Js.Dict.entries
                  ->Belt.Array.keepMapU((. (itemKey, item)) =>
                      User.fromItemKey(~key=itemKey)
                      ->Belt.Option.map(x => (x, item))
                    )
                }
                editable=false
              />;
            } else {
              <div className=Styles.emptyProfile>
                <div className=Styles.bodyText>
                  {React.string("I have no lists!")}
                </div>
              </div>;
            }
          }}
         // TODO: hide if already friends
         <div className=Styles.followBlock>
           <FollowLink user showLogin />
         </div>
       </div>
     | None => React.null
     }}
  </div>;
};