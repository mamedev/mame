// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "audio.h"

const osd::channel_position osd::channel_position::UNKNOWN(0.0, 0.0,  0.0);
const osd::channel_position osd::channel_position::ONREQ  (0.0, 0.0, 10.0);
const osd::channel_position osd::channel_position::LFE    (0.0, 0.0, 11.0);

bool osd::channel_position::is_lfe() const     { return *this == LFE; }
bool osd::channel_position::is_onreq() const   { return *this == ONREQ; }
bool osd::channel_position::is_unknown() const { return *this == UNKNOWN; }

const osd::channel_position osd::channel_position::FC      ( 0.0,  0.0,  1.0);
const osd::channel_position osd::channel_position::FL      (-0.2,  0.0,  1.0);
const osd::channel_position osd::channel_position::FR      ( 0.2,  0.0,  1.0);
const osd::channel_position osd::channel_position::RC      ( 0.0,  0.0, -0.5);
const osd::channel_position osd::channel_position::RL      (-0.2,  0.0, -0.5);
const osd::channel_position osd::channel_position::RR      ( 0.2,  0.0, -0.5);
const osd::channel_position osd::channel_position::HC      ( 0.0,  0.0, -0.1);
const osd::channel_position osd::channel_position::HL      (-0.1,  0.0, -0.1);
const osd::channel_position osd::channel_position::HR      ( 0.1,  0.0, -0.1);
const osd::channel_position osd::channel_position::BACKREST( 0.0, -0.2,  0.1);

const osd::detail::position_name_mapping osd::detail::position_name_mappings[] = {
  { channel_position::FC, "Front center"    },
  { channel_position::FL, "Front left"      },
  { channel_position::FR, "Front right"     },
  { channel_position::RC, "Rear center"     },
  { channel_position::RL, "Rear left"       },
  { channel_position::RR, "Rear right"      },
  { channel_position::HC, "Headrest center" },
  { channel_position::HL, "Headrest left"   },
  { channel_position::HR, "Headrest right"  },
  { channel_position::BACKREST, "Backrest"  },
  { channel_position::LFE, "Subwoofer"      },
  { channel_position::ONREQ, "Auxiliary"    },
  { channel_position::UNKNOWN, "Unknown"    }
};

std::string osd::channel_position::name() const
{
	for(unsigned int i=0; i != sizeof(detail::position_name_mappings)/sizeof(detail::position_name_mappings[0]); i++)
		if(*this == detail::position_name_mappings[i].m_pos)
			return detail::position_name_mappings[i].m_name;
	return util::string_format("[%f %f %f]", m_x, m_y, m_z);
}

