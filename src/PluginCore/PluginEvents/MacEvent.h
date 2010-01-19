/**********************************************************\
Original Author: Richard Bateman (taxilian)

Created:    Oct 19, 2009
License:    Dual license model; choose one of two:
            Eclipse Public License - Version 1.0
            http://www.eclipse.org/legal/epl-v10.html
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 PacketPass, Inc and the Firebreath development team
\**********************************************************/

#ifndef H_FB_EVENTS_MACEVENT
#define H_FB_EVENTS_MACEVENT

#ifndef XP_MACOSX
#define EventRecord NPEvent
#else
#include <Carbon/Carbon.h>
#endif

#include "PluginEvent.h"

namespace FB {

    class MacEvent : public PluginEvent
    {
    public:
        MacEvent(EventRecord *src)
        {
            memcpy(&msg, src, sizeof(EventRecord));
        }

    public:
        EventRecord msg;
    };
};

#endif
