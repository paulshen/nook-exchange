module Styles = {
  open Css;
  let body =
    style([
      boxSizing(borderBox),
      maxWidth(px(400)),
      padding2(~v=px(32), ~h=px(32)),
      width(vw(90.)),
    ]);
  let input =
    style([
      backgroundColor(rgba(0, 0, 0, 0.05)),
      border(px(1), solid, transparent),
      borderRadius(px(4)),
      padding2(~v=px(10), ~h=px(12)),
      boxSizing(borderBox),
      outlineStyle(none),
      fontSize(px(16)),
      width(pct(100.)),
      marginBottom(px(16)),
      transition(~duration=200, "all"),
      focus([
        backgroundColor(Colors.white),
        borderColor(rgba(0, 0, 0, 0.15)),
        boxShadow(Shadow.box(~spread=px(4), rgba(0, 0, 0, 0.05))),
      ]),
      disabled([color(Colors.gray)]),
    ]);
  let submitBar =
    style([display(flexBox), alignItems(center), justifyContent(flexEnd)]);
  let successMessage = style([flexGrow(1.), color(Colors.green)]);
  let submitButton =
    style([
      backgroundColor(Colors.green),
      borderWidth(zero),
      borderRadius(px(4)),
      color(Colors.white),
      cursor(pointer),
      marginLeft(px(16)),
      padding2(~v=px(8), ~h=px(16)),
      fontSize(px(16)),
      transition(~duration=200, "all"),
      disabled([opacity(0.5)]),
    ]);
  let title =
    style([fontSize(px(20)), textAlign(center), marginBottom(px(24))]);
  let blurb = style([marginBottom(px(16))]);
  let urlPreview =
    style([
      fontSize(px(12)),
      letterSpacing(pxFloat(0.3)),
      color(hex("b0b0b0")),
      marginBottom(px(8)),
    ]);
  let errorMessage =
    style([marginTop(px(-10)), marginBottom(px(16)), color(Colors.red)]);
  let discordSection =
    style([
      borderBottom(px(1), dashed, Colors.lightGray),
      marginBottom(px(16)),
      paddingBottom(px(16)),
    ]);
  let discordButton = style([marginTop(px(16))]);
};

type submitStatus =
  | Success
  | Error(string);

module WithUser = {
  [@react.component]
  let make = (~user: User.t, ~onClose) => {
    let (username, setUsername) = React.useState(() => user.username);
    let (password, setPassword) = React.useState(() => "");
    let (oldPassword, setOldPassword) = React.useState(() => "");
    let hasChanges = user.username != username || password != "";
    let (isSubmitting, setIsSubmitting) = React.useState(() => false);
    let (status, setStatus) = React.useState(() => None);

    React.useEffect0(() => {
      Analytics.Amplitude.logEvent(~eventName="Settings Viewed");
      None;
    });

    let onSubmit = e => {
      ReactEvent.Form.preventDefault(e);
      {
        let prevUsername = user.username;
        setIsSubmitting(_ => true);
        let usernameUpdate =
          username == user.username ? None : Some(username);
        let%Repromise result =
          UserStore.patchMe(
            ~username=?usernameUpdate,
            ~newPassword=?password != "" ? Some(password) : None,
            ~oldPassword=?oldPassword != "" ? Some(oldPassword) : None,
            (),
          );
        setIsSubmitting(_ => false);
        switch (result) {
        | Ok () =>
          setStatus(_ => Some(Success));
          setPassword(_ => "");
          setOldPassword(_ => "");
        | Error(err) => setStatus(_ => Some(Error(err)))
        };
        switch (usernameUpdate) {
        | Some(username) =>
          let url = ReasonReactRouter.dangerouslyGetInitialUrl();
          switch (url.path) {
          | ["u", pathUsername, ..._] =>
            if (Js.String.toLowerCase(pathUsername)
                == Js.String.toLowerCase(prevUsername)) {
              ReasonReactRouter.push("/u/" ++ username);
            }
          | _ => ()
          };
        | None => ()
        };
        Promise.resolved();
      }
      |> ignore;
    };

    <Modal onBackdropClick={_ => onClose()}>
      <div className=Styles.body>
        <div className=Styles.title> {React.string("Settings")} </div>
        <div className=Styles.discordSection>
          {switch (user.discordId) {
           | Some(_) =>
             <div>
               {React.string("Your Discord account is connected. Visit the ")}
               <a href="https://discord.gg/v9fuKru" target="_blank">
                 {React.string("Discord server")}
               </a>
               {React.string(" to trade with others!")}
             </div>
           | None =>
             <>
               <div>
                 {React.string(
                    "Join the Discord server to trade with others!",
                  )}
               </div>
               <div>
                 <button
                   onClick={_ => {
                     let state =
                       "connect_"
                       ++ string_of_int(Js.Math.random_int(100000, 999999));
                     Dom.Storage.(
                       localStorage |> setItem("discord_state", state)
                     );
                     Webapi.Dom.(
                       location->Location.setHref(
                         "https://discord.com/api/oauth2/authorize?client_id=703109829610176522&redirect_uri=http%3A%2F%2Flocalhost%3A8080%2Fdiscord_oauth2&response_type=code&scope=guilds.join%20email&prompt=none&state="
                         ++ state,
                       )
                     );
                   }}
                   className={Cn.make([
                     LoginOverlay.Styles.discordButton,
                     Styles.discordButton,
                   ])}>
                   <span className=LoginOverlay.Styles.discordButtonLogo />
                   {React.string("Connect Discord account")}
                 </button>
               </div>
             </>
           }}
        </div>
        <form onSubmit>
          <div className=Styles.blurb>
            {React.string(
               user.discordId === None
                 ? "You can change your username or password here!"
                 : "You can change your username here!",
             )}
          </div>
          <div className=Styles.urlPreview>
            {React.string("nook.exchange/u/" ++ username)}
          </div>
          <input
            type_="text"
            placeholder="Username"
            value=username
            onChange={e => {
              let value = ReactEvent.Form.target(e)##value;
              setUsername(_ => value);
            }}
            className=Styles.input
          />
          {if (user.discordId === None) {
             <>
               <input
                 type_="password"
                 placeholder="New Password (optional)"
                 value=password
                 onChange={e => {
                   let value = ReactEvent.Form.target(e)##value;
                   setPassword(_ => value);
                 }}
                 className=Styles.input
               />
               <input
                 type_="password"
                 placeholder="Old Password (required)"
                 value=oldPassword
                 onChange={e => {
                   let value = ReactEvent.Form.target(e)##value;
                   setOldPassword(_ => value);
                 }}
                 className=Styles.input
               />
             </>;
           } else {
             React.null;
           }}
          {switch (status) {
           | Some(Error(error)) =>
             <div className=Styles.errorMessage> {React.string(error)} </div>
           | _ => React.null
           }}
          <div className=Styles.submitBar>
            {status == Some(Success)
               ? <div className=Styles.successMessage>
                   {React.string("Update successful!")}
                 </div>
               : React.null}
            <button
              type_="submit"
              disabled={
                !hasChanges
                || isSubmitting
                || user.discordId === None
                && oldPassword == ""
              }
              className=Styles.submitButton>
              {React.string("Submit")}
            </button>
          </div>
        </form>
      </div>
      <button
        onClick={_ => onClose()}
        className=LoginOverlay.Styles.closeButton
      />
    </Modal>;
  };
};

[@react.component]
let make = (~onClose) => {
  let user = UserStore.useMe();
  switch (user) {
  | Some(user) => <WithUser user onClose />
  | None => React.null
  };
};