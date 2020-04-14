module Styles = {
  open Css;
  let textarea =
    style([
      backgroundColor(rgba(0, 0, 0, 0.05)),
      border(px(1), solid, transparent),
      borderRadius(px(4)),
      padding(px(8)),
      boxSizing(borderBox),
      outlineStyle(none),
      fontSize(px(14)),
      width(pct(100.)),
      marginBottom(px(8)),
      transition(~duration=200, "all"),
      width(pct(100.)),
      focus([
        backgroundColor(hex("ffffff")),
        borderColor(rgba(0, 0, 0, 0.15)),
        boxShadow(Shadow.box(~spread=px(4), rgba(0, 0, 0, 0.05))),
      ]),
    ]);
};

[@react.component]
let make = (~itemId, ~variation, ~userItem: User.item) => {
  let (userItemNote, setUserItemNote) = React.useState(() => userItem.note);

  let updateNote = () =>
    UserStore.setItem(
      ~itemId,
      ~variation,
      ~item={status: userItem.status, note: userItemNote},
    );
  let updateNoteRef = React.useRef(updateNote);
  React.useEffect(() => {
    React.Ref.setCurrent(updateNoteRef, updateNote);
    None;
  });
  let throttleNoteTimeoutRef = React.useRef(None);
  React.useEffect0(() => {
    Some(
      () => {
        switch (React.Ref.current(throttleNoteTimeoutRef)) {
        | Some(throttleNoteTimeout) =>
          Js.Global.clearTimeout(throttleNoteTimeout)
        | None => ()
        }
      },
    )
  });

  <textarea
    value=userItemNote
    placeholder="Add a note"
    className=Styles.textarea
    onChange={e => {
      let value = ReactEvent.Form.target(e)##value;
      setUserItemNote(_ => value);

      switch (React.Ref.current(throttleNoteTimeoutRef)) {
      | Some(throttleNoteTimeout) =>
        Js.Global.clearTimeout(throttleNoteTimeout)
      | None => ()
      };
      React.Ref.setCurrent(
        throttleNoteTimeoutRef,
        Some(
          Js.Global.setTimeout(
            () => {
              React.Ref.setCurrent(throttleNoteTimeoutRef, None);
              React.Ref.current(updateNoteRef, ());
            },
            300,
          ),
        ),
      );
    }}
  />;
};