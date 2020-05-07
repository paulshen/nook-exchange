module Styles = {
  open Css;
  let root =
    style([paddingTop(px(32)), display(flexBox), justifyContent(center)]);
  let list =
    style([
      display(flexBox),
      flexWrap(wrap),
      justifyContent(center),
      marginRight(px(-16)),
    ]);
  let listCard =
    style([
      backgroundColor(Colors.white),
      display(flexBox),
      flexDirection(column),
      alignItems(center),
      color(Colors.charcoal),
      padding2(~v=px(16), ~h=px(16)),
      borderRadius(px(8)),
      marginRight(px(16)),
      marginBottom(px(16)),
      textDecoration(none),
      textAlign(center),
      width(px(160)),
    ]);
  let listImage =
    style([width(px(128)), height(px(128)), display(block)]);
  let itemName = style([fontSize(px(16))]);
  let variantName =
    style([fontSize(px(12)), color(Colors.gray), marginTop(px(8))]);
};

[@react.component]
let make = (~listId) => {
  let (list, setList) = React.useState(() => None);
  React.useEffect0(() => {
    {
      let%Repromise response = BAPI.getItemList(~listId);
      let%Repromise.JsExn json =
        Fetch.Response.json(Belt.Result.getExn(response));
      open Json.Decode;
      setList(_ =>
        Some(
          {
            id: Some(json |> field("id", string)),
            itemIds: json |> field("itemIds", array(tuple2(int, int))),
          }: QuicklistStore.t,
        )
      );
      Promise.resolved();
    }
    |> ignore;
    None;
  });

  <div className=Styles.root>
    {switch (list) {
     | Some(list) =>
       <div className=Styles.list>
         {list.itemIds
          |> Js.Array.mapi(((itemId, variant), i) => {
               let item = Item.getItem(~itemId);
               <Link
                 path={ItemDetailOverlay.getItemDetailUrl(
                   ~itemId,
                   ~variant=Some(variant),
                 )}
                 className=Styles.listCard
                 key={string_of_int(i)}>
                 <div className=Styles.itemName>
                   {React.string(Item.getName(item))}
                 </div>
                 <img
                   src={Item.getImageUrl(~item, ~variant)}
                   className=Styles.listImage
                 />
                 {switch (
                    Item.getVariantName(
                      ~item,
                      ~variant,
                      ~hidePattern=true,
                      (),
                    )
                  ) {
                  | Some(variantName) =>
                    <div className=Styles.variantName>
                      {React.string(variantName)}
                    </div>
                  | None => React.null
                  }}
               </Link>;
             })
          |> React.array}
       </div>
     | None => React.null
     }}
  </div>;
};