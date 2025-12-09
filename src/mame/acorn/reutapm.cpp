// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Reuters APM Board (Application Processor Module)

    https://chrisacorns.computinghistory.org.uk/Computers/Reuters.html

******************************************************************************/

#include "emu.h"
#include "bbc.h"
#include "acorn_serproc.h"

#include "machine/keyboard.h"


namespace {

class reuters_state : public bbc_state
{
public:
	reuters_state(const machine_config &mconfig, device_type type, const char *tag)
		: bbc_state(mconfig, type, tag)
		, m_view_shadow(*this, "view_shadow")
		, m_adlc(*this, "mc6854")
		, m_statid(*this, "STATID")
	{ }

	void reutapm(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	memory_view m_view_shadow;
	optional_device<mc6854_device> m_adlc;
	required_ioport m_statid;

	void reutapm_fetch(address_map &map) ATTR_COLD;
	void reutapm_mem(address_map &map) ATTR_COLD;

	uint8_t fetch_r(offs_t offset);
	void romsel_w(offs_t offset, uint8_t data);
	uint8_t paged_r(offs_t offset);
	void paged_w(offs_t offset, uint8_t data);

	void kbd_put(uint8_t data);

	uint8_t m_keydata;
};


void reuters_state::machine_start()
{
	bbc_state::machine_start();
}


void reuters_state::reutapm_fetch(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(reuters_state::fetch_r));
}


void reuters_state::reutapm_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));                                   //    0000-7FFF                 Regular RAM
	map(0x3000, 0x7fff).view(m_view_shadow);                                                                          //    3000-7FFF                 20K Shadow RAM
	m_view_shadow[0](0x3000, 0x7fff).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset + 0xb000]; }));
	m_view_shadow[0](0x3000, 0x7fff).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0xb000] = data; }));
	map(0x8000, 0xbfff).rw(FUNC(reuters_state::paged_r), FUNC(reuters_state::paged_w));                               //    8000-BFFF                 Paged ROM/RAM
	map(0xc000, 0xffff).rw(FUNC(reuters_state::mos_r), FUNC(reuters_state::mos_w));                                   //    C000-FBFF                 OS ROM
	map(0xfc00, 0xfcff).rw("1mhzbus", FUNC(bbc_1mhzbus_slot_device::fred_r), FUNC(bbc_1mhzbus_slot_device::fred_w));  //    FC00-FCFF                 FRED Address Page
	map(0xfd00, 0xfdff).rw("1mhzbus", FUNC(bbc_1mhzbus_slot_device::jim_r), FUNC(bbc_1mhzbus_slot_device::jim_w));    //    FD00-FDFF                 JIM Address Page
	map(0xfe00, 0xfeff).lr8(NAME([]() { return 0xfe; })).nopw();                                                      //    FE00-FEFF                 SHEILA Address Page
	map(0xfe00, 0xfe00).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));     //    FE00-FE07  6845 CRTC      Video controller
	map(0xfe01, 0xfe01).mirror(0x06).rw(m_crtc, FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0xfe08, 0xfe09).rw("acia1", FUNC(acia6850_device::read), FUNC(acia6850_device::write));                       //    FE08-FE09  6850 ACIA      Serial controller
	map(0xfe0a, 0xfe0b).rw("acia2", FUNC(acia6850_device::read), FUNC(acia6850_device::write));                       //    FE0A-FE0B  6850 ACIA      Serial controller
	map(0xfe10, 0xfe11).rw("serproc1", FUNC(acorn_serproc_device::read), FUNC(acorn_serproc_device::write));          //    FE10-FE11  Serial ULA     Serial system chip
	map(0xfe12, 0xfe13).rw("serproc2", FUNC(acorn_serproc_device::read), FUNC(acorn_serproc_device::write));          //    FE12-FE13  Serial ULA     Serial system chip
	map(0xfe18, 0xfe1f).lr8(NAME([this]() { econet_int_enable(0); return m_statid->read(); }));                       // R: FE18-FE1F  INTOFF/STATID  ECONET Interrupt Off / ID No.
	map(0xfe18, 0xfe1f).lw8(NAME([this](uint8_t data) { econet_int_enable(0); }));                                    // W: FE18-FE1F  INTOFF         ECONET Interrupt Off
	map(0xfe20, 0xfe2f).lr8(NAME([this]() { econet_int_enable(1); return 0xfe; }));                                   // R: FE20-FE2F  INTON          ECONET Interrupt On
	map(0xfe20, 0xfe2f).w(FUNC(reuters_state::video_ula_w));                                                          // W: FE20-FE2F  Video ULA      Video system chip
	map(0xfe30, 0xfe3f).w(FUNC(reuters_state::romsel_w));                                                             // W: FE30-FE3F  84LS161        Paged ROM selector
	map(0xfe30, 0xfe30).portr("SW2");
	map(0xfe32, 0xfe32).portr("SW1");
	map(0xfe40, 0xfe4f).mirror(0x10).m(m_sysvia, FUNC(via6522_device::map));                                          //    FE40-FE5F  6522 VIA       SYSTEM VIA
	map(0xfe60, 0xfe6f).mirror(0x10).m(m_uservia, FUNC(via6522_device::map));                                         //    FE60-FE7F  6522 VIA       USER VIA
	map(0xfea0, 0xfea3).mirror(0x1c).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));               //    FEA0-FEBF  68B54 ADLC     ECONET controller
	map(0xfee0, 0xfeff).rw("tube", FUNC(bbc_tube_slot_device::host_r), FUNC(bbc_tube_slot_device::host_w));           //    FEE0-FEFF  Tube ULA       Tube system interface
}


uint8_t reuters_state::fetch_r(offs_t offset)
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


void reuters_state::romsel_w(offs_t offset, uint8_t data)
{
	// the B+ addresses all 16 ROM sockets and extra 12K of RAM at 0x8000 and 20K of shadow RAM at 0x3000
	switch (offset & 0x07)
	{
	case 0x00:
		m_paged_ram = BIT(data, 7);

		m_romsel = data & 0x0f;
		break;

	case 0x04:
		// the video display should now use this flag to display the shadow RAM memory
		m_vdusel = BIT(data, 7);
		setvideoshadow(m_vdusel);
		break;
	}
}


uint8_t reuters_state::paged_r(offs_t offset)
{
	uint8_t data;

	if (m_paged_ram && ((m_romsel & 0x0e) == 0x02) && offset < 0x3000)
		data = m_ram->pointer()[offset + 0x8000];
	else
		data = m_region_rom->base()[offset + (m_romsel << 14)];

	return data;
}

void reuters_state::paged_w(offs_t offset, uint8_t data)
{
	if (m_paged_ram && ((m_romsel & 0x0e) == 0x02) && offset < 0x3000)
		m_ram->pointer()[offset + 0x8000] = data;
}


static INPUT_PORTS_START(reutapm)
	PORT_START("SW2")
	//PORT_DIPNAME(0x01, 0x00, "Unknown") PORT_DIPLOCATION("SW2:1")
	//PORT_DIPNAME(0x02, 0x00, "Unknown") PORT_DIPLOCATION("SW2:2")
	//PORT_DIPNAME(0x04, 0x00, "Unknown") PORT_DIPLOCATION("SW2:3")
	PORT_DIPNAME(0x08, 0x00, "Video Out") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(   0x00, "Colour")
	PORT_DIPSETTING(   0x08, "Mono")
	PORT_DIPNAME(0x30, 0x00, "S2 Input") PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(   0x00, "9600/3600 Hz")
	PORT_DIPSETTING(   0x10, "4800/1800 Hz")
	PORT_DIPSETTING(   0x20, "1200/450 Hz")
	PORT_DIPSETTING(   0x30, "300/112 Hz")
	PORT_DIPNAME(0x40, 0x00, "S1 Input") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(   0x00, "1200 Hz")
	PORT_DIPSETTING(   0x40, "300 Hz")
	PORT_DIPNAME(0x80, 0x00, "Printer") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(   0x00, "Parallel")
	PORT_DIPSETTING(   0x80, "S1 Serial")

	PORT_START("SW1")
	PORT_DIPNAME(0x03, 0x01, "Selection Char") PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(   0x00, "00 hex")
	PORT_DIPSETTING(   0x01, "E5 hex")
	PORT_DIPSETTING(   0x02, "E3 hex")
	PORT_DIPSETTING(   0x03, "8E hex")
	PORT_DIPNAME(0x04, 0x04, "Video Switching") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(   0x00, "Enabled")
	PORT_DIPSETTING(   0x04, "Disabled")
	PORT_DIPNAME(0x08, 0x00, "Switch Chars") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(   0x00, "Output")
	PORT_DIPSETTING(   0x10, "Suppressed")
	PORT_DIPNAME(0x10, 0x10, "KBD In Strobe") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(   0x00, "+ve")
	PORT_DIPSETTING(   0x10, "-ve")
	PORT_DIPNAME(0x20, 0x00, "KBD Out Strobe") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(   0x00, "+ve")
	PORT_DIPSETTING(   0x20, "-ve")
	PORT_DIPNAME(0x40, 0x00, "Video In") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(   0x00, "Colour")
	PORT_DIPSETTING(   0x40, "Mono")
	PORT_DIPNAME(0x80, 0x80, "Video Frequency") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(   0x00, "60 Hz")
	PORT_DIPSETTING(   0x80, "50 Hz")

	PORT_START("STATID")
	PORT_CONFNAME(0xff, 0xfe, "Econet Station ID")
	PORT_CONFSETTING( 0x00,   "0" )    PORT_CONFSETTING( 0x01,   "1" )    PORT_CONFSETTING( 0x02,   "2" )    PORT_CONFSETTING( 0x03,   "3" )    PORT_CONFSETTING( 0x04,   "4" )
	PORT_CONFSETTING( 0x05,   "5" )    PORT_CONFSETTING( 0x06,   "6" )    PORT_CONFSETTING( 0x07,   "7" )    PORT_CONFSETTING( 0x08,   "8" )    PORT_CONFSETTING( 0x09,   "9" )
	PORT_CONFSETTING( 0x0a,  "10" )    PORT_CONFSETTING( 0x0b,  "11" )    PORT_CONFSETTING( 0x0c,  "12" )    PORT_CONFSETTING( 0x0d,  "13" )    PORT_CONFSETTING( 0x0e,  "14" )
	PORT_CONFSETTING( 0x0f,  "15" )    PORT_CONFSETTING( 0x10,  "16" )    PORT_CONFSETTING( 0x11,  "17" )    PORT_CONFSETTING( 0x12,  "18" )    PORT_CONFSETTING( 0x13,  "19" )
	PORT_CONFSETTING( 0x14,  "20" )    PORT_CONFSETTING( 0x15,  "21" )    PORT_CONFSETTING( 0x16,  "22" )    PORT_CONFSETTING( 0x17,  "23" )    PORT_CONFSETTING( 0x18,  "24" )
	PORT_CONFSETTING( 0x19,  "25" )    PORT_CONFSETTING( 0x1a,  "26" )    PORT_CONFSETTING( 0x1b,  "27" )    PORT_CONFSETTING( 0x1c,  "28" )    PORT_CONFSETTING( 0x1d,  "29" )
	PORT_CONFSETTING( 0x1e,  "30" )    PORT_CONFSETTING( 0x1f,  "31" )    PORT_CONFSETTING( 0x20,  "32" )    PORT_CONFSETTING( 0x21,  "33" )    PORT_CONFSETTING( 0x22,  "34" )
	PORT_CONFSETTING( 0x23,  "35" )    PORT_CONFSETTING( 0x24,  "36" )    PORT_CONFSETTING( 0x25,  "37" )    PORT_CONFSETTING( 0x26,  "38" )    PORT_CONFSETTING( 0x27,  "39" )
	PORT_CONFSETTING( 0x28,  "40" )    PORT_CONFSETTING( 0x29,  "41" )    PORT_CONFSETTING( 0x2a,  "42" )    PORT_CONFSETTING( 0x2b,  "43" )    PORT_CONFSETTING( 0x2c,  "44" )
	PORT_CONFSETTING( 0x2d,  "45" )    PORT_CONFSETTING( 0x2e,  "46" )    PORT_CONFSETTING( 0x2f,  "47" )    PORT_CONFSETTING( 0x30,  "48" )    PORT_CONFSETTING( 0x31,  "49" )
	PORT_CONFSETTING( 0x32,  "50" )    PORT_CONFSETTING( 0x33,  "51" )    PORT_CONFSETTING( 0x34,  "52" )    PORT_CONFSETTING( 0x35,  "53" )    PORT_CONFSETTING( 0x36,  "54" )
	PORT_CONFSETTING( 0x37,  "55" )    PORT_CONFSETTING( 0x38,  "56" )    PORT_CONFSETTING( 0x39,  "57" )    PORT_CONFSETTING( 0x3a,  "58" )    PORT_CONFSETTING( 0x3b,  "59" )
	PORT_CONFSETTING( 0x3c,  "60" )    PORT_CONFSETTING( 0x3d,  "61" )    PORT_CONFSETTING( 0x3e,  "62" )    PORT_CONFSETTING( 0x3f,  "63" )    PORT_CONFSETTING( 0x40,  "64" )
	PORT_CONFSETTING( 0x41,  "65" )    PORT_CONFSETTING( 0x42,  "66" )    PORT_CONFSETTING( 0x43,  "67" )    PORT_CONFSETTING( 0x44,  "68" )    PORT_CONFSETTING( 0x45,  "69" )
	PORT_CONFSETTING( 0x46,  "70" )    PORT_CONFSETTING( 0x47,  "71" )    PORT_CONFSETTING( 0x48,  "72" )    PORT_CONFSETTING( 0x49,  "73" )    PORT_CONFSETTING( 0x4a,  "74" )
	PORT_CONFSETTING( 0x4b,  "75" )    PORT_CONFSETTING( 0x4c,  "76" )    PORT_CONFSETTING( 0x4d,  "77" )    PORT_CONFSETTING( 0x4e,  "78" )    PORT_CONFSETTING( 0x4f,  "79" )
	PORT_CONFSETTING( 0x50,  "80" )    PORT_CONFSETTING( 0x51,  "81" )    PORT_CONFSETTING( 0x52,  "82" )    PORT_CONFSETTING( 0x53,  "83" )    PORT_CONFSETTING( 0x54,  "84" )
	PORT_CONFSETTING( 0x55,  "85" )    PORT_CONFSETTING( 0x56,  "86" )    PORT_CONFSETTING( 0x57,  "87" )    PORT_CONFSETTING( 0x58,  "88" )    PORT_CONFSETTING( 0x59,  "89" )
	PORT_CONFSETTING( 0x5a,  "90" )    PORT_CONFSETTING( 0x5b,  "91" )    PORT_CONFSETTING( 0x5c,  "92" )    PORT_CONFSETTING( 0x5d,  "93" )    PORT_CONFSETTING( 0x5e,  "94" )
	PORT_CONFSETTING( 0x5f,  "95" )    PORT_CONFSETTING( 0x60,  "96" )    PORT_CONFSETTING( 0x61,  "97" )    PORT_CONFSETTING( 0x62,  "98" )    PORT_CONFSETTING( 0x63,  "99" )
	PORT_CONFSETTING( 0x64, "100" )    PORT_CONFSETTING( 0x65, "101" )    PORT_CONFSETTING( 0x66, "102" )    PORT_CONFSETTING( 0x67, "103" )    PORT_CONFSETTING( 0x68, "104" )
	PORT_CONFSETTING( 0x69, "105" )    PORT_CONFSETTING( 0x6a, "106" )    PORT_CONFSETTING( 0x6b, "107" )    PORT_CONFSETTING( 0x6c, "108" )    PORT_CONFSETTING( 0x6d, "109" )
	PORT_CONFSETTING( 0x6e, "110" )    PORT_CONFSETTING( 0x6f, "111" )    PORT_CONFSETTING( 0x70, "112" )    PORT_CONFSETTING( 0x71, "113" )    PORT_CONFSETTING( 0x72, "114" )
	PORT_CONFSETTING( 0x73, "115" )    PORT_CONFSETTING( 0x74, "116" )    PORT_CONFSETTING( 0x75, "117" )    PORT_CONFSETTING( 0x76, "118" )    PORT_CONFSETTING( 0x77, "119" )
	PORT_CONFSETTING( 0x78, "120" )    PORT_CONFSETTING( 0x79, "121" )    PORT_CONFSETTING( 0x7a, "122" )    PORT_CONFSETTING( 0x7b, "123" )    PORT_CONFSETTING( 0x7c, "124" )
	PORT_CONFSETTING( 0x7d, "125" )    PORT_CONFSETTING( 0x7e, "126" )    PORT_CONFSETTING( 0x7f, "127" )    PORT_CONFSETTING( 0x80, "128" )    PORT_CONFSETTING( 0x81, "129" )
	PORT_CONFSETTING( 0x82, "130" )    PORT_CONFSETTING( 0x83, "131" )    PORT_CONFSETTING( 0x84, "132" )    PORT_CONFSETTING( 0x85, "133" )    PORT_CONFSETTING( 0x86, "134" )
	PORT_CONFSETTING( 0x87, "135" )    PORT_CONFSETTING( 0x88, "136" )    PORT_CONFSETTING( 0x89, "137" )    PORT_CONFSETTING( 0x8a, "138" )    PORT_CONFSETTING( 0x8b, "139" )
	PORT_CONFSETTING( 0x8c, "140" )    PORT_CONFSETTING( 0x8d, "141" )    PORT_CONFSETTING( 0x8e, "142" )    PORT_CONFSETTING( 0x8f, "143" )    PORT_CONFSETTING( 0x90, "144" )
	PORT_CONFSETTING( 0x91, "145" )    PORT_CONFSETTING( 0x92, "146" )    PORT_CONFSETTING( 0x93, "147" )    PORT_CONFSETTING( 0x94, "148" )    PORT_CONFSETTING( 0x95, "149" )
	PORT_CONFSETTING( 0x96, "150" )    PORT_CONFSETTING( 0x97, "151" )    PORT_CONFSETTING( 0x98, "152" )    PORT_CONFSETTING( 0x99, "153" )    PORT_CONFSETTING( 0x9a, "154" )
	PORT_CONFSETTING( 0x9b, "155" )    PORT_CONFSETTING( 0x9c, "156" )    PORT_CONFSETTING( 0x9d, "157" )    PORT_CONFSETTING( 0x9e, "158" )    PORT_CONFSETTING( 0x9f, "159" )
	PORT_CONFSETTING( 0xa0, "160" )    PORT_CONFSETTING( 0xa1, "161" )    PORT_CONFSETTING( 0xa2, "162" )    PORT_CONFSETTING( 0xa3, "163" )    PORT_CONFSETTING( 0xa4, "164" )
	PORT_CONFSETTING( 0xa5, "165" )    PORT_CONFSETTING( 0xa6, "166" )    PORT_CONFSETTING( 0xa7, "167" )    PORT_CONFSETTING( 0xa8, "168" )    PORT_CONFSETTING( 0xa9, "169" )
	PORT_CONFSETTING( 0xaa, "170" )    PORT_CONFSETTING( 0xab, "171" )    PORT_CONFSETTING( 0xac, "172" )    PORT_CONFSETTING( 0xad, "173" )    PORT_CONFSETTING( 0xae, "174" )
	PORT_CONFSETTING( 0xaf, "175" )    PORT_CONFSETTING( 0xb0, "176" )    PORT_CONFSETTING( 0xb1, "177" )    PORT_CONFSETTING( 0xb2, "178" )    PORT_CONFSETTING( 0xb3, "179" )
	PORT_CONFSETTING( 0xb4, "180" )    PORT_CONFSETTING( 0xb5, "181" )    PORT_CONFSETTING( 0xb6, "182" )    PORT_CONFSETTING( 0xb7, "183" )    PORT_CONFSETTING( 0xb8, "184" )
	PORT_CONFSETTING( 0xb9, "185" )    PORT_CONFSETTING( 0xba, "186" )    PORT_CONFSETTING( 0xbb, "187" )    PORT_CONFSETTING( 0xbc, "188" )    PORT_CONFSETTING( 0xbd, "189" )
	PORT_CONFSETTING( 0xbe, "190" )    PORT_CONFSETTING( 0xbf, "191" )    PORT_CONFSETTING( 0xc0, "192" )    PORT_CONFSETTING( 0xc1, "193" )    PORT_CONFSETTING( 0xc2, "194" )
	PORT_CONFSETTING( 0xc3, "195" )    PORT_CONFSETTING( 0xc4, "196" )    PORT_CONFSETTING( 0xc5, "197" )    PORT_CONFSETTING( 0xc6, "198" )    PORT_CONFSETTING( 0xc7, "199" )
	PORT_CONFSETTING( 0xc8, "200" )    PORT_CONFSETTING( 0xc9, "201" )    PORT_CONFSETTING( 0xca, "202" )    PORT_CONFSETTING( 0xcb, "203" )    PORT_CONFSETTING( 0xcc, "204" )
	PORT_CONFSETTING( 0xcd, "205" )    PORT_CONFSETTING( 0xce, "206" )    PORT_CONFSETTING( 0xcf, "207" )    PORT_CONFSETTING( 0xd0, "208" )    PORT_CONFSETTING( 0xd1, "209" )
	PORT_CONFSETTING( 0xd2, "210" )    PORT_CONFSETTING( 0xd3, "211" )    PORT_CONFSETTING( 0xd4, "212" )    PORT_CONFSETTING( 0xd5, "213" )    PORT_CONFSETTING( 0xd6, "214" )
	PORT_CONFSETTING( 0xd7, "215" )    PORT_CONFSETTING( 0xd8, "216" )    PORT_CONFSETTING( 0xd9, "217" )    PORT_CONFSETTING( 0xda, "218" )    PORT_CONFSETTING( 0xdb, "219" )
	PORT_CONFSETTING( 0xdc, "220" )    PORT_CONFSETTING( 0xdd, "221" )    PORT_CONFSETTING( 0xde, "222" )    PORT_CONFSETTING( 0xdf, "223" )    PORT_CONFSETTING( 0xe0, "224" )
	PORT_CONFSETTING( 0xe1, "225" )    PORT_CONFSETTING( 0xe2, "226" )    PORT_CONFSETTING( 0xe3, "227" )    PORT_CONFSETTING( 0xe4, "228" )    PORT_CONFSETTING( 0xe5, "229" )
	PORT_CONFSETTING( 0xe6, "230" )    PORT_CONFSETTING( 0xe7, "231" )    PORT_CONFSETTING( 0xe8, "232" )    PORT_CONFSETTING( 0xe9, "233" )    PORT_CONFSETTING( 0xea, "234" )
	PORT_CONFSETTING( 0xeb, "235" )    PORT_CONFSETTING( 0xec, "236" )    PORT_CONFSETTING( 0xed, "237" )    PORT_CONFSETTING( 0xee, "238" )    PORT_CONFSETTING( 0xef, "239" )
	PORT_CONFSETTING( 0xf0, "240" )    PORT_CONFSETTING( 0xf1, "241" )    PORT_CONFSETTING( 0xf2, "242" )    PORT_CONFSETTING( 0xf3, "243" )    PORT_CONFSETTING( 0xf4, "244" )
	PORT_CONFSETTING( 0xf5, "245" )    PORT_CONFSETTING( 0xf6, "246" )    PORT_CONFSETTING( 0xf7, "247" )    PORT_CONFSETTING( 0xf8, "248" )    PORT_CONFSETTING( 0xf9, "249" )
	PORT_CONFSETTING( 0xfa, "250" )    PORT_CONFSETTING( 0xfb, "251" )    PORT_CONFSETTING( 0xfc, "252" )    PORT_CONFSETTING( 0xfd, "253" )    PORT_CONFSETTING( 0xfe, "254" )
	PORT_CONFSETTING( 0xff, "255" )
INPUT_PORTS_END


void reuters_state::kbd_put(uint8_t data)
{
	m_keydata = data;

	m_uservia->write_cb1(0);
	m_uservia->write_cb1(1);
}


void reuters_state::reutapm(machine_config &config)
{
	M6512(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &reuters_state::reutapm_mem);
	m_maincpu->set_addrmap(AS_OPCODES, &reuters_state::reutapm_fetch);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

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
	m_crtc->set_reconfigure_callback(FUNC(reuters_state::crtc_reconfigure));
	m_crtc->set_update_row_callback(FUNC(reuters_state::crtc_update_row));
	m_crtc->out_de_callback().set(FUNC(reuters_state::de_changed));
	m_crtc->out_hsync_callback().set(FUNC(reuters_state::hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(reuters_state::vsync_changed));

	LS259(config, m_latch);
	m_latch->q_out_cb<6>().set_output("capslock_led");
	m_latch->q_out_cb<7>().set_output("shiftlock_led");

	MOS6522(config, m_sysvia, 16_MHz_XTAL / 16);
	//m_sysvia->writepa_handler().set(FUNC(reuters_state::sysvia_pa_w)); // keyboard out
	m_sysvia->writepb_handler().set(m_latch, FUNC(ls259_device::write_nibble_d3));
	//m_sysvia->writepb_handler().set(FUNC(reuters_state::sysvia_pb_w)); // alarm AL1/AL2 on PB5/PB4
	//m_sysvia->ca2_handler().set(FUNC(reuters_state::kbd_strobe)); // keyboard strobe out
	m_sysvia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	MOS6522(config, m_uservia, 16_MHz_XTAL / 16);
	m_uservia->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_uservia->readpb_handler().set([this]() { return m_keydata; });
	m_uservia->ca2_handler().set("printer", FUNC(centronics_device::write_strobe));
	m_uservia->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(reuters_state::kbd_put));

	centronics_device &centronics(CENTRONICS(config, "printer", centronics_devices, "printer"));
	centronics.ack_handler().set(m_uservia, FUNC(via6522_device::write_ca1));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(latch);

	acia6850_device &acia1(ACIA6850(config, "acia1", 0));
	acia1.txd_handler().set("rs423", FUNC(rs232_port_device::write_txd));
	acia1.rts_handler().set("rs423", FUNC(rs232_port_device::write_rts));
	acia1.rts_handler().append("serproc1", FUNC(acorn_serproc_device::write_rtsi));
	acia1.irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));

	acorn_serproc_device &serproc1(ACORN_SERPROC(config, "serproc1", 16_MHz_XTAL / 13));
	serproc1.rxc_handler().set("acia1", FUNC(acia6850_device::write_rxc));
	serproc1.txc_handler().set("acia1", FUNC(acia6850_device::write_txc));
	serproc1.rxd_handler().set("acia1", FUNC(acia6850_device::write_rxd));
	serproc1.dcd_handler().set("acia1", FUNC(acia6850_device::write_dcd));
	serproc1.ctso_handler().set("acia1", FUNC(acia6850_device::write_cts));

	rs232_port_device &rs423(RS232_PORT(config, "rs423", default_rs232_devices, nullptr));
	rs423.rxd_handler().set("serproc1", FUNC(acorn_serproc_device::write_din));
	rs423.cts_handler().set("serproc1", FUNC(acorn_serproc_device::write_ctsi));

	acia6850_device &acia2(ACIA6850(config, "acia2"));
	acia2.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia2.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	acia2.rts_handler().append("serproc2", FUNC(acorn_serproc_device::write_rtsi));
	acia2.irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));

	acorn_serproc_device &serproc2(ACORN_SERPROC(config, "serproc2", 16_MHz_XTAL / 13));
	serproc2.rxc_handler().set("acia2", FUNC(acia6850_device::write_rxc));
	serproc2.txc_handler().set("acia2", FUNC(acia6850_device::write_txc));
	serproc2.rxd_handler().set("acia2", FUNC(acia6850_device::write_rxd));
	serproc2.ctso_handler().set("acia2", FUNC(acia6850_device::write_cts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("serproc2", FUNC(acorn_serproc_device::write_din));
	rs232.cts_handler().set("serproc2", FUNC(acorn_serproc_device::write_ctsi));
	rs232.dcd_handler().set("acia2", FUNC(acia6850_device::write_dcd));
	rs232.dsr_handler().set("sysvia", FUNC(via6522_device::write_cb2));
	rs232.dsr_handler().append("sysvia", FUNC(via6522_device::write_pb6));

	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("network", FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(FUNC(reuters_state::adlc_irq_w));
	//m_adlc->out_rts_cb().

	econet_device &econet(ECONET(config, "network", 0));
	econet.clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	econet.data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));
	ECONET_SLOT(config, "econet", "network", econet_devices);

	bbc_1mhzbus_slot_device &bus(BBC_1MHZBUS_SLOT(config, m_1mhzbus, 16_MHz_XTAL / 16, bbc_1mhzbus_devices, nullptr)); // TODO: Reuters Solid State Store
	bus.irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<4>));
	bus.nmi_handler().set(FUNC(reuters_state::bus_nmi_w));

	bbc_tube_slot_device &tube(BBC_TUBE_SLOT(config, m_tube, bbc_tube_devices, nullptr));
	tube.irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<5>));
}


ROM_START(reutapm)
	ROM_REGION(0x44000, "rom", ROMREGION_ERASEFF)
	// page 0  00000  IC71 DNFS                      // page 8  20000  IC33
	// page 1  04000  IC71                           // page 9  24000  IC33
	// page 2  08000                                 // page 10 28000  IC44
	// page 3  0c000                                 // page 11 2c000  IC44
	// page 4  10000  IC22                           // page 12 30000  IC50
	// page 5  14000  IC22                           // page 13 34000  IC50
	// page 6  18000  IC29                           // page 14 38000  IC57
	// page 7  1c000  IC29                           // page 15 3C000  IC57
	ROM_SYSTEM_BIOS(0, "hwtest", "HWTEST")
	ROMX_LOAD("ros_dnfs_0251720-02_v2.0.rom",      0x00000, 0x4000, CRC(1c4145d5) SHA1(9a8c1977b08399455a55f4b83f4b8f637cf2bc04), ROM_BIOS(0))
	ROM_CONTINUE(0x40000, 0x4000)
	ROMX_LOAD("environment_0251.721-01_v0.11.rom", 0x38000, 0x8000, CRC(5615f7e4) SHA1(d7ab8b67e01a05bae098941f877bfc604292395d), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "topic", "TOPIC")
	ROMX_LOAD("ros_dnfs_251720_v3.65.rom", 0x00000, 0x4000, CRC(1c4145d5) SHA1(9a8c1977b08399455a55f4b83f4b8f637cf2bc04), ROM_BIOS(1))
	ROM_CONTINUE(0x40000, 0x4000)
	ROMX_LOAD("topic_v1.1.rom",            0x30000, 0x4000, CRC(a3951386) SHA1(91eee39009e9148d1970b4707bb291d344f2a2b1), ROM_BIOS(1))
	ROMX_LOAD("environment_251721_v7.rom", 0x38000, 0x4000, CRC(0bfa2aa0) SHA1(9b5e9fea700715d03bd00c2362f57294161a7441), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "apm", "APM")
	ROMX_LOAD("r0.3_nfs3.65.rom",   0x00000, 0x4000, CRC(1c4145d5) SHA1(9a8c1977b08399455a55f4b83f4b8f637cf2bc04), ROM_BIOS(2))
	ROM_CONTINUE(0x40000, 0x4000)
	ROMX_LOAD("apm-f814-24oct.rom", 0x30000, 0x4000, CRC(5a59b894) SHA1(ae50aa7f13d63a6b582baf3a967257fb8c315684), ROM_BIOS(2))
	ROMX_LOAD("sysrom-v6-4bc8.rom", 0x38000, 0x4000, CRC(249ff54c) SHA1(e5d178d19b2c22852f576a90fae4109e22d324ac), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "lib", "Library")
	ROMX_LOAD("reuterb.rom",    0x30000, 0x4000, BAD_DUMP CRC(9e02f59b) SHA1(1e63aa3bf4b37bf9ba41e454f95db05c3d15bfbf), ROM_BIOS(3))
	ROMX_LOAD("reutera100.rom", 0x38000, 0x4000, CRC(98ebabfb) SHA1(a7887e1e5c206203491e1e06682b9508b0fef49d), ROM_BIOS(3))
	ROMX_LOAD("mos_r030.rom",   0x40000, 0x4000, CRC(8b652337) SHA1(6a5c7ace255c8ac96c983d5ba67084fbd71ff61e), ROM_BIOS(3))

	ROM_REGION(0x4000, "mos", 0)
	ROM_COPY("rom", 0x40000, 0, 0x4000)
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT  COMPAT MACHINE     INPUT      CLASS           INIT       COMPANY               FULLNAME          FLAGS
COMP( 1985, reutapm,    0,      0,     reutapm,    reutapm,   reuters_state,  init_bbc,  "Acorn Computers",    "Reuters APM",    MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
