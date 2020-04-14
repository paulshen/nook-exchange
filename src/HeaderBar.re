module Styles = {
  open Css;
  let root = style([display(flexBox), justifyContent(spaceBetween)]);
};

[@react.component]
let make = (~onLogin) => {
  let user = UserStore.useMe();
  <div className=Styles.root>
    <div> <Link path="/"> {React.string("Tanukichi")} </Link> </div>
    <div>
      {switch (user) {
       | Some(user) =>
         <div>
           <div>
             <Link path={"/u/" ++ user.id}>
               {React.string("My Profile")}
             </Link>
             {React.string(" (" ++ user.id ++ ")")}
           </div>
           <div>
             <a
               href="#"
               onClick={e => {
                 UserStore.logout() |> ignore;
                 ReactEvent.Mouse.preventDefault(e);
               }}>
               {React.string("Logout")}
             </a>
           </div>
         </div>
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