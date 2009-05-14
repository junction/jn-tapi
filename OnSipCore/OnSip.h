#ifndef ONSIP_H
#define ONSIP_H


#define ONSIP_AUTH_NODE  "authorize-plain"
#define ONSIP_AUTH_COMMAND  "commands.auth.xmpp.onsip.com"

#define ONSIP_PUBSUB_ACTIVECALLS  "pubsub.active-calls.xmpp.onsip.com"

#define ONSIP_ACTIVECALLS_EVENT_XMLNS  "onsip:active-calls"

#define ONSIP_ACTIVECALLS_COMMAND "commands.active-calls.xmpp.onsip.com"

#include "gloox.h"
#include "client.h"
#include "tag.h"
#include "jid.h"

#include "Utils.h"
#include "Threads.h"

using namespace gloox;

#include <string.h>
using namespace std;

#endif