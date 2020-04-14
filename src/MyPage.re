module Styles = {
  open Css;
  let emptyProfile =
    style([
      maxWidth(px(768)),
      margin3(~top=zero, ~bottom=px(128), ~h=auto),
      textAlign(center),
    ]);
  let username =
    style([fontSize(px(36)), textAlign(center), marginBottom(px(32))]);
  let bodyText = style([fontSize(px(18))]);
};

[@react.component]
let make = (~user: User.t) =>
  <div>
    <div className=Styles.username> {React.string(user.id)} </div>
    {if (user.items->Js.Dict.keys->Array.length > 0) {
       <UserItemBrowser
         userItems={
           user.items
           ->Js.Dict.entries
           ->Belt.Array.mapU((. (itemKey, item)) =>
               (User.fromItemKey(~key=itemKey), item)
             )
         }
         editable=true
       />;
     } else {
       <>
         <div className=Styles.emptyProfile>
           <div className=Styles.bodyText>
             <div> {React.string("Your profile is empty!")} </div>
             <div> {React.string("Add your first item below.")} </div>
           </div>
         </div>
         <ItemBrowser showLogin={() => ()} />
       </>;
     }}
  </div>;