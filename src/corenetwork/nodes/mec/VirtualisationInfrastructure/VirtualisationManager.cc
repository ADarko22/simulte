//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

//
//  @author Angelo Buono
//


#include "VirtualisationManager.h"

Define_Module(VirtualisationManager);

void VirtualisationManager::initialize(int stage)
{
    EV << "VirtualisationManager::initialize - stage " << stage << endl;

    cSimpleModule::initialize(stage);

    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    // get reference to the binder
    binder_ = getBinder();

    destPort_ = par("destPort").longValue();
    localPort_ = par("localPort").longValue();

    EV << "VirtualisationManager::initialize - binding to port: local:" << localPort_ << " , dest:" << destPort_ << endl;
    socket.setOutputGate(gate("udpOut"));
    socket.bind(localPort_);

    virtualisationInfr = getParentModule();

    //initializing VirtualisationInfrastructure gates & connections
    if(virtualisationInfr != NULL){
        meHost = virtualisationInfr->getParentModule();
        if(meHost->hasPar("maxMEApps"))
                maxMEApps = meHost->par("maxMEApps").longValue();
        else
            throw cRuntimeError("VirtualisationManager::initialize - \tFATAL! Error when getting meHost.maxMEApps parameter!");

        // VirtualisationInfrastructure internal gate connections with VirtualisationManager
        virtualisationInfr->setGateSize("meAppOut", maxMEApps);
        virtualisationInfr->setGateSize("meAppIn", maxMEApps);
        this->setGateSize("meAppOut", maxMEApps);
        this->setGateSize("meAppIn", maxMEApps);
        for(int index = 0; index < maxMEApps; index++){
            this->gate("meAppOut", index)->connectTo(virtualisationInfr->gate("meAppOut", index));
            virtualisationInfr->gate("meAppIn", index)->connectTo(this->gate("meAppIn", index));
        }
    }
    else{
        EV << "VirtualisationManager::initialize - ERROR getting virtualisationInfrastructure cModule!" << endl;
        throw cRuntimeError("VirtualisationManager::initialize - \tFATAL! Error when getting getParentModule()");
    }
    currentMEApps = 0;

    //initializing MEPlatform Services
    if(meHost != NULL){
        mePlatform = meHost->getSubmodule("mePlatform");
        if(mePlatform == NULL){
            EV << "VirtualisationManager::initialize - ERROR getting mePlatform cModule!" << endl;
            throw cRuntimeError("VirtualisationManager::initialize - \tFATAL! Error when getting meHost->getSubmodule()");
        }
        if(meHost->hasPar("MEClusterizeServicePath")){
            meClusterizeService = meHost->getModuleByPath(meHost->par("MEClusterizeServicePath"));
            EV << "VirtualisationManager::initialize - MEClusterizeService found!" << endl;
        }else
            EV << "VirtualisationManager::initialize - WARNING MEClusterizeService not found!" << endl;
    }

    interfaceTableModule = par("interfaceTableModule").stringValue();

    for(int i = 0; i < maxMEApps; i++)
        freeGates.push_back(i);
}

void VirtualisationManager::handleMessage(cMessage *msg)
{
    EV << "VirtualisationManager::handleMessage" << endl;

    if (msg->isSelfMessage())
        return;

    if(msg->arrivedOn("resourceManagerIn")){

        handleResource(msg);
    }
    else{
        MEAppPacket* mepkt = check_and_cast<MEAppPacket*>(msg);
        if(mepkt == 0){
            EV << "VirtualisationManager::handleMessage - \tFATAL! Error when casting to MEAppPacket" << endl;
            throw cRuntimeError("VirtualisationManager::handleMessage - \tFATAL! Error when casting to MEAppPacket");
        }
        else if(!strcmp(mepkt->getName(), START_CLUSTERIZE) || !strcmp(mepkt->getName(), STOP_CLUSTERIZE)
                                || !strcmp(mepkt->getName(), CONFIG_CLUSTERIZE) || !strcmp(mepkt->getName(), INFO_CLUSTERIZE)){
            /*
             * HANDLING CLUSTERIZE PACKETS
             */
            ClusterizePacket* pkt = check_and_cast<ClusterizePacket*>(msg);
            handleClusterize(pkt);
        }
    }
}

/*
 * ###################################RESOURCE MANAGER decision HANDLER#################################
 */

void VirtualisationManager::handleResource(cMessage* msg){

    MEAppPacket* mepkt = check_and_cast<MEAppPacket*>(msg);
    if(mepkt == 0){
        EV << "VirtualisationManager::handleMessage - \tFATAL! Error when casting to MEAppPacket" << endl;
        throw cRuntimeError("VirtualisationManager::handleMessage - \tFATAL! Error when casting to MEAppPacket");
    }
    /*
     * HANDLING CLUSTERIZE PACKETS for RESOURCES ALLOCATION/DEALLOCATION
     */
    else if(!strcmp(mepkt->getName(), START_CLUSTERIZE)){

        ClusterizePacket* pkt = check_and_cast<ClusterizePacket*>(msg);
        EV << "VirtualisationManager::handleMessage - calling instantiateMEClusterizeApp" << endl;
        instantiateMEClusterizeApp(pkt);
    }
    else if(!strcmp(mepkt->getName(), STOP_CLUSTERIZE)){

        ClusterizePacket* pkt = check_and_cast<ClusterizePacket*>(msg);
        //Sending ACK to the UEClusterizeApp
        EV << "VirtualisationManager::handleResource - calling terminateMEClusterizeApp" << endl;
        terminateMEClusterizeApp(pkt);
    }
}
/*
 * ######################################################################################################
 */

/*
 * #######################################CLUSTERIZE PACKETS HANDLERS####################################
 */

void VirtualisationManager::handleClusterize(ClusterizePacket* pkt){

    EV << "VirtualisationManager::handleClusterize - received "<< pkt->getType()<<" Delay: " << (simTime()-pkt->getTimestamp()) << endl;

    /* -------------------------------
     * Handling ClusterizeStartPacket */
    if(!strcmp(pkt->getName(), START_CLUSTERIZE)){

       startClusterize(pkt);
    }
    /* -------------------------------
     * Handling ClusterizeInfoPacket */
    else if(!strcmp(pkt->getName(), INFO_CLUSTERIZE)){

        ClusterizeInfoPacket* ipkt = check_and_cast<ClusterizeInfoPacket*>(pkt);

        upstreamClusterize(ipkt);
    }
    /* -------------------------------
     * Handling ClusterizeConfigPacket */
    else if(!strcmp(pkt->getName(), CONFIG_CLUSTERIZE)){

        ClusterizeConfigPacket* cpkt = check_and_cast<ClusterizeConfigPacket*>(pkt);

        downstreamClusterize(cpkt);
    }
    /* -------------------------------
     * Handling ClusterizeStopPacket */
    else if(!strcmp(pkt->getName(), STOP_CLUSTERIZE)){

        stopClusterize(pkt);
    }
}

void VirtualisationManager::startClusterize(ClusterizePacket* pkt){

    EV << "VirtualisationManager::startClusterize - processing..." << endl;

    //creating the map key
    std::stringstream key;
    key << pkt->getSourceAddress();

    //checking if there are available slots for MEClusterizeApp && the MEClusterizeApp is not already instantiated!
    if(currentMEApps < maxMEApps && meAppMapTable.find(key.str()) == meAppMapTable.end()){

        EV << "VirtualisationManager::startClusterize - currentMEApps: " << currentMEApps << " / " << maxMEApps << endl;
        //Checking for resource availability to the Resource Manager
        EV << "VirtualisationManager::startClusterize - forwarding " << pkt->getType() << " to Resource Manager!" << endl;
        send(pkt, "resourceManagerOut");
    }
    else{
        if(meAppMapTable.find(key.str()) != meAppMapTable.end()){

            EV << "VirtualisationManager::startClusterize - \tWARNING: MEClusterizeApp ALREADY STARTED!" << endl;
            //Sending ACK to the UEClusterizeApp to confirm the instantiation in case of previous ack lost!
            EV << "VirtualisationManager::startClusterize  - calling ackClusterize with  "<< ACK_START_CLUSTERIZE << endl;
            ackClusterize(pkt, ACK_START_CLUSTERIZE);
        }else
            EV << "VirtualisationManager::startClusterize - \tWARNING: maxMEApp LIMIT REACHED!" << endl;
    }
}


void VirtualisationManager::upstreamClusterize(ClusterizeInfoPacket* pkt){

    EV << "VirtualisationManager::upstreamClusterize - processing..."<< endl;

    //creating the map key
    std::stringstream key;
    key << pkt->getSourceAddress();

    if(!meAppMapTable.empty() && meAppMapTable.find(key.str()) != meAppMapTable.end()){

        send(pkt, "meAppOut", meAppMapTable[key.str()].meAppGateIndex);

        EV << "VirtualisationManager::upstreamClusterize - forwarding ClusterizeInfoPacket to meAppOut["<< meAppMapTable[key.str()].meAppGateIndex << "] gate" << endl;
    }
    else{

        EV << "VirtualisationManager::upstreamClusterize - \tWARNING: NO MEClusterizeApp INSTANCE FOUND!" << endl;
    }
}

void VirtualisationManager::downstreamClusterize(ClusterizeConfigPacket* pkt){

    //creating the map key
    std::stringstream key;
    key << pkt->getDestinationAddress();

    const char* destSimbolicAddr = pkt->getDestinationAddress();

    if(!meAppMapTable.empty() && meAppMapTable.find(key.str()) != meAppMapTable.end()){

        destAddress_ = meAppMapTable[key.str()].ueAddress;

        EV << "VirtualisationManager::downstreamClusterize - forwarding ClusterizeConfigPacket to "<< destSimbolicAddr << ": [" << destAddress_.str() <<"]" << endl;

        //checking if the Car is in the network & sending by socket
        MacNodeId destId = binder_->getMacNodeId(destAddress_.toIPv4());
        if(destId == 0){
            EV << "VirtualisationeManager::downstreamClusterize - \tWARNING " << destSimbolicAddr << "has left the network!" << endl;
            //throw cRuntimeError("VirtualisationManager::downstreamClusterize - \tFATAL! Error destination has left the network!");

            //NOTE: in case the STOP_CLUSTERIZE sent from the UEClusterizeApp is not received correctly!
            //
            //starting the MEClusterizeApp termination procedure
            //
            //creating the STOP_CLUSTERIZE ClusterizePacket

            ClusterizePacket* spkt = ClusterizePacketBuilder().buildClusterizePacket(STOP_CLUSTERIZE, pkt->getSno(), simTime(), pkt->getByteLength(), pkt->getCarOmnetID(), pkt->getDestinationAddress(), pkt->getSourceAddress());

            //
            EV << "VirtualisationeManager::downstreamClusterize - calling stopClusterize for " << destSimbolicAddr << endl;
            //calling the stopClusterize to handle the MEClusterizeApp termination
            //due to correspondent Car exit the network
            stopClusterize(spkt);

            delete(pkt);
        }
        else
            socket.sendTo(pkt, destAddress_, destPort_);
    }
    else
        EV << "VirtualisationeManager::downstreamClusterize - \tWARNING forwarding to "<< destSimbolicAddr << ": map entry "<< key.str() <<" not found!" << endl;

}


void VirtualisationManager::stopClusterize(ClusterizePacket* pkt){

    EV << "VirtualisationManager::stopClusterize - processing..." << endl;

    //creating the map key
    std::stringstream key;
    key << pkt->getSourceAddress();

    if(!meAppMapTable.empty() && meAppMapTable.find(key.str()) != meAppMapTable.end()){

        //Asking to free resources  to the Resource Manager
        EV << "VirtualisationManager::stopClusterize - forwarding "<< pkt->getType() << " to Resource Manager!" << endl;
        send(pkt, "resourceManagerOut");
    }
    else{
        EV << "VirtualisationManager::stopClusterize - \tWARNING: NO MEClusterizeApp INSTANCE FOUND!" << endl;
    }
}

void VirtualisationManager::instantiateMEClusterizeApp(ClusterizePacket* pkt){

    //creating the map key
    std::stringstream key;
    key << pkt->getSourceAddress();

    if(currentMEApps < maxMEApps &&  meAppMapTable.find(key.str()) == meAppMapTable.end()){

        EV << "VirtualisationManager::instantiateMEClusterizeApp - key: " << key.str() << endl;

        //getting the first available gates
        int index = freeGates.front();
        freeGates.erase(freeGates.begin());
        EV << "VirtualisationManager::instantiateMEClusterizeApp - freeGate: " << index << endl;

        //getting the UEClusterizeApp L3Address
        inet::L3Address ueAppAddress = inet::L3AddressResolver().resolve(pkt->getSourceAddress());
        EV << "VirtualisationManager::instantiateMEClusterizeApp - UEAppL3Address: " << ueAppAddress.str() << endl;

        // creating MEClusterizeApp instance
        cModuleType *moduleType = cModuleType::get("lte.apps.vehicular.mec.clusterize.MEClusterizeApp");
        cModule *module = moduleType->create("MEClusterizeApp", meHost);     //name & its Parent Module
        std::stringstream appName;
        appName << "MEApp[" <<  key.str().c_str() << "]";
        module->setName(appName.str().c_str());

        //displaying ME App dynamically created (after 70 they will overlap..)
        std::stringstream display;
        display << "p=" << (50 + ((index%10)*100)%1000) << "," << (100 + (50*(index/10)%350)) << ";is=vs";
        module->setDisplayString(display.str().c_str());

        module->par("sourceAddress") = pkt->getDestinationAddress();
        module->par("destAddress") = pkt->getSourceAddress();
        module->par("interfaceTableModule") = interfaceTableModule;

        module->finalizeParameters();

        EV << "VirtualisationManager::instantiateMEClusterizeApp - UEAppSimbolicAddress: " << pkt->getSourceAddress()<< endl;

        //connecting VirtualisationInfrastructure gates to the MEClusterizeApp gates
        virtualisationInfr->gate("meAppOut", index)->connectTo(check_and_cast<MEClusterizeApp*>(module)->gate("virtualisationInfrastructureIn"));
        check_and_cast<MEClusterizeApp*>(module)->gate("virtualisationInfrastructureOut")->connectTo(virtualisationInfr->gate("meAppIn", index));

        //connecting MEPlatform gates to the MEClusterizeApp gates
        mePlatform->gate("meAppOut", index)->connectTo(check_and_cast<MEClusterizeApp*>(module)->gate("mePlatformIn"));
        check_and_cast<MEClusterizeApp*>(module)->gate("mePlatformOut")->connectTo(mePlatform->gate("meAppIn", index));

        //connecting internal MEPlatform gates to the MEClusterizeService gates
        meClusterizeService->gate("meAppOut", index)->connectTo(mePlatform->gate("meAppOut", index));
        mePlatform->gate("meAppIn", index)->connectTo(meClusterizeService->gate("meAppIn", index));

        module->buildInside();
        module->scheduleStart(simTime());
        module->callInitialize();

        //creating the map entry
        meAppMapTable[key.str()].meAppGateIndex = index;
        meAppMapTable[key.str()].meAppModule = module;
        meAppMapTable[key.str()].ueAddress = ueAppAddress;

        currentMEApps++;

        EV << "VirtualisationManager::instantiateMEClusterizeApp - MEClusterizeApp[" << key.str() <<"] instanced!" << endl;
        EV << "VirtualisationManager::instantiateMEClusterizeApp - currentMEApps: " << currentMEApps << " / " << maxMEApps << endl;

        //Sending ACK to the UEClusterizeApp
        EV << "VirtualisationManager::instantiateMEClusterizeApp - calling ackClusterize with  "<< ACK_START_CLUSTERIZE << endl;
        ackClusterize(pkt, ACK_START_CLUSTERIZE);
    }
}

void VirtualisationManager::terminateMEClusterizeApp(ClusterizePacket* pkt){

    //creating the map key
    std::stringstream key;
    key << pkt->getSourceAddress();

    if(!meAppMapTable.empty() && meAppMapTable.find(key.str()) != meAppMapTable.end()){

        //Sending ACK to the UEClusterizeApp
        EV << "VirtualisationManager::terminateMEClusterizeApp - calling ackClusterize with  "<< ACK_STOP_CLUSTERIZE << endl;
        //before to remove the map entry!
        ackClusterize(pkt, ACK_STOP_CLUSTERIZE);

        EV << "VirtualisationManager::terminateMEClusterizeApp - currentMEApps: " << currentMEApps << " / " << maxMEApps << endl;

        int index = meAppMapTable[key.str()].meAppGateIndex;

        meAppMapTable[key.str()].meAppModule->callFinish();
        meAppMapTable[key.str()].meAppModule->deleteModule();

        //disconnecting internal MEPlatform gates to the MEClusterizeService gates
        meClusterizeService->gate("meAppOut", index)->disconnect();
        meClusterizeService->gate("meAppIn", index)->disconnect();

        //disconnecting MEPlatform gates to the MEClusterizeApp gates
        mePlatform->gate("meAppOut", index)->disconnect();
        mePlatform->gate("meAppIn", index)->disconnect();

        //update map
        std::map<std::string, meAppMapEntry>::iterator it1;
        it1 = meAppMapTable.find(key.str());
        if (it1 != meAppMapTable.end())
            meAppMapTable.erase (it1);

        freeGates.push_back(index);

        currentMEApps--;

        EV << "VirtualisationManager::terminateMEClusterizeApp - MEClusterizeApp[" << key.str() << "] terminated!" << endl;
        EV << "VirtualisationManager::terminateMEClusterizeApp - currentMEApps: " << currentMEApps << " / " << maxMEApps << endl;
    }
    else{
        //Sending ACK to the UEClusterizeApp
        EV << "VirtualisationManager::terminateMEClusterizeApp - calling ackClusterize with  "<< ACK_STOP_CLUSTERIZE << endl;
        ackClusterize(pkt, ACK_STOP_CLUSTERIZE);

        EV << "VirtualisationManager::terminateMEClusterizeApp - \tWARNING: NO MEClusterizeApp INSTANCE FOUND!" << endl;
    }
}

void VirtualisationManager::ackClusterize(ClusterizePacket* pkt, const char* type){

    //creating the map key
    std::stringstream key;
    key << pkt->getSourceAddress();

    const char* destSimbolicAddr = pkt->getSourceAddress();

    if(!meAppMapTable.empty() && meAppMapTable.find(key.str()) != meAppMapTable.end()){

        destAddress_ = meAppMapTable[key.str()].ueAddress;

        //checking if the Car is in the network & sending by socket
        MacNodeId destId = binder_->getMacNodeId(destAddress_.toIPv4());
        if(destId == 0){
            EV << "VirtualisationManager::ackClusterize - \tWARNING " << destSimbolicAddr << "has left the network!" << endl;
            //throw cRuntimeError("VirtualisationManager::ackClusterize - \tFATAL! Error destination has left the network!");
        }
        else{
            EV << "VirtualisationManager::ackClusterize - sending ack " << type <<" to "<< destSimbolicAddr << ": [" << destAddress_.str() <<"]" << endl;

            ClusterizePacket* ack = ClusterizePacketBuilder().buildClusterizePacket(type, pkt->getSno(), simTime(), pkt->getByteLength(), pkt->getCarOmnetID(), pkt->getDestinationAddress(), destSimbolicAddr);

            socket.sendTo(ack, destAddress_, destPort_);
        }
    }
    else
        EV << "VirtualisationManager::ackClusterize - \tWARNING sending ack " << type <<" to "<< destSimbolicAddr << ": map entry "<< key.str() <<" not found!" << endl;

    delete(pkt);
}
/*
 * ######################################################################################################
 */

