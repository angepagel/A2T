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

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace Veins;

Define_Module(Veins::TraCIDemoRSU11p);

void TraCIDemoRSU11p::initialize(int stage) {
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0)
    {
        lastMessageTreeId = 0;
        reinitializationDelay = 3;
        lastUpdate = simTime();
        highestPriority = 0;
    }
}

void TraCIDemoRSU11p::onWSA(DemoServiceAdvertisment* wsa)
{
    // if this RSU receives a WSA for service 42, it will tune to the chan
    if (wsa->getPsid() == 42) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
    }
}

void TraCIDemoRSU11p::onWSM(BaseFrame1609_4* frame)
{
    /* For some reason, an initialize() function wouldn't initialize the traci interface.
     * Meanwhile, this is done manually here. :^(
     */
    Veins::TraCIScenarioManager* manager = Veins::TraCIScenarioManagerAccess().get();
    TraCICommandInterface* traci = manager->getCommandInterface();

    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    if (associatedTlId.empty())
    {
        for (std::string junctionId: traci->getJunctionIds())
        {
            TraCICommandInterface::Junction junction = traci->junction(junctionId);
            double distanceFromJunction = traci->getDistance(curPosition, junction.getPosition(), false);

            if (distanceFromJunction < 10)
            {
                for (std::string tlId: traci->getTrafficlightIds())
                    if (junctionId == tlId) associatedTlId = tlId;
            }
        }
    }

    TraCICommandInterface::Trafficlight associatedTl = traci->trafficlight(associatedTlId);

    long messageTreeId = wsm->getTreeId();

    if (lastMessageTreeId != messageTreeId) // The message has not been processed yet
    {
        lastMessageTreeId = messageTreeId;

        if (simTime()-lastUpdate >= reinitializationDelay)
        {
            lastUpdate = simTime();
            highestPriority = 0;
            associatedTl.reinitialize(); // Checks if the traffic light has to be set back to its normal state
        }

        int wsmPriority = wsm->getPriority();

        if (wsm->getIsFromAmbulance())
        {
            std::string amuLaneId = wsm->getAmuLaneId();
            std::string amuRoadId = traci->lane(amuLaneId).getRoadId();

            if (associatedTl.isControlling(amuRoadId))
            {
                if (wsmPriority >= highestPriority)
                {
                    highestPriority = wsmPriority;
                    lastUpdate = simTime();

                    associatedTl.prioritizeRoad(amuRoadId);
                }
            }
       }
    }
    else EV << "<!> This message has already been processed." << endl;
}
