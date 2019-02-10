# bs-auth0-session

[![npm](https://img.shields.io/npm/v/bs-auth0-session.svg)](https://www.npmjs.com/package/bs-auth0-session)
[![Greenkeeper badge](https://badges.greenkeeper.io/mscharley/bs-auth0-session.svg)](https://greenkeeper.io/)

**Source:** [https://github.com/mscharley/bs-auth0-session](https://github.com/mscharley/bs-auth0-session)  
**Author:** Matthew Scharley  
**Contributors:** [See contributors on GitHub][gh-contrib]  
**Bugs/Support:** [Github Issues][gh-issues]  
**Copyright:** 2018  
**License:** [MIT license][license]  
**Status:** Active

## Synopsis

Session management for ReasonReact using Auth0 for login.

## Usage

This is a rather long example, but it should be complete and functional.

```reason
/* Util.re */
[@bs.val] [@bs.scope ("window", "location")]
external locationOrigin: string = "origin";

let urlToString = url =>
  (
    switch (url.React.Router.path) {
    | [] => "/"
    | _ =>
      url.React.Router.path->Belt.List.reduce("", (a, b) => a ++ "/" ++ b)
    }
  )
  ++ (url.search != "" ? "?" ++ url.search : "")
  ++ (url.hash != "" ? "#" ++ url.hash : "");

/* Router.re */
open BsAuth0Session;

type page =
  | Home
  | Callback
  | Logout
  | Login;

type state = {
  page: option(page),
  returnUrl: string,
};

type action =
  | Navigate(option(page));

let routerFn: ReasonReact.Router.url => option(page) =
  fun
  | {path: []} => Some(Home)
  | {path: ["oauth", "callback"]} => Some(Callback)
  | {path: ["logout"]} => Some(Logout)
  | {path: ["login"]} => Some(Login)
  | _ => None;

let component = ReasonReact.reducerComponent("Router");

let make = _children => {
  let router = (url, {ReasonReact.send}) => url->routerFn->Navigate;

  {
    ...component,
    initialState: _ => {
      let initialUrl = ReasonReact.Router.dangerouslyGetInitialUrl();
      {
        page: initialUrl->routerFn,
        returnUrl: initialUrl->Util.urlToString,
      }
    },
    didMount: self => {
      let watcherId = ReasonReact.Router.watchUrl(self.handle(router));
      self.onUnmount(_ => ReasonReact.Router.unwatchUrl(watcherId));
    },
    reducer: (action, state) =>
      switch (action) {
      | Navigate(page) => Update({...state, page})
      },
    render: ({state: {page, returnUrl}}) =>
      <SessionContext.Provider
        domain="my-domain.au.auth0.com"
        clientId="dJ9HRAcoottSxYnYourClientIdHere"
        callbackUrl={Util.locationOrigin ++ "/oauth/callback"}>
          <SessionContext.LoggedOutConsumer>
            ...{
                pending =>
                  switch (page, pending) {
                  | (Some(Logout), _) =>
                    <LogoutPage returnUrl={Util.locationOrigin ++ "/"} /> /* !! Provided !! */
                  | (Some(Callback), _) =>
                    <CallbackPage /> /* !! Provided !! */
                  | (_, true) => <LoadingPage />
                  | (_, false) =>
                    <Button
                      onClick={
                        _ev => session->Session.doLogin(~returnUrl, ())
                      }>
                      {ReasonReact.string("Login")}
                    </Button>
                  }
              }
          </SessionContext.LoggedOutConsumer>
          <SessionContext.LoggedInConsumer>
            ...{
                _session =>
                  switch (page) {
                  | Some(Logout) => <LogoutPage returnUrl={Util.locationOrigin ++ "/"} /> /* !! Provided !! */
                  | Some(Home) => <HomePage />
                  | Some(Callback) => React.null
                  | Some(Login) =>
                    React.Router.push("/");
                    React.null;
                  | None => <NotFoundPage />
                  }
              }
          </SessionContext.LoggedInConsumer>
      </SessionContext.Provider>,
  }
}
```

There's a whole suite of Consumers for all sorts of use-cases. There is also `SessionContext.Consumer` which is your escape hatch which exposes the raw session data that this library tracks.

Check [`SessionContext.rei`][sc-rei] for more details on the available consumers.

  [gh-contrib]: https://github.com/mscharley/bs-auth0-session/graphs/contributors
  [gh-issues]: https://github.com/mscharley/bs-auth0-session/issues
  [license]: https://github.com/mscharley/bs-auth0-session/blob/master/LICENSE
  [sc-rei]: https://github.com/mscharley/bs-auth0-session/blob/master/src/SessionContext.rei
