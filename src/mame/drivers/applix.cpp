// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Applix 1616 computer

    See for docs: http;//www.microbee-mspp.org.au

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
    - Optional serial device Z8530 Z80SCC
    - Optional SCSI controller NCR5380 and hard drive (max 40mb)
    - Joystick
    - Audio: it could be better
    - DAC output is used to compare against analog inputs; core doesn't permit
      audio outputs to be used for non-speaker purposes.
    - Bios 5 crashes mess after scrolling about half a screen

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs51/mcs51.h"
#include "video/mc6845.h"
#include "machine/6522via.h"
#include "sound/dac.h"
#include "sound/wave.h"
#include "machine/wd_fdc.h"
#include "formats/applix_dsk.h"
#include "imagedev/cassette.h"
#include "bus/centronics/ctronics.h"



class applix_state : public driver_device
{
public:
	applix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_base(*this, "base"),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_via(*this, "via6522"),
		m_centronics(*this, "centronics"),
		m_cent_data_out(*this, "cent_data_out"),
		m_fdc(*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1"),
		m_dacl(*this, "dacl"),
		m_dacr(*this, "dacr"),
		m_cass(*this, "cassette"),
		m_io_dsw(*this, "DSW"),
		m_io_fdc(*this, "FDC"),
		m_io_k0f(*this, "K0f"),
		m_io_k300(*this, "K30_0"),
		m_io_k301(*this, "K30_1"),
		m_io_k310(*this, "K31_0"),
		m_io_k311(*this, "K31_1"),
		m_io_k320(*this, "K32_0"),
		m_io_k321(*this, "K32_1"),
		m_io_k330(*this, "K33_0"),
		m_io_k331(*this, "K33_1"),
		m_io_k340(*this, "K34_0"),
		m_io_k341(*this, "K34_1"),
		m_io_k350(*this, "K35_0"),
		m_io_k351(*this, "K35_1"),
		m_io_k360(*this, "K36_0"),
		m_io_k361(*this, "K36_1"),
		m_io_k370(*this, "K37_0"),
		m_io_k371(*this, "K37_1"),
		m_io_k380(*this, "K38_0"),
		m_io_k390(*this, "K39_0"),
		m_io_k3a0(*this, "K3a_0"),
		m_io_k3b0(*this, "K3b_0"),
		m_io_k0b(*this, "K0b"),
		m_expansion(*this, "expansion"),
		m_palette(*this, "palette"){ }

	DECLARE_READ16_MEMBER(applix_inputs_r);
	DECLARE_WRITE16_MEMBER(palette_w);
	DECLARE_WRITE16_MEMBER(analog_latch_w);
	DECLARE_WRITE16_MEMBER(dac_latch_w);
	DECLARE_WRITE16_MEMBER(video_latch_w);
	DECLARE_READ8_MEMBER(applix_pb_r);
	DECLARE_WRITE8_MEMBER(applix_pa_w);
	DECLARE_WRITE8_MEMBER(applix_pb_w);
	DECLARE_WRITE_LINE_MEMBER(vsync_w);
	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_READ8_MEMBER(port08_r);
	DECLARE_READ8_MEMBER(port10_r);
	DECLARE_READ8_MEMBER(port18_r);
	DECLARE_READ8_MEMBER(port20_r);
	DECLARE_READ8_MEMBER(port60_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(port10_w);
	DECLARE_WRITE8_MEMBER(port18_w);
	DECLARE_WRITE8_MEMBER(port20_w);
	DECLARE_WRITE8_MEMBER(port60_w);
	DECLARE_READ16_MEMBER(fdc_data_r);
	DECLARE_READ16_MEMBER(fdc_stat_r);
	DECLARE_WRITE16_MEMBER(fdc_data_w);
	DECLARE_WRITE16_MEMBER(fdc_cmd_w);
	DECLARE_FLOPPY_FORMATS(floppy_formats);
	DECLARE_READ8_MEMBER( internal_data_read );
	DECLARE_WRITE8_MEMBER( internal_data_write );
	DECLARE_READ8_MEMBER( p1_read );
	DECLARE_WRITE8_MEMBER( p1_write );
	DECLARE_READ8_MEMBER( p2_read );
	DECLARE_WRITE8_MEMBER( p2_write );
	DECLARE_READ8_MEMBER( p3_read );
	DECLARE_WRITE8_MEMBER( p3_write );
	TIMER_DEVICE_CALLBACK_MEMBER(cass_timer);
	DECLARE_DRIVER_INIT(applix);
	MC6845_UPDATE_ROW(crtc_update_row);
	UINT8 m_video_latch;
	UINT8 m_pa;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(applix);
	UINT8 m_palette_latch[4];
	required_shared_ptr<UINT16> m_base;
private:
	UINT8 m_pb;
	UINT8 m_analog_latch;
	UINT8 m_dac_latch;
	UINT8 m_port08;
	UINT8 m_data_to_fdc;
	UINT8 m_data_from_fdc;
	bool m_data;
	bool m_data_or_cmd;
	bool m_buffer_empty;
	bool m_fdc_cmd;
	UINT8 m_clock_count;
	bool m_cp;
	UINT8   m_p1;
	UINT8   m_p1_data;
	UINT8   m_p2;
	UINT8   m_p3;
	UINT16  m_last_write_addr;
	UINT8 m_cass_data[4];
	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<via6522_device> m_via;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<wd1772_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<dac_device> m_dacl;
	required_device<dac_device> m_dacr;
	required_device<cassette_image_device> m_cass;
	required_ioport m_io_dsw;
	required_ioport m_io_fdc;
	required_ioport m_io_k0f;
	required_ioport m_io_k300;
	required_ioport m_io_k301;
	required_ioport m_io_k310;
	required_ioport m_io_k311;
	required_ioport m_io_k320;
	required_ioport m_io_k321;
	required_ioport m_io_k330;
	required_ioport m_io_k331;
	required_ioport m_io_k340;
	required_ioport m_io_k341;
	required_ioport m_io_k350;
	required_ioport m_io_k351;
	required_ioport m_io_k360;
	required_ioport m_io_k361;
	required_ioport m_io_k370;
	required_ioport m_io_k371;
	required_ioport m_io_k380;
	required_ioport m_io_k390;
	required_ioport m_io_k3a0;
	required_ioport m_io_k3b0;
	required_ioport m_io_k0b;
	required_shared_ptr<UINT16> m_expansion;
public:
	required_device<palette_device> m_palette;
};

/*
d0,1,2 = joystick
d3     = cassette LED, low=on
d4,5,6 = audio select
d7     = cassette relay, low=on
*/
WRITE16_MEMBER( applix_state::analog_latch_w )
{
	data &= 0xff;
	if (data != m_analog_latch)
	{
		m_cass->change_state(
			(BIT(data,7)) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);

		m_analog_latch = data;
	}
}

WRITE16_MEMBER( applix_state::dac_latch_w )
{
	data &= 0xff;
	m_dac_latch = data;

	if ((m_analog_latch & 0x70) == 0) // right
		m_dacr->write_unsigned8(m_dac_latch);
	else
	if ((m_analog_latch & 0x70) == 0x10) // left
		m_dacl->write_unsigned8(m_dac_latch);
}

//cent = odd, video = even
WRITE16_MEMBER( applix_state::palette_w )
{
	offset >>= 4;
	if (ACCESSING_BITS_0_7)
	{
		m_cent_data_out->write(space, 0, data);
	}
	else
		m_palette_latch[offset] = (data >> 8) & 15;
}

WRITE16_MEMBER( applix_state::video_latch_w )
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
READ16_MEMBER( applix_state::applix_inputs_r )
{
	return m_io_dsw->read() | m_cass_data[2];
}

READ8_MEMBER( applix_state::applix_pb_r )
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
WRITE8_MEMBER( applix_state::applix_pa_w )
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
WRITE8_MEMBER( applix_state::applix_pb_w )
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
READ8_MEMBER( applix_state::port00_r )
{
	return (UINT8)m_data_or_cmd | ((UINT8)m_data << 1) | ((UINT8)m_buffer_empty << 2) | m_io_fdc->read();
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
READ8_MEMBER( applix_state::port08_r )
{
	return m_port08 | 3;
}

/*
d0 = /INUSE
d1 = /EJECT
d2-7 same as for port08_r
*/
WRITE8_MEMBER( applix_state::port08_w )
{
	m_port08 = data;
	membank("bank1")->set_entry(BIT(data, 6));

	floppy_image_device *floppy = nullptr;
	if (BIT(data, 2)) floppy = m_floppy0->get_device();
	if (BIT(data, 3)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 5));
	}
}

READ8_MEMBER( applix_state::port10_r )
{
	return 0;
}

WRITE8_MEMBER( applix_state::port10_w )
{
}

READ8_MEMBER( applix_state::port18_r )
{
	m_data = 0;
	return m_data_to_fdc;
}

WRITE8_MEMBER( applix_state::port18_w )
{
	m_data_from_fdc = data;
	m_buffer_empty = 0;
	m_fdc_cmd = BIT(offset, 2);
}

READ8_MEMBER( applix_state::port20_r )
{
	return 0;
}

WRITE8_MEMBER( applix_state::port20_w )
{
}

READ8_MEMBER( applix_state::port60_r )
{
	return 0;
}

WRITE8_MEMBER( applix_state::port60_w )
{
}

READ16_MEMBER( applix_state::fdc_stat_r )
{
	UINT8 data = 0;
	switch (offset)
	{
	case 0: data = (UINT8)m_buffer_empty^1; break;
	case 1: data = (UINT8)m_data^1; break;
	default: data = (UINT8)m_fdc_cmd; // case 2
	}
	return data << 7;
}

READ16_MEMBER( applix_state::fdc_data_r )
{
	m_buffer_empty = 1;
	return m_data_from_fdc;
}

WRITE16_MEMBER( applix_state::fdc_data_w )
{
	m_data_to_fdc = data;
	m_data = 1;
	m_data_or_cmd = 0;
}

WRITE16_MEMBER( applix_state::fdc_cmd_w )
{
	m_data_to_fdc = data;
	m_data = 1;
	m_data_or_cmd = 1;
}

static ADDRESS_MAP_START(applix_mem, AS_PROGRAM, 16, applix_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)
	AM_RANGE(0x000000, 0x3fffff) AM_RAM AM_SHARE("expansion") // Expansion
	AM_RANGE(0x400000, 0x47ffff) AM_RAM AM_MIRROR(0x80000) AM_SHARE("base") // Main ram
	AM_RANGE(0x500000, 0x51ffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x600000, 0x60007f) AM_WRITE(palette_w)
	AM_RANGE(0x600080, 0x6000ff) AM_WRITE(dac_latch_w)
	AM_RANGE(0x600100, 0x60017f) AM_WRITE(video_latch_w) //video latch (=border colour, high nybble; video base, low nybble) (odd)
	AM_RANGE(0x600180, 0x6001ff) AM_WRITE(analog_latch_w)
	//AM_RANGE(0x700000, 0x700007) z80-scc (ch b control, ch b data, ch a control, ch a data) on even addresses
	AM_RANGE(0x700080, 0x7000ff) AM_READ(applix_inputs_r)
	AM_RANGE(0x700100, 0x70011f) AM_MIRROR(0x60) AM_DEVREADWRITE8("via6522", via6522_device, read, write, 0xff00)
	AM_RANGE(0x700180, 0x700181) AM_MIRROR(0x7c) AM_DEVREADWRITE8("crtc", mc6845_device, status_r, address_w, 0xff00)
	AM_RANGE(0x700182, 0x700183) AM_MIRROR(0x7c) AM_DEVREADWRITE8("crtc", mc6845_device, register_r, register_w, 0xff00)
	AM_RANGE(0xffffc0, 0xffffc1) AM_READWRITE(fdc_data_r,fdc_data_w)
	//AM_RANGE(0xffffc2, 0xffffc3) AM_READWRITE(fdc_int_r,fdc_int_w) // optional
	AM_RANGE(0xffffc8, 0xffffcd) AM_READ(fdc_stat_r)
	AM_RANGE(0xffffd0, 0xffffd1) AM_WRITE(fdc_cmd_w)
	//600000, 6FFFFF  io ports and latches
	//700000, 7FFFFF  peripheral chips and devices
	//800000, FFC000  optional roms
	//FFFFC0, FFFFFF  disk controller board
ADDRESS_MAP_END

static ADDRESS_MAP_START( subcpu_mem, AS_PROGRAM, 8, applix_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_RAMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( subcpu_io, AS_IO, 8, applix_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x07) AM_READ(port00_r) //PORTR
	AM_RANGE(0x08, 0x0f) AM_READWRITE(port08_r,port08_w) //Disk select
	AM_RANGE(0x10, 0x17) AM_READWRITE(port10_r,port10_w) //IRQ
	AM_RANGE(0x18, 0x1f) AM_READWRITE(port18_r,port18_w) //data&command
	AM_RANGE(0x20, 0x27) AM_MIRROR(0x18) AM_READWRITE(port20_r,port20_w) //SCSI NCR5380
	AM_RANGE(0x40, 0x43) AM_MIRROR(0x1c) AM_DEVREADWRITE("fdc", wd1772_t, read, write) //FDC
	AM_RANGE(0x60, 0x63) AM_MIRROR(0x1c) AM_READWRITE(port60_r,port60_w) //anotherZ80SCC
ADDRESS_MAP_END

static ADDRESS_MAP_START( keytronic_pc3270_program, AS_PROGRAM, 8, applix_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("kbdcpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( keytronic_pc3270_io, AS_IO, 8, applix_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(internal_data_read, internal_data_write)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READWRITE(p1_read, p1_write)
	AM_RANGE(MCS51_PORT_P2, MCS51_PORT_P2) AM_READWRITE(p2_read, p2_write)
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READWRITE(p3_read, p3_write)
ADDRESS_MAP_END

// io priorities:
// 4 cassette
// 3 scc
// 2 via

/* Input ports */
static INPUT_PORTS_START( applix )
	PORT_START( "K0f" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_5)                                PORT_CHAR('5')                      /* 06 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_4)                                PORT_CHAR('4')                      /* 05 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_T)                                PORT_CHAR('T')                      /* 14 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_R)                                PORT_CHAR('R')                      /* 13 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_G)                                PORT_CHAR('G')                      /* 22 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F)                                PORT_CHAR('F')                      /* 21 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F7 (IRMA)")              /* 41 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?6a?")                   /* 6a */

	PORT_START( "K30_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_N)                                PORT_CHAR('N')                      /* 31 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_M)                                PORT_CHAR('M')                      /* 32 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_B)                                PORT_CHAR('B')                      /* 30 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_V)                                PORT_CHAR('V')                      /* 2f */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_C)                                PORT_CHAR('C')                      /* 2e */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_COMMA)                            PORT_CHAR(',')                      /* 33 */
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_6)                                PORT_CHAR('6')                      /* 07 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_7)                                PORT_CHAR('7')                      /* 08 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_Y)                                PORT_CHAR('Y')                      /* 15 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_U)                                PORT_CHAR('U')                      /* 16 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_H)                                PORT_CHAR('H')                      /* 23 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_J)                                PORT_CHAR('J')                      /* 24 */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K31_1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F7)                               PORT_CHAR(UCHAR_MAMEKEY(F7))        /* 37 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_F8)                               PORT_CHAR(UCHAR_MAMEKEY(F8))        /* 5f */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_LSHIFT)                           PORT_NAME("LShift")                 /* 2a */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("<")                      /* 70 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_Z)                                PORT_CHAR('Z')                      /* 2c */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_X)                                PORT_CHAR('X')                      /* 2d */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?6c?")                   /* 6c */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F9 (IRMA)")              /* 43 */

	PORT_START( "K32_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_9)                                PORT_CHAR('9')                      /* 0a */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_8)                                PORT_CHAR('8')                      /* 09 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_O)                                PORT_CHAR('O')                      /* 18 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_I)                                PORT_CHAR('I')                      /* 17 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_L)                                PORT_CHAR('L')                      /* 26 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_K)                                PORT_CHAR('K')                      /* 25 */
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_1)                                PORT_CHAR('1')                      /* 02 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_TILDE)                            PORT_CHAR('`')                      /* 29 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_Q)                                PORT_CHAR('Q')                      /* 10 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_TAB)                              PORT_CHAR(9)                        /* 0f */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_A)                                PORT_CHAR('A')                      /* 1e */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_CAPSLOCK)                         PORT_NAME("Caps")                   /* 3a */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?68?")                   /* 68 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F5 (IRMA)")              /* 3f */

	PORT_START( "K34_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_SLASH)                            PORT_CHAR('/')                      /* 35 */
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_RSHIFT)                           PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))    /* 36 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("Left")                   /* 56 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_STOP)                             PORT_CHAR('.')                      /* 34 */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K34_1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_2)                                PORT_CHAR('2')                      /* 02 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_3)                                PORT_CHAR('3')                      /* 03 */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_W)                                PORT_CHAR('W')                      /* 11 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_E)                                PORT_CHAR('E')                      /* 12 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_S)                                PORT_CHAR('S')                      /* 1f */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_D)                                PORT_CHAR('D')                      /* 20 */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?67?")                   /* 67 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F4 (IRMA)")              /* 3e */

	PORT_START( "K35_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_0)                                PORT_CHAR('0')                      /* 0b */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_MINUS)                            PORT_CHAR('-')                      /* 0c */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_P)                                PORT_CHAR('P')                      /* 19 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_OPENBRACE)                        PORT_CHAR('[')                      /* 1a */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_COLON)                            PORT_CHAR(';')                      /* 27 */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_QUOTE)                            PORT_CHAR('\'')                     /* 28 */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "K35_1" )
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("?66?")                   /* 66 */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )                                                       PORT_NAME("F3 (IRMA)")              /* 3d */

	PORT_START( "K36_0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_BACKSPACE)                        PORT_CHAR(8)                        /* 0e */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_EQUALS)                           PORT_CHAR('=')                      /* 0d */
	PORT_BIT( 0x14, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_ENTER)                            PORT_CHAR(13)                       /* 1c */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_BACKSLASH)                        PORT_CHAR('\\')                     /* 2b */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE(KEYCODE_CLOSEBRACE)                       PORT_CHAR(']')                      /* 1b */
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
	UINT8* ROM = memregion("maincpu")->base();
	memcpy(m_expansion, ROM, 8);
	membank("bank1")->set_entry(0);
	m_p3 = 0xff;
	m_last_write_addr = 0;
	m_maincpu->reset();
}

FLOPPY_FORMATS_MEMBER( applix_state::floppy_formats )
	FLOPPY_APPLIX_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( applix_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END


PALETTE_INIT_MEMBER(applix_state, applix)
{ // shades need to be verified - the names on the right are from the manual
	const UINT8 colors[16*3] = {
	0x00, 0x00, 0x00,   //  0 Black
	0x40, 0x40, 0x40,   //  1 Dark Grey
	0x00, 0x00, 0x80,   //  2 Dark Blue
	0x00, 0x00, 0xff,   //  3 Mid Blue
	0x00, 0x80, 0x00,   //  4 Dark Green
	0x00, 0xff, 0x00,   //  5 Green
	0x00, 0xff, 0xff,   //  6 Blue Grey
	0x00, 0x7f, 0x7f,   //  7 Light Blue
	0x7f, 0x00, 0x00,   //  8 Dark Red
	0xff, 0x00, 0x00,   //  9 Red
	0x7f, 0x00, 0x7f,   // 10 Dark Violet
	0xff, 0x00, 0xff,   // 11 Violet
	0x7f, 0x7f, 0x00,   // 12 Brown
	0xff, 0xff, 0x00,   // 13 Yellow
	0xbf, 0xbf, 0xbf,   // 14 Light Grey
	0xff, 0xff, 0xff }; // 15 White

	UINT8 r, b, g, i, color_count = 0;

	for (i = 0; i < 48; color_count++)
	{
		r = colors[i++]; g = colors[i++]; b = colors[i++];
		palette.set_pen_color(color_count, rgb_t(r, g, b));
	}
}


void applix_state::video_start()
{
}

MC6845_UPDATE_ROW( applix_state::crtc_update_row )
{
// The display is bitmapped. 2 modes are supported here, 320x200x16 and 640x200x4.
// Need to display a border colour.
// There is a monochrome mode, but no info found as yet.
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 i;
	UINT16 chr,x;
	UINT32 mem, vidbase = (m_video_latch & 15) << 14, *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		mem = vidbase + ma + x + (ra<<12);
		chr = m_base[mem];

		if (BIT(m_pa, 3))
		// 640 x 200 x 4of16 mode
		{
			for (i = 0; i < 8; i++)
			{
				*p++ = palette[m_palette_latch[chr>>14]];
				chr <<= 2;
			}
		}
		else
		// 320 x 200 x 16 mode
		{
			for (i = 0; i < 4; i++)
			{
				*p++ = palette[chr>>12];
				*p++ = palette[chr>>12];
				chr <<= 4;
			}
		}
	}
}

WRITE_LINE_MEMBER( applix_state::vsync_w )
{
	m_via->write_ca2(state);
}

TIMER_DEVICE_CALLBACK_MEMBER(applix_state::cass_timer)
{
	/* cassette - turn 2500/5000Hz to a bit */
	m_cass_data[1]++;
	UINT8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

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

static MACHINE_CONFIG_START( applix, applix_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 7500000)
	MCFG_CPU_PROGRAM_MAP(applix_mem)
	MCFG_CPU_ADD("subcpu", Z80, XTAL_16MHz / 2) // Z80H
	MCFG_CPU_PROGRAM_MAP(subcpu_mem)
	MCFG_CPU_IO_MAP(subcpu_io)
	MCFG_CPU_ADD("kbdcpu", I8051, 11060250)
	MCFG_CPU_PROGRAM_MAP(keytronic_pc3270_program)
	MCFG_CPU_IO_MAP(keytronic_pc3270_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(applix_state, applix)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("dacl", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 2.0 )
	MCFG_SOUND_ADD("dacr", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 2.0 )
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)

	/* Devices */
	MCFG_MC6845_ADD("crtc", MC6845, "screen", 1875000) // 6545
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(applix_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(applix_state, vsync_w))

	MCFG_DEVICE_ADD("via6522", VIA6522, 0)
	MCFG_VIA6522_READPB_HANDLER(READ8(applix_state, applix_pb_r))
	// in CB1 kbd clk
	// in CA2 vsync
	// in CB2 kdb data
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(applix_state, applix_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(applix_state, applix_pb_w))
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE("maincpu", m68000_device, write_irq2))

	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(DEVWRITELINE("via6522", via6522_device, write_ca1))
	MCFG_CENTRONICS_BUSY_HANDLER(DEVWRITELINE("via6522", via6522_device, write_pa0))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_WD1772_ADD("fdc", XTAL_16MHz / 2) //connected to Z80H clock pin
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", applix_floppies, "35dd", applix_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", applix_floppies, "35dd", applix_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("applix_c", applix_state, cass_timer, attotime::from_hz(100000))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( applix )
	ROM_REGION16_BE(0x20000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v4.5a", "V4.5a")
	ROMX_LOAD( "1616osl.45a", 0x00000, 0x10000, CRC(9dfb3224) SHA1(5223833a357f90b147f25826c01713269fc1945f), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "1616osh.45a", 0x00001, 0x10000, CRC(951bd441) SHA1(e0a38c8d0d38d84955c1de3f6a7d56ce06b063f6), ROM_SKIP(1) | ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "v4.4a", "V4.4a")
	ROMX_LOAD( "1616osl.44a", 0x00000, 0x10000, CRC(4a1a90d3) SHA1(4df504bbf6fc5dad76c29e9657bfa556500420a6), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "1616osh.44a", 0x00001, 0x10000, CRC(ef619994) SHA1(ff16fe9e2c99a1ffc855baf89278a97a2a2e881a), ROM_SKIP(1) | ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "v4.3a", "V4.3a")
	ROMX_LOAD( "1616osl.43a", 0x00000, 0x10000, CRC(c09b9ff8) SHA1(c46f2a98470d2d09cf9f9eec0f4096ab762407b5), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "1616osh.43a", 0x00001, 0x10000, CRC(071a2505) SHA1(42c4cc6e3e78b6a5320f9d9c858fc9f4e6220857), ROM_SKIP(1) | ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(3, "v4.0c", "V4.0c")
	ROMX_LOAD( "1616osl.40c", 0x00000, 0x10000, CRC(6a517b5d) SHA1(e0f4eba0cb8d273ba681b9d2c6d4b1beff9ef325), ROM_SKIP(1) | ROM_BIOS(4) )
	ROMX_LOAD( "1616osh.40c", 0x00001, 0x10000, CRC(7851651f) SHA1(d7d329aa7fe9f4418de0cdf813b61e70243e0e77), ROM_SKIP(1) | ROM_BIOS(4) )
	ROM_SYSTEM_BIOS(4, "v3.0b", "V3.0b")
	ROMX_LOAD( "1616osl.30b", 0x00000, 0x10000, CRC(fb9198c3) SHA1(e0e7a1dd176c1cbed063df1c405821c261d48f3a), ROM_SKIP(1) | ROM_BIOS(5) )
	ROMX_LOAD( "1616osh.30b", 0x00001, 0x10000, CRC(a279e1d7) SHA1(3451b2cae87a9ccee5f579fd1d49cf52d9f97b83), ROM_SKIP(1) | ROM_BIOS(5) )
	ROM_SYSTEM_BIOS(5, "v2.4a", "V2.4a")
	ROMX_LOAD( "1616osl.24a", 0x00000, 0x08000, CRC(b155830b) SHA1(b32db6a06c8a3c544210ba9faba7c49497c504fb), ROM_SKIP(1) | ROM_BIOS(6) )
	ROMX_LOAD( "1616osh.24a", 0x00001, 0x08000, CRC(6d9fc0e0) SHA1(07111f46386494ed3f426c1e50308f0209587f06), ROM_SKIP(1) | ROM_BIOS(6) )

	ROM_REGION(0x18000, "subcpu", 0)
	ROM_LOAD( "1616ssdv.022", 0x0000, 0x8000, CRC(6d8e413a) SHA1(fc27d92c34f231345a387b06670f36f8c1705856) )

	ROM_REGION(0x20000, "user1", 0)
	ROM_LOAD( "ssdcromv.22",  0x0000, 0x8000, CRC(c85c47fb) SHA1(6f0bb3753fc0d74ee5901d71d05a74ec6a4a1d05) )
	ROM_LOAD( "ssddromv.14a", 0x8000, 0x8000, CRC(8fe2db78) SHA1(487484003aba4d8960101ced6a689dc81676235d) )

	ROM_REGION(0x2000, "kbdcpu", 0)
	ROM_LOAD( "14166.bin", 0x0000, 0x2000, CRC(1aea1b53) SHA1(b75b6d4509036406052157bc34159f7039cdc72e) )
ROM_END


DRIVER_INIT_MEMBER(applix_state, applix)
{
	UINT8 *RAM = memregion("subcpu")->base();
	membank("bank1")->configure_entries(0, 2, &RAM[0x8000], 0x8000);
}


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   CLASS         INIT    COMPANY          FULLNAME       FLAGS */
COMP( 1986, applix, 0,       0,     applix, applix, applix_state, applix, "Applix Pty Ltd", "Applix 1616", 0 )



/**************************************************** KEYBOARD MODULE *****************************************/

READ8_MEMBER( applix_state::internal_data_read )
{
	m_via->write_cb2( BIT(offset, 8) ); // data
	bool cp = !BIT(offset, 9);
	if (cp != m_cp)
	{
		m_cp = cp;
		if (cp)
			m_clock_count++;
	}
	if (m_clock_count > 2)
		m_via->write_cb1( cp );

	return 0xff;
}


WRITE8_MEMBER( applix_state::internal_data_write )
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
		case 0x30:
			m_p1_data = m_io_k300->read();
			break;
		case 0x31:
			m_p1_data = m_io_k310->read();
			break;
		case 0x32:
			m_p1_data = m_io_k320->read();
			break;
		case 0x33:
			m_p1_data = m_io_k330->read();
			break;
		case 0x34:
			m_p1_data = m_io_k340->read();
			break;
		case 0x35:
			m_p1_data = m_io_k350->read();
			break;
		case 0x36:
			m_p1_data = m_io_k360->read();
			break;
		case 0x37:
			m_p1_data = m_io_k370->read() | (m_io_k360->read() & 0x01);
			break;
		case 0x38:
			m_p1_data = m_io_k380->read();
			break;
		case 0x39:
			m_p1_data = m_io_k390->read();
			break;
		case 0x3a:
			m_p1_data = m_io_k3a0->read();
			break;
		case 0x3b:
			m_p1_data = m_io_k3b0->read();
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
		case 0x30:
			m_p1_data = m_io_k301->read();
			break;
		case 0x31:
			m_p1_data = m_io_k311->read();
			break;
		case 0x32:
			m_p1_data = m_io_k321->read();
			break;
		case 0x33:
			m_p1_data = m_io_k331->read();
			break;
		case 0x34:
			m_p1_data = m_io_k341->read();
			break;
		case 0x35:
			m_p1_data = m_io_k351->read();
			break;
		case 0x36:
			m_p1_data = m_io_k361->read();
			break;
		case 0x37:
			m_p1_data = m_io_k371->read();
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


READ8_MEMBER( applix_state::p1_read )
{
	return m_p1 & m_p1_data;
}


WRITE8_MEMBER( applix_state::p1_write )
{
	m_p1 = data;
}


READ8_MEMBER( applix_state::p2_read )
{
	return m_p2;
}


WRITE8_MEMBER( applix_state::p2_write )
{
	m_p2 = data;
}


READ8_MEMBER( applix_state::p3_read )
{
	UINT8 data = m_p3;

	data &= ~0x14;

	/* -INT0 signal */
	data |= 4;

	/* T0 signal */
	data |= 0;

	return data;
}


WRITE8_MEMBER( applix_state::p3_write )
{
	m_p3 = data;
}
