// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/******************************************************************************

    BBC Model A,B

    ANA01 - Model A
    ANA02 - Model A with Econet interface

    ANB01 - Model B
    ANB02 - Model B with Econet interface
    ANB03 - Model B with Disc interface
    ANB04 - Model B with Disc and Econet interfaces

    GNB14 - Model B with Disc, Econet & Speech (German export)
    UNB09 - Model B with Disc, Econet & Speech (US export)
            Model B with Disc (Norway dealer import)

******************************************************************************/

#include "emu.h"
#include "bbc.h"
#include "acorn_serproc.h"
#include "bbc_kbd.h"

#include "bus/bbc/vsp/slot.h"
#include "sound/tms5220.h"

#include "formats/uef_cas.h"
#include "formats/csw_cas.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "bbc.lh"


namespace {

class bbcb_state : public bbc_state
{
public:
	bbcb_state(const machine_config &mconfig, device_type type, const char *tag)
		: bbc_state(mconfig, type, tag)
		, m_kbd(*this, "kbd")
		, m_sn(*this, "sn")
		, m_vsp(*this, "vsp")
		, m_adlc(*this, "mc6854")
		, m_statid(*this, "STATID")
	{ }

	void bbca(machine_config &config);
	void bbcb(machine_config &config);
	void bbcb_de(machine_config &config);
	void bbcb_no(machine_config &config);
	void bbcb_us(machine_config &config);
	void dolphinm(machine_config &config);
	void sist1(machine_config &config);
	void torchf(machine_config &config);
	void torchh(machine_config &config);
	void torch301(machine_config &config);
	void torch725(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<bbc_kbd_device> m_kbd;
	required_device<sn76489a_device> m_sn;
	optional_device<bbc_vsp_slot_device> m_vsp;
	optional_device<mc6854_device> m_adlc;
	optional_ioport m_statid;

	void bbca_mem(address_map &map) ATTR_COLD;
	void bbcb_mem(address_map &map) ATTR_COLD;
	void dolphin_mem(address_map &map) ATTR_COLD;

	void trigger_reset(int state);

	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	uint8_t romsel_r(offs_t offset);
	void romsel_w(offs_t offset, uint8_t data);
	uint8_t paged_r(offs_t offset);
	void paged_w(offs_t offset, uint8_t data);

	uint8_t sysvia_pa_r();
	void sysvia_pa_w(uint8_t data);
	void update_sdb();
	uint8_t sysvia_pb_r();
	void sysvia_pb_w(uint8_t data);

	uint8_t m_sdb = 0x00;
};


void bbcb_state::machine_start()
{
	bbc_state::machine_start();

	save_item(NAME(m_sdb));
}

void bbcb_state::machine_reset()
{
	// install econet hardware
	if (m_bbcconfig.read_safe(0) & 0x04)
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xfea0, 0xfea3, 0, 0x1c, 0, emu::rw_delegate(*m_adlc, FUNC(mc6854_device::read)), emu::rw_delegate(*m_adlc, FUNC(mc6854_device::write)));
	else
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xfea0, 0xfea3, 0, 0x1c, 0, read8smo_delegate(*this, []() { return 0xfe; }, "fe_r"));
}


void bbcb_state::bbca_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));                                    //    0000-7FFF                 Regular RAM
	map(0x8000, 0xbfff).rw(FUNC(bbcb_state::paged_r), FUNC(bbcb_state::paged_w));                                      //    8000-BFFF                 Paged ROM/RAM
	map(0xc000, 0xffff).rom().region("mos", 0);                                                                        //    C000-FBFF                 OS ROM
	map(0xfe00, 0xfeff).lr8(NAME([]() { return 0xfe; })).nopw();                                                       //    FE00-FEFF                 SHEILA Address Page
	map(0xfe00, 0xfe00).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));      //    FE00-FE07  6845 CRTC      Video controller
	map(0xfe01, 0xfe01).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0xfe08, 0xfe09).mirror(0x06).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));            //    FE08-FE0F  6850 ACIA      Serial controller
	map(0xfe10, 0xfe17).rw("serproc", FUNC(acorn_serproc_device::read), FUNC(acorn_serproc_device::write));            //    FE10-FE17  Serial ULA     Serial system chip
	map(0xfe20, 0xfe2f).w(FUNC(bbcb_state::video_ula_w));                                                              // W: FE20-FE2F  Video ULA      Video system chip
	map(0xfe30, 0xfe3f).w(FUNC(bbcb_state::romsel_w));                                                                 // W: FE30-FE3F  74LS161        Paged ROM selector
	map(0xfe40, 0xfe4f).mirror(0x10).m(m_sysvia, FUNC(via6522_device::map));                                           //    FE40-FE5F  6522 VIA       SYSTEM VIA
}


void bbcb_state::bbcb_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(FUNC(bbcb_state::ram_r), FUNC(bbcb_state::ram_w));                                          //    0000-7FFF                 Regular RAM
	map(0x8000, 0xbfff).rw(FUNC(bbcb_state::paged_r), FUNC(bbcb_state::paged_w));                                      //    8000-BFFF                 Paged ROM/RAM
	map(0xc000, 0xffff).rw(FUNC(bbcb_state::mos_r), FUNC(bbcb_state::mos_w));                                          //    C000-FBFF                 OS ROM
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
	map(0xfe20, 0xfe2f).w(FUNC(bbcb_state::video_ula_w));                                                              // W: FE20-FE2F  Video ULA      Video system chip
	map(0xfe30, 0xfe3f).rw(FUNC(bbcb_state::romsel_r), FUNC(bbcb_state::romsel_w));                                    // W: FE30-FE3F  84LS161        Paged ROM selector
	map(0xfe40, 0xfe4f).mirror(0x10).m(m_sysvia, FUNC(via6522_device::map));                                           //    FE40-FE5F  6522 VIA       SYSTEM VIA
	map(0xfe60, 0xfe6f).mirror(0x10).m(m_uservia, FUNC(via6522_device::map));                                          //    FE60-FE7F  6522 VIA       USER VIA
	map(0xfe80, 0xfe9f).rw(m_fdc, FUNC(bbc_fdc_slot_device::read), FUNC(bbc_fdc_slot_device::write));                  //    FE80-FE9F  8271 FDC       Floppy disc controller
	map(0xfea0, 0xfea3).mirror(0x1c).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));                //    FEA0-FEBF  68B54 ADLC     ECONET controller
	map(0xfec0, 0xfec3).mirror(0x1c).rw("upd7002", FUNC(upd7002_device::read), FUNC(upd7002_device::write));           //    FEC0-FEDF  uPD7002        Analogue to digital converter
	map(0xfee0, 0xfeff).rw(m_tube, FUNC(bbc_tube_slot_device::host_r), FUNC(bbc_tube_slot_device::host_w));            //    FEE0-FEFF  Tube ULA       Tube system interface
}


void bbcb_state::dolphin_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));                                    //    0000-7FFF                 Regular RAM
	map(0x8000, 0xbfff).rw(FUNC(bbcb_state::paged_r), FUNC(bbcb_state::paged_w));                                      //    8000-BFFF                 Paged ROM/RAM
	map(0xc000, 0xffff).rom().region("mos", 0);                                                                        //    C000-FBFF                 OS ROM
	//map(0xfc00, 0xfcff).rw(m_1mhzbus, FUNC(bbc_1mhzbus_slot_device::fred_r), FUNC(bbc_1mhzbus_slot_device::fred_w));   //    FC00-FCFF                 FRED Address Page
	//map(0xfd00, 0xfdff).rw(m_1mhzbus, FUNC(bbc_1mhzbus_slot_device::jim_r), FUNC(bbc_1mhzbus_slot_device::jim_w));     //    FD00-FDFF                 JIM Address Page
	map(0xfe00, 0xfeff).lr8(NAME([]() { return 0xfe; })).nopw();                                                       //    FE00-FEFF                 SHEILA Address Page
	map(0xfe00, 0xfe00).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));      //    FE00-FE07  6845 CRTC      Video controller
	map(0xfe01, 0xfe01).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0xfe08, 0xfe09).mirror(0x06).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));            //    FE08-FE0F  6850 ACIA      Serial controller
	map(0xfe10, 0xfe17).rw("serproc", FUNC(acorn_serproc_device::read), FUNC(acorn_serproc_device::write));            //    FE10-FE17  Serial ULA     Serial system chip
	map(0xfe20, 0xfe2f).w(FUNC(bbcb_state::video_ula_w));                                                              // W: FE20-FE2F  Video ULA      Video system chip
	map(0xfe30, 0xfe3f).lw8(NAME([this](uint8_t data) { m_romsel = data & 0x0f; }));                                   // W: FE30-FE3F  84LS161        Paged ROM selector
	map(0xfe40, 0xfe4f).mirror(0x10).m(m_sysvia, FUNC(via6522_device::map));                                           //    FE40-FE5F  6522 VIA       SYSTEM VIA
	map(0xfe60, 0xfe6f).mirror(0x10).m(m_uservia, FUNC(via6522_device::map));                                          //    FE60-FE7F  6522 VIA       USER VIA
	map(0xfe80, 0xfe9f).rw(m_fdc, FUNC(bbc_fdc_slot_device::read), FUNC(bbc_fdc_slot_device::write));                  //    FE80-FE9F  8271 FDC       Floppy disc controller
	//map(0xfec0, 0xfec3).mirror(0x1c).rw("upd7002", FUNC(upd7002_device::read), FUNC(upd7002_device::write));           //    FEC0-FEDF  uPD7002        Analogue to digital converter
}


uint8_t bbcb_state::ram_r(offs_t offset)
{
	if (m_internal && m_internal->overrides_ram())
		return m_internal->ram_r(offset);
	else
		return m_ram->pointer()[offset & m_ram->mask()];
}

void bbcb_state::ram_w(offs_t offset, uint8_t data)
{
	if (m_internal && m_internal->overrides_ram())
		m_internal->ram_w(offset, data);
	else
		m_ram->pointer()[offset & m_ram->mask()] = data;
}

uint8_t bbcb_state::romsel_r(offs_t offset)
{
	if (m_internal && m_internal->overrides_rom())
		return m_internal->romsel_r(offset);
	else
		return 0xfe;
}

void bbcb_state::romsel_w(offs_t offset, uint8_t data)
{
	// no sideways expansion board fitted so address only the 4 on board ROM sockets
	m_romsel = data & 0x03;

	// pass ROMSEL to internal expansion board
	if (m_internal && m_internal->overrides_rom())
		m_internal->romsel_w(offset, data);
}

uint8_t bbcb_state::paged_r(offs_t offset)
{
	uint8_t data;

	if (m_internal && m_internal->overrides_rom())
	{
		data = m_internal->paged_r(offset);
	}
	else
	{
		if (m_rom[m_romsel] && m_rom[m_romsel]->present())
		{
			data = m_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_rom->base()[offset + (m_romsel << 14)];
		}
	}

	return data;
}

void bbcb_state::paged_w(offs_t offset, uint8_t data)
{
	if (m_internal && m_internal->overrides_rom())
	{
		m_internal->paged_w(offset, data);
	}
	else
	{
		if (m_rom[m_romsel])
		{
			m_rom[m_romsel]->write(offset, data);
		}
	}
}


uint8_t bbcb_state::sysvia_pa_r()
{
	update_sdb();

	return m_sdb;
}

void bbcb_state::sysvia_pa_w(uint8_t data)
{
	m_sdb = data;

	update_sdb();
}


void bbcb_state::update_sdb()
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
			m_sdb = m_vsp->read();
			break;
		case tms5200_device::WS:
			m_vsp->write(m_sdb);
			break;
		}
	}

	// keyboard
	m_sdb = m_kbd->read(m_sdb);
}


uint8_t bbcb_state::sysvia_pb_r()
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

void bbcb_state::sysvia_pb_w(uint8_t data)
{
	m_latch->write_nibble_d3(data);

	if (m_analog)
	{
		m_analog->pb_w(data & 0x30);
	}

	update_sdb();
}


void bbcb_state::trigger_reset(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);

	if (!state)
	{
		if (m_uservia) m_uservia->reset();
		if (m_adlc) m_adlc->reset();
		if (m_fdc) m_fdc->reset();
		if (m_1mhzbus) m_1mhzbus->reset();
		if (m_tube) m_tube->reset();
		if (m_internal) m_internal->reset();
	}
}


static INPUT_PORTS_START(bbc_statid)
	PORT_START("STATID")
	PORT_DIPNAME(0xff, 0xfe, "Econet ID") PORT_DIPLOCATION("S11:1,2,3,4,5,6,7,8")
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


static INPUT_PORTS_START(bbca)
	PORT_INCLUDE(bbc_config)
INPUT_PORTS_END

static INPUT_PORTS_START(bbcb)
	PORT_INCLUDE(bbc_config)
	PORT_INCLUDE(bbc_statid)
INPUT_PORTS_END

static INPUT_PORTS_START(torch)
	PORT_INCLUDE(bbc_statid)
INPUT_PORTS_END


static const char *const bbc_sample_names[] =
{
	"*bbc",
	"motoroff",
	"motoron",
	nullptr
};


/***************************************************************************

    BBC Micro

****************************************************************************/

void bbcb_state::bbca(machine_config &config)
{
	M6502(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &bbcb_state::bbca_mem);
	config.set_perfect_quantum(m_maincpu);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	RAM(config, m_ram).set_default_size("16K").set_extra_options("32K");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 312, 0, 256);
	m_screen->set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette).set_entries(16);

	SAA5050(config, m_trom, 12_MHz_XTAL / 2);

	HD6845S(config, m_crtc, 16_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_reconfigure_callback(FUNC(bbcb_state::crtc_reconfigure));
	m_crtc->set_update_row_callback(FUNC(bbcb_state::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(bbcb_state::de_changed));
	m_crtc->out_hsync_callback().set(FUNC(bbcb_state::hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(bbcb_state::vsync_changed));

	config.set_default_layout(layout_bbc);

	LS259(config, m_latch);
	m_latch->q_out_cb<3>().set(m_kbd, FUNC(bbc_kbd_device::write_kb_en));
	m_latch->q_out_cb<6>().set_output("capslock_led");
	m_latch->q_out_cb<7>().set_output("shiftlock_led");

	MOS6522(config, m_sysvia, 16_MHz_XTAL / 16);
	m_sysvia->readpa_handler().set(FUNC(bbcb_state::sysvia_pa_r));
	m_sysvia->writepa_handler().set(FUNC(bbcb_state::sysvia_pa_w));
	m_sysvia->readpb_handler().set(FUNC(bbcb_state::sysvia_pb_r));
	m_sysvia->writepb_handler().set(FUNC(bbcb_state::sysvia_pb_w));
	m_sysvia->cb2_handler().set([this](int state) { if (state) m_crtc->assert_light_pen_input(); });
	m_sysvia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	BBC_KBD(config, m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcb_state::trigger_reset));

	SPEAKER(config, "mono").front_center();

	SN76489A(config, m_sn, 16_MHz_XTAL / 4);
	m_sn->add_route(ALL_OUTPUTS, "mono", 1.0);

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
	serproc.casmo_handler().set(FUNC(bbcb_state::cassette_motor));
	serproc.casin_handler("cassette", FUNC(cassette_image_device::input));
	serproc.casout_handler("cassette", FUNC(cassette_image_device::output));

	rs232_port_device &rs423(RS232_PORT(config, "rs423", default_rs232_devices, nullptr));
	rs423.rxd_handler().set("serproc", FUNC(acorn_serproc_device::write_din));
	rs423.cts_handler().set("serproc", FUNC(acorn_serproc_device::write_ctsi));

	BBC_ROMSLOT16(config, m_rom[0], bbc_rom_devices, nullptr); // IC101
	BBC_ROMSLOT16(config, m_rom[1], bbc_rom_devices, nullptr); // IC100
	BBC_ROMSLOT16(config, m_rom[2], bbc_rom_devices, nullptr); // IC88
	BBC_ROMSLOT16(config, m_rom[3], bbc_rom_devices, nullptr); // IC52

	SOFTWARE_LIST(config, "cass_ls").set_original("bbc_cass").set_filter("A");
	SOFTWARE_LIST(config, "rom_ls").set_original("bbc_rom").set_filter("B");
}


void bbcb_state::bbcb(machine_config &config)
{
	bbca(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &bbcb_state::bbcb_mem);

	m_ram->set_default_size("32K");

	BBC_VSP_SLOT(config, m_vsp, bbc_vsp_devices, "speech");
	m_vsp->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_vsp->set_vsm_region("vsm");

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

	BBC_FDC_SLOT(config, m_fdc, 16_MHz_XTAL / 2, bbc_fdc_devices, "acorn8271");
	m_fdc->intrq_wr_callback().set(FUNC(bbcb_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(bbcb_state::fdc_drq_w));

	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("network", FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(FUNC(bbcb_state::adlc_irq_w));
	//m_adlc->out_rts_cb().

	econet_device &econet(ECONET(config, "network", 0));
	econet.clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	econet.data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));
	ECONET_SLOT(config, "econet", "network", econet_devices);

	BBC_1MHZBUS_SLOT(config, m_1mhzbus, 16_MHz_XTAL / 16, bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_1mhzbus->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	m_1mhzbus->nmi_handler().set(FUNC(bbcb_state::bus_nmi_w));

	BBC_TUBE_SLOT(config, m_tube, bbc_tube_devices, nullptr);
	m_tube->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<4>));

	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, nullptr);
	m_userport->cb1_handler().set(m_uservia, FUNC(via6522_device::write_cb1));
	m_userport->cb2_handler().set(m_uservia, FUNC(via6522_device::write_cb2));

	BBC_INTERNAL_SLOT(config, m_internal, 16_MHz_XTAL, bbcb_internal_devices, nullptr);
	m_internal->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<5>));
	m_internal->nmi_handler().set(FUNC(bbcb_state::bus_nmi_w));

	m_irqs->output_handler().append(m_internal, FUNC(bbc_internal_slot_device::irq6502_w));

	subdevice<software_list_device>("cass_ls")->set_filter("A,B");
	SOFTWARE_LIST(config, "flop_ls_b").set_original("bbcb_flop");
	SOFTWARE_LIST(config, "flop_ls_b_orig").set_original("bbcb_flop_orig");
	SOFTWARE_LIST(config, "hdd_ls").set_original("bbc_hdd").set_filter("B");
	SOFTWARE_LIST(config, "vsm_ls").set_original("bbc_vsm");
}


void bbcb_state::bbcb_de(machine_config &config)
{
	bbcb(config);

	m_fdc->set_fixed(true);
	m_fdc->set_insert_rom(false);
}


void bbcb_state::bbcb_no(machine_config &config)
{
	bbcb(config);

	BBC_KBD_NO(config.replace(), m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcb_state::trigger_reset));

	m_fdc->set_fixed(true);
	m_fdc->set_insert_rom(false);
}


void bbcb_state::bbcb_us(machine_config &config)
{
	bbcb(config);

	m_screen->set_raw(16_MHz_XTAL, 1024, 0, 640, 262, 0, 200);

	m_fdc->set_fixed(true);
	m_fdc->set_insert_rom(false);

	SOFTWARE_LIST(config, "flop_ls_b_us").set_original("bbcb_flop_us");
}


void bbcb_state::dolphinm(machine_config &config)
{
	bbca(config);

	// The following ports are not fitted (maybe optional upgrades):
	//   Speech sockets (TMS5220/TMS6100)
	//   1MHz Expansion Bus Interface
	//   Analogue Interface, and uPD7002
	//   Econet, and MC6854 (no provision on board)

	m_maincpu->set_addrmap(AS_PROGRAM, &bbcb_state::dolphin_mem);

	m_ram->set_default_size("32K");

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

	BBC_FDC_SLOT(config, m_fdc, 16_MHz_XTAL / 2, bbc_fdc_devices, "acorn1770").set_fixed(true); // WD1772
	m_fdc->set_insert_rom(false);
	m_fdc->intrq_wr_callback().set(FUNC(bbcb_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(bbcb_state::fdc_drq_w));

	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, nullptr);
	m_userport->cb1_handler().set(m_uservia, FUNC(via6522_device::write_cb1));
	m_userport->cb2_handler().set(m_uservia, FUNC(via6522_device::write_cb2));

	config.device_remove("romslot1");
	config.device_remove("romslot3");
	BBC_ROMSLOT32(config.replace(), m_rom[0], bbc_rom_devices, nullptr); // IC52 32K socket
	BBC_ROMSLOT32(config.replace(), m_rom[2], bbc_rom_devices, nullptr); // IC53 32K socket

	subdevice<software_list_device>("cass_ls")->set_filter("A,B");
	SOFTWARE_LIST(config, "flop_ls_b").set_original("bbcb_flop");
	SOFTWARE_LIST(config, "flop_ls_b_orig").set_original("bbcb_flop_orig");

	// TODO: Cartridge Bus Interface (in place of Tube port, unknown purpose/pinout)
}


/***************************************************************************

    Torch Computers

****************************************************************************/

void bbcb_state::torchf(machine_config &config)
{
	bbcb(config);

	TORCHB_KBD(config.replace(), m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcb_state::trigger_reset));

	m_fdc->set_fixed(true);
	m_fdc->set_insert_rom(false);

	// Torch Z80 Communicator co-processor
	m_tube->set_default_option("zep100").set_fixed(true);
}


void bbcb_state::torchh(machine_config &config)
{
	torchf(config);

	//subdevice<floppy_connector>("acorn8271:i8271:1")->set_default_option(nullptr);

	// 10MB or 21MB HDD
	m_1mhzbus->set_default_option("torchhd").set_fixed(true);
}


void bbcb_state::torch301(machine_config &config)
{
	torchf(config);

	TORCHI_KBD(config.replace(), m_kbd, 16_MHz_XTAL / 16);
	m_kbd->ca2_handler().set(m_sysvia, FUNC(via6522_device::write_ca2));
	m_kbd->rst_handler().set(FUNC(bbcb_state::trigger_reset));

	//subdevice<floppy_connector>("acorn8271:i8271:1")->set_default_option(nullptr);

	// Torch Z80 Communicator co-processor
	m_tube->set_default_option("zep100").set_fixed(true);
	m_tube->set_insert_rom(false);

	// 20MB HDD
	m_1mhzbus->set_default_option("torchhd").set_fixed(true);
}


void bbcb_state::torch725(machine_config &config)
{
	torch301(config);

	// Torch 68000 Atlas co-processor
	//m_tube->set_default_option("atlas").set_fixed(true);
	m_tube->set_insert_rom(false);
}


/***************************************************************************

    Cisco Systems

****************************************************************************/

void bbcb_state::sist1(machine_config &config)
{
	bbcb(config);

	m_1mhzbus->set_default_option("cisco").set_fixed(true);
}


ROM_START(bbca)
	ROM_REGION(0x4000, "mos", 0)
	ROM_SYSTEM_BIOS(0, "120", "OS 1.20")
	ROMX_LOAD("os12.rom",  0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "100", "OS 1.00")
	ROMX_LOAD("os10.rom",  0x0000, 0x4000, CRC(9679b8f8) SHA1(d35f6723132aabe3c4d00fc16fd9ecc6768df753), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "092", "OS 0.92")
	ROMX_LOAD("os092.rom", 0x0000, 0x4000, CRC(59ef7eb8) SHA1(dca33995c0d008a527efe923d03333394b01022c), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "010", "OS 0.10") // OS0.1 does not support ROM paging, load BASIC into all pages
	ROMX_LOAD("os01.rom",  0x0000, 0x4000, CRC(45ee0980) SHA1(4b0ece6dc139d5d3f4fabd023716fb6f25149b80), ROM_BIOS(3))

	// page 0 00000 IC52
	// page 1 04000 IC88
	// page 2 08000 IC100
	// page 3 0c000 IC101 BASIC
	ROM_REGION(0x40000, "rom", ROMREGION_ERASEFF)
	ROMX_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(0))
	ROMX_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(1))
	ROMX_LOAD("basic1.rom", 0x0c000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(2))
	ROMX_LOAD("basic1.rom", 0x00000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(3))
	ROM_RELOAD(             0x04000, 0x4000 )
	ROM_RELOAD(             0x08000, 0x4000 )
	ROM_RELOAD(             0x0c000, 0x4000 )
ROM_END


ROM_START(bbcb)
	ROM_REGION(0x4000, "mos", 0)
	ROM_SYSTEM_BIOS(0, "120", "OS 1.20")
	ROMX_LOAD("os12.rom",  0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "100", "OS 1.00")
	ROMX_LOAD("os10.rom",  0x0000, 0x4000, CRC(9679b8f8) SHA1(d35f6723132aabe3c4d00fc16fd9ecc6768df753), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "092", "OS 0.92")
	ROMX_LOAD("os092.rom", 0x0000, 0x4000, CRC(59ef7eb8) SHA1(dca33995c0d008a527efe923d03333394b01022c), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "010", "OS 0.10") // OS0.1 does not support ROM paging, load BASIC into all pages
	ROMX_LOAD("os01.rom",  0x0000, 0x4000, CRC(45ee0980) SHA1(4b0ece6dc139d5d3f4fabd023716fb6f25149b80), ROM_BIOS(3))

	// page 0 00000 IC52  DFS
	// page 1 04000 IC88
	// page 2 08000 IC100
	// page 3 0c000 IC101 BASIC
	ROM_REGION(0x40000, "rom", ROMREGION_ERASEFF)
	ROMX_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(0))
	ROMX_LOAD("basic2.rom", 0x0c000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281), ROM_BIOS(1))
	ROMX_LOAD("basic1.rom", 0x0c000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(2))
	ROMX_LOAD("basic1.rom", 0x00000, 0x4000, CRC(b3364108) SHA1(890f6e3e7fab3340f75b85e93ff29332bc9ecb2e), ROM_BIOS(3))
	ROM_RELOAD(             0x04000, 0x4000 )
	ROM_RELOAD(             0x08000, 0x4000 )
	ROM_RELOAD(             0x0c000, 0x4000 )

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("cm62024.bin", 0x3c000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


ROM_START(bbcb_de)
	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os_de.rom",   0x0000, 0x4000, CRC(b7262caf) SHA1(aadf90338ee9d1c85dfa73beba50e930c2a38f10))

	// page 0 00000 IC72 DFS
	// page 1 04000 IC73
	// page 2 08000 IC74
	// page 3 0c000 IC75 BASIC
	ROM_REGION(0x40000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("basic2.rom",  0xc000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("dfs10.rom",   0x0000, 0x4000, CRC(7e367e8c) SHA1(161f585dc45665ea77433c84afd2f95049f7f5a0))

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("cm62024.bin", 0x3c000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END


ROM_START(bbcb_no)
	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("nos12.rom", 0x0000, 0x4000, CRC(49859294) SHA1(2b6aecd33a43f296c20832524e47cc7e3a9c3b17))

	// page 0 00000 IC72 DFS
	// page 1 04000 IC73 VIEW2.1
	// page 2 08000 IC74
	// page 3 0c000 IC75 BASIC
	ROM_REGION(0x40000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("dfs0.9h.rom",  0x0000, 0x2000, CRC(af2fa873) SHA1(dbbec4d2540a854c120be3194c7566a2b79d153b))
	ROM_LOAD("viewa210.rom", 0x4000, 0x4000, CRC(4345359f) SHA1(88c93df1854f5fbe6cd6e5f0e29a8bf4ea3b5614))
	ROM_LOAD("basic2.rom",   0xc000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("vm61002.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


ROM_START(bbcb_us)
	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("usmos10.rom", 0x0000, 0x4000, CRC(c8e946a9) SHA1(83d91d089dca092d2c8b7c3650ff8143c9069b89))

	// page 0 00000 IC72 VIEW2.1
	// page 1 04000 IC73 US DNFS
	// page 2 08000 IC74 US BASIC
	// page 3 0c000 IC75
	ROM_REGION(0x40000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("usbasic3.rom", 0x8000, 0x4000, CRC(161b9539) SHA1(b39014610a968789afd7695aa04d1277d874405c))
	ROM_LOAD("viewa210.rom", 0x0000, 0x4000, CRC(4345359f) SHA1(88c93df1854f5fbe6cd6e5f0e29a8bf4ea3b5614))
	ROM_LOAD("usdnfs10.rom", 0x4000, 0x4000, CRC(7e367e8c) SHA1(161f585dc45665ea77433c84afd2f95049f7f5a0))

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("vm61002.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


ROM_START(dolphinm)
	// page ? 00000 IC51 OS/BASIC
	// page ? 04000 IC52
	// page ? 08000 IC53 DFS
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("hope_os_basic.rom", 0x3c000, 0x8000, CRC(8b30f9e5) SHA1(0a07161fa2fbc102fb306233bd81790c7b04f793))
	ROM_LOAD("hope_dfs.rom",      0x0c000, 0x4000, CRC(5d404973) SHA1(4c9f7cf141ad028fe5e517c45e6e3a43b1a76d24))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
ROM_END


ROM_START(torchf)
	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os12.rom", 0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d))

	// page 0 00000 IC52  BASIC
	// page 1 04000 IC88  DNFS
	// page 2 08000 IC100 CPN (inserted by bbc_tube_device)
	// page 3 0c000 IC101
	ROM_REGION(0x40000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("basic2.rom",         0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("dnfs120-201666.rom", 0x4000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("vm61002.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


ROM_START(torchh)
	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os12.rom", 0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d))

	// page 0 00000 IC52  BASIC
	// page 1 04000 IC88  DNFS
	// page 2 08000 IC100 CPN (inserted by bbc_tube_device)
	// page 3 0c000 IC101
	ROM_REGION(0x40000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("basic2.rom",         0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("dnfs120-201666.rom", 0x4000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("vm61002.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))

	DISK_REGION("1mhzbus:torchhd:sasi:0:s1410")
	DISK_IMAGE("torch_utilities", 0, BAD_DUMP SHA1(33a5f169bd91b9c6049e8bd0b237429c091fddd0)) // NEC D5126 contains Standard and Hard Disc Utilities, not known what was factory installed
ROM_END


ROM_START(torch301)
	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os12.rom", 0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d))

	// page 0 00000 IC52  BASIC
	// page 1 04000 IC88  ECO 3.35K
	// page 2 08000 IC100
	// page 3 0c000 IC101 MCP 1.01
	ROM_REGION(0x40000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("basic2.rom",      0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("eco_3.35k.rom",   0x4000, 0x4000, CRC(3ca2faea) SHA1(7462ced7b83d74b822815bc00ed40a89f84e0276))
	ROM_LOAD("mcp_1.01_ci.rom", 0xc000, 0x4000, CRC(436e7fe9) SHA1(be10872aeb88714bd56462a2e86929953dee1c01))

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("vm61002.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


ROM_START(torch725)
	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os12.rom", 0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d))

	// page 0 00000 IC52  BASIC
	// page 1 04000 IC88  ECO 3.35K
	// page 2 08000 IC100 Unix Host
	// page 3 0c000 IC101 MCP 1.22
	ROM_REGION(0x40000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("basic2.rom",      0x0000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("eco_3.35k.rom",   0x4000, 0x4000, CRC(3ca2faea) SHA1(7462ced7b83d74b822815bc00ed40a89f84e0276))
	ROM_LOAD("unix_1.00.rom",   0x8000, 0x4000, CRC(90f85ce9) SHA1(e37d043c8df30c49ba8717e1aa0b92105cb0c937))
	ROM_LOAD("mcp_1.22_ci.rom", 0xc000, 0x4000, CRC(764f4948) SHA1(409762deafb76b1f86be39bfbf2f812d5de3ff92))

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("vm61002.bin", 0x0000, 0x4000, CRC(bf4b3b64) SHA1(66876702d1d95eecc034d20f25047f893a27cde5))
ROM_END


ROM_START(sist1)
	ROM_REGION(0x4000, "mos", 0)
	ROM_LOAD("os12.rom", 0x0000, 0x4000, CRC(3c14fc70) SHA1(0d9bcaf6a393c9ce2359ed700ddb53c232c2c45d))

	// page 0 00000 IC52  DFS
	// page 1 04000 IC88  PROGS
	// page 2 08000 IC100 BASIC
	// page 3 0c000 IC101 STARTUP
	ROM_REGION(0x40000, "rom", ROMREGION_ERASEFF)
	ROM_LOAD("dnfs120-201666.rom", 0x0000, 0x4000, CRC(8ccd2157) SHA1(7e3c536baeae84d6498a14e8405319e01ee78232))
	ROM_LOAD("sist1_progs.bin",    0x4000, 0x4000, CRC(aea21243) SHA1(4398ba29c871fa397654aa182c63ccdcad597625))
	ROM_LOAD("basic2.rom",         0x8000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281))
	ROM_LOAD("sist1_startup.bin",  0xc000, 0x4000, CRC(9cd1602c) SHA1(5ea266f47ff83821ccdbec006b8506b2e892b115))

	ROM_REGION(0x40000, "vsm", ROMREGION_ERASE00)
	ROM_LOAD("cm62024.bin", 0x3c000, 0x4000, CRC(98e1bf9e) SHA1(b369809275cb67dfd8a749265e91adb2d2558ae6))
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT  COMPAT MACHINE     INPUT     CLASS         INIT       COMPANY                FULLNAME                          FLAGS
COMP( 1981, bbcb,       0,      bbca,  bbcb,       bbcb,     bbcb_state,   init_bbc,  "Acorn Computers",     "BBC Micro Model B",              MACHINE_IMPERFECT_GRAPHICS )
COMP( 1981, bbca,       bbcb,   0,     bbca,       bbca,     bbcb_state,   init_bbc,  "Acorn Computers",     "BBC Micro Model A",              MACHINE_IMPERFECT_GRAPHICS )
COMP( 1982, bbcb_de,    bbcb,   0,     bbcb_de,    bbcb,     bbcb_state,   init_bbc,  "Acorn Computers",     "BBC Micro Model B (German)",     MACHINE_IMPERFECT_GRAPHICS )
COMP( 1984, bbcb_no,    bbcb,   0,     bbcb_no,    bbcb,     bbcb_state,   init_bbc,  "Acorn Computers",     "BBC Micro Model B (Norway)",     MACHINE_IMPERFECT_GRAPHICS )
COMP( 1983, bbcb_us,    bbcb,   0,     bbcb_us,    bbcb,     bbcb_state,   init_bbc,  "Acorn Computers",     "BBC Micro Model B (US)",         MACHINE_IMPERFECT_GRAPHICS )
COMP( 1989, dolphinm,   bbcb,   0,     dolphinm,   bbca,     bbcb_state,   init_bbc,  "Hope Computers",      "Dolphin Microcomputer",          MACHINE_IMPERFECT_GRAPHICS )

// Torch Computers
COMP( 1982, torchf,     bbcb,   0,     torchf,     torch,    bbcb_state,   init_bbc,  "Torch Computers",     "Torch CF240",                    MACHINE_IMPERFECT_GRAPHICS )
COMP( 1983, torchh,     bbcb,   0,     torchh,     torch,    bbcb_state,   init_bbc,  "Torch Computers",     "Torch CH240",                    MACHINE_IMPERFECT_GRAPHICS )
COMP( 1983, torch301,   bbcb,   0,     torch301,   torch,    bbcb_state,   init_bbc,  "Torch Computers",     "Torch Model 301",                MACHINE_NOT_WORKING )
COMP( 1983, torch725,   bbcb,   0,     torch725,   torch,    bbcb_state,   init_bbc,  "Torch Computers",     "Torch Model 725",                MACHINE_NOT_WORKING )

// Industrial
COMP( 198?, sist1,      bbcb,   0,     sist1,      bbcb,     bbcb_state,   init_bbc,  "Cisco Systems",       "Cisco SIST1 Terminal",           MACHINE_NOT_WORKING )
