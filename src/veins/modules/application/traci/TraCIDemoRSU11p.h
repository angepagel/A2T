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

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"

namespace Veins {

/**
 * Small RSU Demo using 11p
 */
class TraCIDemoRSU11p : public DemoBaseApplLayer {
protected:
    bool initialized;
    std::string associatedTlId; /* ID of the traffic light associated to the RSU */
    long lastMessageTreeId; /* Tree ID of the last message received */
    simtime_t reinitializationDelay; /* Time before the highest priority known is reinitialized at 0 */
    std::string lastAmuId;
    simtime_t lastUpdate;
    int highestPriority;
protected:
    void initialize(int stage);
    void onWSM(BaseFrame1609_4* wsm) override;
};

} // namespace Veins
