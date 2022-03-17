#pragma once

#include "Appliance.hpp"

class Gateway : public Appliance
{
public:
    enum class State
    {
        VerifyConfig,
        RadioListen,
        WebSocketListen,
        SerialListen,
        ActionHandler,
        RadioPairing,
        FactoryReset,
        RadioReset
    };

    Gateway();

protected:
    virtual void setupStateMachine() override;
    virtual void init() override;

    /** States */
    
    // VerifyConfig,
    void onVerifyConfig();

    // RadioListen,
    void onRadioListen();

    // WebSocketListen,
    void onWebSocketListen();

    // SerialListen,
    void onSerialListen();

    // ActionHandler,
    void onEnterActionHandler();
    void onActionHandler();
    void onLeaveActionHandler();

    // RadioPairing,
    void onEnterRadioPairing();
    void onRadioPairing();
    void onLeaveRadioPairing();

    // FactoryReset,
    void onFactoryReset();

    // RadioReset
    void onRadioReset();

private:
    

};
