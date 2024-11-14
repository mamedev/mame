// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
Tandy 1000
==========

Tandy 1000 machines are similar to the IBM 5160s with CGA graphics. Tandy
added some additional graphic capabilities similar, but not equal, to
those added for the IBM PC Jr.

The Tandy 1000's features include a pair of 6-pin mini-DIN connectors for
analog joysticks. They are compatible with CoCo joysticks but not standard
PC joysticks, though the software interface is identical. This difference
is not accurately reflected in the current slot device configuration.

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
1000RSX/1000RSX-HD  1M-9M RAM           25.0/8.0 MHz    v01.10.00

According to http://nerdlypleasures.blogspot.com/2014/04/the-original-8-bit-ide-interface.html
the 286 based Tandy 1000 TL/2, TL/3, RLX, RLX-B and the 8086 based Tandy 1000RL & RL-HD
used XTA (8-bit IDE) harddisks.
*/

#include "emu.h"
#include "machine/genpc.h"
#include "machine/pckeybrd.h"
#include "machine/nvram.h"
#include "machine/bankdev.h"
#include "pc_t1t.h"
#include "sound/sn76496.h"
#include "cpu/i86/i86.h"
#include "cpu/i86/i286.h"
#include "bus/pc_joy/pc_joy.h"
#include "screen.h"
#include "softlist_dev.h"

DECLARE_DEVICE_TYPE(T1000_MOTHERBOARD, t1000_mb_device)

class t1000_mb_device : public pc_noppi_mb_device
{
public:
	// construction/destruction
	t1000_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: pc_noppi_mb_device(mconfig, T1000_MOTHERBOARD, tag, owner, clock)
	{ }
protected:
	virtual void device_start() override ATTR_COLD;
};

void t1000_mb_device::device_start()
{
}

DEFINE_DEVICE_TYPE(T1000_MOTHERBOARD, t1000_mb_device, "t1000_mb", "Tandy 1000 Motherboard")

class tandy1000_state : public driver_device
{
public:
	tandy1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_biosbank(*this, "biosbank")
		, m_keyboard(*this, "pc_keyboard")
		, m_mb(*this, "mb")
		, m_video(*this, "pcvideo_t1000")
		, m_ram(*this, RAM_TAG)
		, m_vram_bank(0) { }

	void tandy1000_common(machine_config &config);
	void tandy1000_90key(machine_config &config);
	void tandy1000_101key(machine_config &config);
	void t1000tl(machine_config &config);
	void t1000tl2(machine_config &config);
	void t1000sx(machine_config &config);
	void t1000rl(machine_config &config);
	void t1000sl2(machine_config &config);
	void t1000hx(machine_config &config);
	void t1000tx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

// Memory regions for the machines that support rom banking
	optional_device<address_map_bank_device> m_biosbank;

	required_device<pc_keyboard_device> m_keyboard;
	required_device<t1000_mb_device> m_mb;
	required_device<pcvideo_t1000_device> m_video;
	required_device<ram_device> m_ram;

	void pc_t1t_p37x_w(offs_t offset, uint8_t data);
	uint8_t pc_t1t_p37x_r(offs_t offset);

	void tandy1000_pio_w(offs_t offset, uint8_t data);
	uint8_t tandy1000_pio_r(offs_t offset);
	uint8_t tandy1000_bank_r(offs_t offset);
	void tandy1000_bank_w(offs_t offset, uint8_t data);
	void nmi_vram_bank_w(uint8_t data);
	void vram_bank_w(uint8_t data);
	uint8_t vram_r(offs_t offset);
	void vram_w(offs_t offset, uint8_t data);
	void devctrl_w(uint8_t data);
	uint8_t unk_r();

	int tandy1000_read_eeprom();
	void tandy1000_write_eeprom(uint8_t data);
	void tandy1000_set_bios_bank();

	DECLARE_MACHINE_RESET(tandy1000rl);

	struct
	{
		uint8_t low = 0, high = 0;
	} m_eeprom_ee[0x40]; /* only 0 to 4 used in hx, addressing seems to allow this */

	int m_eeprom_state = 0;
	int m_eeprom_clock = 0;
	uint8_t m_eeprom_oper = 0;
	uint16_t m_eeprom_data = 0;

	uint8_t m_tandy_data[8]{};

	uint8_t m_tandy_bios_bank = 0;    /* I/O port FFEAh */
	uint8_t m_tandy_ppi_porta = 0, m_tandy_ppi_ack = 0;
	uint8_t m_tandy_ppi_portb = 0, m_tandy_ppi_portc = 0;
	uint8_t m_vram_bank = 0;
	static void cfg_fdc_35(device_t *device);
	static void cfg_fdc_525(device_t *device);

	void biosbank_map(address_map &map) ATTR_COLD;
	void tandy1000_16_io(address_map &map) ATTR_COLD;
	void tandy1000_286_map(address_map &map) ATTR_COLD;
	void tandy1000_286_bank_map(address_map &map) ATTR_COLD;
	void tandy1000_bank_io(address_map &map) ATTR_COLD;
	void tandy1000_bank_map(address_map &map) ATTR_COLD;
	void tandy1000_io(address_map &map) ATTR_COLD;
	void tandy1000_map(address_map &map) ATTR_COLD;
	void tandy1000tx_io(address_map &map) ATTR_COLD;
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

void tandy1000_state::tandy1000_write_eeprom(uint8_t data)
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

void tandy1000_state::pc_t1t_p37x_w(offs_t offset, uint8_t data)
{
//  DBG_LOG(2,"T1T_p37x_w",("%.5x #%d $%02x\n", m_maincpu->pc( ),offset, data));
	if (offset!=4)
		logerror("T1T_p37x_w %.5x #%d $%02x\n", m_maincpu->pc( ),offset, data);
	m_tandy_data[offset]=data;
	switch( offset )
	{
		case 4:
			tandy1000_write_eeprom(data);
			break;
	}
}

uint8_t tandy1000_state::pc_t1t_p37x_r(offs_t offset)
{
	int data = m_tandy_data[offset];
//  DBG_LOG(1,"T1T_p37x_r",("%.5x #%d $%02x\n", m_maincpu->pc( ), offset, data));
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

void tandy1000_state::tandy1000_pio_w(offs_t offset, uint8_t data)
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
		{
			m_mb->m_pic8259->ir1_w(0);
			m_tandy_ppi_ack = 1;
		}
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

uint8_t tandy1000_state::tandy1000_pio_r(offs_t offset)
{
	int data=0xff;
	switch (offset)
	{
	case 0:
		if (m_tandy_ppi_ack)
		{
			m_tandy_ppi_porta = m_keyboard->read();
			m_tandy_ppi_ack = 0;
		}
		data = m_tandy_ppi_porta;
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

void tandy1000_state::nmi_vram_bank_w(uint8_t data)
{
	m_mb->nmi_enable_w(data & 0x80);
	vram_bank_w(data & 0x1e);
	m_video->disable_w((data & 1) ? ASSERT_LINE : CLEAR_LINE);
}

void tandy1000_state::vram_bank_w(uint8_t data)
{
	m_vram_bank = (data >> 1) & 7;
}

void tandy1000_state::devctrl_w(uint8_t data)
{
	m_video->disable_w((data & 4) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t tandy1000_state::unk_r()
{
	return 0; // status port?
}

uint8_t tandy1000_state::vram_r(offs_t offset)
{
	uint8_t vram_base = (m_ram->size() >> 17) - 1;
	if((m_vram_bank - vram_base) == (offset >> 17))
		return m_ram->pointer()[offset & 0x1ffff];
	return 0xff;
}

void tandy1000_state::vram_w(offs_t offset, uint8_t data)
{
	uint8_t vram_base = (m_ram->size() >> 17) - 1;
	if((m_vram_bank - vram_base) == (offset >> 17))
		m_ram->pointer()[offset & 0x1ffff] = data;
}

void tandy1000_state::tandy1000_set_bios_bank()
{
	int bank;

	if ( m_tandy_bios_bank & 0x10 )
	{
		if (m_tandy_bios_bank & 0x04 )
		{
			bank = (m_tandy_bios_bank & 3) + 8;
		}
		else
		{
			bank = m_tandy_bios_bank & 3;
		}
	}
	else
	{
		bank = m_tandy_bios_bank & 0x0f;
	}

	m_biosbank->set_bank( bank );
}

MACHINE_RESET_MEMBER(tandy1000_state, tandy1000rl)
{
	m_tandy_bios_bank = 6;
	tandy1000_set_bios_bank();
}

void tandy1000_state::machine_start()
{
	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - (128*1024) - 1, &m_ram->pointer()[128*1024]);
	if(m_maincpu->space(AS_PROGRAM).data_width() == 8)
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(m_ram->size() - (128*1024), 640*1024 - 1,
				read8sm_delegate(*this, FUNC(tandy1000_state::vram_r)), write8sm_delegate(*this, FUNC(tandy1000_state::vram_w)));
	else
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(m_ram->size() - (128*1024), 640*1024 - 1,
				read8sm_delegate(*this, FUNC(tandy1000_state::vram_r)), write8sm_delegate(*this, FUNC(tandy1000_state::vram_w)), 0xffff);
	subdevice<nvram_device>("nvram")->set_base(m_eeprom_ee, sizeof(m_eeprom_ee));

	m_eeprom_state = 0;
}

uint8_t tandy1000_state::tandy1000_bank_r(offs_t offset)
{
	uint8_t data = 0xFF;

	logerror( "%s: tandy1000_bank_r: offset = %x\n", machine().describe_context(), offset );

	switch( offset )
	{
	case 0x00:  /* FFEA */
		data = m_tandy_bios_bank ^ 0x10; // Bit 4 is read back inverted
		break;
	}

	return data;
}


void tandy1000_state::tandy1000_bank_w(offs_t offset, uint8_t data)
{
	logerror( "%s: tandy1000_bank_w: offset = %x, data = %02x\n", machine().describe_context(), offset, data );

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

static INPUT_PORTS_START( t1000 )
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
INPUT_PORTS_END

/* 90-key Tandy Keyboard layout, used on earlier models
   later models use the Tandy Enhanced Keyboard with a standard 101-key Enhanced AT layout */
static INPUT_PORTS_START( t1000_keyboard )
	PORT_INCLUDE(pc_keyboard)

	PORT_MODIFY("pc_keyboard_2")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Up") PORT_CODE(KEYCODE_UP) /*                             29  A9 */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Left") PORT_CODE(KEYCODE_LEFT) /*                             2B  AB */

	PORT_MODIFY("pc_keyboard_3")
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE   /* Caps Lock                   3A  BA */

	PORT_MODIFY("pc_keyboard_4")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK) PORT_TOGGLE /* Num Lock                    45  C5 */
	/* Hold corresponds to Scroll Lock, but pauses the system when pressed - leaving unmapped by default to avoid conflicting with the UI Toggle key */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Hold")     /*                            46  C6 */
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

class t1000_keyboard_device : public pc_keyboard_device
{
public:
	t1000_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

DEFINE_DEVICE_TYPE(T1000_KEYB, t1000_keyboard_device, "t1000_keyb", "Tandy 1000 Keyboard")

t1000_keyboard_device::t1000_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pc_keyboard_device(mconfig, T1000_KEYB, tag, owner, clock)
{
	m_type = KEYBOARD_TYPE::PC;
}

ioport_constructor t1000_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(t1000_keyboard);
}



void tandy1000_state::tandy1000_map(address_map &map)
{
	map.unmap_value_high();
	map(0xb8000, 0xbffff).m("pcvideo_t1000:vram", FUNC(address_map_bank_device::amap8));
	map(0xe0000, 0xfffff).rom().region("bios", 0);
}

void tandy1000_state::tandy1000_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(t1000_mb_device::map));
	map(0x0060, 0x0063).rw(FUNC(tandy1000_state::tandy1000_pio_r), FUNC(tandy1000_state::tandy1000_pio_w));
	map(0x00a0, 0x00a0).w(FUNC(tandy1000_state::nmi_vram_bank_w));
	map(0x00c0, 0x00c0).w("sn76496", FUNC(ncr8496_device::write));
	map(0x0200, 0x0207).rw("pc_joy", FUNC(pc_joy_device::joy_port_r), FUNC(pc_joy_device::joy_port_w));
	map(0x0378, 0x037f).rw(FUNC(tandy1000_state::pc_t1t_p37x_r), FUNC(tandy1000_state::pc_t1t_p37x_w));
	map(0x03d0, 0x03df).r(m_video, FUNC(pcvideo_t1000_device::read)).w(m_video, FUNC(pcvideo_t1000_device::write));
}

void tandy1000_state::tandy1000_bank_map(address_map &map)
{
	map.unmap_value_high();
	map(0xb8000, 0xbffff).m("pcvideo_t1000:vram", FUNC(address_map_bank_device::amap8));
	map(0xe0000, 0xeffff).m(m_biosbank, FUNC(address_map_bank_device::amap16));
	map(0xf0000, 0xfffff).rom().region("rom", 0x70000);
}

void tandy1000_state::biosbank_map(address_map &map)
{
	map.unmap_value_high();
	map(0x80000, 0xfffff).rom().region("rom", 0);
}

void tandy1000_state::tandy1000_16_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).m(m_mb, FUNC(t1000_mb_device::map));
	map(0x0060, 0x0063).rw(FUNC(tandy1000_state::tandy1000_pio_r), FUNC(tandy1000_state::tandy1000_pio_w));
	map(0x0065, 0x0065).w(FUNC(tandy1000_state::devctrl_w));
	map(0x00a0, 0x00a0).r(FUNC(tandy1000_state::unk_r));
	map(0x00c0, 0x00c1).w("sn76496", FUNC(ncr8496_device::write));
	map(0x0200, 0x0207).rw("pc_joy", FUNC(pc_joy_device::joy_port_r), FUNC(pc_joy_device::joy_port_w));
	map(0x0378, 0x037f).rw(FUNC(tandy1000_state::pc_t1t_p37x_r), FUNC(tandy1000_state::pc_t1t_p37x_w));
	map(0x03d0, 0x03df).r(m_video, FUNC(pcvideo_t1000_device::read)).w(m_video, FUNC(pcvideo_t1000_device::write));
	map(0xffe8, 0xffe8).w(FUNC(tandy1000_state::vram_bank_w));
}

void tandy1000_state::tandy1000_bank_io(address_map &map)
{
	map.unmap_value_high();
	tandy1000_16_io(map);
	map(0xffea, 0xffeb).rw(FUNC(tandy1000_state::tandy1000_bank_r), FUNC(tandy1000_state::tandy1000_bank_w));
}

void tandy1000_state::tandy1000tx_io(address_map &map)
{
	map.unmap_value_high();
	tandy1000_16_io(map);
	map(0x00a0, 0x00a0).w(FUNC(tandy1000_state::nmi_vram_bank_w));
}

void tandy1000_state::tandy1000_286_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x000fffff);
	map(0xb8000, 0xbffff).m("pcvideo_t1000:vram", FUNC(address_map_bank_device::amap8));
	map(0xe0000, 0xfffff).rom().region("bios", 0);
}

void tandy1000_state::tandy1000_286_bank_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x000fffff);
	map(0xb8000, 0xbffff).m("pcvideo_t1000:vram", FUNC(address_map_bank_device::amap8));
	map(0xe0000, 0xeffff).m(m_biosbank, FUNC(address_map_bank_device::amap16));
	map(0xf0000, 0xfffff).rom().region("rom", 0x70000);
}

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


void tandy1000_state::cfg_fdc_35(device_t *device)
{
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:0")).set_default_option("35dd");
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:0")).set_fixed(true);
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:1")).set_default_option(nullptr);
}

void tandy1000_state::cfg_fdc_525(device_t *device)
{
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:0")).set_fixed(true);
	dynamic_cast<device_slot_interface &>(*device->subdevice("fdc:1")).set_default_option(nullptr);
}

static GFXDECODE_START( gfx_t1000 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, t1000_charlayout, 3, 1 )
GFXDECODE_END

void tandy1000_state::tandy1000_common(machine_config &config)
{
	T1000_MOTHERBOARD(config, m_mb, 0);
	m_mb->set_cputag("maincpu");
	m_mb->int_callback().set_inputline("maincpu", 0);
	m_mb->nmi_callback().set_inputline("maincpu", INPUT_LINE_NMI);

	/* video hardware */
	PCVIDEO_T1000(config, m_video, 0);
	m_video->set_screen("pcvideo_t1000:screen");
	GFXDECODE(config, "gfxdecode", "pcvideo_t1000:palette", gfx_t1000);

	/* sound hardware */
	NCR8496(config, "sn76496", XTAL(14'318'181)/4).add_route(ALL_OUTPUTS, "mb:mono", 0.80);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	isa8_slot_device &isa_fdc(ISA8_SLOT(config, "isa_fdc", 0, "mb:isa", pc_isa8_cards, "fdc_xt", true)); // FIXME: determine ISA bus clock
	isa_fdc.set_option_machine_config("fdc_xt", cfg_fdc_35);

	ISA8_SLOT(config, "isa_lpt", 0, "mb:isa", pc_isa8_cards, "lpt", true);
	ISA8_SLOT(config, "isa_com", 0, "mb:isa", pc_isa8_cards, "com", true);

	PC_JOY(config, "pc_joy");

	/* internal ram */
	RAM(config, m_ram).set_default_size("640K");

	SOFTWARE_LIST(config, "disk_list").set_original("t1000");
	SOFTWARE_LIST(config, "pc_list").set_compatible("ibm5150");
}

void tandy1000_state::tandy1000_90key(machine_config &config)
{
	T1000_KEYB(config, m_keyboard);
	m_keyboard->keypress().set("mb:pic8259", FUNC(pic8259_device::ir1_w));
}

void tandy1000_state::tandy1000_101key(machine_config &config)
{
	AT_KEYB(config, m_keyboard, pc_keyboard_device::KEYBOARD_TYPE::AT, 1);
	m_keyboard->keypress().set("mb:pic8259", FUNC(pic8259_device::ir1_w));
}

void tandy1000_state::t1000hx(machine_config &config)
{
	I8088(config, m_maincpu, 8000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &tandy1000_state::tandy1000_map);
	m_maincpu->set_addrmap(AS_IO, &tandy1000_state::tandy1000_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	tandy1000_common(config);

	tandy1000_90key(config);

	// plus cards are isa with a nonstandard conntector
	ISA8_SLOT(config, "plus1", 0, "mb:isa", pc_isa8_cards, nullptr, false); // FIXME: determine ISA bus clock

	m_ram->set_extra_options("256K, 384K");
}

void tandy1000_state::t1000sx(machine_config &config)
{
	t1000hx(config);
	subdevice<isa8_slot_device>("isa_fdc")->set_option_machine_config("fdc_xt", cfg_fdc_525);

	ISA8_SLOT(config, "isa1", 0, "mb:isa", pc_isa8_cards, nullptr, false); // FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa2", 0, "mb:isa", pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa3", 0, "mb:isa", pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa4", 0, "mb:isa", pc_isa8_cards, nullptr, false);

	m_ram->set_extra_options("384K");
}

void tandy1000_state::t1000rl(machine_config &config)
{
	I8086(config, m_maincpu, XTAL(28'636'363) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &tandy1000_state::tandy1000_bank_map);
	m_maincpu->set_addrmap(AS_IO, &tandy1000_state::tandy1000_bank_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	tandy1000_common(config);

	tandy1000_101key(config);

	ADDRESS_MAP_BANK(config, "biosbank").set_map(&tandy1000_state::biosbank_map).set_options(ENDIANNESS_LITTLE, 16, 20, 0x10000);

	MCFG_MACHINE_RESET_OVERRIDE(tandy1000_state,tandy1000rl)

	m_ram->set_extra_options("384K");
}

void tandy1000_state::t1000sl2(machine_config &config)
{
	t1000rl(config);
	m_maincpu->set_clock(XTAL(24'000'000) / 3);

	ISA8_SLOT(config, "isa1", 0, "mb:isa", pc_isa8_cards, nullptr, false); // FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa2", 0, "mb:isa", pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa3", 0, "mb:isa", pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa4", 0, "mb:isa", pc_isa8_cards, nullptr, false);
}

void tandy1000_state::t1000tl2(machine_config &config)
{
	I80286(config, m_maincpu, XTAL(28'636'363) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &tandy1000_state::tandy1000_286_map);
	m_maincpu->set_addrmap(AS_IO, &tandy1000_state::tandy1000_16_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	tandy1000_common(config);

	tandy1000_101key(config);

	ISA8_SLOT(config, "isa1", 0, "mb:isa", pc_isa8_cards, nullptr, false); // FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa2", 0, "mb:isa", pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa3", 0, "mb:isa", pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa4", 0, "mb:isa", pc_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa5", 0, "mb:isa", pc_isa8_cards, nullptr, false);
}

void tandy1000_state::t1000tl(machine_config &config)
{
	t1000tl2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &tandy1000_state::tandy1000_286_bank_map);
	m_maincpu->set_addrmap(AS_IO, &tandy1000_state::tandy1000_bank_io);

	ADDRESS_MAP_BANK(config, "biosbank").set_map(&tandy1000_state::biosbank_map).set_options(ENDIANNESS_LITTLE, 16, 20, 0x10000);
}

void tandy1000_state::t1000tx(machine_config &config)
{
	t1000tl2(config);
	m_maincpu->set_addrmap(AS_IO, &tandy1000_state::tandy1000tx_io);

	config.device_remove("pc_keyboard");
	tandy1000_90key(config);
}

#ifdef UNUSED_DEFINITION
ROM_START( t1000 )
	// Schematic shows 2 32KB ROMs at U9 and U10 for Tandy 1000; 1000A is a different mainboard.
	ROM_REGION(0x20000,"bios", 0)
	ROM_SYSTEM_BIOS( 0, "v010000", "v010000" )
	ROMX_LOAD("v010000.f0", 0x10000, 0x10000, NO_DUMP, ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v010100", "v010100" )
	ROMX_LOAD("v010100.f0", 0x10000, 0x10000, CRC(b6760881) SHA1(8275e4c48ac09cf36685db227434ca438aebe0b9), ROM_BIOS(1))

	// Part of video array at u76?
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u76", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END

ROM_START( t1000a )
	ROM_REGION(0x20000,"bios", 0)
	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0x00000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_LOAD("v010100.f0", 0x10000, 0x10000, CRC(b6760881) SHA1(8275e4c48ac09cf36685db227434ca438aebe0b9))

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u25", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END

ROM_START( t1000ex )
	ROM_REGION(0x20000,"bios", 0)
	// partlist says it has 1 128kb rom, schematics list a 32k x 8 rom
	// "8040328.u17"
	ROM_LOAD("t1000hx.e0", 0x00000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_LOAD("v010200.f0", 0x10000, 0x10000, CRC(0e016ecf) SHA1(2f5ac8921b7cba56b02122ef772f5f11bbf6d8a2))

	// TODO: Add dump of the 8048 at u8 if it ever gets dumped
	ROM_REGION(0x400, "kbdc", 0)
	ROM_LOAD("8048.u8", 0x000, 0x400, NO_DUMP)

	// Most likely part of big blue at u28
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u28", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END

// The T1000SL and T1000SL/2 only differ in amount of RAM installed and BIOS version (SL/2 has v01.04.04)
ROM_START( t1000sl )
	ROM_REGION(0x20000,"bios", 0)

	// 8076312.hu1 - most likely v01.04.00
	// 8075312.hu2


	// partlist says it has 1 128kbyte rom
	ROM_LOAD("t1000hx.e0", 0x00000, 0x10000, CRC(61dbf242) SHA1(555b58d8aa8e0b0839259621c44b832d993beaef))  // not sure about this one
	ROM_SYSTEM_BIOS( 0, "v010400", "v010400" )
	ROMX_LOAD("v010400.f0", 0x10000, 0x10000, NO_DUMP, ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v010401", "v010401" )
	ROMX_LOAD("v010401.f0", 0x10000, 0x10000, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "v010402", "v010402" )
	ROMX_LOAD("v010402.f0", 0x10000, 0x10000, NO_DUMP, ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "v020001", "v020001" )
	ROMX_LOAD("v020001.f0", 0x10000, 0x10000, NO_DUMP, ROM_BIOS(3) )

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u25", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450))
ROM_END


ROM_START( t1000tl )
	ROM_REGIoN(0x10000, "bios", ROMREGION_ERASE00)

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
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD("v020000.u12", 0x00000, 0x20000, CRC(6f3acd80) SHA1(976af8c04c3f6fde14d7047f6521d302bdc2d017)) // TODO: Rom label

	// TODO: Add dump of the 8048 at u9 if it ever gets dumped
	ROM_REGION(0x400, "kbdc", 0)
	ROM_LOAD("8048.u9", 0x000, 0x400, NO_DUMP)

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u31", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location, probably internal to "big blue" at u31
ROM_END

ROM_START( t1000sx )
	ROM_REGION(0x20000,"bios", 0)
	ROM_LOAD("8040328.u41", 0x18000, 0x8000, CRC(4e2b9f0b) SHA1(e79a9ed9e885736e30d9b135557f0e596ce5a70b))

	// No character rom is listed in the schematics?
	// But disabling it results in no text being printed
	// Part of bigblue at u30??
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u30", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


ROM_START( t1000tx )
	ROM_REGION16_LE(0x20000, "bios", 0)
	// There should be 2 32KBx8 ROMs, one for odd at u38, one for even at u39
	// The machine already boots up with just this one rom
	ROM_LOAD("t1000tx.bin", 0x18000, 0x8000, BAD_DUMP CRC(9b34765c) SHA1(0b07e87f6843393f7d4ca4634b832b0c0bec304e))

	// No character rom is listed in the schematics?
	// It is most likely part of the big blue chip at u36
	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u36", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


ROM_START( t1000rl )
	// bankable ROM regions
	ROM_REGION16_LE(0x80000, "rom", 0)
	/* v2.0.0.1 */
	/* Rom is labeled "(C) TANDY CORP. 1990 // 8079073 // LH534G70 JAPAN // 9034 D" */
	ROM_LOAD("8079073.u23", 0x00000, 0x80000, CRC(6fab50f7) SHA1(2ccc02bee4c250dc1b7c17faef2590bc158860b0) )

	ROM_REGION(0x08000,"gfx1", 0)
	/* Character rom located at U3 w/label "8079027 // NCR // 609-2495004 // F841030 A9025" */
	ROM_LOAD("8079027.u3", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


ROM_START( t1000sl2 )
	// bankable ROM regions
	ROM_REGION16_LE(0x80000, "rom", 0)
	// v01.04.04 BIOS
	// Fix up memory region (highest address bit flipped??)
	ROM_LOAD16_BYTE("8079047.hu1", 0x40000, 0x20000, CRC(c773ec0e) SHA1(7deb71f14c2c418400b639d60066ab61b7e9df32))
	ROM_CONTINUE(0x00000, 0x20000)
	ROM_LOAD16_BYTE("8079048.hu2", 0x40001, 0x20000, CRC(0f3e6586) SHA1(10f1a7204f69b82a18bc94a3010c9660aec0c802))
	ROM_CONTINUE(0x00001, 0x20000)

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u25", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450))

	ROM_REGION(0x80, "nmc9246n", 0)
	ROM_LOAD("seeprom.bin", 0, 0x80, CRC(4fff41df) SHA1(41a7009694550c017996932beade608cff968f4a))
ROM_END


ROM_START( t1000tl )
	ROM_REGION16_LE(0x80000, "rom", 0)
	// v01.04.02 BIOS
	ROM_LOAD16_BYTE( "8079037.u55", 0x00000, 0x40000, CRC(869dd92e) SHA1(91422085ef541fefede8fcc9a454d0538298d087)) // ┬®TANDY CORP.1988 8079037 LH532370 8849 D.A-EVEN.U55
	ROM_LOAD16_BYTE( "8079038.u57", 0x00001, 0x40000, CRC(9520db5b) SHA1(9416e85cfcf04b18afc2efb8021a1ed79357ad33)) // ┬®TANDY CORP.1988 8079038 LH532371 8848 D.A-ODD.U57

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u24", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // 8079027 NCR 609-2495004 F831628 A8834
ROM_END


ROM_START( t1000tl2 )
	ROM_REGION16_LE(0x20000, "bios", 0)
	ROM_LOAD( "t10000tl2.bin", 0x10000, 0x10000, CRC(e288f12c) SHA1(9d54ccf773cd7202c9906323f1b5a68b1b3a3a67))

	ROM_REGION(0x08000,"gfx1", 0)
	ROM_LOAD("8079027.u24", 0x00000, 0x04000, CRC(33d64a11) SHA1(b63da2a656b6c0a8a32f2be8bdcb51aed983a450)) // TODO: Verify location
ROM_END


// tandy 1000
//    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT  CLASS            INIT        COMPANY              FULLNAME           FLAGS
COMP( 1987, t1000hx,  ibm5150, 0,      t1000hx,  t1000, tandy1000_state, empty_init, "Tandy Radio Shack", "Tandy 1000 HX",   0 )
COMP( 1987, t1000sx,  ibm5150, 0,      t1000sx,  t1000, tandy1000_state, empty_init, "Tandy Radio Shack", "Tandy 1000 SX",   0 )
COMP( 1987, t1000tx,  ibm5150, 0,      t1000tx,  t1000, tandy1000_state, empty_init, "Tandy Radio Shack", "Tandy 1000 TX",   0 )
COMP( 1989, t1000rl,  ibm5150, 0,      t1000rl,  t1000, tandy1000_state, empty_init, "Tandy Radio Shack", "Tandy 1000 RL",   0 )
COMP( 1988, t1000tl,  ibm5150, 0,      t1000tl,  t1000, tandy1000_state, empty_init, "Tandy Radio Shack", "Tandy 1000 TL",   0 )
COMP( 1989, t1000tl2, ibm5150, 0,      t1000tl2, t1000, tandy1000_state, empty_init, "Tandy Radio Shack", "Tandy 1000 TL/2", 0 )
COMP( 1988, t1000sl2, ibm5150, 0,      t1000sl2, t1000, tandy1000_state, empty_init, "Tandy Radio Shack", "Tandy 1000 SL/2", 0 )
