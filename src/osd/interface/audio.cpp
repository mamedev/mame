// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "audio.h"

#include "util/language.h"
#include "util/strformat.h"

#include <algorithm>
#include <iterator>


namespace osd {

namespace {

struct position_name_mapping {
	channel_position m_pos;
	const char *m_name;
};

const position_name_mapping f_position_name_mappings[] = {
		{ channel_position::FC(),       N_p("audio-position", "Front center")    },
		{ channel_position::FL(),       N_p("audio-position", "Front left")      },
		{ channel_position::FR(),       N_p("audio-position", "Front right")     },
		{ channel_position::RC(),       N_p("audio-position", "Rear center")     },
		{ channel_position::RL(),       N_p("audio-position", "Rear left")       },
		{ channel_position::RR(),       N_p("audio-position", "Rear right")      },
		{ channel_position::HC(),       N_p("audio-position", "Headrest center") },
		{ channel_position::HL(),       N_p("audio-position", "Headrest left")   },
		{ channel_position::HR(),       N_p("audio-position", "Headrest right")  },
		{ channel_position::BACKREST(), N_p("audio-position", "Backrest")        },
		{ channel_position::LFE(),      N_p("audio-position", "Subwoofer")       },
		{ channel_position::ONREQ(),    N_p("audio-position", "Auxiliary")       },
		{ channel_position::UNKNOWN(),  N_p("audio-position", "Unknown")         } };

} // anonymous namespace

std::string channel_position::name() const
{
	auto const found = std::find_if(
			std::begin(f_position_name_mappings),
			std::end(f_position_name_mappings),
			[this] (const position_name_mapping &val) { return *this == val.m_pos; });
	if (std::end(f_position_name_mappings) != found)
		return _("audio-position", found->m_name);
	else
		return util::string_format(_("audio-position", "[%1$f %2$f %3$f]"), m_x, m_y, m_z);
}

} // anonymous namespace
