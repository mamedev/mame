// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Yamaha PSR-340 PortaSound keyboard
    Preliminary driver by R. Belmont

    CPU and Sound: SWX00B, which is an H8S/2000 series CPU and Yamaha
      AWM2 with capabilities somewhat intermediate between SWP00 and
      SWP30.
    LCD controller: KS0066U (apparently equivalent to Samsung S6A0069)
    FDC: HD63266F (uPD765 derivative)
    RAM: 256KiB, 2x uPD431000 or equivalent
*/

#include "emu.h"

#include "bus/midi/midi.h"
#include "cpu/h8/swx00.h"
#include "machine/nvram.h"
#include "video/hd44780.h"

#include "mks3.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class psr340_state : public driver_device
{
public:
	psr340_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_nvram(*this, "ram"),
		m_mks3(*this, "mks3"),
		m_lcdc(*this, "ks0066"),
		m_outputs(*this, "%02d.%x.%x", 0U, 0U, 0U),
		m_key(*this, "S%c", 'A')
	{ }

	void psr340(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<swx00_device> m_maincpu;
	required_shared_ptr<u16> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<mks3_device> m_mks3;
	required_device<hd44780_device> m_lcdc;
	output_finder<80, 8, 5> m_outputs;
	required_ioport_array<8> m_key;

	void c_map(address_map &map) ATTR_COLD;
	void s_map(address_map &map) ATTR_COLD;

	void pdt_w(u16 data);
	u8 pad_r();
	void txd_w(u8 data);

	void render_w(int state);

	u8 m_matrixsel = 0U;
};

u8 psr340_state::pad_r()
{
	u8 data = 0;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_matrixsel, i))
		{
			data |= m_key[i]->read();
		}
	}

	return data;
}

void psr340_state::pdt_w(u16 data)
{
	// bit 14 = mks3 ic, bit 11 = E, bit 12 = RS, R/W is connected to GND so write-only
	// all bits are also matrix select for reading the controls
	m_mks3->ic_w(BIT(data, 14));
	m_lcdc->rs_w(BIT(data, 12));
	m_lcdc->e_w(BIT(data, 11));
	m_lcdc->db_w(data);
	m_matrixsel = data;
}

void psr340_state::txd_w(u8 data)
{
	m_mks3->req_w(BIT(data, 1));
}

void psr340_state::c_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0); // cs0
	map(0x400000, 0x43ffff).ram().share(m_ram); // cs2

	map(0x600000, 0x600000).lr8(NAME([]() -> uint8_t { return 0x80; }));    // FDC status, cs3, cs4 w/ dack
}

void psr340_state::s_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("wave", 0);
}

void psr340_state::machine_start()
{
	save_item(NAME(m_matrixsel));
	m_outputs.resolve();
}

void psr340_state::machine_reset()
{
}

void psr340_state::render_w(int state)
{
	if(!state)
		return;

	const u8 *render = m_lcdc->render();
	for(int yy=0; yy != 8; yy++)
		for(int x=0; x != 80; x++) {
			uint8_t v = render[16*x + yy];
			for(int xx=0; xx != 5; xx++)
				m_outputs[x][yy][xx] = (v >> xx) & 1;
		}
}


static INPUT_PORTS_START(psr340)
	PORT_START("SA")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Song") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Record") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Main A") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("-") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Execute") PORT_CODE(KEYCODE_ENTER)

	PORT_START("SB")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Function") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Dual") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Intro/End") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Metronome") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Tenkey +") PORT_CODE(KEYCODE_EQUALS)

	PORT_START("SC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("O.T.S. 6") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Touch") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Start/Stop") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Porta Grand") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Tenkey -") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("7") PORT_CODE(KEYCODE_7)

	PORT_START("SD")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("O.T.S. 5") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Harmony") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Sync Start") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Utility") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("9") PORT_CODE(KEYCODE_9)

	PORT_START("SE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("O.T.S. 4") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Reverb") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Accomp On") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Save") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("6") PORT_CODE(KEYCODE_6)

	PORT_START("SF")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("O.T.S. 3") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Demo") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Load") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("+") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("4") PORT_CODE(KEYCODE_4)

	PORT_START("SG")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("O.T.S. 2") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Chord Guide") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Voice") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("1") PORT_CODE(KEYCODE_1)

	PORT_START("SH")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("O.T.S. 1") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Main B") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Style") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("3") PORT_CODE(KEYCODE_3)
INPUT_PORTS_END

void psr340_state::psr340(machine_config &config)
{
	/* basic machine hardware */
	SWX00(config, m_maincpu, 8.4672_MHz_XTAL*2, swx00_device::MODE_DUAL);
	m_maincpu->set_addrmap(swx00_device::AS_C, &psr340_state::c_map);
	m_maincpu->set_addrmap(swx00_device::AS_S, &psr340_state::s_map);
	m_maincpu->read_adc<0>().set_constant(0x3ff); // Battery level
	m_maincpu->read_adc<1>().set_constant(0x000); // GND
	m_maincpu->read_adc<2>().set_constant(0x000); // GND
	m_maincpu->read_adc<3>().set_constant(0x000); // GND

	m_maincpu->write_pdt().set(FUNC(psr340_state::pdt_w));
	m_maincpu->read_pad().set(FUNC(psr340_state::pad_r));
	m_maincpu->write_txd().set(FUNC(psr340_state::txd_w));

	m_maincpu->add_route(0, "lspeaker", 1.0);
	m_maincpu->add_route(1, "rspeaker", 1.0);

	// mks3 is connected to sclki, sync comms on sci1
	// something generates 500K for sci0, probably internal to the swx00
	m_maincpu->sci_set_external_clock_period(0, attotime::from_hz(500000));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	MKS3(config, m_mks3);
	m_mks3->write_da().set(m_maincpu, FUNC(swx00_device::sci_rx_w<1>));
	m_mks3->write_clk().set(m_maincpu, FUNC(swx00_device::sci_clk_w<1>));

	KS0066(config, m_lcdc, 270000); // 91K resistor
	m_lcdc->set_default_bios_tag("f05");
	m_lcdc->set_lcd_size(2, 40);

	/* video hardware */
	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_SVG);
	screen.set_refresh_hz(60);
	screen.set_size(800, 384);
	screen.set_visarea_full();
	screen.screen_vblank().set(FUNC(psr340_state::render_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_maincpu, FUNC(swx00_device::sci_rx_w<0>));

	auto &mdout(MIDI_PORT(config, "mdout", midiout_slot, "midiout"));
	m_maincpu->write_sci_tx<0>().set(mdout, FUNC(midi_port_device::write_txd));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

ROM_START( psr340 )
	ROM_REGION(0x200000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP("xv89710.bin", 0x000000, 0x200000, CRC(271ccb8a) SHA1(ec6abbdb82a5e851b77338c79ecabfd8040f023d))

	ROM_REGION16_BE(0x200000, "wave", 0)
	ROM_LOAD("xv89810.bin", 0x000000, 0x200000, CRC(10e68363) SHA1(5edee814bf07c49088da44474fdd5c817e7c5af0))

	ROM_REGION(399369, "screen", 0)
	ROM_LOAD("psr340-lcd.svg", 0, 399369, CRC(f9d11ca6) SHA1(da036d713c73d6b452a3e2d2b2234d473422d5fb))
ROM_END

} // anonymous namespace


CONS(1994, psr340, 0, 0, psr340, psr340, psr340_state, empty_init, "Yamaha", "PSR-340 PortaSound", MACHINE_NOT_WORKING)
