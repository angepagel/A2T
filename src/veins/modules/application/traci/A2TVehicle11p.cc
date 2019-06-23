//
// Copyright (C) 2006-2011 Christoph Sommer <christoph.sommer@uibk.ac.at>
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
#include <veins/modules/application/traci/A2TVehicle11p.h>


using namespace Veins;

Define_Module(Veins::A2TVehicle11p);

void A2TVehicle11p::initialize(int stage)
{
    A2TBaseApplLayer::initialize(stage);
    if (stage == 0)
    {
        // ---------------------- Simulation parameters ----------------------
        // TODO Set these parameters in the omnetpp.ini file
        a2t = true;
        warningDistance = 200; // meters
        broadcastInterval = 1; // seconds
        maxHop = 2;
        minForwardDistance = 100;
        laneChangeDuration = 6.0; // seconds
        // ---------------------- Simulation parameters ----------------------

        lastMessageTreeId = 0;
        lastBroadcastAt = simTime();
        priority = 0;
    }
}

void A2TVehicle11p::onWSM(BaseFrame1609_4* frame)
{
    A2TMessage11p* wsm = check_and_cast<A2TMessage11p*>(frame);

    long messageTreeId = frame->getTreeId();

    if (lastMessageTreeId != messageTreeId) // The message has not been processed yet
    {
        lastMessageTreeId = messageTreeId;
        int wsmPriority = wsm->getPriority();

        if (wsm->getIsFromAmbulance())
        {
            Coord amuCoord(wsm->getAmuX(), wsm->getAmuY());
            double distanceFromAmbulance = traci->getDistance(curPosition, amuCoord, false);

            if (distanceFromAmbulance <= warningDistance)
            {
                std::string amuLaneId = wsm->getAmuLaneId();
                std::string amuRoadId = traci->lane(amuLaneId).getRoadId();
                std::string vehicleType = traciVehicle->getTypeId();

                if (vehicleType != "ambulance" || (vehicleType == "ambulance" && wsmPriority > priority))
                {
                    traciVehicle->changeLane(0, laneChangeDuration);
                    // TODO: This needs to be improved.
                }
            }

            int hopCount = wsm->getHopCount();
            int maxHop = wsm->getMaxHop();

            if (hopCount < maxHop) // Repeat the message if it has not exceeded its maximum number of hops
            {
                Coord senderCoord(wsm->getSenderX(), wsm->getSenderY());
                double distanceFromSender = traci->getDistance(curPosition, senderCoord, false);

                if (distanceFromSender > minForwardDistance)
                {
                    wsm->setSenderAddress(myId);
                    wsm->setSenderX(mobility->getPositionAt(simTime()).x);
                    wsm->setSenderY(mobility->getPositionAt(simTime()).y);
                    wsm->setHopCount(++hopCount);
                    wsm->setSerial(3);
                    scheduleAt(simTime() + uniform(0.01,0.1), wsm->dup());
                }
            }
        }
    }
}

void A2TVehicle11p::handleSelfMsg(cMessage* msg)
{
    if (A2TMessage11p* wsm = dynamic_cast<A2TMessage11p*>(msg)) {
        // send this message on the service channel until the counter is 3 or higher.
        // this code only runs when channel switching is enabled
        sendDown(wsm->dup());
        wsm->setSerial(wsm->getSerial() + 1);
        if (wsm->getSerial() >= 3) {
            // stop service advertisements
            stopService();
            delete (wsm);
        }
        else {
            scheduleAt(simTime() + 1, wsm);
        }
    }
    else {
        A2TBaseApplLayer::handleSelfMsg(msg);
    }
}

void A2TVehicle11p::handlePositionUpdate(cObject* obj)
{
    A2TBaseApplLayer::handlePositionUpdate(obj);

    if (a2t)
    {
        if (traciVehicle->getTypeId() == "ambulance")
        {
            if (priority == 0)
            {
                std::string id = mobility->getExternalId();

                // -----------------------------------
                if      (id == "amu0")  priority = 2;
                else if (id == "amu1")  priority = 2;
                else if (id == "amu2")  priority = 2;
            }

            if (simTime()-lastBroadcastAt >= broadcastInterval)
            {
                double posX = mobility->getPositionAt(simTime()).x;
                double posY = mobility->getPositionAt(simTime()).y;

                A2TMessage11p* wsm = new A2TMessage11p();
                populateWSM(wsm);

                wsm->setIsFromAmbulance(true);
                wsm->setAmuId(mobility->getExternalId().c_str());
                wsm->setAmuLaneId(traciVehicle->getLaneId().c_str());
                wsm->setPriority(priority);
                wsm->setSenderX(posX);
                wsm->setSenderY(posX);
                wsm->setAmuX(posX);
                wsm->setAmuY(posY);
                wsm->setMaxHop(maxHop);
                wsm->setHopCount(1);

                lastBroadcastAt = simTime();
                sendDown(wsm); // Send the message to the lower layer
            }
        }
    }
}
