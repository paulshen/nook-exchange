let cardWidth = 176;

type followee = {
  id: string,
  username: string,
  lastUpdate: option(float),
  items: array((int, int, User.item)),
};

let compareOptionTimestamps = (aTime, bTime) =>
  switch (aTime, bTime) {
  | (None, None) => 0
  | (Some(_), None) => (-1)
  | (None, Some(_)) => 1
  | (Some(aTime), Some(bTime)) => int_of_float(bTime -. aTime)
  };

let fetchFeed = () => {
  let%Repromise.JsExn response =
    Fetch.fetchWithInit(
      Constants.bapiUrl ++ "/@me/follow-feed",
      Fetch.RequestInit.make(
        ~method_=Get,
        ~headers=
          Fetch.HeadersInit.make({
            "X-Client-Version": Constants.gitCommitRef,
            "Authorization":
              "Bearer " ++ Belt.Option.getExn(UserStore.sessionId^),
          }),
        ~credentials=Include,
        ~mode=CORS,
        (),
      ),
    );
  let%Repromise.JsExn json = Fetch.Response.json(response);
  open Json.Decode;
  let feed =
    json
    |> array(json => {
         let items =
           (json |> field("items", dict(User.itemFromJson)))
           ->Js.Dict.entries
           ->Belt.Array.keepMap(((itemKey, item)) => {
               item->Belt.Option.flatMap(item => {
                 User.fromItemKey(~key=itemKey)
                 ->Belt.Option.map(((itemId, variant)) =>
                     (itemId, variant, item)
                   )
               })
             })
           |> Js.Array.sortInPlaceWith(
                ((_, _, aItem: User.item), (_, _, bItem: User.item)) =>
                compareOptionTimestamps(aItem.timeUpdated, bItem.timeUpdated)
              );
         {
           id: json |> field("id", string),
           username: json |> field("username", string),
           lastUpdate:
             Belt.Array.get(items, 0)
             ->Belt.Option.flatMap(((_, _, item)) => item.timeUpdated),
           items,
         };
       })
    |> Js.Array.sortInPlaceWith((a, b) =>
         compareOptionTimestamps(a.lastUpdate, b.lastUpdate)
       );
  Promise.resolved(feed);
};

module Followee = {
  let getRootWidth = numCards => numCards * cardWidth + (numCards - 1) * 16;
  module Styles = {
    open Css;
    let root =
      style([
        width(px(getRootWidth(5))),
        backgroundColor(hex("b0dec1c0")),
        borderRadius(px(16)),
        padding3(~top=px(32), ~bottom=px(16), ~h=px(32)),
        margin3(~top=zero, ~h=auto, ~bottom=px(48)),
        width(auto),
        media("(min-width: 660px)", [width(px(getRootWidth(3)))]),
        media("(min-width: 860px)", [width(px(getRootWidth(4)))]),
        media("(min-width: 1040px)", [width(px(getRootWidth(5)))]),
        media(
          "(max-width: 640px)",
          [width(auto), padding(px(16)), borderRadius(zero)],
        ),
      ]);
    let usernameRow =
      style([
        display(flexBox),
        alignItems(center),
        justifyContent(spaceBetween),
        marginBottom(px(24)),
      ]);
    let usernameRowUnfollowed = style([important(marginBottom(px(16)))]);
    let usernameLink =
      style([
        fontSize(px(24)),
        color(Colors.charcoal),
        textDecoration(none),
        media("(hover: hover)", [hover([textDecoration(underline)])]),
      ]);
    let unfollowLink =
      style([
        textDecoration(none),
        media("(hover: hover)", [hover([textDecoration(underline)])]),
      ]);
    let items =
      style([display(flexBox), flexWrap(wrap), marginRight(px(-16))]);
    let listLink =
      style([
        position(absolute),
        top(px(8)),
        left(px(8)),
        display(flexBox),
        alignItems(center),
        color(Colors.gray),
        textDecoration(none),
        media("(hover: hover)", [hover([textDecoration(underline)])]),
      ]);
    let itemMatch =
      style([boxShadow(Shadow.box(~spread=px(2), Colors.green))]);
  };

  let getNumCards = viewportWidth =>
    if (viewportWidth >= 1040) {
      10;
    } else if (viewportWidth >= 860) {
      8;
    } else {
      6;
    };

  module ItemCard = {
    [@react.component]
    let make = (~itemId, ~variant, ~userItem: User.item, ~username) => {
      let viewerItem = UserStore.useItem(~itemId, ~variation=variant);
      let isMatch =
        switch (viewerItem) {
        | Some(viewerItem) =>
          switch (userItem.status, viewerItem.status) {
          | (ForTrade, Wishlist)
          | (CanCraft, Wishlist)
          | (CatalogOnly, Wishlist)
          | (Wishlist, ForTrade)
          | (Wishlist, CanCraft)
          | (Wishlist, CatalogOnly) => true
          | _ => false
          }
        | None => false
        };
      <UserItemCard
        itemId
        variation=variant
        userItem
        list={
          switch (userItem.status) {
          | CatalogOnly
          | ForTrade => ForTrade
          | CanCraft => CanCraft
          | Wishlist => Wishlist
          }
        }
        editable=false
        showRecipe=false
        customTopLeft={
          <Link
            path={
              "/u/"
              ++ username
              ++ "/"
              ++ ViewingList.viewingListToUrl(
                   switch (userItem.status) {
                   | Wishlist => Wishlist
                   | ForTrade => ForTrade
                   | CanCraft => CanCraft
                   | CatalogOnly => Catalog
                   },
                 )
            }
            className=Styles.listLink>
            {React.string(User.itemStatusToString(userItem.status))}
          </Link>
        }
        className={Cn.ifTrue(Styles.itemMatch, isMatch)}
      />;
    };
  };

  [@react.component]
  let make = (~followee) => {
    let viewportWidth = Utils.useViewportWidth();
    let numCards = getNumCards(viewportWidth);
    let (unfollowed, setUnfollowed) = React.useState(() => false);

    <div className=Styles.root>
      <div
        className={Cn.make([
          Styles.usernameRow,
          Cn.ifTrue(Styles.usernameRowUnfollowed, unfollowed),
        ])}>
        <Link path={"/u/" ++ followee.username} className=Styles.usernameLink>
          {React.string(followee.username)}
        </Link>
        {!unfollowed
           ? <a
               href="#"
               onClick={e => {
                 ReactEvent.Mouse.preventDefault(e);
                 {
                   let%Repromise success =
                     BAPI.unfollowUser(
                       ~userId=followee.id,
                       ~sessionId=Belt.Option.getExn(UserStore.sessionId^),
                     );
                   if (success) {
                     setUnfollowed(_ => true);
                   };
                   Promise.resolved();
                 }
                 |> ignore;
               }}
               className=Styles.unfollowLink>
               {React.string("Unfollow")}
             </a>
           : <span> {React.string("Unfollowed")} </span>}
      </div>
      {!unfollowed
         ? <div className=Styles.items>
             {followee.items
              |> Js.Array.slice(~start=0, ~end_=numCards - 1)
              |> Js.Array.map(((itemId, variant, userItem)) =>
                   <ItemCard
                     itemId
                     variant
                     userItem
                     username={followee.username}
                     key={string_of_int(itemId) ++ string_of_int(variant)}
                   />
                 )
              |> React.array}
             <Link
               path={"/u/" ++ followee.username}
               className={Cn.make([
                 UserItemCard.Styles.card,
                 UserProfileBrowser.Styles.cardSeeAll,
               ])}>
               {React.string("Go to profile")}
               <span
                 className={Cn.make([
                   UserProfileBrowser.Styles.sectionTitleLinkIcon,
                   UserProfileBrowser.Styles.cardSeeAllLinkIcon,
                 ])}
               />
             </Link>
           </div>
         : React.null}
    </div>;
  };
};

module WithViewer = {
  module Styles = {
    open Css;
    let root = style([paddingTop(px(32))]);
  };

  [@react.component]
  let make = (~me: User.t) => {
    let (feed: option(array(followee)), setFeed) =
      React.useState(() => None);

    React.useEffect0(() => {
      {
        let%Repromise feed = fetchFeed();
        setFeed(_ => Some(feed));
        Promise.resolved();
      }
      |> ignore;
      None;
    });

    <div className=Styles.root>
      {switch (feed) {
       | Some(feed) =>
         <div>
           {feed
            |> Js.Array.map(followee =>
                 <Followee followee key={followee.id} />
               )
            |> React.array}
         </div>
       | None => React.null
       }}
    </div>;
  };
};

[@react.component]
let make = () => {
  let me = UserStore.useMe();

  switch (me) {
  | Some(me) => <WithViewer me />
  | None => React.null
  };
};