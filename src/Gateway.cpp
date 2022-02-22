#include "Gateway.hpp"
#include <ArduinoLog.h>
#include <EEPROM.h>
#include "RadioConfigData.hpp"

StatusLed Gateway::statusLed = {2};
hw_timer_t * timer = NULL;

void IRAM_ATTR onTimer()
{
    Gateway::statusLed.execISR();
}

Gateway::Gateway()
    : Appliance(0)
{

}

void Gateway::setup()
{
    Log.verboseln(F("Appliance::setup"));

    EEPROM.begin(512);

    pinMode(RadioPairPin_, INPUT_PULLUP);

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 10000, true);
    timerAlarmDisable(timer);

    statusLedBlink(-1);

    readConfiguration();
}

void Gateway::loop()
{
    if(isRadioPairBtnTriggered())
    {
        radioPairRoutine();
    }
}

    // Status Led
void Gateway::statusLedBlink(int16_t speed)
{
    if(speed)
    {
        statusLed.startBlink(speed);
        timerAlarmEnable(timer);
    }
    else if(!speed)
    {
        statusLed.startBlink(0);
    }
    else if(speed == -1)
    {
        statusLed.stopBlink();
        timerAlarmDisable(timer);
    }
}