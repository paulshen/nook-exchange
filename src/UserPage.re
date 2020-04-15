module Styles = {
  open Css;
  let emptyProfile =
    style([
      maxWidth(px(768)),
      margin3(~top=zero, ~bottom=px(128), ~h=auto),
      textAlign(center),
    ]);
  let username =
    style([
      fontSize(px(36)),
      textAlign(center),
      marginTop(px(32)),
      marginBottom(px(32)),
    ]);
  let bodyText = style([fontSize(px(18))]);
};

module ViewingPage = {
  [@react.component]
  let make = (~userId) => {
    let (user, setUser) = React.useState(() => None);
    React.useEffect1(
      () => {
        {
          let%Repromise.JsExn response =
            Fetch.fetch(Constants.apiUrl ++ "/users/" ++ userId);
          switch (Fetch.Response.status(response)) {
          | 200 =>
            let%Repromise.JsExn json = Fetch.Response.json(response);
            setUser(_ => Some(User.fromAPI(json)));
            Promise.resolved();
          | _ => Promise.resolved()
          };
        }
        |> ignore;
        None;
      },
      [|userId|],
    );
    <div>
      <div className=Styles.username> {React.string(userId)} </div>
      {switch (user) {
       | Some(user) =>
         if (user.items->Js.Dict.keys->Js.Array.length > 0) {
           <UserItemBrowser
             userItems={
               user.items
               ->Js.Dict.entries
               ->Belt.Array.mapU((. (itemKey, item)) =>
                   (User.fromItemKey(~key=itemKey), item)
                 )
             }
             editable=false
           />;
         } else {
           <div className=Styles.emptyProfile>
             <div className=Styles.bodyText>
               {React.string("This profile is empty!")}
             </div>
           </div>;
         }
       | None => React.null
       }}
    </div>;
  };
};

[@react.component]
let make = (~userId) => {
  let user = UserStore.useMe();
  switch (user) {
  | Some(user) =>
    if (user.id == userId) {
      <MyPage user />;
    } else {
      <ViewingPage userId />;
    }
  | None => <ViewingPage userId />
  };
};