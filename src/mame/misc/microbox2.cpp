// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Micro Concepts Microbox II

    Usage:
      Insert the FLEX disk in drive 0 and the Distribution disk in drive 1,
      then type BO to boot FLEX.
      Type DEMO.1 to run the demo from drive 1.

    TODO:
    - implement WD2123 device (dual channel 8251A)
    - improve UPD7220A to handle scrolling

***************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6809/m6809.h"
#include "formats/flex_dsk.h"
#include "imagedev/floppy.h"
#include "machine/6821pia.h"
#include "machine/6883sam.h"
#include "machine/i8255.h"
#include "machine/keyboard.h"
#include "machine/mc146818.h"
#include "machine/wd_fdc.h"
//#include "machine/wd2123.h"
#include "sound/beep.h"
#include "video/upd7220.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class microbx2_state : public driver_device
{
public:
	microbx2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_epromdisc(*this, "epromdisc")
		, m_ram(*this, "ram", 0x10000, ENDIANNESS_BIG)
		, m_map_view(*this, "map_view")
		, m_pia1(*this, "pia1")
		, m_video_ram(*this, "video_ram")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0)
		, m_beeper(*this, "beeper")
		, m_sw1(*this, "SW1")
	{}

	static constexpr feature_type unemulated_features() { return feature::COMMS; }

	void microbx2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<uint8_t> m_epromdisc;
	memory_share_creator<uint8_t> m_ram;
	memory_view m_map_view;
	required_device<pia6821_device> m_pia1;
	required_shared_ptr<uint16_t> m_video_ram;
	required_device<wd1770_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<beep_device> m_beeper;
	required_ioport m_sw1;

	void upd7220_map(address_map &map) ATTR_COLD;

	UPD7220_DISPLAY_PIXELS_MEMBER(display_pixels);

	void mem_map(address_map &map) ATTR_COLD;
	void ram_map(address_map &map) ATTR_COLD;
	void rom0_map(address_map &map) ATTR_COLD;
	void rom1_map(address_map &map) ATTR_COLD;
	void rom2_map(address_map &map) ATTR_COLD;
	void io0_map(address_map &map) ATTR_COLD;
	void io1_map(address_map &map) ATTR_COLD;
	void io2_map(address_map &map) ATTR_COLD;
	void boot_map(address_map &map) ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	void kbd_w(uint8_t data);
	void pia1_pb_w(uint8_t data);
	uint8_t pia2_pa_r();
	void pia2_pb_w(uint8_t data);
	void pia2_pc_w(uint8_t data);

	uint8_t m_keydata;
	uint16_t m_eprom_addr;
	int m_ls393_input;
};


void microbx2_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw("sam", FUNC(sam6883_device::read), FUNC(sam6883_device::write));
}

void microbx2_state::ram_map(address_map &map)
{
	// $0000-$FEFF
	map(0x0000, 0xffff).ram().share("ram");
	map(0xe000, 0xefff).view(m_map_view);
	m_map_view[0](0xe000, 0xefff).rom().region("maincpu", 0x0000);
	map(0xf000, 0xffff).rom().region("maincpu", 0x1000);
}

void microbx2_state::rom0_map(address_map &map)
{
	// $8000-$9FFF
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x0000, 0x1fff).lw8(NAME([this](offs_t offset, uint8_t data) { m_ram[offset + 0x8000] = data; }));
}

void microbx2_state::rom1_map(address_map &map)
{
	// $A000-$BFFF
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x0000, 0x1fff).lw8(NAME([this](offs_t offset, uint8_t data) { m_ram[offset + 0xa000] = data; }));
}

void microbx2_state::rom2_map(address_map &map)
{
	// $C000-$FEFF
	map(0x0000, 0x1fff).mirror(0x2000).rom().region("maincpu", 0);
	map(0x0000, 0x3eff).lw8(NAME([this](offs_t offset, uint8_t data) { m_ram[offset + 0xc000] = data; }));
}

void microbx2_state::io0_map(address_map &map)
{
	// $FF00-$FF1F
	map(0x00, 0x03).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	//map(0x04, 0x05).rw("deuce", FUNC(wd2123_device::read_b), FUNC(wd2123_device::write_b));
	//map(0x08, 0x09).rw("deuce", FUNC(wd2123_device::read_a), FUNC(wd2123_device::write_a));
	//map(0x0c, 0x0d).w("deuce", FUNC(wd2123_device::baud));
	map(0x10, 0x13).rw("fdc", FUNC(wd1770_device::read), FUNC(wd1770_device::write));
	map(0x14, 0x15).rw("gdc", FUNC(upd7220a_device::read), FUNC(upd7220a_device::write));
	map(0x18, 0x18).w("rtc", FUNC(mc146818_device::address_w));
	map(0x19, 0x19).rw("rtc", FUNC(mc146818_device::data_r), FUNC(mc146818_device::data_w));
	map(0x1c, 0x1f).rw("pia2", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void microbx2_state::io1_map(address_map &map)
{
	// $FF20-$FF3F
}

void microbx2_state::io2_map(address_map &map)
{
	// $FF40-$FF5F
}

void microbx2_state::boot_map(address_map &map)
{
	// $FF60-$FFEF
	map(0x60, 0x7f).nopw(); // SAM Registers
}

void microbx2_state::upd7220_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram().share("video_ram");
}


void microbx2_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_FLEX_FORMAT);
}


static INPUT_PORTS_START( microbx2 )
	PORT_START("SW1")
	PORT_DIPNAME(0x10, 0x10, "Initial Input") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(   0x00, "Serial Keyboard")
	PORT_DIPSETTING(   0x10, "Parallel Keyboard")
	PORT_DIPNAME(0x20, 0x20, "Initial Output") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(   0x00, "Serial Terminal")
	PORT_DIPSETTING(   0x20, "Video Monitor")
	PORT_DIPNAME(0x40, 0x40, "Step Rate") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(   0x00, "30ms")
	PORT_DIPSETTING(   0x40, "6ms")
	PORT_DIPNAME(0x80, 0x80, "Auto Boot") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(   0x00, "FLEX")
	PORT_DIPSETTING(   0x80, "Monitor")
INPUT_PORTS_END


UPD7220_DISPLAY_PIXELS_MEMBER(microbx2_state::display_pixels)
{
	uint16_t const gfx = m_video_ram[address & 0x3ffff];

	for (int i = 0; i < 16; i++)
	{
		bitmap.pix(y, x + i) = BIT(gfx, i) ? rgb_t::white() : rgb_t::black();
	}
}


void microbx2_state::machine_start()
{
	m_ls393_input = 1;

	save_item(NAME(m_eprom_addr));
}


void microbx2_state::kbd_w(uint8_t data)
{
	m_keydata = data;

	m_pia1->ca1_w(0);
	m_pia1->ca1_w(1);
}


void microbx2_state::pia1_pb_w(uint8_t data)
{
	// bit 0: drv
	floppy_image_device *floppy = m_floppy[BIT(data, 0)]->get_device();
	m_fdc->set_floppy(floppy);

	// bit 1: dden
	m_fdc->dden_w(BIT(data, 1));

	// bit 2: map
	if (BIT(data, 2))
		m_map_view.select(0);
	else
		m_map_view.disable();

	// bit 3: sounder
	m_beeper->set_state(BIT(data, 3));

	// bit 6: side
	if (floppy)
		floppy->ss_w(!BIT(data, 6));
}


uint8_t microbx2_state::pia2_pa_r()
{
	// D0-7
	return m_epromdisc[m_eprom_addr];
}

void microbx2_state::pia2_pb_w(uint8_t data)
{
	// A8-15
	m_eprom_addr = (m_eprom_addr & 0xc0ff) | ((data & 0x3f) << 8);
}


void microbx2_state::pia2_pc_w(uint8_t data)
{
	// CE lines (27128 EPROMs)
	m_eprom_addr &= 0x3fff;
	switch (~data & 0x0f)
	{
	case 0x01: m_eprom_addr |= 0x0000; break;
	case 0x02: m_eprom_addr |= 0x4000; break;
	case 0x04: m_eprom_addr |= 0x8000; break;
	case 0x08: m_eprom_addr |= 0xc000; break;
	}

	// LS393 input
	if (!BIT(data, 4) && m_ls393_input)
		m_eprom_addr++;

	m_ls393_input = BIT(data, 4);

	// LS393 clear
	if (BIT(data, 5))
		m_eprom_addr &= 0xff00;
}


static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD",   0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD",   0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_8)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY",   0xff, RS232_PARITY_NONE)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_1)
DEVICE_INPUT_DEFAULTS_END


void microbx2_state::microbx2(machine_config &config)
{
	MC6809E(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &microbx2_state::mem_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(16_MHz_XTAL, 1024, 0, 768, 674, 31, 607);
	screen.set_screen_update("gdc", FUNC(upd7220a_device::screen_update));

	upd7220a_device &gdc(UPD7220A(config, "gdc", 16_MHz_XTAL / 8));
	gdc.set_addrmap(0, &microbx2_state::upd7220_map);
	gdc.set_display_pixels(FUNC(microbx2_state::display_pixels));
	gdc.set_screen("screen");

	sam6883_device &sam(SAM6883(config, "sam", 16_MHz_XTAL, m_maincpu));
	sam.set_addrmap(0, &microbx2_state::ram_map);  // RAM at $0000
	sam.set_addrmap(1, &microbx2_state::rom0_map); // RAM at $8000
	sam.set_addrmap(2, &microbx2_state::rom1_map); // RAM at $A000
	sam.set_addrmap(3, &microbx2_state::rom2_map); // RAM at $C000
	sam.set_addrmap(4, &microbx2_state::io0_map);  // IO0 at $FF00
	sam.set_addrmap(5, &microbx2_state::io1_map);  // IO1 at $FF20
	sam.set_addrmap(6, &microbx2_state::io2_map);  // IO2 at $FF40
	sam.set_addrmap(7, &microbx2_state::boot_map); // BOOT at $FF60

	PIA6821(config, m_pia1);
	m_pia1->readpa_handler().set([this]() { return m_keydata; });
	m_pia1->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_pia1->readpb_handler().set_ioport("SW1");
	m_pia1->writepb_handler().set(FUNC(microbx2_state::pia1_pb_w));
	m_pia1->cb2_handler().set("centronics", FUNC(centronics_device::write_strobe));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(microbx2_state::kbd_w));

	centronics_device &centronics(CENTRONICS(config, "centronics", centronics_devices, "printer"));
	centronics.ack_handler().set("pia1", FUNC(pia6821_device::cb1_w));
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	centronics.set_output_latch(cent_data_out);

	i8255_device &pia2(I8255(config, "pia2"));
	pia2.in_pa_callback().set(FUNC(microbx2_state::pia2_pa_r));
	pia2.out_pb_callback().set(FUNC(microbx2_state::pia2_pb_w));
	pia2.out_pc_callback().set(FUNC(microbx2_state::pia2_pc_w));

	mc146818_device &rtc(MC146818(config, "rtc", 32.768_kHz_XTAL));
	rtc.set_binary(true);

	//wd2123_device &deuce(WD2123(config, "deuce", 1.8432_MHz_XTAL));
	//deuce.txd_a_handler().set("rs232a", FUNC(rs232_port_device::write_txd));
	//deuce.rts_a_handler().set("rs232a", FUNC(rs232_port_device::write_rts));
	//deuce.txd_b_handler().set("rs232b", FUNC(rs232_port_device::write_txd));
	//deuce.rts_b_handler().set("rs232b", FUNC(rs232_port_device::write_rts));

	//rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	//rs232a.rxd_handler().set("deuce", FUNC(wd2123_device::write_rxd_a));
	//rs232a.cts_handler().set("deuce", FUNC(wd2123_device::write_cts_a));
	//rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	//rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	//rs232b.rxd_handler().set("deuce", FUNC(wd2123_device::write_rxd_b));
	//rs232b.cts_handler().set("deuce", FUNC(wd2123_device::write_cts_b));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 1200).add_route(ALL_OUTPUTS, "mono", 0.50); // TODO: unknown frequency

	WD1770(config, m_fdc, 16_MHz_XTAL / 2);

	FLOPPY_CONNECTOR(config, "fdc:0", "525qd", FLOPPY_525_QD, true, microbx2_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", "525qd", FLOPPY_525_QD, true, microbx2_state::floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "flop_list").set_original("microbox2_flop");
}


ROM_START(microbx2)
	ROM_REGION(0x2000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "450", "Mon09 Ver 4.5")
	ROMX_LOAD("mon09_4.5.bin",  0x0000, 0x2000, CRC(4ccf92bf) SHA1(a0e778ed7498afcbca17082ca64828ed7967b3c3), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "422", "Mon09 Ver 4.22")
	ROMX_LOAD("mon09_4.22.bin", 0x0000, 0x2000, CRC(f6ea63ca) SHA1(a9c77df9f959ac26f21472d53aad8f9f6045c054), ROM_BIOS(1))

	ROM_REGION(0x10000,"epromdisc",0)
	ROM_LOAD("eprom_disc_1.bin", 0x0000, 0x4000, CRC(5132c397) SHA1(cb882af5cd6632503b9f8ec98e863c8217f0022a))
	ROM_LOAD("eprom_disc_2.bin", 0x4000, 0x4000, CRC(dcc3862a) SHA1(a78df097b9e5c8c6cf75a1af0d081bd6cf0cb39a))
	ROM_LOAD("eprom_disc_3.bin", 0x8000, 0x4000, CRC(64de116d) SHA1(dd8001aae11fa6f2d0b2061a64e1f1ef59cb2bc3))
	ROM_LOAD("eprom_disc_4.bin", 0xc000, 0x4000, CRC(4533d8d9) SHA1(56f0d2fed44841cdfb019e49632c5ceca67c92b2))
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT         COMPANY            FULLNAME        FLAGS
COMP( 1984, microbx2,  0,      0,      microbx2,  microbx2,  microbx2_state,  empty_init,  "Micro Concepts",  "Microbox II",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
