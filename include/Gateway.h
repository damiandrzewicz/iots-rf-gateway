#pragma once
#include <functional>
#include "RFM69_ATC.h"
#include "BaseFrame.h"
#include "DataFrame.h"
#include "NodeRegisterFrame.h"
#include "NodeManager.h"
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
            log_d("after strncpy: %s", buffer_.data());

            if(!processRadioData()){
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

    bool processRadioData(){
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
                processRegisterNode();
            }
            else if(bfd.actionType == ActionType::Alive){

            }
        }

        return true;
    }

    bool processRegisterNode(){
        NodeRegisterRequest nrReq;
        NodeRegisterResponse nrRes;

        NodeRegisterFrame nrf;
        if(!nrf.parse(nrReq, buffer_)){
            log_w("parsing 'NodeRegisterRequest' failed!");
            return false;
        }

        auto nodeInfo = nodeManager_.findNextAvaliableNodeInfo();
        if(!nodeInfo){
            log_w("findNextAvaliableId failed!");
            return false;
        }

        nrRes.nodeId = nodeInfo->getNodeId();

        if(!nrf.build(buffer_, nrRes)){
            log_w("build 'NodeRegisterResponse' failed!");
            return false;
        }

        sendACKRepsonse();

        nodeInfo->doRegister(nrReq.uuid, nrReq.deviceClass, nrReq.sleepTime);
        log_d("registered node: %s", nodeInfo->toStringShort().c_str());
        return true;
    }

    void checkNodesAlive(){
        if(millis() - lastNodeAliveCheck > 1000){
            lastNodeAliveCheck = millis();
            log_d("checking inactive nodes...");
            auto inactiveCount = nodeManager_.updateNodeStates();
            log_d("found %d inactive nodes", inactiveCount);
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