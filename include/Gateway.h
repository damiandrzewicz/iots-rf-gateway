#pragma once
#include <functional>
#include "RFM69_ATC.h"
#include "BaseFrame.h"
#include "DataFrame.h"
#include "NodeManager.h"
#include "NodeRegisterFrame.h"
#include "NodeAliveFrame.h"

//#include "Timer.h"

class Gateway
{
public:
    void setup(){
        initRadio();
    }

    void loop(){
        checkRadio();
        checkNodesAlive();
    }

protected:
    void initRadio(){
        log_d("initializing radio: radioType=[RF69_868MHZ], nodeId(gateway)=[%d], networkId=[%d], customFrequency=[%d], encryptKey=[%s]", 
            nodeId_, networkId_, customFrequency_, encryptKey_.c_str()
        );
        if(radio_.initialize(radioType_, 1, networkId_)){
            radio_.setHighPower();
            radio_.encrypt(encryptKey_.c_str());
            radio_.setFrequency(customFrequency_);
            log_d("radio init ok!");
        }
        else{
            log_e("radio init failed!");
            while(1){}
        }
    }

    void checkRadio(){
        if (radio_.receiveDone())
        {
            char *data = reinterpret_cast<char*>(radio_.DATA);
            log_i("-->radio: sender=[%d], dataLen=[%d], rssi=[%d], data=[%s]",
                radio_.SENDERID, radio_.DATALEN, radio_.RSSI, data
            );

            strncpy(buffer_.data(), data, buffer_.Size);

            if(!processRadioData(radio_.SENDERID)){
                return;
            }
        }
    }

    void sendACKRepsonse(){
        if(radio_.ACKRequested()){
            if(buffer_.size()){
                log_i("<--radio ACK: data=[%s]", buffer_.data());
                radio_.sendACK(buffer_.data(), buffer_.size());
            }
            else{
                radio_.sendACK();
            }
        }
    }

    bool processRadioData(uint16_t nodeId){
        BaseFrameData bfd;
        BaseFrame baseFrame;
        if(!baseFrame.parse(buffer_, bfd)){
            log_w("parsing 'BaseFrame' failed!");
            return false;
        }

        // Process only following requests
        if(bfd.actionDirection == ActionDirection::Request){
            // Register - can be send only from node to gateway
            if(bfd.actionType == ActionType::Register){
                processNodeRegister();
            }
            else if(bfd.actionType == ActionType::Alive){
                processNodeAlive(nodeId);
            }
        }

        return true;
    }

    bool processNodeRegister(){
        NodeRegisterRequest nrReq;
        NodeRegisterResponse nrRes;

        NodeRegisterFrame nrf;
        if(!nrf.parse(nrReq, buffer_)){
            log_w("parsing 'NodeRegisterRequest' failed!");
            return false;
        }

        auto nodeInfo = nodeManager_.findNextFreeId();
        if(!nodeInfo){
            log_w("Node Pool full!");
            return false;
        }


        nrRes.error = ActionError::Ok;
        nrRes.nodeId = nodeInfo->nodeId_;

        if(!nrf.build(buffer_, nrRes)){
            log_w("build 'NodeRegisterResponse' failed!");
            return false;
        }

        sendACKRepsonse();

        nodeInfo = nodeManager_.registerNode(nodeInfo->nodeId_, nrReq.uuid, nrReq.deviceClass, nrReq.sleepTime);
        log_d("Registered Node:[%s]", nodeInfo->shortInfo().c_str());
        return true;
    }

    bool processNodeAlive(uint16_t nodeId)
    {
        NodeAliveFrame nodeAliveFrame;
        NodeAliveRequest request;
        NodeAliveResponse response;
        
        if(auto nodeState = nodeManager_.isRegistered(nodeId)){
            // Parse request
            if(!nodeAliveFrame.parse(request, buffer_)){
                log_w("parsing 'NodeRegisterRequest' failed!");
                return false;
            }
            response.error = ActionError::Ok;

            // Build response
            if(!nodeAliveFrame.build(buffer_, response)){
                log_w("building 'NodeAliveResponse' failed!");
                return false;
            }

            sendACKRepsonse();

            nodeState->nodeInfo_.batteryPercent_ = request.batteryPercent;
            nodeState->setActiveNow();
        }
        else{
            response.error = ActionError::NotRegistered;

            if(!nodeAliveFrame.build(buffer_, response)){
                log_w("building 'NodeAliveResponse' failed!");
                return false;
            }
 
            sendACKRepsonse();
        }

        return true;
    }

    void checkNodesAlive(){
        // Call it every 1 second
        if(millis() - lastNodeAliveCheck > 1000){
            lastNodeAliveCheck = millis();
            nodeManager_.updateNodeStates();
        }
    }

private:
    static constexpr int NSS_PIN = RF69_SPI_CS;
    static constexpr int IRQ_PIN = 4;
    RFM69_ATC radio_ = { NSS_PIN, IRQ_PIN };

    String encryptKey_ = "sampleEncryptKey";
    int radioType_ = RF69_868MHZ;
    int nodeId_ = 1;
    int networkId_ = 101;
    long customFrequency_ = 869000000;

    Buffer buffer_;
    NodeManager nodeManager_;
    long lastNodeAliveCheck = 0;
};