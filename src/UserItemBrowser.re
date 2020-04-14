open Belt;

module UserItemCard = {
  [@react.component]
  let make = (~itemId, ~variation, ~userItem: User.item, ~editable) => {
    let item = Item.getItem(~itemId);
    <div className=ItemCard.Styles.card>
      <div className=ItemCard.Styles.body>
        <div className=ItemCard.Styles.name> {React.string(item.name)} </div>
        <img
          src={
            "https://imgur.com/"
            ++ (
              switch (variation) {
              | Some(variation) => variation
              | None => item.image
              }
            )
            ++ ".png"
          }
          className=ItemCard.Styles.mainImage
        />
        <div>
          {React.string(item.orderable ? "Orderable" : "Not Orderable")}
        </div>
      </div>
      {editable
         ? <>
             <UserItemNote itemId={item.id} variation userItem />
             <button
               className=ItemCard.Styles.removeButton
               onClick={_ => {
                 UserStore.removeItem(~itemId=item.id, ~variation)
               }}>
               {React.string({j|❌|j})}
             </button>
             <div className=ItemCard.Styles.statusButtons>
               {ItemCard.renderStatusButton(
                  ~itemId=item.id,
                  ~variation,
                  ~status=Want,
                  ~userItem=Some(userItem),
                  (),
                )}
               {ItemCard.renderStatusButton(
                  ~itemId=item.id,
                  ~variation,
                  ~status=WillTrade,
                  ~userItem=Some(userItem),
                  (),
                )}
             </div>
           </>
         : <div>
             {switch (userItem.status) {
              | Want => <div> {React.string("I want this!")} </div>
              | WillTrade => <div> {React.string("I'll trade this!")} </div>
              }}
             {if (userItem.note->Js.String.length > 0) {
                <div> {React.string(userItem.note)} </div>;
              } else {
                React.null;
              }}
           </div>}
    </div>;
  };
};

module Section = {
  let numResultsPerPage = 12;

  [@react.component]
  let make =
      (
        ~status: User.itemStatus,
        ~userItems: array(((string, option(string)), User.item)),
        ~editable,
      ) => {
    let (filters, setFilters) =
      React.useState(() =>
        ({orderable: None, hasRecipe: None, category: None}: ItemFilters.t)
      );
    let (pageOffset, setPageOffset) = React.useState(() => 0);
    let filteredItems =
      React.useMemo2(
        () =>
          userItems->Belt.Array.keep((((itemId, _), _)) =>
            ItemFilters.doesItemMatchFilters(
              ~item=Item.getItem(~itemId),
              ~filters,
            )
          ),
        (userItems, filters),
      );
    let numResults = filteredItems->Belt.Array.length;

    <div className=ItemBrowser.Styles.root>
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
         ->Belt.Array.mapU((. ((itemId, variation), userItem)) => {
             <UserItemCard
               itemId
               variation
               userItem
               editable
               key={itemId ++ Option.getWithDefault(variation, "")}
             />
           })
         ->React.array}
      </div>
    </div>;
  };
};

[@react.component]
let make =
    (~userItems: array(((string, option(string)), User.item)), ~editable) => {
  let (wants, willTrades) =
    userItems->Array.partitionU((. (_, item)) => item.status == Want);
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