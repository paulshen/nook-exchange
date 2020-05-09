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
      ]);
    let usernameRow = style([marginBottom(px(24))]);
    let usernameLink =
      style([
        fontSize(px(24)),
        marginBottom(px(24)),
        color(Colors.charcoal),
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

  let getNumCards = viewportWidth => {
    10;
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

    <div className=Styles.root>
      <div className=Styles.usernameRow>
        <Link path={"/u/" ++ followee.username} className=Styles.usernameLink>
          {React.string(followee.username)}
        </Link>
      </div>
      <div className=Styles.items>
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
          {React.string("See more")}
          <span
            className={Cn.make([
              UserProfileBrowser.Styles.sectionTitleLinkIcon,
              UserProfileBrowser.Styles.cardSeeAllLinkIcon,
            ])}
          />
        </Link>
      </div>
    </div>;
  };
};

module WithViewer = {
  module Styles = {
    open Css;
    let root = style([]);
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