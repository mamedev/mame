// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        FK-1

        12/05/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/i8251.h"
#include "machine/ram.h"


class fk1_state : public driver_device
{
public:
	fk1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;

	DECLARE_WRITE8_MEMBER(fk1_ppi_1_a_w);
	DECLARE_WRITE8_MEMBER(fk1_ppi_1_b_w);
	DECLARE_WRITE8_MEMBER(fk1_ppi_1_c_w);
	DECLARE_READ8_MEMBER(fk1_ppi_1_a_r);
	DECLARE_READ8_MEMBER(fk1_ppi_1_b_r);
	DECLARE_READ8_MEMBER(fk1_ppi_1_c_r);
	DECLARE_WRITE8_MEMBER(fk1_ppi_2_a_w);
	DECLARE_WRITE8_MEMBER(fk1_ppi_2_c_w);
	DECLARE_READ8_MEMBER(fk1_ppi_2_b_r);
	DECLARE_READ8_MEMBER(fk1_ppi_2_c_r);
	DECLARE_WRITE8_MEMBER(fk1_ppi_3_a_w);
	DECLARE_WRITE8_MEMBER(fk1_ppi_3_b_w);
	DECLARE_WRITE8_MEMBER(fk1_ppi_3_c_w);
	DECLARE_READ8_MEMBER(fk1_ppi_3_a_r);
	DECLARE_READ8_MEMBER(fk1_ppi_3_b_r);
	DECLARE_READ8_MEMBER(fk1_ppi_3_c_r);
	DECLARE_WRITE_LINE_MEMBER(fk1_pit_out0);
	DECLARE_WRITE_LINE_MEMBER(fk1_pit_out1);
	DECLARE_WRITE_LINE_MEMBER(fk1_pit_out2);
	DECLARE_WRITE8_MEMBER(fk1_intr_w);
	DECLARE_READ8_MEMBER(fk1_bank_ram_r);
	DECLARE_READ8_MEMBER(fk1_bank_rom_r);
	DECLARE_WRITE8_MEMBER(fk1_disk_w);
	DECLARE_READ8_MEMBER(fk1_mouse_r);
	DECLARE_WRITE8_MEMBER(fk1_reset_int_w);
	UINT8 m_video_rol;
	UINT8 m_int_vector;
	virtual void machine_reset();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(vsync_callback);
	IRQ_CALLBACK_MEMBER(fk1_irq_callback);
};


/*
Port A:
        Printer
Port B:
        Keyboard
Port C:
    READING :
        7 - / OBF(buffer overflow for the printer)
        6 - INTE printer
        5 - ERROR printer
        4 - /PEND, lack of paper
        3 - INTR printer
        2 - INTE keyboard
        1 - IBF (data from the keyboard)
        0 - INTR keyboard
    WRITING :
        6 - INTE printer
        2 - INTE keyboard
 */

WRITE8_MEMBER( fk1_state::fk1_ppi_1_a_w )
{
//  logerror("fk1_ppi_1_a_w %02x\n",data);
}

WRITE8_MEMBER( fk1_state::fk1_ppi_1_b_w )
{
//  logerror("fk1_ppi_1_b_w %02x\n",data);
}

WRITE8_MEMBER( fk1_state::fk1_ppi_1_c_w )
{
	//logerror("fk1_ppi_1_c_w %02x\n",data);
}

READ8_MEMBER( fk1_state::fk1_ppi_1_a_r )
{
	//logerror("fk1_ppi_1_a_r\n");
	return 0xff;
}

READ8_MEMBER( fk1_state::fk1_ppi_1_b_r )
{
//  logerror("fk1_ppi_1_b_r\n");
	return 0;
}

READ8_MEMBER( fk1_state::fk1_ppi_1_c_r )
{
//  logerror("fk1_ppi_1_c_r\n");
	return 0;
}

/*
Port A:
    Writing data to disk
Port B:
    Reading data from disk
Port C:
    READING:
        7 - / OF A data write to disk,
        6 - INTE A,
        5 - Select the drive A, B,
        4 - Not connected
        3 - INTR A
        2 - INTE B read data,
        1 - IBF B read data,
        0 - INTR B

    WRITING:
        6 - INTE A - reading data
        2 - INTE B - writing data
*/

WRITE8_MEMBER( fk1_state::fk1_ppi_2_a_w )
{
//  logerror("write to disk %02x\n",data);
}

WRITE8_MEMBER( fk1_state::fk1_ppi_2_c_w )
{
//  logerror("fk1_ppi_2_c_w %02x\n",data);
}

READ8_MEMBER( fk1_state::fk1_ppi_2_b_r )
{
//  logerror("read from disk\n");
	return 0;
}

READ8_MEMBER( fk1_state::fk1_ppi_2_c_r )
{
//  logerror("fk1_ppi_2_c_r\n");
	return 0;
}


/*

Port A:
    6 - / disk, authorization disk operations,
    3 - INTE MOUSE, permit suspension from mouse,
    2 - INTE RTC from the system clock of 50 Hz,
    1 and 0 - to set the type for hours write to the disk (01 index, 11 address mark, mark the date, 00 other).

Port B:
    Video ROL register
Port C

    READING:
    7 - INDEX ( index mark on the disk)
    6 - I do not know,
    5 - WRITE_PROTECT the protected disk,
    4 - TRAC_00 as a sign of trace

    WRITING:
    3 - HEAD LOAD
    2 - TRACK_43
    1 - DIRC - direction to set the direction of stepping disk
    0 - STEP, move disk (0 1 .. 0.).

*/
WRITE8_MEMBER( fk1_state::fk1_ppi_3_a_w )
{
//  logerror("fk1_ppi_3_a_w %02x\n",data);
}

WRITE8_MEMBER( fk1_state::fk1_ppi_3_b_w )
{
	m_video_rol = data;
}

WRITE8_MEMBER( fk1_state::fk1_ppi_3_c_w )
{
//  logerror("fk1_ppi_3_c_w %02x\n",data);
}

READ8_MEMBER( fk1_state::fk1_ppi_3_a_r )
{
//  logerror("fk1_ppi_3_a_r\n");
	return 0;
}

READ8_MEMBER( fk1_state::fk1_ppi_3_b_r )
{
	return m_video_rol;
}

READ8_MEMBER( fk1_state::fk1_ppi_3_c_r )
{
//  logerror("fk1_ppi_3_c_r\n");
	return 0;
}

WRITE_LINE_MEMBER( fk1_state::fk1_pit_out0 )
{
	// System time
	logerror("WRITE_LINE_MEMBER(fk1_pit_out0)\n");
}

WRITE_LINE_MEMBER( fk1_state::fk1_pit_out1 )
{
	// Timeout for disk operation
	logerror("WRITE_LINE_MEMBER(fk1_pit_out1)\n");
}

WRITE_LINE_MEMBER( fk1_state::fk1_pit_out2 )
{
	// Overflow for disk operations
	logerror("WRITE_LINE_MEMBER(fk1_pit_out2)\n");
}

/*
    0 no interrupt allowed,
    1 allowed INTR-7,
    2 allowed INTR-7 and INTR-6,
    8 any interruption allowed.
*/

WRITE8_MEMBER( fk1_state::fk1_intr_w )
{
	logerror("fk1_intr_w %02x\n",data);
}

READ8_MEMBER( fk1_state::fk1_bank_ram_r )
{
	address_space &space_mem = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	space_mem.install_write_bank(0x0000, 0x3fff, "bank1");
	membank("bank1")->set_base(ram);
	membank("bank2")->set_base(ram + 0x4000);
	return 0;
}

READ8_MEMBER( fk1_state::fk1_bank_rom_r )
{
	address_space &space_mem = m_maincpu->space(AS_PROGRAM);
	space_mem.unmap_write(0x0000, 0x3fff);
	membank("bank1")->set_base(memregion("maincpu")->base());
	membank("bank2")->set_base(m_ram->pointer() + 0x10000);
	return 0;
}

/*
    4 - FORMAT authorization
    3 - READ_ADDRESS_MARK
    2 - READ_DATA_MARK
    1 - WRITE
    0 - READ
    Functions are allowed in one.
*/

WRITE8_MEMBER( fk1_state::fk1_disk_w )
{
//  logerror("fk1_disk_w %02x\n",data);
}

/*
7 and 6 - 1 to connected mouse and 0 if not connected,
5 - / T2, right-click
4 - / T1, left-click
3 - / BY, Y-axis
2 - / AY, Y-axis
1 - / BX, X-axis
0 - / AX, X-axis
*/

READ8_MEMBER( fk1_state::fk1_mouse_r )
{
//  logerror("fk1_mouse_r\n");
	return 0;
}

/*Write to port 70 resets the interrupt from the system clock of 50 Hz. */

WRITE8_MEMBER( fk1_state::fk1_reset_int_w )
{
	logerror("fk1_reset_int_w\n");
}

static ADDRESS_MAP_START(fk1_mem, AS_PROGRAM, 8, fk1_state)
	AM_RANGE(0x0000, 0x3fff) AM_RAMBANK("bank1")
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("bank2")
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank3")
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank4")
ADDRESS_MAP_END

static ADDRESS_MAP_START(fk1_io, AS_IO, 8, fk1_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x00, 0x03 ) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE( 0x10, 0x13 ) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE( 0x20, 0x23 ) AM_DEVREADWRITE("ppi8255_2", i8255_device, read, write)
	AM_RANGE( 0x30, 0x30 ) AM_READWRITE(fk1_bank_ram_r,fk1_intr_w)
	AM_RANGE( 0x40, 0x40 ) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE( 0x41, 0x41 ) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE( 0x50, 0x50 ) AM_READWRITE(fk1_bank_rom_r,fk1_disk_w)
	AM_RANGE( 0x60, 0x63 ) AM_DEVREADWRITE("ppi8255_3", i8255_device, read, write)
	AM_RANGE( 0x70, 0x70 ) AM_READWRITE(fk1_mouse_r,fk1_reset_int_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( fk1 )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('\xA4')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(fk1_state::keyboard_callback)
{
	if (ioport("LINE0")->read())
	{
		m_int_vector = 6;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

/*
7 ? DATA_READY
6 ? TIMEOUT
5 ? OVERFLOW
4 ? RTC_50HZ
3 ? MOUSE
2 ? SIO
1 ? KEYBOARD
0 ? PRINTER
*/

IRQ_CALLBACK_MEMBER(fk1_state::fk1_irq_callback)
{
	logerror("IRQ %02x\n", m_int_vector*2);
	return m_int_vector * 2;
}

TIMER_DEVICE_CALLBACK_MEMBER(fk1_state::vsync_callback)
{
	m_int_vector = 3;
	m_maincpu->set_input_line(0, HOLD_LINE);
}


void fk1_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	space.unmap_write(0x0000, 0x3fff);
	membank("bank1")->set_base(memregion("maincpu")->base()); // ROM
	membank("bank2")->set_base(ram + 0x10000); // VRAM
	membank("bank3")->set_base(ram + 0x8000);
	membank("bank4")->set_base(ram + 0xc000);
}

UINT32 fk1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 code;
	int y, x, b;
	UINT8 *ram = m_ram->pointer();

	for (x = 0; x < 64; x++)
	{
		for (y = 0; y < 256; y++)
		{
			code = ram[x * 0x100 + ((y + m_video_rol) & 0xff) + 0x10000];
			for (b = 0; b < 8; b++)
			{
				bitmap.pix16(y, x*8+b) =  ((code << b) & 0x80) ? 1 : 0;
			}
		}
	}
	return 0;
}

static MACHINE_CONFIG_START( fk1, fk1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_8MHz / 2)
	MCFG_CPU_PROGRAM_MAP(fk1_mem)
	MCFG_CPU_IO_MAP(fk1_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(fk1_state,fk1_irq_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(fk1_state, screen_update)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(50)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(fk1_state, fk1_pit_out0))
	MCFG_PIT8253_CLK1(1000000)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(fk1_state, fk1_pit_out1))
	MCFG_PIT8253_CLK2(0)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(fk1_state, fk1_pit_out2))

	MCFG_DEVICE_ADD("ppi8255_1", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(fk1_state, fk1_ppi_1_a_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(fk1_state, fk1_ppi_1_a_w))
	MCFG_I8255_IN_PORTB_CB(READ8(fk1_state, fk1_ppi_1_b_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(fk1_state, fk1_ppi_1_b_w))
	MCFG_I8255_IN_PORTC_CB(READ8(fk1_state, fk1_ppi_1_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(fk1_state, fk1_ppi_1_c_w))

	MCFG_DEVICE_ADD("ppi8255_2", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(fk1_state, fk1_ppi_2_a_w))
	MCFG_I8255_IN_PORTB_CB(READ8(fk1_state, fk1_ppi_2_b_r))
	MCFG_I8255_IN_PORTC_CB(READ8(fk1_state, fk1_ppi_2_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(fk1_state, fk1_ppi_2_c_w))

	MCFG_DEVICE_ADD("ppi8255_3", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(fk1_state, fk1_ppi_3_a_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(fk1_state, fk1_ppi_3_a_w))
	MCFG_I8255_IN_PORTB_CB(READ8(fk1_state, fk1_ppi_3_b_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(fk1_state, fk1_ppi_3_b_w))
	MCFG_I8255_IN_PORTC_CB(READ8(fk1_state, fk1_ppi_3_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(fk1_state, fk1_ppi_3_c_w))

	/* uart */
	MCFG_DEVICE_ADD("uart", I8251, 0)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("80K") // 64 + 16

	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard_timer", fk1_state, keyboard_callback, attotime::from_hz(24000))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("vsync_timer", fk1_state, vsync_callback, attotime::from_hz(50))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( fk1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "orig", "Original BIOS" )
	ROMX_LOAD( "fk1.u65",      0x0000, 0x0800, CRC(145561f8) SHA1(a4eb17d773e51b34620c508b6cebcb4531ae99c2), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "diag", "Diag BIOS" )
	ROMX_LOAD( "fk1-diag.u65", 0x0000, 0x0800, CRC(e0660ae1) SHA1(6ad609049b28f27126af0a8a6224362351073dee), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT COMPANY   FULLNAME       FLAGS */
COMP( 1989, fk1,    0,      0,       fk1,       fk1, driver_device,      0,  "Statni statek Klicany", "FK-1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
