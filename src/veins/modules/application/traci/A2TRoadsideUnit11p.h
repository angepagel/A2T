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

#pragma once

#include <veins/modules/application/ieee80211p/A2TBaseApplLayer.h>

namespace Veins {

/**
 * @author Ange Pagel
 */

class A2TRoadsideUnit11p : public A2TBaseApplLayer {
public:
    void initialize(int stage) override;

protected:
    // ---------------------- Simulation parameters ----------------------
    /**
     * <!> These parameters are set in the omnetpp.ini file <!>
     */
    simtime_t reinitializationDelay; /* Time after which the RSU resets if it has not been updated (in seconds) */
    // ---------------------- Simulation parameters ----------------------

    bool initialized;           /* Initialization state of the RSU */
    std::string associatedTlId; /* ID of the traffic light associated to the RSU */
    long lastMessageTreeId;     /* Tree ID of the last message received */
    simtime_t lastUpdate;       /* Time of the last update */
    std::string memorizedAmuId; /* Ambulance ID memorized by the RSU */
    int highestPriority;        /* Highest priority recorded */

protected:
    void onWSM(BaseFrame1609_4* wsm) override;

private:
    /**
     * Set up the TraCI interface
     */
    void setTraCI();

    /**
     * Associate a close traffic light with the RSU
     */
    void associateTrafficlight();

    /**
     * Update the RSU information
     * @param memorizedAmuId Ambulance ID memorized
     * @param highestPriority Highest priority recorded
     */
    void update(std::string memorizedAmuId, int highestPriority);
};

} // namespace Veins
