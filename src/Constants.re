[@bs.val] [@bs.scope ("process", "env")]
external nodeEnv: option(string) = "NODE_ENV";

let apiUrl = "https://paulshen-animalcrossing.builtwithdark.com";
let cdnUrl = nodeEnv === Some("production") ? "https://s3.nook.exchange" : "";
let imageUrl =
  nodeEnv === Some("production")
    ? "https://s.nook.exchange/images" : "/images";

let gtagId = "UA-55966633-6";
let amplitudeApiKey = "d91231e0b8a96baf38ba67eb36d25a48";
let sentryId = "https://b3e870897abf4c5caef3e12320202dee@o378523.ingest.sentry.io/5202020";