let menuHeight = Css.px(52);

module Styles = {
  open Css;
  let wrapper = style([height(menuHeight)]);
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
      hover([backgroundColor(hex("fffffff0"))]),
    ]);
  let rootWithBackground = style([backgroundColor(hex("fffffff0"))]);
  let rootIsScrollingUp =
    style([
      media("(max-width: 600px)", [backgroundColor(hex("fffffff0"))]),
    ]);
  let rootIsNearTop = style([important(backgroundColor(transparent))]);
  let standardViewport =
    style([media("(max-width: 500px)", [display(none)])]);
  let smallViewport =
    style([
      important(display(none)),
      media("(max-width: 500px)", [important(display(inherit_))]),
    ]);
  let logoLink =
    style([
      position(absolute),
      left(pct(50.)),
      marginLeft(px(-100)),
      top(px(8)),
      zIndex(1),
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
  let twitterLink = style([]);
};

module Menu = {
  module MenuStyles = {
    open Css;
    let root =
      style([
        position(fixed),
        left(zero),
        top(menuHeight),
        backgroundColor(Colors.white),
        padding2(~v=px(8), ~h=zero),
        borderTopRightRadius(px(8)),
        borderBottomRightRadius(px(8)),
        overflow(hidden),
        minWidth(px(256)),
        boxSizing(borderBox),
        Colors.darkLayerShadow,
        zIndex(1),
        opacity(0.),
        transition(~duration=200, "all"),
        transform(translateX(px(-64))),
      ]);
    let rootAppear = style([opacity(1.), transform(translateX(zero))]);
    let menuItem =
      style([
        display(block),
        fontSize(px(16)),
        width(pct(100.)),
        cursor(pointer),
        padding2(~v=px(6), ~h=px(16)),
        boxSizing(borderBox),
        textDecoration(none),
        hover([backgroundColor(Colors.green), color(Colors.white)]),
      ]);
  };

  [@react.component]
  let make = (~onClose, ~user: option(User.t), ~onLogin) => {
    let (animateIn, setAnimateIn) = React.useState(() => false);
    React.useEffect0(() => {
      setAnimateIn(_ => true);
      let onClick = _ => {
        onClose();
      };
      open Webapi.Dom;
      window |> Window.addClickEventListener(onClick);
      Some(() => {window |> Window.removeClickEventListener(onClick)});
    });
    <div
      className={Cn.make([
        MenuStyles.root,
        Cn.ifTrue(MenuStyles.rootAppear, animateIn),
      ])}>
      <Link
        path="/"
        className={Cn.make([MenuStyles.menuItem, Styles.smallViewport])}>
        {React.string("Browse Items")}
      </Link>
      {switch (user) {
       | Some(user) =>
         <>
           <Link path={"/u/" ++ user.username} className=MenuStyles.menuItem>
             {React.string("My Profile")}
           </Link>
           <Link path="/catalog" className=MenuStyles.menuItem>
             {React.string("My Catalog")}
           </Link>
           <a
             href="https://twitter.com/nookexchange"
             target="_blank"
             className=MenuStyles.menuItem>
             {React.string("Twitter")}
           </a>
           <a
             href="#"
             className=MenuStyles.menuItem
             onClick={e => {
               UserStore.logout() |> ignore;
               ReactEvent.Mouse.preventDefault(e);
             }}>
             {React.string("Logout")}
           </a>
         </>
       | None =>
         <>
           <a
             href="#"
             onClick={e => {
               onLogin();
               ReactEvent.Mouse.preventDefault(e);
             }}
             className=MenuStyles.menuItem>
             {React.string("Login")}
           </a>
           <a
             href="https://twitter.com/nookexchange"
             target="_blank"
             className=MenuStyles.menuItem>
             {React.string("Twitter")}
           </a>
         </>
       }}
    </div>;
  };
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
    let newIsNearTop = scrollTop^ < 100.;
    if (newIsNearTop != isNearTop) {
      setIsNearTop(_ => newIsNearTop);
    };
    let isNearTop = ref(newIsNearTop);
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
  let (showMenu, setShowMenu) = React.useState(() => false);
  <div className=Styles.wrapper>
    <div
      className={Cn.make([
        Styles.root,
        Cn.ifTrue(Styles.rootWithBackground, showMenu),
        Cn.ifTrue(Styles.rootIsScrollingUp, isScrollingUp),
        Cn.ifTrue(Styles.rootIsNearTop, isNearTop),
      ])}>
      <div className={Cn.make([Styles.nav, Styles.navLeft])}>
        <div className=Styles.navLink>
          <a
            href="#"
            onClick={e => {
              ReactEvent.Mouse.preventDefault(e);
              setShowMenu(show => !show);
            }}>
            {React.string("Menu")}
          </a>
        </div>
        <div className=Styles.navLink>
          <Link path="/" className=Styles.standardViewport>
            {React.string("Browse Items")}
          </Link>
        </div>
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
           </>
         }}
        <div className={Cn.make([Styles.navLink, Styles.standardViewport])}>
          <a href="https://twitter.com/nookexchange" target="_blank">
            {React.string("Twitter")}
          </a>
        </div>
      </div>
    </div>
    <Link path="/" className=Styles.logoLink>
      <h1 className=Styles.logo> {React.string("Nook Exchange")} </h1>
    </Link>
    {showMenu
       ? <Menu user onLogin onClose={() => setShowMenu(_ => false)} />
       : React.null}
  </div>;
};