// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca
/*************************************************************************

  Cuatro en Linea.
  System I.
  1991, Compumatic

  Driver by David Haywood & Roberto Fresca.

**************************************************************************

  1x Z84C00HB6 CPU @ 8 MHz for program.
  1x Z84C00AB6 CPU @ 4 MHz for sound.
  1x AY-3-8910 A
  1x UMC UM487F (HCGA Controller)

  2x NEC D41464C (64K x 4-bit Dynamic NMOS RAM) for VRAM.
  1x UMC UM6264A (8K x 8-bit CMOS SRAM).

  2x 27512 EPROMS.
  1x X24C16P Serial EEPROM.

  1x GAL16V8AS

  1x ES2 CM3080 (unknown DIP-18 IC)
  1x ES2 9046 (unknown PLCC-84 IC)
  1x 8952 CM 32 (unknown DIP-40 IC)

  1x 16.0000 MHz crystal. ; Divided by 2 (through CM3080) for main CPU Z84C00HB6.
  1x 8.000 MHz crystal.   ; Divided by 2 for audio CPU Z84C00AB6.
  1x 14.31818 MHz crystal ; For HCGA controller.

  CN1: 1 x 8 connector.
  CN2: 1 x 8 connector.
  CN3: 2 x 5 connector.
  CN4: 2 x 5 connector.
  CN5: 2 x 5 connector.
  CN6: 2 x 28 Jamma connector.
  CN7: 1 x 20 connector.
  CN8: 1 x 4 connector.
  CN9: 1 x 4 connector.
  CN10: DB9 video out connector.
  CN11 1 x 2 bridge connector.

**************************************************************************

  UM487F HCGA Controller notes...

  The fact that there is a 14.318 MHz crystal tied to pin 65, just point
  that the video controller is working in CGA mode. MGA mode needs a
  16.257 MHz crystal instead, and tied to pin 64 (currently tied to GND).

  Also a signal of 8Mhz (shared with the program CPU is entering from the
  pin 1 (CLK) needed for clock the UM6845 mode.

  UM487F Access:

  Offsets are for sure the CGA mode. MGA mode has different ones.

  3D4h: -W  CRTC index register.
  3D5h: RW  CRTC data register.
  3D8h: -W  Mode control register.
  3D9h: -W  Color select register.
  3DAh: R-  Status register.
  3BFh: -W  Config register.

  Mode CTRL (3D8h): 0x6A / 0x62
  ----- bits -----
  7 6 5 4  3 2 1 0   For CGA Mode.
  - x x -  x - x -
  | | | |  | | | |
  | | | |  | | | '-- 40*25 text.
  | | | |  | | '---- Graphics.
  | | | |  | '------ Color Mode.
  | | | |  '-------- Enable Video.
  | | | '----------- 320x200 Graphics.
  | | '------------- Enable Blink.
  | '--------------- Enable Change Mode.
  '----------------- (not for CGA)

  Color Sel (3D9h): 00

  Index register (3D4h): 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
  Data register (3D5h):  38 28 2D 0A 7F 06 64 70 02 01 06 07 10 00 00 00

  Config Register (3BFh): 0x40
  (bit 6 active means CGA Mode)


  So... Screen size is set to 320x200.
  but...

  The embedded CRT controller is set to:
  Screen Total:   0x38+1 * 0x7F+1 = (57 * 128) chars.
  Screen Visible: 0x28 * 0x64 = (40 * 100) chars.

  NOTE: All (registers and offsets) match the CGA ISA card.
  Maybe we can find a workaround to hook the controller
  without the ISA bus.

**************************************************************************

  Custom IC's...

  8952 CM 32 pinouts and peripheral circuitry:

                               8952 CM 32
                            .------v------.
    74HC244 (IC9), PIN 08 --|01         40|-- 74HC244 (IC9), PIN 17
    74HC244 (IC9), PIN 06 --|02         39|-- 74HC244 (IC9), PIN 15
    74HC244 (IC9), PIN 04 --|03         38|-- 74HC244 (IC9), PIN 13
    74HC244 (IC9), PIN 02 --|04         37|-- 74HC244 (IC9), PIN 11
   74HC244 (IC10), PIN 08 --|05         36|-- 74HC244 (IC10), PIN 17
   74HC244 (IC10), PIN 06 --|06         35|-- 74HC244 (IC10), PIN 15
   74HC244 (IC10), PIN 04 --|07    8    34|-- 74HC244 (IC10), PIN 13
   74HC244 (IC10), PIN 02 --|08    9    33|-- 74HC244 (IC10), PIN 11
                            |      5      |
                         /--|09    2    32|--\
  To CN1 (through IC15) | --|10         31|-- | To CN1 (through IC15)
                        | --|11         30|-- |
                         \--|12    C    29|--/
                            |      M     |
                         /--|13         28|--\
  To CN2 (through IC14) | --|14    3    27|-- | To CN2 (through IC14)
                        | --|15    2    26|-- |
                         \--|16         25|--/
                            |             |
      To CN3 and ES2 9046 --|17         24|--\
      To CN3 and ES2 9046 --|18         23|-- > bridge to GND
                   To CN3 --|19         22|--/
                            |             |
                      GND --|20         21|-- VCC
                            '-------------'

                 74HC244 (IC9)                                 74HC244 (IC10)
                  .---v---.                                     .---v---.
   GAL (PIN 17) --|01   20|-- VCC                GAL (PIN 16) --|01   20|-- VCC
  8952 (PIN 04) --|02   19|-- GAL (PIN 17)      8952 (PIN 08) --|02   19|-- GAL (PIN 16)
  MAIN Z80 (D7) --|03   18|-- MAIN Z80 (D0)     MAIN Z80 (D7) --|03   18|-- MAIN Z80 (D0)
  8952 (PIN 03) --|04   17|-- 8952 (PIN 40)     8952 (PIN 07) --|04   17|-- 8952 (PIN 36)
  MAIN Z80 (D6) --|05   16|-- MAIN Z80 (D1)     MAIN Z80 (D6) --|05   16|-- MAIN Z80 (D1)
  8952 (PIN 02) --|06   15|-- 8952 (PIN 39)     8952 (PIN 06) --|06   15|-- 8952 (PIN 35)
  MAIN Z80 (D5) --|07   14|-- MAIN Z80 (D2)     MAIN Z80 (D5) --|07   14|-- MAIN Z80 (D2)
  8952 (PIN 01) --|08   13|-- 8952 (PIN 38)     8952 (PIN 05) --|08   13|-- 8952 (PIN 34)
  MAIN Z80 (D4) --|09   12|-- MAIN Z80 (D3)     MAIN Z80 (D4) --|09   12|-- MAIN Z80 (D3)
            GND --|10   11|-- 8952 (PIN 37)               GND --|10   11|-- 8952 (PIN 33)
                  '-------'                                     '-------'


  ES2 CM3080 pinouts and peripheral circuitry:

                    CM3080
                   .---v---.
             VCC --|01   18|-- VCC          .--------.
             N/C --|02   17|----------------+ 16 MHz |
             N/C --|03   16|----------------+  Xtal  |
             N/C --|04   15|-- CLK OUT --.  '--------'
             GND --|05   14|-- N/C       |
             GND --|06   13|-- N/C       '--+-- (8MHz) UM487F (PIN 01, CLK)
  MAIN Z80 (/M1) --|07   12|-- GND          +-- (8MHz) MAIN Z80 (PIN 06, CLK)
    GAL (PIN 11) --|08   11|-- VCC
             GND --|09   10|-- MAIN Z80 (/INT)
                   '-------'


  Notes:

  - Looks like the GAL is switching the different '8952 CM 32' outputs
     through the 74HC244 drivers to the Z80 data bus.
  - CN1, CN2 & CN3 are blind connectors.
  - 8952 pinouts to CN1 & CN2, are also passing through locations
     IC14 & IC15 (both are unpopulated from factory).
  - GAL is GAL16V8 at location IC4.
  - CM3080 pins 16 & 17 have a 1 Megohm resistor in parallel before connect the
     16 MHz. crystal.

**************************************************************************

  TODO:

  - Proper UM487F device emulation.
  - Interlaced video mode.
  - Sound.
  - More work...

*************************************************************************/

#define MAIN_CLOCK           XTAL_16MHz
#define SEC_CLOCK            XTAL_8MHz
#define HCGA_CLOCK           XTAL_14_31818MHz

#define PRG_CPU_CLOCK        MAIN_CLOCK /2      /* 8 MHz. (measured) */
#define SND_CPU_CLOCK        SEC_CLOCK /2       /* 4 MHz. (measured) */
#define SND_AY_CLOCK         SEC_CLOCK /4       /* 2 MHz. (measured) */
#define CRTC_CLOCK           SEC_CLOCK /2       /* 8 MHz. (measured) */


#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "bus/isa/isa.h"
#include "bus/isa/cga.h"
#include "video/cgapal.h"

class _4enlinea_state : public driver_device
{
public:
	_4enlinea_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ay(*this, "aysnd"),
		m_maincpu(*this, "maincpu")
		{ }


	required_device<ay8910_device> m_ay;

	DECLARE_READ8_MEMBER(serial_r);
	DECLARE_READ8_MEMBER(serial_status_r);
	DECLARE_WRITE8_MEMBER(serial_w);
	DECLARE_WRITE8_MEMBER(serial_status_w);
	DECLARE_READ8_MEMBER(hack_r);
	INTERRUPT_GEN_MEMBER(_4enlinea_irq);
	INTERRUPT_GEN_MEMBER(_4enlinea_audio_irq);

	UINT8 m_irq_count;
	UINT8 m_serial_flags;
	UINT8 m_serial_data[2];

	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;

};


/***********************************
*          Video Hardware          *
***********************************/

// TODO: this is actually UM487F
class isa8_cga_4enlinea_device : public isa8_cga_device
{
public:
	// construction/destruction
	isa8_cga_4enlinea_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( _4enlinea_io_read );
	DECLARE_WRITE8_MEMBER( _4enlinea_mode_control_w );
	virtual void device_start() override;
	virtual const rom_entry *device_rom_region() const override;
};

const rom_entry *isa8_cga_4enlinea_device::device_rom_region() const
{
	return nullptr;
}

const device_type ISA8_CGA_4ENLINEA = &device_creator<isa8_cga_4enlinea_device>;

isa8_cga_4enlinea_device::isa8_cga_4enlinea_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_CGA_4ENLINEA, "ISA8_CGA_4ENLINEA", tag, owner, clock, "4enlinea_cga", __FILE__)
{
}


READ8_MEMBER( isa8_cga_4enlinea_device::_4enlinea_io_read )
{
	UINT8 data;

	switch (offset)
	{
	case 0xa:
		data = isa8_cga_device::io_read(space, offset);
		data|= (data & 8) << 4;
		break;

	default:
		data = isa8_cga_device::io_read(space, offset);
		break;
	}
	return data;
}

WRITE8_MEMBER( isa8_cga_4enlinea_device::_4enlinea_mode_control_w )
{
	// TODO
}

void isa8_cga_4enlinea_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	set_isa_device();
	m_vram_size = 0x4000;
	m_vram.resize(m_vram_size);

	//m_isa->install_device(0x3bf, 0x3bf, 0, 0, NULL, write8_delegate( FUNC(isa8_cga_4enlinea_device::_4enlinea_mode_control_w), this ) );
	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate( FUNC(isa8_cga_4enlinea_device::_4enlinea_io_read), this ), write8_delegate( FUNC(isa8_cga_device::io_write), this ) );
	m_isa->install_bank(0x8000, 0xbfff, 0, 0, "bank1", &m_vram[0]);

	/* Initialise the cga palette */
	int i;

	for ( i = 0; i < CGA_PALETTE_SETS * 16; i++ )
	{
		m_palette->set_pen_color( i, cga_palette[i][0], cga_palette[i][1], cga_palette[i][2] );
	}

	i = 0x8000;
	for ( int r = 0; r < 32; r++ )
	{
		for ( int g = 0; g < 32; g++ )
		{
			for ( int b = 0; b < 32; b++ )
			{
				m_palette->set_pen_color( i, r << 3, g << 3, b << 3 );
				i++;
			}
		}
	}

//  m_chr_gen_base = memregion(subtag("gfx1"))->base();
//  m_chr_gen = m_chr_gen_base + m_chr_gen_offset[1];
}


READ8_MEMBER(_4enlinea_state::serial_r)
{
	if(offset == 0)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI,CLEAR_LINE);
		m_serial_flags |= 0x20;
	}

	return m_serial_data[offset];
}



/***********************************
*      Memory Map Information      *
***********************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, _4enlinea_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
//  AM_RANGE(0x8000, 0xbfff) AM_RAM // CGA VRAM
	AM_RANGE(0xc000, 0xdfff) AM_RAM

	AM_RANGE(0xe000, 0xe001) AM_READ(serial_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_portmap, AS_IO, 8, _4enlinea_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3ff)

//  AM_RANGE(0x3d4, 0x3df) CGA regs
	AM_RANGE(0x3bf, 0x3bf) AM_WRITENOP // CGA mode control, TODO
ADDRESS_MAP_END

READ8_MEMBER(_4enlinea_state::serial_status_r)
{
	return m_serial_flags;
}

WRITE8_MEMBER(_4enlinea_state::serial_status_w)
{
	m_serial_flags = data; // probably just clears
}

/* TODO: do this really routes to 0xe000-0xe001 of Main CPU? */
WRITE8_MEMBER(_4enlinea_state::serial_w)
{
	m_serial_data[offset] = data;
	if(offset == 0)
		m_maincpu->set_input_line(INPUT_LINE_NMI,ASSERT_LINE);
}

READ8_MEMBER(_4enlinea_state::hack_r)
{
	return machine().rand();
}

static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, _4enlinea_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xf800, 0xfbff) AM_RAM
	AM_RANGE(0xfc24, 0xfc24) AM_READ(hack_r)
	AM_RANGE(0xfc28, 0xfc28) AM_READ(hack_r)
	AM_RANGE(0xfc30, 0xfc31) AM_WRITE(serial_w)
	AM_RANGE(0xfc32, 0xfc32) AM_READWRITE(serial_status_r,serial_status_w)
	AM_RANGE(0xfc48, 0xfc49) AM_DEVREADWRITE("aysnd", ay8910_device, data_r, address_data_w)

ADDRESS_MAP_END


static ADDRESS_MAP_START( audio_portmap, AS_IO, 8, _4enlinea_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


/***********************************
*           Input Ports            *
***********************************/

static INPUT_PORTS_START( 4enlinea )

/*  Player 1 & 2 ports are tied to both AY-3-8910 ports.
    Coin 1 & 2 are tied to the big ES2 9046 CPLD/FPGA,
    so... It's a mystery to figure out.
*/
	PORT_START("IN-P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY  PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                   PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )                   PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN-P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY  PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                   PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )                   PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )


	PORT_START( "pcvideo_cga_config" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/***********************************
*         Graphics Layouts         *
***********************************/



/****************************************
*      Graphics Decode Information      *
****************************************/

//static GFXDECODE_START( 4enlinea )
//GFXDECODE_END


/****************************************
*          Machine Start/Reset          *
****************************************/

void _4enlinea_state::machine_start()
{
}

void _4enlinea_state::machine_reset()
{
}

/***********************************
*         Machine Drivers          *
***********************************/

SLOT_INTERFACE_START( 4enlinea_isa8_cards )
	SLOT_INTERFACE_INTERNAL("4enlinea",  ISA8_CGA_4ENLINEA)
SLOT_INTERFACE_END

/* TODO: irq sources are unknown */
INTERRUPT_GEN_MEMBER(_4enlinea_state::_4enlinea_irq)
{
	if(m_irq_count == 0)
	{
		//device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
	else
		device.execute().set_input_line(0, HOLD_LINE);

	m_irq_count++;
	m_irq_count&=3;
}

INTERRUPT_GEN_MEMBER(_4enlinea_state::_4enlinea_audio_irq)
{
	device.execute().set_input_line(0, HOLD_LINE);
}

static MACHINE_CONFIG_START( 4enlinea, _4enlinea_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, PRG_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(_4enlinea_state, _4enlinea_irq, 60) //TODO
//  MCFG_CPU_PERIODIC_INT_DRIVER(_4enlinea_state, irq0_line_hold, 4*35)

	MCFG_CPU_ADD("audiocpu", Z80, SND_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(audio_map)
	MCFG_CPU_IO_MAP(audio_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(_4enlinea_state, _4enlinea_audio_irq, 60) //TODO

	MCFG_DEVICE_ADD("isa", ISA8, 0)
	MCFG_ISA8_CPU(":maincpu")
	MCFG_ISA8_SLOT_ADD("isa", "isa1", 4enlinea_isa8_cards, "4enlinea", true)


/*  6845 clock is a guess, since it's a UM6845R embedded in the UM487F.
    CRTC_CLOCK is 8MHz, entering for pin 1 of UM487F. This clock is used
    only for UM6845R embedded mode. The frequency divisor is unknown.

    CRTC_CLOCK / 4.0 = 66.961296 Hz.
    CRTC_CLOCK / 4.5 = 59.521093 Hz.
    CRTC_CLOCK / 5.0 = 53.569037 Hz.
*/

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, SND_AY_CLOCK)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN-P2"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("IN-P1"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/***********************************
*             Rom Load             *
***********************************/

ROM_START( 4enlinea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cuatro_en_linea_27c256__cicplay-2.ic6",  0x0000, 0x8000, CRC(f8f14bf8) SHA1(e48fbedbd1b9be6fb56a0f65db80eddbedb487c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cuatro_en_linea_27c256__cicplay-1.ic19", 0x0000, 0x8000, CRC(307a57a3) SHA1(241329d919ec43d0eeb1dad0a4db6cf6de06e7e1) )

	ROM_REGION( 0x0800, "eeprom", 0 )   /* default serial EEPROM */
	ROM_LOAD( "cuatro_en_linea_x24c16p__nosticker.ic17", 0x000, 0x800, CRC(21f81f5a) SHA1(00b10eee5af1ca79ced2878f4be4cac2bb8d26a0) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "cuatro_en_linea_gal16v8as__nosticker.ic04", 0x000, 0x117, CRC(094edf29) SHA1(428a2f6568ac1032833ee0c65fa8304967a58607) )
ROM_END


/***********************************
*           Game Drivers           *
***********************************/

/*    YEAR  NAME       PARENT   MACHINE   INPUT     STATE          INIT   ROT    COMPANY       FULLNAME          FLAGS  */
GAME( 1991, 4enlinea,  0,       4enlinea, 4enlinea, driver_device, 0,     ROT0, "Compumatic", "Cuatro en Linea", MACHINE_NOT_WORKING )
