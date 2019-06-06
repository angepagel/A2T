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

Define_Module(TraCIDemo11p);

void TraCIDemo11p::initialize(int stage) {
    BaseWaveApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        lastDroveAt = simTime();
        currentSubscribedServiceId = -1;
        // -------------------------- A2T --------------------------
        a2t = true;
        priority = 0;
        lastBroadcastAt = simTime();
        lastMessageTreeId = 0;
        // -------------------------- A2T --------------------------
    }
}

void TraCIDemo11p::onWSA(WaveServiceAdvertisment* wsa) {
    if (currentSubscribedServiceId == -1) {
        mac->changeServiceChannel(wsa->getTargetChannel());
        currentSubscribedServiceId = wsa->getPsid();
        if  (currentOfferedServiceId != wsa->getPsid()) {
            stopService();
            startService((Channels::ChannelNumber) wsa->getTargetChannel(), wsa->getPsid(), "Mirrored Traffic Service");
        }
    }
}

void TraCIDemo11p::onWSM(WaveShortMessage* wsm) {
    // -------------------------- Veins example original code --------------------------
    /*
    if (mobility->getRoadId()[0] != ':') traciVehicle->changeRoute(wsm->getWsmData(), 9999);
    if (!sentMessage) {
        sentMessage = true;
        //repeat the received traffic update once in 2 seconds plus some random delay
        wsm->setSenderAddress(myId);
        wsm->setSerial(3);
        scheduleAt(simTime() + 2 + uniform(0.01,0.2), wsm->dup());
    }
    */
    // -------------------------- Veins example original code --------------------------

    // -------------------------- A2T --------------------------
    long messageTreeId = wsm->getTreeId();

    EV << "<!> WSM received, tree id: " << messageTreeId << endl;

    if (lastMessageTreeId != messageTreeId) // The message has not been processed yet
    {
        EV << "<!> This message has not been received yet." << endl;
        EV << "<!> Processing..." << endl;

        lastMessageTreeId = messageTreeId;

        int amuPriority = wsm->getPriority();
        int hopCount = wsm->getHopCount();
        int maxHop = wsm->getMaxHop();

        if (wsm->getIsFromAmbulance())
        {
            EV << "<!> This message comes from an ambulance." << endl;

            std::string amuLaneId = wsm->getAmuLaneId();
            std::string amuRoadId = traci->lane(amuLaneId).getRoadId();

            if (amuRoadId == mobility->getRoadId()) // Is the vehicle on the same road as the AMU ?
            {
                // No action is taken if the vehicle is also an AMU
                if (traciVehicle->getTypeId() != "ambulance")
                {
                    EV << "Clearing the way and switching to the lowest speed lane." << endl;
                    traciVehicle->changeLane(0, 10000); // 10000ms = 10s
                }

                // TODO New lane changing system
                /* Switching to lane 0 may block the AMU as this lane (supposedly the slowest)
                 * is possibly the only lane allowing to turn on the next road in AMU's path.
                 */
            }

            if (hopCount < maxHop) // Repeat the message if it has not exceeded its maximum number of hops
            {
                double minForwardDistance = 200;

                Coord senderCoord(wsm->getSenderX(), wsm->getSenderY());
                double distanceFromSender = traci->getDistance(curPosition, senderCoord, false);

                if (distanceFromSender > minForwardDistance)
                {
                    wsm->setSenderAddress(myId);
                    wsm->setSenderX(mobility->getCurrentPosition().x);
                    wsm->setSenderY(mobility->getCurrentPosition().y);
                    wsm->setHopCount(++hopCount);
                    wsm->setSerial(3);
                    scheduleAt(simTime() + uniform(0.01,0.2), wsm->dup());
                }
            }
        }
    }
    else EV << "<!> This message has already been processed." << endl;
    // -------------------------- A2T --------------------------
}

void TraCIDemo11p::handleSelfMsg(cMessage* msg) {
    if (WaveShortMessage* wsm = dynamic_cast<WaveShortMessage*>(msg)) {
        //send this message on the service channel until the counter is 3 or higher.
        //this code only runs when channel switching is enabled
        sendDown(wsm->dup());
        wsm->setSerial(wsm->getSerial() +1);
        if (wsm->getSerial() >= 3) {
            //stop service advertisements
            stopService();
            delete(wsm);
        }
        else {
            scheduleAt(simTime()+1, wsm);
        }
    }
    else {
        BaseWaveApplLayer::handleSelfMsg(msg);
    }
}

void TraCIDemo11p::handlePositionUpdate(cObject* obj) {
    BaseWaveApplLayer::handlePositionUpdate(obj);

    // -------------------------- Veins example original code --------------------------
    /*
    // stopped for for at least 10s?
    if (mobility->getSpeed() < 1) {
        if (simTime() - lastDroveAt >= 10 && sentMessage == false) {
            findHost()->getDisplayString().updateWith("r=16,red");
            sentMessage = true;

            WaveShortMessage* wsm = new WaveShortMessage();
            populateWSM(wsm);
            wsm->setWsmData(mobility->getRoadId().c_str());

            //host is standing still due to crash
            if (dataOnSch) {
                startService(Channels::SCH2, 42, "Traffic Information Service");
                //started service and server advertising, schedule message to self to send later
                scheduleAt(computeAsynchronousSendingTime(1,type_SCH),wsm);
            }
            else {
                //send right away on CCH, because channel switching is disabled
                sendDown(wsm);
            }
        }
    }
    else {
        lastDroveAt = simTime();
    }
    */
    // -------------------------- Veins example original code --------------------------


    // -------------------------- A2T --------------------------
    if (a2t) // Are A2T communications enabled ?
    {
        int broadcastInterval = 3; /* Interval between the broadcast of a new message (in seconds) */
        int maxHop = 2;

        // TODO How would these parameter affect the results of the simulation ?

        /* A WSM has a typical range of 300 meters.
         * Maximum range of the message according to the maximum number of hops of the message :
         * range(1) = 0.3km
         * range(2) = 0.6km
         * range(3) = 0.9km
         * range(4) = 1.2km
         * etc.
         */

        if (traciVehicle->getTypeId() == "ambulance")
        {

            if (priority == 0) // Generate a random priority if not set
            {
                // TODO Generate random priority
            }


            if (simTime()-lastBroadcastAt >= broadcastInterval)
            {
                double posX = mobility->getCurrentPosition().x;
                double posY = mobility->getCurrentPosition().y;

                WaveShortMessage* wsm = new WaveShortMessage();
                populateWSM(wsm);

                wsm->setIsFromAmbulance(true);
                wsm->setAmuLaneId(traciVehicle->getLaneId().c_str());
                wsm->setSenderX(posX);
                wsm->setSenderY(posX);
                wsm->setAmuX(posX);
                wsm->setAmuY(posY);
                wsm->setMaxHop(maxHop);
                wsm->setHopCount(1);

                EV << "<!> Ambulance broadcasts a WSM." << endl;
                EV << "<!> WSM sent, tree id: " << wsm->getTreeId() << endl;

                lastBroadcastAt = simTime();
                sendDown(wsm); // Send the message to the lower layer
            }
        }
    }
    // -------------------------- A2T --------------------------
}
