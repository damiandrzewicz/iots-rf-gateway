# IoTs RF Core Library

@startuml

  [*] --> VerifyConfig
  VerifyConfig --> RadioListen : success
  VerifyConfig --> SerialListen : failed
  RadioListen --> WebSocketListen : success
  WebSocketListen --> SerialListen : success
  SerialListen --> ActionHandler : configured
  SerialListen --> VerifyConfig : !configured

  ActionHandler --> RadioPairing : success &&\npairing
  ActionHandler --> FactoryReset : success &&\nfactoryReset
  ActionHandler --> RadioReset : success &&\nradioReset

  ActionHandler --> RadioListen : success &&\nno flag

  state join_state <<join>>

  RadioPairing --> join_state
  FactoryReset --> join_state
  RadioReset --> join_state

  join_state --> VerifyConfig : success/failed

@enduml
