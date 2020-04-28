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
  let inputWithValue =
    style([boxShadow(Shadow.box(~spread=px(2), hex("3aa56380")))]);
  let clearFilters =
    style([
      alignSelf(flexStart),
      fontSize(px(16)),
      lineHeight(px(37)),
      paddingRight(px(16)),
    ]);
  let pager =
    style([fontSize(px(16)), lineHeight(px(32)), marginBottom(px(8))]);
  let pagerArrow =
    style([
      fontSize(px(24)),
      lineHeight(px(24)),
      textDecoration(none),
      opacity(0.8),
      padding2(~v=zero, ~h=px(8)),
      transition(~duration=200, "all"),
      hover([opacity(1.)]),
    ]);
};

type sort =
  | ABC
  | SellPriceDesc
  | SellPriceAsc
  | UserDefault;

type mask =
  | Orderable
  | HasRecipe;

type t = {
  text: string,
  mask: option(mask),
  category: option(string),
  sort,
};

let serialize = (~filters, ~defaultSort, ~pageOffset) => {
  let p = [||];
  if (filters.sort != defaultSort && filters.sort != UserDefault) {
    p
    |> Js.Array.push((
         "s",
         switch (filters.sort) {
         | ABC => "abc"
         | SellPriceDesc => "pd"
         | SellPriceAsc => "pa"
         | UserDefault => ""
         },
       ))
    |> ignore;
  };
  if (filters.text != "") {
    p |> Js.Array.push(("q", filters.text)) |> ignore;
  };
  switch (filters.mask) {
  | Some(Orderable) => p |> Js.Array.push(("orderable", "")) |> ignore
  | Some(HasRecipe) => p |> Js.Array.push(("has-recipe", "")) |> ignore
  | None => ()
  };
  switch (filters.category) {
  | Some(category) => p |> Js.Array.push(("c", category)) |> ignore
  | None => ()
  };
  if (pageOffset != 0) {
    p |> Js.Array.push(("p", string_of_int(pageOffset + 1))) |> ignore;
  };
  p;
};

let fromUrlSearch = (~urlSearch, ~defaultSort) => {
  open Belt;
  open Webapi.Url.URLSearchParams;
  let searchParams = make(urlSearch);
  (
    {
      text: (searchParams |> get("q"))->Option.getWithDefault(""),
      mask:
        searchParams |> has("orderable")
          ? Some(Orderable)
          : searchParams |> has("has-recipe") ? Some(HasRecipe) : None,
      category:
        Option.flatMap(searchParams |> get("c"), category =>
          if (Item.validCategoryStrings |> Js.Array.includes(category)) {
            Some(category);
          } else {
            None;
          }
        ),
      sort:
        switch (searchParams |> get("s")) {
        | Some("abc") => ABC
        | Some("pd") => SellPriceDesc
        | Some("pa") => SellPriceAsc
        | _ => defaultSort
        },
    },
    Option.map(searchParams |> get("p"), s => int_of_string(s) - 1)
    ->Option.getWithDefault(0),
  );
};

let doesItemMatchCategory = (~item: Item.t, ~category: string) => {
  switch (category) {
  | "furniture" =>
    Item.furnitureCategories |> Js.Array.includes(item.category)
  | "clothing" => Item.clothingCategories |> Js.Array.includes(item.category)
  | "other" => Item.otherCategories |> Js.Array.includes(item.category)
  | "recipes" => item.isRecipe
  | category => item.category == category
  };
};

let doesItemMatchFilters = (~item: Item.t, ~filters: t) => {
  (
    switch (filters.text) {
    | "" => true
    | text =>
      let fragments =
        (
          Js.String.toLowerCase(text)
          |> Js.String.splitByRe([%bs.re {|/[\s-]+/|}])
        )
        ->Belt.Array.keepMap(x => x);
      fragments->Belt.Array.every(fragment =>
        Js.String.toLowerCase(Item.getName(item))
        |> Js.String.includes(fragment)
        || item.tags->Belt.Array.some(Js.String.includes(fragment))
      );
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
    | Some(category) => doesItemMatchCategory(~item, ~category)
    | None => true
    }
  );
};

let compareArrays = (a, b) => {
  let rv = ref(None);
  let i = ref(0);
  while (i^ < Js.Array.length(a) && rv^ === None) {
    if (a[i^] < b[i^]) {
      rv := Some(-1);
    } else if (a[i^] > b[i^]) {
      rv := Some(1);
    };
    i := i^ + 1;
  };
  Belt.Option.getWithDefault(rv^, 0);
};

let compareItemsABC = (a: Item.t, b: Item.t) =>
  int_of_float(Js.String.localeCompare(b.name, a.name));
let compareItemsSellPriceDesc = (a: Item.t, b: Item.t) =>
  Belt.Option.getWithDefault(b.sellPrice, 0)
  - Belt.Option.getWithDefault(a.sellPrice, 0);
let compareItemsSellPriceAsc = (a: Item.t, b: Item.t) =>
  Belt.Option.getWithDefault(a.sellPrice, 0)
  - Belt.Option.getWithDefault(b.sellPrice, 0);
exception UnexpectedSort(sort);
let getSort = (~sort) => {
  switch (sort) {
  | ABC => compareItemsABC
  | SellPriceDesc => compareItemsSellPriceDesc
  | SellPriceAsc => compareItemsSellPriceAsc
  | UserDefault => raise(UnexpectedSort(sort))
  };
};
let getUserItemSort =
    (~prioritizeViewerStatuses: array(User.itemStatus)=[||], ~sort) => {
  Belt.(
    switch (sort) {
    | ABC => (
        ((aId, _), (bId, _)) =>
          compareItemsABC(
            Item.getItem(~itemId=aId),
            Item.getItem(~itemId=bId),
          )
      )
    | SellPriceDesc => (
        ((aId, _), (bId, _)) =>
          compareItemsSellPriceDesc(
            Item.getItem(~itemId=aId),
            Item.getItem(~itemId=bId),
          )
      )
    | SellPriceAsc => (
        ((aId, _), (bId, _)) =>
          compareItemsSellPriceAsc(
            Item.getItem(~itemId=aId),
            Item.getItem(~itemId=bId),
          )
      )
    | UserDefault => (
        ((aId, aVariant), (bId, bVariant)) => {
          let aItem = Item.getItem(~itemId=aId);
          let bItem = Item.getItem(~itemId=bId);
          compareArrays(
            [|
              switch (UserStore.getItem(~itemId=aId, ~variation=aVariant)) {
              | Some(aUserItem) =>
                prioritizeViewerStatuses
                |> Js.Array.includes(aUserItem.status)
                  ? (-1) : 0
              | None => 0
              },
              Item.categories |> Js.Array.indexOf(aItem.category),
              - Option.getWithDefault(aItem.sellPrice, 0),
            |],
            [|
              switch (UserStore.getItem(~itemId=bId, ~variation=bVariant)) {
              | Some(bUserItem) =>
                prioritizeViewerStatuses
                |> Js.Array.includes(bUserItem.status)
                  ? (-1) : 0
              | None => 0
              },
              Item.categories |> Js.Array.indexOf(bItem.category),
              - Option.getWithDefault(bItem.sellPrice, 0),
            |],
          );
        }
      )
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
               className=Styles.pagerArrow>
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
        {pageOffset < (numResults - 1) / numResultsPerPage
           ? <a
               href="#"
               onClick={e => {
                 ReactEvent.Mouse.preventDefault(e);
                 setPageOffset(pageOffset => pageOffset + 1);
               }}
               className=Styles.pagerArrow>
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

let getCategoryLabel = category => {
  switch (category) {
  | "furniture" => "All Furniture"
  | "clothing" => "All Clothing"
  | category => Utils.capitalizeFirstLetter(category)
  };
};

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
  let make =
      (~filters: t, ~onChange, ~userItemIds: option(array(string))=?, ()) => {
    let shouldRenderCategory = category => {
      switch (userItemIds) {
      | Some(userItemIds) =>
        userItemIds->Belt.Array.some(itemId =>
          doesItemMatchCategory(~item=Item.getItem(~itemId), ~category)
        )
      | None => true
      };
    };
    let renderButton = category =>
      if (shouldRenderCategory(category)) {
        let isSelected = filters.category == Some(category);
        <Button
          onClick={_ => {
            onChange({
              ...filters,
              text: "",
              category: isSelected ? None : Some(category),
            })
          }}
          className={Cn.make([
            CategoryStyles.button,
            Cn.ifTrue(CategoryStyles.buttonNotSelected, !isSelected),
          ])}
          key=category>
          {React.string(getCategoryLabel(category))}
        </Button>;
      } else {
        React.null;
      };

    let selectCategories =
      Belt.Array.concatMany([|
        [|"wallpapers", "floors", "rugs"|],
        Item.clothingCategories,
        Item.otherCategories,
      |]);

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
      {renderButton("furniture")}
      {renderButton("housewares")}
      {renderButton("miscellaneous")}
      {renderButton("wall-mounted")}
      {renderButton("recipes")}
      {renderButton("clothing")}
      <select
        value={
          switch (filters.category) {
          | Some(category) =>
            selectCategories |> Js.Array.includes(category) ? category : ""
          | None => ""
          }
        }
        onChange={e => {
          let value = ReactEvent.Form.target(e)##value;
          onChange({
            ...filters,
            text: "",
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
              selectCategories |> Js.Array.includes(category)
            | None => false
            },
          ),
        ])}>
        <option value=""> {React.string("-- Other Categories")} </option>
        {selectCategories
         ->Belt.Array.mapU((. category) =>
             shouldRenderCategory(category)
               ? <option value=category key=category>
                   {React.string(getCategoryLabel(category))}
                 </option>
               : React.null
           )
         ->React.array}
      </select>
    </div>;
  };
};

module UserCategorySelector = {
  module CategoryStyles = {
    open Css;
    let select = style([height(px(37))]);
  };

  [@react.component]
  let make = (~filters: t, ~onChange, ~userItemIds: array(string)) => {
    let shouldRenderCategory = category => {
      userItemIds->Belt.Array.some(itemId =>
        doesItemMatchCategory(~item=Item.getItem(~itemId), ~category)
      );
    };

    <select
      value={Belt.Option.getWithDefault(filters.category, "")}
      onChange={e => {
        let value = ReactEvent.Form.target(e)##value;
        if (value == "") {
          onChange({...filters, category: None});
        } else {
          onChange({...filters, text: "", category: Some(value)});
        };
      }}
      className={Cn.make([
        Styles.select,
        CategoryStyles.select,
        Cn.ifTrue(Styles.inputWithValue, filters.category != None),
      ])}>
      <option value=""> {React.string("All categories")} </option>
      {Item.validCategoryStrings
       ->Belt.Array.mapU((. category) =>
           shouldRenderCategory(category)
             ? <option value=category key=category>
                 {React.string(Utils.capitalizeFirstLetter(category))}
               </option>
             : React.null
         )
       ->React.array}
    </select>;
  };
};

[@react.component]
let make =
    (
      ~filters,
      ~onChange,
      ~userItemIds: option(array(string))=?,
      ~isViewingSelf=false,
      (),
    ) => {
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
      defaultValue={filters.text}
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
      className={Cn.make([
        Styles.textInput,
        Cn.ifTrue(Styles.inputWithValue, filters.text != ""),
      ])}
    />
    {switch (userItemIds) {
     | Some(userItemIds) =>
       <UserCategorySelector userItemIds filters onChange />
     | None => React.null
     }}
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
      className={Cn.make([
        Styles.select,
        Cn.ifTrue(Styles.inputWithValue, filters.mask != None),
      ])}>
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
        | UserDefault => "user-default"
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
            | "user-default" => UserDefault
            | _ => SellPriceDesc
            },
        });
      }}
      className={Cn.make([Styles.select, Styles.selectSort])}>
      {if (userItemIds !== None) {
         <option value="user-default">
           {React.string(isViewingSelf ? "Sort: Category" : "Sort: Default")}
         </option>;
       } else {
         React.null;
       }}
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
             sort: filters.sort,
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