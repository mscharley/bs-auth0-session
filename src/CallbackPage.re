let component = __MODULE__ |> ReasonReact.statelessComponent;
let make = _children => {
  ...component,
  render: _self =>
    <SessionContext.Consumer>
      ...{
           ((session, _)) => {
             session->Session.doCallback;
             ReasonReact.null;
           }
         }
    </SessionContext.Consumer>,
};
