//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/application/traci/TraCIDemoRSU11p.h"

Define_Module(TraCIDemoRSU11p);

void TraCIDemoRSU11p::onWSA(WaveServiceAdvertisment* wsa) {
    //if this RSU receives a WSA for service 42, it will tune to the chan
    if (wsa->getPsid() == 42) {
        mac->changeServiceChannel(wsa->getTargetChannel());
    }
}

void TraCIDemoRSU11p::onWSM(WaveShortMessage* wsm) {

    // -------------------------- Veins example original code --------------------------
    /*
    //this rsu repeats the received traffic update in 2 seconds plus some random delay
    wsm->setSenderAddress(myId);
    sendDelayedDown(wsm->dup(), 2 + uniform(0.01,0.2));
    */
    // -------------------------- Veins example original code --------------------------

    // -------------------------- A2T --------------------------

    /* For some reason, an initialize() function wouldn't initialize the traci interface.
     * Meanwhile, this is done manually here. :^(
     */
    Veins::TraCIScenarioManager* manager = Veins::TraCIScenarioManagerAccess().get();
    TraCICommandInterface* traci = manager->getCommandInterface();

    EV << "======================= RSU INFORMATION =======================" << endl;

    if (wsm->getAmbulance())
    {
        EV << "<!> Received a message from an AMU." << endl;

        std::string amuRoadId = wsm->getRoadId();

        // For each traffic light
        for (auto const& tlId: traci->getTrafficlightIds())
        {
            std::list<std::string> controlledLanesIds = traci->trafficlight(tlId).getControlledLanes();

            // Checks if the traffic light has to be set back to its normal state
            traci->trafficlight(tlId).checkForReinitialization();

            // For each controlled lane
            for (std::string laneId: controlledLanesIds)
            {
                // Is the controlled lane's road the same as the AMU road ?
                if (traci->lane(laneId).getRoadId() == amuRoadId)
                {
                    traci->trafficlight(tlId).prioritizeRoad(amuRoadId);
                    break;
                }
            }
        }
    }
    // -------------------------- A2T --------------------------
}
