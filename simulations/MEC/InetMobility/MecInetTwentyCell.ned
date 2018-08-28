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

package lte.simulations.MEC.InetMobility;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;

import lte.world.radio.LteChannelControl;
import lte.epc.PgwStandardSimplified;
import lte.corenetwork.binder.LteBinder;
import lte.corenetwork.nodes.eNodeB;
import lte.corenetwork.nodes.Ue;
import lte.corenetwork.nodes.cars.Car;

//mec
import lte.corenetwork.nodes.mec.MEHost;
import lte.corenetwork.nodes.mec.VirtualisationInfrastructure.VirtualisationInfrastructure;
import lte.corenetwork.nodes.mec.MEPlatform.MEPlatform;

//veins mobility
import org.car2x.veins.subprojects.veins_inet.VeinsInetManager;

//statistics
import lte.apps.d2dMultihop.statistics.MultihopD2DStatistics;

network MecInetTwentyCell
{
    parameters:
        @display("bgb=1000,4999");	//put to 6000
        
        int numCars = default(1);
        int numClusters = default(1);
        int numRoads = default(1);
        
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)

    submodules:

        routingRecorder: RoutingTableRecorder {	@display("p=885,115;is=s");}
        
        configurator: IPv4NetworkConfigurator {	@display("p=885,195");	config = xmldoc("demo.xml");}
            
        d2dMultihopStatistics: MultihopD2DStatistics {	@display("p=885,340;is=s");}

        //# LTE modules
        channelControl: LteChannelControl {	@display("p=885,35;is=s");}
        
        binder: LteBinder {	@display("p=885,275;is=s");}

        eNodeB1: eNodeB {	@display("p=100,100;is=vl");}
        
        eNodeB2: eNodeB {	@display("p=200,350;is=vl");}
        
        eNodeB3: eNodeB {	@display("p=100,600;is=vl");}
        
        eNodeB4: eNodeB {	@display("p=200,850;is=vl");}
        
        eNodeB5: eNodeB {	@display("p=100,1100;is=vl");}
        
        eNodeB6: eNodeB {	@display("p=200,1350;is=vl");}
        
        eNodeB7: eNodeB {	@display("p=100,1600;is=vl");}
        
        eNodeB8: eNodeB {	@display("p=200,1850;is=vl");}
        
        eNodeB9: eNodeB {	@display("p=100,2100;is=vl");}
        
        eNodeB10: eNodeB {	@display("p=200,2350;is=vl");}
        
        eNodeB11: eNodeB {	@display("p=100,2600;is=vl");}
        
        eNodeB12: eNodeB {	@display("p=200,2850;is=vl");}
        
        eNodeB13: eNodeB {	@display("p=100,3100;is=vl");}
        
        eNodeB14: eNodeB {	@display("p=200,3350;is=vl");}
        
        eNodeB15: eNodeB {	@display("p=100,3600;is=vl");}
        
        eNodeB16: eNodeB {	@display("p=200,3850;is=vl");}
        
        eNodeB17: eNodeB {	@display("p=100,4100;is=vl");}
        
        eNodeB18: eNodeB {	@display("p=200,4350;is=vl");}
        
        eNodeB19: eNodeB {	@display("p=100,4600;is=vl");}
        
        eNodeB20: eNodeB {	@display("p=200,4850;is=vl");}
        
        car[numCars]: Car {	 @display("p=300,300");}

        pgw: PgwStandardSimplified {	@display("p=970,2500;is=l");	nodeType = "PGW";}

        //# MEC module
        meHost: MEHost {	@display("p=615,2500;is=l");}

    connections allowunconnected:

		//#eNodeBs PPP connections with PGW
		pgw.pppg++ <--> Eth10G <--> eNodeB1.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB2.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB3.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB4.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB5.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB6.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB7.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB8.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB9.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB10.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB11.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB12.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB13.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB14.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB15.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB16.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB17.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB18.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB19.ppp;
		pgw.pppg++ <--> Eth10G <--> eNodeB20.ppp;

		//#eNodeBs X2 connections between eNodeBs
		eNodeB1.x2++ <--> Eth10G <--> eNodeB2.x2++;
		eNodeB2.x2++ <--> Eth10G <--> eNodeB3.x2++;
		eNodeB3.x2++ <--> Eth10G <--> eNodeB4.x2++;
		eNodeB4.x2++ <--> Eth10G <--> eNodeB5.x2++;
		eNodeB5.x2++ <--> Eth10G <--> eNodeB6.x2++;
		eNodeB6.x2++ <--> Eth10G <--> eNodeB7.x2++;
		eNodeB7.x2++ <--> Eth10G <--> eNodeB8.x2++;
		eNodeB8.x2++ <--> Eth10G <--> eNodeB9.x2++;
		eNodeB9.x2++ <--> Eth10G <--> eNodeB10.x2++;
		eNodeB10.x2++ <--> Eth10G <--> eNodeB11.x2++;
		eNodeB11.x2++ <--> Eth10G <--> eNodeB12.x2++;
		eNodeB12.x2++ <--> Eth10G <--> eNodeB13.x2++;
		eNodeB13.x2++ <--> Eth10G <--> eNodeB14.x2++;
		eNodeB14.x2++ <--> Eth10G <--> eNodeB15.x2++;
		eNodeB15.x2++ <--> Eth10G <--> eNodeB16.x2++;
		eNodeB16.x2++ <--> Eth10G <--> eNodeB17.x2++;
		eNodeB17.x2++ <--> Eth10G <--> eNodeB18.x2++;
		eNodeB18.x2++ <--> Eth10G <--> eNodeB19.x2++;
		eNodeB19.x2++ <--> Eth10G <--> eNodeB20.x2++;

        //# direct connection between eNodeB2 and meHost
        meHost.pppENB++ <--> Eth10G <--> eNodeB1.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB2.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB3.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB4.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB5.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB6.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB7.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB8.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB9.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB10.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB11.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB12.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB13.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB14.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB15.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB16.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB17.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB18.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB19.pppMEHost;
        meHost.pppENB++ <--> Eth10G <--> eNodeB20.pppMEHost;
}
