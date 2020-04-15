module Styles = {
  open Css;
  let emptyProfile =
    style([
      maxWidth(px(768)),
      margin3(~top=px(32), ~bottom=px(128), ~h=auto),
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
      backgroundColor(hex("ffffffc0")),
      boxSizing(borderBox),
      lineHeight(px(20)),
      margin2(~v=px(16), ~h=auto),
      maxWidth(px(512)),
      padding2(~v=px(16), ~h=px(24)),
      borderRadius(px(8)),
      media("(max-width: 512px)", [borderRadius(zero), padding(px(16))]),
    ]);
  let url = style([]);
  let bodyText = style([fontSize(px(18))]);
};

module ProfileTextarea = {
  module Styles = {
    open Css;
    let root = style([display(flexBox), flexDirection(column)]);
    let textarea =
      style([
        backgroundColor(hex("E6F2E8")),
        border(px(1), solid, transparent),
        borderRadius(px(4)),
        padding(px(8)),
        height(px(64)),
        boxSizing(borderBox),
        outlineStyle(none),
        fontSize(px(14)),
        width(pct(100.)),
        transition(~duration=200, "all"),
        width(pct(100.)),
        placeholder([opacity(0.7)]),
        marginBottom(px(8)),
        focus([
          backgroundColor(Colors.white),
          borderColor(rgba(0, 0, 0, 0.1)),
        ]),
      ]);
    let removeLink =
      style([
        alignSelf(flexEnd),
        position(relative),
        top(px(-2)),
        fontSize(px(12)),
        lineHeight(px(12)),
        opacity(0.5),
        marginBottom(px(-8)),
        hover([opacity(0.8)]),
        media("(max-width: 512px)", [marginBottom(zero)]),
      ]);
  };

  [@react.component]
  let make = (~user: User.t) => {
    let (profileText, setProfileText) =
      React.useState(() => user.profileText);
    let updateProfileText = () => UserStore.updateProfileText(~profileText);
    let updateProfileTextRef = React.useRef(updateProfileText);
    React.useEffect(() => {
      React.Ref.setCurrent(updateProfileTextRef, updateProfileText);
      None;
    });
    let throttleUpdateTimeoutRef = React.useRef(None);
    React.useEffect0(() => {
      Some(
        () => {
          switch (React.Ref.current(throttleUpdateTimeoutRef)) {
          | Some(throttleUpdateTimeout) =>
            Js.Global.clearTimeout(throttleUpdateTimeout)
          | None => ()
          }
        },
      )
    });

    <div className=Styles.root>
      <textarea
        value=profileText
        placeholder="Add a welcome message and your Discord username!\nMaybe a note about your lists below?"
        className=Styles.textarea
        onChange={e => {
          let value = ReactEvent.Form.target(e)##value;
          setProfileText(_ => value);

          switch (React.Ref.current(throttleUpdateTimeoutRef)) {
          | Some(throttleUpdateTimeout) =>
            Js.Global.clearTimeout(throttleUpdateTimeout)
          | None => ()
          };
          React.Ref.setCurrent(
            throttleUpdateTimeoutRef,
            Some(
              Js.Global.setTimeout(
                () => {
                  React.Ref.setCurrent(throttleUpdateTimeoutRef, None);
                  React.Ref.current(updateProfileTextRef, ());
                },
                1000,
              ),
            ),
          );
        }}
      />
      {user.profileText != ""
         ? <a
             href="#"
             onClick={e => {
               setProfileText(_ => "");
               UserStore.updateProfileText(~profileText="");
               ReactEvent.Mouse.preventDefault(e);
             }}
             className=Styles.removeLink>
             {React.string("Remove note")}
           </a>
         : React.null}
    </div>;
  };
};

[@react.component]
let make = (~user: User.t) =>
  <div>
    <div className=Styles.username> {React.string(user.id)} </div>
    <div className=Styles.userBody>
      <ProfileTextarea user />
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