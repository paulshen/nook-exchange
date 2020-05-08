let cardWidth = 176;

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
        [paddingTop(px(16)), marginLeft(px(16)), marginRight(px(16))],
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
  let viewButtonSelected = style([important(opacity(1.))]);
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
    style([display(flexBox), flexWrap(wrap), marginRight(px(-16))]);
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
    let variantName =
      style([
        color(Colors.gray),
        padding2(~v=zero, ~h=px(8)),
        textAlign(`right),
      ]);
    let imageWrapper = style([position(relative), fontSize(zero)]);
    let image = style([width(px(48)), height(px(48))]);
    let recipeIcon =
      style([
        position(absolute),
        bottom(px(-4)),
        right(px(-4)),
        width(px(24)),
        height(px(24)),
      ]);
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
      <div className=Styles.imageWrapper>
        <img src={Item.getImageUrl(~item, ~variant)} className=Styles.image />
        <img
          src={Constants.cdnUrl ++ "/images/DIYRecipe.png"}
          className=Styles.recipeIcon
        />
      </div>
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
      let list: QuicklistStore.t = {
        id: Some(json |> field("id", string)),
        userId: json |> optional(field("userId", string)),
        itemIds: json |> field("itemIds", array(tuple2(int, int))),
      };
      setList(_ => Some(list));
      Analytics.Amplitude.logEventWithProperties(
        ~eventName="Item List Page Viewed",
        ~eventProperties={
          "listId": listId,
          "numItems": Js.Array.length(list.itemIds),
        },
      );
      Promise.resolved();
    }
    |> ignore;
    None;
  });

  let (viewMode, setViewMode) = React.useState(() => List);
  let me = UserStore.useMe();
  let numViewToggleLoggedRef = React.useRef(0);

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
        onClick={_ => {
          setViewMode(_ => List);
          if (React.Ref.current(numViewToggleLoggedRef) < 5) {
            Analytics.Amplitude.logEventWithProperties(
              ~eventName="Item List Page View Toggled",
              ~eventProperties={
                "view": "list",
                "listId": listId,
                "numItems":
                  switch (list) {
                  | Some(list) => Js.Array.length(list.itemIds)
                  | None => 0
                  },
              },
            );
            React.Ref.setCurrent(
              numViewToggleLoggedRef,
              React.Ref.current(numViewToggleLoggedRef) + 1,
            );
          };
        }}
        className={Cn.make([
          Styles.viewButton,
          Cn.ifTrue(Styles.viewButtonSelected, viewMode == List),
        ])}>
        <span className=Styles.listIcon />
        {React.string("List")}
      </button>
      <button
        onClick={_ => {
          setViewMode(_ => Grid);
          if (React.Ref.current(numViewToggleLoggedRef) < 5) {
            Analytics.Amplitude.logEventWithProperties(
              ~eventName="Item List Page View Toggled",
              ~eventProperties={
                "view": "grid",
                "listId": listId,
                "numItems":
                  switch (list) {
                  | Some(list) => Js.Array.length(list.itemIds)
                  | None => 0
                  },
              },
            );
            React.Ref.setCurrent(
              numViewToggleLoggedRef,
              React.Ref.current(numViewToggleLoggedRef) + 1,
            );
          };
        }}
        className={Cn.make([
          Styles.viewButton,
          Cn.ifTrue(Styles.viewButtonSelected, viewMode == Grid),
        ])}>
        <span className=Styles.gridIcon />
        {React.string("Grid")}
      </button>
    </div>
    {switch (list) {
     | Some(list) =>
       <div>
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
                 | Grid =>
                   <UserItemCard
                     itemId
                     variation=variant
                     editable=false
                     showRecipe=false
                     showMetaIcons=false
                     key={string_of_int(i)}
                   />
                 | List => <ListRow itemId variant key={string_of_int(i)} />
                 }
               })
            |> React.array}
         </div>
         {switch (me) {
          | Some(me) =>
            if (list.userId == Some(me.id)) {
              React.null;
            } else {
              React.null;
            }
          | None => React.null
          }}
       </div>
     | None => React.null
     }}
  </div>;
};