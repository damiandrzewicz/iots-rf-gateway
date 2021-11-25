#pragma once

#include <Arduino.h>
#include <vector>
#include "NodeState.h"

class NodeManager
{
public:

    NodeManager(){
        init();
    }

    static void init(){
        for(int i = MinNodeId; i <= MaxNodeId; i++){
            nodePool_.push_back(NodeState(i));
        }
    }

    NodeInfo *findNextFreeId() const {
        auto it = std::find_if(nodePool_.begin(), nodePool_.end(), [&](const NodeState &ndc){
            return !ndc.registered_;
        });
        return it != nodePool_.end() ? &(*it).nodeInfo_ : nullptr;
    }

    NodeState *getNodeState(uint8_t nodeId) const {
        if(!verifyId(nodeId)){ return nullptr; }
        auto it = std::find_if(nodePool_.begin(), nodePool_.end(), [&](const NodeState &ndc){
            return ndc.nodeInfo_.nodeId_ == nodeId;
        });
        return it != nodePool_.end() ? &(*it) : nullptr;
    }

    NodeInfo *getNodeInfo(uint8_t nodeId) const {
        auto state = getNodeState(nodeId);
        return state ? &state->nodeInfo_ : nullptr;
    }


    NodeInfo *getNodeInfo(String uuid) const {
        auto it = std::find_if(nodePool_.begin(), nodePool_.end(), [&](const NodeState &ndc){
            return ndc.nodeInfo_.uuid_ == uuid;
        });

        return it != nodePool_.end() ? &(*it).nodeInfo_ : nullptr;
    }

    bool verifyId(uint8_t nodeId) const {
        return nodeId >= MinNodeId && nodeId <= MaxNodeId;
    }

    NodeState *isRegistered(uint8_t nodeId) const {
        auto state = getNodeState(nodeId);
        return state && state->registered_ ? state : nullptr;
    }
    
    NodeInfo *registerNode(uint8_t nodeId, String uuid, DeviceClass deviceClass, int inactiveDelayS){
        auto state = getNodeState(nodeId);
        if(!state){ return nullptr; }

        state->registered_ = true;
        state->inactiveDelayS_ = inactiveDelayS;
        state->setActiveNow();

        state->nodeInfo_.uuid_ = uuid;
        state->nodeInfo_.deviceClass_ = deviceClass;

        return &state->nodeInfo_;
    }

    bool unregisterNode(NodeState &ndc){
        ndc.reset();
        ndc.nodeInfo_.reset();
        return true;
    }


    void updateNodeStates(){
        // Find post inactive delay expired nodes
        uint8_t registeredCount = 0;
        uint8_t inactiveCount = 0;
        uint8_t unregisteredCount = 0;

        std::for_each(nodePool_.begin(), nodePool_.end(), [&](NodeState &ndc){

            if(!ndc.registered_){ return; }

            registeredCount++;

            if(!ndc.isActive()){
                String info = ndc.nodeInfo_.shortInfo();
                ndc.setInactiveDetectedNow();
  
                inactiveCount++;
                log_d("Inactive:[%s]", info.c_str());

                // If nodes inactive for some time then forget about them
                if(ndc.checkInactiveTimeout(PostInactiveTimeoutS)){
                    log_d("Inactive timeout:[%s]", info.c_str());
                    unregisterNode(ndc);

                    unregisteredCount++;
                    registeredCount--;
                    inactiveCount--;
                }
            }
        });

        log_d("RegisteredCount=[%d], InactiveCount=[%d], UnregisteredCount=[%d]", registeredCount, inactiveCount, unregisteredCount);

    }

private:   
    static constexpr uint8_t MinNodeId = 7; 
    static constexpr uint8_t MaxNodeId = 254;
    static constexpr int MaxNodeNumber = MaxNodeId - MinNodeId + 1;
    static constexpr int PostInactiveTimeoutS = 10; // seconds
    static std::vector<NodeState> nodePool_;
};

std::vector<NodeState> NodeManager::nodePool_;