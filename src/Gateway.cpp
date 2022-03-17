#include "Gateway.hpp"
#include <ArduinoLog.h>
#include <EEPROM.h>
#include "ezButton.h"

#include "messages/radio/concrete/RadioPairRequest.hpp"
#include "messages/radio/concrete/RadioPairResponse.hpp"

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
        nullptr,// [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onVerifyConfig(); }, 
        nullptr,// [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );

    stateMachine_.AddState(stateName[static_cast<int>(State::RadioListen)], 0,
        nullptr,// [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onRadioListen(); }, 
        nullptr,// [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );
    
    stateMachine_.AddState(stateName[static_cast<int>(State::WebSocketListen)], 0,
        nullptr,// [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onWebSocketListen(); }, 
        nullptr,// [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );

    stateMachine_.AddState(stateName[static_cast<int>(State::SerialListen)], 0,
        nullptr,// [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActiveStateName(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onSerialListen(); }, 
        nullptr,// [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActiveStateName(); }, 
        this
    );

    stateMachine_.AddState(stateName[static_cast<int>(State::ActionHandler)], 0,
        [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterActionHandler(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onActionHandler(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveActionHandler(); }, 
        this
    );

    stateMachine_.AddState(stateName[static_cast<int>(State::RadioPairing)], 5000,
        [](void *ctx){ static_cast<Gateway*>(ctx)->onEnterRadioPairing(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onRadioPairing(); }, 
        [](void *ctx){ static_cast<Gateway*>(ctx)->onLeaveRadioPairing(); }, 
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

    stateMachine_.AddTransition(static_cast<int>(State::VerifyConfig), static_cast<int>(State::SerialListen), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->result == FSM_State::Result::Failed;
        },
        this    
    );

    stateMachine_.AddTransition(static_cast<int>(State::RadioListen), static_cast<int>(State::WebSocketListen), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->result != FSM_State::Result::InWork;
        },
        this    
    );

    stateMachine_.AddTransition(static_cast<int>(State::WebSocketListen), static_cast<int>(State::SerialListen), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->result != FSM_State::Result::InWork;
        },
        this    
    );

    stateMachine_.AddTransition(static_cast<int>(State::SerialListen), static_cast<int>(State::VerifyConfig), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->result != FSM_State::Result::InWork &&
                static_cast<Gateway*>(ctx)->radioConfig_.isEmpty();
        },
        this    
    );

    stateMachine_.AddTransition(static_cast<int>(State::SerialListen), static_cast<int>(State::ActionHandler), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->result != FSM_State::Result::InWork &&
                !static_cast<Gateway*>(ctx)->radioConfig_.isEmpty();
        },
        this    
    );

    stateMachine_.AddTransition(static_cast<int>(State::ActionHandler), static_cast<int>(State::RadioPairing), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->timeout &&
                static_cast<Gateway*>(ctx)->stateBtnMode_ == StateBtnMode::Pairing;
        },
        this    
    );

    stateMachine_.AddTransition(static_cast<int>(State::ActionHandler), static_cast<int>(State::FactoryReset), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->timeout &&
                static_cast<Gateway*>(ctx)->stateBtnMode_ == StateBtnMode::FactoryReset;
        },
        this    
    );

    stateMachine_.AddTransition(static_cast<int>(State::ActionHandler), static_cast<int>(State::RadioReset), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->timeout &&
                static_cast<Gateway*>(ctx)->stateBtnMode_ == StateBtnMode::RadioReset;
        },
        this    
    );

    stateMachine_.AddTransition(static_cast<int>(State::ActionHandler), static_cast<int>(State::VerifyConfig), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->timeout &&
                static_cast<Gateway*>(ctx)->stateBtn_.getState() == HIGH;
        },
        this    
    );


    stateMachine_.AddTransition(static_cast<int>(State::RadioPairing), static_cast<int>(State::VerifyConfig), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->result != FSM_State::Result::InWork;
        },
        this    
    );

    stateMachine_.AddTransition(static_cast<int>(State::RadioReset), static_cast<int>(State::VerifyConfig), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->result != FSM_State::Result::InWork;
        },
        this    
    );

    stateMachine_.AddTransition(static_cast<int>(State::FactoryReset), static_cast<int>(State::VerifyConfig), 
        [](void *ctx){
            return 
                static_cast<Gateway*>(ctx)->stateMachine_.CurrentState()->result != FSM_State::Result::InWork;
        },
        this    
    );
}

void Gateway::init()
{
    Log.verboseln(F("Gateway::init"));

    EEPROM.begin(512);
    
    radioSetup(1, radioConfigPairing_);

    radioConfig_.gatewayId = 1;
    radioConfig_.networkId = 111;
    radioConfig_.customFrequency = 869000000U;
    strcpy(radioConfig_.encryptKey, "sampleEntryptKey");
    radioConfig_.rssi = -80;
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

    stateMachine_.CurrentState()->result = FSM_State::Result::Success;
}

// WebSocketListen,
void Gateway::onWebSocketListen()
{
    stateMachine_.CurrentState()->result = FSM_State::Result::Success;
}

// SerialListen,
void Gateway::onSerialListen()
{
    stateMachine_.CurrentState()->result = FSM_State::Result::Success;
}

// ActionHandler,
void Gateway::onEnterActionHandler()
{
    onEnterActiveStateName();
    stateMachine_.CurrentState()->maxTime = stateBtn_.getDebounceTime() + 5;
    stateLed_.blink(50, 950, 1000);
}

void Gateway::onActionHandler()
{
    
}

void Gateway::onLeaveActionHandler()
{
    onLeaveActiveStateName();
    stateLed_.cancel();
}

// RadioPairing,
void Gateway::onEnterRadioPairing()
{
    onEnterActiveStateName();
    radioSetup(1, radioConfigPairing_);
}

void Gateway::onRadioPairing()
{
    auto currentState = stateMachine_.CurrentState();
    static auto success = false;

    if(currentState->step == 1)
    {
        stateLed_.blink(50, 250);
        currentState->step++;   // Next step
        success = false;
    }
    else if(currentState->step == 2)
    {
        if(!currentState->timeout)
        {
            // Keep pairing
            if(radio_.receiveDone()){
                radioPayloadToBuffer();

                RadioPairRequest request(messageBuffer_);
                if(request.parse())
                {
                    if(radio_.ACK_REQUESTED)
                    {
                        messageBuffer_.clear();
                        RadioPairResponse response(messageBuffer_);
                        response.model().gatewayId = radioConfig_.gatewayId;
                        response.model().networkId = radioConfig_.networkId;
                        response.model().customFrequency = radioConfig_.customFrequency;
                        response.model().encryptKey = radioConfig_.encryptKey;
                        response.model().rssi = radioConfig_.rssi;

                        if(!response.build())
                        {
                            Log.warningln(F("RadioPairResponse build failed!"));
                        }
                        else
                        {
                            sendACKRepsonse(&messageBuffer_);
                            currentState->step++;   // Next step
                            success = true;
                        }
                    }
                }
                else
                {
                    Log.warningln(F("RadioPairRequest parse failed!"));
                }
            }   
        }
        else
        {
            // Timeout
            currentState->step++;   // Next step
        }
    }
    else if(currentState->step == 3)
    {
        stateLed_.cancel();
        success ? stateLed_.blinkInPeriod(1000, 0, 3000) : stateLed_.blinkNumberOfTimes(500, 500, 5);
        currentState->step++;
    }
    else if(currentState->step == 4)
    {
        // Led finished work
        if(stateLed_.getState() == LED_IDLE)
        {
            Log.verboseln(F("Led finished work"));
            currentState->result = success ? FSM_State::Result::Success : FSM_State::Result::Failed;
        }
    }
}

void Gateway::onLeaveRadioPairing()
{
    onLeaveActiveStateName();

    // Sommit EEPROM if success
    if(stateMachine_.CurrentState()->result == FSM_State::Result::Success)
    {
        Log.verboseln(F("pairing success!"));
    }
    else if(stateMachine_.CurrentState()->result == FSM_State::Result::Failed)
    {
        Log.warningln(F("pairing failed!"));
    }
}

// FactoryReset,
void Gateway::onFactoryReset()
{
    stateMachine_.CurrentState()->result = FSM_State::Result::Success;
}

// RadioReset
void Gateway::onRadioReset()
{
    stateMachine_.CurrentState()->result = FSM_State::Result::Success;
}