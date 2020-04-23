[@react.component]
let make = (~username, ~urlRest, ~url) => {
  let me = UserStore.useMe();
  switch (me) {
  | Some(me) =>
    if (Js.String.toLowerCase(me.username) == Js.String.toLowerCase(username)) {
      <MyPage user=me urlRest url />;
    } else {
      <UserViewingPage username urlRest url />;
    }
  | None => <UserViewingPage username urlRest url />
  };
};