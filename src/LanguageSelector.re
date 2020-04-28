module Modal = {
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
        backgroundColor(hex("6f8477d0")),
      ]);
    let root =
      style([
        backgroundColor(hex("ffffff")),
        padding2(~v=px(32), ~h=px(32)),
        borderRadius(px(4)),
        position(relative),
        maxWidth(px(448)),
        boxSizing(borderBox),
        width(pct(90.)),
        boxShadow(Shadow.box(~spread=px(12), rgba(0, 0, 0, 0.1))),
        overflow(auto),
        maxHeight(vh(100.)),
        media(
          "(max-width: 540px)",
          [paddingTop(px(24)), paddingBottom(px(24))],
        ),
      ]);
    let body = style([maxWidth(px(320)), margin2(~v=zero, ~h=auto)]);
    [@bs.module "./assets/close.png"] external closePng: string = "default";
    let closeButton =
      style([
        backgroundImage(url(closePng)),
        borderWidth(zero),
        padding(zero),
        cursor(pointer),
        height(px(16)),
        width(px(16)),
        position(absolute),
        top(px(16)),
        right(px(16)),
        opacity(0.5),
        hover([opacity(1.)]),
      ]);
    let label = style([marginBottom(px(12))]);
    let row = style([marginBottom(px(6))]);
    let languageLink =
      style([
        opacity(0.8),
        textDecoration(none),
        hover([textDecoration(underline)]),
      ]);
    let languageLinkSelected =
      style([opacity(1.), textDecoration(underline)]);
  };

  [@react.component]
  let make = (~onClose) => {
    let selectedLocale = SettingsStore.useLocale();
    <div className=Styles.overlay>
      <div className=Styles.backdrop onClick={_ => onClose()} />
      <div className=Styles.root>
        <div className=Styles.label>
          {React.string("Select a language:")}
        </div>
        <div>
          {SettingsStore.locales
           ->Belt.Array.map(locale => {
               <div
                 className=Styles.row key={SettingsStore.localeToJs(locale)}>
                 <a
                   href="#"
                   onClick={e => {
                     ReactEvent.Mouse.preventDefault(e);
                     SettingsStore.setLocale(~locale);
                     onClose();
                   }}
                   className={Cn.make([
                     Styles.languageLink,
                     Cn.ifTrue(
                       Styles.languageLinkSelected,
                       selectedLocale == locale,
                     ),
                   ])}>
                   {React.string(SettingsStore.localeToString(locale))}
                 </a>
               </div>
             })
           ->React.array}
        </div>
        <button onClick={_ => onClose()} className=Styles.closeButton />
      </div>
    </div>;
  };
};

let show = () => {
  let modalKey = ref(None);
  modalKey :=
    Some(
      ReactAtmosphere.API.pushLayer(~render=_ => {
        <Modal
          onClose={() =>
            ReactAtmosphere.API.removeLayer(
              ~key=Belt.Option.getExn(modalKey^),
            )
          }
        />
      }),
    );
};