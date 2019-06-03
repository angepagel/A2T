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
        lastMessageSentAt = simTime();
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
    findHost()->getDisplayString().updateWith("r=16,green");

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
    if (traciVehicle->getTypeId() == "passenger")
    {
        EV << "======================= PASSENGER INFORMATION =======================" << endl;
        EV << "WSM received by vehicle id:" << mobility->getNode()->getId() << "." << endl;
        EV << "Vehicle driving on road id:" << mobility->getRoadId() << "." << endl;

        // Is the message emitted by an AMU ?
        if (wsm->getAmbulance())
        {
            // Euclidian distance from the AMU
            double distance = sqrt(
                    pow((mobility->getCurrentPosition().x - wsm->getPosx()), 2)
                    + pow((mobility->getCurrentPosition().y - wsm->getPosy()), 2)
                );

            EV << "<!> This message comes from an AMU." << endl;
            EV << "Position of the AMU: x:" << wsm->getPosx() << " y:" << wsm->getPosy() << "." << endl;
            EV << "Distance from the AMU: " << distance << "." << endl;

            // Is the vehicle in the communication radius of the AMU ?
            if (distance < wsm->getRadius())
            {
                EV << "<!> This vehicle is in the AMU communication radius." << endl;

                // Is the vehicle on the same road as the AMU ?
                if (wsm->getRoadId() == mobility->getRoadId())
                {
                    EV << "<!> This vehicle is on the same road as the AMU." << endl;
                    EV << "Switching to lowest speed lane..." << endl;

                    findHost()->getDisplayString().updateWith("r=16,blue");

                    // Switch to lowest speed lane
                    traciVehicle->changeLane(0, 1000);
                }
            }
        }
    }
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
    int broadcastInterval = 10; // How would this parameter affect the results of the simulation ?

    if (traciVehicle->getTypeId() == "ambulance")
    {
        if (simTime() - lastMessageSentAt >= broadcastInterval) {

            // Creation of a new WSM object
            WaveShortMessage* wsm = new WaveShortMessage();
            populateWSM(wsm);

            int commRadius = 20; // How would this parameter affect the results of the simulation ?

            // Set the WSM informations
            wsm->setAmbulance(true);
            wsm->setLaneId(traciVehicle->getLaneId().c_str());
            wsm->setPosx(mobility->getCurrentPosition().x);
            wsm->setPosy(mobility->getCurrentPosition().y);
            wsm->setRadius(commRadius);

            EV << "======================= AMBULANCE BROADCAST =======================" << endl;
            EV << "WSM sent from AMU id:" << wsm->getSenderAddress() << "." << endl;
            EV << "WSM position information set: x:" << wsm->getPosx() << " y:" << wsm->getPosy() << "." << endl;

            lastMessageSentAt = simTime();
            sendDown(wsm); // Send the message to the lower layer
        }
    }
    // -------------------------- A2T --------------------------

}
