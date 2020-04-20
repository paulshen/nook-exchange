module Styles = {
  open Css;
  let root =
    style([
      padding2(~v=zero, ~h=px(16)),
      margin3(~top=px(32), ~bottom=zero, ~h=auto),
      media("(min-width: 600px)", [width(px(544))]),
      media("(min-width: 940px)", [width(px(832))]),
      media("(min-width: 1200px)", [width(px(1120))]),
      media("(min-width: 1520px)", [width(px(1408))]),
    ]);
  let tagline =
    style([
      textAlign(center),
      marginBottom(px(48)),
      media("(max-width: 600px)", [display(none)]),
    ]);
  let cards =
    style([
      display(flexBox),
      flexWrap(wrap),
      marginRight(px(-32)),
      justifyContent(center),
      media("(max-width: 600px)", [marginRight(px(-16))]),
    ]);
  let filterBar =
    style([
      display(flexBox),
      justifyContent(spaceBetween),
      marginBottom(px(16)),
      flexWrap(wrap),
    ]);
  let bottomFilterBar = style([display(flexBox), justifyContent(flexEnd)]);
  let noResults = style([fontSize(px(20)), paddingTop(px(32))]);
};

let getNumResultsPerPage = () => {
  open Webapi.Dom;
  let windowWidth = window |> Window.innerWidth;
  if (windowWidth >= 1520) {
    30;
  } else {
    24;
  };
};

[@react.component]
let make = (~showLogin) => {
  let (numResultsPerPage, _setNumResultsPerPage) =
    React.useState(() => getNumResultsPerPage());
  let (filters, setFilters) =
    React.useState(() =>
      (
        {text: "", mask: None, category: None, sort: SellPriceDesc}: ItemFilters.t
      )
    );
  let (pageOffset, setPageOffset) = React.useState(() => 0);
  let setFilters =
    React.useCallback0(f => {
      setFilters((filters: ItemFilters.t) => {
        let updatedFilters: ItemFilters.t = f(filters);
        Analytics.Amplitude.logEventWithProperties(
          ~eventName="Filters Changed",
          ~eventProperties={
            "filterText": updatedFilters.text,
            "filterMask": updatedFilters.mask,
            "filterCategory": updatedFilters.category,
            "filterSort": updatedFilters.sort,
          },
        );
        updatedFilters;
      })
    });
  let filteredItems =
    React.useMemo1(
      () =>
        Item.all->Belt.Array.keep(item =>
          ItemFilters.doesItemMatchFilters(~item, ~filters)
        )
        |> Js.Array.sortInPlaceWith(ItemFilters.getSort(~filters)),
      [|filters|],
    );
  let numResults = filteredItems->Belt.Array.length;

  <div className=Styles.root>
    <div className=Styles.tagline>
      {React.string("Your friendly Animal Crossing marketplace!")}
    </div>
    <ItemFilters.CategoryButtons
      filters
      onChange={filters => {
        setFilters(_ => filters);
        setPageOffset(_ => 0);
      }}
    />
    <div className=Styles.filterBar>
      <ItemFilters
        filters
        onChange={filters => {
          setFilters(_ => filters);
          setPageOffset(_ => 0);
        }}
      />
      <ItemFilters.Pager
        numResults
        pageOffset
        numResultsPerPage
        setPageOffset
      />
    </div>
    <div className=Styles.cards>
      {filteredItems
       ->Belt.Array.slice(
           ~offset=pageOffset * numResultsPerPage,
           ~len=numResultsPerPage,
         )
       ->Belt.Array.mapWithIndexU((. i, item) => {
           <ItemCard item showLogin key={item.id ++ string_of_int(i)} />
         })
       ->React.array}
    </div>
    <div className=Styles.bottomFilterBar>
      <ItemFilters.Pager
        numResults
        pageOffset
        numResultsPerPage
        setPageOffset={f => {
          setPageOffset(f);
          Webapi.Dom.(window |> Window.scrollTo(0., 0.));
        }}
      />
    </div>
    {if (Js.Array.length(filteredItems) == 0) {
       <div className=Styles.noResults>
         {React.string("There are no results. Try changing or ")}
         <a
           href="#"
           onClick={e => {
             setFilters(_ =>
               (
                 {text: "", mask: None, category: None, sort: SellPriceDesc}: ItemFilters.t
               )
             );
             ReactEvent.Mouse.preventDefault(e);
           }}>
           {React.string("clearing the filters!")}
         </a>
       </div>;
     } else {
       React.null;
     }}
  </div>;
};