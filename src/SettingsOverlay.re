module Styles = {
  open Css;
  let body = style([maxWidth(px(320)), margin2(~v=zero, ~h=auto)]);
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
      padding2(~v=px(10), ~h=px(16)),
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
};

type submitStatus =
  | Success
  | Error(string);

[@react.component]
let make = (~onClose) => {
  let user = UserStore.useMe()->Belt.Option.getExn;

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
      let usernameUpdate = username == user.username ? None : Some(username);
      let%Repromise result =
        UserStore.patchMe(
          ~username=?usernameUpdate,
          ~newPassword=?password != "" ? Some(password) : None,
          ~oldPassword,
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

  <div className=LoginOverlay.Styles.overlay>
    <div className=LoginOverlay.Styles.backdrop onClick={_ => onClose()} />
    <div className=LoginOverlay.Styles.root>
      <div className=Styles.body>
        <form onSubmit>
          <div className=Styles.title> {React.string("Settings")} </div>
          <div className=Styles.blurb>
            {React.string("You can change your username or password here!")}
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
              disabled={!hasChanges || isSubmitting || oldPassword == ""}
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
    </div>
  </div>;
};