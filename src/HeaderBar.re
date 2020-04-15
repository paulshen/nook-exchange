module Styles = {
  open Css;
  let root =
    style([
      display(flexBox),
      fontSize(px(16)),
      justifyContent(spaceBetween),
      padding3(~top=px(16), ~bottom=px(32), ~h=px(16)),
      media("(max-width: 600px)", [paddingBottom(zero)]),
    ]);
  let standardLink = style([media("(max-width: 600px)", [display(none)])]);
  let smallViewportLink =
    style([
      display(none),
      media("(max-width: 600px)", [display(inline)]),
    ]);
  let logoLink =
    style([
      position(absolute),
      left(pct(50.)),
      marginLeft(px(-100)),
      top(px(8)),
      media("(max-width: 600px)", [display(none)]),
    ]);
  let logo = style([width(px(200)), height(px(60))]);
  let navRight = style([display(flexBox)]);
  let logoutLink = style([marginLeft(px(16))]);
};

[@bs.module] external logo: string = "./assets/logo.png";

[@react.component]
let make = (~onLogin) => {
  let user = UserStore.useMe();
  <div className=Styles.root>
    <Link path="/" className=Styles.logoLink>
      <img src=logo className=Styles.logo alt="Nook Exchange" />
    </Link>
    <div>
      <Link path="/">
        <span className=Styles.standardLink>
          {React.string("Browse Items")}
        </span>
        <span className=Styles.smallViewportLink>
          {React.string("Nook Exchange")}
        </span>
      </Link>
    </div>
    <div className=Styles.navRight>
      {switch (user) {
       | Some(user) =>
         <>
           <div>
             <Link path={"/u/" ++ user.id}> {React.string(user.id)} </Link>
           </div>
           <div className=Styles.logoutLink>
             <a
               href="#"
               onClick={e => {
                 UserStore.logout() |> ignore;
                 ReactEvent.Mouse.preventDefault(e);
               }}>
               {React.string("Logout")}
             </a>
           </div>
         </>
       | None =>
         <a
           href="#"
           onClick={e => {
             onLogin();
             ReactEvent.Mouse.preventDefault(e);
           }}>
           {React.string("Login")}
         </a>
       }}
    </div>
  </div>;
};