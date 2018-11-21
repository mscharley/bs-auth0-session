type pair;

[@bs.get] external provider: pair => ReasonReact.reactClass = "Provider";
[@bs.get] external consumer: pair => ReasonReact.reactClass = "Consumer";

[@bs.module "react"] external createContext: 'a => pair = "";

module MakePair = (Config: {
                     type t;
                     let defaultValue: t;
                   }) => {
  type t = Config.t;
  let pair = createContext(Config.defaultValue);

  module Provider = {
    let make = (~value: Config.t, children) =>
      ReasonReact.wrapJsForReason(
        ~reactClass=provider(pair),
        ~props={"value": value},
        children,
      );
  };

  module Consumer = {
    let make = (children: Config.t => ReasonReact.reactElement) =>
      ReasonReact.wrapJsForReason(
        ~reactClass=consumer(pair),
        ~props=Js.Obj.empty(),
        children,
      );
  };
};
