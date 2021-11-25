#pragma once

#include <Arduino.h>
#include "DeviceClass.h"

struct NodeInfo{
    NodeInfo(uint8_t id) : nodeId_(id){
        reset();
    }

    void reset(){
        uuid_ = "";
        batteryPercent_ = -1;
    }

    String shortInfo() const {
        String temp;
        temp = "NodeData(nodeId=[" + String(nodeId_) + "],uuid=[" + uuid_ + "])";
        return temp; 
    }

    const uint8_t nodeId_;
    String uuid_;
    DeviceClass deviceClass_;
    int batteryPercent_;
};