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
    let root =
      style([
        display(flexBox),
        flexDirection(column),
        marginBottom(px(8)),
      ]);
    let textarea =
      style([
        backgroundColor(hex("f6f6f6")),
        border(px(1), solid, transparent),
        borderRadius(px(4)),
        padding(px(7)),
        height(px(64)),
        boxSizing(borderBox),
        outlineStyle(none),
        fontSize(px(14)),
        lineHeight(px(18)),
        width(pct(100.)),
        transition(~duration=200, "all"),
        width(pct(100.)),
        placeholder([opacity(0.7)]),
        marginBottom(px(6)),
        focus([
          backgroundColor(Colors.white),
          borderColor(rgba(0, 0, 0, 0.1)),
        ]),
      ]);
    let profileTextPreview =
      style([
        backgroundColor(hex("f6f6f6")),
        borderRadius(px(4)),
        padding4(~top=px(8), ~bottom=px(8), ~left=px(8), ~right=px(32)),
        fontSize(px(14)),
        lineHeight(px(18)),
        marginBottom(px(6)),
        cursor(pointer),
        whiteSpace(`preLine),
        transition(~duration=200, "all"),
      ]);
    let updateBar = style([justifyContent(flexEnd)]);
    let previewWrapper =
      style([
        position(relative),
        hover([
          selector(
            "& ." ++ profileTextPreview,
            [backgroundColor(hex("f0f0f0"))],
          ),
        ]),
      ]);
    let editLink =
      style([
        color(hex("a0a0a0")),
        fontSize(px(12)),
        opacity(0.8),
        textDecoration(none),
        position(absolute),
        top(px(8)),
        right(px(8)),
        hover([opacity(1.), textDecoration(underline)]),
      ]);
  };

  [@react.component]
  let make = (~user: User.t) => {
    let (isEditing, setIsEditing) = React.useState(() => false);
    let (profileText, setProfileText) =
      React.useState(() => user.profileText);

    let textareaRef = React.useRef(Js.Nullable.null);
    React.useEffect1(
      () => {
        if (isEditing) {
          open Webapi.Dom;
          let textareaElement = Utils.getElementForDomRef(textareaRef);
          textareaElement->Element.unsafeAsHtmlElement->HtmlElement.focus;
        };
        None;
      },
      [|isEditing|],
    );

    <div className=Styles.root>
      {isEditing || user.profileText == ""
         ? <textarea
             value=profileText
             placeholder="Add a welcome message and your Discord username!\nYou can use emoji :nmt: and :bell:"
             className=Styles.textarea
             onChange={e => {
               let value = ReactEvent.Form.target(e)##value;
               setProfileText(_ => value);
             }}
             ref={ReactDOMRe.Ref.domRef(textareaRef)}
           />
         : <div className=Styles.previewWrapper>
             <div
               onClick={_ => {setIsEditing(_ => true)}}
               className=Styles.profileTextPreview>
               {Emoji.parseText(user.profileText)}
             </div>
             <a
               href="#"
               onClick={e => {
                 setIsEditing(_ => true);
                 ReactEvent.Mouse.preventDefault(e);
               }}
               className=Styles.editLink>
               {React.string("Edit")}
             </a>
           </div>}
      {isEditing || profileText != user.profileText
         ? <div
             className={Cn.make([
               UserItemNote.Styles.updateBar,
               Styles.updateBar,
             ])}>
             <Button
               small=true
               onClick={_ => {
                 UserStore.updateProfileText(~profileText);
                 setIsEditing(_ => false);
               }}>
               {React.string("Save")}
             </Button>
             <a
               href="#"
               onClick={e => {
                 setProfileText(_ => user.profileText);
                 setIsEditing(_ => false);
                 ReactEvent.Mouse.preventDefault(e);
               }}
               className=UserItemNote.Styles.cancelLink>
               {React.string("Cancel")}
             </a>
           </div>
         : React.null}
    </div>;
  };
};

module Loaded = {
  [@react.component]
  let make = (~user: User.t, ~url) =>
    <div>
      <div className=Styles.username> {React.string(user.username)} </div>
      <div className=Styles.userBody>
        <ProfileTextarea user />
        <div className=Styles.url>
          {React.string("Preview your profile: ")}
          <Link path={"/u/" ++ user.username}>
            {React.string("https://nook.exchange/u/" ++ user.username)}
          </Link>
        </div>
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
           <ItemBrowser showLogin={() => ()} url />
         </>;
       }}
    </div>;
};

[@react.component]
let make = (~url) => {
  let me = UserStore.useMe();
  switch (me) {
  | Some(user) => <Loaded user url />
  | None => React.null
  };
};