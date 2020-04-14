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
      <div> {React.string(userId)} </div>
      {switch (user) {
       | Some(user) =>
         <UserItemBrowser userItems={user.items} editable=false />
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