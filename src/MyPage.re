module Styles = {
  open Css;
  let emptyProfile =
    style([
      maxWidth(px(768)),
      margin3(~top=zero, ~bottom=px(128), ~h=auto),
      textAlign(center),
    ]);
  let username =
    style([
      fontSize(px(36)),
      textAlign(center),
      marginTop(px(32)),
      marginBottom(px(32)),
    ]);
  let userBody =
    style([
      backgroundColor(hex("ffffff80")),
      boxSizing(borderBox),
      lineHeight(px(20)),
      margin2(~v=px(16), ~h=auto),
      maxWidth(px(512)),
      padding(px(16)),
      borderRadius(px(8)),
      media("(max-width: 512px)", [borderRadius(zero)]),
    ]);
  let url = style([]);
  let bodyText = style([fontSize(px(18))]);
};

[@react.component]
let make = (~user: User.t) =>
  <div>
    <div className=Styles.username> {React.string(user.id)} </div>
    <div className=Styles.userBody>
      <div className=Styles.url>
        {React.string("Share your URL: ")}
        <a href={"https://nook.exchange/u/" ++ user.id}>
          {React.string("https://nook.exchange/u/" ++ user.id)}
        </a>
      </div>
      <div> {React.string("Only you can edit your items!")} </div>
    </div>
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