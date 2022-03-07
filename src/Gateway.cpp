#include "Gateway.hpp"
#include <ArduinoLog.h>
#include <EEPROM.h>
#include "ezButton.h"

const char* const stateName[] PROGMEM = { 
    "VerifyConfig",
    "RadioListen",
    "WebSocketListen",
    "SerialListen",
    "ActionHandler",
    "RadioPairing",
    "FactoryReset",
    "RadioReset"
};

Gateway::Gateway()
    : Appliance(RF69_SPI_CS, 4, 0, 2)
{

}

void Gateway::setupStateMachine()
{
    Log.verboseln(F("Gateway::setupStateMachine"));

    /** States */

        stateMachine_.AddState(stateName[static_cast<int>(State::VerifyConfig)], 0,
        [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onVerifyConfig(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );

    stateMachine_.AddState(stateName[static_cast<int>(State::RadioListen)], 0,
        [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onRadioListen(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );
    
    stateMachine_.AddState(stateName[static_cast<int>(State::WebSocketListen)], 0,
        [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onWebSocketListen(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );

    stateMachine_.AddState(stateName[static_cast<int>(State::SerialListen)], 0,
        [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onSerialListen(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );

    stateMachine_.AddState(stateName[static_cast<int>(State::ActionHandler)], 0,
        [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onActionHandler(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );

    stateMachine_.AddState(stateName[static_cast<int>(State::RadioPairing)], 5000,
        [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onRadioPairing(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );

    stateMachine_.AddState(stateName[static_cast<int>(State::FactoryReset)], 0,
        [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onFactoryReset(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );

    stateMachine_.AddState(stateName[static_cast<int>(State::RadioReset)], 0,
        [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onRadioReset(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );

    /** Transitions */

    stateMachine_.AddTransition(static_cast<int>(State::VerifyConfig), static_cast<int>(State::RadioListen), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->result == FSM_State::Result::Success;
        },
        this    
    );
}

void Gateway::init()
{
    Log.verboseln(F("Gateway::init"));

    EEPROM.begin(512);

    radioSetup(1, radioConfigPairing_);
}       
/** States */

// VerifyConfig,
void Gateway::onVerifyConfig()
{
    stateMachine_.CurrentState()->result = FSM_State::Result::Success;
}

// RadioListen,
void Gateway::onRadioListen()
{
    if(radio_.receiveDone()){
        radioPayloadToBuffer();
        
        if(radio_.ACK_REQUESTED)
        {
            messageBuffer_.clear();
            messageBuffer_.appendText("response");
            sendACKRepsonse(&messageBuffer_);
        }
    }
}

// WebSocketListen,
void Gateway::onWebSocketListen()
{
    
}

// SerialListen,
void Gateway::onSerialListen()
{
    
}

// ActionHandler,
void Gateway::onActionHandler()
{
    
}

// RadioPairing,
void Gateway::onRadioPairing()
{
    
}

// FactoryReset,
void Gateway::onFactoryReset()
{
    
}

// RadioReset
void Gateway::onRadioReset()
{
    
}