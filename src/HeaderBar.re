module Styles = {
  open Css;
  let root =
    style([
      display(flexBox),
      fontSize(px(16)),
      justifyContent(spaceBetween),
      padding3(~top=px(16), ~bottom=px(16), ~h=px(16)),
      selector(
        "& a",
        [textDecoration(none), hover([textDecoration(underline)])],
      ),
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
  [@bs.module "./assets/logo.png"] external logo: string = "default";
  let logo =
    style([
      backgroundImage(url(logo)),
      backgroundSize(cover),
      margin(zero),
      width(px(200)),
      height(px(60)),
      textIndent(px(-9999)),
    ]);
  let nav = style([display(flexBox)]);
  let navLink = style([color(Colors.gray), marginLeft(px(16))]);
  let requestFeatureLink =
    style([media("(max-width: 800px)", [display(none)])]);
};

[@react.component]
let make = (~onLogin) => {
  let user = UserStore.useMe();
  <div className=Styles.root>
    <Link path="/" className=Styles.logoLink>
      <h1 className=Styles.logo> {React.string("Nook Exchange")} </h1>
    </Link>
    <div className=Styles.nav>
      <Link path="/">
        <span className=Styles.standardLink>
          {React.string("Browse items")}
        </span>
        <span className=Styles.smallViewportLink>
          {React.string("Nook Exchange")}
        </span>
      </Link>
    </div>
    <div className=Styles.nav>
      {switch (user) {
       | Some(user) =>
         <>
           <div className=Styles.navLink>
             <Link path="/me"> {React.string("My Profile")} </Link>
             {React.string(" (")}
             <Link path={"/u/" ++ user.username}>
               {React.string(user.username)}
             </Link>
             {React.string(")")}
           </div>
           <div
             className={Cn.make([Styles.navLink, Styles.requestFeatureLink])}>
             <a href="https://twitter.com/nookexchange" target="_blank">
               {React.string("Twitter")}
             </a>
           </div>
           <div className=Styles.navLink>
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
         <div className=Styles.navLink>
           <a
             href="#"
             onClick={e => {
               onLogin();
               ReactEvent.Mouse.preventDefault(e);
             }}>
             {React.string("Login")}
           </a>
         </div>
       }}
    </div>
  </div>;
};