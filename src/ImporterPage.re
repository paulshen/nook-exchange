open Belt;

module Styles = {
  open Css;
  let root =
    style([
      padding2(~v=zero, ~h=px(16)),
      margin3(~top=px(48), ~bottom=px(32), ~h=auto),
      maxWidth(px(720)),
    ]);
  let textarea =
    style([
      width(pct(100.)),
      minHeight(px(256)),
      boxSizing(borderBox),
      marginBottom(px(8)),
    ]);
  let searchButtonRow = style([display(flexBox), justifyContent(flexEnd)]);

  let sectionTitle = style([fontSize(px(32)), marginBottom(px(16))]);
  let missingRows = style([marginBottom(px(64))]);
  let missingRow = style([color(Colors.red)]);
  let matchRows = style([marginBottom(px(64))]);
  let resultRow = style([display(flexBox), marginBottom(px(8))]);
  let resultQueryCell =
    style([
      width(pct(20.)),
      minWidth(px(128)),
      flexShrink(0.),
      paddingRight(px(16)),
    ]);
  let resultDestinationCell = style([whiteSpace(nowrap)]);
  let rowImages = style([display(flexBox), flexWrap(wrap)]);
  let resultVariantCell =
    style([flexGrow(1.), padding2(~v=zero, ~h=px(16))]);
  let variationImage =
    style([
      display(block),
      cursor(pointer),
      width(px(32)),
      height(px(32)),
      borderRadius(px(4)),
      hover([backgroundColor(hex("00000010"))]),
    ]);
  let variationImageSelected = style([backgroundColor(hex("3aa56380"))]);
};

[@bs.deriving jsConverter]
type itemDestination = [
  | [@bs.as "For Trade"] `ForTrade
  | [@bs.as "Can Craft"] `CanCraft
  | [@bs.as "Catalog Only"] `CatalogOnly
  | [@bs.as "Ignore"] `Ignore
];

type itemState = {
  itemId: string,
  selectedVariants: array(int),
  destination: itemDestination,
};

module ResultRowWithItem = {
  [@react.component]
  let make =
      (~query, ~item, ~itemState: itemState, ~onChange: itemState => unit) => {
    let numVariations = Item.getNumVariations(~item);
    let renderDestinationOption = destination => {
      <div>
        <label>
          <input
            type_="radio"
            name={item.id}
            value={itemDestinationToJs(destination)}
            checked={itemState.destination == destination}
            onChange={e => {
              let value = ReactEvent.Form.target(e)##value;
              onChange({
                ...itemState,
                destination: itemDestinationFromJs(value)->Option.getExn,
              });
            }}
          />
          {React.string(itemDestinationToJs(destination))}
        </label>
      </div>;
    };

    let getToggleTogether = v => {
      [%debugger];
      switch (item.variations) {
      | TwoDimensions(a, b) =>
        if (a > 1 && item.flags land 4 != 0) {
          Some(
            Array.make(a, None)
            ->Array.mapWithIndex((i, _) => i * a + v mod a),
          );
        } else if (b > 1 && item.flags land 8 != 0) {
          Some(
            Array.make(b, None)->Array.mapWithIndex((i, _) => v / b * b + i),
          );
        } else {
          None;
        }
      | _ => None
      };
    };

    let onClickVariant = v => {
      let selectedVariants =
        if (itemState.selectedVariants |> Js.Array.includes(v)) {
          let itemsToRemove =
            getToggleTogether(v)->Option.getWithDefault([|v|]);
          itemState.selectedVariants
          ->Array.keep(x => !(itemsToRemove |> Js.Array.includes(x)));
        } else {
          let itemsToAdd =
            getToggleTogether(v)->Option.getWithDefault([|v|]);
          let clone = Js.Array.copy(itemState.selectedVariants);
          itemsToAdd->Array.forEach(v =>
            if (!(clone |> Js.Array.includes(v))) {
              clone |> Js.Array.push(v) |> ignore;
            }
          );
          clone;
        };
      onChange({...itemState, selectedVariants});
    };

    <div className=Styles.resultRow>
      <div className=Styles.resultQueryCell> {React.string(query)} </div>
      <div className=Styles.resultVariantCell>
        <div className=Styles.rowImages>
          {let children = [||];
           for (v in 0 to numVariations - 1) {
             let image =
               <img
                 src={Item.getImageUrl(~item, ~variant=v)}
                 className={Cn.make([
                   Styles.variationImage,
                   Cn.ifTrue(
                     Styles.variationImageSelected,
                     Js.Array.includes(v, itemState.selectedVariants),
                   ),
                 ])}
               />;
             children
             |> Js.Array.push(
                  switch (Item.getVariantName(~item, ~variant=v)) {
                  | Some(variantName) =>
                    <ReactAtmosphere.Tooltip
                      text={React.string(variantName)}
                      key={string_of_int(v)}>
                      {(
                         ({onMouseEnter, onMouseLeave, onFocus, onBlur, ref}) =>
                           <div
                             onClick={_ => onClickVariant(v)}
                             onMouseEnter
                             onMouseLeave
                             onFocus
                             onBlur
                             ref={ReactDOMRe.Ref.domRef(ref)}>
                             image
                           </div>
                       )}
                    </ReactAtmosphere.Tooltip>
                  | None =>
                    <div
                      onClick={_ => onClickVariant(v)}
                      key={string_of_int(v)}>
                      image
                    </div>
                  },
                )
             |> ignore;
           };
           children->React.array}
        </div>
      </div>
      <div className=Styles.resultDestinationCell>
        {renderDestinationOption(`ForTrade)}
        {renderDestinationOption(`CatalogOnly)}
        {renderDestinationOption(`Ignore)}
      </div>
    </div>;
  };
};

module Results = {
  [@react.component]
  let make = (~rows: array((string, option(Item.t)))) => {
    let missingRows = rows->Array.keep(((_, item)) => item == None);
    let (itemsState, setItemStates) =
      React.useState(() => {
        Js.Dict.fromArray(
          rows->Array.keepMap(((query, itemOption)) =>
            itemOption->Option.map(item => {
              (
                query,
                {
                  itemId: item.id,
                  selectedVariants:
                    switch (item.variations) {
                    | Single => [|0|]
                    | OneDimension(a) =>
                      if (item.customizable) {
                        Array.make(a, None)->Array.mapWithIndex((i, _) => i);
                      } else {
                        [||];
                      }
                    | TwoDimensions(a, b) =>
                      if ((a == 1 || item.flags land 4 != 0)
                          && (b == 1 || item.flags land 8 != 0)) {
                        Array.make(a * b, None)
                        ->Array.mapWithIndex((i, _) => i);
                      } else {
                        [||];
                      }
                    },
                  destination: `CatalogOnly,
                }: itemState,
              )
            })
          ),
        )
      });
    <div>
      {Js.Array.length(missingRows) > 0
         ? <div className=Styles.missingRows>
             <div className=Styles.sectionTitle>
               {React.string("Missing rows")}
             </div>
             {missingRows
              ->Array.mapWithIndex((i, (query, _)) =>
                  <div className=Styles.missingRow key={string_of_int(i)}>
                    {React.string(query)}
                  </div>
                )
              ->React.array}
           </div>
         : React.null}
      {Js.Array.length(rows) > 0
         ? <div className=Styles.matchRows>
             <div className=Styles.sectionTitle>
               {React.string("Matches!")}
             </div>
             {rows
              ->Belt.Array.keepMap(((query, item)) =>
                  item->Option.map(item =>
                    <ResultRowWithItem
                      query
                      item
                      itemState={
                        itemsState->Js.Dict.get(query)->Option.getExn
                      }
                      onChange={itemState => {
                        setItemStates(itemsState => {
                          let clone = Utils.cloneJsDict(itemsState);
                          clone->Js.Dict.set(query, itemState);
                          clone;
                        })
                      }}
                      key={item.name}
                    />
                  )
                )
              ->React.array}
           </div>
         : React.null}
    </div>;
  };
};

let process = value => {
  let rows = value |> Js.String.split("\n");
  let results =
    rows->Belt.Array.map(row => (row, Item.getByName(~name=row)));
  results;
};

[@react.component]
let make = () => {
  let (value, setValue) = React.useState(() => "");
  let (results, setResults) = React.useState(() => None);
  let onSubmit = e => {
    ReactEvent.Form.preventDefault(e);
    setResults(_ => Some(process(value)));
  };

  <div className=Styles.root>
    {switch (results) {
     | Some(results) => <Results rows=results />
     | None => React.null
     }}
    <div>
      <form onSubmit>
        <textarea
          value
          onChange={e => {
            let value = ReactEvent.Form.target(e)##value;
            setValue(_ => value);
          }}
          className=Styles.textarea
        />
        <div className=Styles.searchButtonRow>
          <Button> {React.string("Search!")} </Button>
        </div>
      </form>
    </div>
  </div>;
};