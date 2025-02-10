// license:BSD-3-Clause
// copyright-holders:giulioz

/*
 * Roland PRO-E Intelligent Arranger (1989)
 * Partial emulation by Giulio Zausa
 *
 * The unit is very similar to the other Roland LA-based arrangers of that era (E-20, RA-50, ...)
 * Internally, it uses a main board with a MC68HC11 microcontroller, which computes the arranger sequences.
 * The main board communicates through a transmit-only serial port with a sound board (called CK-34),
 * which is very similar to the late models MT-32. The CK-34 board has not yet been dumped nor it's
 * emulated here, but it seems to not work with MIDI directly, but rather with a proprietary serial protocol.
 * The midi out of the emulated PRO-E though contains the entire arranger sequence, so another MT-32 emulator
 * (MUNT) can be used to test this.
 * 
 * TODO:
 * - Hook up rlndtnsc1 cards (some are already dumped)
 * - Add panel, buttons and leds
 * - Fix timers in the cpu to make the arranger work correctly
 * - Hook up MT-32 emulation when ready, dump CK-34 control rom
 * - Dump newer versions (2.0 is confirmed to exist)
 */

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "video/hd44780.h"

#include "mb63h149.h"

#include "emupal.h"
#include "screen.h"


namespace {

static INPUT_PORTS_START(proe)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("STYLE") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MAN BASS")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LOW TONE")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SOLO TONE")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MIDI FNCT")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PARAMETER")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ARR LOOP REC/PLAY")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ARR LOOP EXECUTE")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("NUMBER 4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("NUMBER 5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("NUMBER 6") PORT_CODE(KEYCODE_6)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TRANSP")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("MEL INT")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("VARIAT")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ADV ARR") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ARR HOLD") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("REV ON/OFF")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SYNC STOP")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SYNC STRT")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("NUMBER 1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("NUMBER 2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("NUMBER 3") PORT_CODE(KEYCODE_3)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("USER 1") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("USER 2") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("USER 3") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("USER 4") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("USER 5") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("USER 6") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("USER 7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("USER 8")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 10")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 11")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 12")

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("KM DRUMS")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FILL ORIG") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("KM EFFECT")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FILL VARI") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("INTRO END") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TAP TEMPO BREAK")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PLAY COMP") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("REC COMP")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 7")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 8")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 9")

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("KM SOLO")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("GROUP A/B")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("KM LOWER")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("WRITE")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("KM ARR")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FROM CARD")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("KM MBASS")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("TO CARD")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 4")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 5")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 6")

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SOLO +")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SOLO -")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFECT +")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFECT -")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FADE OUT")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("FADE IN")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("START STOP") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFF HOLD")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 1")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 2")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("EFFEFCT 3")

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LOWER +")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LOWER -")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ACCOMP +")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("ACCOMP -")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS +")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BASS -")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DRUMS +")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DRUMS -")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PLAY TRACK SEL")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("USER BANK")
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CURS8A")

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BANK 1") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BANK 2") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BANK 3") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BANK 4") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BANK 5") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BANK 6") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BANK 7") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BANK 8") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("NUMBER 7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("NUMBER 8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x400, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("REV TYPE") PORT_CODE(KEYCODE_K)

	// To enable test mode
	// PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("REV TYPE") PORT_CODE(KEYCODE_K)
INPUT_PORTS_END

class roland_proe_state : public driver_device
{
public:
	roland_proe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ram(*this, "ram", 0x2000 * 4, ENDIANNESS_LITTLE)
		, m_rom_bank(*this, "rom_bank")
		, m_ram_bank(*this, "ram_bank")
		, m_cpu(*this, "cpu")
		, m_patterns(*this, "patterns")
		, m_lcd(*this, "lcd")
		, m_keys(*this, "KEY%u", 0)
		, m_acia(*this, "acia")
		, m_mdout(*this, "mdout")
		, m_keyscan(*this, "keyscan")
	{
	}

	void proe(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	memory_share_creator<u8> m_ram;
	required_memory_bank m_rom_bank;
	required_memory_bank m_ram_bank;
	void bank_w(u8 data);
	void proe_map_a(address_map &map);
	
	u8 key_scan_i = 0;
	u8 pa_r();
	void pa_w(u8 data);
	u8 keys_r();
	void leds_w(u8 data);

	u8 lcd_mode = 0;
	void lcd_data_w(u8 data);
	void lcd_control_w(u8 data);
	u8 lcd_r();
	HD44780_PIXEL_UPDATE(pixel_update);
	void lcd_palette(palette_device &palette) const;

	void write_acia_clock(int state);
	void int_midi_w(u8 data);

	required_device<mc68hc11a0_device> m_cpu;
	required_region_ptr<u8> m_patterns;
	required_device<hd44780_device> m_lcd;
	required_ioport_array<8> m_keys;
	required_device<acia6850_device> m_acia;
	required_device<midi_port_device> m_mdout;
	required_device<mb63h149_device> m_keyscan;
};

void roland_proe_state::machine_start()
{
	m_rom_bank->configure_entries(0, 8, memregion("patterns")->base(), 0x8000);
	m_rom_bank->configure_entries(8, 8, memregion("patterns")->base(), 0x8000); // TODO: card goes here
	m_ram_bank->configure_entries(0, 4, &m_ram[0], 0x2000);
}

void roland_proe_state::machine_reset()
{
	key_scan_i = 0;
}

void roland_proe_state::write_acia_clock(int state)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

void roland_proe_state::int_midi_w(u8 data)
{
	// logerror("midi tx %02x\n", data);
}

void roland_proe_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0, 0, 0));
	palette.set_pen_color(1, rgb_t(0, 255, 0));
}

HD44780_PIXEL_UPDATE(roland_proe_state::pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

void roland_proe_state::lcd_data_w(u8 data)
{
	if (lcd_mode == 0x04)
		m_lcd->data_w(data);
	else if (lcd_mode == 0xf8)
		m_lcd->control_w(data);
}

void roland_proe_state::lcd_control_w(u8 data)
{
	// 0x04: data
	// 0x06: ??
	// 0xf8: control
	if (data == 0x04 || data == 0x06 || data == 0xf8)
		lcd_mode = data;
}

u8 roland_proe_state::lcd_r()
{
	return 0x00;
}

u8 roland_proe_state::keys_r()
{
	u8 result = m_keys[key_scan_i]->read() & 0xff;
	return result;
}

u8 roland_proe_state::pa_r()
{
	u8 result = (m_keys[key_scan_i]->read() >> 8) | 0b11111000;
	return result;
}

void roland_proe_state::pa_w(u8 data)
{
	key_scan_i = (data >> 3) & 0x07;
}

void roland_proe_state::leds_w(u8 data)
{
	// TODO
	// logerror("leds %02x\n", data);
}

void roland_proe_state::bank_w(u8 data)
{
	m_rom_bank->set_entry(data >> 4);
	m_ram_bank->set_entry(data & 3);
}

// u8 roland_proe_state::patterns_r(offs_t offset)
// {
// 	return m_patterns[(offset & 0x7fff) | (rom_bank << 15)];
// }

void roland_proe_state::proe_map_a(address_map &map)
{
	// 0x0000-0x00ff: cpu internal ram
	// 0x1000-0x103f: cpu registers
	map(0x1200, 0x1200).r(FUNC(roland_proe_state::keys_r));
	map(0x1400, 0x1401).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x160d, 0x160d).w(FUNC(roland_proe_state::lcd_control_w));
	map(0x160e, 0x160e).rw(FUNC(roland_proe_state::lcd_r), FUNC(roland_proe_state::lcd_data_w));
	map(0x180d, 0x180d).w(FUNC(roland_proe_state::bank_w));
	map(0x180e, 0x180e).w(FUNC(roland_proe_state::leds_w));
	map(0x1a00, 0x1bff).rw(m_keyscan, FUNC(mb63h149_device::read), FUNC(mb63h149_device::write));
	map(0x2000, 0x3fff).bankrw("ram_bank");
	map(0x4000, 0xbfff).bankrw("rom_bank");
	map(0xc000, 0xffff).rom().region("cpu", 0); // ic4
}

void roland_proe_state::proe(machine_config &config)
{
	XTAL main_clock = 16_MHz_XTAL;

	MC68HC11A0(config, m_cpu, main_clock / 2); // MC68HC11A0P
	m_cpu->set_addrmap(AS_PROGRAM, &roland_proe_state::proe_map_a);
	m_cpu->in_pa_callback().set(FUNC(roland_proe_state::pa_r));
	m_cpu->out_pa_callback().set(FUNC(roland_proe_state::pa_w));
	m_cpu->serial_tx_cb().set(FUNC(roland_proe_state::int_midi_w));

	MB63H149(config, m_keyscan, main_clock);
	m_keyscan->int_callback().set_inputline(m_cpu, MC68HC11_IRQ_LINE);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update(m_lcd, FUNC(hd44780_device::screen_update));
	screen.set_size(16*6, 16);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(roland_proe_state::lcd_palette), 2);

	HD44780(config, m_lcd, 270'000);
	m_lcd->set_lcd_size(2, 16);
	m_lcd->set_pixel_update_cb(FUNC(roland_proe_state::pixel_update));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	ACIA6850(config, m_acia, 0);
	m_acia->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set_inputline(m_cpu, MC68HC11_XIRQ_LINE);

	clock_device &acia_clock(CLOCK(config, "acia_clock", main_clock / 8));
	acia_clock.signal_handler().set(FUNC(roland_proe_state::write_acia_clock));
}

ROM_START(proe)
	ROM_REGION(0x4000, "cpu", 0)
	ROM_LOAD("roland_proe_v100.ic4", 0x0000, 0x4000, CRC(2924631e) SHA1(facf5e7eef6b57d00931aa6a9fc7e76c4616235d))
	
	ROM_REGION(0x40000, "patterns", 0)
	ROM_LOAD("roland_proe_v100.ic6", 0x0000, 0x10000, CRC(a5ea2d41) SHA1(ba826f822462f011eb268a04481db9259876ad23))
	ROM_LOAD("roland_proe_v100.ic7", 0x10000, 0x10000, CRC(b27a483f) SHA1(de8f21730b9a568930723031afb89736924aedf5))
	ROM_LOAD("roland_proe_v100.ic17", 0x20000, 0x10000, CRC(8e822ac1) SHA1(3e6bbd154410a5fee0fd3dabe27beea5a7fb7f72))
	ROM_LOAD("roland_proe_v100.ic18", 0x30000, 0x10000, CRC(f78ea4ff) SHA1(aac72f6ceb98036a5737e81290d091c9b2ec5538))
ROM_END

} // anonymous namespace


SYST(1989, proe, 0, 0, proe, proe, roland_proe_state, empty_init, "Roland", "PRO-E Intelligent Arranger", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
