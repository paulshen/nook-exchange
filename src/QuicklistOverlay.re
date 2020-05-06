module Styles = {
  open Css;
  let root =
    style([
      position(fixed),
      bottom(zero),
      left(px(32)),
      width(px(288)),
      borderTopLeftRadius(px(8)),
      borderTopRightRadius(px(8)),
      overflow(hidden),
      backgroundColor(Colors.green),
      color(Colors.white),
      transition(~duration=300, "all"),
      transform(translate3d(zero, pct(100.), zero)),
    ]);
  let rootWithQuicklist = style([Colors.darkLayerShadow]);
  let shown = style([transform(translate3d(zero, px(384), zero))]);
  let introBar =
    style([
      display(flexBox),
      justifyContent(spaceBetween),
      alignItems(center),
      padding2(~v=px(8), ~h=px(8)),
      transition(~duration=200, "all"),
    ]);
  let button =
    style([
      backgroundColor(Colors.white),
      color(Colors.green),
      borderWidth(zero),
      borderRadius(px(4)),
      padding2(~v=px(8), ~h=px(12)),
      cursor(pointer),
      hover([backgroundColor(Colors.white)]),
    ]);
  let barLink =
    style([
      color(Colors.white),
      marginRight(px(8)),
      textDecoration(none),
    ]);
  let shownPanel =
    style([
      boxShadow(Shadow.box(~blur=px(24), hex("00000080"))),
      transform(translate3d(zero, zero, zero)),
      backgroundColor(Colors.white),
      color(Colors.charcoal),
      selector("& ." ++ barLink, [color(Colors.green)]),
    ]);
  let body =
    style([
      height(px(384)),
      boxSizing(borderBox),
      padding3(~top=px(16), ~h=px(16), ~bottom=px(8)),
      display(flexBox),
      flexDirection(column),
    ]);
  let bodyList = style([flexGrow(1.)]);
  let saveRow = style([display(flexBox), justifyContent(flexEnd)]);
  let saveButton =
    style([
      padding2(~v=px(12), ~h=px(24)),
      backgroundColor(Colors.green),
      color(Colors.white),
      borderRadius(px(8)),
    ]);
  let listImages = style([display(flexBox), flexWrap(wrap)]);
  let listImage = style([display(block), width(px(64)), height(px(64))]);
};

type visibility =
  | Hidden
  | Bar
  | Panel;

[@react.component]
let make = () => {
  let quicklist = QuicklistStore.useQuicklist();
  let (visibility, setVisibility) = React.useState(() => Bar);
  let url = ReasonReactRouter.useUrl();

  if (visibility == Panel && quicklist == None) {
    setVisibility(_ => Bar);
  };

  <div
    className={Cn.make([
      Styles.root,
      Cn.ifTrue(
        Styles.shown,
        quicklist != None || visibility != Hidden && url.path != [],
      ),
      Cn.ifTrue(Styles.shownPanel, visibility == Panel),
      Cn.ifTrue(Styles.rootWithQuicklist, quicklist != None),
    ])}>
    <div className=Styles.introBar>
      {switch (quicklist) {
       | Some(quicklist) =>
         let numItems = quicklist.itemIds->Js.Array.length;
         <button
           onClick={_ => {
             setVisibility(visibility =>
               switch (visibility) {
               | Bar => Panel
               | _ => Bar
               }
             )
           }}
           className=Styles.button>
           {React.string(
              visibility == Panel
                ? "Hide Panel"
                : numItems > 0
                    ? "See "
                      ++ string_of_int(numItems)
                      ++ " item"
                      ++ (numItems == 1 ? "" : "s")
                    : "Select items!",
            )}
         </button>;
       | None =>
         <button
           onClick={_ => {QuicklistStore.startList()}} className=Styles.button>
           {React.string("Start a quick list")}
         </button>
       }}
      {switch (quicklist) {
       | Some(_quicklist) =>
         <a
           href="#"
           onClick={e => {
             ReactEvent.Mouse.preventDefault(e);
             QuicklistStore.removeList();
           }}
           className=Styles.barLink>
           {React.string("Cancel")}
         </a>
       | None =>
         <a
           href="#"
           onClick={e => {
             ReactEvent.Mouse.preventDefault(e);
             setVisibility(_ => Hidden);
           }}
           className=Styles.barLink>
           {React.string("Hide")}
         </a>
       }}
    </div>
    <div className=Styles.body>
      <div className=Styles.bodyList>
        {switch (quicklist) {
         | Some(quicklist) =>
           <div className=Styles.listImages>
             {quicklist.itemIds
              ->Belt.Array.map(((itemId, variant)) => {
                  let item = Item.getItem(~itemId);
                  <img
                    src={Item.getImageUrl(~item, ~variant)}
                    className=Styles.listImage
                    key={string_of_int(itemId) ++ string_of_int(variant)}
                  />;
                })
              ->React.array}
           </div>
         | None => React.null
         }}
      </div>
      <div className=Styles.saveRow>
        <button onClick={_ => {Js.log("hi")}} className=Styles.saveButton>
          {React.string("Save and generate URL")}
        </button>
      </div>
    </div>
  </div>;
};