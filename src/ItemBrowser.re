[@react.component]
let make = () => {
  <div>
    {Item.all
     ->Belt.Array.map(item => {<ItemCard item key={item.id} />})
     ->React.array}
  </div>;
};