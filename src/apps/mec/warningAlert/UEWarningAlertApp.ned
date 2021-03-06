//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

//
//	@author	Angelo Buono
//


package lte.apps.mec.warningAlert;

import inet.applications.contract.IUDPApp;

// UEWarningAlertApp simple module in charge to:
//
// 1) sending WarningAlertPacket of type START_MEAPP until receiving an ACK_START_MEAPP type 
//		to instantiate the required MEWarningAlertAlertApp instance
// 2) handling ACK_START_MEAPP
// 3) sending WarningAlertPacket of type INFO_UEAPP with its informations (position)
// 4) handling WarningAlertPacket of type INFO_MEAPP alerting of Danger on the road
// 5) sending WarningAlertPacket of type STOP_MEAPP until receiving an ACK_STOP_MEAPP type
//		to terminate the MEWarningAlertApp instance
// 6) handlin ACK_STOP_MEAPP

simple UEWarningAlertApp like IUDPApp
{
    parameters:       
         @display("i=block/source");    
        
        //connection infos
        int localPort = default(4000);
        int destPort = default(4000);
        string destAddress;
        int packetSize = default(10);
        
        //resource required infos
        int requiredRam @unit("B") = default(10MB);
        int requiredDisk @unit("B") = default(100MB);
        double requiredCpu = default(0.01);
        
        //autoscheduling infos
        double period @unit("s") = default(1s);
        double startTime @unit("s") = default(0s); 
        double stopTime @unit("s") = default(120s);
        
        string interfaceTableModule;
                    
    gates:
        
        output udpOut;
        input udpIn;
}

