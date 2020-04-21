module Styles = {
  open Css;
  let root = style([display(flexBox), flexWrap(wrap)]);
  let select =
    style([
      borderColor(hex("00000020")),
      fontSize(px(16)),
      height(px(37)),
      marginRight(px(8)),
      marginBottom(px(8)),
    ]);
  let selectSort = style([marginRight(px(16))]);
  let textInput =
    style([
      backgroundColor(Colors.white),
      border(px(1), solid, hex("00000020")),
      fontSize(px(16)),
      marginRight(px(8)),
      marginBottom(px(8)),
      padding2(~v=zero, ~h=px(12)),
      borderRadius(px(4)),
      height(px(35)),
      width(px(180)),
    ]);
  let clearFilters =
    style([
      alignSelf(flexStart),
      fontSize(px(16)),
      lineHeight(px(37)),
      paddingRight(px(16)),
    ]);
  let pager = style([fontSize(px(16)), marginBottom(px(8))]);
  let pagerArrow =
    style([
      fontSize(px(24)),
      lineHeight(px(24)),
      textDecoration(none),
      opacity(0.8),
      transition(~duration=200, "all"),
      hover([opacity(1.)]),
    ]);
  let pagerArrowLeft = style([marginRight(px(8))]);
  let pagerArrowRight = style([marginLeft(px(8))]);
};

type sort =
  | ABC
  | SellPriceDesc
  | SellPriceAsc;

type mask =
  | Orderable
  | HasRecipe;

type t = {
  text: string,
  mask: option(mask),
  category: option(string),
  sort,
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
    switch (filters.mask) {
    | Some(Orderable) => item.orderable
    | Some(HasRecipe) => item.recipe !== None
    | None => true
    }
  )
  && (
    switch (filters.category) {
    | Some("furniture") =>
      Item.furnitureCategories->Belt.Array.map(Js.String.toLowerCase)
      |> Js.Array.includes(item.category)
    | Some("clothing") =>
      Item.clothingCategories->Belt.Array.map(Js.String.toLowerCase)
      |> Js.Array.includes(item.category)
    | Some("other") =>
      Item.otherCategories->Belt.Array.map(Js.String.toLowerCase)
      |> Js.Array.includes(item.category)
    | Some(category) => item.category == category
    | None => true
    }
  );
};

let getSort = (~filters: t) => {
  switch (filters.sort) {
  | ABC => (
      (a: Item.t, b: Item.t) =>
        int_of_float(Js.String.localeCompare(b.name, a.name))
    )
  | SellPriceDesc => (
      (a: Item.t, b: Item.t) =>
        Belt.Option.getWithDefault(b.sellPrice, 0)
        - Belt.Option.getWithDefault(a.sellPrice, 0)
    )
  | SellPriceAsc => (
      (a: Item.t, b: Item.t) =>
        Belt.Option.getWithDefault(a.sellPrice, 0)
        - Belt.Option.getWithDefault(b.sellPrice, 0)
    )
  };
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

module CategoryButtons = {
  module CategoryStyles = {
    open Css;
    let root =
      style([
        marginBottom(px(8)),
        media("(min-width: 1200px)", [marginBottom(px(4))]),
      ]);
    let button = style([marginRight(px(8)), marginBottom(px(8))]);
    let buttonNotSelected = style([opacity(0.5), hover([opacity(1.)])]);
    let select =
      style([height(px(37)), opacity(0.5), hover([opacity(1.)])]);
    let selectSelected = style([height(px(37)), opacity(1.)]);
  };

  [@react.component]
  let make = (~filters: t, ~onChange) => {
    let renderButton = (category, label) => {
      let isSelected = filters.category == Some(category);
      <Button
        onClick={_ => {
          onChange({
            ...filters,
            category: isSelected ? None : Some(category),
          })
        }}
        className={Cn.make([
          CategoryStyles.button,
          Cn.ifTrue(CategoryStyles.buttonNotSelected, !isSelected),
        ])}
        key=category>
        {React.string(label)}
      </Button>;
    };

    <div className=CategoryStyles.root>
      <Button
        onClick={_ => {onChange({...filters, category: None})}}
        className={Cn.make([
          CategoryStyles.button,
          Cn.ifTrue(
            CategoryStyles.buttonNotSelected,
            filters.category != None,
          ),
        ])}>
        {React.string("Everything!")}
      </Button>
      {renderButton("furniture", "All Furniture")}
      {Item.furnitureCategories
       ->Belt.Array.mapU((. category) =>
           renderButton(Js.String.toLowerCase(category), category)
         )
       ->React.array}
      {renderButton("clothing", "All Clothing")}
      <select
        value={
          switch (filters.category) {
          | Some(category) =>
            Item.clothingCategories |> Js.Array.includes(category)
              ? category : ""
          | None => ""
          }
        }
        onChange={e => {
          let value = ReactEvent.Form.target(e)##value;
          onChange({
            ...filters,
            category:
              switch (value) {
              | "" => None
              | category => Some(category)
              },
          });
        }}
        className={Cn.make([
          Styles.select,
          CategoryStyles.select,
          Cn.ifTrue(
            CategoryStyles.selectSelected,
            switch (filters.category) {
            | Some(category) =>
              Item.clothingCategories
              |> Js.Array.includes(category)
              || Item.otherCategories
              |> Js.Array.includes(category)
            | None => false
            },
          ),
        ])}>
        <option value=""> {React.string("-- Other Categories")} </option>
        {Belt.Array.concat(Item.clothingCategories, Item.otherCategories)
         ->Belt.Array.mapU((. category) =>
             <option value={Js.String.toLowerCase(category)} key=category>
               {React.string(category)}
             </option>
           )
         ->React.array}
      </select>
    </div>;
  };
};

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

  React.useEffect0(() => {
    open Webapi.Dom;
    let onKeyDown = e => {
      switch (KeyboardEvent.key(e)) {
      | "Esc"
      | "Escape" => onChange({...filters, text: ""})
      | _ => ()
      };
    };
    window |> Window.addKeyDownEventListener(onKeyDown);
    Some(() => {window |> Window.removeKeyDownEventListener(onKeyDown)});
  });

  <div className=Styles.root>
    <input
      type_="text"
      ref={ReactDOMRe.Ref.domRef(inputTextRef)}
      placeholder="Search.. Esc to clear"
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
    <select
      value={
        switch (filters.mask) {
        | Some(Orderable) => "orderable"
        | Some(HasRecipe) => "has-recipe"
        | None => "none"
        }
      }
      onChange={e => {
        let value = ReactEvent.Form.target(e)##value;
        onChange({
          ...filters,
          mask:
            switch (value) {
            | "orderable" => Some(Orderable)
            | "has-recipe" => Some(HasRecipe)
            | "none" => None
            | _ => None
            },
        });
      }}
      className=Styles.select>
      <option value="none"> {React.string("No Filter")} </option>
      <option value="orderable"> {React.string("Orderable")} </option>
      <option value="has-recipe"> {React.string("Has Recipe")} </option>
    </select>
    <select
      value={
        switch (filters.sort) {
        | ABC => "abc"
        | SellPriceDesc => "sell-desc"
        | SellPriceAsc => "sell-asc"
        }
      }
      onChange={e => {
        let value = ReactEvent.Form.target(e)##value;
        onChange({
          ...filters,
          sort:
            switch (value) {
            | "abc" => ABC
            | "sell-desc" => SellPriceDesc
            | "sell-asc" => SellPriceAsc
            | _ => ABC
            },
        });
      }}
      className={Cn.make([Styles.select, Styles.selectSort])}>
      <option value="sell-desc">
        {React.string({j|Sell Price ↓|j})}
      </option>
      <option value="sell-asc"> {React.string({j|Sell Price ↑|j})} </option>
      <option value="abc"> {React.string("A - Z")} </option>
    </select>
    {if (filters.text != "" || filters.mask != None || filters.category != None) {
       <a
         href="#"
         onClick={e => {
           ReactEvent.Mouse.preventDefault(e);
           onChange({
             text: "",
             mask: None,
             category: None,
             sort: SellPriceDesc,
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