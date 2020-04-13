[@react.component]
let make = () => {
  let url = ReasonReactRouter.useUrl();
  switch (url.path) {
  | ["user", userId] => <UserPage userId />
  | _ => <ItemBrowser />
  };
};