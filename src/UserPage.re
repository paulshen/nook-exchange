open Belt;

module UserItem = {
  [@react.component]
  let make = (~userItem: User.item, ~editable) => {
    let item = Item.getItem(~itemId=userItem.itemId);
    <div className=ItemCard.Styles.card>
      <div> {React.string(item.id)} </div>
      <div> {React.string(item.name)} </div>
      <img
        src={
          "https://imgur.com/"
          ++ (
            switch (userItem.variation) {
            | Some(variation) => variation
            | None => item.image
            }
          )
          ++ ".png"
        }
      />
      <div>
        {React.string(item.orderable ? "Orderable" : "Not Orderable")}
      </div>
      {editable
         ? <div>
             <button
               onClick={_ => {
                 UserStore.setItem(
                   ~item={
                     itemId: item.id,
                     variation: userItem.variation,
                     status: Want,
                   },
                 )
               }}
               className={Cn.ifTrue(
                 ItemCard.Styles.buttonSelected,
                 userItem.status == Want,
               )}>
               {React.string("I want this")}
             </button>
             <button
               onClick={_ => {
                 UserStore.setItem(
                   ~item={
                     itemId: item.id,
                     variation: userItem.variation,
                     status: Have,
                   },
                 )
               }}
               className={Cn.ifTrue(
                 ItemCard.Styles.buttonSelected,
                 userItem.status == Have,
               )}>
               {React.string("I'll trade this")}
             </button>
             <button
               onClick={_ => {
                 UserStore.removeItem(
                   ~itemId=item.id,
                   ~variation=userItem.variation,
                 )
               }}>
               {React.string("Remove")}
             </button>
           </div>
         : <div>
             {switch (userItem.status) {
              | Want => <div> {React.string("I want this!")} </div>
              | Have => <div> {React.string("I have this!")} </div>
              }}
           </div>}
    </div>;
  };
};

[@react.component]
let make = (~userId) => {
  let viewer = UserStore.useMe();
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
       <div className=ItemBrowser.Styles.cards>
         {user.items
          ->Array.mapU((. item) => {
              <UserItem
                userItem=item
                editable={
                  Option.map(viewer, viewer => viewer.id) == Some(userId)
                }
                key={item.itemId ++ Option.getWithDefault(item.variation, "")}
              />
            })
          ->React.array}
       </div>
     | None => React.null
     }}
  </div>;
};