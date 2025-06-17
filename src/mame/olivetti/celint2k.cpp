// license: BSD-3-Clause
// copyright-holders:Nigel Barnes

/********************************************************************************************

Skeleton driver for Olivetti Celint 2000 phone with Videotext terminal.
In Spain, Banco Santander distributed it as the Superfono Santander (with a custom ROM) as
part of a "bank at home" pilot program.

Main PCB:
    ____________________________________________________________________________________
   |                          |           _________________                            |
   |                          |          |  GoldStar      |  ___________  ___________  |
   |   POWER SUPPLY           |   BATT   |  GM76C256L-85  | |SN74HC245N| |_SN74HC74_|  |
   |                          |          |________________|               ___________  |
   |                          |           Xtal                           |M74HC00B1_|  |
   |__________________________|        4.4334 MHz         ____________                 |
   |                                                      |           |                |
   |                                     Xtal 32.768 kHz  | OKI M6255 |                |
   |                 ___________          ___________     |___________|                |
   |                |SN74HC08N_|         |_PCF8573P_|   _________________________      |
   |                 ___________                       | GoldStar               |      |
   |                |M74HC374B1|                       | GM76C8128ALL-85        |      |
   |                                                   |________________________|      |
   |                 _________________________          _________________________      |
   |                | GolsStar               |         |                        |      |
   | SMARTCARD      | GM76C88L-15            |         | EPROM                  |      |
   |  READER        |________________________|         |________________________|      |
   |                 ______________________________                                    |
   |                | Zilog                       |    Xtal           ___________      |
   |                | Z84C0006PEC Z80 CPU         |   11.0573 MHz    |SN74LS145N|      |
   |                |_____________________________|         ________________           |
   |                                     ___________       |               |           |
   |                                    |GAL16V8-20|       | Zilog         |           |
   |                                                       | Z84C9008VSC   |           |
   |                                                       | Z80 KIO       |           |
   |                     _____________________             |_______________|           |
   |                    |                    |                                         |
   |                    | 73K322L-IP         |  _____        ___________      :        |
   |                    |____________________| |TL7705ACP   |HCF4094BE_|      ·        |
   |                                            ___________                            |
   |      _______                              |HCF4066BE_|                            |
   |     |7805CT|             ___________       _____                                  |
 __|_                        |_TL084CN__|      MC34119P                                |
|    |                                                         _____________________   |
|DB25|                                                        |                    |   |
|RS232                                                        | MC34118P           |   |
|____|    ___________                                         |____________________|   |
   |     |MC145406P_|                                                                  |
 __|_                                  ___________                                     |
|    |                                |_GD4053B__|                                     |
|P/T |                                     _______                         ___________ |
|____|                                    |H11D1_|                        |__M3541B__| |
   |                                                                                   |
 __|_                                                  ___________                     |
|    |                                                |_KA8501A__|                     |
|LINE|                                                                                 |
|____|                                                                                 |
  _|_                                                                    SPEAKER       |
 | CD/MF switch                                                                        |
 |___|                                                           _______               |
   |                                           _____            |LS1240A               |
   |__________________________________________|    |___________________________________|
                                              |____|

Video screen is driven by 9 Hitachi HD61105A chips (separate PCB).

********************************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/pcf8573.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "video/msm6255.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class celint2k_state : public driver_device
{
public:
	celint2k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "msm6255")
		, m_pio(*this, "pio")
		, m_ctc(*this, "ctc")
		, m_sio(*this, "sio")
		, m_rtc(*this, "rtc")
		, m_ram(*this, "ram", 0x20000, ENDIANNESS_LITTLE)
		, m_bank_view(*this, "bank")
		, m_bank_ram(*this, "bank_ram")
		, m_bank_rom(*this, "bank_rom")
	{ }

	void celint2k(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<msm6255_device> m_lcdc;
	required_device<z80pio_device> m_pio;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_device<pcf8573_device> m_rtc;
	memory_share_creator<uint8_t> m_ram;
	memory_view m_bank_view;
	memory_bank_creator m_bank_ram;
	memory_bank_creator m_bank_rom;

	void lcdc_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void celint2k_palette(palette_device &palette) const;

	[[maybe_unused]] void port_e0_w(uint8_t data);
	uint8_t pa_data_r();
	void pa_data_w(uint8_t data);
	uint8_t pb_data_r();
	void pb_data_w(uint8_t data);
	uint8_t pc_data_r();
	void pc_data_w(uint8_t data);
	void pc_ctrl_w(uint8_t data);
	void kio_cmd_w(uint8_t data);

	uint8_t m_pc_data_in;
	uint8_t m_pc_data_out;
	uint8_t m_pc_ctrl;
};

void celint2k_state::celint2k_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

void celint2k_state::lcdc_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x7fff).ram().share("videoram");
}

void celint2k_state::mem_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().region("maincpu", 0);
	map(0x6000, 0x7fff).ram();
	map(0x8000, 0xffff).view(m_bank_view);
	m_bank_view[0](0x8000, 0xffff).ram().share("videoram");
	m_bank_view[1](0x8000, 0xffff).bankrw(m_bank_ram);
	m_bank_view[2](0x8000, 0xffff).bankr(m_bank_rom);
}

void celint2k_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x04, 0x07).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x08, 0x0b).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x0c, 0x0c).rw(FUNC(celint2k_state::pc_data_r), FUNC(celint2k_state::pc_data_w));
	map(0x0d, 0x0d).w(FUNC(celint2k_state::pc_ctrl_w));
	map(0x0e, 0x0e).w(FUNC(celint2k_state::kio_cmd_w));
	map(0x20, 0x21).m(m_lcdc, FUNC(msm6255_device::map));
	//map(0x40, 0x40).w()
	//map(0x80, 0x80).w()
	//map(0xa0, 0xa0).r()
	//map(0xe0, 0xe0).w(FUNC(celint2k_state::port_e0_w));
}

void celint2k_state::machine_start()
{
	m_bank_ram->configure_entries(0, 4, m_ram, 0x8000);
	m_bank_rom->configure_entries(0, 7, memregion("maincpu")->base() + 0x8000, 0x8000);
}

void celint2k_state::machine_reset()
{
	m_bank_ram->set_entry(0);
	m_bank_rom->set_entry(0);

	m_pc_data_in = 0x00;
	m_pc_data_out = 0x00;
	m_pc_ctrl = 0xff;

	m_bank_view.select(2);
	m_bank_rom->set_entry(0);
}

void celint2k_state::port_e0_w(uint8_t data)
{
	logerror("%s port_e0_w: %02x\n", machine().describe_context(), data);
}

uint8_t celint2k_state::pa_data_r()
{
	// input 0xff
	uint8_t data = 0x00;
	//logerror("%s pa_data_r: %02x\n", machine().describe_context(), data);
	return data;
}

void celint2k_state::pa_data_w(uint8_t data)
{
	// output 0x00
	//logerror("%s pa_data_w: %02x\n", machine().describe_context(), data);
}

uint8_t celint2k_state::pb_data_r()
{
	uint8_t data = 0x00;
	//logerror("%s pb_data_r: %02x\n", machine().describe_context(), data);
	data = m_rtc->sda_r() << 1;

	return data;
}

void celint2k_state::pb_data_w(uint8_t data)
{
	//logerror("%s pb_data_w: %02x\n", machine().describe_context(), data);

	m_rtc->sda_w(BIT(data, 3));
	m_rtc->scl_w(BIT(data, 2));
}

uint8_t celint2k_state::pc_data_r()
{
	uint8_t m_pc_data_in = 0x00;
	uint8_t data = (m_pc_data_in & m_pc_ctrl) | (m_pc_data_out & (m_pc_ctrl ^ 0xff));
	logerror("%s pc_data_r: %02x & %02x\n", machine().describe_context(), data, m_pc_ctrl);
	return data;
}

void celint2k_state::pc_data_w(uint8_t data)
{
	// output 0xff
	m_pc_data_out = data;
	logerror("%s pc_data_w: %02x | %02x\n", machine().describe_context(), m_pc_data_out, m_pc_ctrl);

	switch (data & 0x0f)
	{
	case 0x00:
		m_bank_view.select(0);
		break;
	case 0x02:
		m_bank_view.select(1);
		break;
	case 0x06:
		m_bank_view.select(2);
		//switch (data & 0x70)
		//{
		//case 0x70:
		//  m_bank_rom->set_entry(0);
		//  break;
		//default:
		//  m_bank_rom->set_entry((data >> 4) & 7);
		//  break;
		//}
		//if ((data >> 4) < 7)
		//  m_bank_rom->set_entry(data >> 4);
		break;
	default:
		logerror("%s pc_data_w: %02x | %02x unknown bank\n", machine().describe_context(), m_pc_data_out, m_pc_ctrl);
		break;
	}
}

void celint2k_state::pc_ctrl_w(uint8_t data)
{
	//logerror("%s pc_ctrl_w: %02x\n", machine().describe_context(), data);
	m_pc_ctrl = data;
}

void celint2k_state::kio_cmd_w(uint8_t data)
{
	logerror("%s kio_cmd_w: %02x\n", machine().describe_context(), data);

	if (BIT(data, 4))
		m_pio->reset();

	if (BIT(data, 5))
		m_ctc->reset();

	if (BIT(data, 6))
		m_sio->reset();
}

INPUT_PORTS_START( celint2k )
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "sio" },
	{ "ctc" },
	{ "pio" },
	{ nullptr }
};

void celint2k_state::celint2k(machine_config &config)
{
	Z80(config, m_maincpu, 11'057'300 / 2); // Z84C0006PEC, verified divisor
	m_maincpu->set_addrmap(AS_PROGRAM, &celint2k_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &celint2k_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	Z80PIO(config, m_pio, 11'057'300 / 2); // TODO: part of Zilog Z84C9008VSC Z80 KIO
	m_pio->out_pa_callback().set(FUNC(celint2k_state::pa_data_w));
	m_pio->in_pa_callback().set(FUNC(celint2k_state::pa_data_r));
	m_pio->out_pb_callback().set(FUNC(celint2k_state::pb_data_w));
	m_pio->in_pb_callback().set(FUNC(celint2k_state::pb_data_r));
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc, 11'057'300 / 2); // TODO: part of Zilog Z84C9008VSC Z80 KIO
	m_ctc->set_clk<0>(2.4576_MHz_XTAL / 2);
	m_ctc->set_clk<1>(2.4576_MHz_XTAL / 2);
	m_ctc->zc_callback<0>().set(m_sio, FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<0>().append(m_sio, FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<1>().set(m_sio, FUNC(z80sio_device::rxcb_w));
	m_ctc->zc_callback<1>().append(m_sio, FUNC(z80sio_device::txcb_w));
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80SIO(config, m_sio, 11'057'300 / 2); // TODO: part of Zilog Z84C9008VSC Z80 KIO
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_sio->out_txda_callback().set("serial", FUNC(rs232_port_device::write_txd));
	m_sio->out_rtsa_callback().set("serial", FUNC(rs232_port_device::write_rts));
	m_sio->out_dtra_callback().set("serial", FUNC(rs232_port_device::write_dtr));
	m_sio->out_txdb_callback().set("host", FUNC(rs232_port_device::write_txd));

	rs232_port_device &serial(RS232_PORT(config, "serial", default_rs232_devices, nullptr));
	serial.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	serial.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	serial.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));

	rs232_port_device &host(RS232_PORT(config, "host", default_rs232_devices, nullptr));
	host.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));

	PCF8573(config, m_rtc, 32.768_kHz_XTAL);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD)); // TECDIS C425901 backlight 320x240 gLCD
	screen.set_refresh_hz(60); // Guess
	screen.set_screen_update(m_lcdc, FUNC(msm6255_device::screen_update));
	screen.set_size(480, 240);
	screen.set_visarea(0, 480-1, 0, 240-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(celint2k_state::celint2k_palette), 2);

	MSM6255(config, m_lcdc, 4'433'400 );
	m_lcdc->set_addrmap(0, &celint2k_state::lcdc_map);
	m_lcdc->set_screen("screen");

	SPEAKER(config, "mono").front_center();
}

ROM_START( celint2kss )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "sa21-b_eae5_27c020.ic19", 0x00000, 0x40000, CRC(0f5fd110) SHA1(9d1abc90db5eb5efbcde1da4b8a7ef6438723664) )

	ROM_REGION( 0x0117, "pld", 0 )
	ROM_LOAD( "gal16v8-20lnc.ic15", 0x0000, 0x0117, CRC(45724282) SHA1(09c1029af68ef6f8bd1d17d19dbce7a691f80171) )
ROM_END

} // anonymous namespace

COMP( 1995, celint2kss, 0, 0, celint2k, celint2k, celint2k_state, empty_init, "Olivetti", "Celint 2000 (Superfono Santander edition)", MACHINE_NOT_WORKING ) // Labeled as model "MULTIMEDIA - T"
