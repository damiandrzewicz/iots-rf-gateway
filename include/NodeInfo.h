#pragma once

#include <Arduino.h>
#include "DeviceClass.h"

class NodeInfo{
public:
    enum class State{
        NotRegistered,
        IdBooked,
        Registered
    };

    NodeInfo(uint8_t id){ 
        clear();
        nodeId = id; 
    }

    void doRegister(String uuid, DeviceClass deviceClass, int inactiveDelaySec){
        this->state = State::Registered;
        this->uuid = uuid;
        this->deviceClass = deviceClass;
        this->inactiveDelaySec = inactiveDelaySec;
        setActiveNow();
    }

    void doBookId(){
        state = State::IdBooked;
    }

    void doUnregister(){
        clear();
    }

    void setActiveNow(){
        lastUpdated = millis();
        inactiveDetected = -1;
    }

    void setLastInactiveDetectedNow(){
        if(inactiveDetected == -1){
            inactiveDetected = millis();
        }
    }

    bool isPostInactiveDelayExpiredNow(long postInactiveDelay) const {
        return millis() - inactiveDetected > (postInactiveDelay * 1000);
    }

    bool isActiveNow() const {
        return millis() - lastUpdated <= (inactiveDelaySec * 1000);
    }

    uint8_t getNodeId() const {
        return nodeId;
    }

    State getState() const {
        return state;
    }

    String getUUID() const {
        return uuid;
    }

    DeviceClass getDeviceClass() const {
        return deviceClass;
    }

    uint8_t getBatteryPercent() const {
        return batteryPercent;
    }

    long getLastInactiveDetected() const {
        return inactiveDetected;
    }

    // Helper methods
    String toString() const {
        char buff[200];
        sprintf(buff, "NodeInfo(state=[%d], nodeId=[%d], uuid=[%s], deviceClass=[%d], lastUpdated=[%ld], isActiveNow=[%d], batteryPercent=[%d], inactiveDelaySec=[%d])",
            (int)state, nodeId, uuid.c_str(), (int)deviceClass, lastUpdated, isActiveNow(), batteryPercent, inactiveDelaySec
        );

        return buff;
    }

    String toStringShort(){
        String temp;
        temp = "NodeInfo(id=[" + String(nodeId) + "],uuid=[" + uuid + "],inactDelSec=[" + inactiveDelaySec + ")]";
        return temp; 
    }

private:
    void clear(){
        state = State::NotRegistered;
        lastUpdated = -1;
        batteryPercent = -1;
        inactiveDetected = -1;
        inactiveDelaySec = 10 * 60 * 60; // s
    }

private:
    // Public members
    State state;
    uint8_t nodeId;
    String uuid;
    DeviceClass deviceClass;
    long lastUpdated;
    long inactiveDetected;
    int batteryPercent;
    int inactiveDelaySec;
};