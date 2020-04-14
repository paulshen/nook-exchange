module Styles = {
  open Css;
  let root =
    style([
      width(px(256)),
      margin3(~top=px(32), ~bottom=zero, ~h=auto),
      media("(min-width: 600px)", [width(px(544))]),
      media("(min-width: 940px)", [width(px(832))]),
      media("(min-width: 1200px)", [width(px(1120))]),
      media("(min-width: 1520px)", [width(px(1408))]),
    ]);
  let cards =
    style([
      display(flexBox),
      flexWrap(wrap),
      marginRight(px(-32)),
      paddingTop(px(32)),
    ]);
  let filterBar =
    style([
      display(flexBox),
      justifyContent(spaceBetween),
      alignItems(center),
    ]);
  let bottomFilterBar = style([display(flexBox), justifyContent(flexEnd)]);
  let noResults = style([fontSize(px(20)), paddingTop(px(16))]);
};

let numResultsPerPage = 20;

[@react.component]
let make = (~showLogin) => {
  let (filters, setFilters) =
    React.useState(() =>
      (
        {text: "", orderable: None, hasRecipe: None, category: None}: ItemFilters.t
      )
    );
  let (pageOffset, setPageOffset) = React.useState(() => 0);
  let filteredItems =
    React.useMemo1(
      () =>
        Item.all->Belt.Array.keep(item =>
          ItemFilters.doesItemMatchFilters(~item, ~filters)
        ),
      [|filters|],
    );
  let numResults = filteredItems->Belt.Array.length;

  <div className=Styles.root>
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
        setPageOffset
      />
    </div>
    {if (Js.Array.length(filteredItems) == 0) {
       <div className=Styles.noResults>
         {React.string(
            "There are no results. Try changing or clearing the filters!",
          )}
       </div>;
     } else {
       React.null;
     }}
  </div>;
};