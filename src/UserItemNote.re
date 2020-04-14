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

  <div>
    <textarea
      value=userItemNote
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
    />
  </div>;
};