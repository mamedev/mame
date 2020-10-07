// license:CC0
// copyright-holders:Aaron Giles
#ifndef MAME_AUDIO_NL_FROGS_H
#define MAME_AUDIO_NL_FROGS_H

#pragma once

#define FROGS_TEST_INDEPENDENT_NETLISTS (0)

#if (FROGS_TEST_INDEPENDENT_NETLISTS)

NETLIST_EXTERNAL(frogs_jump)
NETLIST_EXTERNAL(frogs_tongue)
NETLIST_EXTERNAL(frogs_hop)
NETLIST_EXTERNAL(frogs_capture)
NETLIST_EXTERNAL(frogs_splash)
NETLIST_EXTERNAL(frogs_fly)
NETLIST_EXTERNAL(frogs_mixer)

#else

NETLIST_EXTERNAL(frogs)

#endif

#endif // MAME_AUDIO_NL_FROGS_H
