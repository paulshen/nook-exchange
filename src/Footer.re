module Styles = {
  open Css;
  let footer =
    style([
      display(flexBox),
      flexDirection(column),
      fontSize(px(12)),
      alignItems(center),
      textAlign(center),
      margin3(~top=px(96), ~bottom=px(48), ~h=zero),
    ]);
  let contents =
    style([
      color(Colors.gray),
      lineHeight(px(16)),
      maxWidth(px(512)),
      padding2(~v=zero, ~h=px(8)),
    ]);
  let logoutLink =
    style([
      display(none),
      media("(max-width: 420px)", [display(inline)]),
    ]);
  let disclaimer = style([marginTop(px(8))]);
};

[@react.component]
let make = () => {
  let isLoggedIn = UserStore.useMe() !== None;
  let locale = SettingsStore.useLocale();
  <div className=Styles.footer>
    <div className=Styles.contents>
      <div>
        <a href="https://twitter.com/nookexchange" target="_blank">
          {React.string("Twitter")}
        </a>
        {React.string(" | ")}
        <a
          href="https://docs.google.com/spreadsheets/d/13d_LAJPlxMa_DubPTuirkIV4DERBMXbrWQsmSh8ReK4/edit?usp=sharing"
          target="_blank">
          {React.string("Data Source")}
        </a>
        {React.string(" | ")}
        <a
          href="https://github.com/alexislours/acnh-translations"
          target="_blank">
          {React.string("Translations")}
        </a>
        {React.string(" | ")}
        <span>
          {React.string("Language: ")}
          <a
            href="#"
            onClick={e => {
              ReactEvent.Mouse.preventDefault(e);
              LanguageSelector.show();
            }}>
            {React.string(SettingsStore.localeToString(locale))}
          </a>
        </span>
        {isLoggedIn
           ? <span className=Styles.logoutLink>
               {React.string(" | ")}
               <a
                 href="#"
                 onClick={e => {
                   UserStore.logout() |> ignore;
                   ReactEvent.Mouse.preventDefault(e);
                 }}>
                 {React.string("Logout")}
               </a>
             </span>
           : React.null}
      </div>
      <div>
        <a href="/terms"> {React.string("Terms of Service")} </a>
        {React.string(" | ")}
        <a href="/privacy"> {React.string("Privacy Policy")} </a>
        {React.string(" | Thanks for visiting!")}
      </div>
      <div className=Styles.disclaimer>
        {React.string(
           "Animal Crossing is a registered trademark of Nintendo. Nook Exchange in no way claims ownership of any intellectual property associated with Animal Crossing.",
         )}
      </div>
    </div>
  </div>;
};