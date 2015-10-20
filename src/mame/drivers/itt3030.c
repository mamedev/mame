// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/***************************************************************************

    ITT 3030


    ToDo:
    - Check Beeper
    - finish hooking up keyboard
    - According to the manual, the keyboard is based on a 8278 ... it's nowhere to be found. The keyboard / video card has a 8741 instead of which a ROM dump exists
    - serial port
    - daisy chain
    - ...


    CPU Board, all ICs shown:

    |-----------------------------------------------------------------|
    |                                                                 |
    |    74LS640N          Z80_Combo            74LS138N              |
    |                                                       74LS00N   |
    |    74LS240N                               74LS74AN              |
    |                                                       74LS00N   |
    |C   74LS240N          Z80_CPU              74LS240N             C|
    |N                                                      74LS74AN N|
    |1   74LS241N                               74LS240N             2|
    |                      ROM_1      74LS20N               74LS38N   |
    |    74LS240N                               74LS240N              |
    |                                 74LS04N               74LS02N   |
    |    74LS138N                               74LS74AN              |
    |                                                       74LS175N  |
    |    75154N        74LS156N                 74LS00N               |
    |                                                       74LS123N  |
    |    75150P 75150P 74LS175N   X1  74LS00N   74LS132N              |
    |-----------------------------------------------------------------|

    Z80_Combo:  Mostek MK3886 Z80 Combo Chip, Serial, Timer, 256 bytes RAM, Interrupt Controller
    Z80_CPU:    Zilog Z80A CPU
    ROM_1:      NEC D2716D marked "BOOTV1.2"
    X1:         Crystal 4,194 MHz
    CN1:        Bus Connector
    CN2:        Memory Board Connector

----------------------------------------------------------------------------------

    Video / Keyboard Combination board, all ICs shown:

    |-----------------------------------------------------------------|
    |                                                                 |
    |         X1     74276N      MCU_1                  74LS85N       |
    |                                                                 |
    |    74LS138N    74LS240                            74LS240N      |
    |                            75LS257AN  74LS166AN                 |
    |    74LS08N     74LS85N                            74LS241N      |
    |                            75LS257AN  ROM_1                    C|
    |    74LS132N    74LS32N                            74LS240N     N|
    |                            75LS257AN                           1|
    |    74LS10N     74LS08N                            74LS240N      |
    |                            75LS257AN  RAM_1                     |
    |    74LS163AN   74LS173AN                          74LS374N      |
    |                                                                 |
    |    74LS86N     74LS240N                           74LS640N      |
    |                            Video_1                              |
    |    74LS74AN    74LS240N                           74LS640N      |
    |-----------------------------------------------------------------|

    X1:     Crystal 6 MHz
    MCU_1:  NEC D8741AD marked "V1.1 3030"
    ROM_1:  MBM 2716 marked "GB 136-0"
    RAM_1:  NEC D4016D
    Video_1 Video-IC SND5027E, compatible with TMS9927

----------------------------------------------------------------------------------

    Floppy Controller board, all ICs shown

    |-----------------------------------------------------------------|
    |                                                                 |
    |  X1   74LS51N    F    74LS74AN   74LS02N        MC4044P         |
    |                  D                        567                   |
    |     74LS04N      C    74LS00N    74LS01N  :::   MC4024P         |
    |                                                                 |
    |   74LS00N        1    74LS74AN   74LS74AN       74LS14N        C|
    |                  7                                             N|
    |  74LS240N        9    74LS161N   74LS393N   74LS74AN           1|
    |                  1                                              |
    |   74LS132N            74LS14N    74LS14N    74LS374N            |
    |                                                                 |
    |    74LS123N   74LS04N  74LS163N   74LS14N    74LS241N           |
    |                                                                 |
    |     74LS393N   74LS138  74LS175N   74LS85N    74LS645N          |
    |                                                                 |
    |-----------------------------------------------------------------|

    X1:     Crystal 8 MHz
    FDC:    Siemens SAB1791-02P
    567:    Jumper Pad (emtpy)

----------------------------------------------------------------------------------

    256K RAM board, all ICs shown:

    |-----------------------------------------------------------------|
    |                                                                 |
    |   HM4864P   HM4864P   HM4864P   HM4864P       74LS245N          |
    |                                                                 |
    |   HM4864P   HM4864P   HM4864P   HM4864P       P     74LS14N     |
    |                                               R                 |
    |   HM4864P   HM4864P   HM4864P   HM4864P       M     74LS00N     |
    |                                                                C|
    |   HM4864P   HM4864P   HM4864P   HM4864P       AM         A     N|
    |                                               29         M     1|
    |   HM4864P   HM4864P   HM4864P   HM4864P       66         2      |
    |                                               PC         9      |
    |   HM4864P   HM4864P   HM4864P   HM4864P                  6      |
    |                                               AM         4      |
    |   HM4864P   HM4864P   HM4864P   HM4864P       29         8      |
    |                                               66         P      |
    |   HM4864P   HM4864P   HM4864P   HM4864P       PC         C      |
    |                                                      SN7474N    |
    |-----------------------------------------------------------------|

    PRM:    N82S129F 1K Bipolar PROM
            AM2966PC: Octal Dynamic Memory Drivers with Three-State Outputs
            AM29648PC
    CN1:    Connector to CN2 of Z80 CPU card

----------------------------------------------------------------------------------

    Parallel I/O board, all ICs shown:

    |-------------------------------------|                                                                 |
    |                                     |
    |  74   74                            |
    |  LS   LS        Z80A PIO            |
    |  00   14                            |
    |   N    N                            |
    |                                     |
    |                                     |
    |  74   74        74   74   D4   74   |
    |  LS   LS        LS   LS   I3   LS   |
    |  13   14        24   85   P2   64   |
    |  2N    N        1N    N    1   0N   |
    |                                     |
    |             CN1                     |
    |                                     |
    |             74LS00N                 |
    |-------------------------------------|

    CN1: Bus connector
    DIP: 4x DIP current setting: off-on-on-off, sets the address for the parallel port

----------------------------------------------------------------------------------

Beeper Circuit, all ICs shown:

    |---------------------------|                                                                 |
    |                           |
    |   BEEP       74LS132N     |
    | R1                        |
    |              74LS14N      |
    |                           |
    |   74LS132N   74LS193N     |
    |                           |
    |   74LS74AN   74LS165N     |
    |            CN1            |
    |---------------------------|

    CN1: Connector to mainboard
    R1:  looks like a potentiometer
    BEEP: Beeper ... touted in the manual as "Hupe" ... i.e. "horn" :)

----------------------------------------------------------------------------------

    Other boards and extensions mentioned in the manual:
    - S100 bus adapter board
    - IEEE 488 bus adapter board
    - 64K memory board
    - 8086 CPU board
    - external harddisk
    - TV adapter B/W (TV, Save/Load from Audio Cassette) with PROM/RAM/BASIC-Module with 16K or 32K RAM
    - TV adapter color with connection to Video / Keyboard combination card
    - Monitor adapters B/W and color
    - Video / Keyboard interface 2 with grayscale, 8 colors, loadable character set, blinking
    - Graphics Adapter with 16 colours, hi-res 512x256 pixels
    - RTC
    - Arithmetics chip

***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/wd_fdc.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "formats/itt3030_dsk.h"
#include "video/tms9927.h"          //Display hardware
#include "sound/beep.h"
#include "cpu/mcs48/mcs48.h"        //Keyboard MCU ... talks to the 8278 on the keyboard circuit

#define MAIN_CLOCK XTAL_4.194MHz

class itt3030_state : public driver_device
{
public:
	itt3030_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_kbdmcu(*this, "kbdmcu")
		, m_ram(*this, "mainram")
		, m_crtc(*this, "crt5027")
		, m_48kbank(*this, "lowerbank")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_beep(*this, "beeper")
		, m_keyrow1(*this, "ROW1")
		, m_keyrow2(*this, "ROW2")
		, m_keyrow3(*this, "ROW3")
		, m_keyrow4(*this, "ROW4")
		, m_keyrow5(*this, "ROW5")
		, m_keyrow6(*this, "ROW6")
		, m_keyrow7(*this, "ROW7")
		, m_keyrow8(*this, "ROW8")
		, m_keyrow9(*this, "ROW9")
		, m_keyrow10(*this, "ROW10")
		, m_keyrow11(*this, "ROW11")
		, m_keyrow12(*this, "ROW12")
		, m_keyrow13(*this, "ROW13")
		, m_keyrow14(*this, "ROW14")
		, m_keyrow15(*this, "ROW15")
		, m_keyrow16(*this, "ROW16")
		, m_vram(*this, "vram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<i8041_device> m_kbdmcu;
	required_device<ram_device> m_ram;
	required_device<crt5027_device> m_crtc;
	required_device<address_map_bank_device> m_48kbank;
	required_device<fd1791_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<beep_device> m_beep;

	required_ioport m_keyrow1, m_keyrow2, m_keyrow3, m_keyrow4, m_keyrow5, m_keyrow6, m_keyrow7, m_keyrow8, m_keyrow9;
	required_ioport m_keyrow10, m_keyrow11, m_keyrow12, m_keyrow13, m_keyrow14, m_keyrow15, m_keyrow16;

	// shared pointers
	required_shared_ptr<UINT8> m_vram;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
public:

	DECLARE_READ8_MEMBER(vsync_r);
	DECLARE_READ8_MEMBER(i8078_r);
	DECLARE_WRITE8_MEMBER(i8078_w);
	DECLARE_WRITE8_MEMBER( beep_w );
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(bankl_r);
	DECLARE_WRITE8_MEMBER(bankl_w);
	DECLARE_READ8_MEMBER(bankh_r);
	DECLARE_WRITE8_MEMBER(bankh_w);
	DECLARE_READ8_MEMBER(kbd_fifo_r);
	DECLARE_READ8_MEMBER(kbd_matrix_r);
	DECLARE_WRITE8_MEMBER(kbd_matrix_w);
	DECLARE_READ8_MEMBER(fdc_r);
	DECLARE_WRITE8_MEMBER(fdc_w);
	DECLARE_READ8_MEMBER(fdc_stat_r);
	DECLARE_WRITE8_MEMBER(fdc_cmd_w);
	DECLARE_FLOPPY_FORMATS(itt3030_floppy_formats);

	DECLARE_WRITE_LINE_MEMBER(fdcirq_w);
	DECLARE_WRITE_LINE_MEMBER(fdcdrq_w);
	DECLARE_WRITE_LINE_MEMBER(fdchld_w);

private:
	UINT8 m_unk;
	UINT8 m_bank;
	UINT8 m_kbdrow, m_kbdcol, m_kbdclk, m_kbdread;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	floppy_image_device *m_curfloppy;
	bool m_fdc_irq, m_fdc_drq, m_fdc_hld;
	floppy_connector *m_con1, *m_con2, *m_con3;
	bool m_keyskip;
};

void itt3030_state::video_start()
{
	m_unk = 0x80;
}

READ8_MEMBER(itt3030_state::vsync_r)
{
	UINT8 ret = 0;

	if (machine().first_screen()->vblank())
	{
		ret |= 0xc0;    // set both bits 6 and 7 if vblank
	}

	if (machine().first_screen()->hblank())
	{
		ret |= 0x80;    // set only bit 7 if hblank
	}

	return ret;
}

// TODO: make this a correct device once we figure out how everything goes together in this system
READ8_MEMBER(itt3030_state::i8078_r)
{
	if (offset)
	{
//      printf("Read 8078 status\n");
		if (!m_keyskip)
		{
			return 0x10;
		}
	}
	else
	{
//      printf("Read 8078 data\n");
		if (!m_keyskip)
		{
			m_keyskip = true;   // don't keep reporting the 'b' key, just the once to trick the boot ROM
			return 0x0c;
		}
	}

	return 0;
}

WRITE8_MEMBER(itt3030_state::i8078_w)
{
	#if 0
	if (offset)
	{
		printf("%02x to 8078 command\n", data);
	}
	else
	{
		printf("%02x to 8078 data\n", data);
	}
	#endif
}

WRITE8_MEMBER( itt3030_state::beep_w )
{
	m_beep->set_state(data&0x32);
}

WRITE8_MEMBER(itt3030_state::bank_w)
{
	int bank = 0;
	m_bank = data>>4;

	if (!(m_bank & 1)) // bank 8
	{
		bank = 8;
	}
	else
	{
		bank = m_bank >> 1;
	}

//  printf("bank_w: new value %02x, m_bank %x, bank %x\n", data, m_bank, bank);

	m_48kbank->set_bank(bank);
}

READ8_MEMBER(itt3030_state::bankl_r)
{
	return m_ram->read(offset);
}

WRITE8_MEMBER(itt3030_state::bankl_w)
{
	m_ram->write(offset, data);
}

READ8_MEMBER(itt3030_state::bankh_r)
{
	return m_ram->read(((m_bank>>1)*0x10000) + offset + 0xc000);
}

WRITE8_MEMBER(itt3030_state::bankh_w)
{
	m_ram->write(((m_bank>>1)*0x10000) + offset + 0xc000, data);
}

UINT32 itt3030_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for(int y = 0; y < 24; y++ )
	{
		for(int x = 0; x < 80; x++ )
		{
			UINT8 code = m_vram[x + y*128];
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,  code , 0, 0,0, x*8,y*16);
		}
	}

	return 0;
}

WRITE_LINE_MEMBER(itt3030_state::fdcirq_w)
{
	m_fdc_irq = state;
}

#include "debugger.h"

WRITE_LINE_MEMBER(itt3030_state::fdcdrq_w)
{
	m_fdc_drq = state;
}

WRITE_LINE_MEMBER(itt3030_state::fdchld_w)
{
	m_fdc_hld = state;
}

/*
    7 Data Request (DRQ - inverted 1791-Signal)
    6 Interrupt Request (INTRQ - 1791-Signal)
    5 Head Load (HLD - inverted 1791-Signal)
    4 Ready 3 (Drive 3 ready)
    3 Ready 2 (Drive 2 ready)
    2 Ready l (Drive 1 ready)
    1 Write protect (the disk in the selected drive is write protected)
    0 HLT (Halt signal during head load and track change)
*/
READ8_MEMBER(itt3030_state::fdc_stat_r)
{
	UINT8 res = 0;
	floppy_image_device *floppy1 = m_con1 ? m_con1->get_device() : 0;
	floppy_image_device *floppy2 = m_con2 ? m_con2->get_device() : 0;
	floppy_image_device *floppy3 = m_con3 ? m_con3->get_device() : 0;

	res = m_fdc_drq ? 0x80 : 0x00;
	res |= m_fdc_irq ? 0x40 : 0x00;
	res |= m_fdc_hld ? 0x00 : 0x20;
	if (floppy3) res |= floppy3->ready_r() ? 0x10 : 0;
	if (floppy2) res |= floppy2->ready_r() ? 0x08 : 0;
	if (floppy1) res |= floppy1->ready_r() ? 0x04 : 0;
	if (m_curfloppy) res |= m_curfloppy->wpt_r() ? 0x02 : 0;

	return res;
}

/* As far as we can tell, the mess of ttl de-inverts the bus */
READ8_MEMBER(itt3030_state::fdc_r)
{
	return m_fdc->gen_r(offset) ^ 0xff;
}

WRITE8_MEMBER(itt3030_state::fdc_w)
{
	m_fdc->gen_w(offset, data ^ 0xff);
}

/*
    7 SEL1 - Select drive 1
    6 SEL2 - Select drive 2
    5 SEL3 - Select drive 3
    4 MOTOR - Motor on
    3 DOOR - Drive door lock drives 1 + 2 (not possible with all drives)
    2 SIDESEL - Select disk side
    1 KOMP - write comp on/off
    0 RG J - Change separator stage to read
*/
WRITE8_MEMBER(itt3030_state::fdc_cmd_w)
{
	floppy_image_device *floppy = NULL;

	logerror("%02x to fdc_cmd_w: motor %d side %d\n", data, (data & 0x10)>>4, (data & 4)>>2);

	// select drive
	if (data & 0x80)
	{
		floppy = m_con1 ? m_con1->get_device() : 0;
	}
	else if (data & 0x40)
	{
		floppy = m_con2 ? m_con2->get_device() : 0;
	}
	else if (data & 0x20)
	{
		floppy = m_con3 ? m_con3->get_device() : 0;
	}

	// selecting a new drive?
	if (floppy != m_curfloppy)
	{
		m_fdc->set_floppy(floppy);
		m_curfloppy = floppy;
	}

	if (floppy != NULL)
	{
		// side select
		floppy->ss_w((data & 4) ? 1 : 0);

		// motor control (active low)
		floppy->mon_w((data & 0x10) ? 0 : 1);
	}
}

// The lower 48K is switchable among the first 48K of each of 8 64K banks numbered 0-7 or "bank 8" which is the internal ROM and VRAM
// The upper 16K is always the top 16K of the selected bank 0-7, which allows bank/bank copies and such
// Port F6 bits 7-5 select banks 0-7, bit 4 enables bank 8

static ADDRESS_MAP_START( itt3030_map, AS_PROGRAM, 8, itt3030_state )
	AM_RANGE(0x0000, 0xbfff) AM_DEVICE("lowerbank", address_map_bank_device, amap8)
	AM_RANGE(0xc000, 0xffff) AM_READWRITE(bankh_r, bankh_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lower48_map, AS_PROGRAM, 8, itt3030_state )
	AM_RANGE(0x00000, 0x7ffff) AM_READWRITE(bankl_r, bankl_w)   // pages 0-7
	AM_RANGE(0x80000, 0x807ff) AM_ROM AM_REGION("maincpu", 0)   // begin "page 8"
	AM_RANGE(0x80800, 0x80fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x81000, 0x810ff) AM_RAM AM_MIRROR(0x100)  // only 256 bytes, but ROM also clears 11xx?
	AM_RANGE(0x83000, 0x83fff) AM_RAM AM_SHARE("vram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( itt3030_io, AS_IO, 8, itt3030_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x26) AM_DEVREADWRITE("crt5027", crt5027_device, read, write)
	AM_RANGE(0x30, 0x31) AM_READWRITE(i8078_r, i8078_w)
	AM_RANGE(0x32, 0x32) AM_WRITE(beep_w)
	AM_RANGE(0x35, 0x35) AM_READ(vsync_r)
	AM_RANGE(0x40, 0x40) AM_READ(kbd_fifo_r)
	AM_RANGE(0x50, 0x53) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0x54, 0x54) AM_READWRITE(fdc_stat_r, fdc_cmd_w)
	AM_RANGE(0xf6, 0xf6) AM_WRITE(bank_w)
ADDRESS_MAP_END

READ8_MEMBER(itt3030_state::kbd_fifo_r)
{
	return m_kbdmcu->upi41_master_r(space, 0);  // offset 0 is data, 1 is status
}

READ8_MEMBER(itt3030_state::kbd_matrix_r)
{
	return m_kbdread;
}

WRITE8_MEMBER(itt3030_state::kbd_matrix_w)
{
	ioport_port *ports[16] = { m_keyrow1, m_keyrow2, m_keyrow3, m_keyrow4, m_keyrow5, m_keyrow6, m_keyrow7, m_keyrow8, m_keyrow9,
								m_keyrow10, m_keyrow11, m_keyrow12, m_keyrow13, m_keyrow14, m_keyrow15, m_keyrow16 };
	int col_masks[8] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 };
	int tmp_read;

	m_kbdrow = data & 0xf;
	m_kbdcol = (data >> 4) & 0x7;

	if ((data & 0x80) && (!m_kbdclk))
	{
		tmp_read = ports[m_kbdrow]->read() & col_masks[m_kbdcol];
		m_kbdread = (tmp_read != 0) ? 1 : 0;
	}

	m_kbdclk = (data & 0x80) ? 1 : 0;
}

// Schematics say:
// Port 1 goes to the keyboard matrix.
// bits 0-3 select matrix rows, bits 4-6 choose column to read, bit 7 clocks the process (rising edge strobes the row, falling edge reads the data)
// T0 is the key matrix return
// Port 2 bit 2 is shown as "IRQ" on the schematics, and the code does a lot with it as well (debug?)
static ADDRESS_MAP_START( kbdmcu_io, AS_IO, 8, itt3030_state )
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(kbd_matrix_r)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(kbd_matrix_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_NOP AM_WRITENOP
ADDRESS_MAP_END

static INPUT_PORTS_START( itt3030 )
	PORT_START("ROW1")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("ROW2")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("ROW3")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')   // actually UK pound symbol
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("ROW4")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("ROW5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("ROW6")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("ROW7")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("ROW8")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_START("ROW9")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')

	PORT_START("ROW10")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')

	PORT_START("ROW11")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('@') PORT_CHAR('?')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('`')

	PORT_START("ROW12")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_F11) PORT_CHAR(27)
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('/') PORT_CHAR('\\')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_MAMEKEY(RSHIFT))

	PORT_START("ROW13")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(R)") PORT_CODE(KEYCODE_F12) PORT_CHAR('=')
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('~')
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_CHAR('^')    // PC doesn't have 3 keys to the right of L, so we sub DEL for the 3rd one
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("ROW14")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(CL)") PORT_CODE(KEYCODE_F13) PORT_CHAR(4)    // produces control-D always
	PORT_BIT(0x001e, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW15")
	PORT_BIT(0x001f, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ROW16")
	PORT_BIT(0x001f, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8, 16,              /* 8x16 characters */
	128,                /* 128 characters */
	1,                /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{7, 6, 5, 4, 3, 2, 1, 0},
	{ 0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* size of one char */
};

static GFXDECODE_START( itt3030 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END


void itt3030_state::machine_start()
{
	save_item(NAME(m_unk));
	save_item(NAME(m_bank));
	save_item(NAME(m_kbdrow));
	save_item(NAME(m_kbdcol));
	save_item(NAME(m_kbdclk));
	save_item(NAME(m_kbdread));

	m_kbdclk = 0;   // must be initialized here b/c mcs48_reset() causes write of 0xff to all ports
}

void itt3030_state::machine_reset()
{
	m_bank = 9;
	m_48kbank->set_bank(8);
	m_kbdread = 1;
	m_kbdrow = m_kbdcol = 0;
	m_kbdclk = 1;
	m_fdc_irq = m_fdc_drq = m_fdc_hld = 0;
	m_curfloppy = NULL;
	m_keyskip = false;

	// look up floppies in advance
	m_con1 = machine().device<floppy_connector>("fdc:0");
	m_con2 = machine().device<floppy_connector>("fdc:1");
	m_con3 = machine().device<floppy_connector>("fdc:2");
}

FLOPPY_FORMATS_MEMBER( itt3030_state::itt3030_floppy_formats )
	FLOPPY_ITT3030_FORMAT
FLOPPY_FORMATS_END


static SLOT_INTERFACE_START( itt3030_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_QD )
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( itt3030, itt3030_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(itt3030_map)
	MCFG_CPU_IO_MAP(itt3030_io)

	MCFG_CPU_ADD("kbdmcu", I8041, XTAL_6MHz)
	MCFG_CPU_IO_MAP(kbdmcu_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(250))
	MCFG_SCREEN_UPDATE_DRIVER(itt3030_state, screen_update)
	MCFG_SCREEN_SIZE(80*8, 24*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*8-1, 0, 24*16-1)
	MCFG_SCREEN_PALETTE("palette")

	/* devices */
	MCFG_DEVICE_ADD("lowerbank", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(lower48_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	MCFG_DEVICE_ADD("crt5027", CRT5027, XTAL_6MHz)
	MCFG_TMS9927_CHAR_WIDTH(16)

	MCFG_FD1791_ADD("fdc", XTAL_20MHz / 20)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(itt3030_state, fdcirq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(itt3030_state, fdcdrq_w))
	MCFG_WD_FDC_HLD_CALLBACK(WRITELINE(itt3030_state, fdchld_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", itt3030_floppies, "525dd", itt3030_state::itt3030_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", itt3030_floppies, "525dd", itt3030_state::itt3030_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", itt3030_floppies, "525dd", itt3030_state::itt3030_floppy_formats)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", itt3030)

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	/* internal ram */
	MCFG_RAM_ADD("mainram")
	MCFG_RAM_DEFAULT_SIZE("512K")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( itt3030 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bootv1.2.bin", 0x0000, 0x0800, CRC(90279d45) SHA1(a39a3f31f4f98980b1ef50805870837fbf72261d))
	ROM_REGION( 0x0800, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "gb136-0.bin", 0x0000, 0x0800, CRC(6a3895a8) SHA1(f3b977ffa2f54c346521c9ef034830de8f404621))
	ROM_REGION( 0x0400, "kbdmcu", ROMREGION_ERASE00 )
	ROM_LOAD( "8741ad.bin", 0x0000, 0x0400, CRC(cabf4394) SHA1(e5d1416b568efa32b578ca295a29b7b5d20c0def))
ROM_END

COMP( 1982, itt3030,  0,   0,  itt3030,  itt3030,  driver_device, 0,  "ITT RFA",      "ITT3030", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
