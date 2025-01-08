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

  Known games on this or similar hardware:

  - [DUMPED]  4 en Línea (Compumatic)
  - [DUMPED]  Dardos (Oper Coin)
  - [DUMPED]  Olympic Darts (K7 Kursaal. At least three different hardware revisions)
  - [DUMPED]  Sport Darts TV (Compumatic / Desarrollos y Recambios S.L.)
  - [MISSING] Dart Queen (Compumatic / Daryde)

**************************************************************************

  TODO:

  - Proper UM487F device emulation.
  - Interlaced video mode.
  - Sound.
  - k7_olym accesses i2c device with an id of 0xeb. Device is unknown.
  - More work...

*************************************************************************/

#include "emu.h"
#include "bus/isa/cga.h"
#include "bus/isa/isa.h"
#include "cpu/z80/z80.h"
#include "machine/i2cmem.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/cgapal.h"
#include "video/mc6845.h"
#include "speaker.h"


#define MAIN_CLOCK           XTAL(16'000'000)
#define SEC_CLOCK            XTAL(8'000'000)
#define HCGA_CLOCK           XTAL(14'318'181)

#define PRG_CPU_CLOCK        MAIN_CLOCK /2      // 8 MHz. (measured)
#define SND_CPU_CLOCK        SEC_CLOCK /2       // 4 MHz. (measured)
#define SND_AY_CLOCK         SEC_CLOCK /4       // 2 MHz. (measured)
#define CRTC_CLOCK           SEC_CLOCK /2       // 8 MHz. (measured)

class _4enlinea_state : public driver_device
{
public:
	_4enlinea_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_ay(*this, "aysnd"),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom")
	{ }

	void _4enlinea(machine_config &config);
	void k7_olym(machine_config &config);

private:
	required_device<ay8910_device> m_ay;

	uint8_t serial_r(offs_t offset);
	uint8_t serial_status_r();
	void serial_w(offs_t offset, uint8_t data);
	void serial_status_w(uint8_t data);
	uint8_t hack_r();
	INTERRUPT_GEN_MEMBER(_4enlinea_irq);
	INTERRUPT_GEN_MEMBER(_4enlinea_audio_irq);

	uint8_t eeprom_data_r();
	void eeprom_data_w(uint8_t data);
	void eeprom_control_w(uint8_t data);
	void eeprom_clock_w(uint8_t data);

	uint8_t k7_in_r();
	void k7_out0_w(uint8_t data);
	void k7_out1_w(uint8_t data);

	uint8_t m_irq_count = 0;
	uint8_t m_serial_flags = 0;
	uint8_t m_serial_data[2]{};

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<i2cmem_device> m_eeprom;

	void audio_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;

	void k7_mem_map(address_map &map) ATTR_COLD;
	void k7_io_map(address_map &map) ATTR_COLD;
};


/***********************************
*          Video Hardware          *
***********************************/

// TODO: this is actually UM487F
class isa8_cga_4enlinea_device : public isa8_cga_device
{
public:
	// construction/destruction
	isa8_cga_4enlinea_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t _4enlinea_io_read (offs_t offset);
	void _4enlinea_mode_control_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

const tiny_rom_entry *isa8_cga_4enlinea_device::device_rom_region() const
{
	return nullptr;
}

DEFINE_DEVICE_TYPE(ISA8_CGA_4ENLINEA, isa8_cga_4enlinea_device, "4enlinea_cga", "ISA8 CGA - 4enlinea")

isa8_cga_4enlinea_device::isa8_cga_4enlinea_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_cga_device( mconfig, ISA8_CGA_4ENLINEA, tag, owner, clock)
{
}


uint8_t isa8_cga_4enlinea_device::_4enlinea_io_read(offs_t offset)
{
	uint8_t data;

	switch (offset)
	{
	case 0xa:
		data = isa8_cga_device::io_read(offset);
		data|= (data & 8) << 4;
		break;

	default:
		data = isa8_cga_device::io_read(offset);
		break;
	}
	return data;
}

void isa8_cga_4enlinea_device::_4enlinea_mode_control_w(uint8_t data)
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

	//m_isa->install_device(0x3bf, 0x3bf, 0, 0, nullptr, write8_delegate(*this, FUNC(isa8_cga_4enlinea_device::_4enlinea_mode_control_w)));
	m_isa->install_device(0x3d0, 0x3df, read8sm_delegate(*this, FUNC(isa8_cga_4enlinea_device::_4enlinea_io_read)), write8sm_delegate(*this, FUNC(isa8_cga_device::io_write)));
	m_isa->install_bank(0x8000, 0xbfff, &m_vram[0]);

	// Initialise the CGA palette
	int i;

	for (int i = 0; i < CGA_PALETTE_SETS * 16; i++ )
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


uint8_t _4enlinea_state::serial_r(offs_t offset)
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

void _4enlinea_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
//  map(0x8000, 0xbfff).ram(); // CGA VRAM
	map(0xc000, 0xdfff).ram();

	map(0xe000, 0xe001).r(FUNC(_4enlinea_state::serial_r));
}

void _4enlinea_state::main_portmap(address_map &map)
{
	map.global_mask(0x3ff);

//  map(0x3d4, 0x3df) CGA regs
	map(0x3bf, 0x3bf).nopw(); // CGA mode control, TODO
}

uint8_t _4enlinea_state::serial_status_r()
{
	return m_serial_flags;
}

void _4enlinea_state::serial_status_w(uint8_t data)
{
	m_serial_flags = data; // probably just clears
}

// TODO: do this really routes to 0xe000-0xe001 of Main CPU?
void _4enlinea_state::serial_w(offs_t offset, uint8_t data)
{
	m_serial_data[offset] = data;
	if(offset == 0)
		m_maincpu->set_input_line(INPUT_LINE_NMI,ASSERT_LINE);
}

uint8_t _4enlinea_state::eeprom_data_r()
{
	return m_eeprom->read_sda();
}

void _4enlinea_state::eeprom_data_w(uint8_t data)
{
	m_eeprom->write_sda(BIT(data, 0));
}

void _4enlinea_state::eeprom_control_w(uint8_t data)
{
	if (BIT(data, 0))
		m_eeprom->write_sda(1);
}

void _4enlinea_state::eeprom_clock_w(uint8_t data)
{
	m_eeprom->write_scl(BIT(data, 6));
}

uint8_t _4enlinea_state::hack_r()
{
	return machine().rand();
}

void _4enlinea_state::audio_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xf800, 0xfbff).ram();
	map(0xfc24, 0xfc24).rw(FUNC(_4enlinea_state::eeprom_data_r), FUNC(_4enlinea_state::eeprom_data_w));
	map(0xfc25, 0xfc25).w(FUNC(_4enlinea_state::eeprom_control_w));
	map(0xfc26, 0xfc26).w(FUNC(_4enlinea_state::eeprom_clock_w));
	map(0xfc28, 0xfc28).r(FUNC(_4enlinea_state::hack_r));
	map(0xfc30, 0xfc31).w(FUNC(_4enlinea_state::serial_w));
	map(0xfc32, 0xfc32).rw(FUNC(_4enlinea_state::serial_status_r), FUNC(_4enlinea_state::serial_status_w));
	map(0xfc48, 0xfc48).w(m_ay, FUNC(ay8910_device::address_w));
	map(0xfc49, 0xfc49).r(m_ay, FUNC(ay8910_device::data_r));
	map(0xfc4a, 0xfc4a).w(m_ay, FUNC(ay8910_device::data_w));
}

uint8_t _4enlinea_state::k7_in_r()
{
	return m_eeprom->read_sda() << 4;
}

void _4enlinea_state::k7_out0_w(uint8_t data)
{
	m_eeprom->write_sda(!BIT(data, 3));
	m_eeprom->write_scl(BIT(data, 2));
}

void _4enlinea_state::k7_out1_w(uint8_t data)
{
}

void _4enlinea_state::k7_mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0xc000, 0xdfff).rom().region("maincpu", 0x8000);
	map(0xe000, 0xffff).ram().share("nvram");
}

void _4enlinea_state::k7_io_map(address_map &map)
{
	map(0x0000, 0x0000).mirror(0xfc00).w(FUNC(_4enlinea_state::k7_out0_w));
	map(0x0001, 0x0001).mirror(0xfc00).rw(FUNC(_4enlinea_state::k7_in_r), FUNC(_4enlinea_state::k7_out1_w));
	map(0x0100, 0x0100).w(m_ay, FUNC(ay8910_device::address_w));
	map(0x0101, 0x0101).r(m_ay, FUNC(ay8910_device::data_r));
	map(0x0102, 0x0102).w(m_ay, FUNC(ay8910_device::data_w));
//  0x03bf W (0x40)
}


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


static INPUT_PORTS_START( k7_olym )
	PORT_START("IN-P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN-P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "pcvideo_cga_config" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/***********************************
*         Graphics Layouts         *
***********************************/



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

void _4enlinea_isa8_cards(device_slot_interface &device)
{
	device.option_add_internal("4enlinea",  ISA8_CGA_4ENLINEA);
}

// TODO: IRQ sources are unknown
INTERRUPT_GEN_MEMBER(_4enlinea_state::_4enlinea_irq)
{
	if(m_irq_count == 0)
	{
		//device.execute().pulse_input_line(INPUT_LINE_NMI, attotime::zero);
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

void _4enlinea_state::_4enlinea(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, PRG_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &_4enlinea_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &_4enlinea_state::main_portmap);
	m_maincpu->set_periodic_int(FUNC(_4enlinea_state::_4enlinea_irq), attotime::from_hz(60)); //TODO
//  m_maincpu->set_periodic_int(FUNC(_4enlinea_state::irq0_line_hold), attotime::from_hz(4*35));

	z80_device &audiocpu(Z80(config, "audiocpu", SND_CPU_CLOCK));
	audiocpu.set_addrmap(AS_PROGRAM, &_4enlinea_state::audio_map);
	audiocpu.set_periodic_int(FUNC(_4enlinea_state::_4enlinea_audio_irq), attotime::from_hz(60)); //TODO

	I2C_24C16(config, m_eeprom); // X24C16P

	// FIXME: determine ISA bus clock
	isa8_device &isa(ISA8(config, "isa", 0));
	isa.set_memspace("maincpu", AS_PROGRAM);
	isa.set_iospace("maincpu", AS_IO);

	ISA8_SLOT(config, "isa1", 0, "isa", _4enlinea_isa8_cards, "4enlinea", true);


/*  6845 clock is a guess, since it's a UM6845R embedded in the UM487F.
    CRTC_CLOCK is 8MHz, entering for pin 1 of UM487F. This clock is used
    only for UM6845R embedded mode. The frequency divisor is unknown.

    CRTC_CLOCK / 4.0 = 66.961296 Hz.
    CRTC_CLOCK / 4.5 = 59.521093 Hz.
    CRTC_CLOCK / 5.0 = 53.569037 Hz.
*/

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay, SND_AY_CLOCK);
	m_ay->port_a_read_callback().set_ioport("IN-P2");
	m_ay->port_b_read_callback().set_ioport("IN-P1");
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.50);
}


void _4enlinea_state::k7_olym(machine_config &config)
{
	Z80(config, m_maincpu, 14.318181_MHz_XTAL / 2); // Z84C00BB6
	m_maincpu->set_addrmap(AS_PROGRAM, &_4enlinea_state::k7_mem_map);
	m_maincpu->set_addrmap(AS_IO, &_4enlinea_state::k7_io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // D4464C-15L (6264) + battery

	I2C_24C16(config, m_eeprom); // X24C16P

	isa8_device &isa(ISA8(config, "isa", 0));
	isa.set_memspace("maincpu", AS_PROGRAM);
	isa.set_iospace("maincpu", AS_IO);

	ISA8_SLOT(config, "isa1", 0, "isa", _4enlinea_isa8_cards, "4enlinea", true); // UM487F

	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay, 14.318181_MHz_XTAL / 8); // Winbond WF19054
	m_ay->port_a_read_callback().set_ioport("IN-P2");
	m_ay->port_b_read_callback().set_ioport("IN-P1");
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.50);
}


/***********************************
*             Rom Load             *
***********************************/

ROM_START( 4enlinea )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cuatro_en_linea_27c256__cicplay-2.ic6",  0x0000, 0x8000, CRC(f8f14bf8) SHA1(e48fbedbd1b9be6fb56a0f65db80eddbedb487c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cuatro_en_linea_27c256__cicplay-1.ic19", 0x0000, 0x8000, CRC(307a57a3) SHA1(241329d919ec43d0eeb1dad0a4db6cf6de06e7e1) )

	ROM_REGION( 0x0800, "eeprom", 0 ) // default serial EEPROM
	ROM_LOAD( "cuatro_en_linea_x24c16p__nosticker.ic17", 0x0000, 0x0800, CRC(21f81f5a) SHA1(00b10eee5af1ca79ced2878f4be4cac2bb8d26a0) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "cuatro_en_linea_gal16v8as__nosticker.ic04", 0x0000, 0x0117, CRC(094edf29) SHA1(428a2f6568ac1032833ee0c65fa8304967a58607) )
ROM_END

ROM_START( 4enlineb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cuatro_en_linea_2_a06.ic6",  0x0000, 0x8000, CRC(f8f14bf8) SHA1(e48fbedbd1b9be6fb56a0f65db80eddbedb487c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cuatro_en_linea_1_a06.ic19", 0x0000, 0x8000, CRC(993d0581) SHA1(d6e366dd827543508037d2071c4b6e638c2cf87b) )

	ROM_REGION( 0x0800, "eeprom", 0 ) // From an operated PCB, a clean one for default need to be created...
	ROM_LOAD( "cuatro_en_linea_24c16.ic17", 0x0000, 0x0800, CRC(56722dd4) SHA1(f818d882b3070f9b1fac486987a044ab1d418985) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "cuatro_en_linea_gal16v8a.ic04", 0x0000, 0x0117, CRC(1edaf06c) SHA1(51e44c2e6b54991330d6ef945e98fa2c8a49408d) )
ROM_END

/*
  Dardos
  Oper Coin. 1991.
  Running in 487 System I.
*/
ROM_START( dardos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "diana_iv_video_27-1-92.bin",  0x0000, 0x8000, CRC(f23b5313) SHA1(488cf9bedce7b0c7b474bd93da70181c81fa300b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "diana_iv_master_27-1-92.bin", 0x0000, 0x8000, CRC(4b2c868a) SHA1(91120a32fac9c5a6e7746d2e2587921f7d42eaa3) )
ROM_END


/* Kursaal K7 Olympic Darts PCB
    __________________________________________________      SUBBOARD CM3080
    |           ________  __   ______  ______________ |     ________________
    |  _______  | DB9   | |_| |_CN8__| |____CN7______||     |___ __________ |
    |  |______| |_______| CN9                         |__   ||  ||HEF4020BP||
    | ________                                         __|  ||A | _________ |
    | |D41464C|                                        __|  ||  | |________||
    | ________                           _____         __|  ||__| _________ |
    | |D41464C|                         DA741CN        __|  |     |TC4011BP||
    | ________                         _______    ___  __|  |    __________ |
 IC4->|GAL16V8|  _______              HCF4069UBE  XT5  __|  |    |__EMPTY__||
    | ________   |UMC   |  ________   _______________  __|  |    __________ |
IC11->|GAL16V8|  |UM487F|  74HC273AP  |WF19054       | __|  |    |HEF4020BP||
    |            |______|  ________   |______________| __|  |_______________|
    | ________             74HC273AP  _______________  __|    A=74LS368ANA
    | |74LS04N|           ___________ |Z84C00BB6     | __|
    |  _____              | SUBBOARD ||______________| __|
    |  |XT2_|<-14.31818MHz| CM3080   |____________     __|
    |           ________  |          ||M27C512 ROM|    __|
    |          HCF4069UBE |          ||___________|    __|
    |                     |          |____________     __|
    |                     |          ||D4464C-15L |    __|
    |                     |__________||___________|    __|
    | 7808CT    ________    ________  _____   _____   |
    |           |_______|  74LS541B1 X24C16P  |BATT|  |
    | _____  ___________  _____________       |____|  |
    | |CN1_| |__CN5_____| |__CN4_______|              |
    |_________________________________________________|
*/
ROM_START( k7_olym )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "odk7_v3.11_27c512.ic18", 0x00000, 0x10000, CRC(063d24fe) SHA1(ad4509438d2028ede779f5aa9a918d1020c1db41) )

	// The EEPROM contains a custom message (operators can set on-screen messages).
	// A clean one for default need to be created...
	ROM_REGION( 0x0800, "eeprom", 0 )
	ROM_LOAD( "x24c16p.bin", 0x0000, 0x0800, CRC(4c6685b2) SHA1(38c4f64f038d7ce185d6fd0b6eec4c9818f64e8e) )

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "a1_gal16v8a.ic11", 0x0000, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "b1_gal16v8a.ic4",  0x0117, 0x0117, NO_DUMP ) // protected
ROM_END

ROM_START( k7_olym30 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dardos_k7_3.0_21-11-94_27c512.ic19", 0x00000, 0x10000,  CRC(87af55a6) SHA1(7d12ce7afe8a50ba895f05029c1bd05a3641f7fd) )

	ROM_REGION( 0x0800, "eeprom", 0 )
	ROM_LOAD( "x24c16p.bin", 0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "a1_gal16v8a.ic11", 0x0000, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "b1_gal16v8a.ic4",  0x0117, 0x0117, NO_DUMP ) // protected
ROM_END


ROM_START( sprtdart )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sport_dart_27c512.ic19", 0x00000, 0x10000, CRC(6c9ae27f) SHA1(92fbdef7747a9096daf4714f45b119ad8f3a1436) )

	ROM_REGION( 0x0800, "eeprom", 0 )
	ROM_LOAD( "24c16.ic17", 0x0000, 0x0800, NO_DUMP ) // Undumped

	ROM_REGION( 0x0300, "plds", 0 )
	ROM_LOAD( "gal16v8a.ic11", 0x0000, 0x0117, NO_DUMP ) // protected
	ROM_LOAD( "gal16v8a.ic4",  0x0117, 0x0117, NO_DUMP ) // protected
ROM_END


/***********************************
*           Game Drivers           *
***********************************/

//    YEAR  NAME       PARENT    MACHINE    INPUT     CLASS            INIT        ROT    COMPANY                                      FULLNAME                            FLAGS
GAME( 1991, 4enlinea,  0,        _4enlinea, 4enlinea, _4enlinea_state, empty_init, ROT0, "Compumatic / CIC Play",                     "Cuatro en Linea (rev. A-07)", MACHINE_NOT_WORKING )
GAME( 1991, 4enlineb,  4enlinea, _4enlinea, 4enlinea, _4enlinea_state, empty_init, ROT0, "Compumatic / CIC Play",                     "Cuatro en Linea (rev. A-06)", MACHINE_NOT_WORKING )
GAME( 1992, dardos,    0,        _4enlinea, 4enlinea, _4enlinea_state, empty_init, ROT0, "Oper Coin",                                 "Dardos",                      MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 1994, k7_olym,   0,        k7_olym,   k7_olym,  _4enlinea_state, empty_init, ROT0, "K7 Kursaal / NMI Electronics",              "Olympic Darts K7 (v3.11)",    MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 1994, k7_olym30, k7_olym,  k7_olym,   k7_olym,  _4enlinea_state, empty_init, ROT0, "K7 Kursaal / NMI Electronics",              "Olympic Darts K7 (v3.00)",    MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 1993, sprtdart,  0,        k7_olym,   k7_olym,  _4enlinea_state, empty_init, ROT0, "Compumatic / Desarrollos y Recambios S.L.", "Sport Darts T.V.",            MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
