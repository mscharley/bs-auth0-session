type t = (Session.t, option(Session.errorEvent));

module Provider: {
  type state;
  type action;
  let make:
    (
      ~domain: string,
      ~clientId: string,
      ~callbackUrl: string,
      ~audience: string=?,
      ~scope: string=?,
      ReasonReact.reactElement
    ) =>
    ReasonReact.component(state, ReasonReact.noRetainedProps, action);
};

module Consumer: {
  let make:
    (t => ReasonReact.reactElement) =>
    ReasonReact.component(
      ReasonReact.stateless,
      ReasonReact.noRetainedProps,
      ReasonReact.actionless,
    );
};

module Manager: {
  let make:
    (
      ~domain: string,
      ~clientId: string,
      ~callbackUrl: string,
      ~audience: string=?,
      ~scope: string=?,
      t => ReasonReact.reactElement
    ) =>
    ReasonReact.component(
      ReasonReact.stateless,
      ReasonReact.noRetainedProps,
      ReasonReact.actionless,
    );
};

module ErrorConsumer: {
  let make:
    (option(Session.errorEvent) => ReasonReact.reactElement) =>
    ReasonReact.component(
      ReasonReact.stateless,
      ReasonReact.noRetainedProps,
      ReasonReact.actionless,
    );
};

module LoggedOutConsumer: {
  let make:
    (bool => ReasonReact.reactElement) =>
    ReasonReact.component(
      ReasonReact.stateless,
      ReasonReact.noRetainedProps,
      ReasonReact.actionless,
    );
};

module LoggedInConsumer: {
  let make:
    (Session.t => ReasonReact.reactElement) =>
    ReasonReact.component(
      ReasonReact.stateless,
      ReasonReact.noRetainedProps,
      ReasonReact.actionless,
    );
};

module IdConsumer: {
  let make:
    (Session.id => ReasonReact.reactElement) =>
    ReasonReact.component(
      ReasonReact.stateless,
      ReasonReact.noRetainedProps,
      ReasonReact.actionless,
    );
};
