let component = __MODULE__ |> ReasonReact.statelessComponent;
let make = (~returnUrl, _children) => {
  ...component,
  render: _self =>
    <SessionContext.Consumer>
      ...{
           ((session, _)) => {
             session->Session.doLogout(~returnUrl, ());
             ReasonReact.null;
           }
         }
    </SessionContext.Consumer>,
};
