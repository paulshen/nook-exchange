[@bs.val] [@bs.scope ("process", "env")]
external nodeEnv: option(string) = "NODE_ENV";
[@bs.val] [@bs.scope ("process", "env")]
external gitCommitRef: option(string) = "COMMIT_REF";
let gitCommitRef =
  Belt.Option.getWithDefault(gitCommitRef, "")
  |> Js.String.slice(~from=0, ~to_=8);

let apiUrl = "https://paulshen-animalcrossing.builtwithdark.com";
let bapiUrl = "https://a.nook.exchange";
let cdnUrl =
  nodeEnv === Some("production")
    ? "https://b.nook.exchange/file/nook-exchange" : "";

let gtagId = "UA-55966633-6";
let amplitudeApiKey = "d91231e0b8a96baf38ba67eb36d25a48";
let sentryId = "https://b3e870897abf4c5caef3e12320202dee@o378523.ingest.sentry.io/5202020";

exception Uhoh;

let headerHeight = 52;