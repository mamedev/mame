// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "audio/cheekyms.h"
#include "audio/nl_cheekyms.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(CHEEKY_MOUSE_AUDIO, cheekyms_audio_device, "cheekyms_audio", "Cheeky Mouse Sound Board")


cheekyms_audio_device::cheekyms_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CHEEKY_MOUSE_AUDIO, tag, owner, clock)
	, m_mute(*this, "sound_nl:mute")
	, m_cheese(*this, "sound_nl:cheese")
	, m_music(*this, "sound_nl:music")
	, m_mouse(*this, "sound_nl:mouse")
	, m_hammer(*this, "sound_nl:hammer")
	, m_pest(*this, "sound_nl:pest")
	, m_mouse_dies(*this, "sound_nl:mouse_dies")
	, m_pest_dies(*this, "sound_nl:pest_dies")
	, m_coin_extra(*this, "sound_nl:coin_extra")
{
}


WRITE_LINE_MEMBER(cheekyms_audio_device::mute_w)        { m_mute->write_line(state);       }
WRITE_LINE_MEMBER(cheekyms_audio_device::cheese_w)      { m_cheese->write_line(state);     }
WRITE_LINE_MEMBER(cheekyms_audio_device::music_w)       { m_music->write_line(state);      }
WRITE_LINE_MEMBER(cheekyms_audio_device::mouse_w)       { m_mouse->write_line(state);      }
WRITE_LINE_MEMBER(cheekyms_audio_device::hammer_w)      { m_hammer->write_line(state);     }
WRITE_LINE_MEMBER(cheekyms_audio_device::pest_w)        { m_pest->write_line(state);       }
WRITE_LINE_MEMBER(cheekyms_audio_device::mouse_dies_w)  { m_mouse_dies->write_line(state); }
WRITE_LINE_MEMBER(cheekyms_audio_device::pest_dies_w)   { m_pest_dies->write_line(state);  }
WRITE_LINE_MEMBER(cheekyms_audio_device::coin_extra_w)  { m_coin_extra->write_line(state); }


MACHINE_CONFIG_START(cheekyms_audio_device::device_add_mconfig)
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("sound_nl", NETLIST_SOUND, 48000)
	MCFG_NETLIST_SETUP(cheekyms)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NETLIST_LOGIC_INPUT("sound_nl", "mute",       "I_MUTE.IN",       0)
	MCFG_NETLIST_LOGIC_INPUT("sound_nl", "cheese",     "I_CHEESE.IN",     0)
	MCFG_NETLIST_LOGIC_INPUT("sound_nl", "music",      "I_MUSIC.IN",      0)
	MCFG_NETLIST_LOGIC_INPUT("sound_nl", "mouse",      "I_MOUSE.IN",      0)
	MCFG_NETLIST_LOGIC_INPUT("sound_nl", "hammer",     "I_HAMMER.IN",     0)
	MCFG_NETLIST_LOGIC_INPUT("sound_nl", "pest",       "I_PEST.IN",       0)
	MCFG_NETLIST_LOGIC_INPUT("sound_nl", "mouse_dies", "I_MOUSE_DIES.IN", 0)
	MCFG_NETLIST_LOGIC_INPUT("sound_nl", "pest_dies",  "I_PEST_DIES.IN",  0)
	MCFG_NETLIST_LOGIC_INPUT("sound_nl", "coin_extra", "I_COIN_EXTRA.IN", 0)

	MCFG_NETLIST_STREAM_OUTPUT("sound_nl", 0, "VR1.2")
	MCFG_NETLIST_ANALOG_MULT_OFFSET(30000.0 * 10.0, 0.0) // FIXME: no clue what numbers to use here
MACHINE_CONFIG_END


void cheekyms_audio_device::device_start()
{
}
