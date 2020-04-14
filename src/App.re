module HeaderBar = {
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
               <Link path={"/u/" ++ user.id}> {React.string(user.id)} </Link>
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
};

[@react.component]
let make = () => {
  let url = ReasonReactRouter.useUrl();
  let (showLogin, setShowLogin) = React.useState(() => false);
  <>
    <HeaderBar onLogin={_ => setShowLogin(_ => true)} />
    {switch (url.path) {
     | ["u", userId] => <UserPage userId />
     | _ => <ItemBrowser />
     }}
    {showLogin
       ? <LoginOverlay onClose={() => setShowLogin(_ => false)} /> : React.null}
  </>;
};