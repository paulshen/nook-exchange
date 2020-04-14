open Belt;

module Styles = {
  open Css;
  let card =
    style([
      border(px(1), solid, hex("f0f0f0")),
      display(flexBox),
      flexDirection(column),
      alignItems(center),
      padding2(~v=px(32), ~h=px(32)),
      width(px(224)),
    ]);
  let mainImage = style([height(px(128)), width(px(128))]);
  let variations =
    style([display(flexBox), flexWrap(wrap), justifyContent(center)]);
  let variationImage =
    style([display(block), width(px(48)), height(px(48))]);
  let buttonSelected = style([fontWeight(semiBold)]);
};

module Recipe = {
  [@react.component]
  let make = (~recipe: Item.recipe) => {
    <div>
      {recipe
       ->Array.map(((itemId, quantity)) =>
           <div key=itemId>
             {React.string(itemId ++ " x " ++ string_of_int(quantity))}
           </div>
         )
       ->React.array}
    </div>;
  };
};

[@react.component]
let make = (~item: Item.t, ~showLogin) => {
  let (variation, setVariation) =
    React.useState(() =>
      Option.flatMap(item.variations, variations => variations[0])
    );
  let userItem = UserStore.useItem(~itemId=item.id, ~variation);
  let userItemStatus = Option.map(userItem, userItem => userItem.status);

  let renderStatusButton = (status, label) => {
    <button
      onClick={_ =>
        if (UserStore.isLoggedIn()) {
          UserStore.setItem(
            ~itemId=item.id,
            ~variation,
            ~item={
              status,
              note:
                switch (userItem) {
                | Some(userItem) => userItem.note
                | None => ""
                },
            },
          );
        } else {
          showLogin();
        }
      }
      className={Cn.ifTrue(
        Styles.buttonSelected,
        userItemStatus == Some(status),
      )}>
      {React.string(label)}
    </button>;
  };

  <div className=Styles.card>
    <div>
      {React.string(item.name)}
      {switch (variation) {
       | Some(variation) => React.string(" " ++ variation)
       | None => React.null
       }}
    </div>
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
      className=Styles.mainImage
    />
    {switch (item.variations) {
     | Some(variations) =>
       <div className=Styles.variations>
         {variations
          ->Array.map(variation =>
              <div
                onClick={_ => {setVariation(_ => Some(variation))}}
                key=variation>
                <img
                  src={"https://imgur.com/" ++ variation ++ ".png"}
                  className=Styles.variationImage
                />
              </div>
            )
          ->React.array}
       </div>
     | None => React.null
     }}
    {switch (item.recipe) {
     | Some(recipe) => <Recipe recipe />
     | None => React.null
     }}
    <div>
      {React.string(item.orderable ? "Orderable" : "Not Orderable")}
    </div>
    <div>
      {renderStatusButton(Want, "I want this")}
      {renderStatusButton(WillTrade, "I'll trade this")}
      {switch (userItem) {
       | Some(userItem) =>
         <>
           <UserItemNote itemId={item.id} variation userItem />
           <button
             onClick={_ => {
               UserStore.removeItem(~itemId=item.id, ~variation)
             }}>
             {React.string("Remove")}
           </button>
         </>
       | None => React.null
       }}
    </div>
  </div>;
};