open Belt;

module Styles = {
  open Css;
  let root =
    style([
      padding2(~v=zero, ~h=px(16)),
      margin3(~top=px(16), ~bottom=px(32), ~h=auto),
      maxWidth(px(512)),
    ]);
  let topBlurb =
    style([
      backgroundColor(hex("ffffffc0")),
      boxSizing(borderBox),
      lineHeight(px(20)),
      marginBottom(px(32)),
      padding2(~v=px(16), ~h=px(24)),
      borderRadius(px(8)),
      selector(
        "& > p",
        [
          marginTop(zero),
          marginBottom(px(8)),
          lastChild([marginBottom(zero)]),
        ],
      ),
      media(
        "(max-width: 512px)",
        [
          padding(px(16)),
          marginLeft(px(-16)),
          marginRight(px(-16)),
          borderRadius(zero),
        ],
      ),
    ]);
  let textarea =
    style([
      borderRadius(px(4)),
      borderColor(Colors.lightGray),
      width(pct(100.)),
      minHeight(px(256)),
      padding(px(8)),
      boxSizing(borderBox),
      marginBottom(px(8)),
    ]);
  let searchButtonRow = style([display(flexBox), justifyContent(flexEnd)]);

  let sectionTitle = style([fontSize(px(24)), marginBottom(px(8))]);
  let missingRows = style([marginBottom(px(64))]);
  let missingRow = style([color(Colors.red)]);
  let matchRows = style([marginBottom(px(64))]);
  let itemRow =
    style([
      display(flexBox),
      media("(max-width: 550px)", [flexWrap(wrap), marginBottom(px(8))]),
    ]);
  let itemRowName =
    style([
      flexGrow(1.),
      fontSize(px(16)),
      paddingRight(px(16)),
      paddingTop(px(7)),
      media(
        "(max-width: 550px)",
        [width(pct(100.)), marginBottom(px(4))],
      ),
    ]);
  let itemRowNameLink =
    style([
      color(Colors.charcoal),
      textDecoration(none),
      hover([textDecoration(none)]),
    ]);
  let itemRowVariants =
    style([media("(max-width: 550px)", [width(pct(100.))])]);

  let variantRowImageLink = style([marginRight(px(8))]);
  let variantRowImage =
    style([display(block), width(px(32)), height(px(32))]);
  let variantRowName =
    style([
      width(px(128)),
      media("(max-width: 550px)", [width(auto), flexGrow(1.)]),
    ]);
  let variantRow =
    style([
      display(flexBox),
      alignItems(center),
      media(
        "(hover: hover)",
        [
          hover([
            selector("& ." ++ variantRowName, [fontWeight(`num(800))]),
          ]),
        ],
      ),
    ]);
  let variantRowUserStatus = style([width(px(32))]);
  let radioButtons = style([display(flexBox)]);
  let radioButton =
    style([
      backgroundColor(Colors.white),
      borderStyle(solid),
      borderColor(Colors.lightGray),
      borderTopWidth(px(1)),
      borderBottomWidth(px(1)),
      borderLeftWidth(px(1)),
      borderRightWidth(zero),
      padding2(~v=px(1), ~h=px(6)),
      firstChild([
        borderTopLeftRadius(px(4)),
        borderBottomLeftRadius(px(4)),
      ]),
      lastChild([
        borderRightWidth(px(1)),
        borderTopRightRadius(px(4)),
        borderBottomRightRadius(px(4)),
      ]),
    ]);
  let radioButtonDisabled = style([opacity(0.1)]);
  let radioButtonSelected =
    style([
      backgroundColor(hex("3aa56380")),
      borderColor(Colors.green),
      selector("& + ." ++ radioButton, [borderLeftColor(Colors.green)]),
    ]);

  let resultsOverlay =
    style([
      position(fixed),
      bottom(zero),
      backgroundColor(Colors.white),
      padding(px(16)),
      borderTopLeftRadius(px(8)),
      borderTopRightRadius(px(8)),
      left(pct(50.)),
      width(px(512)),
      boxSizing(borderBox),
      marginLeft(px(-256)),
      display(flexBox),
      alignItems(center),
      justifyContent(spaceBetween),
      Colors.darkLayerShadow,
      media(
        "(max-width: 550px)",
        [width(auto), left(zero), right(zero), marginLeft(zero)],
      ),
    ]);
  let bulkActionsLink =
    style([textDecoration(none), hover([textDecoration(underline)])]);
  let bulkActionsButtons =
    style([
      display(flexBox),
      flexDirection(column),
      padding(px(16)),
      backgroundColor(Colors.white),
      borderRadius(px(8)),
      Colors.darkLayerShadow,
      selector(
        "& > a",
        [textDecoration(none), hover([textDecoration(underline)])],
      ),
    ]);
};

[@bs.deriving jsConverter]
type itemDestination = [
  | [@bs.as "For Trade"] `ForTrade
  | [@bs.as "Can Craft"] `CanCraft
  | [@bs.as "Catalog Only"] `CatalogOnly
  | [@bs.as "Ignore"] `Ignore
];

let itemDestinationToEmoji = destination => {
  switch (destination) {
  | `ForTrade => {j|🤝|j}
  | `CanCraft => {j|🔨|j}
  | `CatalogOnly => {j|📖|j}
  | `Ignore => {j|🤝|j}
  };
};

type itemState = {
  itemId: int,
  selectedVariants: array(int),
  destination: itemDestination,
};

module VariantRow = {
  [@react.component]
  let make = (~item: Item.t, ~variant, ~itemsState, ~onChange) => {
    let renderDestinationOption = destination => {
      let disabled = destination == `CanCraft && item.recipe == None;
      <button
        onClick={_ => {onChange(item.id, variant, destination)}}
        disabled
        className={Cn.make([
          Styles.radioButton,
          Cn.ifTrue(
            Styles.radioButtonSelected,
            Js.Dict.get(
              itemsState,
              User.getItemKey(~itemId=item.id, ~variation=variant),
            )
            == Some(destination),
          ),
        ])}>
        <span className={Cn.ifTrue(Styles.radioButtonDisabled, disabled)}>
          {React.string(
             destination == `Ignore
               ? "Skip" : itemDestinationToEmoji(destination),
           )}
        </span>
      </button>;
    };

    <div className=Styles.variantRow>
      <Link
        path={ItemDetailOverlay.getItemDetailUrl(
          ~itemId=item.id,
          ~variant=Some(variant),
        )}
        className=Styles.variantRowImageLink>
        <img
          src={Item.getImageUrl(~item, ~variant)}
          className=Styles.variantRowImage
        />
      </Link>
      <div className=Styles.variantRowName>
        {switch (Item.getVariantName(~item, ~variant, ~hidePattern=true, ())) {
         | Some(variantName) => React.string(variantName)
         | None => React.null
         }}
      </div>
      <div className=Styles.variantRowUserStatus>
        {switch (UserStore.getItem(~itemId=item.id, ~variation=variant)) {
         | Some(userItem) =>
           <div>
             {React.string(User.itemStatusToEmoji(userItem.status))}
           </div>
         | None => React.null
         }}
      </div>
      <div className=Styles.radioButtons>
        {renderDestinationOption(`ForTrade)}
        {renderDestinationOption(`CanCraft)}
        {renderDestinationOption(`CatalogOnly)}
        {renderDestinationOption(`Ignore)}
      </div>
    </div>;
  };
};

module ResultRowWithItem = {
  [@react.component]
  let make =
      (
        ~query,
        ~item,
        ~itemsState,
        ~onChange: (int, int, itemDestination) => unit,
      ) => {
    let collapsedVariants = Item.getCollapsedVariants(~item);

    <div className=Styles.itemRow>
      <div className=Styles.itemRowName>
        <Link
          path={ItemDetailOverlay.getItemDetailUrl(
            ~itemId=item.id,
            ~variant=None,
          )}
          className=Styles.itemRowNameLink>
          {React.string(Item.getName(item))}
        </Link>
      </div>
      <div className=Styles.itemRowVariants>
        {collapsedVariants
         |> Js.Array.map(variant => {
              <VariantRow
                item
                variant
                itemsState
                onChange
                key={string_of_int(variant)}
              />
            })
         |> React.array}
      </div>
    </div>;
  };
};

module BulkActions = {
  [@react.component]
  let make = (~setItemStates) => {
    let (showPopup, setShowPopup) = React.useState(() => false);
    let reference = React.useRef(Js.Nullable.null);

    <>
      <a
        href="#"
        onClick={e => {
          ReactEvent.Mouse.preventDefault(e);
          setShowPopup(show => !show);
        }}
        className=Styles.bulkActionsLink
        ref={ReactDOMRe.Ref.domRef(reference)}>
        {React.string("Bulk actions")}
      </a>
      {showPopup
         ? <ReactAtmosphere.PopperLayer
             reference
             onOutsideClick={() => setShowPopup(_ => false)}
             options={
               placement: Some("top"),
               modifiers:
                 Some([|
                   {
                     "name": "offset",
                     "options": {
                       "offset": [|0, 4|],
                     },
                   },
                 |]),
             }
             render={_ =>
               <div className=Styles.bulkActionsButtons>
                 <a
                   href="#"
                   onClick={e => {
                     ReactEvent.Mouse.preventDefault(e);
                     setItemStates(itemStates => {
                       itemStates
                       |> Js.Dict.entries
                       |> Js.Array.map(((key, _value)) => (key, `Ignore))
                       |> Js.Dict.fromArray
                     });
                     setShowPopup(_ => false);
                   }}>
                   {React.string("Set every item to Skip")}
                 </a>
                 <a
                   href="#"
                   onClick={e => {
                     ReactEvent.Mouse.preventDefault(e);
                     setItemStates(itemStates => {
                       itemStates
                       |> Js.Dict.entries
                       |> Js.Array.map(((key, _value)) => (key, `ForTrade))
                       |> Js.Dict.fromArray
                     });
                     setShowPopup(_ => false);
                   }}>
                   {React.string("Set every item to For Trade")}
                 </a>
                 <a
                   href="#"
                   onClick={e => {
                     ReactEvent.Mouse.preventDefault(e);
                     setItemStates(itemStates => {
                       itemStates
                       |> Js.Dict.entries
                       |> Js.Array.map(((key, _value)) =>
                            (key, `CatalogOnly)
                          )
                       |> Js.Dict.fromArray
                     });
                     setShowPopup(_ => false);
                   }}>
                   {React.string("Set every item to Catalog")}
                 </a>
                 <a
                   href="#"
                   onClick={e => {
                     ReactEvent.Mouse.preventDefault(e);
                     setItemStates(itemStates => {
                       itemStates
                       |> Js.Dict.entries
                       |> Js.Array.map(((key, value)) => {
                            let (itemId, _variant) =
                              User.fromItemKey(~key)->Option.getExn;
                            let item = Item.getItem(~itemId);
                            (
                              key,
                              if (item.recipe != None) {
                                `CanCraft;
                              } else {
                                value;
                              },
                            );
                          })
                       |> Js.Dict.fromArray
                     });
                     setShowPopup(_ => false);
                   }}>
                   {React.string("Set every craftable item to Can Craft")}
                 </a>
               </div>
             }
           />
         : React.null}
    </>;
  };
};

module Results = {
  [@react.component]
  let make = (~rows: array((string, option(Item.t)))) => {
    let missingRows = rows->Array.keep(((_, item)) => item == None);
    let (itemsState, setItemStates) =
      React.useState(() => {
        Js.Dict.fromArray(
          Array.concatMany(
            rows->Array.map(((query, itemOption)) =>
              switch (itemOption) {
              | Some(item) =>
                let collapsedVariants = Item.getCollapsedVariants(~item);
                let numVariants = Js.Array.length(collapsedVariants);
                collapsedVariants->Belt.Array.map(variant => {
                  (
                    User.getItemKey(~itemId=item.id, ~variation=variant),
                    switch (
                      UserStore.getItem(~itemId=item.id, ~variation=variant)
                    ) {
                    | Some(userItem) =>
                      switch (userItem.status) {
                      | CanCraft => `CanCraft
                      | ForTrade => `ForTrade
                      | CatalogOnly => `CatalogOnly
                      | Wishlist => `Ignore
                      }
                    | None =>
                      numVariants > 1
                        ? `Ignore : item.recipe != None ? `CanCraft : `ForTrade
                    },
                  )
                });
              | None => [||]
              }
            ),
          ),
        )
      });

    <div>
      {Js.Array.length(missingRows) > 0
         ? <div className=Styles.missingRows>
             <div className=Styles.sectionTitle>
               {React.string("Items without matches")}
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
             {rows
              ->Belt.Array.keepMap(((query, item)) =>
                  item->Option.map(item =>
                    <ResultRowWithItem
                      query
                      item
                      itemsState
                      onChange={(itemId, variant, destination) => {
                        setItemStates(itemsState => {
                          let clone = Utils.cloneJsDict(itemsState);
                          clone->Js.Dict.set(
                            User.getItemKey(~itemId, ~variation=variant),
                            destination,
                          );
                          clone;
                        })
                      }}
                      key={item.name}
                    />
                  )
                )
              ->React.array}
           </div>
         : <div> {React.string("No items were matched.")} </div>}
      <div className=Styles.resultsOverlay>
        <BulkActions setItemStates />
        <Button
          onClick={_ => {
            let numForTrade = ref(0);
            let numCanCraft = ref(0);
            let numCatalog = ref(0);
            itemsState
            ->Js.Dict.entries
            ->Array.forEach(((_itemKey, destination)) => {
                switch (destination) {
                | `ForTrade => numForTrade := numForTrade^ + 1
                | `CanCraft => numCanCraft := numCanCraft^ + 1
                | `CatalogOnly => numCatalog := numCatalog^ + 1
                | `Ignore => ()
                }
              });
            ConfirmDialog.confirm(
              ~bodyText=
                "This will add "
                ++ Js.Array.joinWith(
                     ", ",
                     Array.keepMap(
                       [|
                         switch (numForTrade^) {
                         | 0 => None
                         | numForTrade =>
                           Some(string_of_int(numForTrade) ++ " For Trade")
                         },
                         switch (numCanCraft^) {
                         | 0 => None
                         | numCanCraft =>
                           Some(string_of_int(numCanCraft) ++ " Can Craft")
                         },
                         switch (numCatalog^) {
                         | 0 => None
                         | numCatalog =>
                           Some(string_of_int(numCatalog) ++ " Catalog")
                         },
                       |],
                       x =>
                       x
                     ),
                   )
                ++ " items. Are you sure you want to continue?",
              ~confirmLabel="Do it!",
              ~cancelLabel="Never mind",
              ~onConfirm=() => {Js.log("okay")},
              (),
            );
          }}>
          {React.string("Save changes")}
        </Button>
      </div>
    </div>;
  };
};

let process = value => {
  let rows =
    value
    |> Js.String.split("\n")
    |> Js.Array.map(Js.String.trim)
    |> Js.Array.filter(x => x != "");
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
    <PageTitle title="Import items" />
    {switch (results) {
     | Some(results) =>
       <div>
         <div className=Styles.topBlurb>
           <p>
             {React.string(
                {j|For each item, choose For Trade 🤝, Can Craft 🔨, Catalog 📖 or skip. If the item is already in your profile, you will see the status next to the selector.|j},
              )}
           </p>
           <p>
             {React.string(
                "When you are done, press the Save button in the bottom bar. There are bulk actions to help as well.",
              )}
           </p>
         </div>
         <Results rows=results />
       </div>
     | None =>
       <div>
         <div className=Styles.topBlurb>
           <p>
             {React.string(
                "Have a big collection? This tool can help you add many items at once. Start by pasting a list of item names in the textbox. ",
              )}
           </p>
           <p>
             {React.string(
                "If you don't want to type each item by hand, check out ",
              )}
             <a href="https://twitter.com/CatalogScanner" target="_blank">
               {React.string("Catalog Scanner")}
             </a>
             {React.string("!")}
           </p>
         </div>
         <form onSubmit>
           <textarea
             value
             placeholder="Enter each item name on its own line"
             onChange={e => {
               let value = ReactEvent.Form.target(e)##value;
               setValue(_ => value);
             }}
             className=Styles.textarea
           />
           <div className=Styles.searchButtonRow>
             <Button> {React.string("Search for items")} </Button>
           </div>
         </form>
       </div>
     }}
  </div>;
};