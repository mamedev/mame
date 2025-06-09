// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Applix 1616 computer

    See for docs: http://psiphi.server101.com/applix/

    First revealed to the world in December 1986 issue of Electronics Today
    International (ETI) an Australian electronics magazine which is now defunct.

    The main articles appeared in ETI February/March/April 1987, followed by
    other articles in various issues after that.

    Current Status:
    After 60 seconds, boots to the ramdisk. You can enter commands.
    If you have a floppy mounted, it will boot from the disk.

    The system could support 1 or 2 5.25 or 3.5 floppy drives, although 3.5
    was the recommended hardware. Format is similar to the PC 720kb, except
    it has 5 sectors of 1024 bytes, giving 800kb total. We only support the
    3.5-sized disks.

    TODO:
    - Cassette interface (coded but not working)
    - Use kbtro device (tried and failed)
    - Optional SCSI controller NCR5380 and hard drive (max 40mb)
    - Joystick
    - Audio: it could be better
    - DAC output is used to compare against analog inputs; core doesn't permit
      audio outputs to be used for non-speaker purposes.
    - BIOS 5 crashes MAME after scrolling about half a screen

****************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/6522via.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "machine/z80scc.h"
#include "sound/dac.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/applix_dsk.h"



class applix_state : public driver_device
{
public:
	applix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_base(*this, "base")
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_via(*this, "via6522")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_ldac(*this, "ldac")
		, m_rdac(*this, "rdac")
		, m_cass(*this, "cassette")
		, m_io_dsw(*this, "DSW")
		, m_io_fdc(*this, "FDC")
		, m_io_k0f(*this, "K0f")
		, m_io_k3x0(*this, "K3%x_0", 0U)
		, m_io_k3x1(*this, "K3%x_1", 0U)
		, m_io_k0b(*this, "K0b")
		, m_expansion(*this, "expansion")
		, m_palette(*this, "palette")
	{ }

	void applix(machine_config &config);

	void init_applix();

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	u16 applix_inputs_r();
	void palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void analog_latch_w(u16 data);
	void dac_latch_w(u16 data);
	void video_latch_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 applix_pb_r();
	void applix_pa_w(u8 data);
	void applix_pb_w(u8 data);
	void vsync_w(int state);
	u8 port00_r();
	u8 port08_r();
	u8 port10_r();
	u8 port18_r();
	u8 port20_r();
	u8 port60_r();
	void port08_w(u8 data);
	void port10_w(u8 data);
	void port18_w(offs_t offset, u8 data);
	void port20_w(u8 data);
	void port60_w(u8 data);
	u16 fdc_data_r();
	u16 fdc_stat_r(offs_t offset);
	void fdc_data_w(u16 data);
	void fdc_cmd_w(u16 data);
	static void floppy_formats(format_registration &fr);
	u8 internal_data_read(offs_t offset);
	void internal_data_write(offs_t offset, u8 data);
	u8 p1_read();
	void p1_write(u8 data);
	u8 p2_read();
	void p2_write(u8 data);
	u8 p3_read();
	void p3_write(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(cass_timer);

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_BEGIN_UPDATE(crtc_update_border);
	void applix_palette(palette_device &palette) const;

	u8 m_video_latch = 0U;
	u8 m_pa = 0U;
	u8 m_palette_latch[4]{};
	required_shared_ptr<u16> m_base;

	void main_mem(address_map &map) ATTR_COLD;
	void keytronic_pc3270_io(address_map &map) ATTR_COLD;
	void keytronic_pc3270_program(address_map &map) ATTR_COLD;
	void sub_io(address_map &map) ATTR_COLD;
	void sub_mem(address_map &map) ATTR_COLD;

	u8 m_pb = 0U;
	u8 m_analog_latch = 0U;
	u8 m_dac_latch = 0U;
	u8 m_port08 = 0U;
	u8 m_data_to_fdc = 0U;
	u8 m_data_from_fdc = 0U;
	bool m_data = 0;
	bool m_data_or_cmd = 0;
	bool m_buffer_empty = 0;
	bool m_fdc_cmd = 0;
	u8 m_clock_count = 0U;
	bool m_cp = 0;
	u8   m_p1 = 0U;
	u8   m_p1_data = 0U;
	u8   m_p2 = 0U;
	u8   m_p3 = 0U;
	u16  m_last_write_addr = 0U;
	u8 m_cass_data[4]{};
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<via6522_device> m_via;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<wd1772_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<dac_byte_interface> m_ldac;
	required_device<dac_byte_interface> m_rdac;
	required_device<cassette_image_device> m_cass;
	required_ioport m_io_dsw;
	required_ioport m_io_fdc;
	required_ioport m_io_k0f;
	required_ioport_array<12> m_io_k3x0;
	required_ioport_array<8> m_io_k3x1;
	required_ioport m_io_k0b;
	required_shared_ptr<u16> m_expansion;

	required_device<palette_device> m_palette;
};

/*
d0,1,2 = joystick
d3     = cassette LED, low=on
d4,5,6 = audio select
d7     = cassette relay, low=on
*/
void applix_state::analog_latch_w(u16 data)
{
	data &= 0xff;
	if (data != m_analog_latch)
	{
		m_cass->change_state(
			(BIT(data,7)) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

		m_analog_latch = data;
	}
}

void applix_state::dac_latch_w(u16 data)
{
	data &= 0xff;
	m_dac_latch = data;

	if ((m_analog_latch & 0x70) == 0) // right
		m_rdac->write(m_dac_latch);
	else
	if ((m_analog_latch & 0x70) == 0x10) // left
		m_ldac->write(m_dac_latch);
}

//cent = odd, video = even
void applix_state::palette_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset >>= 4;
	if (ACCESSING_BITS_0_7)
	{
		m_cent_data_out->write(data);
	}
	else
		m_palette_latch[offset] = (data >> 8) & 15;
}

void applix_state::video_latch_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_video_latch = data;
}

/*
d0   = dac output + external signal = analog input
d1   = cassette in
d2,3 = joystick in
d4-7 = SW2 dipswitch block
*/
u16 applix_state::applix_inputs_r()
{
	return m_io_dsw->read() | m_cass_data[2];
}

u8 applix_state::applix_pb_r()
{
	return m_pb;
}

/*
d0 = /(in) printer busy signal
d1 = /(out) printer strobe
d2 = /(out) enable cassette write IRQ
d3 = (out) H = 640 video mode
d4 = /(out) enable cassette read IRQ
d5 = /(out) clear cass IRQ and output line
d6 = /(out) reset keyboard by pulling kbd clock low
d7 = /(out) reset keyboard flipflop
*/
void applix_state::applix_pa_w(u8 data)
{
	// Reset flipflop counter
	if (!BIT(data, 7))
		m_clock_count = 0;

	// Reset keyboard
	if (!BIT(data, 6))
	{
		m_p3 = 0xff;
		m_last_write_addr = 0;
	}
	m_cass->output(BIT(data, 5) ? -1.0 : +1.0);

	// high-to-low of PA5 when reading cassette - /PRE on IC32b
	if (BIT(m_pa, 5) && !BIT(data, 5) && !BIT(data, 4))
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);

	// low-to-high of PA2 when writing cassette - /PRE on IC49
	if (!BIT(m_pa, 2) && BIT(data, 2))
		m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);

	m_centronics->write_strobe(BIT(data, 1));

	m_pa = data;
}

/*
d0-6 = user
d7   = square wave output for cassette IRQ
*/
void applix_state::applix_pb_w(u8 data)
{
	// low-to-high of PB7 when writing cassette - CLK on IC49
	if (!BIT(m_pb, 7) && BIT(data, 7))
		if (!BIT(m_pa, 2))
			m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);

	m_pb = data;
}

/*
d0 = H if 68000 sent a command
d1 = H if 68000 sent a byte
d2 = H if 68000 has read last byte
d3 = test switch
*/
u8 applix_state::port00_r()
{
	return (u8)m_data_or_cmd | ((u8)m_data << 1) | ((u8)m_buffer_empty << 2) | m_io_fdc->read();
}

/*
d0 = /RDY
d1 = /DISC CHANGE
d2 = DS0
d3 = DS1
d4 = MOTORON
d5 = SIDE
d6 = BANK
d7 = MAP
*/
u8 applix_state::port08_r()
{
	return m_port08 | 3;
}

/*
d0 = /INUSE
d1 = /EJECT
d2-7 same as for port08_r
*/
void applix_state::port08_w(u8 data)
{
	m_port08 = data;
	membank("bank1")->set_entry(BIT(data, 6));

	floppy_image_device *floppy = nullptr;
	if (BIT(data, 2)) floppy = m_floppy[0]->get_device();
	if (BIT(data, 3)) floppy = m_floppy[1]->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 5));
	}
}

u8 applix_state::port10_r()
{
	return 0;
}

void applix_state::port10_w(u8 data)
{
}

u8 applix_state::port18_r()
{
	m_data = 0;
	return m_data_to_fdc;
}

void applix_state::port18_w(offs_t offset, u8 data)
{
	m_data_from_fdc = data;
	m_buffer_empty = 0;
	m_fdc_cmd = BIT(offset, 2);
}

u8 applix_state::port20_r()
{
	return 0;
}

void applix_state::port20_w(u8 data)
{
}

u8 applix_state::port60_r()
{
	return 0;
}

void applix_state::port60_w(u8 data)
{
}

u16 applix_state::fdc_stat_r(offs_t offset)
{
	u8 data = 0;
	switch (offset)
	{
	case 0: data = (u8)m_buffer_empty^1; break;
	case 1: data = (u8)m_data^1; break;
	default: data = (u8)m_fdc_cmd; // case 2
	}
	return data << 7;
}

u16 applix_state::fdc_data_r()
{
	m_buffer_empty = 1;
	return m_data_from_fdc;
}

void applix_state::fdc_data_w(u16 data)
{
	m_data_to_fdc = data;
	m_data = 1;
	m_data_or_cmd = 0;
}

void applix_state::fdc_cmd_w(u16 data)
{
	m_data_to_fdc = data;
	m_data = 1;
	m_data_or_cmd = 1;
}

void applix_state::main_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xffffff);
	map(0x000000, 0x3fffff).ram().share("expansion"); // Expansion
	map(0x400000, 0x47ffff).ram().mirror(0x80000).share("base"); // Main ram
	map(0x500000, 0x51ffff).rom().region("maincpu", 0);
	map(0x600000, 0x60007f).w(FUNC(applix_state::palette_w));
	map(0x600080, 0x6000ff).w(FUNC(applix_state::dac_latch_w));
	map(0x600100, 0x60017f).w(FUNC(applix_state::video_latch_w)); //video latch (=border colour, high nybble; video base, low nybble) (odd)
	map(0x600180, 0x6001ff).w(FUNC(applix_state::analog_latch_w));
	map(0x700000, 0x700007).mirror(0x78).rw("scc", FUNC(scc8530_device::ab_dc_r), FUNC(scc8530_device::ab_dc_w)).umask16(0xff00).cswidth(16);
	map(0x700080, 0x7000ff).r(FUNC(applix_state::applix_inputs_r));
	map(0x700100, 0x70011f).mirror(0x60).m(m_via, FUNC(via6522_device::map)).umask16(0xff00).cswidth(16);
	map(0x700180, 0x700180).mirror(0x7c).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w)).cswidth(16);
	map(0x700182, 0x700182).mirror(0x7c).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w)).cswidth(16);
	map(0xffffc0, 0xffffc1).rw(FUNC(applix_state::fdc_data_r), FUNC(applix_state::fdc_data_w));
	//map(0xffffc2, 0xffffc3).rw(FUNC(applix_state::fdc_int_r) , FUNC(applix_state::fdc_int_w)); // optional
	map(0xffffc8, 0xffffcd).r(FUNC(applix_state::fdc_stat_r));
	map(0xffffd0, 0xffffd1).w(FUNC(applix_state::fdc_cmd_w));
	//600000, 6FFFFF  io ports and latches
	//700000, 7FFFFF  peripheral chips and devices
	//800000, FFC000  optional roms
	//FFFFC0, FFFFFF  disk controller board
}

void applix_state::sub_mem(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x7fff).ram();
	map(0x8000, 0xffff).bankrw("bank1");
}

void applix_state::sub_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x07).r(FUNC(applix_state::port00_r)); //PORTR
	map(0x08, 0x0f).rw(FUNC(applix_state::port08_r), FUNC(applix_state::port08_w)); //Disk select
	map(0x10, 0x17).rw(FUNC(applix_state::port10_r), FUNC(applix_state::port10_w)); //IRQ
	map(0x18, 0x1f).rw(FUNC(applix_state::port18_r), FUNC(applix_state::port18_w)); //data&command
	map(0x20, 0x27).mirror(0x18).rw(FUNC(applix_state::port20_r), FUNC(applix_state::port20_w)); //SCSI NCR5380
	map(0x40, 0x43).mirror(0x1c).rw(m_fdc, FUNC(wd1772_device::read), FUNC(wd1772_device::write)); //FDC
	map(0x60, 0x63).mirror(0x1c).rw(FUNC(applix_state::port60_r), FUNC(applix_state::port60_w)); //anotherZ80SCC
}

void applix_state::keytronic_pc3270_program(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("kbdcpu", 0);
}

void applix_state::keytronic_pc3270_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(applix_state::internal_data_read), FUNC(applix_state::internal_data_write));
}

// io priorities:
// 4 cassette
// 3 scc
// 2 via

/* Input ports */
static INPUT_PORTS_START( applix )
	PORT_START( "K0f" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_5)                                PORT_CHAR('5') PORT_CHAR('%')       /* 06 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_4)                                PORT_CHAR('4') PORT_CHAR('$')       /* 05 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_T)                                PORT_CHAR('t') PORT_CHAR('T')       /* 14 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_R)                                PORT_CHAR('r') PORT_CHAR('R')       /* 13 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_G)                                PORT_CHAR('g') PORT_CHAR('G')       /* 22 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F)                                PORT_CHAR('f') PORT_CHAR('F')       /* 21 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F7 (IRMA)")              /* 41 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?6a?")                   /* 6a */

	PORT_START( "K30_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_N)                                PORT_CHAR('n') PORT_CHAR('N')       /* 31 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_M)                                PORT_CHAR('m') PORT_CHAR('M')       /* 32 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_B)                                PORT_CHAR('b') PORT_CHAR('B')       /* 30 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_V)                                PORT_CHAR('v') PORT_CHAR('V')       /* 2f */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_C)                                PORT_CHAR('c') PORT_CHAR('C')       /* 2e */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_COMMA)                            PORT_CHAR(',') PORT_CHAR('<')       /* 33 */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K30_1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F1)                               PORT_CHAR(UCHAR_MAMEKEY(F1))        /* 58 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F2)                               PORT_CHAR(UCHAR_MAMEKEY(F2))        /* 59 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F3)                               PORT_CHAR(UCHAR_MAMEKEY(F3))        /* 5a */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F4)                               PORT_CHAR(UCHAR_MAMEKEY(F4))        /* 5b */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F5)                               PORT_CHAR(UCHAR_MAMEKEY(F5))        /* 5c */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F6)                               PORT_CHAR(UCHAR_MAMEKEY(F6))        /* 5d */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?6b?")                   /* 6b */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F8 (IRMA)")              /* 42 */

	PORT_START( "K31_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_6)                                PORT_CHAR('6') PORT_CHAR('^')       /* 07 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_7)                                PORT_CHAR('7') PORT_CHAR('&')       /* 08 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_Y)                                PORT_CHAR('y') PORT_CHAR('Y')       /* 15 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_U)                                PORT_CHAR('u') PORT_CHAR('U')       /* 16 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_H)                                PORT_CHAR('h') PORT_CHAR('H')       /* 23 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_J)                                PORT_CHAR('j') PORT_CHAR('J')       /* 24 */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K31_1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F7)                               PORT_CHAR(UCHAR_MAMEKEY(F7))        /* 37 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F8)                               PORT_CHAR(UCHAR_MAMEKEY(F8))        /* 5f */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_LSHIFT)       PORT_NAME("LShift") PORT_CHAR(UCHAR_SHIFT_1)            /* 2a */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("<")                      /* 70 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_Z)                                PORT_CHAR('z') PORT_CHAR('Z')       /* 2c */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_X)                                PORT_CHAR('x') PORT_CHAR('X')       /* 2d */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?6c?")                   /* 6c */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F9 (IRMA)")              /* 43 */

	PORT_START( "K32_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_9)                                PORT_CHAR('9') PORT_CHAR('(')       /* 0a */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_8)                                PORT_CHAR('8') PORT_CHAR('*')       /* 09 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_O)                                PORT_CHAR('o') PORT_CHAR('O')       /* 18 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_I)                                PORT_CHAR('i') PORT_CHAR('I')       /* 17 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_L)                                PORT_CHAR('l') PORT_CHAR('L')       /* 26 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_K)                                PORT_CHAR('k') PORT_CHAR('K')       /* 25 */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K32_1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F9)                               PORT_CHAR(UCHAR_MAMEKEY(F9))        /* 57 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F10)                              PORT_CHAR(UCHAR_MAMEKEY(F10))       /* 1d */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_LCONTROL)                         PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))  /* 71 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_LALT)                             PORT_NAME("LAlt")                   /* 38 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_SPACE)                            PORT_CHAR(' ')                      /* 39 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_RALT)                             PORT_NAME("RAlt")                   /* 38 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?69?")                   /* 69 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F6 (IRMA)")              /* 40 */

	PORT_START( "K33_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_DOWN)    PORT_NAME("KP 2")                   /* 50 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_END)     PORT_NAME("KP 1")                   /* 4f */
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("Down")                   /* 55 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("Enter")                  /* 75 */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K33_1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_1)                                PORT_CHAR('1') PORT_CHAR('!')       /* 02 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_TILDE)                            PORT_CHAR('`') PORT_CHAR('~')       /* 29 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_Q)                                PORT_CHAR('q') PORT_CHAR('Q')       /* 10 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_TAB)                              PORT_CHAR(9)                        /* 0f */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_A)                                PORT_CHAR('a') PORT_CHAR('A')       /* 1e */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_CAPSLOCK)                         PORT_NAME("Caps")                   /* 3a */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?68?")                   /* 68 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F5 (IRMA)")              /* 3f */

	PORT_START( "K34_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_SLASH)                            PORT_CHAR('/') PORT_CHAR('?')       /* 35 */
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_RSHIFT)                           PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))    /* 36 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("Left")                   /* 56 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_STOP)                             PORT_CHAR('.') PORT_CHAR('>')       /* 34 */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K34_1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_2)                                PORT_CHAR('2') PORT_CHAR('@')       /* 02 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_3)                                PORT_CHAR('3') PORT_CHAR('#')       /* 03 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_W)                                PORT_CHAR('w') PORT_CHAR('W')       /* 11 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_E)                                PORT_CHAR('e') PORT_CHAR('E')       /* 12 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_S)                                PORT_CHAR('s') PORT_CHAR('S')       /* 1f */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_D)                                PORT_CHAR('d') PORT_CHAR('D')       /* 20 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?67?")                   /* 67 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F4 (IRMA)")              /* 3e */

	PORT_START( "K35_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_0)                                PORT_CHAR('0') PORT_CHAR(')')       /* 0b */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_MINUS)                            PORT_CHAR('-') PORT_CHAR('_')       /* 0c */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_P)                                PORT_CHAR('p') PORT_CHAR('P')       /* 19 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_OPENBRACE)                        PORT_CHAR('[') PORT_CHAR('{')       /* 1a */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_COLON)                            PORT_CHAR(';') PORT_CHAR(':')       /* 27 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_QUOTE)                            PORT_CHAR('\'') PORT_CHAR('"')      /* 28 */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K35_1" )
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?66?")                   /* 66 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F3 (IRMA)")              /* 3d */

	PORT_START( "K36_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_BACKSPACE)                        PORT_CHAR(8)                        /* 0e */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_EQUALS)                           PORT_CHAR('=') PORT_CHAR('+')       /* 0d */
	PORT_BIT( 0x14, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_ENTER)                            PORT_CHAR(13)                       /* 1c */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_BACKSLASH)                        PORT_CHAR('\\') PORT_CHAR('|')      /* 2b */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_CLOSEBRACE)                       PORT_CHAR(']') PORT_CHAR('}')       /* 1b */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K36_1" )
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F2 (IRMA)")              /* 3c */

	PORT_START( "K37_0" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("PA1")                    /* 7b */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("|<--")                   /* 7e */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("/a\\")                   /* 7a */
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_PLUS_PAD)                         PORT_NAME("KP +")                   /* 4e */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K37_1" )
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?64?")                   /* 64 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F1 (IRMA)")              /* 3b */

	PORT_START( "K38_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("SysReq")                 /* 54 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   /*PORT_CODE(KEYCODE_SCRLOCK)*/                      PORT_NAME("ScrLock")                /* 46 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("-->|")                   /* 7c */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_PGUP)    PORT_NAME("KP 9")                   /* 49 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_MINUS_PAD)                        PORT_NAME("KP -")                   /* 4a */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_RIGHT)   PORT_NAME("KP 6")                   /* 4d */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K39_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_ESC)                              PORT_NAME("Esc")                    /* 01 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_NUMLOCK)                          PORT_NAME("NumLock")                /* 45 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_HOME)    PORT_NAME("KP 7")                   /* 47 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_UP)      PORT_NAME("KP 8")                   /* 48 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_LEFT)    PORT_NAME("KP 4")                   /* 4b */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_5_PAD)                            PORT_NAME("KP 5")                   /* 4c */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?76?")                   /* 76 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?63?")                   /* 63 */

	PORT_START( "K3a_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("PrtSc *")                /* 6f */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("PA2")                    /* 7f */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("Right")                  /* 7d */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("/a")                     /* 79 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("Center")                 /* 77 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?6e?")                   /* 6e */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?62?")                   /* 62 */

	PORT_START( "K3b_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_PGDN)    PORT_NAME("KP 3")                   /* 51 */
	PORT_BIT( 0x06, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_INSERT)  PORT_NAME("KP 0")                   /* 52 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(KEYCODE_DEL)   PORT_NAME("KP .")                   /* 53 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("Up")                     /* 78 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?6d?")                   /* 6d */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F10 (IRMA)")             /* 44 */

	PORT_START( "K0b" )
	PORT_DIPNAME( 0x01, 0x01, "Protocol selection" )
	PORT_DIPSETTING( 0x00, "Enhanced XT, AT and PS/2 models" )
	PORT_DIPSETTING( 0x01, "Standard PC and XT" )
	PORT_DIPNAME( 0x02, 0x00, "IRMA/Native scan code set" )
	PORT_DIPSETTING( 0x00, "Native scan code set" )
	PORT_DIPSETTING( 0x02, "IRMA Emulation" )
	PORT_DIPNAME( 0x04, 0x04, "Enhanced 101/Native scan code set" )
	PORT_DIPSETTING( 0x00, "Native scan code set" )
	PORT_DIPSETTING( 0x04, "Enhanced 101 scan code set" )
	PORT_DIPNAME( 0x08, 0x08, "Enable E0" )
	PORT_DIPSETTING( 0x00, "Enable E0" )
	PORT_DIPSETTING( 0x08, "Disable E0" )
	PORT_DIPNAME( 0x10, 0x10, "Code tables" )
	PORT_DIPSETTING( 0x00, "U.S. code tables" )
	PORT_DIPSETTING( 0x10, "International code tables" )
	PORT_BIT( 0x60, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x80, "Key click" )
	PORT_DIPSETTING( 0x00, "No key click" )
	PORT_DIPSETTING( 0x80, "Key click" )

	PORT_START("DSW")
	PORT_BIT( 0xf, 0, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Switch 0") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x00, "Switch 1") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x00, "Switch 2") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x80, "Switch 3") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))

	PORT_START("FDC")
	PORT_BIT( 0xf7, 0, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, "FDC Test") PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
INPUT_PORTS_END


void applix_state::machine_reset()
{
	u8* ROM = memregion("maincpu")->base();
	memcpy(m_expansion, ROM, 8);
	membank("bank1")->set_entry(0);
	m_p3 = 0xff;
	m_last_write_addr = 0;
	m_maincpu->reset();
}

void applix_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_APPLIX_FORMAT);
}

static void applix_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}


void applix_state::applix_palette(palette_device &palette) const
{
	// shades need to be verified - the names on the right are from the manual
	constexpr rgb_t colors[16] = {
			{ 0x00, 0x00, 0x00 },   //  0 Black
			{ 0x40, 0x40, 0x40 },   //  1 Dark Grey
			{ 0x00, 0x00, 0x80 },   //  2 Dark Blue
			{ 0x00, 0x00, 0xff },   //  3 Mid Blue
			{ 0x00, 0x80, 0x00 },   //  4 Dark Green
			{ 0x00, 0xff, 0x00 },   //  5 Green
			{ 0x00, 0xff, 0xff },   //  6 Blue Grey
			{ 0x00, 0x7f, 0x7f },   //  7 Light Blue
			{ 0x7f, 0x00, 0x00 },   //  8 Dark Red
			{ 0xff, 0x00, 0x00 },   //  9 Red
			{ 0x7f, 0x00, 0x7f },   // 10 Dark Violet
			{ 0xff, 0x00, 0xff },   // 11 Violet
			{ 0x7f, 0x7f, 0x00 },   // 12 Brown
			{ 0xff, 0xff, 0x00 },   // 13 Yellow
			{ 0xbf, 0xbf, 0xbf },   // 14 Light Grey
			{ 0xff, 0xff, 0xff } }; // 15 White

	palette.set_pen_colors(0, colors);
}


void applix_state::machine_start()
{
	std::fill(std::begin(m_palette_latch), std::end(m_palette_latch), 0);

	save_item(NAME(m_video_latch));
	save_item(NAME(m_pa));
	save_item(NAME(m_palette_latch));
	save_item(NAME(m_pb));
	save_item(NAME(m_analog_latch));
	save_item(NAME(m_dac_latch));
	save_item(NAME(m_port08));
	save_item(NAME(m_data_to_fdc));
	save_item(NAME(m_data_from_fdc));
	save_item(NAME(m_data));
	save_item(NAME(m_data_or_cmd));
	save_item(NAME(m_buffer_empty));
	save_item(NAME(m_fdc_cmd));
	save_item(NAME(m_clock_count));
	save_item(NAME(m_cp));
	save_item(NAME(m_p1));
	save_item(NAME(m_p1_data));
	save_item(NAME(m_p2));
	save_item(NAME(m_p3));
	save_item(NAME(m_last_write_addr));
	save_item(NAME(m_cass_data));
}

MC6845_UPDATE_ROW( applix_state::crtc_update_row )
{
	if (!de)
		return;
	// The display is bitmapped. 2 modes are supported here, 320x200x16 and 640x200x4.
	// There is a monochrome mode, but no info found as yet.
	// The 6845 cursor signal is not used at all.
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u32 const vidbase = (m_video_latch & 15) << 14 | (ra & 7) << 12;
	u32 *p = &bitmap.pix(y + vbp, hbp);

	for (u16 x = 0; x < x_count; x++)
	{
		u32 const mem = vidbase | ((ma + x) & 0xfff);
		u16 chr = m_base[mem];

		if (BIT(m_pa, 3))
		{
			// 640 x 200 x 4of16 mode
			for (int i = 0; i < 8; i++)
			{
				*p++ = palette[m_palette_latch[chr>>14]];
				chr <<= 2;
			}
		}
		else
		{
			// 320 x 200 x 16 mode
			for (int i = 0; i < 4; i++)
			{
				*p++ = palette[chr>>12];
				*p++ = palette[chr>>12];
				chr <<= 4;
			}
		}
	}
}

MC6845_BEGIN_UPDATE( applix_state::crtc_update_border )
{
	bitmap.fill(m_palette->pen(m_video_latch >> 4), cliprect);
}

void applix_state::vsync_w(int state)
{
	m_via->write_ca2(state);
}

TIMER_DEVICE_CALLBACK_MEMBER(applix_state::cass_timer)
{
	/* cassette - turn 2500/5000Hz to a bit */
	m_cass_data[1]++;
	u8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_cass_data[2] = ((m_cass_data[1] < 12) ? 2 : 0);
		m_cass_data[1] = 0;
		// low-to-high transition when reading cassette - CLK on IC32b
		if ((cass_ws) && !BIT(m_pa, 4))
			m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
	}
}

void applix_state::applix(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 30_MHz_XTAL / 4); // MC68000-P10 @ 7.5 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &applix_state::main_mem);

	z80_device &subcpu(Z80(config, "subcpu", 16_MHz_XTAL / 2)); // Z80H
	subcpu.set_addrmap(AS_PROGRAM, &applix_state::sub_mem);
	subcpu.set_addrmap(AS_IO, &applix_state::sub_io);

	i8051_device &kbdcpu(I8051(config, "kbdcpu", 11060250));
	kbdcpu.set_addrmap(AS_PROGRAM, &applix_state::keytronic_pc3270_program);
	kbdcpu.set_addrmap(AS_IO, &applix_state::keytronic_pc3270_io);
	kbdcpu.port_in_cb<1>().set(FUNC(applix_state::p1_read));
	kbdcpu.port_out_cb<1>().set(FUNC(applix_state::p1_write));
	kbdcpu.port_in_cb<2>().set(FUNC(applix_state::p2_read));
	kbdcpu.port_out_cb<2>().set(FUNC(applix_state::p2_write));
	kbdcpu.port_in_cb<3>().set(FUNC(applix_state::p3_read));
	kbdcpu.port_out_cb<3>().set(FUNC(applix_state::p3_write));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 200);
	screen.set_visarea_full();
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	PALETTE(config, m_palette, FUNC(applix_state::applix_palette), 16);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();
	DAC0800(config, "ldac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 0); // 74ls374.u20 + dac0800.u21 + 4052.u23
	DAC0800(config, "rdac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0, 1); // 74ls374.u20 + dac0800.u21 + 4052.u23

	/* Devices */
	MC6845(config, m_crtc, 30_MHz_XTAL / 16); // MC6545 @ 1.875 MHz
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(applix_state::crtc_update_row));
	m_crtc->set_begin_update_callback(FUNC(applix_state::crtc_update_border));
	m_crtc->out_vsync_callback().set(FUNC(applix_state::vsync_w));

	MOS6522(config, m_via, 30_MHz_XTAL / 4 / 10); // VIA uses 68000 E clock
	m_via->readpb_handler().set(FUNC(applix_state::applix_pb_r));
	// in CB1 kbd clk
	// in CA2 vsync
	// in CB2 kdb data
	m_via->writepa_handler().set(FUNC(applix_state::applix_pa_w));
	m_via->writepb_handler().set(FUNC(applix_state::applix_pb_w));
	m_via->irq_handler().set_inputline("maincpu", M68K_IRQ_2);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_via, FUNC(via6522_device::write_ca1));
	m_centronics->busy_handler().set(m_via, FUNC(via6522_device::write_pa0));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "speaker", 0.10, 0);

	WD1772(config, m_fdc, 16_MHz_XTAL / 2); //connected to Z80H clock pin
	FLOPPY_CONNECTOR(config, m_floppy[0], applix_floppies, "35dd", applix_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], applix_floppies, "35dd", applix_state::floppy_formats).enable_sound(true);
	TIMER(config, "applix_c").configure_periodic(FUNC(applix_state::cass_timer), attotime::from_hz(100000));

	scc8530_device &scc(SCC8530(config, "scc", 30_MHz_XTAL / 8));
	scc.out_txda_callback().set("serial_a", FUNC(rs232_port_device::write_txd));
	scc.out_rtsa_callback().set("serial_a", FUNC(rs232_port_device::write_rts));
	scc.out_dtra_callback().set("serial_a", FUNC(rs232_port_device::write_dtr));
	scc.out_txdb_callback().set("serial_b", FUNC(rs232_port_device::write_txd));
	scc.out_rtsb_callback().set("serial_b", FUNC(rs232_port_device::write_rts));
	scc.out_dtrb_callback().set("serial_b", FUNC(rs232_port_device::write_dtr));
	scc.out_int_callback().set_inputline("maincpu", M68K_IRQ_3);

	rs232_port_device &serial_a(RS232_PORT(config, "serial_a", default_rs232_devices, nullptr));
	serial_a.rxd_handler().set("scc", FUNC(scc8530_device::rxa_w));
	serial_a.cts_handler().set("scc", FUNC(scc8530_device::ctsa_w));
	serial_a.cts_handler().set("scc", FUNC(scc8530_device::dcda_w));

	rs232_port_device &serial_b(RS232_PORT(config, "serial_b", default_rs232_devices, nullptr));
	serial_b.rxd_handler().set("scc", FUNC(scc8530_device::rxb_w));
	serial_b.cts_handler().set("scc", FUNC(scc8530_device::ctsb_w));
	serial_b.cts_handler().set("scc", FUNC(scc8530_device::dcdb_w));

	SOFTWARE_LIST(config, "flop_list").set_original("applix_flop");
}

/* ROM definition */
ROM_START( applix )
	ROM_REGION16_BE(0x20000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v4.5a", "V4.5a")
	ROMX_LOAD( "1616osl.45a", 0x00000, 0x10000, CRC(9dfb3224) SHA1(5223833a357f90b147f25826c01713269fc1945f), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "1616osh.45a", 0x00001, 0x10000, CRC(951bd441) SHA1(e0a38c8d0d38d84955c1de3f6a7d56ce06b063f6), ROM_SKIP(1) | ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "v4.4a", "V4.4a")
	ROMX_LOAD( "1616osl.44a", 0x00000, 0x10000, CRC(4a1a90d3) SHA1(4df504bbf6fc5dad76c29e9657bfa556500420a6), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "1616osh.44a", 0x00001, 0x10000, CRC(ef619994) SHA1(ff16fe9e2c99a1ffc855baf89278a97a2a2e881a), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "v4.3a", "V4.3a")
	ROMX_LOAD( "1616osl.43a", 0x00000, 0x10000, CRC(c09b9ff8) SHA1(c46f2a98470d2d09cf9f9eec0f4096ab762407b5), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "1616osh.43a", 0x00001, 0x10000, CRC(071a2505) SHA1(42c4cc6e3e78b6a5320f9d9c858fc9f4e6220857), ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "v4.0c", "V4.0c")
	ROMX_LOAD( "1616osl.40c", 0x00000, 0x10000, CRC(6a517b5d) SHA1(e0f4eba0cb8d273ba681b9d2c6d4b1beff9ef325), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "1616osh.40c", 0x00001, 0x10000, CRC(7851651f) SHA1(d7d329aa7fe9f4418de0cdf813b61e70243e0e77), ROM_SKIP(1) | ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "v3.0b", "V3.0b")
	ROMX_LOAD( "1616osl.30b", 0x00000, 0x10000, CRC(fb9198c3) SHA1(e0e7a1dd176c1cbed063df1c405821c261d48f3a), ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "1616osh.30b", 0x00001, 0x10000, CRC(a279e1d7) SHA1(3451b2cae87a9ccee5f579fd1d49cf52d9f97b83), ROM_SKIP(1) | ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(5, "v2.4a", "V2.4a")
	ROMX_LOAD( "1616osl.24a", 0x00000, 0x08000, CRC(b155830b) SHA1(b32db6a06c8a3c544210ba9faba7c49497c504fb), ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "1616osh.24a", 0x00001, 0x08000, CRC(6d9fc0e0) SHA1(07111f46386494ed3f426c1e50308f0209587f06), ROM_SKIP(1) | ROM_BIOS(5) )

	ROM_REGION(0x18000, "subcpu", 0)
	ROM_LOAD( "1616ssdv.022", 0x0000, 0x8000, CRC(6d8e413a) SHA1(fc27d92c34f231345a387b06670f36f8c1705856) )

	ROM_REGION(0x20000, "user1", 0)
	ROM_LOAD( "ssdcromv.22",  0x0000, 0x8000, CRC(c85c47fb) SHA1(6f0bb3753fc0d74ee5901d71d05a74ec6a4a1d05) )
	ROM_LOAD( "ssddromv.14a", 0x8000, 0x8000, CRC(8fe2db78) SHA1(487484003aba4d8960101ced6a689dc81676235d) )

	ROM_REGION(0x2000, "kbdcpu", 0)
	ROM_LOAD( "14166.bin", 0x0000, 0x2000, CRC(1aea1b53) SHA1(b75b6d4509036406052157bc34159f7039cdc72e) )
ROM_END


void applix_state::init_applix()
{
	u8 *RAM = memregion("subcpu")->base();
	membank("bank1")->configure_entries(0, 2, &RAM[0x8000], 0x8000);
}


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY           FULLNAME       FLAGS
COMP( 1986, applix, 0,      0,      applix,  applix, applix_state, init_applix, "Applix Pty Ltd", "Applix 1616", MACHINE_SUPPORTS_SAVE )



/**************************************************** KEYBOARD MODULE *****************************************/

u8 applix_state::internal_data_read(offs_t offset)
{
	m_via->write_cb2( BIT(offset, 8) ); // data
	bool cp = !BIT(offset, 9);  // clock pulses //TODO tidy this up with real flipflops
	if (cp != m_cp)
	{
		m_cp = cp;
		if (cp)
			m_clock_count++;
	}
	if (m_clock_count > 1)
		m_via->write_cb1( cp );

	return 0xff;
}


void applix_state::internal_data_write(offs_t offset, u8 data)
{
	/* Check for low->high transition on AD8 */
	if ( ! ( m_last_write_addr & 0x0100 ) && ( offset & 0x0100 ) )
	{
		switch (m_p1)
		{
		case 0x0e:
			break;
		case 0x0f:
			m_p1_data = m_io_k0f->read();
			break;
		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36:
		case 0x38: case 0x39: case 0x3a: case 0x3b:
			m_p1_data = m_io_k3x0[m_p1 - 0x30]->read();
			break;
		case 0x37:
			m_p1_data = m_io_k3x0[7]->read() | (m_io_k3x0[6]->read() & 0x01);
			break;
		}
	}

	/* Check for low->high transition on AD9 */
	if ( ! ( m_last_write_addr & 0x0200 ) && ( offset & 0x0200 ) )
	{
		switch (m_p1)
		{
		case 0x0b:
			m_p1_data = m_io_k0b->read();
			break;
		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
			m_p1_data = m_io_k3x1[m_p1 - 0x30]->read();
			break;
		case 0x38:
			m_p1_data = 0xff;
			break;
		case 0x39:
			m_p1_data = 0xff;
			break;
		case 0x3a:
			m_p1_data = 0xff;
			break;
		}
	}

	m_last_write_addr = offset;
}


u8 applix_state::p1_read()
{
	return m_p1 & m_p1_data;
}


void applix_state::p1_write(u8 data)
{
	m_p1 = data;
}


u8 applix_state::p2_read()
{
	return m_p2;
}


void applix_state::p2_write(u8 data)
{
	m_p2 = data;
}


u8 applix_state::p3_read()
{
	u8 data = m_p3;

	data &= ~0x14;

	/* -INT0 signal */
	data |= 4;

	/* T0 signal */
	data |= 0;

	return data;
}


void applix_state::p3_write(u8 data)
{
	m_p3 = data;
}
