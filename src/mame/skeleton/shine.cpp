// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***********************************************************************************

Shine/1

TODO: everything to be verified.

The system rom has an inbuilt monitor program - unknown how to access it.

The floppy disk is accessed via an "expansion" socket on the back. Inbuilt commands
such as DIR [1|2], RENAME, ERASE, PROT, XFER look interesting. The DIR command does
multiple reads of 0x9E00.

There's a DIN socket for cassette. Commands BAUD, SAVE, LOAD appear to be the ones
but the syntax has yet to be worked out. BAUD [0-9] is allowed but what is it doing?


************************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "machine/6522via.h"
#include "machine/wd_fdc.h"
#include "machine/input_merger.h"
#include "sound/spkrdev.h"
#include "video/mc6847.h"
#include "bus/centronics/ctronics.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "utf8.h"


namespace {

class shine_state : public driver_device
{
public:
	shine_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vdg(*this, "vdg")
		, m_ram(*this, "ram")
		, m_via(*this, "via%u", 0)
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0)
		, m_centronics(*this, "centronics")
		, m_speaker(*this, "speaker")
		, m_cass(*this, "cassette")
		, m_irqs(*this, "irqs")
		, m_y(*this, "Y%u", 0U)
		, m_video_ram(*this, "video_ram")
	{ }

	void shine(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void shine_mem(address_map &map) ATTR_COLD;
	uint8_t via0_pa_r();
	void via0_pb_w(uint8_t data);
	void floppy_w(uint8_t data);
	uint8_t vdg_videoram_r(offs_t offset);

	required_device<cpu_device> m_maincpu;
	required_device<mc6847_base_device> m_vdg;
	required_device<ram_device> m_ram;
	required_device_array<via6522_device, 2> m_via;
	required_device<fd1771_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<centronics_device> m_centronics;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cass;
	required_device<input_merger_device> m_irqs;
	required_ioport_array<8> m_y;
	required_shared_ptr<uint8_t> m_video_ram;

	/* keyboard state */
	u8 m_keylatch = 0U;
};



void shine_state::shine_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x67ff).ram();
	map(0x6800, 0x7fff).ram().share(m_video_ram);
	map(0x9400, 0x940f).m(m_via[0], FUNC(via6522_device::map));
	map(0x9800, 0x980f).m(m_via[1], FUNC(via6522_device::map));
	map(0x9c00, 0x9c03).rw(m_fdc, FUNC(fd1771_device::read), FUNC(fd1771_device::write));
	map(0x9d00, 0x9d00).w(FUNC(shine_state::floppy_w));
	map(0xb000, 0xffff).rom().region("maincpu",0);
}


static INPUT_PORTS_START( shine )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL")               PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT")              PORT_CODE(KEYCODE_LSHIFT)     PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_A)          PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_B)          PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_C)          PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_D)          PORT_CHAR('D')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_E)          PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_F)          PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_G)          PORT_CHAR('G')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_H)          PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_I)          PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_J)          PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_K)          PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_L)          PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_M)          PORT_CHAR('M')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_N)          PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_O)          PORT_CHAR('O')

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_P)          PORT_CHAR('P')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_R)          PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_S)          PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_T)          PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_U)          PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_V)          PORT_CHAR('V')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_W)          PORT_CHAR('W')

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_X)          PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('[')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                                 PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP)              PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('^')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC")                PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC),27)

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("COPY")               PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RET")                PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL")                PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE")              PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REPT")               PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP UTF8_DOWN)    PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))    PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LOCK")               PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
INPUT_PORTS_END

/***************************************************************************
    DEVICE CONFIGURATION
***************************************************************************/

uint8_t shine_state::via0_pa_r()
{
	uint8_t data;

	data = m_y[m_keylatch]->read();

	return data;
}

void shine_state::via0_pb_w(uint8_t data)
{
	/* keyboard column */
	m_keylatch = data & 0x07;

	/* MC6847 - all to be verified */
	m_vdg->ag_w(BIT(data, 4));
	m_vdg->gm0_w(BIT(data, 5));
	m_vdg->gm1_w(BIT(data, 6));
	m_vdg->gm2_w(BIT(data, 7));
}


void shine_state::floppy_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	if (!BIT(data, 0)) floppy = m_floppy[0]->get_device();
	if (!BIT(data, 1)) floppy = m_floppy[1]->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
		floppy->mon_w(0);
}


uint8_t shine_state::vdg_videoram_r(offs_t offset)
{
	if (offset == ~0) return 0xff;

	offset ^= 0x17ff;

	m_vdg->as_w(BIT(m_video_ram[offset], 6));
	m_vdg->inv_w(BIT(m_video_ram[offset], 7));

	return m_video_ram[offset];
}


void shine_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_keylatch));
}


void shine_state::shine(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 2000000); // 2MHz ??
	m_maincpu->set_addrmap(AS_PROGRAM, &shine_state::shine_mem);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline("maincpu", M6502_IRQ_LINE);

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	MC6847_NTSC(config, m_vdg, 3.579545_MHz_XTAL); // or really PAL?
	m_vdg->set_screen("screen");
	m_vdg->input_callback().set(FUNC(shine_state::vdg_videoram_r));
	m_vdg->set_black_and_white(true);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 1.00);

	RAM(config, m_ram);
	m_ram->set_default_size("32K");
	m_ram->set_extra_options("16K");

	MOS6522(config, m_via[0], 1000000);
	m_via[0]->readpa_handler().set(FUNC(shine_state::via0_pa_r));
	m_via[0]->writepb_handler().set(FUNC(shine_state::via0_pb_w));
	m_via[0]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_via[0]->cb2_handler().set(m_speaker, FUNC(speaker_sound_device::level_w));

	MOS6522(config, m_via[1], 1000000);
	m_via[1]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");

	FD1771(config, m_fdc, 1000000);

	FLOPPY_CONNECTOR(config, m_floppy[0], "525qd", FLOPPY_525_QD, true, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], "525qd", FLOPPY_525_QD, false, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}


ROM_START( shine )
	ROM_REGION( 0x5000, "maincpu", 0 )
	ROM_LOAD("basic_plus.ic58", 0x0000, 0x1000, CRC(c75d9675) SHA1(eecc12533aa33c7744e1ea1fde56883739ac7436))
	ROM_LOAD("disco-dk2.ic59",  0x1000, 0x1000, CRC(b9230d50) SHA1(3975e5850356c035cd1963e0fbca1e980463ee01))
	ROM_LOAD("d3.ic62",         0x2000, 0x1000, CRC(7c88e219) SHA1(93003715bb82d63b6d4f673f89eae59c73f2945e))
	ROM_LOAD("e3.ic61",         0x3000, 0x1000, CRC(5a1624e9) SHA1(b4fbc983c646d4e70dda878d5dd75e45408522a9))
	ROM_LOAD("f3.ic60",         0x4000, 0x1000, CRC(1549ca2f) SHA1(5b011cdca0121a550af956b6d4580544942459ce))
ROM_END

} // anonymous namespace


/*    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT   CLASS         INIT         COMPANY                  FULLNAME    FLAGS */
COMP( 1983, shine,  0,      0,      shine,    shine,  shine_state,  empty_init,  "Lorenzon Elettronica",  "Shine/1", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
