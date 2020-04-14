module Styles = {
  open Css;
  let root =
    style([
      display(flexBox),
      justifyContent(spaceBetween),
      padding2(~v=px(16), ~h=px(16)),
    ]);
  let title = style([fontSize(px(16))]);
  let navRight = style([display(flexBox)]);
  let logoutLink = style([marginLeft(px(16))]);
};

[@react.component]
let make = (~onLogin) => {
  let user = UserStore.useMe();
  <div className=Styles.root>
    <div>
      <Link path="/" className=Styles.title>
        {React.string("Tanukichi")}
      </Link>
    </div>
    <div className=Styles.navRight>
      {switch (user) {
       | Some(user) =>
         <>
           <div>
             <Link path={"/u/" ++ user.id}>
               {React.string("My Profile")}
             </Link>
             {React.string(" (" ++ user.id ++ ")")}
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