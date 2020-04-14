module Styles = {
  open Css;
  let root = style([display(flexBox), alignItems(center)]);
  let select =
    style([
      borderColor(hex("00000040")),
      fontSize(px(16)),
      height(px(32)),
      marginRight(px(16)),
    ]);
  let textInput =
    style([
      backgroundColor(hex("fffffff0")),
      fontSize(px(16)),
      marginRight(px(16)),
      padding2(~v=zero, ~h=px(12)),
      borderRadius(px(4)),
      borderWidth(zero),
      height(px(32)),
    ]);
  let clearFilters = style([fontSize(px(16))]);
  let pager = style([fontSize(px(16))]);
  let pagerArrow =
    style([
      fontSize(px(24)),
      textDecoration(none),
      opacity(0.8),
      transition(~duration=200, "all"),
      hover([opacity(1.)]),
    ]);
  let pagerArrowLeft = style([marginRight(px(8))]);
  let pagerArrowRight = style([marginLeft(px(8))]);
};

type t = {
  text: string,
  orderable: option(bool),
  hasRecipe: option(bool),
  category: option(string),
};

let doesItemMatchFilters = (~item: Item.t, ~filters: t) => {
  (
    switch (filters.text) {
    | "" => true
    | text =>
      Js.String.toLowerCase(item.name)
      |> Js.String.indexOf(Js.String.toLowerCase(text)) != (-1)
    }
  )
  && (
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

module Pager = {
  [@react.component]
  let make = (~numResults, ~pageOffset, ~numResultsPerPage, ~setPageOffset) =>
    if (numResults > 8) {
      <div className=Styles.pager>
        {pageOffset > 0
           ? <a
               href="#"
               onClick={e => {
                 ReactEvent.Mouse.preventDefault(e);
                 setPageOffset(pageOffset => pageOffset - 1);
               }}
               className={Cn.make([Styles.pagerArrow, Styles.pagerArrowLeft])}>
               {React.string("<")}
             </a>
           : React.null}
        {React.string(
           "Showing "
           ++ string_of_int(pageOffset * numResultsPerPage + 1)
           ++ " - "
           ++ string_of_int(
                Js.Math.min_int(
                  (pageOffset + 1) * numResultsPerPage,
                  numResults,
                ),
              )
           ++ " of "
           ++ string_of_int(numResults),
         )}
        {pageOffset < numResults / numResultsPerPage
           ? <a
               href="#"
               onClick={e => {
                 ReactEvent.Mouse.preventDefault(e);
                 setPageOffset(pageOffset => pageOffset + 1);
               }}
               className={Cn.make([
                 Styles.pagerArrow,
                 Styles.pagerArrowRight,
               ])}>
               {React.string(">")}
             </a>
           : React.null}
      </div>;
    } else {
      React.null;
    };
};

external unsafeAsHtmlInputElement: 'a => Webapi.Dom.HtmlInputElement.t =
  "%identity";

[@react.component]
let make = (~filters, ~onChange) => {
  let inputTextRef = React.useRef(Js.Nullable.null);
  let updateTextTimeoutRef = React.useRef(None);
  React.useEffect1(
    () => {
      if (filters.text == "") {
        Webapi.Dom.(
          Utils.getElementForDomRef(inputTextRef)
          ->unsafeAsHtmlInputElement
          ->HtmlInputElement.setValue("")
        );
      };
      Some(
        () => {
          switch (React.Ref.current(updateTextTimeoutRef)) {
          | Some(updateTextTimeout) =>
            Js.Global.clearTimeout(updateTextTimeout)
          | None => ()
          };
          React.Ref.setCurrent(updateTextTimeoutRef, None);
        },
      );
    },
    [|filters|],
  );

  <div className=Styles.root>
    <select
      value={Belt.Option.getWithDefault(filters.category, "")}
      onChange={e => {
        let value = ReactEvent.Form.target(e)##value;
        onChange({
          ...filters,
          category:
            switch (value) {
            | "none" => None
            | category => Some(category)
            },
        });
      }}
      className=Styles.select>
      <option value="none"> {React.string("-- Category")} </option>
      {Item.categories
       ->Belt.Array.mapWithIndexU((. i, category) =>
           <option value=category key={string_of_int(i)}>
             {React.string(category)}
           </option>
         )
       ->React.array}
    </select>
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
      }}
      className=Styles.select>
      <option value="none"> {React.string("-- Orderable")} </option>
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
      }}
      className=Styles.select>
      <option value="none"> {React.string("-- Recipe")} </option>
      <option value="true"> {React.string("Has recipe")} </option>
      <option value="false"> {React.string("No recipe")} </option>
    </select>
    <input
      type_="text"
      ref={ReactDOMRe.Ref.domRef(inputTextRef)}
      placeholder="Search by text.."
      onChange={e => {
        let value = ReactEvent.Form.target(e)##value;
        switch (React.Ref.current(updateTextTimeoutRef)) {
        | Some(updateTextTimeout) =>
          Js.Global.clearTimeout(updateTextTimeout)
        | None => ()
        };
        React.Ref.setCurrent(
          updateTextTimeoutRef,
          Some(
            Js.Global.setTimeout(
              () => {
                React.Ref.setCurrent(updateTextTimeoutRef, None);
                onChange({...filters, text: value});
              },
              300,
            ),
          ),
        );
      }}
      className=Styles.textInput
    />
    {if (filters.text != ""
         || filters.hasRecipe != None
         || filters.orderable != None
         || filters.category != None) {
       <a
         href="#"
         onClick={e => {
           ReactEvent.Mouse.preventDefault(e);
           onChange({
             text: "",
             orderable: None,
             hasRecipe: None,
             category: None,
           });
         }}
         className=Styles.clearFilters>
         {React.string("Clear filters")}
       </a>;
     } else {
       React.null;
     }}
  </div>;
};