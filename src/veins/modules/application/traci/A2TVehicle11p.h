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

#pragma once

#include <veins/modules/application/ieee80211p/A2TBaseApplLayer.h>

namespace Veins {

/**
 * @author Ange Pagel
 */

class A2TVehicle11p : public A2TBaseApplLayer {
public:
    void initialize(int stage) override;

protected:
    // ---------------------- Simulation parameters ----------------------
    // TODO Set these parameters in the omnetpp.ini file
    bool a2t;                       /* Enable or disable A2T communications */
    simtime_t broadcastInterval;    /* [AMU] Interval between messages broadcast (in seconds) */
    int maxHop;                     /* Maximum number of hops of a message */
    double minForwardDistance;      /* Minimum distance from which a vehicle is selected to forward the message (in meters) */
    int warningDistance;            /* [A2V] Distance before which a vehicle will react when an ambulance approaches (in meters) */
    double laneChangeDuration;      /* [A2V] Duration for which a vehicle changes lanes (in seconds) */
    // ---------------------- Simulation parameters ----------------------

    long lastMessageTreeId;     /* Tree ID of the last message received */
    simtime_t lastBroadcastAt;  /* [AMU] Simulation time of the last message broadcasted */
    int priority;               /* Priority of the vehicle */

protected:
    void onWSM(BaseFrame1609_4* wsm) override;
    void handleSelfMsg(cMessage* msg) override;
    void handlePositionUpdate(cObject* obj) override;
};

} // namespace Veins
