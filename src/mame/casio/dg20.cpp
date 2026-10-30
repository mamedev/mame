// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*
    Casio Digital Guitars (DG-10 / DG-20)

    TODO:
    - PCM drums (MSM6294)
    - effects section (chorus, flanger, distortion)
    - implement uPD931 vibrato register
    - reasonable controls/layout somehow
*/

#include "emu.h"

#include "bus/midi/midioutport.h"
#include "cpu/upd7810/upd7810.h"
#include "sound/flt_biquad.h"
#include "sound/flt_rc.h"
#include "sound/upd931.h"

#include "speaker.h"

namespace {

class dg20_state : public driver_device
{
public:
	dg20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_upd931(*this, "upd931")
		, m_filter_bq(*this, "filter_bq")
		, m_filter_rc(*this, "filter_rc")
		, m_keys(*this, "KC%u", 0)
		, m_frets(*this, "GC%u", 0)
	{ }

	void dg10(machine_config &config) ATTR_COLD;
	void dg20(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(string_set_w);

	DECLARE_INPUT_CHANGED_MEMBER(effects_w);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void keys_w(offs_t offset, u8) { m_key_sel = offset; }
	u8 keys_r();

	void frets_w(offs_t offset, u8) { m_fret_sel = offset; }
	template <int Group> u8 frets_r();

	u8 strings_r() { return m_strings; }
	void strings_clr_w(offs_t offset, u8) { m_strings &= offset; }

	void filter_w(u8 data);

	void maincpu_map(address_map &map) ATTR_COLD;

	required_device<upd78c10_device> m_maincpu;
	required_device<upd931_device> m_upd931;
	required_device<filter_biquad_device> m_filter_bq;
	required_device<filter_rc_device> m_filter_rc;

	optional_ioport_array<5> m_keys;
	required_ioport_array<6> m_frets;

	u8 m_key_sel, m_fret_sel;
	u8 m_strings;
};

/**************************************************************************/
static INPUT_PORTS_START(dg10)
	PORT_START("PA")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("upd931", FUNC(upd931_device::db_w))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("upd931", FUNC(upd931_device::i1_w))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("upd931", FUNC(upd931_device::i2_w))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("upd931", FUNC(upd931_device::i3_w))
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_OUTPUT ) // upd931 reset

	PORT_START("PB")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_OUTPUT ) // TODO: LEDs
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM ) // TODO: APO enable
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OUTPUT ) // msm6294 reset
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OUTPUT ) // TODO: APO

	PORT_START("PC")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) // low = DG-10
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dg20_state::effects_w), 0)

	PORT_START("STRINGS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("String 1") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dg20_state::string_set_w), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("String 2") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dg20_state::string_set_w), 1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("String 3") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dg20_state::string_set_w), 2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("String 4") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dg20_state::string_set_w), 3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("String 5") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dg20_state::string_set_w), 4)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("String 6") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(dg20_state::string_set_w), 5)

	PORT_START("GC0")
	PORT_BIT( 0x000003, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 20")
	PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 19")
	PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 18")
	PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 17")
	PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 16")
	PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 15")
	PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 14")
	PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 13")
	PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 12")
	PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 11")
	PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 10")
	PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 9")
	PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 8")
	PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 7")
	PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 6")
	PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 5")
	PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 4")
	PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 3")
	PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 2")
	PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 1 Fret 1")
	PORT_BIT( 0xc00000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GC1")
	PORT_BIT( 0x000003, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 20")
	PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 19")
	PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 18")
	PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 17")
	PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 16")
	PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 15")
	PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 14")
	PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 13")
	PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 12")
	PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 11")
	PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 10")
	PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 9")
	PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 8")
	PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 7")
	PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 6")
	PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 5")
	PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 4")
	PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 3")
	PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 2")
	PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 2 Fret 1")
	PORT_BIT( 0xc00000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GC2")
	PORT_BIT( 0x000003, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 20")
	PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 19")
	PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 18")
	PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 17")
	PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 16")
	PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 15")
	PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 14")
	PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 13")
	PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 12")
	PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 11")
	PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 10")
	PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 9")
	PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 8")
	PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 7")
	PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 6")
	PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 5")
	PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 4")
	PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 3")
	PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 2")
	PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 3 Fret 1")
	PORT_BIT( 0xc00000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GC3")
	PORT_BIT( 0x000003, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 20")
	PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 19")
	PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 18")
	PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 17")
	PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 16")
	PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 15")
	PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 14")
	PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 13")
	PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 12")
	PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 11")
	PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 10")
	PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 9")
	PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 8")
	PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 7")
	PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 6")
	PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 5")
	PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 4")
	PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 3")
	PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 2")
	PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 4 Fret 1")
	PORT_BIT( 0xc00000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GC4")
	PORT_BIT( 0x000003, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 20")
	PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 19")
	PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 18")
	PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 17")
	PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 16")
	PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 15")
	PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 14")
	PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 13")
	PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 12")
	PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 11")
	PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 10")
	PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 9")
	PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 8")
	PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 7")
	PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 6")
	PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 5")
	PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 4")
	PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 3")
	PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 2")
	PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 5 Fret 1")
	PORT_BIT( 0xc00000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GC5")
	PORT_BIT( 0x000003, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 20")
	PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 19")
	PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 18")
	PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 17")
	PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 16")
	PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 15")
	PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 14")
	PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 13")
	PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 12")
	PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 11")
	PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 10")
	PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 9")
	PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 8")
	PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 7")
	PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 6")
	PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 5")
	PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 4")
	PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 3")
	PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 2")
	PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("String 6 Fret 1")
	PORT_BIT( 0xc00000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KC1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 6")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KC2")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Sustain / Reverb")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Solo")
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone Select")

	PORT_START("KC3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 5")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm 6")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Rhythm Select")

	PORT_START("KC4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tempo Up")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tempo Down")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Transpose Up")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Transpose Down")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Start / Stop")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Synchro / Fill-In")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Mute")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END

/**************************************************************************/
static INPUT_PORTS_START(dg20)
	PORT_INCLUDE(dg10)

	PORT_MODIFY("PC")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM ) // high = DG-20
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("Foot Switch")

	PORT_START("KC0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Drum Pad 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Drum Pad 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Drum Pad 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Drum Pad 4")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("KC1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 7")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 8")

	PORT_MODIFY("KC2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 9")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Tone 10")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("MIDI") PORT_TOGGLE
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END

/**************************************************************************/
void dg20_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x4000).rom();
	map(0x8000, 0x87ff).w(FUNC(dg20_state::keys_w));
	map(0x8800, 0x8fff).w(FUNC(dg20_state::frets_w));
	map(0x9000, 0x97ff).w(FUNC(dg20_state::strings_clr_w));
	map(0x9800, 0x9fff).r(FUNC(dg20_state::keys_r));
	map(0xa000, 0xa7ff).r(FUNC(dg20_state::strings_r));
	map(0xa800, 0xafff).r(FUNC(dg20_state::frets_r<0>));
	map(0xb000, 0xb7ff).r(FUNC(dg20_state::frets_r<1>));
	map(0xb800, 0xbfff).r(FUNC(dg20_state::frets_r<2>));
}

/**************************************************************************/
void dg20_state::dg10(machine_config &config)
{
	UPD78C10(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &dg20_state::maincpu_map);
	m_maincpu->pa_out_cb().set_ioport("PA");
	m_maincpu->pb_in_cb().set_ioport("PB");
	m_maincpu->pb_out_cb().set_ioport("PB");
	m_maincpu->pc_in_cb().set_ioport("PC");
	m_maincpu->pc_out_cb().set_ioport("PC");

	SPEAKER(config, "speaker").front_center();

	UPD931(config, m_upd931, 4'946'864);
	m_upd931->filter_cb().set(FUNC(dg20_state::filter_w));
	m_upd931->add_route(0, m_filter_bq, 2.0);

	FILTER_BIQUAD(config, m_filter_bq).add_route(0, m_filter_rc, 1.0);
	FILTER_RC(config, m_filter_rc).add_route(0, "speaker", 1.0);

	// MSM6294(config, "pcm", 256'000);
}

/**************************************************************************/
void dg20_state::dg20(machine_config &config)
{
	dg10(config);

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
	m_maincpu->txd_func().set("mdout", FUNC(midi_port_device::write_txd));
}

/**************************************************************************/
void dg20_state::machine_start()
{
	m_key_sel = m_fret_sel = 0xff;
	m_strings = 0;

	save_item(NAME(m_key_sel));
	save_item(NAME(m_fret_sel));
	save_item(NAME(m_strings));
}

/**************************************************************************/
void dg20_state::machine_reset()
{
}

/**************************************************************************/
u8 dg20_state::keys_r()
{
	u8 data = 0;
	for (int i = 0; i < m_keys.size(); i++)
		if (!BIT(m_key_sel, i))
			data |= m_keys[i].read_safe(0);

	return data;
}

/**************************************************************************/
template <int Group>
u8 dg20_state::frets_r()
{
	u32 data = 0;
	for (int i = 0; i < m_frets.size(); i++)
		if (!BIT(m_fret_sel, i))
			data |= m_frets[i]->read();

	return data >> (8 * Group);
}

/**************************************************************************/
INPUT_CHANGED_MEMBER(dg20_state::string_set_w)
{
	if (!oldval && newval)
		m_strings |= (1 << param);
}

/**************************************************************************/
void dg20_state::filter_w(u8 data)
{
	const double rc_c = BIT(data, 0) ? CAP_N(22) : 0;
	const double bq_c1 = BIT(data, 1) ? CAP_N(27) : 0;
	const double bq_c2 = BIT(data, 2) ? CAP_N(1) : 0;

	m_filter_bq->opamp_sk_lowpass_modify(RES_K(33), RES_K(33), RES_M(999.99), RES_R(0.001), CAP_N(5.6) + bq_c1, CAP_P(10) + bq_c2);
	m_filter_rc->filter_rc_set_RC(filter_rc_device::LOWPASS, RES_K(10), 0, 0, CAP_N(3.3) + rc_c);
}

/**************************************************************************/
INPUT_CHANGED_MEMBER(dg20_state::effects_w)
{
	/* TODO:
	bit 0 low:  enable chorus, reduce non-chorus volume
	bit 1 low:  faster chorus LFO speed
	bit 2 high: reduce total volume
	bit 3 low:  flanger (DG-20 only)
	bit 4 high: distortion
	see the service manual (section "Preset Effects in Each Tone") for a table of which tones use which effects.
	*/
	logerror("effects_w: %02x\n", newval);
}

/**************************************************************************/
ROM_START( dg20 )
	ROM_REGION(0x4000, "maincpu", 0) // both halves are identical, A14 is connected to +5V
	ROM_LOAD( "upd23c256eac-055.bin", 0x0000, 0x4000, CRC(c53d90da) SHA1(7d8b33de4bf01f598cc57b8c318c84a99ee26502) )
	ROM_CONTINUE(0x0000, 0x4000)

	ROM_REGION(0x4000, "pcm", 0)
	ROM_LOAD( "msm6294-06.bin", 0x0000, 0x4000, NO_DUMP )
ROM_END

#define rom_dg10 rom_dg20

} // anonymous namespace

SYST( 1987, dg20, 0,    0, dg20, dg20, dg20_state, empty_init, "Casio", "DG-20 Digital Guitar", MACHINE_NOT_WORKING )
SYST( 1987, dg10, dg20, 0, dg10, dg10, dg20_state, empty_init, "Casio", "DG-10 Digital Guitar", MACHINE_NOT_WORKING )
