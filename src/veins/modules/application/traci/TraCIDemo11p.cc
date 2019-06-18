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

#include "veins/modules/application/traci/TraCIDemo11p.h"

#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

using namespace Veins;

Define_Module(Veins::TraCIDemo11p);

void TraCIDemo11p::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        a2t = true;
        priority = 0;
        lastBroadcastAt = simTime();
        lastMessageTreeId = 0;
        warningDistance = 200;
    }
}

void TraCIDemo11p::onWSA(DemoServiceAdvertisment* wsa)
{
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
        currentSubscribedServiceId = wsa->getPsid();
        if (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService(static_cast<Channel>(wsa->getTargetChannel()), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void TraCIDemo11p::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);

    long messageTreeId = frame->getTreeId();

    if (lastMessageTreeId != messageTreeId) // The message has not been processed yet
    {
        lastMessageTreeId = messageTreeId;

        int wsmPriority = wsm->getPriority();
        int hopCount = wsm->getHopCount();
        int maxHop = wsm->getMaxHop();

        if (wsm->getIsFromAmbulance())
        {
            Coord amuCoord(wsm->getAmuX(), wsm->getAmuY());
            double distanceFromAmbulance = traci->getDistance(curPosition, amuCoord, false);

            if (distanceFromAmbulance <= warningDistance)
            {
                double laneChangeDuration = 6.0;

                std::string amuLaneId = wsm->getAmuLaneId();
                std::string amuRoadId = traci->lane(amuLaneId).getRoadId();
                std::string vehicleType = traciVehicle->getTypeId();

                if (vehicleType != "ambulance" || (vehicleType == "ambulance" && wsmPriority > priority))
                {
                    traciVehicle->changeLane(0, laneChangeDuration);
                    // TODO: This needs to be improved.
                }
            }


            if (hopCount < maxHop) // Repeat the message if it has not exceeded its maximum number of hops
            {
                double minForwardDistance = 200;

                Coord senderCoord(wsm->getSenderX(), wsm->getSenderY());
                double distanceFromSender = traci->getDistance(curPosition, senderCoord, false);

                if (distanceFromSender > minForwardDistance)
                {
                    wsm->setSenderAddress(myId);
                    wsm->setSenderX(mobility->getPositionAt(simTime()).x);
                    wsm->setSenderY(mobility->getPositionAt(simTime()).y);
                    wsm->setHopCount(++hopCount);
                    wsm->setSerial(3);
                    scheduleAt(simTime() + uniform(0.01,0.3), wsm->dup());
                }
            }
        }
    }
}

void TraCIDemo11p::handleSelfMsg(cMessage* msg)
{
    if (TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(msg)) {
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
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void TraCIDemo11p::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    if (a2t)
    {
        simtime_t broadcastInterval = 1; // Interval between the broadcast of a new message (in seconds)
        int maxHop = 2; // Maximum number of hops of a message

        if (traciVehicle->getTypeId() == "ambulance")
        {

            if (priority == 0)
            {
                std::string id = mobility->getExternalId();

                if      (id == "amu0")  priority = 2;
                else if (id == "amu1")  priority = 1;
            }

            if (simTime()-lastBroadcastAt >= broadcastInterval)
            {
                double posX = mobility->getPositionAt(simTime()).x;
                double posY = mobility->getPositionAt(simTime()).y;

                TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
                populateWSM(wsm);

                wsm->setIsFromAmbulance(true);
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
