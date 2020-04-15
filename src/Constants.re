[@bs.val] [@bs.scope ("process", "env")]
external nodeEnv: option(string) = "NODE_ENV";

let apiUrl = "https://paulshen-animalcrossing.builtwithdark.com";
let gtagId = "UA-55966633-6";
let cdnUrl =
  nodeEnv === Some("production")
    ? "http://d2hw75cwo7i27d.cloudfront.net" : "";