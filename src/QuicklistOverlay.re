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
      transition(~duration=250, "all"),
      transform(translate3d(zero, pct(100.), zero)),
    ]);
  let rootWithQuicklist = style([Colors.darkLayerShadow]);
  let introBar =
    style([
      display(flexBox),
      justifyContent(spaceBetween),
      alignItems(center),
      padding2(~v=px(8), ~h=px(8)),
      height(px(33)),
      transition(~duration=200, "all"),
    ]);
  let shown = style([transform(translate3d(zero, px(384), zero))]);
  let button =
    style([
      backgroundColor(Colors.white),
      color(Colors.green),
      borderWidth(zero),
      borderRadius(px(4)),
      padding2(~v=px(8), ~h=px(12)),
      cursor(pointer),
      disabled([opacity(0.8)]),
      hover([backgroundColor(Colors.white)]),
    ]);
  let barLink =
    style([
      color(Colors.white),
      marginRight(px(8)),
      textDecoration(none),
      hover([textDecoration(underline)]),
    ]);
  [@bs.module "./assets/down_caret.png"]
  external downCaretIcon: string = "default";
  let hidePanelLink =
    style([
      marginLeft(px(8)),
      textDecoration(none),
      hover([textDecoration(underline)]),
    ]);
  let hidePanelLinkCaret =
    style([
      backgroundImage(url(downCaretIcon)),
      backgroundSize(cover),
      display(inlineBlock),
      height(px(16)),
      width(px(16)),
      verticalAlign(`top),
    ]);
  let shownPanel =
    style([
      boxShadow(Shadow.box(~blur=px(24), hex("00000080"))),
      transform(translate3d(zero, zero, zero)),
      backgroundColor(Colors.white),
      color(Colors.charcoal),
      selector("& ." ++ barLink, [color(Colors.linkGreen)]),
      selector(
        "& ." ++ introBar,
        [borderBottom(px(1), solid, Colors.faintGray)],
      ),
    ]);
  let body =
    style([
      height(px(384)),
      boxSizing(borderBox),
      display(flexBox),
      flexDirection(column),
    ]);
  let bodyListWrapper =
    style([flexGrow(1.), overflow(hidden), position(relative)]);
  let bodyList = style([height(pct(100.)), overflow(auto)]);
  let listImages =
    style([
      display(flexBox),
      flexWrap(wrap),
      paddingTop(px(16)),
      paddingLeft(px(16)),
      paddingBottom(px(16)),
      position(relative),
    ]);
  let listImagesScrollFade =
    style([
      backgroundImage(
        linearGradient(
          deg(0.),
          [(zero, hex("ffffffff")), (pct(100.), hex("ffffff00"))],
        ),
      ),
      position(absolute),
      bottom(zero),
      left(zero),
      right(zero),
      height(px(16)),
    ]);
  let listImage =
    style([
      display(block),
      width(px(64)),
      height(px(64)),
      cursor(pointer),
      media(
        "(hover: hover)",
        [hover([backgroundColor(Colors.faintGray)])],
      ),
    ]);
  let emptyList = style([paddingTop(px(32)), textAlign(center)]);
  let saveRow =
    style([
      backgroundColor(Colors.white),
      display(flexBox),
      flexDirection(column),
      padding2(~v=px(8), ~h=px(16)),
    ]);
  let saveButton =
    style([
      padding2(~v=px(12), ~h=zero),
      backgroundColor(Colors.green),
      color(Colors.white),
      borderRadius(px(10)),
      width(pct(100.)),
    ]);
};

type visibility =
  | Hidden
  | Bar
  | Panel;

module CreateDialog = {
  module Styles = {
    open Css;
    let body = style([padding(px(16))]);
    let title = style([fontSize(px(24)), marginBottom(px(8))]);
    let listUrl =
      style([
        width(pct(100.)),
        padding2(~v=px(12), ~h=px(8)),
        backgroundColor(transparent),
        border(px(2), dashed, hex("bae8cc")),
        borderRadius(px(4)),
        textAlign(center),
        cursor(pointer),
        outlineStyle(none),
        margin2(~v=px(16), ~h=zero),
      ]);
    let listUrlCopied = style([borderColor(hex("3cb56c"))]);
  };

  [@bs.module] external copyToClipboard: string => unit = "copy-to-clipboard";

  module ConfirmModal = {
    [@react.component]
    let make = (~onClose) => {
      let (hasCopied, setHasCopied) = React.useState(() => false);
      <Modal>
        <div className=Styles.body>
          <div className=Styles.title> {React.string("List created!")} </div>
          <div>
            {React.string("This is your URL. Tap to copy. ")}
            <a href="https://nook.exchange" target="_blank">
              {React.string("Open in new tab.")}
            </a>
          </div>
          <button
            className={Cn.make([
              Styles.listUrl,
              Cn.ifTrue(Styles.listUrlCopied, hasCopied),
            ])}
            onClick={_ => {
              copyToClipboard("https://nook.exchange/u/kFb8X");
              setHasCopied(_ => true);
            }}>
            {React.string("https://nook.exchange/u/kFb8X")}
          </button>
          {if (UserStore.isLoggedIn()) {
             <div>
               {React.string(
                  {j|Lists are saved to your account. The page is coming soon 🙃|j},
                )}
             </div>;
           } else {
             <div>
               {React.string("Create an account to save your lists!")}
             </div>;
           }}
        </div>
        <Modal.FooterBar>
          <Button onClick={_ => onClose()}> {React.string("Okay")} </Button>
        </Modal.FooterBar>
      </Modal>;
    };
  };

  let show = () => {
    let modalKey = ref(None);
    let onClose = () =>
      ReactAtmosphere.API.removeLayer(~key=Belt.Option.getExn(modalKey^));
    modalKey :=
      Some(
        ReactAtmosphere.API.pushLayer(~render=_ => <ConfirmModal onClose />),
      );
  };
};

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
        quicklist != None
        || visibility != Hidden
        && ,
      ),
      Cn.ifTrue(Styles.shownPanel, visibility == Panel),
      Cn.ifTrue(Styles.rootWithQuicklist, quicklist != None),
    ])}>
    <div className=Styles.introBar>
      {visibility == Panel
         ? <a
             href="#"
             onClick={e => {
               ReactEvent.Mouse.preventDefault(e);
               setVisibility(_ => Bar);
             }}
             className=Styles.hidePanelLink>
             <span className=Styles.hidePanelLinkCaret />
             {React.string("Close Panel")}
           </a>
         : (
           switch (quicklist) {
           | Some(quicklist) =>
             let numItems = quicklist.itemIds->Js.Array.length;
             <button
               onClick={_ => {setVisibility(_ => Panel)}}
               disabled={Js.Array.length(quicklist.itemIds) == 0}
               className=Styles.button>
               {React.string(
                  numItems > 0
                    ? "See "
                      ++ string_of_int(numItems)
                      ++ " item"
                      ++ (numItems == 1 ? "" : "s")
                    : "Select items!",
                )}
             </button>;
           | None =>
             <button
               onClick={_ => {QuicklistStore.startList()}}
               className=Styles.button>
               {React.string("Share a list of items")}
             </button>
           }
         )}
      {switch (quicklist) {
       | Some(quicklist) =>
         <a
           href="#"
           onClick={e => {
             ReactEvent.Mouse.preventDefault(e);
             let numItems = Js.Array.length(quicklist.itemIds);
             if (numItems > 1) {
               ConfirmDialog.confirm(
                 ~bodyText=
                   "You have "
                   ++ string_of_int(numItems)
                   ++ " items that will be discarded. Are you sure?",
                 ~confirmLabel="Yes, discard list",
                 ~cancelLabel="Not yet",
                 ~onConfirm=() => QuicklistStore.removeList(),
                 (),
               );
             } else {
               QuicklistStore.removeList();
             };
             setVisibility(_ => Bar);
           }}
           className=Styles.barLink>
           {React.string("Discard list")}
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
      <div className=Styles.bodyListWrapper>
        <div className=Styles.bodyList>
          {switch (quicklist) {
           | Some(quicklist) =>
             if (Js.Array.length(quicklist.itemIds) > 0) {
               <div className=Styles.listImages>
                 {quicklist.itemIds
                  ->Belt.Array.map(((itemId, variant)) => {
                      let item = Item.getItem(~itemId);
                      <ReactAtmosphere.Tooltip
                        text={React.string("Tap to remove")}
                        options={
                          placement: Some("top"),
                          modifiers:
                            Some([|
                              {
                                "name": "offset",
                                "options": {
                                  "offset": [|0, 4|],
                                },
                              },
                            |]),
                        }
                        key={string_of_int(itemId) ++ string_of_int(variant)}>
                        {(
                           ({onMouseEnter, onMouseLeave, ref}) =>
                             <img
                               src={Item.getImageUrl(~item, ~variant)}
                               className=Styles.listImage
                               onMouseEnter
                               onMouseLeave
                               onClick={_ => {
                                 QuicklistStore.removeItem(~itemId, ~variant)
                               }}
                               ref={ReactDOMRe.Ref.domRef(ref)}
                             />
                         )}
                      </ReactAtmosphere.Tooltip>;
                    })
                  ->React.array}
               </div>;
             } else {
               <div className=Styles.emptyList>
                 {React.string("Your list is empty!")}
               </div>;
             }
           | None => React.null
           }}
        </div>
        <div className=Styles.listImagesScrollFade />
      </div>
      <div className=Styles.saveRow>
        <button
          onClick={_ => {
            // QuicklistStore.saveList() |> ignore;
            CreateDialog.show()
          }}
          disabled={
            switch (quicklist) {
            | Some(quicklist) => Js.Array.length(quicklist.itemIds) == 0
            | None => true
            }
          }
          className=Styles.saveButton>
          {React.string("Share quick list")}
        </button>
      </div>
    </div>
  </div>;
};