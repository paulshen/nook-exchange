module Styles = {
  open Css;
  let wrapper = style([height(px(52))]);
  let root =
    style([
      position(fixed),
      left(zero),
      right(zero),
      top(zero),
      display(flexBox),
      flexWrap(wrap),
      fontSize(px(16)),
      justifyContent(spaceBetween),
      padding3(~top=px(16), ~bottom=zero, ~h=px(16)),
      zIndex(1),
      transition(~duration=400, "all"),
      selector(
        "& a",
        [textDecoration(none), hover([textDecoration(underline)])],
      ),
      media("(max-width: 600px)", [paddingBottom(zero)]),
      hover([backgroundColor(hex("ffffffc0"))]),
    ]);
  let rootWithBackground = style([backgroundColor(hex("ffffffc0"))]);
  let rootIsNearTop = style([important(backgroundColor(transparent))]);
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
      media("(max-width: 680px)", [display(none)]),
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
  let navLeft = style([marginBottom(px(16)), marginRight(px(16))]);
  let navLink =
    style([
      color(Colors.gray),
      marginLeft(px(16)),
      firstChild([marginLeft(zero)]),
    ]);
  let logoutLink = style([media("(max-width: 420px)", [display(none)])]);
  let twitterLink = style([]);
};

[@react.component]
let make = (~onLogin) => {
  let user = UserStore.useMe();
  let (isNearTop, setIsNearTop) =
    React.useState(() => {Webapi.Dom.(window |> Window.pageYOffset < 100.)});
  let (isScrollingUp, setIsScrollingUp) = React.useState(() => false);
  React.useEffect0(() => {
    open Webapi.Dom;
    let scrollTop = ref(window |> Window.pageYOffset);
    let isScrollingUp = ref(false);
    let isNearTop = ref(scrollTop^ < 100.);
    let onScroll = e => {
      let newScrollTop = window |> Window.pageYOffset;
      let newIsScrollingUp = newScrollTop < scrollTop^;
      let newIsNearTop = newScrollTop < 100.;
      if (newIsScrollingUp != isScrollingUp^) {
        setIsScrollingUp(_ => newIsScrollingUp);
      };
      if (newIsNearTop != isNearTop^) {
        setIsNearTop(_ => newIsNearTop);
      };
      scrollTop := newScrollTop;
      isScrollingUp := newIsScrollingUp;
      isNearTop := newIsNearTop;
    };
    window |> Window.addEventListener("scroll", onScroll);
    Some(() => {window |> Window.removeEventListener("scroll", onScroll)});
  });
  <div className=Styles.wrapper>
    <div
      className={Cn.make([
        Styles.root,
        Cn.ifTrue(Styles.rootWithBackground, isScrollingUp),
        Cn.ifTrue(Styles.rootIsNearTop, isNearTop),
      ])}>
      <div className={Cn.make([Styles.nav, Styles.navLeft])}>
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
               <Link path={"/u/" ++ user.username}>
                 {React.string(user.username)}
               </Link>
             </div>
             {if (Js.Dict.values(user.items)
                  ->Belt.Array.some(userItem =>
                      switch (userItem.status) {
                      | ForTrade
                      | CanCraft
                      | CatalogOnly => true
                      | Wishlist => false
                      }
                    )) {
                <div className=Styles.navLink>
                  <Link path="/catalog"> {React.string("Catalog")} </Link>
                </div>;
              } else {
                React.null;
              }}
             <div className={Cn.make([Styles.navLink, Styles.twitterLink])}>
               <a href="https://twitter.com/nookexchange" target="_blank">
                 {React.string("Twitter")}
               </a>
             </div>
             <div className={Cn.make([Styles.navLink, Styles.logoutLink])}>
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
           <>
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
             <div className=Styles.navLink>
               <a href="https://twitter.com/nookexchange" target="_blank">
                 {React.string("Twitter")}
               </a>
             </div>
           </>
         }}
      </div>
    </div>
    <Link path="/" className=Styles.logoLink>
      <h1 className=Styles.logo> {React.string("Nook Exchange")} </h1>
    </Link>
  </div>;
};