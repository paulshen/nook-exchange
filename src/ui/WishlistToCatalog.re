module PersistConfig = {
  let key = "confirm_wishlist_to_catalog";
  let value = ref(Dom.Storage.localStorage |> Dom.Storage.getItem(key));
  let confirm = () => {
    let nowString = Js.Date.now()->Js.Float.toString;
    value := Some(nowString);
    Dom.Storage.localStorage |> Dom.Storage.setItem(key, nowString);
  };
};

module Styles = {
  open Css;
  let overlay =
    style([
      position(fixed),
      top(zero),
      bottom(zero),
      left(zero),
      right(zero),
      display(flexBox),
      alignItems(center),
      justifyContent(center),
    ]);
  let backdrop =
    style([
      position(absolute),
      top(zero),
      bottom(zero),
      left(zero),
      right(zero),
      backgroundColor(hex("6f847780")),
    ]);
  let root =
    style([
      backgroundColor(hex("ffffff")),
      padding2(~v=px(16), ~h=px(16)),
      borderRadius(px(4)),
      position(relative),
      maxWidth(px(400)),
      boxSizing(borderBox),
      width(pct(90.)),
      boxShadow(Shadow.box(~spread=px(12), rgba(0, 0, 0, 0.1))),
      overflow(auto),
      maxHeight(vh(100.)),
      // media(
      //   "(max-width: 540px)",
      //   [paddingTop(px(24)), paddingBottom(px(24))],
      // ),
    ]);
  let text = style([padding(px(8))]);
  let buttonRow =
    style([
      display(flexBox),
      alignItems(center),
      justifyContent(flexEnd),
      paddingTop(px(16)),
    ]);
  let confirmButton = style([marginLeft(px(16))]);
  let notNowLink =
    style([
      opacity(0.8),
      transition(~duration=200, "all"),
      hover([opacity(1.)]),
    ]);
};

let confirm = (~onConfirm) =>
  if (Belt.Option.isSome(PersistConfig.value^)) {
    onConfirm();
  } else {
    let modalKey = ref(None);
    let closeModal = () => {
      ReactAtmosphere.API.removeLayer(~key=Belt.Option.getExn(modalKey^));
    };
    modalKey :=
      Some(
        ReactAtmosphere.API.pushLayer(~render=_ => {
          <div className=Styles.overlay>
            <div className=Styles.backdrop onClick={_ => closeModal()} />
            <div className=Styles.root>
              <div className=Styles.text>
                {React.string(
                   "Adding an item to your catalog will remove it from your Wishlist. Do you want to continue?",
                 )}
              </div>
              <div className=Styles.buttonRow>
                <a
                  href="#"
                  onClick={e => {
                    ReactEvent.Mouse.preventDefault(e);
                    closeModal();
                  }}
                  className=Styles.notNowLink>
                  {React.string("Not now")}
                </a>
                <Button
                  onClick={_ => {
                    onConfirm();
                    PersistConfig.confirm();
                    closeModal();
                  }}
                  className=Styles.confirmButton>
                  {React.string("Move to catalog")}
                </Button>
              </div>
            </div>
          </div>
        }),
      );
  };