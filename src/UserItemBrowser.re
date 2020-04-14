open Belt;

module UserItemCard = {
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
                     status: WillTrade,
                   },
                 )
               }}
               className={Cn.ifTrue(
                 ItemCard.Styles.buttonSelected,
                 userItem.status == WillTrade,
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
              | WillTrade => <div> {React.string("I'll trade this!")} </div>
              }}
           </div>}
    </div>;
  };
};

module Section = {
  [@react.component]
  let make = (~status: User.itemStatus, ~userItems, ~editable) => {
    <div>
      <div>
        {React.string(
           switch (status) {
           | Want => "Want"
           | WillTrade => "Will Trade"
           },
         )}
      </div>
      <div className=ItemBrowser.Styles.cards>
        {userItems
         ->Array.mapU((. item) => {
             <UserItemCard
               userItem=item
               editable
               key={item.itemId ++ Option.getWithDefault(item.variation, "")}
             />
           })
         ->React.array}
      </div>
    </div>;
  };
};

[@react.component]
let make = (~userItems: array(User.item), ~editable) => {
  let (wants, willTrades) =
    userItems->Array.partitionU((. item) => item.status == Want);
  <div>
    {if (wants->Array.length > 0) {
       <Section status=Want userItems=wants editable />;
     } else {
       React.null;
     }}
    {if (willTrades->Array.length > 0) {
       <Section status=WillTrade userItems=willTrades editable />;
     } else {
       React.null;
     }}
  </div>;
};