type t;
type accessToken;

type id = {
  email: option(string),
  name: option(string),
  picture: option(string),
  userId: string,
};

type loginEvent = {
  accessToken,
  id,
  session: t,
  returnUrl: option(string),
};

type errorEvent = {
  error: string,
  errorDescription: string,
};

type loginHandler = loginEvent => unit;
type errorHandler = errorEvent => unit;

let make:
  (
    ~domain: string,
    ~clientId: string,
    ~callbackUrl: string,
    ~audience: string=?,
    ~scope: string=?,
    ~onLogin: loginHandler=?,
    ~onRenew: loginHandler=?,
    ~onError: errorHandler=?,
    unit
  ) =>
  t;

let isLoggedIn: t => bool;
let isPending: t => bool;

let onLogin: (t, loginHandler) => t;
let onRenew: (t, loginHandler) => t;
let onError: (t, errorHandler) => t;

let doLogin: (t, ~returnUrl: string=?, unit) => unit;
let doRenew: t => unit;
let doLogout: (t, ~returnUrl: string=?, unit) => unit;
let doLocalLogout: t => t;
let doCallback: t => unit;

let accessTokenGet: t => option(accessToken);
let unsafeAccessTokenRaw: accessToken => string;
let idGet: t => option(id);
