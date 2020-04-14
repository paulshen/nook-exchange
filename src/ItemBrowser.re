module Styles = {
  open Css;
  let root =
    style([
      width(px(256)),
      margin2(~v=zero, ~h=auto),
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
      paddingBottom(px(32)),
    ]);
};

let numResultsPerPage = 20;

[@react.component]
let make = (~showLogin) => {
  let (filters, setFilters) =
    React.useState(() =>
      ({orderable: None, hasRecipe: None, category: None}: ItemFilters.t)
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
    <div>
      <ItemFilters
        filters
        onChange={filters => {
          setFilters(_ => filters);
          setPageOffset(_ => 0);
        }}
      />
      {if (numResults > 0) {
         <div>
           {React.string(
              "Showing "
              ++ string_of_int(pageOffset * numResultsPerPage + 1)
              ++ "-"
              ++ string_of_int(
                   Js.Math.min_int(
                     (pageOffset + 1) * numResultsPerPage,
                     numResults,
                   ),
                 )
              ++ " of "
              ++ string_of_int(numResults),
            )}
         </div>;
       } else {
         React.null;
       }}
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
    {if (Js.Array.length(filteredItems) == 0) {
       <div>
         {React.string(
            "There are no results. Try changing or clearing the filters!",
          )}
       </div>;
     } else {
       React.null;
     }}
  </div>;
};