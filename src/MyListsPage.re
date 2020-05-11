module WithViewer = {
  type listInfo = {
    id: string,
    createTime: Js.Date.t,
    title: option(string),
  };

  [@react.component]
  let make = (~me) => {
    let (lists, setLists) = React.useState(() => None);
    React.useEffect0(() => {
      {
        let%Repromise response =
          BAPI.getUserLists(
            ~sessionId=Belt.Option.getExn(UserStore.sessionId^),
          );
        let%Repromise.JsExn json =
          Fetch.Response.json(Belt.Result.getExn(response));
        let lists =
          Json.Decode.(
            json
            |> array(json =>
                 {
                   id: json |> field("id", string),
                   createTime: json |> field("createTime", date),
                   title:
                     (json |> optional(field("title", string)))
                     ->Belt.Option.flatMap(title =>
                         title == "" ? None : Some(title)
                       ),
                 }
               )
          );
        setLists(_ => Some(lists));
        Promise.resolved();
      }
      |> ignore;
      None;
    });
    <div>
      {switch (lists) {
       | Some(lists) =>
         <div>
           {lists
            |> Js.Array.map(list =>
                 <div key={list.id}>
                   <Link path={"/l/" ++ list.id}>
                     {React.string(
                        Belt.Option.getWithDefault(list.title, list.id),
                      )}
                   </Link>
                 </div>
               )
            |> React.array}
         </div>
       | None => React.null
       }}
    </div>;
  };
};

[@react.component]
let make = () => {
  let me = UserStore.useMe();

  switch (me) {
  | Some(me) => <WithViewer me />
  | None => React.null
  };
};