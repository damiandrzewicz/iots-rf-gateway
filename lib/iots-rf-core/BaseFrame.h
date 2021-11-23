#pragma once
#include <Arduino.h>
#include "Action.h"
#include "Buffer.h"

struct BaseFrameData{
    ActionType actionType;
    ActionDirection actionDirection;
};

class BaseFrame
{
public:
    bool parse(Buffer &buffer, BaseFrameData &baseDataFrame){
        int actionType;
        int actionDirection;
        auto res = sscanf(buffer.data(), "%d|%d", &actionType, &actionDirection);
        if( 2 != res){
            log_w("wrong args: res=%d!", res);
            return false;
        }

        baseDataFrame.actionType = static_cast<ActionType>(actionType);
        baseDataFrame.actionDirection = static_cast<ActionDirection>(actionDirection);
        return true;
    }

    bool build(BaseFrameData &baseDataFrame, Buffer &buff){
        buff.clear();
        buff.appendInt(static_cast<uint8_t>(baseDataFrame.actionType));
        buff.appendInt(static_cast<uint8_t>(baseDataFrame.actionDirection));
        return true;
    }
};