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

#ifndef __SIMULTE_MEWARNINGALERTSERVICE_H_
#define __SIMULTE_MEWARNINGALERTSERVICE_H_

//WarningAlertPacket
#include "../../../../../apps/mec/warningAlert/packets/WarningAlertPacket_m.h"
#include "corenetwork/nodes/mec/MEPlatform/MEAppPacket_Types.h"

#include "inet/common/geometry/common/Coord.h"

/*
 * see MEWarningAlertService.ned
 */
class MEWarningAlertService : public cSimpleModule
{
    inet::Coord dangerEdgeA, dangerEdgeB, dangerEdgeC, dangerEdgeD;

    protected:

        virtual int numInitStages() const { return inet::NUM_INIT_STAGES; }
        virtual void initialize(int stage);
        virtual void handleMessage(cMessage *msg);

        void handleInfoUEWarningAlertApp(WarningAlertPacket* pkt);

        //utilities to check if the ue is within the danger area
        bool isInTriangle(inet::Coord P, inet::Coord A, inet::Coord B, inet::Coord C);
        bool isInQuadrilateral(inet::Coord P, inet::Coord A, inet::Coord B, inet::Coord C, inet::Coord D);
};

#endif
