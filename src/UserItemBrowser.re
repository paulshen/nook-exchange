open Belt;

module UserItemCard = {
  [@react.component]
  let make = (~userItem: User.item, ~editable) => {
    let item = Item.getItem(~itemId=userItem.itemId);
    <div className=ItemCard.Styles.card>
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
  let numResultsPerPage = 12;

  [@react.component]
  let make =
      (~status: User.itemStatus, ~userItems: array(User.item), ~editable) => {
    let (filters, setFilters) =
      React.useState(() =>
        ({orderable: None, hasRecipe: None, category: None}: ItemFilters.t)
      );
    let (pageOffset, setPageOffset) = React.useState(() => 0);
    let filteredItems =
      React.useMemo2(
        () =>
          userItems->Belt.Array.keep(userItem =>
            ItemFilters.doesItemMatchFilters(
              ~item=Item.getItem(~itemId=userItem.itemId),
              ~filters,
            )
          ),
        (userItems, filters),
      );
    let numResults = filteredItems->Belt.Array.length;

    <div>
      <div>
        {React.string(
           switch (status) {
           | Want => "Want"
           | WillTrade => "Will Trade"
           },
         )}
      </div>
      <div>
        <ItemFilters
          filters
          onChange={filters => {
            setFilters(_ => filters);
            setPageOffset(_ => 0);
          }}
        />
        {if (numResults > 0) {
           <div>
             {React.string(
                "Showing "
                ++ string_of_int(pageOffset * numResultsPerPage + 1)
                ++ "-"
                ++ string_of_int(
                     Js.Math.min_int(
                       (pageOffset + 1) * numResultsPerPage,
                       numResults,
                     ),
                   )
                ++ " of "
                ++ string_of_int(numResults),
              )}
           </div>;
         } else {
           React.null;
         }}
      </div>
      <div className=ItemBrowser.Styles.cards>
        {filteredItems
         ->Belt.Array.slice(
             ~offset=pageOffset * numResultsPerPage,
             ~len=numResultsPerPage,
           )
         ->Belt.Array.mapU((. userItem) => {
             <UserItemCard
               userItem
               editable
               key={
                 userItem.itemId
                 ++ Option.getWithDefault(userItem.variation, "")
               }
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