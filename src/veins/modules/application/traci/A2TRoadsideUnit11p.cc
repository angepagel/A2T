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

#include <veins/modules/application/traci/A2TMessage11p_m.h>
#include <veins/modules/application/traci/A2TRoadsideUnit11p.h>


using namespace Veins;

Define_Module(Veins::A2TRoadsideUnit11p);

void A2TRoadsideUnit11p::initialize(int stage)
{
    A2TBaseApplLayer::initialize(stage);
    if (stage == 0)
    {
        // ---------------------- Simulation parameters ----------------------
        // TODO Set these parameters in the omnetpp.ini file
        reinitializationDelay = 5; // seconds
        // ---------------------- Simulation parameters ----------------------

        initialized = false;
        lastMessageTreeId = 0;
        lastUpdate = simTime();
        highestPriority = 0;
    }
}

void A2TRoadsideUnit11p::onWSM(BaseFrame1609_4* frame)
{
    A2TMessage11p* wsm = check_and_cast<A2TMessage11p*>(frame);

    if (!initialized)
    {
        setTraCI();
        associateTrafficlight();

        initialized = true;
    }

    if (!associatedTlId.empty())
    {
        TraCICommandInterface::Trafficlight associatedTl = traci->trafficlight(associatedTlId);

        long messageTreeId = wsm->getTreeId();

        if (lastMessageTreeId != messageTreeId) // The message has not been processed yet
        {
            lastMessageTreeId = messageTreeId;

            EV << "Traffic light state:" << endl;
            EV << "  - Cooldown: " << simTime()-lastUpdate << endl;
            EV << "  - Highest priority: " << highestPriority << endl;
            EV << "  - Last ambulance ID: " << memorizedAmuId << endl;
            EV << "  - Last update: " << lastUpdate << endl;

            if (simTime()-lastUpdate >= reinitializationDelay)
            {
                lastUpdate = simTime();
                memorizedAmuId = "none";
                highestPriority = 0;
                associatedTl.reinitialize(); // Check if the traffic light has to be set back to its normal state
            }

            if (wsm->getIsFromAmbulance())
            {
                std::string amuLaneId = wsm->getAmuLaneId();
                std::string amuRoadId = traci->lane(amuLaneId).getRoadId();

                if (associatedTl.isControlling(amuRoadId))
                {
                    int wsmPriority = wsm->getPriority();
                    std::string wsmAmuId = wsm->getAmuId();

                    if (wsmPriority == highestPriority)
                    {
                        if (wsmAmuId == memorizedAmuId || memorizedAmuId == "none")
                        {
                            associatedTl.prioritizeRoad(amuRoadId);
                            update(wsmAmuId, wsmPriority);
                        }
                    }
                    else if (wsmPriority > highestPriority)
                    {
                        associatedTl.prioritizeRoad(amuRoadId);
                        update(wsmAmuId, wsmPriority);
                    }
                }
            }
        }
        else EV << "<!> This message has already been processed." << endl;
    }

}

void A2TRoadsideUnit11p::setTraCI()
{
    Veins::TraCIScenarioManager* manager = Veins::TraCIScenarioManagerAccess().get();
    traci = manager->getCommandInterface();
}

void A2TRoadsideUnit11p::associateTrafficlight()
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

void A2TRoadsideUnit11p::update(std::string memorizedAmuId, int highestPriority)
{
    this->memorizedAmuId = memorizedAmuId;
    this->highestPriority = highestPriority;
    lastUpdate = simTime();
}
