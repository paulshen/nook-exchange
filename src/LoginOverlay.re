module Styles = {
  open Css;
  let overlay =
    style([
      position(fixed),
      top(zero),
      bottom(zero),
      left(zero),
      right(zero),
      display(flexBox),
      alignItems(center),
      justifyContent(center),
    ]);
  let backdrop =
    style([
      position(absolute),
      top(zero),
      bottom(zero),
      left(zero),
      right(zero),
      backgroundColor(hex("808080c0")),
    ]);
  let root =
    style([
      backgroundColor(hex("ffffff")),
      padding2(~v=px(32), ~h=px(32)),
      borderRadius(px(4)),
      position(relative),
    ]);
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
        backgroundColor(hex("ffffff")),
        borderColor(rgba(0, 0, 0, 0.15)),
        boxShadow(Shadow.box(~spread=px(4), rgba(0, 0, 0, 0.05))),
      ]),
    ]);
  let submitButton =
    style([
      backgroundColor(Colors.green),
      borderWidth(zero),
      borderRadius(px(4)),
      color(Colors.white),
      padding2(~v=px(10), ~h=px(16)),
      fontSize(px(16)),
    ]);
  let divider =
    style([
      backgroundColor(hex("e0e0e0")),
      height(px(1)),
      width(pct(100.)),
      margin2(~v=px(24), ~h=zero),
    ]);
  let registerTitle = style([textAlign(center), marginBottom(px(16))]);
  let registerBlurb = style([marginBottom(px(16))]);
  let urlPreview =
    style([
      fontSize(px(12)),
      letterSpacing(pxFloat(0.3)),
      color(hex("b0b0b0")),
      marginBottom(px(8)),
    ]);
};

let getElementForDomRef = domRef => {
  domRef->React.Ref.current->Js.Nullable.toOption->Belt.Option.getExn;
};

[@react.component]
let make = (~onClose) => {
  let usernameRef = React.useRef(Js.Nullable.null);
  let passwordRef = React.useRef(Js.Nullable.null);

  let onLoginSubmit = e => {
    ReactEvent.Form.preventDefault(e);
    open Webapi.Dom;
    let userId =
      getElementForDomRef(usernameRef)
      ->Element.unsafeAsHtmlElement
      ->HtmlElement.value;
    let password =
      getElementForDomRef(passwordRef)
      ->Element.unsafeAsHtmlElement
      ->HtmlElement.value;
    {
      let%Repromise result = UserStore.login(~userId, ~password);
      switch (result) {
      | Ok(_) => onClose()
      | Error(_) => ()
      };
      Promise.resolved();
    }
    |> ignore;
  };

  let (desiredUsername, setDesiredUsername) = React.useState(() => "");
  let (createdCredentials, setCreatedCredentials) =
    React.useState(() => None);
  let onCreateSubmit = e => {
    ReactEvent.Form.preventDefault(e);
    {
      let%Repromise result =
        UserStore.login(~userId=desiredUsername, ~password="foobar");
      switch (result) {
      | Ok(_) => setCreatedCredentials(_ => Some((desiredUsername, "foobar")))
      | Error(_) => ()
      };
      Promise.resolved();
    }
    |> ignore;
  };

  switch (createdCredentials) {
  | Some((userId, password)) =>
    <div>
      <div> {React.string("Your account is created!")} </div>
      <div> {React.string("Your username is " ++ userId)} </div>
      <div> {React.string("Your password is " ++ password)} </div>
      <div> {React.string("Write it down!")} </div>
      <div>
        {React.string("Add items to profile and share it with others!")}
      </div>
      <div> {React.string("/u/" ++ userId)} </div>
      <div>
        <button onClick={_ => onClose()} className=Styles.submitButton>
          {React.string("Close!")}
        </button>
      </div>
    </div>
  | None =>
    <div className=Styles.overlay>
      <div className=Styles.backdrop onClick={_ => onClose()} />
      <div className=Styles.root>
        <div className=Styles.body>
          <div>
            <form onSubmit=onLoginSubmit>
              <div>
                <input
                  type_="text"
                  placeholder="Username"
                  className=Styles.input
                  ref={ReactDOMRe.Ref.domRef(usernameRef)}
                />
              </div>
              <div>
                <input
                  type_="password"
                  placeholder="Password"
                  className=Styles.input
                  ref={ReactDOMRe.Ref.domRef(passwordRef)}
                />
              </div>
              <button type_="submit" className=Styles.submitButton>
                {React.string("Login")}
              </button>
            </form>
          </div>
          <div className=Styles.divider />
          <div>
            <div className=Styles.registerTitle>
              {React.string("Don't have an account?")}
            </div>
            <div className=Styles.registerBlurb>
              {React.string(
                 "Just enter your desired URL and you'll be given a password. No email needed!",
               )}
            </div>
            <div className=Styles.urlPreview>
              {React.string("tanukichi.com/u/" ++ desiredUsername)}
            </div>
            <div>
              <form onSubmit=onCreateSubmit>
                <input
                  type_="text"
                  placeholder="Username"
                  value=desiredUsername
                  onChange={e => {
                    let value = ReactEvent.Form.target(e)##value;
                    setDesiredUsername(_ => value);
                  }}
                  className=Styles.input
                />
                <button type_="submit" className=Styles.submitButton>
                  {React.string("Create")}
                </button>
              </form>
            </div>
          </div>
        </div>
      </div>
    </div>
  };
};