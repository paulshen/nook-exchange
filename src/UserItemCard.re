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
  let wishlistEllipsisButton =
    style([position(absolute), top(px(8)), right(px(8))]);
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
  let nameLink =
    style([
      color(Colors.charcoal),
      textDecoration(none),
      hover([textDecoration(underline)]),
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
        selector("& ." ++ ItemImage.Styles.variantButton, [opacity(0.5)]),
      ]),
    ]);
  let cardOnCatalogPage = style([paddingBottom(px(16))]);
  let itemImage = style([marginLeft(px(-8)), marginRight(px(-8))]);
  let name =
    style([fontSize(px(16)), marginBottom(px(4)), textAlign(center)]);
  let userItemNote = style([marginTop(px(8))]);
  let userNote =
    style([
      borderTop(px(1), solid, hex("f0f0f0")),
      unsafe("alignSelf", "stretch"),
      lineHeight(px(18)),
      marginTop(px(4)),
      padding3(~top=px(8), ~bottom=zero, ~h=px(4)),
    ]);
  let removeButton =
    style([
      position(absolute),
      top(px(10)),
      right(px(10)),
      selector("." ++ card ++ ":hover &", [opacity(0.8)]),
    ]);
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
      <ItemImage
        item
        variant=variation
        forceTooltip=true
        narrow=true
        className={Cn.make([ItemCard.Styles.itemImage, Styles.itemImage])}
      />
      <div className=Styles.name>
        <Link
          path={ItemDetailOverlay.getItemDetailUrl(
            ~itemId=item.id,
            ~variant=variation != 0 ? Some(variation) : None,
          )}
          className=Styles.nameLink>
          {React.string(Item.getName(item))}
        </Link>
      </div>
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
         | CatalogOnly => React.null
         | Wishlist => raise(Constants.Uhoh)
         }
       )}
    {editable
       ? <>
           {!onCatalogPage
              ? <UserItemNote
                  itemId={item.id}
                  variation
                  userItem
                  className=Styles.userItemNote
                  key={string_of_int(variation)}
                />
              : React.null}
           {userItem.status == Wishlist
              ? <WishlistEllipsisButton
                  item
                  variation
                  className=Styles.wishlistEllipsisButton
                />
              : <ReactAtmosphere.Tooltip text={React.string("Remove item")}>
                  {({onMouseEnter, onMouseLeave, onFocus, onBlur, ref}) =>
                     <RemoveButton
                       className=Styles.removeButton
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
                               UserStore.removeItem(
                                 ~itemId=item.id,
                                 ~variation,
                               )
                             )
                           | CatalogOnly =>
                             UserStore.removeItem(~itemId=item.id, ~variation)
                           | Wishlist => raise(Constants.Uhoh)
                           };
                         } else {
                           UserStore.removeItem(~itemId=item.id, ~variation);
                         }
                       }
                       ref
                     />}
                </ReactAtmosphere.Tooltip>}
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