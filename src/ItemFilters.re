module Styles = {
  open Css;
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
      whiteSpace(`nowrap),
    ]);
  let root = style([display(flexBox), flexWrap(wrap)]);
  let rootScrollerSpacer =
    style([
      media(
        "(max-width: 600px)",
        [flexShrink(0.), width(px(1)), height(px(1))],
      ),
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
  | UserDefault
  | UserTimeUpdated;

type mask =
  | Orderable
  | HasRecipe;

type excludables =
  | Catalog
  | Wishlist;

type t = {
  text: string,
  mask: option(mask),
  category: option(string),
  exclude: array(excludables),
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
         | UserTimeUpdated => "tu"
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
  if (Js.Array.length(filters.exclude) > 0) {
    p
    |> Js.Array.push((
         "e",
         filters.exclude
         ->Belt.Array.map(exclude =>
             switch (exclude) {
             | Catalog => "catalog"
             | Wishlist => "wishlist"
             }
           )
         |> Js.Array.joinWith(","),
       ))
    |> ignore;
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
      exclude:
        switch (searchParams |> get("e")) {
        | Some(e) =>
          (e |> Js.String.split(","))
          ->Belt.Array.keepMap(fragment => {
              switch (fragment) {
              | "wishlist" => Some(Wishlist)
              | "catalog" => Some(Catalog)
              | _ => None
              }
            })
        | None => [||]
        },
      sort:
        switch (searchParams |> get("s")) {
        | Some("abc") => ABC
        | Some("pd") => SellPriceDesc
        | Some("pa") => SellPriceAsc
        | Some("tu") => UserTimeUpdated
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
  | "recipes" => Item.isRecipe(~item)
  | category => item.category == category
  };
};

let removeAccents = str => {
  str
  |> Js.String.normalizeByForm("NFD")
  |> Js.String.replaceByRe([%bs.re "/[\u0300-\u036f]/g"], "");
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
        |> removeAccents
        |> Js.String.includes(removeAccents(fragment))
        || item.tags
        |> Js.Array.includes(fragment)
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
  int_of_float(Js.String.localeCompare(Item.getName(b), Item.getName(a)));
let compareItemsSellPriceDesc = (a: Item.t, b: Item.t) =>
  compareArrays(
    [|
      Belt.Option.getWithDefault(b.sellPrice, 0),
      Item.getName(a)
      |> Js.String.localeCompare(Item.getName(b))
      |> int_of_float,
    |],
    [|Belt.Option.getWithDefault(a.sellPrice, 0), 0|],
  );
let compareItemsSellPriceAsc = (a: Item.t, b: Item.t) =>
  compareArrays(
    [|
      Belt.Option.getWithDefault(a.sellPrice, 0),
      Item.getName(a)
      |> Js.String.localeCompare(Item.getName(b))
      |> int_of_float,
    |],
    [|Belt.Option.getWithDefault(b.sellPrice, 0), 0|],
  );

exception UnexpectedSort(sort);
let getSort = (~sort) => {
  switch (sort) {
  | ABC => compareItemsABC
  | SellPriceDesc => compareItemsSellPriceDesc
  | SellPriceAsc => compareItemsSellPriceAsc
  | UserTimeUpdated
  | UserDefault => raise(UnexpectedSort(sort))
  };
};

let wrapWithVariantSort = (sort, a, b) => {
  let order = sort(a, b);
  if (order != 0) {
    order;
  } else {
    let ((_, aVariant), _) = a;
    let ((_, bVariant), _) = b;
    aVariant - bVariant;
  };
};
let getUserItemSort =
    (~prioritizeViewerStatuses: array(User.itemStatus)=[||], ~sort) => {
  Belt.(
    switch (sort) {
    | ABC =>
      wrapWithVariantSort((((aId, _), _), ((bId, _), _)) =>
        compareItemsABC(
          Item.getItem(~itemId=aId),
          Item.getItem(~itemId=bId),
        )
      )
    | SellPriceDesc =>
      wrapWithVariantSort((((aId, _), _), ((bId, _), _)) =>
        compareItemsSellPriceDesc(
          Item.getItem(~itemId=aId),
          Item.getItem(~itemId=bId),
        )
      )
    | SellPriceAsc =>
      wrapWithVariantSort((((aId, _), _), ((bId, _), _)) =>
        compareItemsSellPriceAsc(
          Item.getItem(~itemId=aId),
          Item.getItem(~itemId=bId),
        )
      )
    | UserDefault =>
      wrapWithVariantSort(
        (
          (
            (aId, aVariant),
            {priorityTimestamp: aPriorityTimestamp}: User.item,
          ),
          (
            (bId, bVariant),
            {priorityTimestamp: bPriorityTimestamp}: User.item,
          ),
        ) => {
        let aItem = Item.getItem(~itemId=aId);
        let bItem = Item.getItem(~itemId=bId);
        compareArrays(
          [|
            switch (UserStore.getItem(~itemId=aId, ~variation=aVariant)) {
            | Some(aUserItem) =>
              prioritizeViewerStatuses |> Js.Array.includes(aUserItem.status)
                ? (-1.) : 0.
            | None => 0.
            },
            -. Option.getWithDefault(aPriorityTimestamp, 0.),
            Item.categories
            |> Js.Array.indexOf(aItem.category)
            |> float_of_int,
            - Option.getWithDefault(aItem.sellPrice, 0) |> float_of_int,
            Item.getName(aItem)
            |> Js.String.localeCompare(Item.getName(bItem)),
          |],
          [|
            switch (UserStore.getItem(~itemId=bId, ~variation=bVariant)) {
            | Some(bUserItem) =>
              prioritizeViewerStatuses |> Js.Array.includes(bUserItem.status)
                ? (-1.) : 0.
            | None => 0.
            },
            -. Option.getWithDefault(bPriorityTimestamp, 0.),
            Item.categories
            |> Js.Array.indexOf(bItem.category)
            |> float_of_int,
            - Option.getWithDefault(bItem.sellPrice, 0) |> float_of_int,
            0.,
          |],
        );
      })
    | UserTimeUpdated =>
      wrapWithVariantSort(
        (
          ((aId, _), aUserItem: User.item),
          ((bId, _), bUserItem: User.item),
        ) => {
        let aItem = Item.getItem(~itemId=aId);
        let bItem = Item.getItem(~itemId=bId);
        compareArrays(
          [|
            -. aUserItem.timeUpdated->Belt.Option.getWithDefault(0.),
            Item.getName(bItem)
            |> Js.String.localeCompare(Item.getName(aItem)),
          |],
          [|-. bUserItem.timeUpdated->Belt.Option.getWithDefault(0.), 0.|],
        );
      })
    }
  );
};

module Pager = {
  [@react.component]
  let make =
      (
        ~numResults,
        ~pageOffset,
        ~numResultsPerPage,
        ~setPageOffset,
        ~className=?,
        (),
      ) =>
    if (numResults > 8) {
      <div className={Cn.make([Styles.pager, Cn.unpack(className)])}>
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
  | "recipes" => "Recipes"
  | "furniture" => "Furniture"
  | "clothing" => "Clothing"
  | category => Utils.capitalizeFirstLetter(category)
  };
};

module CategoryButtons = {
  module CategoryStyles = {
    open Css;
    let button =
      style([marginRight(px(8)), transition(~duration=200, "all")]);
    let buttonNotSelected =
      style([
        backgroundColor(hex("8fcaa3e0")),
        hover([backgroundColor(hex("3aa563e0"))]),
      ]);
    let select =
      style([
        height(px(37)),
        opacity(0.7),
        marginRight(px(16)),
        marginBottom(zero),
        hover([opacity(1.)]),
      ]);
    let selectSelected = style([height(px(37)), opacity(1.)]);
    let excludeNotice =
      style([display(inlineBlock), marginBottom(px(12))]);
    let root =
      style([
        paddingBottom(px(8)),
        media(
          "(max-width: 600px)",
          [
            display(flexBox),
            alignItems(center),
            marginLeft(px(-16)),
            marginRight(px(-16)),
            paddingLeft(px(16)),
            overflowX(auto),
            selector("& ." ++ button, [marginBottom(zero)]),
            selector("& ." ++ select, [marginBottom(zero)]),
          ],
        ),
      ]);
  };

  [@react.component]
  let make =
      (~filters: t, ~onChange, ~userItemIds: option(array(int))=?, ()) => {
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
        [|
          "housewares",
          "miscellaneous",
          "wall-mounted",
          "wallpapers",
          "floors",
          "rugs",
        |],
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
        <option value=""> {React.string("-- Categories")} </option>
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
      {switch (filters.exclude) {
       | [|Wishlist, Catalog|]
       | [|Catalog, Wishlist|] =>
         <div className=CategoryStyles.excludeNotice>
           {React.string("Hiding items in Catalog and Wishlist")}
         </div>
       | [|Catalog|] =>
         <div className=CategoryStyles.excludeNotice>
           {React.string("Hiding items in Catalog")}
         </div>
       | [|Wishlist|] =>
         <div className=CategoryStyles.excludeNotice>
           {React.string("Hiding items in Wishlist")}
         </div>
       | _ => React.null
       }}
      <div className=Styles.rootScrollerSpacer />
    </div>;
  };
};

module UserCategorySelector = {
  module CategoryStyles = {
    open Css;
    let select = style([height(px(37))]);
  };

  [@react.component]
  let make = (~filters: t, ~onChange, ~userItemIds: array(int)) => {
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
      ~userItemIds: option(array(int))=?,
      ~isViewingSelf=false,
      (),
    ) => {
  let inputTextRef = React.useRef(Js.Nullable.null);
  let updateTextTimeoutRef = React.useRef(None);
  React.useEffect1(
    () => {
      Webapi.Dom.(
        Utils.getElementForDomRef(inputTextRef)
        ->unsafeAsHtmlInputElement
        ->HtmlInputElement.setValue(filters.text)
      );
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
      | "Escape" =>
        let url = ReasonReactRouter.dangerouslyGetInitialUrl();
        // don't trigger if ItemDetailOverlay is shown
        if (!(url.hash |> Js.Re.test_([%bs.re "/i(-?\d+)(:(\d+))?/g"]))) {
          onChange({...filters, text: ""});
        };
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
              500,
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
        | UserTimeUpdated => "time-updated"
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
            | "time-updated" => UserTimeUpdated
            | _ => SellPriceDesc
            },
        });
      }}
      className={Cn.make([Styles.select, Styles.selectSort])}>
      {if (userItemIds !== None) {
         <>
           <option value="user-default">
             {React.string(isViewingSelf ? "Sort: Category" : "Sort: Default")}
           </option>
           <option value="time-updated">
             {React.string({j|Time Updated ↓|j})}
           </option>
         </>;
       } else {
         React.null;
       }}
      <option value="sell-desc">
        {React.string({j|Sell Price ↓|j})}
      </option>
      <option value="sell-asc"> {React.string({j|Sell Price ↑|j})} </option>
      <option value="abc"> {React.string("A - Z")} </option>
    </select>
    {if (filters.text != ""
         || filters.mask != None
         || filters.exclude != [||]
         || filters.category != None) {
       <a
         href="#"
         onClick={e => {
           ReactEvent.Mouse.preventDefault(e);
           onChange({
             text: "",
             mask: None,
             category: None,
             exclude: [||],
             sort: filters.sort,
           });
         }}
         className=Styles.clearFilters>
         {React.string("Clear filters")}
       </a>;
     } else {
       React.null;
     }}
    <div className=Styles.rootScrollerSpacer />
  </div>;
};