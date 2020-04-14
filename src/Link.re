[@react.component]
let make = (~path, ~children, ~className=?, ()) => {
  <a
    href=path
    onClick={e => {
      ReasonReactRouter.push(path);
      ReactEvent.Mouse.preventDefault(e);
    }}
    ?className>
    children
  </a>;
};