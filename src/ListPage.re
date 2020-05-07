let cardWidth = 192;

module Styles = {
  open Css;
  let root =
    style([
      paddingTop(px(32)),
      display(flexBox),
      flexDirection(column),
      margin2(~v=zero, ~h=auto),
      media(
        "(max-width: 630px)",
        [marginLeft(px(16)), marginRight(px(16))],
      ),
    ]);
  let gridWidth = numCards => numCards * cardWidth + (numCards - 1) * 16;
  let rootGrid =
    style([
      width(px(gridWidth(4))),
      media("(max-width: 840px)", [width(px(gridWidth(3)))]),
      media("(max-width: 630px)", [width(auto)]),
    ]);
  let rootList = style([maxWidth(px(560))]);
  let topRow =
    style([
      display(flexBox),
      justifyContent(flexEnd),
      marginBottom(px(16)),
    ]);
  [@bs.module "./assets/grid.png"] external gridPng: string = "default";
  [@bs.module "./assets/list.png"] external listPng: string = "default";
  let viewButton =
    style([
      display(flexBox),
      borderWidth(zero),
      alignItems(center),
      backgroundColor(Colors.green),
      color(Colors.white),
      borderRadius(px(4)),
      padding2(~v=px(6), ~h=px(10)),
      marginLeft(px(8)),
      opacity(0.5),
      transition(~duration=200, "all"),
      hover([opacity(0.8)]),
    ]);
  let viewButtonSelected = style([opacity(1.)]);
  let gridIcon =
    style([
      display(inlineBlock),
      width(px(18)),
      height(px(18)),
      backgroundImage(url(gridPng)),
      backgroundSize(cover),
      marginRight(px(8)),
    ]);
  let listIcon =
    style([
      display(inlineBlock),
      width(px(18)),
      height(px(18)),
      backgroundImage(url(listPng)),
      backgroundSize(cover),
      marginRight(px(8)),
    ]);
  let list =
    style([
      borderRadius(px(8)),
      overflow(hidden),
      border(px(1), solid, Colors.lightGreen),
    ]);
  let grid =
    style([
      display(flexBox),
      flexWrap(wrap),
      marginRight(px(-16)),
      media(
        "(max-width: 630px)",
        [marginRight(zero), justifyContent(spaceBetween)],
      ),
    ]);
};

module GridCard = {
  module Styles = {
    open Css;
    let gridItem =
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
        width(px(192)),
        boxSizing(borderBox),
        media(
          "(max-width: 630px)",
          [width(Calc.(pct(50.) - px(8))), marginRight(zero)],
        ),
        media(
          "(hover: hover)",
          [
            hover([boxShadow(Shadow.box(~blur=px(32), hex("3aa56340")))]),
          ],
        ),
      ]);
    let gridImage =
      style([width(px(128)), height(px(128)), display(block)]);
    let spacer = style([flexGrow(1.)]);
    let itemName = style([fontSize(px(16))]);
    let variantName = style([color(Colors.gray), marginTop(px(8))]);
  };

  [@react.component]
  let make = (~itemId, ~variant) => {
    let item = Item.getItem(~itemId);
    <Link
      path={ItemDetailOverlay.getItemDetailUrl(
        ~itemId,
        ~variant=Some(variant),
      )}
      className=Styles.gridItem>
      <div className=Styles.itemName>
        {React.string(Item.getName(item))}
      </div>
      <img
        src={Item.getImageUrl(~item, ~variant)}
        className=Styles.gridImage
      />
      <div className=Styles.spacer />
      {switch (Item.getVariantName(~item, ~variant, ~hidePattern=true, ())) {
       | Some(variantName) =>
         <div className=Styles.variantName> {React.string(variantName)} </div>
       | None => React.null
       }}
    </Link>;
  };
};

module ListRow = {
  module Styles = {
    open Css;
    let itemName = style([flexGrow(1.), padding2(~v=zero, ~h=px(16))]);
    let row =
      style([
        backgroundColor(Colors.white),
        padding2(~v=px(8), ~h=px(8)),
        borderTop(px(1), solid, Colors.faintGray),
        firstChild([borderTopWidth(zero)]),
        display(flexBox),
        alignItems(center),
        fontSize(px(16)),
        color(Colors.charcoal),
        textDecoration(none),
        hover([
          backgroundColor(hex("f2fbf3")),
          borderTopColor(Colors.lightGreen),
          // color(Colors.white),
          // selector("& ." ++ itemName, [textDecoration(underline)]),
        ]),
      ]);
    let variantName = style([color(Colors.gray), paddingRight(px(8))]);
    let image = style([width(px(48)), height(px(48))]);
  };

  [@react.component]
  let make = (~itemId, ~variant) => {
    let item = Item.getItem(~itemId);
    <Link
      path={ItemDetailOverlay.getItemDetailUrl(
        ~itemId,
        ~variant=Some(variant),
      )}
      className=Styles.row>
      <img src={Item.getImageUrl(~item, ~variant)} className=Styles.image />
      <div className=Styles.itemName>
        {React.string(Item.getName(item))}
      </div>
      {switch (Item.getVariantName(~item, ~variant, ~hidePattern=true, ())) {
       | Some(variantName) =>
         <div className=Styles.variantName> {React.string(variantName)} </div>
       | None => React.null
       }}
    </Link>;
  };
};

type viewMode =
  | Grid
  | List;

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

  let (viewMode, setViewMode) = React.useState(() => List);

  <div
    className={Cn.make([
      Styles.root,
      switch (viewMode) {
      | Grid => Styles.rootGrid
      | List => Styles.rootList
      },
    ])}>
    <div className=Styles.topRow>
      <button
        onClick={_ => {setViewMode(_ => Grid)}}
        className={Cn.make([
          Styles.viewButton,
          Cn.ifTrue(Styles.viewButtonSelected, viewMode == Grid),
        ])}>
        <span className=Styles.gridIcon />
        {React.string("Grid")}
      </button>
      <button
        onClick={_ => {setViewMode(_ => List)}}
        className={Cn.make([
          Styles.viewButton,
          Cn.ifTrue(Styles.viewButtonSelected, viewMode == List),
        ])}>
        <span className=Styles.listIcon />
        {React.string("List")}
      </button>
    </div>
    {switch (list) {
     | Some(list) =>
       <div
         className={
           switch (viewMode) {
           | Grid => Styles.grid
           | List => Styles.list
           }
         }>
         {list.itemIds
          |> Js.Array.mapi(((itemId, variant), i) => {
               switch (viewMode) {
               | Grid => <GridCard itemId variant key={string_of_int(i)} />
               | List => <ListRow itemId variant key={string_of_int(i)} />
               }
             })
          |> React.array}
       </div>
     | None => React.null
     }}
  </div>;
};