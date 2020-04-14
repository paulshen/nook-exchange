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

  let (createdCredentials, setCreatedCredentials) =
    React.useState(() => None);
  let desiredUsernameRef = React.useRef(Js.Nullable.null);
  let onCreateSubmit = e => {
    ReactEvent.Form.preventDefault(e);
    open Webapi.Dom;
    let desiredUsername =
      getElementForDomRef(desiredUsernameRef)
      ->Element.unsafeAsHtmlElement
      ->HtmlElement.value;
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
        <button onClick={_ => onClose()}> {React.string("Close!")} </button>
      </div>
    </div>
  | None =>
    <div>
      <div>
        <form onSubmit=onLoginSubmit>
          <div>
            <label> {React.string("Username")} </label>
            <input type_="text" ref={ReactDOMRe.Ref.domRef(usernameRef)} />
          </div>
          <div>
            <label> {React.string("Password")} </label>
            <input
              type_="password"
              ref={ReactDOMRe.Ref.domRef(passwordRef)}
            />
          </div>
          <button type_="submit"> {React.string("Login")} </button>
        </form>
      </div>
      <div>
        <div> {React.string("Don't have an account?")} </div>
        <div>
          {React.string(
             "Just enter your desired URL and you'll be given a password. No email needed!",
           )}
        </div>
        <div>
          <form onSubmit=onCreateSubmit>
            <label> {React.string("/u/")} </label>
            <input
              type_="text"
              ref={ReactDOMRe.Ref.domRef(desiredUsernameRef)}
            />
            <button type_="submit"> {React.string("Create")} </button>
          </form>
        </div>
      </div>
    </div>
  };
};