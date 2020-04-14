[@react.component]
let make = (~user: User.t) =>
  if (user.items->Js.Dict.keys->Array.length > 0) {
    <UserItemBrowser
      userItems={
        user.items
        ->Js.Dict.entries
        ->Belt.Array.mapU((. (itemKey, item)) =>
            (User.fromItemKey(~key=itemKey), item)
          )
      }
      editable=true
    />;
  } else {
    <div>
      <div> {React.string("Your profile is empty!")} </div>
      <div> {React.string("Browse items and add them!")} </div>
    </div>;
  };