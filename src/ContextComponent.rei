module MakePair:
  (Config: {
     type t;
     let defaultValue: t;
   }) =>
   {
    type t = Config.t;
    module Provider: {
      let make:
        (~value: Config.t, 'a) =>
        ReasonReact.component(
          ReasonReact.stateless,
          ReasonReact.noRetainedProps,
          ReasonReact.actionless,
        );
    };
    module Consumer: {
      let make:
        (Config.t => ReasonReact.reactElement) =>
        ReasonReact.component(
          ReasonReact.stateless,
          ReasonReact.noRetainedProps,
          ReasonReact.actionless,
        );
    };
  };
