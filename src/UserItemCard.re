module Styles = {
  open Css;
  let metaIcons =
    style([
      position(absolute),
      top(px(6)),
      left(px(6)),
      opacity(0.),
      transition(~duration=200, "all"),
    ]);
  let topRightIcon =
    style([
      position(absolute),
      top(px(7)),
      right(px(10)),
      fontSize(px(13)),
      boxSizing(borderBox),
      cursor(`default),
      width(px(20)),
      height(px(20)),
      textAlign(center),
      opacity(0.5),
      transition(~duration=200, "all"),
    ]);
  let topRightIconSelected = style([opacity(1.)]);
  let catalogStatusButton =
    style([
      position(absolute),
      top(px(8)),
      left(px(8)),
      fontSize(px(14)),
      opacity(0.),
      cursor(`default),
      transition(~duration=200, "all"),
      media("(hover: none)", [opacity(0.8)]),
    ]);
  let card =
    style([
      backgroundColor(hex("fffffff0")),
      borderRadius(px(8)),
      display(flexBox),
      flexDirection(column),
      alignItems(center),
      marginRight(px(16)),
      marginBottom(px(16)),
      padding3(~top=px(24), ~bottom=px(8), ~h=px(8)),
      position(relative),
      boxSizing(borderBox),
      width(Calc.(pct(50.) - px(16))),
      transition(~duration=200, "all"),
      media("(min-width: 660px)", [width(px(176))]),
      hover([
        selector("& ." ++ metaIcons, [opacity(1.)]),
        selector("& ." ++ topRightIcon, [opacity(1.)]),
        selector("& ." ++ catalogStatusButton, [opacity(1.)]),
      ]),
    ]);
  let cardOnCatalogPage = style([paddingBottom(px(16))]);
  let mainImageWrapperWithRecipe = style([marginBottom(px(16))]);
  let name =
    style([fontSize(px(16)), marginBottom(px(4)), textAlign(center)]);
  let userNote =
    style([
      borderTop(px(1), solid, hex("f0f0f0")),
      unsafe("alignSelf", "stretch"),
      lineHeight(px(18)),
      marginTop(px(8)),
      padding3(~top=px(8), ~bottom=zero, ~h=px(4)),
    ]);
  let removeButton = style([top(px(8)), bottom(initial)]);
  let recipe =
    style([marginTop(px(6)), textAlign(center), fontSize(px(12))]);
};

[@react.component]
let make =
    (
      ~itemId,
      ~variation,
      ~userItem: User.item,
      ~listStatus,
      ~editable,
      ~showRecipe,
      ~onCatalogPage=false,
      (),
    ) => {
  let item = Item.getItem(~itemId);
  let viewerItem = UserStore.useItem(~itemId, ~variation);
  <div
    className={Cn.make([
      Styles.card,
      Cn.ifTrue(Styles.cardOnCatalogPage, onCatalogPage),
    ])}>
    <div className=ItemCard.Styles.body>
      <div
        className={Cn.make([
          ItemCard.Styles.mainImageWrapper,
          Cn.ifTrue(Styles.mainImageWrapperWithRecipe, item.isRecipe),
        ])}>
        <img
          src={Item.getImageUrl(~item, ~variant=variation)}
          className=ItemCard.Styles.mainImage
        />
        {item.isRecipe
           ? <img
               src={Constants.cdnUrl ++ "/images/DIYRecipe.png"}
               className=ItemCard.Styles.recipeIcon
             />
           : React.null}
      </div>
      <div className=Styles.name> {React.string(item.name)} </div>
      {switch (showRecipe, item.recipe) {
       | (true, Some(recipe)) =>
         <div className=Styles.recipe>
           {recipe
            ->Belt.Array.map(((itemId, quantity)) =>
                <div key=itemId>
                  {React.string(itemId ++ " x " ++ string_of_int(quantity))}
                </div>
              )
            ->React.array}
         </div>
       | _ => React.null
       }}
    </div>
    {!onCatalogPage
       ? <div className=Styles.metaIcons>
           {switch (item.recipe) {
            | Some(recipe) => <ItemCard.RecipeIcon recipe />
            | None => React.null
            }}
           {if (item.orderable) {
              <ItemCard.OrderableIcon />;
            } else {
              React.null;
            }}
         </div>
       : (
         switch (userItem.status) {
         | CanCraft
         | ForTrade =>
           <ReactAtmosphere.Tooltip
             text={React.string(
               "In "
               ++ (userItem.status == ForTrade ? "For Trade" : "Can Craft"),
             )}
             options={Obj.magic({"modifiers": None})}>
             {(
                ({onMouseEnter, onMouseLeave, onFocus, onBlur, ref}) =>
                  <div
                    onMouseEnter
                    onMouseLeave
                    onFocus
                    onBlur
                    className=Styles.catalogStatusButton
                    ref={ReactDOMRe.Ref.domRef(ref)}>
                    {React.string(
                       userItem.status == ForTrade ? {j|🤝|j} : {j|🔨|j},
                     )}
                  </div>
              )}
           </ReactAtmosphere.Tooltip>
         | InCatalog => React.null
         | Wishlist => raise(Constants.Uhoh)
         }
       )}
    {editable
       ? <>
           {!onCatalogPage
              ? <UserItemNote itemId={item.id} variation userItem />
              : React.null}
           <ReactAtmosphere.Tooltip text={React.string("Remove item")}>
             {({onMouseEnter, onMouseLeave, onFocus, onBlur, ref}) =>
                <button
                  className={Cn.make([
                    ItemCard.Styles.removeButton,
                    Styles.removeButton,
                  ])}
                  onMouseEnter
                  onMouseLeave
                  onFocus
                  onBlur
                  onClick={_ =>
                    if (onCatalogPage) {
                      switch (userItem.status) {
                      | CanCraft
                      | ForTrade =>
                        DeleteFromCatalog.confirm(~onConfirm=() =>
                          UserStore.removeItem(~itemId=item.id, ~variation)
                        )
                      | InCatalog =>
                        UserStore.removeItem(~itemId=item.id, ~variation)
                      | Wishlist => raise(Constants.Uhoh)
                      };
                    } else {
                      UserStore.removeItem(~itemId=item.id, ~variation);
                    }
                  }
                  ref={ReactDOMRe.Ref.domRef(ref)}>
                  {React.string({j|❌|j})}
                </button>}
           </ReactAtmosphere.Tooltip>
         </>
       : <>
           {if (userItem.note->Js.String.length > 0) {
              <div className=Styles.userNote>
                {Emoji.parseText(userItem.note)}
              </div>;
            } else {
              React.null;
            }}
           {switch (viewerItem) {
            | Some(viewerItem) =>
              <ReactAtmosphere.Tooltip
                text={React.string(
                  "In your " ++ User.itemStatusToString(viewerItem.status),
                )}>
                {(
                   ({onMouseEnter, onMouseLeave, ref}) =>
                     <div
                       onMouseEnter
                       onMouseLeave
                       className={Cn.make([
                         Styles.topRightIcon,
                         Cn.ifTrue(
                           Styles.topRightIconSelected,
                           User.(
                             switch (listStatus, viewerItem.status) {
                             | (ForTrade, Wishlist)
                             | (CanCraft, Wishlist)
                             | (Wishlist, ForTrade)
                             | (Wishlist, CanCraft) => true
                             | _ => false
                             }
                           ),
                         ),
                       ])}
                       ref={ReactDOMRe.Ref.domRef(ref)}>
                       {React.string(
                          User.itemStatusToEmoji(viewerItem.status),
                        )}
                     </div>
                 )}
              </ReactAtmosphere.Tooltip>
            | None => React.null
            }}
         </>}
  </div>;
};