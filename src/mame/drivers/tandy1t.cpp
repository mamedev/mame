// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
Tandy 1000
==========

Tandy 1000 machines are similar to the IBM 5160s with CGA graphics. Tandy
added some additional graphic capabilities similar, but not equal, to
those added for the IBM PC Jr.

Tandy 1000 (8088) variations:
1000                128KB-640KB RAM     4.77 MHz        v01.00.00, v01.01.00
1000A/1000HD        128KB-640KB RAM     4.77 MHz        v01.01.00
1000SX/1000AX       384KB-640KB RAM     7.16/4.77 MHz   v01.02.00
1000EX              256KB-640KB RAM     7.16/4.77 MHz   v01.02.00
1000HX              256KB-640KB RAM     7.16/4.77 MHz   v02.00.00

Tandy 1000 (8086) variations:
1000RL/1000RL-HD    512KB-768KB RAM     9.44/4.77 MHz   v02.00.00, v02.00.01
1000SL/1000PC       384KB-640KB RAM     8.0/4.77 MHz    v01.04.00, v01.04.01, v01.04.02, v02.00.01
1000SL/2            512KB-640KB RAM     8.0/4.77 MHz    v01.04.04

Tandy 1000 (80286) variations:
1000TX              640KB-768KB RAM     8.0/4.77 MHz    v01.03.00
1000TL              640KB-768KB RAM     8.0/4.77 MHz    v01.04.00, v01.04.01, v01.04.02
1000TL/2            640KB-768KB RAM     8.0/4.77 MHz    v02.00.00
1000TL/3            640KB-768KB RAM     10.0/5.0 MHz    v02.00.00
1000RLX             512KB-1024KB RAM    10.0/5.0 MHz    v02.00.00
1000RLX-HD          1024MB RAM          10.0/5.0 MHz    v02.00.00

Tandy 1000 (80386) variations:
1000RSX/1000RSX-HD  1M-9M RAM           25.0/8.0 MHz    v01.10.00 */

#include "emu.h"
#include "includes/genpc.h"
#include "machine/pckeybrd.h"
#include "machine/nvram.h"
#include "machine/pc_fdc.h"
#include "video/pc_t1t.h"
#include "sound/sn76496.h"
#include "cpu/i86/i86.h"
#include "cpu/i86/i286.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "softlist.h"

class tandy1000_state : public driver_device
{
public:
	tandy1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_romcs0(*this, "romcs0")
		, m_romcs1(*this, "romcs1")
		, m_biosbank(*this, "biosbank")
		, m_keyboard(*this, "pc_keyboard")
		, m_mb(*this, "mb") { }

	required_device<cpu_device> m_maincpu;

// Memory regions for the machines that support rom banking
	optional_memory_region m_romcs0;
	optional_memory_region m_romcs1;
	optional_memory_bank m_biosbank;

	required_device<pc_keyboard_device> m_keyboard;
	required_device<pc_noppi_mb_device> m_mb;

	DECLARE_WRITE8_MEMBER ( pc_t1t_p37x_w );
	DECLARE_READ8_MEMBER ( pc_t1t_p37x_r );

	DECLARE_WRITE8_MEMBER ( tandy1000_pio_w );
	DECLARE_READ8_MEMBER(tandy1000_pio_r);
	DECLARE_READ8_MEMBER( tandy1000_bank_r );
	DECLARE_WRITE8_MEMBER( tandy1000_bank_w );

	int tandy1000_read_eeprom();
	void tandy1000_write_eeprom(UINT8 data);
	void tandy1000_set_bios_bank();

	DECLARE_DRIVER_INIT(t1000hx);
	DECLARE_DRIVER_INIT(t1000sl);

	DECLARE_MACHINE_RESET(tandy1000rl);

	struct
	{
		UINT8 low, high;
	} m_eeprom_ee[0x40]; /* only 0 to 4 used in hx, addressing seems to allow this */

	int m_eeprom_state;
	int m_eeprom_clock;
	UINT8 m_eeprom_oper;
	UINT16 m_eeprom_data;

	UINT8 m_tandy_data[8];

	UINT8 m_tandy_bios_bank;    /* I/O port FFEAh */
	UINT8 m_tandy_ppi_portb, m_tandy_ppi_portc;
};

/* tandy 1000 eeprom
  hx and later
  clock, and data out lines at 0x37c
  data in at 0x62 bit 4 (0x10)

  8x read 16 bit word at x
  30 cx 30 4x 16bit 00 write 16bit at x
*/

int tandy1000_state::tandy1000_read_eeprom()
{
	if ((m_eeprom_state>=100)&&(m_eeprom_state<=199))
		return m_eeprom_data&0x8000;
	return 1;
}

void tandy1000_state::tandy1000_write_eeprom(UINT8 data)
{
	if (!m_eeprom_clock && (data&4) )
	{
//              logerror("!!!tandy1000 eeprom %.2x %.2x\n",m_eeprom_state, data);
		switch (m_eeprom_state)
		{
		case 0:
			if ((data&3)==0) m_eeprom_state++;
			break;
		case 1:
			if ((data&3)==2) m_eeprom_state++;
			break;
		case 2:
			if ((data&3)==3) m_eeprom_state++;
			break;
		case 3:
			m_eeprom_oper=data&1;
			m_eeprom_state++;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			m_eeprom_oper=(m_eeprom_oper<<1)|(data&1);
			m_eeprom_state++;
			break;
		case 10:
			m_eeprom_oper=(m_eeprom_oper<<1)|(data&1);
			logerror("!!!tandy1000 eeprom %.2x\n",m_eeprom_oper);
			if ((m_eeprom_oper&0xc0)==0x80)
			{
				m_eeprom_state=100;
				m_eeprom_data=m_eeprom_ee[m_eeprom_oper&0x3f].low
					|(m_eeprom_ee[m_eeprom_oper&0x3f].high<<8);
				logerror("!!!tandy1000 eeprom read %.2x,%.4x\n",m_eeprom_oper,m_eeprom_data);
			}
			else if ((m_eeprom_oper&0xc0)==0x40)
			{
				m_eeprom_state=200;
			}
			else
				m_eeprom_state=0;
			break;

			/* read 16 bit */
		case 100:
			m_eeprom_state++;
			break;
		case 101:
		case 102:
		case 103:
		case 104:
		case 105:
		case 106:
		case 107:
		case 108:
		case 109:
		case 110:
		case 111:
		case 112:
		case 113:
		case 114:
		case 115:
			m_eeprom_data<<=1;
			m_eeprom_state++;
			break;
		case 116:
			m_eeprom_data<<=1;
			m_eeprom_state=0;
			break;

			/* write 16 bit */
		case 200:
		case 201:
		case 202:
		case 203:
		case 204:
		case 205:
		case 206:
		case 207:
		case 208:
		case 209:
		case 210:
		case 211:
		case 212:
		case 213:
		case 214:
			m_eeprom_data=(m_eeprom_data<<1)|(data&1);
			m_eeprom_state++;
			break;
		case 215:
			m_eeprom_data=(m_eeprom_data<<1)|(data&1);
			logerror("tandy1000 %.2x %.4x written\n",m_eeprom_oper,m_eeprom_data);
			m_eeprom_ee[m_eeprom_oper&0x3f].low=m_eeprom_data&0xff;
			m_eeprom_ee[m_eeprom_oper&0x3f].high=m_eeprom_data>>8;
			m_eeprom_state=0;
			break;
		}
	}
	m_eeprom_clock=data&4;
}

WRITE8_MEMBER( tandy1000_state::pc_t1t_p37x_w )
{
//  DBG_LOG(2,"T1T_p37x_w",("%.5x #%d $%02x\n", space.device().safe_pc( ),offset, data));
	if (offset!=4)
		logerror("T1T_p37x_w %.5x #%d $%02x\n", space.device().safe_pc( ),offset, data);
	m_tandy_data[offset]=data;
	switch( offset )
	{
		case 4:
			tandy1000_write_eeprom(data);
			break;
	}
}

READ8_MEMBER( tandy1000_state::pc_t1t_p37x_r )
{
	int data = m_tandy_data[offset];
//  DBG_LOG(1,"T1T_p37x_r",("%.5x #%d $%02x\n", space.device().safe_pc( ), offset, data));
	return data;
}

/* this is for tandy1000hx
   hopefully this works for all x models
   must not be a ppi8255 chip
   (think custom chip)
   port c:
   bit 4 input eeprom data in
   bit 3 output turbo mode
*/

WRITE8_MEMBER( tandy1000_state::tandy1000_pio_w )
{
	switch (offset)
	{
	case 1:
		m_tandy_ppi_portb = data;
		m_mb->m_pit8253->write_gate2(BIT(data, 0));
		m_mb->pc_speaker_set_spkrdata( data & 0x02 );
		// sx enables keyboard from bit 3, others bit 6, hopefully theres no conflict
		m_keyboard->enable(data&0x48);
		if ( data & 0x80 )
			m_mb->m_pic8259->ir1_w(0);
		break;
	case 2:
		m_tandy_ppi_portc = data;
		if (data & 8)
			m_maincpu->set_clock_scale(1);
		else
			m_maincpu->set_clock_scale(4.77/8);
		break;
	}
}

READ8_MEMBER(tandy1000_state::tandy1000_pio_r)
{
	int data=0xff;
	switch (offset)
	{
	case 0:
		data = m_keyboard->read(space, 0);
		break;
	case 1:
		data=m_tandy_ppi_portb;
		break;
	case 2:
//      if (tandy1000hx) {
//      data=m_tandy_ppi_portc; // causes problems (setuphx)
		if (!tandy1000_read_eeprom()) data&=~0x10;
//  }
		break;
	}
	return data;
}


void tandy1000_state::tandy1000_set_bios_bank()
{
	UINT8 *p = NULL;

	assert( m_romcs0 != NULL );
	assert( m_romcs1 != NULL );
	assert( m_biosbank != NULL );

	if ( m_tandy_bios_bank & 0x10 )
	{
		if ( m_tandy_bios_bank & 0x04 )
		{
			p = m_romcs0->base() + ( m_tandy_bios_bank & 0x03 ) * 0x10000;
		}
		else
		{
			p = m_romcs1->base() + ( m_tandy_bios_bank & 0x03 ) * 0x10000;
		}
	}
	else
	{
		if ( m_tandy_bios_bank & 0x08 )
		{
			p = m_romcs0->base() + ( m_tandy_bios_bank & 0x07 ) * 0x10000;
		}
		else
		{
			p = m_romcs1->base() + ( m_tandy_bios_bank & 0x07 ) * 0x10000;
		}
	}

	m_biosbank->set_base( p );
}

MACHINE_RESET_MEMBER(tandy1000_state, tandy1000rl)
{
	m_tandy_bios_bank = 6;
	tandy1000_set_bios_bank();
}

DRIVER_INIT_MEMBER(tandy1000_state,t1000hx)
{
	machine().device<nvram_device>("nvram")->set_base(m_eeprom_ee, sizeof(m_eeprom_ee));
}


DRIVER_INIT_MEMBER(tandy1000_state,t1000sl)
{
	// Fix up memory region (highest address bit flipped??)
	UINT8 *rom = memregion("romcs0")->base();

	for( int i = 0; i < 0x40000; i++ )
	{
		UINT8 d = rom[0x40000 + i];
		rom[0x40000 + i] = rom[i];
		rom[i] = d;
	}

	DRIVER_INIT_NAME(t1000hx)();
}


READ8_MEMBER( tandy1000_state::tandy1000_bank_r )
{
	UINT8 data = 0xFF;

	logerror( "%s: tandy1000_bank_r: offset = %x\n", space.machine().describe_context(), offset );

	switch( offset )
	{
	case 0x00:  /* FFEA */
		data = m_tandy_bios_bank ^ 0x10; // Bit 4 is read back inverted
		break;
	}

	return data;
}


WRITE8_MEMBER( tandy1000_state::tandy1000_bank_w )
{
	logerror( "%s: tandy1000_bank_w: offset = %x, data = %02x\n", space.machine().describe_context(), offset, data );

	switch( offset )
	{
	case 0x00:  /* FFEA */
		m_tandy_bios_bank = data;
		tandy1000_set_bios_bank();
		break;

	// UART clock, joystick, and sound enable
	// bit 0 - 0 = Clock divided by 13
	//         1 = Clock divided by 1
	// bit 1 - 0 = Disable joystick
	//         1 = Enable joystick
	// bit 2 - 0 = Disable Sound Chip
	//         1 = Enable Sound Chip
	case 0x01:
		break;
	}
}

static INPUT_PORTS_START( tandy1t )
	PORT_START("IN0") /* IN0 */
	PORT_BIT ( 0xf0, 0xf0,   IPT_UNUSED )
	PORT_BIT ( 0x08, 0x08,   IPT_CUSTOM ) PORT_VBLANK("pcvideo_t1000:screen")
	PORT_BIT ( 0x07, 0x07,   IPT_UNUSED )

	PORT_START("DSW0") /* IN1 */
	PORT_BIT ( 0xff, 0xff,   IPT_UNUSED )

	PORT_START("DSW1") /* IN2 */
	PORT_DIPNAME( 0x80, 0x80, "COM1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "COM2: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_BIT ( 0x30, 0x00,   IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, "LPT1: enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_BIT ( 0x06, 0x00,   IPT_UNUSED )
	PORT_DIPNAME( 0x01, 0x00, "Game port enable")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW2") /* IN3 */
	PORT_DIPNAME( 0x08, 0x08, "HDC1 (C800:0 port 320-323)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "HDC2 (CA00:0 port 324-327)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_BIT( 0x02, 0x02,   IPT_UNUSED ) /* no turbo switch */
	PORT_BIT( 0x01, 0x01,   IPT_UNUSED )

	PORT_INCLUDE(pc_keyboard)

	PORT_MODIFY("pc_keyboard_2")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Up") PORT_CODE(KEYCODE_UP) /*                             29  A9 */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Left") PORT_CODE(KEYCODE_LEFT) /*                             2B  AB */

	PORT_MODIFY("pc_keyboard_3")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE   /* Caps Lock                   3A  BA */

	PORT_MODIFY("pc_keyboard_4")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK) PORT_TOGGLE /* Num Lock                    45  C5 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7 \\") PORT_CODE(KEYCODE_7_PAD) /* Keypad 7                    47  C7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8 ~") PORT_CODE(KEYCODE_8_PAD) /* Keypad 8                    48  C8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9 (PgUp)") PORT_CODE(KEYCODE_9_PAD) /* Keypad 9  (PgUp)            49  C9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Down") PORT_CODE(KEYCODE_DOWN) /*                             4A  CA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4 |") PORT_CODE(KEYCODE_4_PAD) /* Keypad 4                    4B  CB */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6") PORT_CODE(KEYCODE_6_PAD) /* Keypad 6                    4D  CD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT) /*                             4E  CE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1 (End)") PORT_CODE(KEYCODE_1_PAD) /* Keypad 1  (End)             4F  CF */

	PORT_MODIFY("pc_keyboard_5")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2 `") PORT_CODE(KEYCODE_2_PAD) /* Keypad 2                    50  D0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3 (PgDn)") PORT_CODE(KEYCODE_3_PAD) /* Keypad 3  (PgDn)            51  D1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0") PORT_CODE(KEYCODE_0_PAD) /* Keypad 0                    52  D2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP - (Del)") PORT_CODE(KEYCODE_MINUS_PAD) /* - Delete                    53  D3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_STOP) /* Break                       54  D4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ Insert") PORT_CODE(KEYCODE_PLUS_PAD) /* + Insert                    55  D5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_DEL_PAD) /* .                           56  D6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER_PAD) /* Enter                       57  D7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Home") PORT_CODE(KEYCODE_HOME) /* HOME                        58  D8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11") PORT_CODE(KEYCODE_F11) /* F11                         59  D9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12") PORT_CODE(KEYCODE_F12) /* F12                         5a  Da */
INPUT_PORTS_END

static ADDRESS_MAP_START(tandy1000_map, AS_PROGRAM, 8, tandy1000_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_RAM
	AM_RANGE(0xb0000, 0xb7fff) AM_NOP
	AM_RANGE(0xb8000, 0xbffff) AM_DEVREADWRITE("pcvideo_t1000", pcvideo_t1000_device, videoram_r, videoram_w);
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xc9fff) AM_ROM
	AM_RANGE(0xca000, 0xcffff) AM_NOP
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(tandy1000_io, AS_IO, 8, tandy1000_state )
	AM_RANGE(0x0060, 0x0063) AM_READWRITE(tandy1000_pio_r,           tandy1000_pio_w)
	AM_RANGE(0x00c0, 0x00c0) AM_DEVWRITE("sn76496", ncr7496_device, write)
	AM_RANGE(0x0200, 0x0207) AM_DEVREADWRITE("pc_joy", pc_joy_device, joy_port_r, joy_port_w)
	AM_RANGE(0x0378, 0x037f) AM_READWRITE(pc_t1t_p37x_r,         pc_t1t_p37x_w)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE("pcvideo_t1000", pcvideo_t1000_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tandy1000_16_map, AS_PROGRAM, 16, tandy1000_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_RAM
	AM_RANGE(0xb0000, 0xb7fff) AM_NOP
	AM_RANGE(0xb8000, 0xbffff) AM_DEVREADWRITE8("pcvideo_t1000", pcvideo_t1000_device, videoram_r, videoram_w, 0xffff)
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xc9fff) AM_ROM
	AM_RANGE(0xca000, 0xcffff) AM_NOP
	AM_RANGE(0xe0000, 0xeffff) AM_ROMBANK("biosbank")                     /* Banked part of the BIOS */
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION( "romcs0", 0x70000 )
ADDRESS_MAP_END

static ADDRESS_MAP_START(tandy1000_16_io, AS_IO, 16, tandy1000_state )
	AM_RANGE(0x0060, 0x0063) AM_READWRITE8(tandy1000_pio_r,          tandy1000_pio_w, 0xffff)
	AM_RANGE(0x00c0, 0x00c1) AM_DEVWRITE8("sn76496",    ncr7496_device, write, 0xffff)
	AM_RANGE(0x0200, 0x0207) AM_DEVREADWRITE8("pc_joy", pc_joy_device, joy_port_r, joy_port_w, 0xffff)
	AM_RANGE(0x0378, 0x037f) AM_READWRITE8(pc_t1t_p37x_r,            pc_t1t_p37x_w, 0xffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("pcvideo_t1000", pcvideo_t1000_device, read, write, 0xffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tandy1000_16_bank_io, AS_IO, 16, tandy1000_state )
	AM_IMPORT_FROM(tandy1000_16_io)
	AM_RANGE(0xffea, 0xffeb) AM_READWRITE8(tandy1000_bank_r, tandy1000_bank_w, 0xffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START(tandy1000_286_map, AS_PROGRAM, 16, tandy1000_state )
	ADDRESS_MAP_GLOBAL_MASK(0x000fffff)
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xaffff) AM_RAM
	AM_RANGE(0xb0000, 0xb7fff) AM_NOP
	AM_RANGE(0xb8000, 0xbffff) AM_DEVREADWRITE8("pcvideo_t1000", pcvideo_t1000_device, videoram_r, videoram_w, 0xffff)
	AM_RANGE(0xc0000, 0xc7fff) AM_NOP
	AM_RANGE(0xc8000, 0xc9fff) AM_ROM
	AM_RANGE(0xca000, 0xcffff) AM_NOP
	AM_RANGE(0xe0000, 0xeffff) AM_NOP
	AM_RANGE(0xf8000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static const gfx_layout t1000_charlayout =
{
	8, 16,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 2048, 4096, 6144, 8192, 10240, 12288, 14336, 16384, 18432, 20480, 22528, 24576, 26624, 28672, 30720 },
	8
};

static MACHINE_CONFIG_FRAGMENT( cfg_fdc_35 )
	MCFG_DEVICE_MODIFY("fdc:0")
	MCFG_SLOT_DEFAULT_OPTION("35dd")
	MCFG_SLOT_FIXED(true)

	MCFG_DEVICE_REMOVE("fdc:1")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( cfg_fdc_525 )
	MCFG_DEVICE_MODIFY("fdc:0")
	MCFG_SLOT_FIXED(true)

	MCFG_DEVICE_REMOVE("fdc:1")
MACHINE_CONFIG_END

static GFXDECODE_START( t1000 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, t1000_charlayout, 3, 1 )
GFXDECODE_END

static MACHINE_CONFIG_FRAGMENT(tandy1000_common)
	MCFG_PCNOPPI_MOTHERBOARD_ADD("mb", "maincpu")

	/* video hardware */
	MCFG_PCVIDEO_T1000_ADD("pcvideo_t1000")
	MCFG_GFXDECODE_ADD("gfxdecode", "pcvideo_t1000:palette", t1000)

	/* sound hardware */
	MCFG_SOUND_ADD("sn76496", NCR7496, XTAL_14_31818MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mb:mono", 0.80)

	MCFG_NVRAM_ADD_0FILL("nvram");

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa_fdc", pc_isa8_cards, "fdc_xt", true)
	MCFG_SLOT_OPTION_MACHINE_CONFIG("fdc_xt", cfg_fdc_35)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa_lpt", pc_isa8_cards, "lpt", true)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa_com", pc_isa8_cards, "com", true)

	MCFG_PC_JOY_ADD("pc_joy")
	MCFG_PC_KEYB_ADD("pc_keyboard", DEVWRITELINE("mb:pic8259", pic8259_device, ir1_w))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( t1000hx, tandy1000_state )
	MCFG_CPU_ADD("maincpu", I8088, 8000000)
	MCFG_CPU_PROGRAM_MAP(tandy1000_map)
	MCFG_CPU_IO_MAP(tandy1000_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_FRAGMENT_ADD(tandy1000_common)

	// plus cards are isa with a nonstandard conntector
	MCFG_ISA8_SLOT_ADD("mb:isa", "plus1", pc_isa8_cards, NULL, false)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( t1000sx, t1000hx )
	MCFG_DEVICE_MODIFY("isa_fdc")
	MCFG_SLOT_OPTION_MACHINE_CONFIG("fdc_xt", cfg_fdc_525)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, NULL, false)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5150")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( t1000_16, tandy1000_state )
	MCFG_CPU_ADD("maincpu", I8086, XTAL_28_63636MHz / 3)
	MCFG_CPU_PROGRAM_MAP(tandy1000_16_map)
	MCFG_CPU_IO_MAP(tandy1000_16_bank_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_FRAGMENT_ADD(tandy1000_common)

	MCFG_MACHINE_RESET_OVERRIDE(tandy1000_state,tandy1000rl)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( t1000_16_8, t1000_16 )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_CLOCK( XTAL_24MHz / 3 )

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, NULL, false)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( t1000_286, tandy1000_state )
	MCFG_CPU_ADD("maincpu", I80286, XTAL_28_63636MHz / 2)
	MCFG_CPU_PROGRAM_MAP(tandy1000_286_map)
	MCFG_CPU_IO_MAP(tandy1000_16_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_FRAGMENT_ADD(tandy1000_common)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, NULL, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, NULL, false)
MACHINE_CONFIG_END

#ifdef UNUSED_DEFINITION
ROM_START( t1000 )
	// Schematics displays 2 32KB ROMs at U9 and U10
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "v010000", "v010000" )
	ROMX_LOAD("v010000.f0", 0xf0000, 0x10000, NO_DUMP, ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "v010100", "v010100" )
	ROMX_LOAD("v010100.f0", 0xf0000, 0x10000, CRC(b6760881) SHA1(8275e4c48ac09cf36685db227434ca438aebe0b9), ROM_BIOS(2))

	// Part of video array at u76?
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u76", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END

ROM_START( t1000a )
	ROM_REGION(0x100000,"maincpu", 0)
	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_LOAD("v010100.f0", 0xf0000, 0x10000, CRC(b6760881) SHA1(8275e4c48ac09cf36685db227434ca438aebe0b9))

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u25", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END

ROM_START( t1000ex )
	ROM_REGION(0x100000,"maincpu", 0)
	// partlist says it has 1 128kb rom, schematics list a 32k x 8 rom
	// "8040328.u17"
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_LOAD("v010200.f0", 0xf0000, 0x10000, CRC(0e016ecf) SHA1(2f5ac8921b7cba56b02122ef772f5f11bbf6d8a2))

	// TODO: Add dump of the 8048 at u8 if it ever gets dumped
	ROM_REGION(0x400, "kbdc", 0)
	ROM_LOAD("8048.u8", 0x000, 0x400, NO_DUMP)

	// Most likely part of big blue at u28
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u28", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END

// The T1000SL and T1000SL/2 only differ in amount of RAM installed and BIOS version (SL/2 has v01.04.04)
ROM_START( t1000sl )
	ROM_REGION(0x100000,"maincpu", 0)

	// 8076312.hu1 - most likely v01.04.00
	// 8075312.hu2


	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0xe0000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_SYSTEM_BIOS( 0, "v010400", "v010400" )
	ROMX_LOAD("v010400.f0", 0xf0000, 0x10000, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v010401", "v010401" )
	ROMX_LOAD("v010401.f0", 0xf0000, 0x10000, NO_DUMP, ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "v010402", "v010402" )
	ROMX_LOAD("v010402.f0", 0xf0000, 0x10000, NO_DUMP, ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "v020001", "v020001" )
	ROMX_LOAD("v020001.f0", 0xf0000, 0x10000, NO_DUMP, ROM_BIOS(4) )

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u25", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450))
ROM_END


ROM_START( t1000tl )
	ROM_REGIoN(0x100000, "maincpu", ROMREGION_ERASE00)

	ROM_REGION(0x80000, "romcs0", 0)
	// These 2 sets most likely have the same contents
	// v01.04.00
	// 8076323.u55 - Sharp - 256KB
	// 8075323.u57 - Sharp - 256KB
	// v01.04.00
	// 8079025.u54 - Hitachi - 256KB
	// 8079026.u56 - Hitachi - 256KB
	ROM_REGION(0x80000, "romcs1", 0)

	// 2x 128x8 eeprom?? @ u58 and u59 - not mentioned in parts list

	ROM_REGION(0x80, "eeprom", 0)
	ROM_LOAD("8040346_9346.u12", xxx ) // 64x16 eeprom

	ROM_REGION(0x08000, "gfx1", 0)
	ROM_LOAD("8079027.u24", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450))
ROM_END
#endif

ROM_START( t1000hx )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("v020000.u12", 0xe0000, 0x20000, CRC(6f3acd80) SHA1(976af8c04c3f6fde14d7047f6521d302bdc2d017)) // TODO: Rom label

	// TODO: Add dump of the 8048 at u9 if it ever gets dumped
	ROM_REGION(0x400, "kbdc", 0)
	ROM_LOAD("8048.u9", 0x000, 0x400, NO_DUMP)

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u31", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location, probably internal to "big blue" at u31
ROM_END

ROM_START( t1000sx )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("8040328.u41", 0xf8000, 0x8000, CRC(4e2b9f0b) SHA1(e79a9ed9e885736e30d9b135557f0e596ce5a70b))

	// No character rom is listed in the schematics?
	// But disabling it results in no text being printed
	// Part of bigblue at u30??
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u30", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


ROM_START( t1000tx )
	ROM_REGION(0x100000,"maincpu", 0)
	// There should be 2 32KBx8 ROMs, one for odd at u38, one for even at u39
	// The machine already boots up with just this one rom
	ROM_LOAD("t1000tx.bin", 0xf8000, 0x8000, BAD_DUMP CRC(9b34765c) SHA1(0b07e87f6843393f7d4ca4634b832b0c0bec304e))

	// No character rom is listed in the schematics?
	// It is most likely part of the big blue chip at u36
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u36", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


ROM_START( t1000rl )
	ROM_REGION(0x100000,"maincpu", ROMREGION_ERASE00)

	// bankable ROM regions
	ROM_REGION(0x80000, "romcs0", 0)
	/* v2.0.0.1 */
	/* Rom is labeled "(C) TANDY CORP. 1990 // 8079073 // LH534G70 JAPAN // 9034 D" */
	ROM_LOAD("8079073.u23", 0x00000, 0x80000, CRC(6fab50f7) SHA1(2ccc02bee4c250dc1b7c17faef2590bc158860b0) )
	ROM_REGION(0x80000, "romcs1", ROMREGION_ERASEFF)

	ROM_REGION(0x08000,"gfx1", 0)
	/* Character rom located at U3 w/label "8079027 // NCR // 609-2495004 // F841030 A9025" */
	ROM_LOAD("8079027.u3", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


ROM_START( t1000sl2 )
	ROM_REGION(0x100000,"maincpu", ROMREGION_ERASE00)

	// bankable ROM regions
	ROM_REGION(0x80000, "romcs0", 0)
	// v01.04.04 BIOS
	ROM_LOAD16_BYTE("8079047.hu1", 0x00000, 0x40000, CRC(c773ec0e) SHA1(7deb71f14c2c418400b639d60066ab61b7e9df32))
	ROM_LOAD16_BYTE("8079048.hu2", 0x00001, 0x40000, CRC(0f3e6586) SHA1(10f1a7204f69b82a18bc94a3010c9660aec0c802))
	ROM_REGION(0x80000, "romcs1", ROMREGION_ERASEFF)

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u25", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450))

	ROM_REGION(0x80, "nmc9246n", 0)
	ROM_LOAD("seeprom.bin", 0, 0x80, CRC(4fff41df) SHA1(41a7009694550c017996932beade608cff968f4a))
ROM_END


ROM_START( t1000tl2 )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD( "t10000tl2.bin", 0xf0000, 0x10000, CRC(e288f12c) SHA1(9d54ccf773cd7202c9906323f1b5a68b1b3a3a67))

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u24", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


/*    YEAR  NAME        PARENT      COMPAT      MACHINE     INPUT       INIT        COMPANY            FULLNAME */
// tandy 1000
COMP( 1987, t1000hx,    ibm5150,    0,          t1000hx,    tandy1t, tandy1000_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 HX", 0)
COMP( 1987, t1000sx,    ibm5150,    0,          t1000sx,    tandy1t, tandy1000_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 SX", 0)
COMP( 1987, t1000tx,    ibm5150,    0,          t1000_286,  tandy1t, tandy1000_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 TX", 0)
COMP( 1989, t1000rl,    ibm5150,    0,          t1000_16,   tandy1t, tandy1000_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 RL", 0)
COMP( 1989, t1000tl2,   ibm5150,    0,          t1000_286,  tandy1t, tandy1000_state,    t1000hx,    "Tandy Radio Shack", "Tandy 1000 TL/2", 0)
COMP( 1988, t1000sl2,   ibm5150,    0,          t1000_16_8, tandy1t, tandy1000_state,    t1000sl,    "Tandy Radio Shack", "Tandy 1000 SL/2", 0)
