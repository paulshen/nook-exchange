module Styles = {
  open Css;
  let cards = style([display(flexBox), flexWrap(wrap)]);
};

type filters = {
  orderable: option(bool),
  hasRecipe: option(bool),
  category: option(string),
};

let doesItemMatchFilters = (~item: Item.t, ~filters: filters) => {
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

let numResultsPerPage = 20;

[@react.component]
let make = () => {
  let (filters, setFilters) =
    React.useState(() => {orderable: None, hasRecipe: None, category: None});
  let (pageOffset, setPageOffset) = React.useState(() => 0);
  let filteredItems =
    React.useMemo1(
      () =>
        Item.all->Belt.Array.keep(item =>
          doesItemMatchFilters(~item, ~filters)
        ),
      [|filters|],
    );

  let numResults = filteredItems->Belt.Array.length;

  <div>
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
          setFilters(filters =>
            {
              ...filters,
              orderable:
                switch (value) {
                | "true" => Some(true)
                | "false" => Some(false)
                | "none" => None
                | _ => None
                },
            }
          );
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
          setFilters(filters =>
            {
              ...filters,
              hasRecipe:
                switch (value) {
                | "true" => Some(true)
                | "false" => Some(false)
                | "none" => None
                | _ => None
                },
            }
          );
        }}>
        <option value="none"> {React.string("---")} </option>
        <option value="true"> {React.string("Has recipe")} </option>
        <option value="false"> {React.string("No recipe")} </option>
      </select>
      <div>
        {React.string(
           "Page "
           ++ string_of_int(pageOffset)
           ++ " of "
           ++ string_of_int(numResults / numResultsPerPage),
         )}
      </div>
    </div>
    <div className=Styles.cards>
      {filteredItems
       ->Belt.Array.slice(
           ~offset=pageOffset * numResultsPerPage,
           ~len=numResultsPerPage,
         )
       ->Belt.Array.mapWithIndexU((. i, item) => {
           <ItemCard item key={item.id ++ string_of_int(i)} />
         })
       ->React.array}
    </div>
  </div>;
};