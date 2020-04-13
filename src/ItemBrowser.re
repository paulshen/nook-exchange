[@react.component]
let make = () => {
  <div>
    {Item.all
     ->Belt.Array.map(item => {
         <div key={item.id}> {React.string(item.name)} </div>
       })
     ->React.array}
  </div>;
};