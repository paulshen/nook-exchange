open Belt;

module Styles = {
  open Css;
  let root =
    style([
      padding2(~v=zero, ~h=px(16)),
      margin3(~top=px(48), ~bottom=px(32), ~h=auto),
      maxWidth(px(720)),
    ]);
  let textarea = style([width(pct(100.)), minHeight(px(256))]);

  let rowImages = style([display(flexBox), flexWrap(wrap)]);
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
      <label>
        {React.string(itemDestinationToJs(destination))}
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
      </label>;
    };

    let onClickVariant = v => {
      let selectedVariants =
        if (itemState.selectedVariants |> Js.Array.includes(v)) {
          itemState.selectedVariants->Array.keep(x => x != v);
        } else {
          itemState.selectedVariants->Array.concat([|v|]);
        };
      onChange({...itemState, selectedVariants});
    };

    <tr>
      <td> {React.string(query)} </td>
      <td> {React.string(item.id)} </td>
      <td>
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
      </td>
      <td>
        {renderDestinationOption(`ForTrade)}
        {renderDestinationOption(`CatalogOnly)}
        {renderDestinationOption(`Ignore)}
      </td>
    </tr>;
  };
};

module Results = {
  [@react.component]
  let make = (~rows: array((string, option(Item.t)))) => {
    let (itemsState, setItemStates) =
      React.useState(() => {
        Js.Dict.fromArray(
          rows->Array.keepMap(((query, itemOption)) =>
            itemOption->Option.map(item =>
              (
                query,
                {
                  itemId: item.id,
                  selectedVariants:
                    Array.make(Item.getNumVariations(~item), None)
                    ->Array.mapWithIndex((i, _) => i),
                  destination: `CatalogOnly,
                }: itemState,
              )
            )
          ),
        )
      });
    <table>
      <tbody>
        {rows
         ->Belt.Array.mapWithIndex((i, (query, item)) =>
             switch (item) {
             | Some(item) =>
               <ResultRowWithItem
                 query
                 item
                 itemState={itemsState->Js.Dict.get(query)->Option.getExn}
                 onChange={itemState => {
                   setItemStates(itemsState => {
                     let clone = Utils.cloneJsDict(itemsState);
                     clone->Js.Dict.set(query, itemState);
                     clone;
                   })
                 }}
                 key={string_of_int(i)}
               />
             | None =>
               <tr key={string_of_int(i)}>
                 <td> {React.string(query)} </td>
                 <td> {React.string("Item not found")} </td>
                 <td />
               </tr>
             }
           )
         ->React.array}
      </tbody>
    </table>;
  };
};

let process = value => {
  let rows = value |> Js.String.split("\n");
  let results =
    rows->Belt.Array.map(row => (row, Item.getByName(~name=row)));
  Js.log(results);
  Js.log(results->Belt.Array.keep(((_, x)) => x === None));
  Js.log(results->Belt.Array.keep(((_, x)) => x !== None));
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
    <div> {React.string("Import!")} </div>
    <div>
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
          <Button> {React.string("Import!")} </Button>
        </form>
      </div>
    </div>
  </div>;
};