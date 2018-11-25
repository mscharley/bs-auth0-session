open Belt.Option;
open Js.Nullable;
open QueryStringify;
open BsAuth0Js;

type accessToken = string;

[@bs.val] [@bs.scope ("window", "location")]
external locationSearch: string = "search";
[@bs.val] [@bs.scope ("window", "location")]
external location: string = "href";

type id = {
  email: option(string),
  name: option(string),
  picture: option(string),
  userId: string,
}
and loginEvent = {
  accessToken,
  id,
  session: t,
  returnUrl: option(string),
}
and errorEvent = {
  error: string,
  errorDescription: string,
}
and options = {
  auth: WebAuth.t,
  callbackUrl: string,
  onLogin: loginHandler,
  onRenew: loginHandler,
  onError: errorHandler,
}
and loginHandler = loginEvent => unit
and errorHandler = errorEvent => unit
and t =
  | Pending(options)
  | LoggedOut(options)
  | Session(options, string, id);

let make =
    (
      ~domain,
      ~clientId,
      ~callbackUrl,
      ~audience=?,
      ~scope="openid profile email",
      ~onLogin=_ => (),
      ~onRenew=_ => (),
      ~onError=_ => (),
      (),
    ) => {
  let auth =
    WebAuth.createWebAuth(
      WebAuth.createOptions(
        ~domain,
        ~clientID=clientId,
        ~responseType="token id_token",
        ~scope,
        ~audience?,
        (),
      ),
    );
  Pending({auth, callbackUrl, onLogin, onRenew, onError});
};

let idFromWebauthPayload = payload => {
  userId: payload->WebAuth.subGet,
  email: payload->WebAuth.emailGet,
  name: payload->WebAuth.nameGet,
  picture: payload->WebAuth.pictureGet,
};

let isLoggedIn = s =>
  switch (s) {
  | Session(_, _, _) => true
  | _ => false
  };

let isPending = s =>
  switch (s) {
  | Pending(_) => true
  | _ => false
  };

let updateState = (s, update) =>
  switch (s) {
  | Pending(state) => Pending(state->update)
  | LoggedOut(state) => LoggedOut(state->update)
  | Session(state, accessToken, id) =>
    Session(state->update, accessToken, id)
  };

let onLogin = (s, handler) =>
  s->updateState(state => {...state, onLogin: handler});
let onRenew = (s, handler) =>
  s->updateState(state => {...state, onRenew: handler});
let onError = (s, handler) =>
  s->updateState(state => {...state, onError: handler});

let rec handleAuthResult = (state, success, err, result) =>
  switch (err->toOption, result->toOption) {
  | (None, None) =>
    state.onError({
      error: "missing_token",
      errorDescription: "There was an error with the authentication server.",
    })
  | (Some(e), _) =>
    state.onError({
      error: e->WebAuth.errorGet,
      errorDescription: e->WebAuth.errorDescriptionGet,
    })
  | (_, Some(result)) =>
    switch (result->WebAuth.accessTokenGet, result->WebAuth.idTokenPayloadGet) {
    | (None, _)
    | (_, None) =>
      state.onError({
        error: "missing_token",
        errorDescription: "There was an error with the authentication server.",
      })
    | (Some(accessToken), Some(idToken)) =>
      let id = idToken->idFromWebauthPayload;
      let search = parseQueryString(locationSearch);

      let event = {
        accessToken,
        id,
        session: Session(state, accessToken, id),
        returnUrl: search->Js.Dict.get("return"),
      };

      let expiryMs = (result->WebAuth.expiresInGet->int_of_float - 120) * 1000;
      Js.Global.setTimeout(() => event.session->doRenew, expiryMs) |> ignore;

      success(event);
    }
  }
and doRenew = s =>
  switch (s) {
  | Pending(state)
  | LoggedOut(state) =>
    if (!(location |> Js.String.startsWith(state.callbackUrl))) {
      state.auth
      ->WebAuth.renewAuthWithOptions(
          WebAuth.renewOptions(
            ~redirectUri=state.callbackUrl ++ "?renew=true",
            (),
          ),
          state->handleAuthResult(state.onLogin),
        );
    }
  | Session(state, _, _) =>
    state.auth
    ->WebAuth.renewAuthWithOptions(
        WebAuth.renewOptions(
          ~redirectUri=state.callbackUrl ++ "?renew=true",
          (),
        ),
        state->handleAuthResult(state.onRenew),
      )
  };

let doLogin = (s, ~returnUrl=?, ()) =>
  switch (s) {
  | Pending(state)
  | LoggedOut(state) =>
    state.auth
    ->WebAuth.authorizeWithOptions(
        WebAuth.authorizeOptions(
          ~redirectUri=
            state.callbackUrl
            ++ returnUrl->map(s => "?return=" ++ s)->getWithDefault(""),
          (),
        ),
      )
  | Session(_, _, _) => ()
  };

let doLogout = (session, ~returnUrl=?, ()) =>
  switch (session) {
  | Pending(state)
  | LoggedOut(state)
  | Session(state, _, _) =>
    state.auth
    ->WebAuth.logoutWithOptions(
        WebAuth.logoutOptions(~returnTo=?returnUrl, ()),
      )
  };

let doLocalLogout =
  fun
  | Session(state, _, _)
  | Pending(state)
  | LoggedOut(state) => LoggedOut(state);

let doCallback = session => {
  let search = parseQueryString(locationSearch);

  switch (search->Js.Dict.get("renew"), session) {
  | (Some("true"), _) => ()
  | (_, Session(state, _, _)) =>
    state.auth->WebAuth.parseHash(handleAuthResult(state, state.onRenew))
  | (_, Pending(state))
  | (_, LoggedOut(state)) =>
    state.auth->WebAuth.parseHash(handleAuthResult(state, state.onLogin))
  };
};

let accessTokenGet =
  fun
  | Session(_, accessToken, _) => Some(accessToken)
  | _ => None;

let unsafeAccessTokenRaw = x => x;

let idGet =
  fun
  | Session(_, _, id) => Some(id)
  | _ => None;
