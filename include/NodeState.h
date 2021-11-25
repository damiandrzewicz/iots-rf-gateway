#pragma once
#include <Arduino.h>
#include "NodeInfo.h"

struct NodeState{

    NodeState(uint8_t id) : nodeInfo_(NodeInfo(id)){
        nodeInfo_.reset();
        reset();
    }

    void reset(){
        registered_ = false;
        lastUpdatedMS_ = -1;
        inactiveDetectedMS_ = -1;
        inactiveDelayS_ = 10 * 60;
    }

    void setActiveNow(){
        lastUpdatedMS_ = millis();
        inactiveDetectedMS_ = -1;
    }

    void setInactiveDetectedNow(){
        if(inactiveDetectedMS_ == -1){
            inactiveDetectedMS_ = millis();
        }
    }

    bool checkInactiveTimeout(uint16_t timeoutS) const {
        return millis() - inactiveDetectedMS_ > (timeoutS * 1000);
    }

    bool isActive() const {
        return millis() - lastUpdatedMS_ <= (inactiveDelayS_ * 1000);
    }

    NodeInfo nodeInfo_;
    bool registered_;
    long lastUpdatedMS_;
    long inactiveDetectedMS_;
    int inactiveDelayS_;
};