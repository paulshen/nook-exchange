open Belt;

module Styles = {
  open Css;
  let card =
    style([
      backgroundColor(hex("fffffff0")),
      borderRadius(px(8)),
      display(flexBox),
      flexDirection(column),
      alignItems(center),
      marginRight(px(32)),
      marginBottom(px(32)),
      padding3(~top=px(24), ~bottom=px(8), ~h=px(8)),
      position(relative),
      width(px(240)),
      boxShadow(Shadow.box(~spread=px(8), rgba(128, 128, 128, 0.1))),
    ]);
  let cardSelected =
    style([
      backgroundColor(hex("ffffff")),
      boxShadow(Shadow.box(~spread=px(8), rgba(128, 128, 128, 0.4))),
    ]);
  let body =
    style([
      flexGrow(1.),
      display(flexBox),
      flexDirection(column),
      alignItems(center),
      marginBottom(px(16)),
    ]);
  let name = style([fontSize(px(22)), marginBottom(px(8))]);
  let mainImage =
    style([
      display(block),
      height(px(128)),
      width(px(128)),
      marginBottom(px(16)),
    ]);
  let variations =
    style([display(flexBox), flexWrap(wrap), justifyContent(center)]);
  let variationImage =
    style([display(block), width(px(32)), height(px(32))]);
  let statusButtons = style([alignSelf(flexStart)]);
  let statusButton =
    style([
      backgroundColor(hex("f1e26f80")),
      borderWidth(zero),
      borderRadius(px(4)),
      fontSize(px(12)),
      marginRight(px(6)),
      outlineStyle(none),
      padding3(~top=px(3), ~bottom=px(2), ~h=px(6)),
      transition(~duration=200, "all"),
      cursor(pointer),
      hover([backgroundColor(hex("f1e26f"))]),
    ]);
  let statusButtonSelected =
    style([
      backgroundColor(hex("f1e26f")),
      boxShadow(Shadow.box(~spread=px(2), hex("bbac92"))),
    ]);
  let removeButton =
    style([
      position(absolute),
      bottom(px(8)),
      right(px(8)),
      backgroundColor(transparent),
      borderWidth(zero),
      fontSize(px(12)),
      opacity(0.3),
      hover([opacity(1.)]),
    ]);
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

let renderStatusButton =
    (
      ~itemId,
      ~variation,
      ~status,
      ~userItem: option(User.item),
      ~showLogin=?,
      (),
    ) => {
  let userItemStatus = Option.map(userItem, userItem => userItem.status);
  <button
    onClick={_ =>
      if (UserStore.isLoggedIn()) {
        UserStore.setItem(
          ~itemId,
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
        Option.map(showLogin, showLogin => showLogin()) |> ignore;
      }
    }
    className={Cn.make([
      Styles.statusButton,
      Cn.ifTrue(Styles.statusButtonSelected, userItemStatus == Some(status)),
    ])}>
    {React.string(
       switch (status) {
       | Want => {j|🙏 Wishlist|j}
       | WillTrade => {j|✅ Available|j}
       },
     )}
  </button>;
};

[@react.component]
let make = (~item: Item.t, ~showLogin) => {
  let (variation, setVariation) =
    React.useState(() =>
      Option.flatMap(item.variations, variations => variations[0])
    );
  let userItem = UserStore.useItem(~itemId=item.id, ~variation);

  <div
    className={Cn.make([
      Styles.card,
      Cn.ifSome(Styles.cardSelected, userItem),
    ])}>
    <div className=Styles.body>
      <div className=Styles.name> {React.string(item.name)} </div>
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
    </div>
    {switch (userItem) {
     | Some(userItem) =>
       <>
         <UserItemNote itemId={item.id} variation userItem />
         <button
           className=Styles.removeButton
           onClick={_ => {UserStore.removeItem(~itemId=item.id, ~variation)}}>
           {React.string({j|❌|j})}
         </button>
       </>
     | None => React.null
     }}
    <div className=Styles.statusButtons>
      {renderStatusButton(
         ~itemId=item.id,
         ~variation,
         ~status=Want,
         ~userItem,
         ~showLogin,
         (),
       )}
      {renderStatusButton(
         ~itemId=item.id,
         ~variation,
         ~status=WillTrade,
         ~userItem,
         ~showLogin,
         (),
       )}
    </div>
  </div>;
};