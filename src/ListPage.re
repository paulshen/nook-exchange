let cardWidth = 176;

module Styles = {
  open Css;
  let root =
    style([
      paddingTop(px(32)),
      display(flexBox),
      flexDirection(column),
      margin2(~v=zero, ~h=auto),
      media(
        "(max-width: 630px)",
        [paddingTop(px(16)), marginLeft(px(16)), marginRight(px(16))],
      ),
    ]);
  let myListsLinkText = style([marginLeft(px(4))]);
  let myListsLink =
    style([
      textDecoration(none),
      hover([
        selector("& ." ++ myListsLinkText, [textDecoration(underline)]),
      ]),
    ]);
  let titleForm = style([display(flexBox), marginBottom(px(16))]);
  let titleInput =
    style([
      boxSizing(borderBox),
      borderRadius(px(4)),
      border(px(1), solid, transparent),
      backgroundColor(transparent),
      padding3(~top=px(6), ~bottom=px(6), ~h=px(7)),
      marginLeft(px(-8)),
      fontSize(px(24)),
      flexGrow(1.),
      transition(~duration=200, "all"),
      focus([
        backgroundColor(Colors.white),
        borderColor(Colors.lightGreen),
      ]),
      media(
        "(hover: hover)",
        [
          hover([
            backgroundColor(Colors.white),
            borderColor(Colors.lightGreen),
          ]),
        ],
      ),
      media("(hover: none)", [borderColor(Colors.lightGreen)]),
    ]);
  let titleInputHasChanges =
    style([
      important(backgroundColor(Colors.white)),
      important(borderColor(Colors.lightGreen)),
    ]);
  let titleSubmitButton =
    style([
      unsafe("alignSelf", "center"),
      marginLeft(px(16)),
      paddingLeft(px(16)),
      paddingRight(px(16)),
    ]);
  let titleSubmitRow =
    style([
      display(flexBox),
      justifyContent(flexStart),
      marginTop(px(6)),
    ]);
  let listTitle = style([fontSize(px(28)), marginBottom(px(16))]);
  let gridWidth = numCards => numCards * cardWidth + (numCards - 1) * 16;
  let rootGrid =
    style([
      width(px(gridWidth(4))),
      media("(max-width: 840px)", [width(px(gridWidth(3)))]),
      media("(max-width: 630px)", [width(auto)]),
    ]);
  let rootList = style([maxWidth(px(560))]);
  let topRow =
    style([
      display(flexBox),
      alignItems(center),
      justifyContent(spaceBetween),
      marginBottom(px(16)),
    ]);
  let listUserLink =
    style([
      textDecoration(none),
      media("(hover: hover)", [hover([textDecoration(underline)])]),
    ]);
  let viewToggles = style([display(flexBox)]);
  [@bs.module "./assets/grid.png"] external gridPng: string = "default";
  [@bs.module "./assets/list.png"] external listPng: string = "default";
  let viewButton =
    style([
      display(flexBox),
      borderWidth(zero),
      alignItems(center),
      backgroundColor(Colors.green),
      color(Colors.white),
      borderRadius(px(4)),
      padding2(~v=px(6), ~h=px(10)),
      marginLeft(px(8)),
      opacity(0.5),
      transition(~duration=200, "all"),
      hover([opacity(0.8)]),
    ]);
  let viewButtonSelected = style([important(opacity(1.))]);
  let gridIcon =
    style([
      display(inlineBlock),
      width(px(18)),
      height(px(18)),
      backgroundImage(url(gridPng)),
      backgroundSize(cover),
      marginRight(px(8)),
    ]);
  let listIcon =
    style([
      display(inlineBlock),
      width(px(18)),
      height(px(18)),
      backgroundImage(url(listPng)),
      backgroundSize(cover),
      marginRight(px(8)),
    ]);
  let list =
    style([
      borderRadius(px(8)),
      overflow(hidden),
      border(px(1), solid, Colors.lightGreen),
    ]);
  let grid =
    style([display(flexBox), flexWrap(wrap), marginRight(px(-16))]);
  let myListFooter =
    style([marginTop(px(32)), display(flexBox), justifyContent(flexEnd)]);
  let deleteListLink =
    style([
      color(Colors.red),
      textDecoration(none),
      media("(hover: hover)", [hover([textDecoration(underline)])]),
    ]);
};

module ListRow = {
  module Styles = {
    open Css;
    let itemName = style([flexGrow(1.), padding2(~v=zero, ~h=px(16))]);
    let row =
      style([
        backgroundColor(Colors.white),
        padding2(~v=px(8), ~h=px(8)),
        borderTop(px(1), solid, Colors.faintGray),
        firstChild([borderTopWidth(zero)]),
        display(flexBox),
        alignItems(center),
        fontSize(px(16)),
        color(Colors.charcoal),
        textDecoration(none),
        hover([
          backgroundColor(Colors.faintGreen),
          borderTopColor(Colors.lightGreen),
          // color(Colors.white),
          // selector("& ." ++ itemName, [textDecoration(underline)]),
        ]),
      ]);
    let variantName =
      style([
        color(Colors.gray),
        padding2(~v=zero, ~h=px(8)),
        textAlign(`right),
      ]);
    let imageWrapper = style([position(relative), fontSize(zero)]);
    let image = style([width(px(48)), height(px(48))]);
    let recipeIcon =
      style([
        position(absolute),
        bottom(px(-4)),
        right(px(-4)),
        width(px(24)),
        height(px(24)),
      ]);
  };

  [@react.component]
  let make = (~itemId, ~variant) => {
    let item = Item.getItem(~itemId);
    <Link
      path={ItemDetailOverlay.getItemDetailUrl(
        ~itemId,
        ~variant=Some(variant),
      )}
      className=Styles.row>
      <div className=Styles.imageWrapper>
        <img src={Item.getImageUrl(~item, ~variant)} className=Styles.image />
        {Item.isRecipe(~item)
           ? <img
               src={Constants.cdnUrl ++ "/images/DIYRecipe.png"}
               className=Styles.recipeIcon
             />
           : React.null}
      </div>
      <div className=Styles.itemName>
        {React.string(Item.getName(item))}
      </div>
      {switch (Item.getVariantName(~item, ~variant, ~hidePattern=true, ())) {
       | Some(variantName) =>
         <div className=Styles.variantName> {React.string(variantName)} </div>
       | None => React.null
       }}
    </Link>;
  };
};

type viewMode =
  | Grid
  | List;

[@react.component]
let make = (~listId) => {
  let (list, setList) = React.useState(() => None);
  let (editTitle, setEditTitle) = React.useState(() => "");
  React.useEffect0(() => {
    {
      let%Repromise response = BAPI.getItemList(~listId);
      let%Repromise.JsExn json =
        Fetch.Response.json(Belt.Result.getExn(response));
      open Json.Decode;
      let list: QuicklistStore.t = {
        id: Some(json |> field("id", string)),
        title:
          (json |> optional(field("title", string)))
          ->Belt.Option.flatMap(title => title != "" ? Some(title) : None),
        userId: json |> optional(field("userId", string)),
        itemIds: json |> field("itemIds", array(tuple2(int, int))),
      };
      let username = json |> optional(field("username", string));
      setList(_ => Some((list, username)));
      setEditTitle(_ => list.title->Belt.Option.getWithDefault(""));
      Analytics.Amplitude.logEventWithProperties(
        ~eventName="Item List Page Viewed",
        ~eventProperties={
          "listId": listId,
          "numItems": Js.Array.length(list.itemIds),
        },
      );
      Promise.resolved();
    }
    |> ignore;
    None;
  });
  let onTitleSubmit = e => {
    ReactEvent.Form.preventDefault(e);
    let editTitle = editTitle |> Js.String.slice(~from=0, ~to_=32);
    QuicklistStore.updateListTitle(~listId, ~title=Some(editTitle)) |> ignore;
    setList(list =>
      list->Belt.Option.map(((list, username)) =>
        (
          {...list, title: editTitle != "" ? Some(editTitle) : None},
          username,
        )
      )
    );
  };

  let (viewMode, setViewMode) = React.useState(() => List);
  let me = UserStore.useMe();
  let numViewToggleLoggedRef = React.useRef(0);

  <div
    className={Cn.make([
      Styles.root,
      switch (viewMode) {
      | Grid => Styles.rootGrid
      | List => Styles.rootList
      },
    ])}>
    <div>
      {switch (list) {
       | Some((list, _)) =>
         list.userId
         ->Belt.Option.flatMap(userId =>
             if (me->Belt.Option.map(me => me.id) == Some(userId)) {
               let hasChanges =
                 list.title != Some(editTitle)
                 && !(editTitle == "" && list.title === None);
               Some(
                 <form onSubmit=onTitleSubmit>
                   <div>
                     <Link path="/lists" className=Styles.myListsLink>
                       {React.string({j|←|j})}
                       <span className=Styles.myListsLinkText>
                         {React.string("My Lists")}
                       </span>
                     </Link>
                   </div>
                   <div className=Styles.titleForm>
                     <input
                       type_="text"
                       value=editTitle
                       onChange={e => {
                         let value = ReactEvent.Form.target(e)##value;
                         setEditTitle(_ => value);
                       }}
                       className={Cn.make([
                         Styles.titleInput,
                         Cn.ifTrue(Styles.titleInputHasChanges, hasChanges),
                       ])}
                       placeholder="Name your list"
                     />
                     {if (hasChanges) {
                        <Button className=Styles.titleSubmitButton>
                          {React.string("Save")}
                        </Button>;
                      } else {
                        React.null;
                      }}
                   </div>
                 </form>,
               );
             } else {
               None;
             }
           )
         ->Belt.Option.getWithDefault(
             switch (list.title) {
             | Some(title) =>
               <div className=Styles.listTitle> {React.string(title)} </div>
             | None => React.null
             },
           )
       | None => React.null
       }}
    </div>
    <div className=Styles.topRow>
      <div>
        {switch (list) {
         | Some((list, username)) =>
           switch (list.userId) {
           | Some(userId) =>
             if (me->Belt.Option.map(me => me.id) == Some(userId)) {
               <div>
                 <a
                   href="#"
                   onClick={e => {
                     ReactEvent.Mouse.preventDefault(e);
                     QuicklistStore.loadList(
                       ~listId,
                       ~title=list.title,
                       ~listItems=list.itemIds,
                     );
                     ReasonReactRouter.push("/");
                   }}
                   className=Styles.listUserLink>
                   {React.string("Edit your list")}
                 </a>
               </div>;
             } else {
               switch (username) {
               | Some(username) =>
                 <Link path={"/u/" ++ username} className=Styles.listUserLink>
                   {React.string("Visit " ++ username ++ "'s profile")}
                 </Link>
               | None => React.null
               };
             }
           | None => React.null
           }
         | None => React.null
         }}
      </div>
      <div className=Styles.viewToggles>
        <button
          onClick={_ => {
            setViewMode(_ => List);
            if (React.Ref.current(numViewToggleLoggedRef) < 5) {
              Analytics.Amplitude.logEventWithProperties(
                ~eventName="Item List Page View Toggled",
                ~eventProperties={
                  "view": "list",
                  "listId": listId,
                  "numItems":
                    switch (list) {
                    | Some((list, _)) => Js.Array.length(list.itemIds)
                    | None => 0
                    },
                },
              );
              React.Ref.setCurrent(
                numViewToggleLoggedRef,
                React.Ref.current(numViewToggleLoggedRef) + 1,
              );
            };
          }}
          className={Cn.make([
            Styles.viewButton,
            Cn.ifTrue(Styles.viewButtonSelected, viewMode == List),
          ])}>
          <span className=Styles.listIcon />
          {React.string("List")}
        </button>
        <button
          onClick={_ => {
            setViewMode(_ => Grid);
            if (React.Ref.current(numViewToggleLoggedRef) < 5) {
              Analytics.Amplitude.logEventWithProperties(
                ~eventName="Item List Page View Toggled",
                ~eventProperties={
                  "view": "grid",
                  "listId": listId,
                  "numItems":
                    switch (list) {
                    | Some((list, _)) => Js.Array.length(list.itemIds)
                    | None => 0
                    },
                },
              );
              React.Ref.setCurrent(
                numViewToggleLoggedRef,
                React.Ref.current(numViewToggleLoggedRef) + 1,
              );
            };
          }}
          className={Cn.make([
            Styles.viewButton,
            Cn.ifTrue(Styles.viewButtonSelected, viewMode == Grid),
          ])}>
          <span className=Styles.gridIcon />
          {React.string("Grid")}
        </button>
      </div>
    </div>
    {switch (list) {
     | Some((list, _)) =>
       <div>
         <div
           className={
             switch (viewMode) {
             | Grid => Styles.grid
             | List => Styles.list
             }
           }>
           {list.itemIds
            |> Js.Array.mapi(((itemId, variant), i) => {
                 switch (viewMode) {
                 | Grid =>
                   <UserItemCard
                     itemId
                     variation=variant
                     editable=false
                     showRecipe=false
                     showMetaIcons=false
                     key={string_of_int(i)}
                   />
                 | List => <ListRow itemId variant key={string_of_int(i)} />
                 }
               })
            |> React.array}
         </div>
         {switch (me) {
          | Some(me) =>
            if (list.userId == Some(me.id)) {
              <div className=Styles.myListFooter>
                <a
                  href="#"
                  onClick={e => {
                    ReactEvent.Mouse.preventDefault(e);
                    ConfirmDialog.confirm(
                      ~bodyText="Are you sure you want to delete this list?",
                      ~confirmLabel="Delete list",
                      ~cancelLabel="Not now",
                      ~onConfirm=
                        () => {
                          {
                            let%Repromise response =
                              BAPI.deleteItemList(
                                ~sessionId=
                                  Belt.Option.getExn(UserStore.sessionId^),
                                ~listId,
                              );
                            UserStore.handleServerResponse(
                              "/item-lists/delete",
                              response,
                            );
                            ReasonReactRouter.push("/lists");
                            Promise.resolved();
                          }
                          |> ignore
                        },
                      (),
                    );
                  }}
                  className=Styles.deleteListLink>
                  {React.string("Delete list")}
                </a>
              </div>;
            } else {
              React.null;
            }
          | None => React.null
          }}
       </div>
     | None => React.null
     }}
  </div>;
};