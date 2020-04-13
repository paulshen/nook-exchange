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
    {React.string(
       Js.Json.stringifyAny(user)->Belt.Option.getWithDefault(""),
     )}
  </div>;
};