type t = {
  orderable: option(bool),
  hasRecipe: option(bool),
  category: option(string),
};

let doesItemMatchFilters = (~item: Item.t, ~filters: t) => {
  (
    switch (filters.orderable) {
    | Some(true) => item.orderable
    | Some(false) => !item.orderable
    | None => true
    }
  )
  && (
    switch (filters.hasRecipe) {
    | Some(true) => Belt.Option.isSome(item.recipe)
    | Some(false) => Belt.Option.isNone(item.recipe)
    | None => true
    }
  )
  && (
    switch (filters.category) {
    | Some(category) => item.category == category
    | None => true
    }
  );
};

[@react.component]
let make = (~filters, ~onChange) => {
  <div>
    <select
      value={
        switch (filters.orderable) {
        | Some(true) => "true"
        | Some(false) => "false"
        | None => "none"
        }
      }
      onChange={e => {
        let value = ReactEvent.Form.target(e)##value;
        onChange({
          ...filters,
          orderable:
            switch (value) {
            | "true" => Some(true)
            | "false" => Some(false)
            | "none" => None
            | _ => None
            },
        });
      }}>
      <option value="none"> {React.string("---")} </option>
      <option value="true"> {React.string("Orderable")} </option>
      <option value="false"> {React.string("Not-orderable")} </option>
    </select>
    <select
      value={
        switch (filters.hasRecipe) {
        | Some(true) => "true"
        | Some(false) => "false"
        | None => "none"
        }
      }
      onChange={e => {
        let value = ReactEvent.Form.target(e)##value;
        onChange({
          ...filters,
          hasRecipe:
            switch (value) {
            | "true" => Some(true)
            | "false" => Some(false)
            | "none" => None
            | _ => None
            },
        });
      }}>
      <option value="none"> {React.string("---")} </option>
      <option value="true"> {React.string("Has recipe")} </option>
      <option value="false"> {React.string("No recipe")} </option>
    </select>
  </div>;
};