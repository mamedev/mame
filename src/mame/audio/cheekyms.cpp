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


void cheekyms_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(cheekyms))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:mute",       "I_MUTE.IN",       0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:cheese",     "I_CHEESE.IN",     0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:music",      "I_MUSIC.IN",      0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:mouse",      "I_MOUSE.IN",      0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:hammer",     "I_HAMMER.IN",     0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:pest",       "I_PEST.IN",       0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:mouse_dies", "I_MOUSE_DIES.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:pest_dies",  "I_PEST_DIES.IN",  0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:coin_extra", "I_COIN_EXTRA.IN", 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "VR1.2").set_mult_offset(30000.0 * 10.0, 0.0);
}


void cheekyms_audio_device::device_start()
{
}
