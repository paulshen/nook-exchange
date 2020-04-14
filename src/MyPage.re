[@react.component]
let make = (~user: User.t) => {
  <UserItemBrowser userItems={user.items} editable=true />;
};