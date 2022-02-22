#pragma once

#include "Appliance.hpp"
#include "StatusLed.hpp"

class Gateway : public Appliance
{
public:
    Gateway();

    virtual void setup() override;
    virtual void loop() override;

protected:
    // Status Led
    virtual void statusLedBlink(int16_t speed) override;
    friend void IRAM_ATTR onTimer();

private:
    static const uint8_t RadioPairPin_ = 0;
    static StatusLed statusLed;
};
