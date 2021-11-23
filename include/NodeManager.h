#pragma once

#include <Arduino.h>
#include <vector>
#include "NodeInfo.h"

class NodeManager
{
public:
    NodeManager(){
        init();
    }

    static void init(){
        for(int i = MinNodeId; i <= MaxNodeId; i++){
            nodePool_.push_back(NodeInfo(i));
        }
    }

    NodeInfo *findNextAvaliableNodeInfo(){
        auto it = std::find_if(nodePool_.begin(), nodePool_.end(), [&](const NodeInfo &ni){
            return ni.getState() == NodeInfo::State::NotRegistered;
        });
        return it != nodePool_.end() ? &(*it) : nullptr;
    }

    NodeInfo *findNodeInfoByNodeId(uint8_t id){
        auto it = std::find_if(nodePool_.begin(), nodePool_.end(), [&](const NodeInfo &ni){
            return ni.getNodeId() == id;
        });

        return it != nodePool_.end() ? &(*it) : nullptr;
    }

    NodeInfo *findNodeInfoByUUID(String uuid){
        auto it = std::find_if(nodePool_.begin(), nodePool_.end(), [&](const NodeInfo &ni){
            return ni.getUUID() == uuid;
        });

        return it != nodePool_.end() ? &(*it) : nullptr;
    }

    uint8_t updateNodeStates(){
        // Find post inactive delay expired nodes
        uint8_t inactiveNodesCount = 0;
        std::for_each(nodePool_.begin(), nodePool_.end(), [&](NodeInfo &ni){
            if(!ni.isActiveNow()){
                ni.setLastInactiveDetectedNow();
                inactiveNodesCount++;
                log_d("node inactive: %s", ni.toStringShort().c_str());

                // If nodes inactive for some time then forget about them
                if(ni.isPostInactiveDelayExpiredNow(DelayBetweenInactiveAndClear)){
                    log_d("post inactive expired: %s", ni.toStringShort().c_str());
                    ni.doUnregister();
                }
            }
        });

        return inactiveNodesCount;
    }

private:   
    static constexpr uint8_t MinNodeId = 7; 
    static constexpr uint8_t MaxNodeId = 254;
    static constexpr int MaxNodeNumber = MaxNodeId - MinNodeId + 1;
    static constexpr int DelayBetweenInactiveAndClear = 10; // seconds
    static std::vector<NodeInfo> nodePool_;
};

std::vector<NodeInfo> NodeManager::nodePool_;