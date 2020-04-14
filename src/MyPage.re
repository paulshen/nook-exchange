[@react.component]
let make = (~user: User.t) =>
  if (user.items->Array.length > 0) {
    <UserItemBrowser userItems={user.items} editable=true />;
  } else {
    <div>
      <div> {React.string("Your profile is empty!")} </div>
      <div> {React.string("Browse items and add them!")} </div>
    </div>;
  };