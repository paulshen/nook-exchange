[@react.component]
let make = (~path, ~children) => {
  <a
    href=path
    onClick={e => {
      ReasonReactRouter.push(path);
      ReactEvent.Mouse.preventDefault(e);
    }}>
    children
  </a>;
};