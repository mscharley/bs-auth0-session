open Session;
module Option = Belt.Option;

type t' = (Session.t, option(Session.errorEvent));
type t = t';

module Impl =
  ContextComponent.MakePair({
    type t = option(t');
    let defaultValue = None;
  });

module Provider = {
  type state = {
    error: option(Session.errorEvent),
    initialised: bool,
    session: Session.t,
  };
  type action =
    | Initialise(Session.t)
    | LoginError(Session.errorEvent)
    | Login(Session.loginEvent)
    | Redirect(string)
    | UpdateSession(Session.t);

  let loginError = (error, {ReasonReact.send}) => LoginError(error)->send;
  let loginHandler = (event, {ReasonReact.send}) => Login(event)->send;
  let sessionRenewHandler = (event: Session.loginEvent, {ReasonReact.send}) =>
    UpdateSession(event.session)->send;

  let component = "Provider-" ++ __MODULE__ |> ReasonReact.reducerComponent;
  let make =
      (~domain, ~clientId, ~callbackUrl, ~audience=?, ~scope=?, children) => {
    ...component,
    initialState: () => {
      error: None,
      initialised: false,
      session:
        Session.make(
          ~domain,
          ~clientId,
          ~callbackUrl,
          ~audience?,
          ~scope?,
          (),
        ),
    },
    didMount: self => {
      let session =
        self.state.session
        ->onLogin(self.handle(loginHandler))
        ->onRenew(self.handle(sessionRenewHandler))
        ->onError(self.handle(loginError));
      self.send(Initialise(session));
    },
    reducer: (action, state) =>
      switch (action) {
      | Initialise(session) =>
        ReasonReact.UpdateWithSideEffects(
          {...state, initialised: true, session},
          (_self => session->doRenew),
        )
      | Redirect(url) =>
        ReasonReact.SideEffects((_self => url->ReasonReact.Router.push))
      | UpdateSession(session) =>
        ReasonReact.Update({...state, session, error: None})
      | Login({session, returnUrl}) =>
        ReasonReact.SideEffects(
          (
            self => {
              self.send(returnUrl->Option.getWithDefault("/")->Redirect);
              self.send(session->UpdateSession);
            }
          ),
        )
      | LoginError(error) =>
        Js.log2("Login error", error);
        let session = state.session->doLocalLogout;
        let newError = error.error == "login_required" ? None : Some(error);
        /* TODO: Deal with getting the correct returnUrl for relogin */
        ReasonReact.UpdateWithSideEffects(
          {...state, session, error: newError},
          (self => self.send(Redirect("/login"))),
        );
      },
    render: ({state: {initialised, session, error}}) =>
      <Impl.Provider value={initialised ? Some((session, error)) : None}>
        ...children
      </Impl.Provider>,
  };
};

module Consumer = {
  let component = "Consumer-" ++ __MODULE__ |> ReasonReact.statelessComponent;
  let make = children => {
    ...component,
    render: _self =>
      <Impl.Consumer>
        ...{
             fun
             | Some(session) => children(session)
             | None => ReasonReact.null
           }
      </Impl.Consumer>,
  };
};

module Manager = {
  let component = "Manager-" ++ __MODULE__ |> ReasonReact.statelessComponent;
  let make =
      (~domain, ~clientId, ~callbackUrl, ~audience=?, ~scope=?, children) => {
    ...component,
    render: _self =>
      <Provider domain clientId callbackUrl ?audience ?scope>
        <Consumer> ...children </Consumer>
      </Provider>,
  };
};

module ErrorConsumer = {
  let component =
    "ErrorConsumer-" ++ __MODULE__ |> ReasonReact.statelessComponent;
  let make = children => {
    ...component,
    render: _self =>
      <Impl.Consumer>
        ...{
             fun
             | Some((_, error)) => children(error)
             | None => ReasonReact.null
           }
      </Impl.Consumer>,
  };
};

module LoggedOutConsumer = {
  let component =
    "LoggedOutConsumer-" ++ __MODULE__ |> ReasonReact.statelessComponent;
  let make = children => {
    ...component,
    render: _self =>
      <Impl.Consumer>
        ...{
             session =>
               switch (
                 session->Option.map(s => s->fst),
                 session
                 ->Option.map(s => s->fst->isLoggedIn)
                 ->Option.getWithDefault(false),
               ) {
               | (_, true) => ReasonReact.null
               | (None, _) => children(true)
               | (Some(session), _) => children(session->isPending)
               }
           }
      </Impl.Consumer>,
  };
};

module LoggedInConsumer = {
  let component =
    "LoggedInConsumer-" ++ __MODULE__ |> ReasonReact.statelessComponent;
  let make = children => {
    ...component,
    render: _self =>
      <Impl.Consumer>
        ...{
             session =>
               switch (
                 session->Option.map(s => s->fst),
                 session
                 ->Option.map(s => s->fst->isLoggedIn)
                 ->Option.getWithDefault(false),
               ) {
               | (Some(session), true) => children(session)
               | _ => ReasonReact.null
               }
           }
      </Impl.Consumer>,
  };
};

module IdConsumer = {
  let component =
    "IdConsumer-" ++ __MODULE__ |> ReasonReact.statelessComponent;
  let make = children => {
    ...component,
    render: _self =>
      <Impl.Consumer>
        ...{
             session =>
               switch (session->Option.flatMap(s => s->fst->idGet)) {
               | Some(id) => children(id)
               | None => ReasonReact.null
               }
           }
      </Impl.Consumer>,
  };
};
