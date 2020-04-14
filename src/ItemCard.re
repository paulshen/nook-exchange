module Styles = {
  open Css;
  let card =
    style([
      border(px(1), solid, hex("f0f0f0")),
      padding2(~v=px(32), ~h=px(32)),
    ]);
  let variations = style([display(flexBox)]);
  let variationImage =
    style([display(block), width(px(48)), height(px(48))]);
  let buttonSelected = style([fontWeight(semiBold)]);
};

module Recipe = {
  [@react.component]
  let make = (~recipe: Item.recipe) => {
    <div>
      {recipe
       ->Belt.Array.map(((itemId, quantity)) =>
           <div key=itemId>
             {React.string(itemId ++ " x " ++ string_of_int(quantity))}
           </div>
         )
       ->React.array}
    </div>;
  };
};

[@react.component]
let make = (~item: Item.t) => {
  let (variation, setVariation) =
    React.useState(() =>
      Belt.Option.map(item.variations, variations => variations[0])
    );
  let userItem = UserStore.useItem(~itemId=item.id, ~variation);
  let userItemStatus = Belt.Option.map(userItem, userItem => userItem.status);
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
    />
    {switch (item.variations) {
     | Some(variations) =>
       <div className=Styles.variations>
         {variations
          ->Belt.Array.map(variation =>
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
      <button
        onClick={_ => {
          UserStore.setItem(
            ~itemId=item.id,
            ~variation,
            ~item={status: Want, note: ""},
          )
        }}
        className={Cn.ifTrue(
          Styles.buttonSelected,
          userItemStatus == Some(Want),
        )}>
        {React.string("I want this")}
      </button>
      <button
        onClick={_ => {
          UserStore.setItem(
            ~itemId=item.id,
            ~variation,
            ~item={status: WillTrade, note: ""},
          )
        }}
        className={Cn.ifTrue(
          Styles.buttonSelected,
          userItemStatus == Some(WillTrade),
        )}>
        {React.string("I'll trade this")}
      </button>
      {switch (userItemStatus) {
       | Some(_) =>
         <button
           onClick={_ => {UserStore.removeItem(~itemId=item.id, ~variation)}}>
           {React.string("Remove")}
         </button>
       | None => React.null
       }}
    </div>
  </div>;
};