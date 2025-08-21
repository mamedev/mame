// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/******************************************************************************

    BBC Master Series

    AMB15 - Master 128
    ADB12 - Master Econet Terminal
    AVC12 - Master AIV (Domesday)
    ARM1  - ARM Evaluation System

******************************************************************************/

#include "emu.h"
#include "bbc.h"
#include "acorn_serproc.h"
#include "bbc_kbd.h"

#include "bus/bbc/modem/modem.h"
#include "cpu/m6502/g65sc02.h"
#include "machine/mc146818.h"
#include "machine/wd_fdc.h"

#include "formats/uef_cas.h"
#include "formats/csw_cas.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "bbcm.lh"


namespace {

class bbcm_state : public bbc_state
{
public:
	bbcm_state(const machine_config &mconfig, device_type type, const char *tag)
		: bbc_state(mconfig, type, tag)
		, m_view_lynne(*this, "view_lynne")
		, m_view_hazel(*this, "view_hazel")
		, m_view_andy(*this, "view_andy")
		, m_view_1mhz(*this, "view_1mhz")
		, m_view_tst(*this, "view_tst")
		, m_kbd(*this, "kbd")
		, m_sn(*this, "sn")
		, m_rtc(*this, "rtc")
		, m_wdfdc(*this, "wdfdc")
		, m_adlc(*this, "mc6854")
		, m_modem(*this, "modem")
		, m_power_led(*this, "power_led")
	{ }

	void bbcm(machine_config &config);
	void bbcmt(machine_config &config);
	void bbcmet(machine_config &config);
	void bbcmaiv(machine_config &config);
	void bbcm512(machine_config &config);
	void bbcmarm(machine_config &config);
	void cfa3000(machine_config &config);
	void daisy(machine_config &config);
	void ht280(machine_config &config);
	void discmon(machine_config &config);
	void discmate(machine_config &config);
	void mpc800(machine_config &config);
	void mpc900(machine_config &config);
	void mpc900gx(machine_config &config);
	void se3010(machine_config &config);

	//static void mpc_prisma_default(device_t *device);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	memory_view m_view_lynne;
	memory_view m_view_hazel;
	memory_view m_view_andy;
	memory_view m_view_1mhz;
	memory_view m_view_tst;
	required_device<bbc_kbd_device> m_kbd;
	optional_device<sn76489a_device> m_sn;
	required_device<mc146818_device> m_rtc;
	optional_device<wd1770_device> m_wdfdc;
	required_device<mc6854_device> m_adlc;
	optional_device<bbc_modem_slot_device> m_modem;
	output_finder<> m_power_led;

	void bbcm_fetch(address_map &map) ATTR_COLD;
	void bbcm_base(address_map &map) ATTR_COLD;
	void bbcm_mem(address_map &map) ATTR_COLD;
	void bbcm_io(address_map &map) ATTR_COLD;
	void bbcmet_mem(address_map &map) ATTR_COLD;
	void bbcmet_io(address_map &map) ATTR_COLD;

	void trigger_reset(int state);

	uint8_t fetch_r(offs_t offset);
	uint8_t acccon_r();
	void acccon_w(uint8_t data);
	void romsel_w(offs_t offset, uint8_t data);
	uint8_t paged_r(offs_t offset);
	void paged_w(offs_t offset, uint8_t data);
	uint8_t fred_r(offs_t offset);
	void fred_w(offs_t offset, uint8_t data);
	uint8_t jim_r(offs_t offset);
	void jim_w(offs_t offset, uint8_t data);
	uint8_t tube_r(offs_t offset);
	void tube_w(offs_t offset, uint8_t data);
	void drive_control_w(uint8_t data);

	uint8_t sysvia_pa_r();
	void sysvia_pa_w(uint8_t data);
	void update_sdb();
	uint8_t sysvia_pb_r();
	void sysvia_pb_w(uint8_t data);

	int m_mc146818_as = 0;
	int m_mc146818_ce = 0;

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


void bbcm_state::machine_start()
{
	bbc_state::machine_start();

	m_power_led.resolve();

	save_item(NAME(m_acccon));
	save_item(NAME(m_sdb));
}


void bbcm_state::machine_reset()
{
	m_power_led = 0;
}


void bbcm_state::bbcm_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(bbcm_state::fetch_r));
}


void bbcm_state::bbcm_base(address_map &map)
{
	map(0x0000, 0x7fff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));                                      //    0000-7FFF                 Regular RAM
	map(0x3000, 0x7fff).view(m_view_lynne);                                                                              //    3000-7FFF                 20K Shadow RAM LYNNE
	m_view_lynne[0](0x3000, 0x7fff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset + 0xb000]; }));
	m_view_lynne[0](0x3000, 0x7fff).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0xb000] = data; }));
	map(0x8000, 0xbfff).rw(FUNC(bbcm_state::paged_r), FUNC(bbcm_state::paged_w));                                        //    8000-8FFF                 Paged ROM/RAM
	map(0x8000, 0x8fff).view(m_view_andy);                                                                               //    8000-8FFF                 4K RAM ANDY
	m_view_andy[0](0x8000, 0x8fff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset + 0x8000]; }));
	m_view_andy[0](0x8000, 0x8fff).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0x8000] = data; }));
	map(0xc000, 0xffff).rw(FUNC(bbcm_state::mos_r), FUNC(bbcm_state::mos_w));                                            //    C000-FFFF                 OS ROM
	map(0xc000, 0xdfff).view(m_view_hazel);                                                                              //    C000-DFFF                 8K RAM HAZEL
	m_view_hazel[0](0xc000, 0xdfff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset + 0x9000]; }));
	m_view_hazel[0](0xc000, 0xdfff).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0x9000] = data; }));
}

void bbcm_state::bbcm_mem(address_map &map)
{
	bbcm_base(map);
	map(0xfc00, 0xfeff).m(FUNC(bbcm_state::bbcm_io));                                                                    //    FC00-FFFF                 OS ROM or hardware IO
	map(0xfc00, 0xfeff).view(m_view_tst);
	m_view_tst[0](0xfc00, 0xfeff).m(FUNC(bbcm_state::bbcm_io));
	m_view_tst[0](0xfc00, 0xfeff).lr8(NAME([this](offs_t offset) { return mos_r(0x3c00 + offset); }));

}

void bbcm_state::bbcm_io(address_map &map)
{
	map(0x0000, 0x00ff).rw(FUNC(bbcm_state::fred_r), FUNC(bbcm_state::fred_w));                                          //    FC00-FCFF  Master         FRED Address Page
	map(0x0100, 0x01ff).rw(FUNC(bbcm_state::jim_r), FUNC(bbcm_state::jim_w));                                            //    FD00-FDFF  Master         JIM Address Page
	map(0x0200, 0x02ff).lr8(NAME([]() { return 0xfe; })).nopw();                                                         //    FE00-FEFF                 SHEILA Address Page
	map(0x0200, 0x0200).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));        //    FE00-FE07  6845 CRTC      Video controller
	map(0x0201, 0x0201).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0x0208, 0x0209).mirror(0x06).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));              //    FE08-FE0F  6850 ACIA      Serial controller
	map(0x0210, 0x0217).rw("serproc", FUNC(acorn_serproc_device::read), FUNC(acorn_serproc_device::write));              //    FE10-FE17  Serial ULA     Serial system chip
	map(0x0218, 0x021b).mirror(0x04).rw("upd7002", FUNC(upd7002_device::read), FUNC(upd7002_device::write));             //    FE18-FE1F  uPD7002        Analogue to digital converter
	map(0x0220, 0x0223).w(FUNC(bbcm_state::video_ula_w));                                                                // W: FE20-FE23  Video ULA      Video system chip
	map(0x0224, 0x0227).w(FUNC(bbcm_state::drive_control_w));                                                            // W: FE24-FE27  FDC Latch      1770 Control latch
	map(0x0228, 0x022f).rw(m_wdfdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write));                              //    FE28-FE2F  1770 FDC       Floppy disc controller
	map(0x0230, 0x0233).w(FUNC(bbcm_state::romsel_w));                                                                   // W: FE30-FE33  ROMSEL         ROM Select
	map(0x0234, 0x0237).rw(FUNC(bbcm_state::acccon_r), FUNC(bbcm_state::acccon_w));                                      //    FE34-FE37  ACCCON         ACCCON select register
	map(0x0238, 0x023b).lr8(NAME([this]() { econet_int_enable(0); return 0xfe; }));                                      // R: FE38-FE3B  INTOFF         ECONET Interrupt Off
	map(0x0238, 0x023b).lw8(NAME([this](uint8_t data) { econet_int_enable(0); }));                                       // W: FE38-FE3B  INTOFF         ECONET Interrupt Off
	map(0x023c, 0x023f).lr8(NAME([this]() { econet_int_enable(1); return 0xfe; }));                                      // R: FE3C-FE3F  INTON          ECONET Interrupt On
	map(0x023c, 0x023f).lw8(NAME([this](uint8_t data) { econet_int_enable(1); }));                                       // W: FE3C-FE3F  INTON          ECONET Interrupt On
	map(0x0240, 0x024f).mirror(0x10).m(m_sysvia, FUNC(via6522_device::map));                                             //    FE40-FE5F  6522 VIA       SYSTEM VIA
	map(0x0260, 0x026f).mirror(0x10).m(m_uservia, FUNC(via6522_device::map));                                            //    FE60-FE7F  6522 VIA       USER VIA
	map(0x0280, 0x028f).mirror(0x10).rw(m_modem, FUNC(bbc_modem_slot_device::read), FUNC(bbc_modem_slot_device::write)); //    FE80-FE9F  Int. Modem     Int. Modem
	map(0x02a0, 0x02a3).mirror(0x1c).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));                  //    FEA0-FEBF  68B54 ADLC     ECONET controller
	map(0x02e0, 0x02ff).rw(FUNC(bbcm_state::tube_r), FUNC(bbcm_state::tube_w));                                          //    FEE0-FEFF  Tube ULA       Tube system interface
}


void bbcm_state::bbcmet_mem(address_map &map)
{
	bbcm_base(map);
	map(0xfc00, 0xfeff).m(FUNC(bbcm_state::bbcmet_io));                                                                  //    FC00-FFFF                 OS ROM or hardware IO
	map(0xfc00, 0xfeff).view(m_view_tst);
	m_view_tst[0](0xfc00, 0xfeff).m(FUNC(bbcm_state::bbcmet_io));
	m_view_tst[0](0xfc00, 0xfeff).lr8(NAME([this](offs_t offset) { return mos_r(0x3c00 + offset); }));
}

void bbcm_state::bbcmet_io(address_map &map)
{
	map(0x0000, 0x00ff).noprw();                                                                                         //    FC00-FCFF                 FRED Address Page
	map(0x0100, 0x01ff).noprw();                                                                                         //    FD00-FDFF                 JIM Address Page
	map(0x0200, 0x02ff).lr8(NAME([]() { return 0xfe; })).nopw();                                                         //    FE00-FEFF                 SHEILA Address Page
	map(0x0200, 0x0200).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));        //    FE00-FE07  6845 CRTC      Video controller
	map(0x0201, 0x0201).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0x0220, 0x0223).w(FUNC(bbcm_state::video_ula_w));                                                                // W: FE20-FE23  Video ULA      Video system chip
	map(0x0230, 0x0233).w(FUNC(bbcm_state::romsel_w));                                                                   // W: FE30-FE33  ROMSEL         ROM Select
	map(0x0234, 0x0237).rw(FUNC(bbcm_state::acccon_r), FUNC(bbcm_state::acccon_w));                                      //    FE34-FE37  ACCCON         ACCCON select register
	map(0x0238, 0x023b).lr8(NAME([this]() { econet_int_enable(0); return 0xfe; }));                                      // R: FE38-FE3B  INTOFF         ECONET Interrupt Off
	map(0x0238, 0x023b).lw8(NAME([this](uint8_t data) { econet_int_enable(0); }));                                       // W: FE38-FE3B  INTOFF         ECONET Interrupt Off
	map(0x023c, 0x023f).lr8(NAME([this]() { econet_int_enable(1); return 0xfe; }));                                      // R: FE3C-FE3F  INTON          ECONET Interrupt On
	map(0x023c, 0x023f).lw8(NAME([this](uint8_t data) { econet_int_enable(1); }));                                       // W: FE3C-FE3F  INTON          ECONET Interrupt On
	map(0x0240, 0x024f).mirror(0x10).m(m_sysvia, FUNC(via6522_device::map));                                             //    FE40-FE5F  6522 VIA       SYSTEM VIA
	map(0x02a0, 0x02a3).mirror(0x1c).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));                  //    FEA0-FEBF  68B54 ADLC     ECONET controller
}


uint8_t bbcm_state::fetch_r(offs_t offset)
{
	if (BIT(m_acccon, ACCCON_X) || (BIT(m_acccon, ACCCON_E) && offset >= 0xc000 && offset <= 0xdfff))
		m_view_lynne.select(0);
	else
		m_view_lynne.disable();

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}


void bbcm_state::romsel_w(offs_t offset, uint8_t data)
{
	// ROMSEL - FE30 write only
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

	// pass ROMSEL to internal expansion board
	if (m_internal)
		m_internal->romsel_w(offset, data);
}


uint8_t bbcm_state::acccon_r()
{
	return m_acccon;
}

void bbcm_state::acccon_w(uint8_t data)
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
	m_irqs->in_w<6>(BIT(m_acccon, ACCCON_IRR));

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


uint8_t bbcm_state::paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (m_romsel)
	{
	case 0: case 1: case 2: case 3:
		if (m_cart[BIT(m_romsel, 1)])
		{
			if (m_cart[BIT(m_romsel, 1)]->present())
				data = m_cart[BIT(m_romsel, 1)]->read(offset, 0, 0, m_romsel & 0x01, 1, 0);
			else
				data = bus_video_data();
		}
		else
		{
			data = m_region_rom->base()[offset + (m_romsel << 14)];
		}
		break;

	default:
		if (m_internal && m_internal->overrides_rom())
		{
			data = m_internal->paged_r(offset);
		}
		else
		{
			switch (m_romsel)
			{
			case 4: case 5: case 6: case 7:
				// 32K sockets
				if (m_rom[m_romsel & 0x0e] && m_rom[m_romsel & 0x0e]->present())
					data = m_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
				else
					data = m_region_rom->base()[offset + (m_romsel << 14)];
				break;
			default:
				// 16K sockets
				if (m_rom[m_romsel] && m_rom[m_romsel]->present())
					data = m_rom[m_romsel]->read(offset);
				else
					data = m_region_rom->base()[offset + (m_romsel << 14)];
				break;
			}
		}
		break;
	}

	return data;
}

void bbcm_state::paged_w(offs_t offset, uint8_t data)
{
	switch (m_romsel)
	{
	case 0: case 1: case 2: case 3:
		if (m_cart[BIT(m_romsel, 1)])
			m_cart[BIT(m_romsel, 1)]->write(offset, data, 0, 0, m_romsel & 0x01, 1, 0);
		else
			m_region_rom->base()[offset + (m_romsel << 14)] = data;
		break;

	default:
		if (m_internal && m_internal->overrides_rom())
		{
			m_internal->paged_w(offset, data);
		}
		else
		{
			switch (m_romsel)
			{
			case 4: case 5: case 6: case 7:
				// 32K sockets
				if (m_rom[m_romsel & 0x0e])
					m_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
				break;
			default:
				// 16K sockets
				if (m_rom[m_romsel])
					m_rom[m_romsel]->write(offset, data);
				break;
			}
		}
		break;
	}
}


uint8_t bbcm_state::fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (BIT(m_acccon, ACCCON_IFJ))
		data = m_cart[m_romsel & 1]->read(offset, 1, 0, m_romsel & 1, 0, 0);
	else
		data = m_1mhzbus->fred_r(offset);

	return data;
}

void bbcm_state::fred_w(offs_t offset, uint8_t data)
{
	if (BIT(m_acccon, ACCCON_IFJ))
		m_cart[m_romsel & 1]->write(offset, data, 1, 0, m_romsel & 1, 0, 0);
	else
		m_1mhzbus->fred_w(offset, data);
}


uint8_t bbcm_state::jim_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (BIT(m_acccon, ACCCON_IFJ))
		data = m_cart[m_romsel & 1]->read(offset, 0, 1, m_romsel & 1, 0, 0);
	else
		data = m_1mhzbus->jim_r(offset);

	return data;
}

void bbcm_state::jim_w(offs_t offset, uint8_t data)
{
	if (BIT(m_acccon, ACCCON_IFJ))
		m_cart[m_romsel & 1]->write(offset, data, 0, 1, m_romsel & 1, 0, 0);
	else
		m_1mhzbus->jim_w(offset, data);
}


uint8_t bbcm_state::tube_r(offs_t offset)
{
	uint8_t data = 0xfe;

	if (BIT(m_acccon, ACCCON_ITU))
		data = m_intube->host_r(offset);
	else
		data = m_extube->host_r(offset);

	return data;
}

void bbcm_state::tube_w(offs_t offset, uint8_t data)
{
	if (BIT(m_acccon, ACCCON_ITU))
		m_intube->host_w(offset, data);
	else
		m_extube->host_w(offset, data);
}


uint8_t bbcm_state::sysvia_pa_r()
{
	update_sdb();

	return m_sdb;
}

void bbcm_state::sysvia_pa_w(uint8_t data)
{
	m_sdb = data;

	update_sdb();
}


void bbcm_state::update_sdb()
{
	uint8_t const latch = m_latch->output_state();

	// rtc
	if (m_mc146818_ce)
	{
		// if data select is set then access the data in the MC146818
		if (BIT(latch, 2)) // DS
		{
			if (BIT(latch, 1)) // RD
			{
				m_sdb = m_rtc->data_r();
			}
			else
			{
				m_rtc->data_w(m_sdb);
			}
		}
		// if address select is set then set the address in the MC146818
		if (m_mc146818_as)
		{
			m_rtc->address_w(m_sdb);
		}
	}

	// keyboard
	m_sdb = m_kbd->read(m_sdb);
}


uint8_t bbcm_state::sysvia_pb_r()
{
	uint8_t data = 0xff;

	if (m_analog)
	{
		data &= ~0x30;
		data |= m_analog->pb_r();
	}

	return data;
}

void bbcm_state::sysvia_pb_w(uint8_t data)
{
	m_latch->write_nibble_d3(data);

	if (m_analog)
	{
		m_analog->pb_w(data & 0x30);
	}

	// set MC146818 CE and AS lines
	m_mc146818_ce = BIT(data, 6);
	m_mc146818_as = BIT(data, 7);

	update_sdb();
}


void bbcm_state::drive_control_w(uint8_t data)
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


void bbcm_state::trigger_reset(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	if (!state)
	{
		if (m_uservia) m_uservia->reset();
		if (m_adlc) m_adlc->reset();
		if (m_rtc) m_rtc->reset();
		if (m_wdfdc) m_wdfdc->reset();
		if (m_1mhzbus) m_1mhzbus->reset();
		if (m_intube) m_intube->reset();
		if (m_extube) m_extube->reset();
		if (m_internal) m_internal->reset();
		if (m_modem) m_modem->reset();
		if (m_cart[0]) m_cart[0]->reset();
		if (m_cart[1]) m_cart[1]->reset();
	}
}


static INPUT_PORTS_START(bbcm)
	PORT_INCLUDE(bbc_config)
INPUT_PORTS_END


static void bbc_floppies(device_slot_interface &device)
{
	device.option_add("525sssd", FLOPPY_525_SSSD);
	device.option_add("525sd",   FLOPPY_525_SD);
	device.option_add("525qd",   FLOPPY_525_QD);
	device.option_add("35dd",    FLOPPY_35_DD);
}


static const char *const bbc_sample_names[] =
{
	"*bbc",
	"motoroff",
	"motoron",
	nullptr
};


/***************************************************************************

    BBC Master Series

****************************************************************************/

void bbcm_state::bbcmet(machine_config &config)
{
	G65SC12(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbcm_state::bbcmet_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &bbcm_state::bbcm_fetch);
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	RAM(config, m_ram).set_default_size("128K");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 624, 0, 512);
	m_screen->set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette).set_entries(16);

	SAA5050(config, m_trom, 12_MHz_XTAL / 2);

	HD6845S(config, m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_reconfigure_callback(FUNC(bbcm_state::crtc_reconfigure));
	m_crtc->set_update_row_callback(FUNC(bbcm_state::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(bbcm_state::de_changed));
	m_crtc->out_hsync_callback().set(FUNC(bbcm_state::hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(bbcm_state::vsync_changed));

	config.set_default_layout(layout_bbcm);

	LS259(config, m_latch);
	m_latch->q_out_cb<0>().set([this](int state) { if (!state) m_sn->write(m_sdb); });
	m_latch->q_out_cb<3>().set(m_kbd, FUNC(bbc_kbd_device::write_kb_en));
	m_latch->q_out_cb<6>().set_output("capslock_led");
	m_latch->q_out_cb<7>().set_output("shiftlock_led");

	MOS6522(config, m_sysvia, 16_MHz_XTAL / 16);
	m_sysvia->readpa_handler().set(FUNC(bbcm_state::sysvia_pa_r));
	m_sysvia->writepa_handler().set(FUNC(bbcm_state::sysvia_pa_w));
	m_sysvia->readpb_handler().set(FUNC(bbcm_state::sysvia_pb_r));
	m_sysvia->writepb_handler().set(FUNC(bbcm_state::sysvia_pb_w));
	m_sysvia->cb2_handler().set([this](int state) { if (state) m_crtc->assert_light_pen_input(); });
	m_sysvia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	BBCM_KBD(config, m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcm_state::trigger_reset));

	SPEAKER(config, "mono").front_center();

	SN76489A(config, m_sn, 16_MHz_XTAL / 4);
	m_sn->add_route(ALL_OUTPUTS, "mono", 1.0);

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_irqs, FUNC(input_merger_device::in_w<7>));

	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("network", FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(FUNC(bbcm_state::adlc_irq_w));

	econet_device &econet(ECONET(config, "network", 0));
	econet.clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::set_cts)).invert();
	econet.data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));
	ECONET_SLOT(config, "econet", "network", econet_devices);

	BBCM_CARTSLOT(config, m_cart[0], 16_MHz_XTAL, bbcm_cart, nullptr);
	m_cart[0]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<10>));
	m_cart[0]->nmi_handler().set(FUNC(bbcm_state::bus_nmi_w));
	BBCM_CARTSLOT(config, m_cart[1], 16_MHz_XTAL, bbcm_cart, nullptr);
	m_cart[1]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<11>));
	m_cart[1]->nmi_handler().set(FUNC(bbcm_state::bus_nmi_w));

	BBC_ROMSLOT16(config, m_rom[0x08], bbc_rom_devices, nullptr); // IC27
}


void bbcm_state::bbcm(machine_config &config)
{
	bbcmet(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &bbcm_state::bbcm_mem);

	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(bbc_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 1.0);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(bbc_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED);
	m_cassette->set_interface("bbc_cass");

	acia6850_device &acia(ACIA6850(config, "acia"));
	acia.txd_handler().set("serproc", FUNC(acorn_serproc_device::write_txd));
	acia.rts_handler().set("serproc", FUNC(acorn_serproc_device::write_rtsi));
	acia.irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));

	acorn_serproc_device &serproc(ACORN_SERPROC(config, "serproc", 16_MHz_XTAL / 13));
	serproc.rxc_handler().set("acia", FUNC(acia6850_device::write_rxc));
	serproc.txc_handler().set("acia", FUNC(acia6850_device::write_txc));
	serproc.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	serproc.dcd_handler().set("acia", FUNC(acia6850_device::write_dcd));
	serproc.ctso_handler().set("acia", FUNC(acia6850_device::write_cts));
	serproc.dout_handler().set("rs423", FUNC(rs232_port_device::write_txd));
	serproc.rtso_handler().set("rs423", FUNC(rs232_port_device::write_rts));
	serproc.casmo_handler().set(FUNC(bbcm_state::cassette_motor));
	serproc.casin_handler("cassette", FUNC(cassette_image_device::input));
	serproc.casout_handler("cassette", FUNC(cassette_image_device::output));

	rs232_port_device &rs423(RS232_PORT(config, "rs423", default_rs232_devices, nullptr));
	rs423.rxd_handler().set("serproc", FUNC(acorn_serproc_device::write_din));
	rs423.cts_handler().set("serproc", FUNC(acorn_serproc_device::write_ctsi));

	MOS6522(config, m_uservia, 16_MHz_XTAL / 16);
	m_uservia->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_uservia->readpb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_r));
	m_uservia->writepb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_w));
	m_uservia->ca2_handler().set("printer", FUNC(centronics_device::write_strobe));
	m_uservia->cb1_handler().set(m_userport, FUNC(bbc_userport_slot_device::write_cb1));
	m_uservia->cb2_handler().set(m_userport, FUNC(bbc_userport_slot_device::write_cb2));
	m_uservia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	centronics_device &centronics(CENTRONICS(config, "printer", centronics_devices, "printer"));
	centronics.ack_handler().set(m_uservia, FUNC(via6522_device::write_ca1));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(latch);

	upd7002_device &upd7002(UPD7002(config, "upd7002", 16_MHz_XTAL / 16));
	upd7002.set_get_analogue_callback(FUNC(bbcm_state::get_analogue_input));
	upd7002.set_eoc_callback(m_sysvia, FUNC(via6522_device::write_cb1));

	BBC_ANALOGUE_SLOT(config, m_analog, bbc_analogue_devices, nullptr);
	m_analog->lpstb_handler().set(m_sysvia, FUNC(via6522_device::write_cb2));
	m_analog->lpstb_handler().append([this](int state) { if (state) m_crtc->assert_light_pen_input(); });

	WD1770(config, m_wdfdc, 16_MHz_XTAL / 2);
	m_wdfdc->intrq_wr_callback().set(FUNC(bbcm_state::fdc_intrq_w));
	m_wdfdc->drq_wr_callback().set(FUNC(bbcm_state::fdc_drq_w));

	FLOPPY_CONNECTOR(config, "wdfdc:0", bbc_floppies, "525qd", bbc_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "wdfdc:1", bbc_floppies, "525qd", bbc_state::floppy_formats).enable_sound(true);

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, 16_MHz_XTAL / 16, bbcm_1mhzbus_devices, nullptr);
	m_1mhzbus->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_1mhzbus->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	m_1mhzbus->nmi_handler().set(FUNC(bbcm_state::bus_nmi_w));

	BBC_TUBE_SLOT(config, m_intube, bbc_intube_devices, nullptr);
	m_intube->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<4>));

	BBC_TUBE_SLOT(config, m_extube, bbc_extube_devices, nullptr);
	m_extube->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<5>));

	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, nullptr);
	m_userport->cb1_handler().set(m_uservia, FUNC(via6522_device::write_cb1));
	m_userport->cb2_handler().set(m_uservia, FUNC(via6522_device::write_cb2));

	BBC_ROMSLOT32(config, m_rom[0x04], bbc_rom_devices, "ram").set_fixed_ram(true); // IC41 32K socket
	BBC_ROMSLOT32(config, m_rom[0x06], bbc_rom_devices, "ram").set_fixed_ram(true); // IC37 32K socket

	BBC_INTERNAL_SLOT(config, m_internal, 16_MHz_XTAL, bbcm_internal_devices, nullptr);
	m_internal->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<8>));
	m_internal->nmi_handler().set(FUNC(bbcm_state::bus_nmi_w));

	BBC_MODEM_SLOT(config, m_modem, 16_MHz_XTAL / 16, bbcm_modem_devices, nullptr);
	m_modem->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<9>));

	SOFTWARE_LIST(config, "cass_ls").set_original("bbc_cass").set_filter("A,B,M");
	SOFTWARE_LIST(config, "flop_ls_m").set_original("bbcm_flop");
	SOFTWARE_LIST(config, "cart_ls_m").set_original("bbcm_cart");
	SOFTWARE_LIST(config, "flop_ls_b").set_compatible("bbcb_flop");
	SOFTWARE_LIST(config, "flop_ls_b_orig").set_compatible("bbcb_flop_orig");
	SOFTWARE_LIST(config, "rom_ls").set_original("bbc_rom").set_filter("M");
	SOFTWARE_LIST(config, "hdd_ls").set_original("bbc_hdd").set_filter("M");
}


void bbcm_state::bbcmt(machine_config &config)
{
	bbcm(config);

	// Acorn 65C102 co-processor
	m_intube->set_default_option("65c102").set_fixed(true);
}


void bbcm_state::bbcmaiv(machine_config &config)
{
	bbcm(config);

	// Acorn 65C102 co-processor
	m_intube->set_default_option("65c102").set_fixed(true);

	// Philips VP415 Laserdisc player
	m_modem->set_default_option("scsiaiv").set_fixed(true);

	// Acorn Tracker Ball
	m_userport->set_default_option("tracker");
}


void bbcm_state::bbcm512(machine_config &config)
{
	bbcm(config);

	// Acorn Intel 80186 co-processor
	m_intube->set_default_option("80186").set_fixed(true);

	// Acorn Mouse
	m_userport->set_default_option("m512mouse");
}


void bbcm_state::bbcmarm(machine_config &config)
{
	bbcm(config);

	// Acorn ARM co-processor
	m_extube->set_default_option("arm").set_fixed(true);

	// Acorn Winchester Disc
	m_1mhzbus->set_default_option("awhd");
}


void bbcm_state::cfa3000(machine_config &config)
{
	bbcm(config);

	m_wdfdc->subdevice<floppy_connector>("0")->set_default_option(nullptr);
	m_wdfdc->subdevice<floppy_connector>("1")->set_default_option(nullptr);

	// LK18 and LK19 are set to enable rom, disabling ram
	m_rom[0x04]->set_default_option("rom").set_fixed(true);
	m_rom[0x06]->set_default_option("rom").set_fixed(true);

	// keyboard
	m_userport->set_default_option("cfa3000kbd").set_fixed(true);

	// option board
	m_1mhzbus->set_default_option("cfa3000opt").set_fixed(true);

	// analogue dials/sensors
	m_analog->set_default_option("cfa3000a").set_fixed(true);

	// software lists
	config.device_remove("cass_ls");
	config.device_remove("flop_ls_m");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");
}


void bbcm_state::daisy(machine_config &config)
{
	bbcm(config);

	// Acorn 65C102 co-processor
	m_intube->set_default_option("65c102").set_fixed(true);

	// Start Interface / 64K SRAM / Analog Board
	//m_1mhzbus->set_default_option("daisy").set_fixed(true);

	// LK18 and LK19 are set to enable ROM, disabling RAM
	m_rom[0x04]->set_default_option("rom").set_fixed(true);
	m_rom[0x06]->set_default_option("rom").set_fixed(true);
}


void bbcm_state::discmon(machine_config &config)
{
	bbcm(config);

	// TODO: Add coin slot

	config.device_remove("cass_ls");
	config.device_remove("flop_ls_m");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");
}


void bbcm_state::discmate(machine_config &config)
{
	bbcm(config);

	// TODO: Add Sony CDK-3000PII Auto Disc Loader

	// TODO: Add interface boards connected to cassette and RS423

	config.device_remove("cass_ls");
	config.device_remove("flop_ls_m");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");
}


void bbcm_state::ht280(machine_config &config)
{
	bbcm(config);

	HT280_KBD(config.replace(), m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcm_state::trigger_reset));

	// Z80 Controller Board
	//m_1mhzbus->set_default_option("ht280").set_fixed(true);

	// LK18 and LK19 are set to enable rom, disabling ram
	m_rom[0x04]->set_default_option("rom").set_fixed(true);
	m_rom[0x06]->set_default_option("rom").set_fixed(true);

	// cartridge sockets
	config.device_remove("cartslot1");
	config.device_remove("cartslot2");
}


//void bbcm_state::mpc_prisma_default(device_t* device)
//{
//	device->subdevice<bbc_1mhzbus_slot_device>("1mhzbus")->set_default_option("awhd");
//	device->subdevice<bbc_1mhzbus_slot_device>("1mhzbus")->set_fixed(true);
//}


void bbcm_state::mpc800(machine_config &config)
{
	bbcm(config);

	// Acorn 65C102 co-processor
	m_intube->set_default_option("65c102").set_fixed(true);

	// Prisma-2
	//m_1mhzbus->set_default_option("prisma2").set_fixed(true);
	//m_1mhzbus->set_option_machine_config("prisma2", mpc_prisma_default);

	// Mouse (AMX compatible)
	m_userport->set_default_option("amxmouse").set_fixed(true);

	// cartridge sockets
	config.device_remove("cartslot1");
	config.device_remove("cartslot2");
}


void bbcm_state::mpc900(machine_config &config)
{
	mpc800(config);

	// Prisma-3
	//m_1mhzbus->set_default_option("prisma3").set_fixed(true);
	//m_1mhzbus->set_option_machine_config("prisma3", mpc_prisma_default);
}


void bbcm_state::mpc900gx(machine_config &config)
{
	mpc800(config);

	// Prisma-3 Plus
	//m_1mhzbus->set_default_option("prisma3p").set_fixed(true);
	//m_1mhzbus->set_option_machine_config("prisma3p", mpc_prisma_default);
}


void bbcm_state::se3010(machine_config &config)
{
	bbcm(config);

	SE3010_KBD(config.replace(), m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcm_state::trigger_reset));

	// cartridge sockets
	config.device_remove("cartslot1");
	config.device_remove("cartslot2");
}


ROM_START(bbcm)
	// page 0  00000  SK3 Rear Cartridge bottom 16K      // page 8  20000  IC27 ANFS
	// page 1  04000  SK3 Rear Cartridge top 16K         // page 9  24000  IC24 DFS + SRAM
	// page 2  08000  SK4 Front Cartridge bottom 16K     // page 10 28000  IC24 Viewsheet
	// page 3  0C000  SK4 Front Cartridge top 16K        // page 11 2C000  IC24 Edit
	// page 4  10000  IC41 SWRAM or bottom 16K           // page 12 30000  IC24 BASIC
	// page 5  14000  IC41 SWRAM or top 16K              // page 13 34000  IC24 ADFS
	// page 6  18000  IC37 SWRAM or bottom 16K           // page 14 38000  IC24 View + MOS code
	// page 7  1C000  IC37 SWRAM or top 16K              // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "320", "MOS 3.20")
	ROMX_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "350", "MOS 3.50")
	ROMX_LOAD("mos350.ic24", 0x20000, 0x20000, CRC(141027b9) SHA1(85211b5bc7c7a269952d2b063b7ec0e1f0196803), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "329", "MOS 3.29")
	ROMX_LOAD("mos329.ic24", 0x20000, 0x20000, CRC(8dd7338b) SHA1(4604203c70c04a9fd003103deec438fc5bd44839), ROM_BIOS(2))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	//ROM_LOAD("anfs425-2201351.rom", 0x20000, 0x4000, CRC(c2a6655e) SHA1(14f75d36ffe9af14aaac42df55b4fe3729ba75cf))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROMX_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94), ROM_BIOS(0))
	ROMX_LOAD("mos350.cmos", 0x00, 0x40, CRC(e84c1854) SHA1(f3cb7f12b7432caba28d067f01af575779220aac), ROM_BIOS(1))
	ROMX_LOAD("mos350.cmos", 0x00, 0x40, CRC(e84c1854) SHA1(f3cb7f12b7432caba28d067f01af575779220aac), ROM_BIOS(2))
ROM_END


#define rom_bbcmt rom_bbcm
#define rom_bbcm512 rom_bbcm


ROM_START(bbcmaiv)
	// page 0  00000  SK3 Rear Cartridge bottom 16K      // page 8  20000  IC27 VFS
	// page 1  04000  SK3 Rear Cartridge top 16K         // page 9  24000  IC24 DFS + SRAM
	// page 2  08000  SK4 Front Cartridge bottom 16K     // page 10 28000  IC24 Viewsheet
	// page 3  0C000  SK4 Front Cartridge top 16K        // page 11 2C000  IC24 Edit
	// page 4  10000  IC41 SWRAM or bottom 16K           // page 12 30000  IC24 BASIC
	// page 5  14000  IC41 SWRAM or top 16K              // page 13 34000  IC24 ADFS
	// page 6  18000  IC37 SWRAM or bottom 16K           // page 14 38000  IC24 View + MOS code
	// page 7  1C000  IC37 SWRAM or top 16K              // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_LOAD("vfs170.rom", 0x20000, 0x4000, CRC(b124a0bb) SHA1(ba31c757815cf470402d7829a70a0e1d3fb1355b) )

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos320aiv.cmos", 0x0e, 0x32, BAD_DUMP CRC(b9ae42a1) SHA1(abf3e94b013f24027ca36c96720963c3411e93f8))
ROM_END


ROM_START(bbcmet)
	// page 0  00000  SK3 Rear Cartridge bottom 16K      // page 8  20000
	// page 1  04000  SK3 Rear Cartridge top 16K         // page 9  24000  IC24 BASIC
	// page 2  08000  SK4 Front Cartridge bottom 16K     // page 10 28000  IC24 ANFS
	// page 3  0C000  SK4 Front Cartridge top 16K        // page 11 2C000  IC24 MOS code
	// page 4  10000  IC41 SWRAM or bottom 16K           // page 12 30000  IC24 unused
	// page 5  14000  IC41 SWRAM or top 16K              // page 13 34000  IC24 BASIC
	// page 6  18000  IC37 SWRAM or bottom 16K           // page 14 38000  IC24 ANFS
	// page 7  1C000  IC37 SWRAM or top 16K              // page 15 3C000  IC24 MOS code
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("mos400.ic24", 0x20000, 0x10000, CRC(81729034) SHA1(d4bc2c7f5e66b5298786138f395908e70c772971))
	ROM_RELOAD(             0x30000, 0x10000) // mirror

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos400.cmos", 0x0e, 0x32, BAD_DUMP CRC(fff41cc5) SHA1(3607568758f90b3bd6c7dc9533e2aa24f9806ff3))
ROM_END


ROM_START(bbcmarm)
	// page 0  00000  SK3 Rear Cartridge bottom 16K      // page 8  20000  IC27 ANFS
	// page 1  04000  SK3 Rear Cartridge top 16K         // page 9  24000  IC24 DFS + SRAM
	// page 2  08000  SK4 Front Cartridge bottom 16K     // page 10 28000  IC24 Viewsheet
	// page 3  0C000  SK4 Front Cartridge top 16K        // page 11 2C000  IC24 Edit
	// page 4  10000  IC41 SWRAM or bottom 16K           // page 12 30000  IC24 BASIC
	// page 5  14000  IC41 SWRAM or top 16K              // page 13 34000  IC24 ADFS
	// page 6  18000  IC37 SWRAM or bottom 16K           // page 14 38000  IC24 View + MOS code
	// page 7  1C000  IC37 SWRAM or top 16K              // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos320arm.cmos", 0x00, 0x40, CRC(56117257) SHA1(ed98563bef18f9d2a0b2d941cd20823d760fb127))
ROM_END


ROM_START(cfa3000)
	// page 0  00000  SK3 Rear Cartridge bottom 16K      // page 8  20000  IC27
	// page 1  04000  SK3 Rear Cartridge top 16K         // page 9  24000  IC24
	// page 2  08000  SK4 Front Cartridge bottom 16K     // page 10 28000  IC24
	// page 3  0C000  SK4 Front Cartridge top 16K        // page 11 2C000  IC24
	// page 4  10000  IC41 SWRAM or bottom 16K           // page 12 30000  IC24
	// page 5  14000  IC41 SWRAM or top 16K              // page 13 34000  IC24 DFS
	// page 6  18000  IC37 SWRAM or bottom 16K           // page 14 38000  IC24 BASIC
	// page 7  1C000  IC37 SWRAM or top 16K              // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "103", "Issue 10.3")
	ROMX_LOAD("cfa3000_3m4_iss10.3.ic41",           0x10000, 0x08000, CRC(ecb385ab) SHA1(eafa9b34cb1cf63790f74332bb7d85ee356b6973), ROM_BIOS(0))
	ROMX_LOAD("cfa3000_sm_iss10.3.ic37",            0x18000, 0x08000, CRC(c07aee5f) SHA1(1994e3755dc15d1ea7e105bc19cd57893b719779), ROM_BIOS(0))
	ROMX_LOAD("acorn_mos,tinsley_64k,iss10.3.ic24", 0x20000, 0x10000, CRC(4413c3ee) SHA1(76d0462b4dabe2461010fce2341570ff3d606d54), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "102", "Issue 10.2")
	ROMX_LOAD("cfa3000_3m4_iss10.2.ic41",           0x10000, 0x08000, CRC(ecb385ab) SHA1(eafa9b34cb1cf63790f74332bb7d85ee356b6973), ROM_BIOS(1))
	ROMX_LOAD("cfa3000_sm_iss10.2.ic37",            0x18000, 0x08000, CRC(e733d5b3) SHA1(07e89943c6ac0953b75686ee06e947f33119dbed), ROM_BIOS(1))
	ROMX_LOAD("acorn_mos,tinsley_64k,iss10.2.ic24", 0x20000, 0x10000, CRC(4413c3ee) SHA1(76d0462b4dabe2461010fce2341570ff3d606d54), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "90", "Issue 9")
	ROMX_LOAD("cfa3000_3m4_iss9.ic41",              0x10000, 0x08000, CRC(a4bd5d53) SHA1(90747ff7bd81ac1e124bae964c206d8df163e1d6), ROM_BIOS(2))
	ROMX_LOAD("cfa3000_sm_iss9.ic37",               0x18000, 0x08000, CRC(559d1fae) SHA1(271e1ab9b53e82028e92e7cdb8c517df06e76477), ROM_BIOS(2))
	ROMX_LOAD("acorn_mos,tinsley_64k,iss9.ic24",    0x20000, 0x10000, CRC(4413c3ee) SHA1(76d0462b4dabe2461010fce2341570ff3d606d54), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "70", "Issue 7")
	ROMX_LOAD("cfa3000_3m4_iss7.ic41",              0x10000, 0x08000, CRC(a0b32288) SHA1(83b047e9eb35f0644bd8f0acb1a56e1428bacc0b), ROM_BIOS(3))
	ROMX_LOAD("cfa3000_sm_iss7.ic37",               0x18000, 0x08000, CRC(3cd42bbd) SHA1(17f6c66039d20a364cc9e1377c7ced14d5302603), ROM_BIOS(3))
	ROMX_LOAD("acorn_mos,tinsley_64k,iss7.ic24",    0x20000, 0x10000, CRC(4413c3ee) SHA1(76d0462b4dabe2461010fce2341570ff3d606d54), ROM_BIOS(3))

	ROM_COPY("rom", 0x20000, 0x30000, 0x10000) // mirror

	ROM_COPY("rom", 0x20000, 0x40000, 0x04000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos350.cmos", 0x00, 0x40, CRC(e84c1854) SHA1(f3cb7f12b7432caba28d067f01af575779220aac))
ROM_END


ROM_START(daisy)
	// page 0  00000  SK3 Rear Cartridge bottom 16K      // page 8  20000  IC27 HiBASIC3
	// page 1  04000  SK3 Rear Cartridge top 16K         // page 9  24000  IC24 DFS + SRAM
	// page 2  08000  SK4 Front Cartridge bottom 16K     // page 10 28000  IC24 Viewsheet
	// page 3  0C000  SK4 Front Cartridge top 16K        // page 11 2C000  IC24 Edit
	// page 4  10000  IC41 SIMDIST                       // page 12 30000  IC24 BASIC
	// page 5  14000  IC41 DHRFDSY                       // page 13 34000  IC24 ADFS
	// page 6  18000  IC37 IRFDSY                        // page 14 38000  IC24 View + MOS code
	// page 7  1C000  IC37 DAISY                         // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_LOAD("simdist_2_22-2-89_dhrfdsy.rom", 0x10000, 0x8000, CRC(8a9d9c4a) SHA1(278c5c63e06359601cdf972f55e98f5a7442a713))
	ROM_LOAD("daisy_irfdsy_vr4_5-1-89.rom",   0x18000, 0x8000, CRC(9662d779) SHA1(0841c1fd34c152f3f02ace8633f1fa3f5139069e))
	ROM_LOAD("hibas03_a063.rom",              0x20000, 0x4000, CRC(6ea7affc) SHA1(99234b55fde57680e4217b72ef4ccb8fc56edeff))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94))
ROM_END


ROM_START(discmon)
	// page 0  00000  SK3 Rear Cartridge bottom 16K      // page 8  20000  IC27 DiscMonitor
	// page 1  04000  SK3 Rear Cartridge top 16K         // page 9  24000  IC24 DFS + SRAM
	// page 2  08000  SK4 Front Cartridge bottom 16K     // page 10 28000  IC24 Viewsheet
	// page 3  0C000  SK4 Front Cartridge top 16K        // page 11 2C000  IC24 Edit
	// page 4  10000  IC41 SWRAM or bottom 16K           // page 12 30000  IC24 BASIC
	// page 5  14000  IC41 SWRAM or top 16K              // page 13 34000  IC24 ADFS
	// page 6  18000  IC37 SWRAM or bottom 16K           // page 14 38000  IC24 View + MOS code
	// page 7  1C000  IC37 SWRAM or top 16K              // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_LOAD("discmonitor406.rom", 0x20000, 0x4000, CRC(12e30e9b) SHA1(0e5356531978e08e75913e793cb0afc0e75e61ad))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94))
ROM_END


ROM_START(discmate)
	// page 0  00000  SK3 Rear Cartridge bottom 16K      // page 8  20000  IC27 Discmaster
	// page 1  04000  SK3 Rear Cartridge top 16K         // page 9  24000  IC24 DFS + SRAM
	// page 2  08000  SK4 Front Cartridge bottom 16K     // page 10 28000  IC24 Viewsheet
	// page 3  0C000  SK4 Front Cartridge top 16K        // page 11 2C000  IC24 Edit
	// page 4  10000  IC41 SWRAM or bottom 16K           // page 12 30000  IC24 BASIC
	// page 5  14000  IC41 SWRAM or top 16K              // page 13 34000  IC24 ADFS
	// page 6  18000  IC37 SWRAM or bottom 16K           // page 14 38000  IC24 View + MOS code
	// page 7  1C000  IC37 SWRAM or top 16K              // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_LOAD("discmaster303.rom", 0x20000, 0x4000, CRC(73974057) SHA1(79f99eae62ab46818386ab8a67fe50319ae30226))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94))
ROM_END


ROM_START(ht280)
	// page 0  00000  SK3 32K SRAM Cartridge bottom 16K  // page 8  20000  IC27 280T D 5 3.04
	// page 1  04000  SK3 32K SRAM Cartridge top 16K     // page 9  24000  IC24 DFS + SRAM
	// page 2  08000  SK4 Care 2 ROM Cartridge           // page 10 28000  IC24 Viewsheet
	// page 3  0C000  SK4 Care 2 ROM Cartridge           // page 11 2C000  IC24 Edit
	// page 4  10000  IC41 280T D 6M+S 3.59              // page 12 30000  IC24 BASIC
	// page 5  14000  IC41 280T D 6M+S 3.59              // page 13 34000  IC24 ADFS
	// page 6  18000  IC37 280T D 1+2 3.57               // page 14 38000  IC24 View + MOS code
	// page 7  1C000  IC37 280T D 1+2 3.57               // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_LOAD("sram_reset.bin",      0x00000, 0x8000, CRC(7311ad92) SHA1(ba52f52012b005285724c9507e684800ef316ba9)) // 32K SRAM Cartridge (saved after *RESET)
	ROM_LOAD("280td_3_3.06.bin",    0x08000, 0x4000, CRC(ff1da25a) SHA1(3b75f1bdee295767a0767ee9a3d9af58b62e6d6d)) // Care 2 ROM Cartridge
	ROM_LOAD("280td_4_3.60.bin",    0x0c000, 0x4000, CRC(30ebb04e) SHA1(40e81b91441eefa199a362f09d623dc834192eb2)) // Care 2 ROM Cartridge
	ROM_LOAD("280td_6m+s_3.54.rom", 0x10000, 0x8000, CRC(7a36b159) SHA1(fc2648418f60fe050152b2eac90de786acd8891a))
	ROM_LOAD("280td_1+2_3.59.rom",  0x18000, 0x8000, CRC(cacec014) SHA1(f445ccf86d7c8415e924e8c3181351e3ca612b80))
	ROM_LOAD("280td_5_3.04.rom",    0x20000, 0x4000, CRC(75cb57b1) SHA1(708b4d8a71dd5050d57c3a8533fd6ca8737084df))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94))
ROM_END


#define rom_ltmpm rom_bbcm


ROM_START(mpc800)
	// page 0  00000  SK3 Rear Cartridge bottom 16K      // page 8  20000  IC27 800 Manager
	// page 1  04000  SK3 Rear Cartridge top 16K         // page 9  24000  IC24 DFS + SRAM
	// page 2  08000  SK4 Front Cartridge bottom 16K     // page 10 28000  IC24 Viewsheet
	// page 3  0C000  SK4 Front Cartridge top 16K        // page 11 2C000  IC24 Edit
	// page 4  10000  IC41 SWRAM or bottom 16K           // page 12 30000  IC24 BASIC
	// page 5  14000  IC41 SWRAM or top 16K              // page 13 34000  IC24 ADFS
	// page 6  18000  IC37 SWRAM or bottom 16K           // page 14 38000  IC24 View + MOS code
	// page 7  1C000  IC37 SWRAM or top 16K              // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_LOAD("prisma2_v1-10.bin",      0x00000, 0x4000, CRC(14b85cdd) SHA1(c975041933dd0e223470292d169dd70568c25c59))
	ROM_LOAD("mpc800manager-2.40.rom", 0x20000, 0x4000, CRC(d5a27b00) SHA1(533e846f47803d61508fe270fd7021c010a21a84))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94))
ROM_END


ROM_START(mpc900)
	// page 0  00000  SK3 Rear Cartridge bottom 16K      // page 8  20000  IC27 900 Manager
	// page 1  04000  SK3 Rear Cartridge top 16K         // page 9  24000  IC24 DFS + SRAM
	// page 2  08000  SK4 Front Cartridge bottom 16K     // page 10 28000  IC24 Viewsheet
	// page 3  0C000  SK4 Front Cartridge top 16K        // page 11 2C000  IC24 Edit
	// page 4  10000  IC41 SWRAM or bottom 16K           // page 12 30000  IC24 BASIC
	// page 5  14000  IC41 SWRAM or top 16K              // page 13 34000  IC24 ADFS
	// page 6  18000  IC37 SWRAM or bottom 16K           // page 14 38000  IC24 View + MOS code
	// page 7  1C000  IC37 SWRAM or top 16K              // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_LOAD("prisma3_v1-30main.bin",   0x00000, 0x4000, CRC(6ed1e8aa) SHA1(ceda5ca869f960a4bb5368adbf4acd4861efd3a0))
	ROM_LOAD("prisma3_v1-30utils.bin",  0x04000, 0x4000, CRC(76c42acb) SHA1(36aab45a3565325dd31ada64934597bafe765ed8))
	ROM_LOAD("mpc900_manager-1.20.rom", 0x20000, 0x4000, BAD_DUMP CRC(3470af89) SHA1(5d54ace2fbfdb9a7ec88aeaebcfe978688ef1893))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94))
ROM_END


ROM_START(mpc900gx)
	// page 0  00000  SK3 Rear Cartridge bottom 16K      // page 8  20000  IC27 900GX Manager
	// page 1  04000  SK3 Rear Cartridge top 16K         // page 9  24000  IC24 DFS + SRAM
	// page 2  08000  SK4 Front Cartridge bottom 16K     // page 10 28000  IC24 Viewsheet
	// page 3  0C000  SK4 Front Cartridge top 16K        // page 11 2C000  IC24 Edit
	// page 4  10000  IC41 SWRAM or bottom 16K           // page 12 30000  IC24 BASIC
	// page 5  14000  IC41 SWRAM or top 16K              // page 13 34000  IC24 ADFS
	// page 6  18000  IC37 SWRAM or bottom 16K           // page 14 38000  IC24 View + MOS code
	// page 7  1C000  IC37 SWRAM or top 16K              // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_LOAD("prisma3_v2-30main.bin",     0x00000, 0x4000, CRC(b9489c7c) SHA1(59a7606e3b92e1c7fbd0075d54e4fffd7c28417e))
	ROM_LOAD("prisma3_v2-30utils.bin",    0x04000, 0x4000, CRC(fa395bca) SHA1(be7b1f590818e362a8acc5b203f0a08fce967f42))
	ROM_LOAD("mpc900gx_manager-1.20.rom", 0x20000, 0x4000, CRC(3470af89) SHA1(5d54ace2fbfdb9a7ec88aeaebcfe978688ef1893))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94))
ROM_END


ROM_START(se3010)
	// page 0  00000  SK3 RFS Data                       // page 8  20000  IC27
	// page 1  04000  SK3 RFS Data                       // page 9  24000  IC24 DFS + SRAM
	// page 2  08000  SK4                                // page 10 28000  IC24 Viewsheet
	// page 3  0C000  SK4                                // page 11 2C000  IC24 Edit
	// page 4  10000  IC41 SWRAM or bottom 16K           // page 12 30000  IC24 BASIC
	// page 5  14000  IC41 SWRAM or top 16K              // page 13 34000  IC24 ADFS
	// page 6  18000  IC37 SWRAM or bottom 16K           // page 14 38000  IC24 View + MOS code
	// page 7  1C000  IC37 SWRAM or top 16K              // page 15 3C000  IC24 Terminal + Tube host + CFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("mos320.ic24", 0x20000, 0x20000, CRC(0f747ebe) SHA1(eacacbec3892dc4809ad5800e6c8299ff9eb528f))

	ROM_SYSTEM_BIOS(0, "44", "V4.4")
	ROMX_LOAD("sprite_control_v4.4.bin", 0x00000, 0x4000, CRC(9c5a507d) SHA1(7c097ea17ad863fac575c91c0b626198059b5aea), ROM_BIOS(0))
	ROMX_LOAD("sprite_master_v4.4.bin",  0x04000, 0x4000, CRC(96fd46cf) SHA1(ac8f55658de0b11e3234454a833abc27c242db23), ROM_BIOS(0))

	//ROM_SYSTEM_BIOS(1, "551", "V5.51")
	//ROMX_LOAD("sprite.bin",       0x00000, 0x4000, NO_DUMP, ROM_BIOS(1))
	//ROMX_LOAD("communicator.bin", 0x04000, 0x4000, NO_DUMP, ROM_BIOS(1))
	//ROMX_LOAD("mdm2.bin",         0x08000, 0x4000, NO_DUMP, ROM_BIOS(1))
	//ROMX_LOAD("mdm3.bin",         0x0c000, 0x4000, NO_DUMP, ROM_BIOS(1))

	ROM_COPY("rom", 0x20000, 0x40000, 0x4000) // Move loaded roms into place
	ROM_FILL(0x20000, 0x4000, 0xff)

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("mos320.cmos", 0x00, 0x40, CRC(c7f9e85a) SHA1(f24cc9db0525910689219f7204bf8b864033ee94))
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT  COMPAT MACHINE     INPUT   CLASS         INIT       COMPANY                        FULLNAME                              FLAGS
COMP( 1986, bbcm,       0,      bbcb,  bbcm,       bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master 128",                     MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, bbcmt,      bbcm,   0,     bbcmt,      bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master Turbo",                   MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, bbcmaiv,    bbcm,   0,     bbcmaiv,    bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master AIV",                     MACHINE_NOT_WORKING )
COMP( 1986, bbcmet,     bbcm,   0,     bbcmet,     bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master ET",                      MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, bbcm512,    bbcm,   0,     bbcm512,    bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master 512",                     MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, bbcmarm,    bbcm,   0,     bbcmarm,    bbcm,   bbcm_state,   init_bbc,  "Acorn Computers",             "BBC Master (ARM Evaluation)",        MACHINE_NOT_WORKING )

// TV Production
COMP( 1987, mpc800,     bbcm,   0,     mpc800,     bbcm,   bbcm_state,   init_bbc,  "G2 Systems",                  "MasterPieCe 800 Series",             MACHINE_NOT_WORKING )
COMP( 1988, mpc900,     bbcm,   0,     mpc900,     bbcm,   bbcm_state,   init_bbc,  "G2 Systems",                  "MasterPieCe 900 Series",             MACHINE_NOT_WORKING )
COMP( 1990, mpc900gx,   bbcm,   0,     mpc900gx,   bbcm,   bbcm_state,   init_bbc,  "G2 Systems",                  "MasterPieCe 900GX Series",           MACHINE_NOT_WORKING )
COMP( 1987, se3010,     bbcm,   0,     se3010,     bbcm,   bbcm_state,   init_bbc,  "Softel Electronics",          "SE3010 Teletext Editing Terminal",   MACHINE_NOT_WORKING )

// Jukeboxes
//COMP( 1988, discmast,   bbcm,   0,     discmast,   bbcm,   bbcm_state,   init_bbc,  "Arbiter Leisure",             "Arbiter Discmaster A-00",            MACHINE_NOT_WORKING )
COMP( 1988, discmon,    bbcm,   0,     discmon,    bbcm,   bbcm_state,   init_bbc,  "Arbiter Leisure",             "Arbiter Discmonitor A-01",           MACHINE_NOT_WORKING )
COMP( 1988, discmate,   bbcm,   0,     discmate,   bbcm,   bbcm_state,   init_bbc,  "Arbiter Leisure",             "Arbiter Discmate A-02",              MACHINE_NOT_WORKING )

// Industrial
COMP( 1986, ltmpm,      bbcm,   0,     bbcm,       0,      bbcm_state,   init_ltmp, "Lawrie T&M Ltd.",             "LTM Portable (Master)",              MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, daisy,      bbcm,   0,     daisy,      bbcm,   bbcm_state,   init_bbc,  "Comus Instruments Ltd.",      "Comus Daisy",                        MACHINE_NOT_WORKING )
COMP( 1989, cfa3000,    bbcm,   0,     cfa3000,    bbcm,   bbcm_state,   init_cfa,  "Tinsley Medical Instruments", "Henson CFA 3000 (Master)",           MACHINE_NOT_WORKING )
COMP( 1992, ht280,      bbcm,   0,     ht280,      0,      bbcm_state,   init_bbc,  "T.S.Harrison",                "Harrison Trainer 280 CNC/Manual",    MACHINE_NOT_WORKING )
