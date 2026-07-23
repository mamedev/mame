// license:BSD-3-Clause
// copyright-holders:Devin Acker

/***************************************************************************
    Akai XR10 16-bit drum machine

    Sound hardware consists of:
    - 2x TMP82C37 DMA controllers connected to 1MB of sample ROM
    - TMW4007 gate array generating sample DMA requests and bank switching
    - TMW4008 gate array reading ROM DMA output, applying envelopes and mixing.

    Currently no sound support at all because the sample ROMs are undumped.
    A number of the original preset rhythms were originally stored in RAM and are probably lost.

    On boot, hold 0+9 on the keypad to initialize the backup RAM.
    Hold 3+4 for a simple test mode which plays a looping sample on all 8 voices and blinks the
    LEDs in response to button presses.
***************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/am9517a.h"
#include "machine/nvram.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class xr10_state : public driver_device
{
public:
	xr10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dma(*this, "dma%u", 0)
		, m_lcdc(*this, "lcdc")
		, m_keys(*this, "KEY%u", 0)
	{}

	void xr10(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;

	HD44780_PIXEL_UPDATE(lcd_update);

	u8 keys_r();
	void keys_w(u8 data) { m_key_sel = data; }

	required_device<upd78c10_device> m_maincpu;
	required_device_array<am9517a_device, 2> m_dma;
	required_device<hd44780_device> m_lcdc;

	required_ioport_array<6> m_keys;

	u8 m_midi_rx;
	u8 m_key_sel;
};

/**************************************************************************/
void xr10_state::machine_start()
{
	m_midi_rx = 1;
	m_key_sel = 0;

	save_item(NAME(m_midi_rx));
	save_item(NAME(m_key_sel));
}

/**************************************************************************/
void xr10_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	// 7f00: TMW4007 gate array (also generates DMA chip selects)
	map(0x7f01, 0x7f08).nopw(); // per-voice ROM bank select
	map(0x7f10, 0x7f1f).nopw(); // voice timer rate (16-bit, big endian)
	map(0x7f40, 0x7f4f).rw(m_dma[0], FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x7f60, 0x7f6f).rw(m_dma[1], FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	// 7f80: TMW4008 gate array
	map(0x7f80, 0x7faf).nopw(); // voice volume (16-bit, big endian, 3 values per voice for left/right/effect send)
	map(0x7fb0, 0x7fbf).nopw(); // voice envelope rate (16-bit, big endian)
	map(0x7fc2, 0x7fc3).nopw(); // key on/off (same value written to both addresses?)
	map(0x7fc5, 0x7fc5).nopw(); // LED outputs
	map(0x8000, 0xffff).ram().share("nvram");
}

/**************************************************************************/
void xr10_state::xr10(machine_config &config)
{
	UPD78C10(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &xr10_state::mem_map);
	m_maincpu->pa_in_cb().set(FUNC(xr10_state::keys_r));
	m_maincpu->pb_out_cb().set(FUNC(xr10_state::keys_w));
	m_maincpu->pb_out_cb().append(m_lcdc, FUNC(hd44780_device::db_w));
	m_maincpu->pb_in_cb().set(m_lcdc, FUNC(hd44780_device::db_r));
	// TODO: PC4 = disable DMA requests?
	m_maincpu->pc_out_cb().set(m_lcdc, FUNC(hd44780_device::e_w)).bit(5);
	m_maincpu->pc_out_cb().append(m_lcdc, FUNC(hd44780_device::rw_w)).bit(6);
	m_maincpu->pc_out_cb().append(m_lcdc, FUNC(hd44780_device::rs_w)).bit(7);
	m_maincpu->an0_func().set_ioport("AN0");
	m_maincpu->an1_func().set_ioport("AN1");
	m_maincpu->an2_func().set_ioport("AN2");
	m_maincpu->an7_func().set_ioport("AN7");

	NVRAM(config, "nvram");

	// MIDI
	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set([this] (int state) { m_midi_rx = state; });
	m_maincpu->rxd_func().set([this] () { return m_midi_rx; });

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->txd_func().set("mdout", FUNC(midi_port_device::write_txd));

	SPEAKER(config, "stereo", 2).front();
	SPEAKER(config, "effect").front_center();

	// voice 0-3 DMA
	AM9517A(config, m_dma[0], 5'000'000);

	// voice 4-5 DMA
	AM9517A(config, m_dma[1], 5'000'000);

	// LCD
	HD44780(config, m_lcdc, 270'000); // TODO: type and clock both guessed
	m_lcdc->set_lcd_size(2, 16);
	m_lcdc->set_pixel_update_cb(FUNC(xr10_state::lcd_update));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	screen.set_size(6 * 16, 16);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);
}

/**************************************************************************/
HD44780_PIXEL_UPDATE(xr10_state::lcd_update)
{
	if (x < 6 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

/**************************************************************************/
u8 xr10_state::keys_r()
{
	u8 data = 0xff;
	for (int i = 0; i < 6; i++)
		if (BIT(m_key_sel, i))
			data &= m_keys[i]->read();

	return data;
}


static INPUT_PORTS_START( xr10 )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad Bank")             PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Timing Correct")       PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Edit")                 PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 4")                PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 7")                PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 10 / Variation C") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 13 / Fill In C")   PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Start / Stop")         PORT_CODE(KEYCODE_COMMA)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 1")                PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Accent")               PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 2")                PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 5")                PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 8")                PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 11 / Variation B") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 14 / Fill In B")   PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Break / Continue")     PORT_CODE(KEYCODE_K)

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Mode")                 PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Erase")                PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 3")                PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 6")                PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 9")                PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 12 / Variation A") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Pad 15 / Fill In A")   PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Intro / End")          PORT_CODE(KEYCODE_I)

	PORT_START("KEY3")
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Cursor Left")  PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Cursor Up")    PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Cursor Down")  PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad - / N") PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("KEY4")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 5")     PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 6")     PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 7")     PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 8")     PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 9")     PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad Enter") PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("KEY5")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 0")     PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 1")     PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 2")     PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 3")     PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 4")     PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad + / Y") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("AN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Fill In Pedal")

	PORT_START("AN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Start / Stop Pedal")

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_NAME("Tempo / Data") PORT_SENSITIVITY(10) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_CODE_DEC(KEYCODE_OPENBRACE) PORT_CODE_INC(KEYCODE_CLOSEBRACE)

	PORT_START("AN7")
	PORT_CONFNAME( 0xff, 0xff, "Battery Level" )
	PORT_CONFSETTING(    0xff, "Normal" )
	PORT_CONFSETTING(    0x00, "Low" )
INPUT_PORTS_END


ROM_START( xr10 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v22", "Version 2.2")
	ROMX_LOAD( "akai xr10 v2.2.ic2", 0x0000, 0x8000, CRC(26676dec) SHA1(0803a3febe9583ce7c5fe8b299f5ec51cf04331e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "v21", "Version 2.1")
	ROMX_LOAD( "akai xr10 v2.1.ic2", 0x0000, 0x8000, CRC(e8fc6ded) SHA1(0018a818ebfd720f98b43b964234037db3a12ccc), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "v11", "Version 1.1")
	ROMX_LOAD( "akai xr10 v1.1.ic2", 0x0000, 0x8000, CRC(240ada36) SHA1(91da36530aad5eb63e7893bc489513206d8ce745), ROM_BIOS(2) )

	ROM_REGION16_LE(0x100000, "waverom", 0)
	ROM_LOAD16_WORD( "mb834200a-20-0d3.ic9", 0x000000, 0x080000, NO_DUMP )
	ROM_LOAD16_WORD( "mb834200a-20-0d2.ic8", 0x080000, 0x080000, NO_DUMP )
ROM_END

}

SYST( 1990, xr10, 0, 0, xr10, xr10, xr10_state, empty_init, "Akai", "XR10 16 Bit PCM Drum Machine", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
