// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    BBC Master Compact

    ADB20 - Master Compact

******************************************************************************/

#include "emu.h"
#include "bbc.h"
#include "acorn_serproc.h"
#include "bbc_kbd.h"

#include "bus/bbc/exp/exp.h"
#include "bus/bbc/joyport/joyport.h"
#include "cpu/m6502/g65sc02.h"
#include "machine/i2cmem.h"
#include "machine/wd_fdc.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "bbcm.lh"


namespace {

class bbcmc_state : public bbc_state
{
public:
	bbcmc_state(const machine_config &mconfig, device_type type, const char *tag)
		: bbc_state(mconfig, type, tag)
		, m_view_lynne(*this, "view_lynne")
		, m_view_hazel(*this, "view_hazel")
		, m_view_andy(*this, "view_andy")
		, m_view_tst(*this, "view_tst")
		, m_kbd(*this, "kbd")
		, m_sn(*this, "sn")
		, m_i2cmem(*this, "i2cmem")
		, m_wdfdc(*this, "wdfdc")
		, m_adlc(*this, "mc6854")
		, m_exp(*this, "exp")
		, m_joyport(*this, "joyport")
		, m_power_led(*this, "power_led")
	{ }

	void bbcmc(machine_config &config);
	void bbcmc_ar(machine_config &config);
	void autoc15(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	memory_view m_view_lynne;
	memory_view m_view_hazel;
	memory_view m_view_andy;
	memory_view m_view_tst;
	required_device<bbc_kbd_device> m_kbd;
	required_device<sn76489a_device> m_sn;
	required_device<i2cmem_device> m_i2cmem;
	required_device<wd1772_device> m_wdfdc;
	required_device<mc6854_device> m_adlc;
	required_device<bbc_exp_slot_device> m_exp;
	required_device<bbc_joyport_slot_device> m_joyport;
	output_finder<> m_power_led;

	void bbcmc_fetch(address_map &map) ATTR_COLD;
	void bbcmc_mem(address_map &map) ATTR_COLD;
	void bbcmc_io(address_map &map) ATTR_COLD;
	void autoc15_mem(address_map &map) ATTR_COLD;
	void autoc15_io(address_map &map) ATTR_COLD;

	void trigger_reset(int state);

	uint8_t fetch_r(offs_t offset);
	uint8_t acccon_r();
	void acccon_w(uint8_t data);
	uint8_t romsel_r();
	void romsel_w(offs_t offset, uint8_t data);
	uint8_t paged_r(offs_t offset);
	void paged_w(offs_t offset, uint8_t data);
	void drive_control_w(uint8_t data);

	uint8_t sysvia_pa_r();
	void sysvia_pa_w(uint8_t data);
	void update_sdb();
	uint8_t sysvia_pb_r();
	void sysvia_pb_w(uint8_t data);
	uint8_t joyport_r();
	void joyport_w(uint8_t data);

	uint8_t m_sdb = 0x00;
	uint8_t m_acccon = 0x00;

	enum
	{
		ACCCON_D = 0,
		ACCCON_E,
		ACCCON_X,
		ACCCON_Y,
		ACCCON_ITU,
		ACCCON_IFJ,
		ACCCON_TST,
		ACCCON_IRR
	};
};


void bbcmc_state::machine_start()
{
	bbc_state::machine_start();

	m_power_led.resolve();

	save_item(NAME(m_acccon));
	save_item(NAME(m_sdb));
}


void bbcmc_state::machine_reset()
{
	m_power_led = 0;
}


void bbcmc_state::bbcmc_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(bbcmc_state::fetch_r));
}


void bbcmc_state::bbcmc_mem(address_map &map)
{
	map(0x0000, 0x7fff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));                                    //    0000-7FFF                 Regular RAM
	map(0x3000, 0x7fff).view(m_view_lynne);                                                                            //    3000-7FFF                 20K Shadow RAM LYNNE
	m_view_lynne[0](0x3000, 0x7fff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset + 0xb000]; }));
	m_view_lynne[0](0x3000, 0x7fff).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0xb000] = data; }));
	map(0x8000, 0xbfff).rw(FUNC(bbcmc_state::paged_r), FUNC(bbcmc_state::paged_w));                                    //    8000-8FFF                 Paged ROM/RAM
	map(0x8000, 0x8fff).view(m_view_andy);                                                                             //    8000-8FFF                 4K RAM ANDY
	m_view_andy[0](0x8000, 0x8fff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset + 0x8000]; }));
	m_view_andy[0](0x8000, 0x8fff).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0x8000] = data; }));
	map(0xc000, 0xffff).rw(FUNC(bbcmc_state::mos_r), FUNC(bbcmc_state::mos_w));                                        //    C000-FFFF                 OS ROM
	map(0xc000, 0xdfff).view(m_view_hazel);                                                                            //    C000-DFFF                 8K RAM HAZEL
	m_view_hazel[0](0xc000, 0xdfff).lr8(NAME([this](offs_t offset) { return m_ram->pointer()[offset + 0x9000]; }));
	m_view_hazel[0](0xc000, 0xdfff).lw8(NAME([this](offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0x9000] = data; }));
	map(0xfc00, 0xfeff).m(FUNC(bbcmc_state::bbcmc_io));                                                                //    FC00-FFFF                 OS ROM or hardware IO
	map(0xfc00, 0xfeff).view(m_view_tst);
	m_view_tst[0](0xfc00, 0xfeff).m(FUNC(bbcmc_state::bbcmc_io));
	m_view_tst[0](0xfc00, 0xfeff).lr8(NAME([this](offs_t offset) { return mos_r(0x3c00 + offset); }));
}

void bbcmc_state::bbcmc_io(address_map &map)
{
	map(0x0000, 0x00ff).rw(m_exp, FUNC(bbc_exp_slot_device::fred_r), FUNC(bbc_exp_slot_device::fred_w));               //    FC00-FCFF  Compact        FRED Address Page
	map(0x0100, 0x01ff).rw(m_exp, FUNC(bbc_exp_slot_device::jim_r), FUNC(bbc_exp_slot_device::jim_w));                 //    FD00-FDFF  Compact        JIM Address Page
	map(0x0200, 0x02ff).lr8(NAME([]() { return 0xfe; })).nopw();                                                       //    FE00-FEFF                 SHEILA Address Page
	map(0x0200, 0x0200).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));      //    FE00-FE07  6845 CRTC      Video controller
	map(0x0201, 0x0201).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0x0208, 0x0209).mirror(0x06).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));            //    FE08-FE0F  6850 ACIA      Serial controller
	map(0x0210, 0x0217).rw("serproc", FUNC(acorn_serproc_device::read), FUNC(acorn_serproc_device::write));            //    FE10-FE17  Serial ULA     Serial system chip
	map(0x0220, 0x0223).w(FUNC(bbcmc_state::video_ula_w));                                                             // W: FE20-FE23  Video ULA      Video system chip
	map(0x0224, 0x0227).w(FUNC(bbcmc_state::drive_control_w));                                                         // W: FE24-FE27  FDC Latch      1772 Control latch
	map(0x0228, 0x022f).rw(m_wdfdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write));                            //    FE28-FE2F  1772 FDC       Floppy disc controller
	map(0x0230, 0x0233).rw(FUNC(bbcmc_state::romsel_r), FUNC(bbcmc_state::romsel_w));                                  //    FE30-FE33  ROMSEL         ROM Select
	map(0x0234, 0x0237).rw(FUNC(bbcmc_state::acccon_r), FUNC(bbcmc_state::acccon_w));                                  //    FE34-FE37  ACCCON         ACCCON select register
	map(0x0238, 0x023b).lr8(NAME([this]() { econet_int_enable(0); return 0xfe; }));                                    // R: FE38-FE3B  INTOFF         ECONET Interrupt Off
	map(0x0238, 0x023b).lw8(NAME([this](uint8_t data) { econet_int_enable(0); }));                                     // W: FE38-FE3B  INTOFF         ECONET Interrupt Off
	map(0x023c, 0x023f).lr8(NAME([this]() { econet_int_enable(1); return 0xfe; }));                                    // R: FE3C-FE3F  INTON          ECONET Interrupt On
	map(0x023c, 0x023f).lw8(NAME([this](uint8_t data) { econet_int_enable(1); }));                                     // W: FE3C-FE3F  INTON          ECONET Interrupt On
	map(0x0240, 0x024f).mirror(0x10).m(m_sysvia, FUNC(via6522_device::map));                                           //    FE40-FE5F  6522 VIA       SYSTEM VIA
	map(0x0260, 0x026f).mirror(0x10).m(m_uservia, FUNC(via6522_device::map));                                          //    FE60-FE7F  6522 VIA       USER VIA
	map(0x0280, 0x029f).lr8(NAME([]() { return 0xfe; })).nopw();                                                       //    FE80-FE9F  Int. Modem     Int. Modem
	map(0x02a0, 0x02a3).mirror(0x1c).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));                //    FEA0-FEBF  68B54 ADLC     ECONET controller
	map(0x02e0, 0x02ff).lr8(NAME([]() { return 0xfe; })).nopw();                                                       //    FEE0-FEFF  Tube ULA       Tube system interface
}


void bbcmc_state::autoc15_mem(address_map &map)
{
	bbcmc_mem(map);
	map(0xfc00, 0xfeff).m(FUNC(bbcmc_state::autoc15_io));                                                              //    FC00-FFFF                 OS ROM or hardware IO
	m_view_tst[0](0xfc00, 0xfeff).m(FUNC(bbcmc_state::autoc15_io));
	m_view_tst[0](0xfc00, 0xfeff).lr8(NAME([this](offs_t offset) { return mos_r(0x3c00 + offset); }));
}

void bbcmc_state::autoc15_io(address_map &map)
{
	bbcmc_io(map);
	map(0x0200, 0x0200).mirror(0x06).rw(m_crtc, FUNC(hd6345_device::status_r), FUNC(hd6345_device::address_w));        //    FE00-FE07  6345 CRTC      Video controller
	map(0x0201, 0x0201).mirror(0x06).rw(m_crtc, FUNC(hd6345_device::register_r), FUNC(hd6345_device::register_w));
}


uint8_t bbcmc_state::fetch_r(offs_t offset)
{
	if (BIT(m_acccon, ACCCON_X) || (BIT(m_acccon, ACCCON_E) && offset >= 0xc000 && offset <= 0xdfff))
		m_view_lynne.select(0);
	else
		m_view_lynne.disable();

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}


uint8_t bbcmc_state::romsel_r()
{
	return m_romsel;
}

void bbcmc_state::romsel_w(offs_t offset, uint8_t data)
{
	// ROMSEL - FE30 read/write register
	//  b7 RAM 1 = Page in ANDY 8000-8FFF
	//         0 = Page in ROM  8000-8FFF
	//  b6     Not Used
	//  b5     Not Used
	//  b4     Not Used
	//  b3-b0  ROM/RAM Bank Select
	if (BIT(data, 7))
		m_view_andy.select(0);
	else
		m_view_andy.disable();

	m_romsel = data & 0x0f;
}


uint8_t bbcmc_state::acccon_r()
{
	return m_acccon;
}

void bbcmc_state::acccon_w(uint8_t data)
{
	// ACCCON - FE34 read/write register
	//  b7 IRR 1 = Causes an IRQ to the processor
	//  b6 TST 1 = Selects FC00-FEFF read from OS-ROM
	//  b5 IFJ 1 = Internal 1MHz bus
	//         0 = External 1MHz bus
	//  b4 ITU 1 = Internal Tube
	//         0 = External Tube
	//  b3 Y   1 = Read/Write HAZEL C000-DFFF RAM
	//         0 = Read/Write ROM C000-DFFF OS-ROM
	//  b2 X   1 = Read/Write LYNNE
	//         0 = Read/Write main memory 3000-8000
	//  b1 E   1 = Causes shadow if VDU code
	//         0 = Main all the time
	//  b0 D   1 = Display LYNNE as screen
	//         0 = Display main RAM screen
	m_acccon = data;

	// Bit IRR causes Interrupt Request.
	m_irqs->in_w<4>(BIT(m_acccon, ACCCON_IRR));

	// Bit D causes the CRT controller to display the contents of LYNNE.
	setvideoshadow(BIT(m_acccon, ACCCON_D));

	// Bit Y causes 8 Kbyte of RAM referred to as HAZEL to be overlayed on the MOS VDU drivers.
	if (BIT(m_acccon, ACCCON_Y))
		m_view_hazel.select(0);
	else
		m_view_hazel.disable();

	// Bit X causes all accesses to 3000-7fff to be re-directed to LYNNE.
	if (BIT(m_acccon, ACCCON_X))
		m_view_lynne.select(0);
	else
		m_view_lynne.disable();

	// Bit TST controls paging of ROM reads in the 0xfc00-0xfeff region
	// if 0 the I/O is paged for both reads and writes
	// if 1 the ROM is paged in for reads but writes still go to I/O
	if (BIT(m_acccon, ACCCON_TST))
		m_view_tst.select(0);
	else
		m_view_tst.disable();
}


uint8_t bbcmc_state::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_romsel)
	{
	case 0: case 1:
		// 32K socket or External (selected by link PL11)
		if (m_rom[m_romsel & 0x0e] && m_rom[m_romsel & 0x0e]->present())
		{
			data = m_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
		}
		else
		{
			data  = m_exp->rom_r(offset | (m_romsel & 0x01) << 14);
			data &= m_region_rom->base()[offset + (m_romsel << 14)];
		}
		break;

	case 4: case 5: case 6: case 7:
		// 32K sockets
		if (m_rom[m_romsel & 0x0e] && m_rom[m_romsel & 0x0e]->present())
		{
			data = m_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
		}
		else
		{
			data = m_region_rom->base()[offset + (m_romsel << 14)];
		}
		break;

	default:
		// 16K sockets
		if (m_rom[m_romsel] && m_rom[m_romsel]->present())
		{
			data = m_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_rom->base()[offset + (m_romsel << 14)];
		}
		break;
	}

	return data;
}

void bbcmc_state::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 0: case 1:
		// 32K socket or External (selected by link PL11)
		if (m_rom[m_romsel & 0x0e] && m_rom[m_romsel & 0x0e]->present())
		{
			m_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
		}
		else
		{
			m_exp->rom_w(offset | (m_romsel & 0x01) << 14, data);
		}
		break;

	case 4: case 5: case 6: case 7:
		// 32K sockets
		if (m_rom[m_romsel & 0x0e])
		{
			m_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
		}
		break;

	default:
		// 16K sockets
		if (m_rom[m_romsel])
		{
			m_rom[m_romsel]->write(offset, data);
		}
		break;
	}
}


uint8_t bbcmc_state::sysvia_pa_r()
{
	update_sdb();

	return m_sdb;
}

void bbcmc_state::sysvia_pa_w(uint8_t data)
{
	m_sdb = data;

	update_sdb();
}


void bbcmc_state::update_sdb()
{
	uint8_t const latch = m_latch->output_state();

	// sound
	if (!BIT(latch, 0))
		m_sn->write(m_sdb);

	// keyboard
	m_sdb = m_kbd->read(m_sdb);
}


uint8_t bbcmc_state::sysvia_pb_r()
{
	uint8_t data = 0xff;

	data &= ~0x30;
	data |= m_i2cmem->read_sda() << 4;

	return data;
}

void bbcmc_state::sysvia_pb_w(uint8_t data)
{
	m_latch->write_nibble_d3(data);

	m_i2cmem->write_sda(BIT(data, 4));
	m_i2cmem->write_scl(BIT(data, 5));

	update_sdb();
}


uint8_t bbcmc_state::joyport_r()
{
	uint8_t data = 0xff;

	if (m_joyport->get_card_device())
	{
		data = m_joyport->pb_r();
	}
	else if (m_exp->get_card_device())
	{
		// Mertec Companion also connects to joystick port
		data = m_exp->pb_r();
	}

	return data;
}

void bbcmc_state::joyport_w(uint8_t data)
{
	if (m_joyport->get_card_device())
	{
		m_joyport->pb_w(data);
	}
	else if (m_exp->get_card_device())
	{
		// Mertec Companion also connects to joystick port
		m_exp->pb_w(data);
	}
}


void bbcmc_state::drive_control_w(uint8_t data)
{
	// Bit       Meaning
	// -----------------
	// 7,6       Not used
	//  5        Double density select (0 = double, 1 = single)
	//  4        Side select (0 = side 0, 1 = side 1)
	//  3        Drive select 2
	//  2        Reset drive controller chip. (0 = reset controller, 1 = no reset)
	//  1        Drive select 1
	//  0        Drive select 0

	floppy_image_device *floppy = nullptr;

	// bit 0, 1, 3: drive select
	if (BIT(data, 0)) floppy = m_wdfdc->subdevice<floppy_connector>("0")->get_device();
	if (BIT(data, 1)) floppy = m_wdfdc->subdevice<floppy_connector>("1")->get_device();
	m_wdfdc->set_floppy(floppy);

	// bit 4: side select
	if (floppy)
		floppy->ss_w(BIT(data, 4));

	// bit 5: density
	m_wdfdc->dden_w(BIT(data, 5));

	// bit 2: reset
	m_wdfdc->mr_w(BIT(data, 2));
}


void bbcmc_state::trigger_reset(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	if (!state)
	{
		if (m_uservia) m_uservia->reset();
		if (m_adlc) m_adlc->reset();
		if (m_wdfdc) m_wdfdc->reset();
		if (m_exp) m_exp->reset();
	}
}


static INPUT_PORTS_START(bbcm)
	PORT_INCLUDE(bbc_config)
INPUT_PORTS_END


static void bbc_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}


/***************************************************************************

    BBC Master Compact

****************************************************************************/

void bbcmc_state::bbcmc(machine_config &config)
{
	G65SC12(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbcmc_state::bbcmc_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &bbcmc_state::bbcmc_fetch);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	RAM(config, m_ram).set_default_size("128K");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 256);
	m_screen->set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette).set_entries(16);

	SAA5050(config, m_trom, 12_MHz_XTAL / 2);

	HD6845S(config, m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_reconfigure_callback(FUNC(bbcmc_state::crtc_reconfigure));
	m_crtc->set_update_row_callback(FUNC(bbcmc_state::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(bbcmc_state::de_changed));
	m_crtc->out_hsync_callback().set(FUNC(bbcmc_state::hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(bbcmc_state::vsync_changed));

	config.set_default_layout(layout_bbcm);

	LS259(config, m_latch);
	m_latch->q_out_cb<3>().set(m_kbd, FUNC(bbc_kbd_device::write_kb_en));
	m_latch->q_out_cb<6>().set_output("capslock_led");
	m_latch->q_out_cb<7>().set_output("shiftlock_led");

	MOS6522(config, m_sysvia, 16_MHz_XTAL / 16);
	m_sysvia->readpa_handler().set(FUNC(bbcmc_state::sysvia_pa_r));
	m_sysvia->writepa_handler().set(FUNC(bbcmc_state::sysvia_pa_w));
	m_sysvia->readpb_handler().set(FUNC(bbcmc_state::sysvia_pb_r));
	m_sysvia->writepb_handler().set(FUNC(bbcmc_state::sysvia_pb_w));
	m_sysvia->cb2_handler().set([this](int state) { if (state) m_crtc->assert_light_pen_input(); });
	m_sysvia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	BBCMC_KBD(config, m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcmc_state::trigger_reset));

	SPEAKER(config, "mono").front_center();

	SN76489A(config, m_sn, 16_MHz_XTAL / 4);
	m_sn->add_route(ALL_OUTPUTS, "mono", 1.0);

	acia6850_device &acia(ACIA6850(config, "acia"));
	acia.txd_handler().set("serproc", FUNC(acorn_serproc_device::write_txd));
	acia.txd_handler().append("rs423", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("serproc", FUNC(acorn_serproc_device::write_rtsi));
	acia.rts_handler().append("rs423", FUNC(rs232_port_device::write_rts));
	acia.irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));

	acorn_serproc_device &serproc(ACORN_SERPROC(config, "serproc", 16_MHz_XTAL / 13));
	serproc.rxc_handler().set("acia", FUNC(acia6850_device::write_rxc));
	serproc.txc_handler().set("acia", FUNC(acia6850_device::write_txc));
	serproc.dcd_handler().set("acia", FUNC(acia6850_device::write_dcd));

	rs232_port_device &rs423(RS232_PORT(config, "rs423", default_rs232_devices, nullptr));
	rs423.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	rs423.cts_handler().set("acia", FUNC(acia6850_device::write_cts));

	I2C_PCD8572(config, "i2cmem", 0);

	centronics_device &centronics(CENTRONICS(config, "printer", centronics_devices, "printer"));
	centronics.ack_handler().set(m_uservia, FUNC(via6522_device::write_ca1));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(latch);

	MOS6522(config, m_uservia, 16_MHz_XTAL / 16);
	m_uservia->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_uservia->readpb_handler().set(FUNC(bbcmc_state::joyport_r)).mask(0x1f);
	m_uservia->readpb_handler().append(m_exp, FUNC(bbc_exp_slot_device::pb_r)).mask(0xe0);
	m_uservia->writepb_handler().set(FUNC(bbcmc_state::joyport_w)).mask(0x1f);
	m_uservia->writepb_handler().append(m_exp, FUNC(bbc_exp_slot_device::pb_w)).mask(0xe0);
	m_uservia->ca2_handler().set("printer", FUNC(centronics_device::write_strobe));
	m_uservia->cb1_handler().set(m_joyport, FUNC(bbc_joyport_slot_device::write_cb1));
	m_uservia->cb2_handler().set(m_joyport, FUNC(bbc_joyport_slot_device::write_cb2));
	m_uservia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	// appended for Mertec Companion that also connects to joystick port
	m_uservia->cb1_handler().append(m_exp, FUNC(bbc_exp_slot_device::write_cb1));
	m_uservia->cb2_handler().append(m_exp, FUNC(bbc_exp_slot_device::write_cb2));

	WD1772(config, m_wdfdc, 16_MHz_XTAL / 2);
	m_wdfdc->intrq_wr_callback().set(FUNC(bbcmc_state::fdc_intrq_w));
	m_wdfdc->drq_wr_callback().set(FUNC(bbcmc_state::fdc_drq_w));

	FLOPPY_CONNECTOR(config, "wdfdc:0", bbc_floppies, "35dd", bbc_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "wdfdc:1", bbc_floppies, nullptr, bbc_state::floppy_formats).enable_sound(true);

	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("network", FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(FUNC(bbcmc_state::adlc_irq_w));

	econet_device &econet(ECONET(config, "network", 0));
	econet.clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::set_cts)).invert();
	econet.data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));
	ECONET_SLOT(config, "econet", "network", econet_devices);

	BBC_EXP_SLOT(config, m_exp, 16_MHz_XTAL / 2, bbc_exp_devices, nullptr);
	m_exp->set_screen("screen");
	m_exp->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	m_exp->nmi_handler().set(FUNC(bbcmc_state::bus_nmi_w));
	m_exp->lpstb_handler().set(m_sysvia, FUNC(via6522_device::write_cb2));
	m_exp->lpstb_handler().append([this](int state) { if (state) m_crtc->assert_light_pen_input(); });
	// CB handlers for Mertec device that also plugs into joystick port.
	m_exp->cb1_handler().set(m_uservia, FUNC(via6522_device::write_cb1));
	m_exp->cb2_handler().set(m_uservia, FUNC(via6522_device::write_cb2));

	BBC_JOYPORT_SLOT(config, m_joyport, bbc_joyport_devices, nullptr);
	m_joyport->cb1_handler().set(m_uservia, FUNC(via6522_device::write_cb1));
	m_joyport->cb2_handler().set(m_uservia, FUNC(via6522_device::write_cb2));

	BBC_ROMSLOT16(config, m_rom[0x03], bbc_rom_devices, nullptr); // IC17
	BBC_ROMSLOT16(config, m_rom[0x02], bbc_rom_devices, nullptr); // IC23
	BBC_ROMSLOT32(config, m_rom[0x00], bbc_rom_devices, nullptr); // IC38
	BBC_ROMSLOT16(config, m_rom[0x08], bbc_rom_devices, nullptr); // IC29
	BBC_ROMSLOT32(config, m_rom[0x04], bbc_rom_devices, "ram").set_fixed_ram(true); // IC41 32K socket
	BBC_ROMSLOT32(config, m_rom[0x06], bbc_rom_devices, "ram").set_fixed_ram(true); // IC37 32K socket

	SOFTWARE_LIST(config, "flop_ls_mc").set_original("bbcmc_flop");
	SOFTWARE_LIST(config, "flop_ls_128s").set_original("pro128s_flop");
	SOFTWARE_LIST(config, "flop_ls_m").set_compatible("bbcm_flop");
	SOFTWARE_LIST(config, "flop_ls_b").set_compatible("bbcb_flop");
	SOFTWARE_LIST(config, "flop_ls_b_orig").set_compatible("bbcb_flop_orig");
	SOFTWARE_LIST(config, "cart_ls_m").set_original("bbcm_cart");
	SOFTWARE_LIST(config, "rom_ls").set_original("bbc_rom").set_filter("M");
}


void bbcmc_state::bbcmc_ar(machine_config &config)
{
	bbcmc(config);

	BBCMC_KBD_AR(config.replace(), m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcmc_state::trigger_reset));
}


void bbcmc_state::autoc15(machine_config &config)
{
	bbcmc(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &bbcmc_state::autoc15_mem);

	AUTOC15_KBD(config.replace(), m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcmc_state::trigger_reset));

	// replaces HD6845 to support smooth scrolling
	HD6345(config.replace(), m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_reconfigure_callback(FUNC(bbcmc_state::crtc_reconfigure));
	m_crtc->set_update_row_callback(FUNC(bbcmc_state::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(bbcmc_state::de_changed));
	m_crtc->out_hsync_callback().set(FUNC(bbcmc_state::hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(bbcmc_state::vsync_changed));

	// Autocue RAM disc
	m_exp->set_default_option("autocue").set_fixed(true);
}


ROM_START(bbcmc)
	// page 0  00000  IC38 or EXTERNAL               // page 8  20000  IC29
	// page 1  04000  IC38 or EXTERNAL               // page 9  24000  unused
	// page 2  08000  IC23                           // page 10 28000  unused
	// page 3  0c000  IC17                           // page 11 2c000  unused
	// page 4  10000  SWRAM                          // page 12 30000  unused
	// page 5  14000  SWRAM                          // page 13 34000  IC49 ADFS
	// page 6  18000  SWRAM                          // page 14 38000  IC49 BASIC
	// page 7  1c000  SWRAM                          // page 15 3c000  IC49 Utils
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "510", "MOS 5.10")
	ROMX_LOAD("mos510.ic49", 0x30000, 0x10000, CRC(9a2a6086) SHA1(094ab37b0b6437c4f1653eaa0602ef102737adb6), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "500", "MOS 5.00")
	ROMX_LOAD("mos500.ic49", 0x30000, 0x10000, CRC(f6170023) SHA1(140d002d2d9cd34b47197a2ba823505af2a84633), ROM_BIOS(1))

	ROM_COPY("rom", 0x30000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x30000, 0x4000, 0xff)

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)
ROM_END


ROM_START(bbcmc_ar)
	// page 0  00000  IC38 or EXTERNAL               // page 8  20000  IC29 Arabian
	// page 1  04000  IC38 or EXTERNAL               // page 9  24000  unused
	// page 2  08000  IC23 International             // page 10 28000  unused
	// page 3  0c000  IC17                           // page 11 2c000  unused
	// page 4  10000  SWRAM                          // page 12 30000  unused
	// page 5  14000  SWRAM                          // page 13 34000  IC49 ADFS
	// page 6  18000  SWRAM                          // page 14 38000  IC49 BASIC
	// page 7  1c000  SWRAM                          // page 15 3c000  IC49 Utils
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "511i", "MOS 5.11i")
	ROMX_LOAD("mos511.ic49", 0x30000, 0x10000, CRC(8708803c) SHA1(d2170c8b9b536f3ad84a4a603a7fe712500cc751), ROM_BIOS(0))

	ROM_COPY("rom", 0x30000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x30000, 0x4000, 0xff)

	ROM_LOAD("international16.rom", 0x08000, 0x4000, CRC(0ef527b1) SHA1(dc5149ccf588cd591a6ad47727474ef3313272ce) )
	ROM_LOAD("arabian-c22.rom"    , 0x20000, 0x4000, CRC(4f3aadff) SHA1(2bbf61ba68264ce5845aab9c54e750b0efe219c8) )

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)
ROM_END


ROM_START(pro128s)
	// page 0  00000  IC38 or EXTERNAL               // page 8  20000  IC29
	// page 1  04000  IC38 or EXTERNAL               // page 9  24000  unused
	// page 2  08000  IC23                           // page 10 28000  unused
	// page 3  0c000  IC17                           // page 11 2c000  unused
	// page 4  10000  SWRAM                          // page 12 30000  unused
	// page 5  14000  SWRAM                          // page 13 34000  IC49 ADFS
	// page 6  18000  SWRAM                          // page 14 38000  IC49 BASIC
	// page 7  1c000  SWRAM                          // page 15 3c000  IC49 Utils
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "i510c", "MOS I5.10C")
	ROMX_LOAD("mos510o.ic49", 0x30000, 0x10000, CRC(c16858d3) SHA1(ad231ed21a55e493b553703285530d1cacd3de7a), ROM_BIOS(0)) // System ROM 0258,211-01

	ROM_COPY("rom", 0x30000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x30000, 0x4000, 0xff)

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)
ROM_END


ROM_START(autoc15)
	// page 0  00000  IC38 SBII                      // page 8  20000  IC29 MODROM 0.47
	// page 1  04000  IC38 SBII                      // page 9  24000  IC49 ADFS
	// page 2  08000  IC23 APROMPT                   // page 10 28000  IC49 BASIC
	// page 3  0c000  IC17 Swedish 16K ISO           // page 11 2c000  IC49 Autocue Giant
	// page 4  10000  SWRAM                          // page 12 30000  IC49 Autocue Large
	// page 5  14000  SWRAM                          // page 13 34000  IC49 Autocue Medium
	// page 6  18000  SWRAM                          // page 14 38000  IC49 Autocue Small
	// page 7  1c000  SWRAM                          // page 15 3c000  IC49 Utils
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "510i", "MOS 5.10i")
	ROMX_LOAD("swedish_mega_29-1.ic49", 0x20000, 0x20000, CRC(67512992) SHA1(5d04b6e53a3a75af22ab10c652cceb9a63b23a6d), ROM_BIOS(0))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_LOAD("sbii-25-1-88.ic38",    0x00000, 0x8000, CRC(36af3215) SHA1(16d39f15b10b4e23e76bad23a53b4111ce877bc1))
	ROM_LOAD("aprompt.ic23",         0x08000, 0x4000, NO_DUMP)
	ROM_LOAD("swedish_16k_iso.ic17", 0x0c000, 0x4000, CRC(bd7716c0) SHA1(8a70f941f4de64d87e956e2086eb50287b8205b9))
	ROM_LOAD("modrom0_47.ic29",      0x20000, 0x4000, CRC(0d7874cb) SHA1(3f467f0b1618fb6546a2b94ca22b9f58d58bbdce))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT  COMPAT MACHINE     INPUT   CLASS          INIT       COMPANY                        FULLNAME                              FLAGS
COMP( 1986, bbcmc,      0,      bbcm,  bbcmc,      bbcm,   bbcmc_state,   init_bbc,  "Acorn Computers",             "BBC Master Compact",                 MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, bbcmc_ar,   bbcmc,  0,     bbcmc_ar,   bbcm,   bbcmc_state,   init_bbc,  "Acorn Computers",             "BBC Master Compact (Arabic)",        MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, pro128s,    bbcmc,  0,     bbcmc,      bbcm,   bbcmc_state,   init_bbc,  "Olivetti",                    "Prodest PC 128S",                    MACHINE_IMPERFECT_GRAPHICS )

// TV Production
COMP( 1988, autoc15,    bbcmc,  0,     autoc15,    bbcm,   bbcmc_state,   init_bbc,  "Autocue Ltd.",                "Autocue 1500 Teleprompter",          MACHINE_NOT_WORKING )
