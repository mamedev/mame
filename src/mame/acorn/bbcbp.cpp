// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/******************************************************************************

    BBC Model B+

    ANB51 - BBC Model B+ 64K
    ANB52 - BBC Model B+ 64K with Econet
    ANB53 - BBC Model B+ 64K with Disc interface
    ANB54 - BBC Model B+ 64K with Disc and Econet interfaces
    ANB55 - BBC Model B+ 128K with Disc interface

    Econet

    AEH25 - Econet X25 Gateway

    Acorn Business Computer

    ABC110        - 64K,   10MB HDD, Z80, CP/M 2.2
    ABC210/ACW443 - 4096K, 20MB HDD, 32016, PanOS
    ABC310        - 1024K, 10MB HDD, 80286, DOS 3.1/GEM

    TODO:
    - Cambridge Workstation has a Tube switch to disable internal co-processor.

******************************************************************************/

#include "emu.h"
#include "bbc.h"
#include "acorn_serproc.h"
#include "bbc_kbd.h"

#include "machine/tms6100.h"
#include "machine/wd_fdc.h"
#include "sound/tms5220.h"

#include "formats/uef_cas.h"
#include "formats/csw_cas.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "bbc.lh"


namespace {

class bbcbp_state : public bbc_state
{
public:
	bbcbp_state(const machine_config &mconfig, device_type type, const char *tag)
		: bbc_state(mconfig, type, tag)
		, m_view_shadow(*this, "view_shadow")
		, m_view_paged(*this, "view_paged")
		, m_kbd(*this, "kbd")
		, m_sn(*this, "sn")
		, m_vsp(*this, "vsp")
		, m_wdfdc(*this, "wdfdc")
		, m_adlc(*this, "mc6854")
		, m_statid(*this, "STATID")
	{ }

	void bbcbp(machine_config &config);
	void bbcbp128(machine_config &config);
	void abc110(machine_config &config);
	void acw443(machine_config &config);
	void abc310(machine_config &config);
	void cfa3000bp(machine_config &config);
	void econx25(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	memory_view m_view_shadow;
	memory_view m_view_paged;
	required_device<bbc_kbd_device> m_kbd;
	required_device<sn76489a_device> m_sn;
	optional_device<tms5220_device> m_vsp;
	optional_device<wd1770_device> m_wdfdc;
	required_device<mc6854_device> m_adlc;
	required_ioport m_statid;

	void bbcbp_fetch(address_map &map) ATTR_COLD;
	void bbcbp_mem(address_map &map) ATTR_COLD;
	void econx25_mem(address_map &map) ATTR_COLD;

	void trigger_reset(int state);

	uint8_t fetch_r(offs_t offset);
	void romsel_w(offs_t offset, uint8_t data);
	uint8_t paged_r(offs_t offset);
	void paged_w(offs_t offset, uint8_t data);
	void drive_control_w(uint8_t data);

	uint8_t sysvia_pa_r();
	void sysvia_pa_w(uint8_t data);
	void update_sdb();
	uint8_t sysvia_pb_r();
	void sysvia_pb_w(uint8_t data);

	uint8_t m_sdb = 0x00;
};


void bbcbp_state::machine_start()
{
	bbc_state::machine_start();

	save_item(NAME(m_sdb));
}


void bbcbp_state::bbcbp_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(bbcbp_state::fetch_r));
}


void bbcbp_state::bbcbp_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));                                    //    0000-7FFF                 Regular RAM
	map(0x3000, 0x7fff).view(m_view_shadow);                                                                           //    3000-7FFF                 20K Shadow RAM
	m_view_shadow[0](0x3000, 0x7fff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset + 0xb000]; }));
	m_view_shadow[0](0x3000, 0x7fff).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0xb000] = data; }));
	map(0x8000, 0xbfff).rw(FUNC(bbcbp_state::paged_r), FUNC(bbcbp_state::paged_w));                                    //    8000-BFFF                 Paged ROM/RAM
	map(0x8000, 0xafff).view(m_view_paged);                                                                            //    8000-AFFF                 12K Paged RAM
	m_view_paged[0](0x8000, 0xafff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset + 0x8000]; }));
	m_view_paged[0](0x8000, 0xafff).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0x8000] = data; }));
	map(0xc000, 0xffff).rw(FUNC(bbcbp_state::mos_r), FUNC(bbcbp_state::mos_w));                                        //    C000-FBFF                 OS ROM
	map(0xfc00, 0xfcff).rw(m_1mhzbus, FUNC(bbc_1mhzbus_slot_device::fred_r), FUNC(bbc_1mhzbus_slot_device::fred_w));   //    FC00-FCFF                 FRED Address Page
	map(0xfd00, 0xfdff).rw(m_1mhzbus, FUNC(bbc_1mhzbus_slot_device::jim_r), FUNC(bbc_1mhzbus_slot_device::jim_w));     //    FD00-FDFF                 JIM Address Page
	map(0xfe00, 0xfeff).lr8(NAME([]() { return 0xfe; })).nopw();                                                       //    FE00-FEFF                 SHEILA Address Page
	map(0xfe00, 0xfe00).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));      //    FE00-FE07  6845 CRTC      Video controller
	map(0xfe01, 0xfe01).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0xfe08, 0xfe09).mirror(0x06).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));            //    FE08-FE0F  6850 ACIA      Serial controller
	map(0xfe10, 0xfe17).rw("serproc", FUNC(acorn_serproc_device::read), FUNC(acorn_serproc_device::write));            //    FE10-FE17  Serial ULA     Serial system chip
	map(0xfe18, 0xfe1f).lr8(NAME([this]() { econet_int_enable(0); return m_statid->read(); }));                        // R: FE18-FE1F  INTOFF/STATID  ECONET Interrupt Off / ID No.
	map(0xfe18, 0xfe1f).lw8(NAME([this](uint8_t data) { econet_int_enable(0); }));                                     // W: FE18-FE1F  INTOFF         ECONET Interrupt Off
	map(0xfe20, 0xfe2f).lr8(NAME([this]() { econet_int_enable(1); return 0xfe; }));                                    // R: FE20-FE2F  INTON          ECONET Interrupt On
	map(0xfe20, 0xfe2f).w(FUNC(bbcbp_state::video_ula_w));                                                             // W: FE20-FE2F  Video ULA      Video system chip
	map(0xfe30, 0xfe3f).w(FUNC(bbcbp_state::romsel_w));                                                                // W: FE30-FE3F  84LS161        Paged ROM selector
	map(0xfe40, 0xfe4f).mirror(0x10).m(m_sysvia, FUNC(via6522_device::map));                                           //    FE40-FE5F  6522 VIA       SYSTEM VIA
	map(0xfe60, 0xfe6f).mirror(0x10).m(m_uservia, FUNC(via6522_device::map));                                          //    FE60-FE7F  6522 VIA       USER VIA
	map(0xfe80, 0xfe83).w(FUNC(bbcbp_state::drive_control_w));                                                         //    FE80-FE83  1770 FDC       Drive control register
	map(0xfe84, 0xfe9f).rw(m_wdfdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write));                            //    FE84-FE9F  1770 FDC       Floppy disc controller
	map(0xfea0, 0xfea3).mirror(0x1c).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));                //    FEA0-FEBF  68B54 ADLC     ECONET controller
	map(0xfec0, 0xfec3).mirror(0x1c).rw("upd7002", FUNC(upd7002_device::read), FUNC(upd7002_device::write));           //    FEC0-FEDF  uPD7002        Analogue to digital converter
	map(0xfee0, 0xfeff).rw(m_tube, FUNC(bbc_tube_slot_device::host_r), FUNC(bbc_tube_slot_device::host_w));            //    FEE0-FEFF  Tube ULA       Tube system interface
}


void bbcbp_state::econx25_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));                                    //    0000-7FFF                 Regular RAM
	map(0x3000, 0x7fff).view(m_view_shadow);                                                                           //    3000-7FFF                 20K Shadow RAM
	m_view_shadow[0](0x3000, 0x7fff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset + 0xb000]; }));
	m_view_shadow[0](0x3000, 0x7fff).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0xb000] = data; }));
	map(0x8000, 0xbfff).rw(FUNC(bbcbp_state::paged_r), FUNC(bbcbp_state::paged_w));                                    //    8000-BFFF                 Paged ROM/RAM
	map(0x8000, 0xafff).view(m_view_paged);                                                                            //    8000-AFFF                 12K Paged RAM
	m_view_paged[0](0x8000, 0xafff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset + 0x8000]; }));
	m_view_paged[0](0x8000, 0xafff).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0x8000] = data; }));
	map(0xc000, 0xffff).rw(FUNC(bbcbp_state::mos_r), FUNC(bbcbp_state::mos_w));                                        //    C000-FBFF                 OS ROM
	map(0xfc00, 0xfcff).rw(m_1mhzbus, FUNC(bbc_1mhzbus_slot_device::fred_r), FUNC(bbc_1mhzbus_slot_device::fred_w));   //    FC00-FCFF                 FRED Address Page
	map(0xfd00, 0xfdff).rw(m_1mhzbus, FUNC(bbc_1mhzbus_slot_device::jim_r), FUNC(bbc_1mhzbus_slot_device::jim_w));     //    FD00-FDFF                 JIM Address Page
	map(0xfe00, 0xfeff).lr8(NAME([]() { return 0xfe; })).nopw();                                                       //    FE00-FEFF                 SHEILA Address Page
	map(0xfe00, 0xfe00).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));      //    FE00-FE07  6845 CRTC      Video controller
	map(0xfe01, 0xfe01).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0xfe08, 0xfe09).mirror(0x06).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));            //    FE08-FE0F  6850 ACIA      Serial controller
	map(0xfe10, 0xfe17).rw("serproc", FUNC(acorn_serproc_device::read), FUNC(acorn_serproc_device::write));            //    FE10-FE17  Serial ULA     Serial system chip
	map(0xfe18, 0xfe1f).lr8(NAME([this]() { econet_int_enable(0); return m_statid->read(); }));                        // R: FE18-FE1F  INTOFF/STATID  ECONET Interrupt Off / ID No.
	map(0xfe18, 0xfe1f).lw8(NAME([this](uint8_t data) { econet_int_enable(0); }));                                     // W: FE18-FE1F  INTOFF         ECONET Interrupt Off
	map(0xfe20, 0xfe2f).lr8(NAME([this]() { econet_int_enable(1); return 0xfe; }));                                    // R: FE20-FE2F  INTON          ECONET Interrupt On
	map(0xfe20, 0xfe2f).w(FUNC(bbcbp_state::video_ula_w));                                                             // W: FE20-FE2F  Video ULA      Video system chip
	map(0xfe30, 0xfe3f).w(FUNC(bbcbp_state::romsel_w));                                                                // W: FE30-FE3F  84LS161        Paged ROM selector
	map(0xfe40, 0xfe4f).mirror(0x10).m(m_sysvia, FUNC(via6522_device::map));                                           //    FE40-FE5F  6522 VIA       SYSTEM VIA
	map(0xfe60, 0xfe6f).mirror(0x10).m(m_uservia, FUNC(via6522_device::map));                                          //    FE60-FE7F  6522 VIA       USER VIA
	map(0xfea0, 0xfea3).mirror(0x1c).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));                //    FEA0-FEBF  68B54 ADLC     ECONET controller
	map(0xfec0, 0xfec3).mirror(0x1c).rw("upd7002", FUNC(upd7002_device::read), FUNC(upd7002_device::write));           //    FEC0-FEDF  uPD7002        Analogue to digital converter
	map(0xfee0, 0xfeff).rw(m_tube, FUNC(bbc_tube_slot_device::host_r), FUNC(bbc_tube_slot_device::host_w));            //    FEE0-FEFF  Tube ULA       Tube system interface
}


uint8_t bbcbp_state::fetch_r(offs_t offset)
{
	switch (offset & 0xf000)
	{
	case 0xa000:
		// Code executing from sideways RAM between 0xa000-0xafff will access the shadow RAM (if selected)
		if (m_vdusel && m_paged_ram)
			m_view_shadow.select(0);
		else
			m_view_shadow.disable();
		break;

	case 0xc000:
	case 0xd000:
		// Access shadow RAM if VDU drivers and shadow RAM selected
		if (m_vdusel)
			m_view_shadow.select(0);
		else
			m_view_shadow.disable();
		break;

	default:
		m_view_shadow.disable();
		break;
	}
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void bbcbp_state::romsel_w(offs_t offset, uint8_t data)
{
	// the BBC Model B+ addresses all 16 ROM sockets and extra 12K of RAM at 0x8000 and 20K of shadow RAM at 0x3000
	switch (offset & 0x07)
	{
	case 0x00:
		m_paged_ram = BIT(data, 7);
		if (m_paged_ram)
			m_view_paged.select(0);
		else
			m_view_paged.disable();

		m_romsel = data & 0x0f;
		break;

	case 0x04:
		// the video display should now use this flag to display the shadow RAM memory
		m_vdusel = BIT(data, 7);
		setvideoshadow(m_vdusel);
		break;
	}

	// pass ROMSEL to internal expansion board
	if (m_internal && m_internal->overrides_rom())
		m_internal->romsel_w(offset, data);
}

uint8_t bbcbp_state::paged_r(offs_t offset)
{
	uint8_t data;

	if (m_internal && m_internal->overrides_rom())
	{
		data = m_internal->paged_r(offset);
	}
	else
	{
		// 32K sockets
		if (m_rom[m_romsel & 0x0e] && m_rom[m_romsel & 0x0e]->present())
			data = m_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
		else
			data = m_region_rom->base()[offset + (m_romsel << 14)];
	}

	return data;
}

void bbcbp_state::paged_w(offs_t offset, uint8_t data)
{
	if (m_internal && m_internal->overrides_rom())
	{
		m_internal->paged_w(offset, data);
	}
	else
	{
		// 32K sockets
		if (m_rom[m_romsel & 0x0e])
			m_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
	}
}


uint8_t bbcbp_state::sysvia_pa_r()
{
	update_sdb();

	return m_sdb;
}

void bbcbp_state::sysvia_pa_w(uint8_t data)
{
	m_sdb = data;

	update_sdb();
}


void bbcbp_state::update_sdb()
{
	uint8_t const latch = m_latch->output_state();

	// sound
	if (!BIT(latch, 0))
		m_sn->write(m_sdb);

	// speech
	if (m_vsp)
	{
		m_vsp->combined_rsq_wsq_w(bitswap<2>(latch, 1, 2));
		switch (bitswap<2>(~latch, 1, 2))
		{
		case tms5200_device::RS:
			m_sdb = m_vsp->status_r();
			break;
		case tms5200_device::WS:
			m_vsp->data_w(m_sdb);
			break;
		}
	}

	// keyboard
	m_sdb = m_kbd->read(m_sdb);
}


uint8_t bbcbp_state::sysvia_pb_r()
{
	uint8_t data = 0xff;

	if (m_analog)
	{
		data &= ~0x30;
		data |= m_analog->pb_r();
	}

	if (m_vsp)
	{
		data &= ~0xc0;
		data |= m_vsp->intq_r() << 6;
		data |= m_vsp->readyq_r() << 7;
	}

	return data;
}

void bbcbp_state::sysvia_pb_w(uint8_t data)
{
	m_latch->write_nibble_d3(data);

	if (m_analog)
	{
		m_analog->pb_w(data & 0x30);
	}

	update_sdb();
}


void bbcbp_state::drive_control_w(uint8_t data)
{
	// Bit       Meaning
	// -----------------
	// 7,6       Not used
	//  5        Reset drive controller chip. (0 = reset controller, 1 = no reset)
	//  4        Interrupt Enable (0 = enable int, 1 = disable int)
	//  3        Double density select (0 = double, 1 = single)
	//  2        Side select (0 = side 0, 1 = side 1)
	//  1        Drive select 1
	//  0        Drive select 0

	floppy_image_device *floppy = nullptr;

	// bit 0, 1: drive select
	if (BIT(data, 0)) floppy = m_wdfdc->subdevice<floppy_connector>("0")->get_device();
	if (BIT(data, 1)) floppy = m_wdfdc->subdevice<floppy_connector>("1")->get_device();
	m_wdfdc->set_floppy(floppy);

	// bit 2: side select
	if (floppy)
		floppy->ss_w(BIT(data, 2));

	// bit 3: density
	m_wdfdc->dden_w(BIT(data, 3));

	// bit 4: disable NMI (S5 wire link not fitted)

	// bit 5: reset
	m_wdfdc->mr_w(BIT(data, 5));
}


void bbcbp_state::trigger_reset(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	if (!state)
	{
		if (m_uservia) m_uservia->reset();
		if (m_adlc) m_adlc->reset();
		if (m_wdfdc) m_wdfdc->reset();
		if (m_1mhzbus) m_1mhzbus->reset();
		if (m_tube) m_tube->reset();
		if (m_internal) m_internal->reset();
	}
}


static INPUT_PORTS_START(bbc_statid)
	PORT_START("STATID")
	PORT_DIPNAME(0xff, 0xfe, "Econet ID") PORT_DIPLOCATION("S23:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(   0x00,   "0" )    PORT_DIPSETTING(   0x01,   "1" )    PORT_DIPSETTING(   0x02,   "2" )    PORT_DIPSETTING(   0x03,   "3" )    PORT_DIPSETTING(   0x04,   "4" )
	PORT_DIPSETTING(   0x05,   "5" )    PORT_DIPSETTING(   0x06,   "6" )    PORT_DIPSETTING(   0x07,   "7" )    PORT_DIPSETTING(   0x08,   "8" )    PORT_DIPSETTING(   0x09,   "9" )
	PORT_DIPSETTING(   0x0a,  "10" )    PORT_DIPSETTING(   0x0b,  "11" )    PORT_DIPSETTING(   0x0c,  "12" )    PORT_DIPSETTING(   0x0d,  "13" )    PORT_DIPSETTING(   0x0e,  "14" )
	PORT_DIPSETTING(   0x0f,  "15" )    PORT_DIPSETTING(   0x10,  "16" )    PORT_DIPSETTING(   0x11,  "17" )    PORT_DIPSETTING(   0x12,  "18" )    PORT_DIPSETTING(   0x13,  "19" )
	PORT_DIPSETTING(   0x14,  "20" )    PORT_DIPSETTING(   0x15,  "21" )    PORT_DIPSETTING(   0x16,  "22" )    PORT_DIPSETTING(   0x17,  "23" )    PORT_DIPSETTING(   0x18,  "24" )
	PORT_DIPSETTING(   0x19,  "25" )    PORT_DIPSETTING(   0x1a,  "26" )    PORT_DIPSETTING(   0x1b,  "27" )    PORT_DIPSETTING(   0x1c,  "28" )    PORT_DIPSETTING(   0x1d,  "29" )
	PORT_DIPSETTING(   0x1e,  "30" )    PORT_DIPSETTING(   0x1f,  "31" )    PORT_DIPSETTING(   0x20,  "32" )    PORT_DIPSETTING(   0x21,  "33" )    PORT_DIPSETTING(   0x22,  "34" )
	PORT_DIPSETTING(   0x23,  "35" )    PORT_DIPSETTING(   0x24,  "36" )    PORT_DIPSETTING(   0x25,  "37" )    PORT_DIPSETTING(   0x26,  "38" )    PORT_DIPSETTING(   0x27,  "39" )
	PORT_DIPSETTING(   0x28,  "40" )    PORT_DIPSETTING(   0x29,  "41" )    PORT_DIPSETTING(   0x2a,  "42" )    PORT_DIPSETTING(   0x2b,  "43" )    PORT_DIPSETTING(   0x2c,  "44" )
	PORT_DIPSETTING(   0x2d,  "45" )    PORT_DIPSETTING(   0x2e,  "46" )    PORT_DIPSETTING(   0x2f,  "47" )    PORT_DIPSETTING(   0x30,  "48" )    PORT_DIPSETTING(   0x31,  "49" )
	PORT_DIPSETTING(   0x32,  "50" )    PORT_DIPSETTING(   0x33,  "51" )    PORT_DIPSETTING(   0x34,  "52" )    PORT_DIPSETTING(   0x35,  "53" )    PORT_DIPSETTING(   0x36,  "54" )
	PORT_DIPSETTING(   0x37,  "55" )    PORT_DIPSETTING(   0x38,  "56" )    PORT_DIPSETTING(   0x39,  "57" )    PORT_DIPSETTING(   0x3a,  "58" )    PORT_DIPSETTING(   0x3b,  "59" )
	PORT_DIPSETTING(   0x3c,  "60" )    PORT_DIPSETTING(   0x3d,  "61" )    PORT_DIPSETTING(   0x3e,  "62" )    PORT_DIPSETTING(   0x3f,  "63" )    PORT_DIPSETTING(   0x40,  "64" )
	PORT_DIPSETTING(   0x41,  "65" )    PORT_DIPSETTING(   0x42,  "66" )    PORT_DIPSETTING(   0x43,  "67" )    PORT_DIPSETTING(   0x44,  "68" )    PORT_DIPSETTING(   0x45,  "69" )
	PORT_DIPSETTING(   0x46,  "70" )    PORT_DIPSETTING(   0x47,  "71" )    PORT_DIPSETTING(   0x48,  "72" )    PORT_DIPSETTING(   0x49,  "73" )    PORT_DIPSETTING(   0x4a,  "74" )
	PORT_DIPSETTING(   0x4b,  "75" )    PORT_DIPSETTING(   0x4c,  "76" )    PORT_DIPSETTING(   0x4d,  "77" )    PORT_DIPSETTING(   0x4e,  "78" )    PORT_DIPSETTING(   0x4f,  "79" )
	PORT_DIPSETTING(   0x50,  "80" )    PORT_DIPSETTING(   0x51,  "81" )    PORT_DIPSETTING(   0x52,  "82" )    PORT_DIPSETTING(   0x53,  "83" )    PORT_DIPSETTING(   0x54,  "84" )
	PORT_DIPSETTING(   0x55,  "85" )    PORT_DIPSETTING(   0x56,  "86" )    PORT_DIPSETTING(   0x57,  "87" )    PORT_DIPSETTING(   0x58,  "88" )    PORT_DIPSETTING(   0x59,  "89" )
	PORT_DIPSETTING(   0x5a,  "90" )    PORT_DIPSETTING(   0x5b,  "91" )    PORT_DIPSETTING(   0x5c,  "92" )    PORT_DIPSETTING(   0x5d,  "93" )    PORT_DIPSETTING(   0x5e,  "94" )
	PORT_DIPSETTING(   0x5f,  "95" )    PORT_DIPSETTING(   0x60,  "96" )    PORT_DIPSETTING(   0x61,  "97" )    PORT_DIPSETTING(   0x62,  "98" )    PORT_DIPSETTING(   0x63,  "99" )
	PORT_DIPSETTING(   0x64, "100" )    PORT_DIPSETTING(   0x65, "101" )    PORT_DIPSETTING(   0x66, "102" )    PORT_DIPSETTING(   0x67, "103" )    PORT_DIPSETTING(   0x68, "104" )
	PORT_DIPSETTING(   0x69, "105" )    PORT_DIPSETTING(   0x6a, "106" )    PORT_DIPSETTING(   0x6b, "107" )    PORT_DIPSETTING(   0x6c, "108" )    PORT_DIPSETTING(   0x6d, "109" )
	PORT_DIPSETTING(   0x6e, "110" )    PORT_DIPSETTING(   0x6f, "111" )    PORT_DIPSETTING(   0x70, "112" )    PORT_DIPSETTING(   0x71, "113" )    PORT_DIPSETTING(   0x72, "114" )
	PORT_DIPSETTING(   0x73, "115" )    PORT_DIPSETTING(   0x74, "116" )    PORT_DIPSETTING(   0x75, "117" )    PORT_DIPSETTING(   0x76, "118" )    PORT_DIPSETTING(   0x77, "119" )
	PORT_DIPSETTING(   0x78, "120" )    PORT_DIPSETTING(   0x79, "121" )    PORT_DIPSETTING(   0x7a, "122" )    PORT_DIPSETTING(   0x7b, "123" )    PORT_DIPSETTING(   0x7c, "124" )
	PORT_DIPSETTING(   0x7d, "125" )    PORT_DIPSETTING(   0x7e, "126" )    PORT_DIPSETTING(   0x7f, "127" )    PORT_DIPSETTING(   0x80, "128" )    PORT_DIPSETTING(   0x81, "129" )
	PORT_DIPSETTING(   0x82, "130" )    PORT_DIPSETTING(   0x83, "131" )    PORT_DIPSETTING(   0x84, "132" )    PORT_DIPSETTING(   0x85, "133" )    PORT_DIPSETTING(   0x86, "134" )
	PORT_DIPSETTING(   0x87, "135" )    PORT_DIPSETTING(   0x88, "136" )    PORT_DIPSETTING(   0x89, "137" )    PORT_DIPSETTING(   0x8a, "138" )    PORT_DIPSETTING(   0x8b, "139" )
	PORT_DIPSETTING(   0x8c, "140" )    PORT_DIPSETTING(   0x8d, "141" )    PORT_DIPSETTING(   0x8e, "142" )    PORT_DIPSETTING(   0x8f, "143" )    PORT_DIPSETTING(   0x90, "144" )
	PORT_DIPSETTING(   0x91, "145" )    PORT_DIPSETTING(   0x92, "146" )    PORT_DIPSETTING(   0x93, "147" )    PORT_DIPSETTING(   0x94, "148" )    PORT_DIPSETTING(   0x95, "149" )
	PORT_DIPSETTING(   0x96, "150" )    PORT_DIPSETTING(   0x97, "151" )    PORT_DIPSETTING(   0x98, "152" )    PORT_DIPSETTING(   0x99, "153" )    PORT_DIPSETTING(   0x9a, "154" )
	PORT_DIPSETTING(   0x9b, "155" )    PORT_DIPSETTING(   0x9c, "156" )    PORT_DIPSETTING(   0x9d, "157" )    PORT_DIPSETTING(   0x9e, "158" )    PORT_DIPSETTING(   0x9f, "159" )
	PORT_DIPSETTING(   0xa0, "160" )    PORT_DIPSETTING(   0xa1, "161" )    PORT_DIPSETTING(   0xa2, "162" )    PORT_DIPSETTING(   0xa3, "163" )    PORT_DIPSETTING(   0xa4, "164" )
	PORT_DIPSETTING(   0xa5, "165" )    PORT_DIPSETTING(   0xa6, "166" )    PORT_DIPSETTING(   0xa7, "167" )    PORT_DIPSETTING(   0xa8, "168" )    PORT_DIPSETTING(   0xa9, "169" )
	PORT_DIPSETTING(   0xaa, "170" )    PORT_DIPSETTING(   0xab, "171" )    PORT_DIPSETTING(   0xac, "172" )    PORT_DIPSETTING(   0xad, "173" )    PORT_DIPSETTING(   0xae, "174" )
	PORT_DIPSETTING(   0xaf, "175" )    PORT_DIPSETTING(   0xb0, "176" )    PORT_DIPSETTING(   0xb1, "177" )    PORT_DIPSETTING(   0xb2, "178" )    PORT_DIPSETTING(   0xb3, "179" )
	PORT_DIPSETTING(   0xb4, "180" )    PORT_DIPSETTING(   0xb5, "181" )    PORT_DIPSETTING(   0xb6, "182" )    PORT_DIPSETTING(   0xb7, "183" )    PORT_DIPSETTING(   0xb8, "184" )
	PORT_DIPSETTING(   0xb9, "185" )    PORT_DIPSETTING(   0xba, "186" )    PORT_DIPSETTING(   0xbb, "187" )    PORT_DIPSETTING(   0xbc, "188" )    PORT_DIPSETTING(   0xbd, "189" )
	PORT_DIPSETTING(   0xbe, "190" )    PORT_DIPSETTING(   0xbf, "191" )    PORT_DIPSETTING(   0xc0, "192" )    PORT_DIPSETTING(   0xc1, "193" )    PORT_DIPSETTING(   0xc2, "194" )
	PORT_DIPSETTING(   0xc3, "195" )    PORT_DIPSETTING(   0xc4, "196" )    PORT_DIPSETTING(   0xc5, "197" )    PORT_DIPSETTING(   0xc6, "198" )    PORT_DIPSETTING(   0xc7, "199" )
	PORT_DIPSETTING(   0xc8, "200" )    PORT_DIPSETTING(   0xc9, "201" )    PORT_DIPSETTING(   0xca, "202" )    PORT_DIPSETTING(   0xcb, "203" )    PORT_DIPSETTING(   0xcc, "204" )
	PORT_DIPSETTING(   0xcd, "205" )    PORT_DIPSETTING(   0xce, "206" )    PORT_DIPSETTING(   0xcf, "207" )    PORT_DIPSETTING(   0xd0, "208" )    PORT_DIPSETTING(   0xd1, "209" )
	PORT_DIPSETTING(   0xd2, "210" )    PORT_DIPSETTING(   0xd3, "211" )    PORT_DIPSETTING(   0xd4, "212" )    PORT_DIPSETTING(   0xd5, "213" )    PORT_DIPSETTING(   0xd6, "214" )
	PORT_DIPSETTING(   0xd7, "215" )    PORT_DIPSETTING(   0xd8, "216" )    PORT_DIPSETTING(   0xd9, "217" )    PORT_DIPSETTING(   0xda, "218" )    PORT_DIPSETTING(   0xdb, "219" )
	PORT_DIPSETTING(   0xdc, "220" )    PORT_DIPSETTING(   0xdd, "221" )    PORT_DIPSETTING(   0xde, "222" )    PORT_DIPSETTING(   0xdf, "223" )    PORT_DIPSETTING(   0xe0, "224" )
	PORT_DIPSETTING(   0xe1, "225" )    PORT_DIPSETTING(   0xe2, "226" )    PORT_DIPSETTING(   0xe3, "227" )    PORT_DIPSETTING(   0xe4, "228" )    PORT_DIPSETTING(   0xe5, "229" )
	PORT_DIPSETTING(   0xe6, "230" )    PORT_DIPSETTING(   0xe7, "231" )    PORT_DIPSETTING(   0xe8, "232" )    PORT_DIPSETTING(   0xe9, "233" )    PORT_DIPSETTING(   0xea, "234" )
	PORT_DIPSETTING(   0xeb, "235" )    PORT_DIPSETTING(   0xec, "236" )    PORT_DIPSETTING(   0xed, "237" )    PORT_DIPSETTING(   0xee, "238" )    PORT_DIPSETTING(   0xef, "239" )
	PORT_DIPSETTING(   0xf0, "240" )    PORT_DIPSETTING(   0xf1, "241" )    PORT_DIPSETTING(   0xf2, "242" )    PORT_DIPSETTING(   0xf3, "243" )    PORT_DIPSETTING(   0xf4, "244" )
	PORT_DIPSETTING(   0xf5, "245" )    PORT_DIPSETTING(   0xf6, "246" )    PORT_DIPSETTING(   0xf7, "247" )    PORT_DIPSETTING(   0xf8, "248" )    PORT_DIPSETTING(   0xf9, "249" )
	PORT_DIPSETTING(   0xfa, "250" )    PORT_DIPSETTING(   0xfb, "251" )    PORT_DIPSETTING(   0xfc, "252" )    PORT_DIPSETTING(   0xfd, "253" )    PORT_DIPSETTING(   0xfe, "254" )
	PORT_DIPSETTING(   0xff, "255" )
INPUT_PORTS_END


static INPUT_PORTS_START(bbcbp)
	PORT_INCLUDE(bbc_config)
	PORT_INCLUDE(bbc_statid)
INPUT_PORTS_END

static INPUT_PORTS_START(abc)
	PORT_INCLUDE(bbc_statid)
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

    BBC Model B+

****************************************************************************/

void bbcbp_state::bbcbp(machine_config &config)
{
	M6512(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbcbp_state::bbcbp_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &bbcbp_state::bbcbp_fetch);
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);
	m_irqs->output_handler().append(m_internal, FUNC(bbc_internal_slot_device::irq6502_w));

	RAM(config, m_ram).set_default_size("64K");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 256);
	m_screen->set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette).set_entries(16);

	SAA5050(config, m_trom, 12_MHz_XTAL / 2);

	HD6845S(config, m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_reconfigure_callback(FUNC(bbcbp_state::crtc_reconfigure));
	m_crtc->set_update_row_callback(FUNC(bbcbp_state::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(bbcbp_state::de_changed));
	m_crtc->out_hsync_callback().set(FUNC(bbcbp_state::hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(bbcbp_state::vsync_changed));

	config.set_default_layout(layout_bbc);

	LS259(config, m_latch);
	m_latch->q_out_cb<3>().set(m_kbd, FUNC(bbc_kbd_device::write_kb_en));
	m_latch->q_out_cb<6>().set_output("capslock_led");
	m_latch->q_out_cb<7>().set_output("shiftlock_led");

	MOS6522(config, m_sysvia, 16_MHz_XTAL / 16);
	m_sysvia->readpa_handler().set(FUNC(bbcbp_state::sysvia_pa_r));
	m_sysvia->writepa_handler().set(FUNC(bbcbp_state::sysvia_pa_w));
	m_sysvia->readpb_handler().set(FUNC(bbcbp_state::sysvia_pb_r));
	m_sysvia->writepb_handler().set(FUNC(bbcbp_state::sysvia_pb_w));
	m_sysvia->cb2_handler().set([this](int state) { if (state) m_crtc->assert_light_pen_input(); });
	m_sysvia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	BBC_KBD(config, m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcbp_state::trigger_reset));

	SPEAKER(config, "mono").front_center();

	SN76489A(config, m_sn, 16_MHz_XTAL / 4);
	m_sn->add_route(ALL_OUTPUTS, "mono", 1.0);

	TMS5220(config, m_vsp, 640000);
	m_vsp->add_route(ALL_OUTPUTS, "mono", 0.5);

	TMS6100(config, "vsm", 0);
	m_vsp->m0_cb().set("vsm", FUNC(tms6100_device::m0_w));
	m_vsp->m1_cb().set("vsm", FUNC(tms6100_device::m1_w));
	m_vsp->addr_cb().set("vsm", FUNC(tms6100_device::add_w));
	m_vsp->data_cb().set("vsm", FUNC(tms6100_device::data_line_r));
	m_vsp->romclk_cb().set("vsm", FUNC(tms6100_device::clk_w));

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
	serproc.casmo_handler().set(FUNC(bbcbp_state::cassette_motor));
	serproc.casin_handler("cassette", FUNC(cassette_image_device::input));
	serproc.casout_handler("cassette", FUNC(cassette_image_device::output));

	rs232_port_device &rs423(RS232_PORT(config, "rs423", default_rs232_devices, nullptr));
	rs423.rxd_handler().set("serproc", FUNC(acorn_serproc_device::write_din));
	rs423.cts_handler().set("serproc", FUNC(acorn_serproc_device::write_ctsi));

	MOS6522(config, m_uservia, 16_MHz_XTAL / 16);
	m_uservia->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_uservia->readpb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_r));
	m_uservia->writepb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_w));
	m_uservia->writepb_handler().append(m_internal, FUNC(bbc_internal_slot_device::latch_fe60_w));
	m_uservia->ca2_handler().set("printer", FUNC(centronics_device::write_strobe));
	m_uservia->cb1_handler().set(m_userport, FUNC(bbc_userport_slot_device::write_cb1));
	m_uservia->cb2_handler().set(m_userport, FUNC(bbc_userport_slot_device::write_cb2));
	m_uservia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	centronics_device &centronics(CENTRONICS(config, "printer", centronics_devices, "printer"));
	centronics.ack_handler().set(m_uservia, FUNC(via6522_device::write_ca1));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(latch);

	upd7002_device &upd7002(UPD7002(config, "upd7002", 16_MHz_XTAL / 16));
	upd7002.get_analogue_callback().set(m_analog, FUNC(bbc_analogue_slot_device::ch_r));
	upd7002.eoc_callback().set(m_sysvia, FUNC(via6522_device::write_cb1));

	BBC_ANALOGUE_SLOT(config, m_analog, bbc_analogue_devices, nullptr);
	m_analog->set_screen("screen");
	m_analog->lpstb_handler().set(m_sysvia, FUNC(via6522_device::write_cb2));
	m_analog->lpstb_handler().append([this](int state) { if (state) m_crtc->assert_light_pen_input(); });

	WD1770(config, m_wdfdc, 16_MHz_XTAL / 2);
	m_wdfdc->intrq_wr_callback().set(FUNC(bbcbp_state::fdc_intrq_w));
	m_wdfdc->drq_wr_callback().set(FUNC(bbcbp_state::fdc_drq_w));

	FLOPPY_CONNECTOR(config, "wdfdc:0", bbc_floppies, "525qd", bbc_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "wdfdc:1", bbc_floppies, "525qd", bbc_state::floppy_formats).enable_sound(true);

	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("network", FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(FUNC(bbcbp_state::adlc_irq_w));
	//m_adlc->out_rts_cb().

	econet_device &econet(ECONET(config, "network", 0));
	econet.clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	econet.data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));
	ECONET_SLOT(config, "econet", "network", econet_devices);

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, 16_MHz_XTAL / 16, bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_1mhzbus->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	m_1mhzbus->nmi_handler().set(FUNC(bbcbp_state::bus_nmi_w));

	BBC_TUBE_SLOT(config, m_tube, bbc_tube_devices, nullptr);
	m_tube->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<4>));

	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, nullptr);
	m_userport->cb1_handler().set(m_uservia, FUNC(via6522_device::write_cb1));
	m_userport->cb2_handler().set(m_uservia, FUNC(via6522_device::write_cb2));

	BBC_ROMSLOT32(config, m_rom[0x02], bbc_rom_devices, nullptr); // IC35 32K socket
	BBC_ROMSLOT32(config, m_rom[0x04], bbc_rom_devices, nullptr); // IC44 32K socket
	BBC_ROMSLOT32(config, m_rom[0x06], bbc_rom_devices, nullptr); // IC57 32K socket
	BBC_ROMSLOT32(config, m_rom[0x08], bbc_rom_devices, nullptr); // IC62 32K socket
	BBC_ROMSLOT32(config, m_rom[0x0a], bbc_rom_devices, nullptr); // IC68 32K socket

	BBC_INTERNAL_SLOT(config, m_internal, 16_MHz_XTAL, bbcbp_internal_devices, nullptr);
	m_internal->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<5>));
	m_internal->nmi_handler().set(FUNC(bbcbp_state::bus_nmi_w));

	SOFTWARE_LIST(config, "cass_ls").set_original("bbc_cass").set_filter("A,B");
	SOFTWARE_LIST(config, "rom_ls").set_original("bbc_rom").set_filter("B+");
	SOFTWARE_LIST(config, "flop_ls_b").set_original("bbcb_flop");
	SOFTWARE_LIST(config, "flop_ls_b_orig").set_original("bbcb_flop_orig");
	SOFTWARE_LIST(config, "hdd_ls").set_original("bbc_hdd").set_filter("B");
}


void bbcbp_state::bbcbp128(machine_config &config)
{
	bbcbp(config);

	m_ram->set_default_size("128K");

	BBC_ROMSLOT32(config, m_rom[0x00], bbc_rom_devices, "ram").set_fixed_ram(true);
	BBC_ROMSLOT32(config, m_rom[0x0c], bbc_rom_devices, "ram").set_fixed_ram(true);
}


void bbcbp_state::cfa3000bp(machine_config &config)
{
	bbcbp(config);

	// no floppy drives
	m_wdfdc->subdevice<floppy_connector>("0")->set_default_option(nullptr);
	m_wdfdc->subdevice<floppy_connector>("1")->set_default_option(nullptr);

	// keyboard
	m_userport->set_default_option("cfa3000kbd").set_fixed(true);

	// option board
	m_1mhzbus->set_default_option("cfa3000opt").set_fixed(true);

	// analogue dials/sensors
	m_analog->set_default_option("cfa3000a").set_fixed(true);

	config.device_remove("cass_ls");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");
}


/***************************************************************************

    Acorn Business Computers

****************************************************************************/

void bbcbp_state::abc110(machine_config &config)
{
	bbcbp(config);

	ABC_KBD(config.replace(), m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcbp_state::trigger_reset));

	// single floppy drive
	m_wdfdc->subdevice<floppy_connector>("1")->set_default_option(nullptr);

	// Acorn Z80 co-processor
	m_tube->set_default_option("z80w").set_fixed(true);

	// Acorn Winchester Disc 10MB
	m_1mhzbus->set_default_option("awhd").set_fixed(true);
}


void bbcbp_state::acw443(machine_config &config)
{
	abc110(config);

	// 32016 co-processor
	m_tube->set_default_option("32016l").set_fixed(true);

	// Acorn Winchester Disc 20MB
	m_1mhzbus->set_default_option("awhd").set_fixed(true);

	SOFTWARE_LIST(config, "flop_ls_32016").set_original("bbc_flop_32016");
}


void bbcbp_state::abc310(machine_config &config)
{
	abc110(config);

	// Acorn 80286 co-processor
	m_tube->set_default_option("80286").set_fixed(true);

	// Acorn Winchester Disc 10MB
	m_1mhzbus->set_default_option("awhd").set_fixed(true);

	// Acorn Mouse
	m_userport->set_default_option("m512mouse");
}


/***************************************************************************

    Econet X25 Gateway

****************************************************************************/

void bbcbp_state::econx25(machine_config &config)
{
	bbcbp(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &bbcbp_state::econx25_mem);

	// Econet X25 Gateway co-processor
	m_tube->set_default_option("x25").set_fixed(true);

	// fdc and speech not fitted
	config.device_remove("wdfdc");
	config.device_remove("vsm");
	config.device_remove("vsp");

	subdevice<centronics_device>("printer")->set_default_option(nullptr);

	config.device_remove("cass_ls");
	config.device_remove("flop_ls_b");
	config.device_remove("flop_ls_b_orig");
}


ROM_START(bbcbp)
	// page 0  00000  SWRAM (B+ 128K only)           // page 8  20000  IC62 32K IN PAGE 9
	// page 1  04000  SWRAM (B+ 128K only)           // page 9  24000  IC62
	// page 2  08000  IC35 32K IN PAGE 3             // page 10 28000  IC68 32K IN PAGE 11
	// page 3  0c000  IC35                           // page 11 2c000  IC68
	// page 4  10000  IC44 32K IN PAGE 5             // page 12 30000  SWRAM (B+ 128K only)
	// page 5  14000  IC44 ADFS                      // page 13 34000  SWRAM (B+ 128K only)
	// page 6  18000  IC57 32K IN PAGE 7             // page 14 38000  IC71 32K IN PAGE 15
	// page 7  1c000  IC57 DDFS                      // page 15 3C000  IC71 BASIC
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("bpos2.ic71",  0x3c000, 0x8000, CRC(9f356396) SHA1(ea7d3a7e3ee1ecfaa1483af994048057362b01f2))
	//ROM_LOAD("adfs130.rom", 0x14000, 0x4000, CRC(d3855588) SHA1(301fd05c475a629c4bec70510d4507256a5b00d8)) // not fitted as standard
	ROM_LOAD("ddfs223.rom", 0x1c000, 0x4000, CRC(7891f9b7) SHA1(0d7ed0b0b3852cb61970ada1993244f2896896aa))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("cm62024.bin", 0x3c000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


#define rom_bbcbp128 rom_bbcbp


ROM_START(abc110)
	// page 0  00000                                 // page 8  20000  IC62 32K IN PAGE 9
	// page 1  04000  IC71 selectable with link S13  // page 9  24000  IC62
	// page 2  08000  IC35 32K IN PAGE 3             // page 10 28000  IC68 32K IN PAGE 11
	// page 3  0c000  IC35                           // page 11 2c000  IC68
	// page 4  10000  IC44 32K IN PAGE 5             // page 12 30000
	// page 5  14000  IC44 DDFS                      // page 13 34000
	// page 6  18000  IC57 32K IN PAGE 7             // page 14 38000
	// page 7  1c000  IC57 ADFS                      // page 15 3C000  IC71 BASIC
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "200", "MOS2.00")
	ROMX_LOAD("mos200.rom",     0x40000, 0x4000, CRC(5e88f994) SHA1(76235ff15d736f5def338f73ac7497c41b916505), ROM_BIOS(0))
	ROMX_LOAD("basic200.rom",   0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "123stor", "MOS1.23 + ViewStore")
	ROMX_LOAD("mos123stor.rom", 0x3c000, 0x8000, CRC(4e84f452) SHA1(145ee54f04b3eb4d0e5afaabe21915be48db3c54), ROM_BIOS(1)) // rom 15 3C000 ViewStore
	ROM_SYSTEM_BIOS(2, "123", "MOS1.23")
	ROMX_LOAD("mos123.rom",     0x40000, 0x4000, CRC(90d31d08) SHA1(42a01892cf8bd2ada4db1c8b36aff80c85eb5dcb), ROM_BIOS(2))
	ROMX_LOAD("basic200.rom",   0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "120", "MOS1.20")
	ROMX_LOAD("mos120.rom",     0x40000, 0x4000, CRC(0a1e83a0) SHA1(21dc3a94eef7c003b194686730fb461779f44925), ROM_BIOS(3))
	ROMX_LOAD("basic200.rom",   0x3c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(3))

	//ROM_LOAD("ddfs223.rom",   0x14000, 0x4000, CRC(7891f9b7) SHA1(0d7ed0b0b3852cb61970ada1993244f2896896aa))
	ROM_LOAD("acwddfs225.rom",  0x14000, 0x4000, CRC(7d0f9016) SHA1(bdfe44c79e18142d747436627e71a362a04cf746))
	ROM_LOAD("adfs130.rom",     0x1c000, 0x4000, CRC(d3855588) SHA1(301fd05c475a629c4bec70510d4507256a5b00d8))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("cm62024.bin", 0x3c000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


#define rom_abc310 rom_abc110


ROM_START(acw443)
	// page 0  00000                                 // page 8  20000  IC62 32K IN PAGE 9
	// page 1  04000  IC71 selectable with link S13  // page 9  24000  IC62 ADFS
	// page 2  08000  IC35 32K IN PAGE 3             // page 10 28000  IC68 BASIC
	// page 3  0c000  IC35 DNFS                      // page 11 2c000  IC68 unused OS?
	// page 4  10000  IC44 32K IN PAGE 5             // page 12 30000
	// page 5  14000  IC44 ACW DFS                   // page 13 34000
	// page 6  18000  IC57 32K IN PAGE 7             // page 14 38000
	// page 7  1c000  IC57 TERMINAL                  // page 15 3C000  IC71 selectable with link S13
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "210", "MOS2.10")
	ROMX_LOAD("acwmos210.rom",     0x40000, 0x4000, CRC(168d6753) SHA1(dcd01d8f5f6e0cd92ae626ca52a3db71abf5d282), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "200", "MOS2.00")
	ROMX_LOAD("mos200.rom",        0x40000, 0x4000, CRC(5e88f994) SHA1(76235ff15d736f5def338f73ac7497c41b916505), ROM_BIOS(1))

	ROM_LOAD("dnfs120-201666.rom", 0x0c000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))
	ROM_LOAD("acwddfs225.rom",     0x14000, 0x4000, CRC(7d0f9016) SHA1(bdfe44c79e18142d747436627e71a362a04cf746))
	ROM_LOAD("acwterminal.rom",    0x1c000, 0x4000, CRC(81afaeb9) SHA1(6618ed9158776b4b8aa030957bd19ba77e4a993c))
	ROM_LOAD("adfs130.rom",        0x24000, 0x4000, CRC(d3855588) SHA1(301fd05c475a629c4bec70510d4507256a5b00d8))
	ROM_LOAD("basic200.rom",       0x28000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("cm62024.bin", 0x3c000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


ROM_START(econx25)
	// page 0  00000                                 // page 8  20000  IC62 BASIC
	// page 1  04000  IC71 selectable with link S13  // page 9  24000  IC62 unused OS
	// page 2  08000  IC35 32K IN PAGE 3             // page 10 28000  IC68 32K IN PAGE 11
	// page 3  0c000  IC35                           // page 11 2c000  IC68
	// page 4  10000  IC44 32K IN PAGE 5             // page 12 30000
	// page 5  14000  IC44                           // page 13 34000
	// page 6  18000  IC57 32K IN PAGE 7             // page 14 38000
	// page 7  1c000  IC57 ANFS                      // page 15 3C000  IC71 selectable with link S13
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("0246,201_01_x25os.rom", 0x40000, 0x4000, CRC(8b652337) SHA1(6a5c7ace255c8ac96c983d5ba67084fbd71ff61e))
	ROM_LOAD("2201,248_03_anfs.rom",  0x1c000, 0x4000, CRC(744a60a7) SHA1(c733b108d74cf3b1c5de395335236800a7c9c0d8))
	ROM_LOAD("0201,241_01_bpos2.rom", 0x20000, 0x8000, CRC(9f356396) SHA1(ea7d3a7e3ee1ecfaa1483af994048057362b01f2))
	// X25 TSI is in IC37 which is supposed to take a speech PHROM, so not sure where this is mapped
	ROM_LOAD("0246,215_02_x25tsi_v0.51.rom", 0x0c000, 0x4000, CRC(71dd84e4) SHA1(bbfa892fdcc6f753dda5134ecb97cc7c42b959c2))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)
ROM_END


ROM_START(cfa3000bp)
	// page 0  00000  SWRAM (B+ 128K only)           // page 8  20000  IC62 32K IN PAGE 9
	// page 1  04000  SWRAM (B+ 128K only)           // page 9  24000  IC62
	// page 2  08000  IC35 32K IN PAGE 3             // page 10 28000  IC68 32K IN PAGE 11
	// page 3  0c000  IC35                           // page 11 2c000  IC68
	// page 4  10000  IC44 32K IN PAGE 5             // page 12 30000  SWRAM (B+ 128K only)
	// page 5  14000  IC44                           // page 13 34000  SWRAM (B+ 128K only)
	// page 6  18000  IC57 32K IN PAGE 7             // page 14 38000  IC71 32K IN PAGE 15
	// page 7  1c000  IC57                           // page 15 3C000  IC71 BASIC
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("bpos2.ic71",     0x3c000, 0x8000, CRC(9f356396) SHA1(ea7d3a7e3ee1ecfaa1483af994048057362b01f2))
	ROM_SYSTEM_BIOS(0, "40", "Issue 4")
	ROMX_LOAD("cfa3000_3.rom", 0x14000, 0x4000, CRC(4f246cd5) SHA1(6ba9625248c585deed5c651a889eecc86384a60d), ROM_BIOS(0))
	ROMX_LOAD("cfa3000_4.rom", 0x1c000, 0x4000, CRC(ca0e30fd) SHA1(abddc7ba6d16855ebda2ef55fe8662bc545ae755), ROM_BIOS(0))
	ROMX_LOAD("cfa3000_s.rom", 0x24000, 0x4000, CRC(71fd4c8a) SHA1(5bad70ee55403bc0191f6b189c9b6e5effdbca4c), ROM_BIOS(0))

	// link S13 set for BASIC to take low priority ROM numbers 0/1
	ROM_COPY("rom", 0x3c000, 0x4000, 0x4000)
	ROM_FILL(0x3c000, 0x4000, 0xff)

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("cm62024.bin", 0x3c000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


#define rom_ltmpbp rom_bbcbp

} // anonymous namespace


//    YEAR  NAME        PARENT  COMPAT MACHINE     INPUT    CLASS         INIT       COMPANY                        FULLNAME                              FLAGS
COMP( 1985, bbcbp,      0,      bbcb,  bbcbp,      bbcbp,   bbcbp_state,  init_bbc,  "Acorn Computers",             "BBC Micro Model B+ 64K",             MACHINE_IMPERFECT_GRAPHICS )
COMP( 1985, bbcbp128,   bbcbp,  0,     bbcbp128,   bbcbp,   bbcbp_state,  init_bbc,  "Acorn Computers",             "BBC Micro Model B+ 128K",            MACHINE_IMPERFECT_GRAPHICS )
COMP( 1985, abc110,     bbcbp,  0,     abc110,     abc,     bbcbp_state,  init_bbc,  "Acorn Computers",             "ABC 110",                            MACHINE_NOT_WORKING )
COMP( 1985, acw443,     bbcbp,  0,     acw443,     abc,     bbcbp_state,  init_bbc,  "Acorn Computers",             "ABC 210/Cambridge Workstation",      MACHINE_NOT_WORKING )
COMP( 1985, abc310,     bbcbp,  0,     abc310,     abc,     bbcbp_state,  init_bbc,  "Acorn Computers",             "ABC 310",                            MACHINE_NOT_WORKING )
COMP( 1986, econx25,    bbcbp,  0,     econx25,    bbcbp,   bbcbp_state,  init_bbc,  "Acorn Computers",             "Econet X25 Gateway",                 MACHINE_NOT_WORKING )

// Industrial
COMP( 1985, ltmpbp,     bbcbp,  0,     bbcbp,      abc,     bbcbp_state,  init_ltmp, "Lawrie T&M Ltd.",             "LTM Portable (B+)",                  MACHINE_IMPERFECT_GRAPHICS )
COMP( 198?, cfa3000bp,  bbcbp,  0,     cfa3000bp,  bbcbp,   bbcbp_state,  init_cfa,  "Tinsley Medical Instruments", "Henson CFA 3000 (B+)",               MACHINE_NOT_WORKING )
