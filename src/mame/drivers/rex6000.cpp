// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Xircom / Intel REX 6000

        Driver by Sandro Ronco

        Known REX 6000 banks:
        0x0000 - 0x00ff     first flash chip
        0x0600 - 0x06ff     second flash chip
        0x1000 - 0x1003     32KB RAM

        Known DataSlim 2 banks:
        0x0000 - 0x00ff     first flash chip
        0x0b00 - 0x0bff     second flash chip
        0x1000 - 0x1003     32KB RAM

        TODO:
        - deleting all Memos causes an error
        - alarm doesn't work
        - alarm sound and keyclick
        - NVRAM and warm start (without the pen calibration)
        - serial I/O

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/rp5c01.h"
#include "machine/ram.h"
#include "machine/intelfsh.h"
#include "imagedev/snapquik.h"
#include "sound/beep.h"
#include "rendlay.h"

#define MAKE_BANK(lo, hi)       ((lo) | ((hi)<<8))

//irq flag/mask bits
#define IRQ_FLAG_KEYCHANGE  0x01
#define IRQ_FLAG_ALARM      0x02
#define IRQ_FLAG_SERIAL     0x04
#define IRQ_FLAG_1HZ        0x10
#define IRQ_FLAG_IRQ2       0x20
#define IRQ_FLAG_IRQ1       0x40
#define IRQ_FLAG_EVENT      0x80

//memory bank types
#define BANK_FLASH0_B0      0x00
#define BANK_FLASH0_B1      0x01
#define BANK_FLASH1_B0      0x02
#define BANK_FLASH1_B1      0x03
#define BANK_RAM            0x10
#define BANK_UNKNOWN        0xff

#define TC8521_TAG  "rtc"

class rex6000_state : public driver_device
{
public:
	rex6000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_ram(*this, RAM_TAG),
			m_beep(*this, "beeper")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<beep_device> m_beep;
	fujitsu_29dl16x_device *m_flash[4];

	UINT8 m_bank[4];
	UINT8 m_beep_io[5];
	UINT8 m_lcd_base[2];
	UINT8 m_touchscreen[0x10];
	UINT8 m_lcd_enabled;
	UINT8 m_lcd_cmd;
	UINT8 *m_ram_base;

	UINT8 m_irq_mask;
	UINT8 m_irq_flag;
	UINT8 m_port6;
	UINT8 m_beep_mode;

	struct
	{
		UINT8 type;
		UINT16 page;
	} m_banks[2];

	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( bankswitch_r );
	DECLARE_WRITE8_MEMBER( bankswitch_w );
	DECLARE_READ8_MEMBER( lcd_base_r );
	DECLARE_WRITE8_MEMBER( lcd_base_w );
	DECLARE_READ8_MEMBER( beep_r );
	DECLARE_WRITE8_MEMBER( beep_w );
	DECLARE_READ8_MEMBER( lcd_io_r );
	DECLARE_WRITE8_MEMBER( lcd_io_w );
	DECLARE_READ8_MEMBER( irq_r );
	DECLARE_WRITE8_MEMBER( irq_w );
	DECLARE_READ8_MEMBER( touchscreen_r );
	DECLARE_WRITE8_MEMBER( touchscreen_w );
	DECLARE_WRITE_LINE_MEMBER( alarm_irq );

	DECLARE_READ8_MEMBER( flash_0x0000_r );
	DECLARE_WRITE8_MEMBER( flash_0x0000_w );
	DECLARE_READ8_MEMBER( flash_0x8000_r );
	DECLARE_WRITE8_MEMBER( flash_0x8000_w );
	DECLARE_READ8_MEMBER( flash_0xa000_r );
	DECLARE_WRITE8_MEMBER( flash_0xa000_w );

	UINT8 identify_bank_type(UINT32 bank);
	DECLARE_PALETTE_INIT(rex6000);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer1);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer2);
	TIMER_DEVICE_CALLBACK_MEMBER(sec_timer);
	DECLARE_QUICKLOAD_LOAD_MEMBER(rex6000);
};


UINT8 rex6000_state::identify_bank_type(UINT32 bank)
{
	if (bank < 0x080)
	{
		return BANK_FLASH0_B0;
	}
	else if (bank >= 0x080 && bank < 0x100)
	{
		return BANK_FLASH0_B1;
	}
	else if ((bank >= 0xb00 && bank < 0xb80) || (bank >= 0x600 && bank < 0x680))
	{
		return BANK_FLASH1_B0;
	}
	else if ((bank >= 0xb80 && bank < 0xc00) || (bank >= 0x680 && bank < 0x700))
	{
		return BANK_FLASH1_B1;
	}
	else if (bank >= 0x1000 && bank < 0x1004)
	{
		return BANK_RAM;
	}
	else
	{
		//logerror("%04x: unkonwn memory bank %x\n", m_maincpu->pc(), bank);
		return BANK_UNKNOWN;
	}
}

READ8_MEMBER( rex6000_state::bankswitch_r )
{
	return m_bank[offset];
}

WRITE8_MEMBER( rex6000_state::bankswitch_w )
{
	address_space& program = m_maincpu->space(AS_PROGRAM);

	m_bank[offset&3] = data;

	switch (offset)
	{
		case 0:     //bank 1 low
		case 1:     //bank 1 high
		{
			//bank 1 start at 0x8000
			m_banks[0].page = (MAKE_BANK(m_bank[0], m_bank[1]&0x0f) + 4);
			m_banks[0].type = identify_bank_type(m_banks[0].page);

			if (m_banks[0].type != BANK_UNKNOWN)
			{
				program.install_readwrite_handler(0x8000, 0x9fff, 0, 0, read8_delegate(FUNC(rex6000_state::flash_0x8000_r), this), write8_delegate(FUNC(rex6000_state::flash_0x8000_w), this));
			}
			else
			{
				program.unmap_readwrite(0x8000, 0x9fff);
			}

			break;
		}
		case 2:     //bank 2 low
		case 3:     //bank 2 high
		{
			m_banks[1].page = MAKE_BANK(m_bank[2], m_bank[3]&0x1f);
			m_banks[1].type = identify_bank_type(m_banks[1].page);

			if (m_banks[1].type == BANK_RAM)
			{
				program.install_ram(0xa000, 0xbfff, m_ram_base + ((m_banks[1].page & 0x03)<<13));
			}
			else if (m_banks[1].type != BANK_UNKNOWN)
			{
				program.install_readwrite_handler(0xa000, 0xbfff, 0, 0, read8_delegate(FUNC(rex6000_state::flash_0xa000_r), this), write8_delegate(FUNC(rex6000_state::flash_0xa000_w), this));
			}
			else
			{
				program.unmap_readwrite(0xa000, 0xbfff);
			}

			break;
		}
	}
}

READ8_MEMBER( rex6000_state::beep_r )
{
	return m_beep_io[offset];
}

WRITE8_MEMBER( rex6000_state::beep_w )
{
	m_beep_io[offset] = data;

	switch (offset)
	{
		case 0:     //alarm mode control
			/*
			    ---- ---x   beep off
			    ---- --x-   continuous beep
			    ---- -x--   continuous alarm 1
			    ---- x---   continuous alarm 2
			    ---x ----   continuous alarm 3
			    --x- ----   single very short beep (keyclick)
			    -x-- ----   single alarm
			    x--- ----   single short beep
			*/
			//TODO: the beeper frequency and length in alarm mode need to be measured
			break;
		case 1:     //tone mode control
			if (m_beep_mode)
			{
				m_beep->set_state(BIT(data, 0));

				//the beeper frequency is update only if the bit 1 is set
				if (BIT(data, 1))
					m_beep->set_frequency(16384 / (((m_beep_io[2] | (m_beep_io[3]<<8)) & 0x0fff) + 2));
			}
			break;
		case 4:     //select alarm/tone mode
			if (m_beep_mode != BIT(data, 0))
				m_beep->set_state(0);      //turned off when mode changes

			m_beep_mode = BIT(data, 0);
			break;
	}
}

READ8_MEMBER( rex6000_state::lcd_base_r )
{
	return m_lcd_base[offset];
}

WRITE8_MEMBER( rex6000_state::lcd_base_w )
{
	m_lcd_base[offset&1] = data;
}

READ8_MEMBER( rex6000_state::lcd_io_r )
{
	return (offset == 0) ? m_lcd_enabled : m_lcd_cmd;
}

WRITE8_MEMBER( rex6000_state::lcd_io_w )
{
	switch (offset)
	{
		case 0:
			m_lcd_enabled = data;
			break;
		case 1:
			m_lcd_cmd = data;
			break;
	}
}

READ8_MEMBER( rex6000_state::irq_r )
{
	switch (offset)
	{
		case 0: //irq flag
			/*
			    ---- ---x   input port
			    ---- --x-   alarm
			    ---- -x--   serial
			    ---- x---   ??
			    ---x ----   1Hz timer
			    --x- ----   32Hz timer
			    -x-- ----   4096Hz timer
			    x--- ----   special events
			*/
			return m_irq_flag;

		case 1:
			return m_port6;

		case 2: //irq mask
			return m_irq_mask;

		default:
			return 0;
	}
}

WRITE8_MEMBER( rex6000_state::irq_w )
{
	switch (offset)
	{
		case 0: //irq flag
			m_irq_flag = data;
			break;
		case 1: //clear irq flag
			m_port6 = data;
			m_irq_flag &= (~data);
			break;
		case 2: //irq mask
			m_irq_mask = data;
			break;
	}
}

READ8_MEMBER( rex6000_state::touchscreen_r )
{
	UINT16 x = ioport("PENX")->read();
	UINT16 y = ioport("PENY")->read();
	UINT16 battery = ioport("BATTERY")->read();

	switch (offset)
	{
		case 0x08:
			return ((ioport("INPUT")->read() & 0x40) ? 0x20 : 0x00) | 0X10;
		case 0x09:
			if (m_touchscreen[4] & 0x80)
				return (battery>>0) & 0xff;
			else
				return (y>>0) & 0xff;
		case 0x0a:
			if (m_touchscreen[4] & 0x80)
				return (battery>>8) & 0xff;
			else
				return (y>>8) & 0xff;
		case 0x0b:
			return (x>>0) & 0xff;
		case 0x0c:
			return (x>>8) & 0xff;
	}

	return m_touchscreen[offset&0x0f];
}

WRITE8_MEMBER( rex6000_state::touchscreen_w )
{
	m_touchscreen[offset&0x0f] = data;
}

READ8_MEMBER( rex6000_state::flash_0x0000_r )
{
	return m_flash[0]->read(offset);
}

WRITE8_MEMBER( rex6000_state::flash_0x0000_w )
{
	m_flash[0]->write(offset, data);
}

READ8_MEMBER( rex6000_state::flash_0x8000_r )
{
	return m_flash[m_banks[0].type]->read(((m_banks[0].page & 0x7f)<<13) | offset);
}

WRITE8_MEMBER( rex6000_state::flash_0x8000_w )
{
	m_flash[m_banks[0].type]->write(((m_banks[0].page & 0x7f)<<13) | offset, data);
}

READ8_MEMBER( rex6000_state::flash_0xa000_r )
{
	return m_flash[m_banks[1].type]->read(((m_banks[1].page & 0x7f)<<13) | offset);
}

WRITE8_MEMBER( rex6000_state::flash_0xa000_w )
{
	m_flash[m_banks[1].type]->write(((m_banks[1].page & 0x7f)<<13) | offset, data);
}


static ADDRESS_MAP_START(rex6000_mem, AS_PROGRAM, 8, rex6000_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x7fff ) AM_READWRITE(flash_0x0000_r, flash_0x0000_w)
	AM_RANGE( 0x8000, 0x9fff ) AM_READWRITE(flash_0x8000_r, flash_0x8000_w)
	AM_RANGE( 0xa000, 0xbfff ) AM_READWRITE(flash_0xa000_r, flash_0xa000_w)
	AM_RANGE( 0xc000, 0xffff ) AM_RAMBANK("ram")            //system RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( rex6000_io, AS_IO, 8, rex6000_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x01, 0x04 ) AM_READWRITE(bankswitch_r, bankswitch_w)
	AM_RANGE( 0x05, 0x07 ) AM_READWRITE(irq_r, irq_w)
	AM_RANGE( 0x10, 0x10 ) AM_READ_PORT("INPUT")
	AM_RANGE( 0x15, 0x19 ) AM_READWRITE(beep_r, beep_w)
	AM_RANGE( 0x22, 0x23 ) AM_READWRITE(lcd_base_r, lcd_base_w)
	AM_RANGE( 0x30, 0x3f ) AM_DEVREADWRITE(TC8521_TAG, rp5c01_device, read, write)
	AM_RANGE( 0x40, 0x47 ) AM_MIRROR(0x08)  AM_NOP  //SIO
	AM_RANGE( 0x50, 0x51 ) AM_READWRITE(lcd_io_r, lcd_io_w)
	AM_RANGE( 0x60, 0x6f ) AM_READWRITE(touchscreen_r, touchscreen_w)
	//AM_RANGE( 0x00, 0xff ) AM_RAM
ADDRESS_MAP_END

INPUT_CHANGED_MEMBER(rex6000_state::trigger_irq)
{
	if (!(m_irq_mask & IRQ_FLAG_KEYCHANGE))
	{
		m_irq_flag |= IRQ_FLAG_KEYCHANGE;

		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

/* Input ports */
INPUT_PORTS_START( rex6000 )
	PORT_START("BATTERY")
	PORT_CONFNAME( 0x03ff, 0x03ff, "Battery Status" )
	PORT_CONFSETTING( 0x03ff, "Good" )
	PORT_CONFSETTING( 0x0000, "Poor" )

	PORT_START("INPUT")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Home")   PORT_CODE(KEYCODE_HOME)         PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Back")   PORT_CODE(KEYCODE_BACKSPACE)    PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Select")     PORT_CODE(KEYCODE_SPACE)    PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Up")     PORT_CODE(KEYCODE_PGUP)         PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Down")   PORT_CODE(KEYCODE_PGDN)         PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Pen")    PORT_CODE(KEYCODE_ENTER) PORT_CODE(MOUSECODE_BUTTON1)   PORT_CHANGED_MEMBER(DEVICE_SELF, rex6000_state, trigger_irq, 0)

	PORT_START("PENX")
	PORT_BIT(0x3ff, 0x00, IPT_LIGHTGUN_X) PORT_NAME("Pen X") PORT_CROSSHAIR(X, 1, 0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT

	PORT_START("PENY")
	PORT_BIT(0x3ff, 0x00, IPT_LIGHTGUN_Y) PORT_NAME("Pen Y") PORT_CROSSHAIR(Y, 1, 0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_INVERT
INPUT_PORTS_END

void rex6000_state::machine_start()
{
	m_flash[0] = machine().device<fujitsu_29dl16x_device>("flash0a");
	m_flash[1] = machine().device<fujitsu_29dl16x_device>("flash0b");
	m_flash[2] = machine().device<fujitsu_29dl16x_device>("flash1a");
	m_flash[3] = machine().device<fujitsu_29dl16x_device>("flash1b");

	m_ram_base = m_ram->pointer();
	membank("ram")->set_base(m_ram_base + 0x4000);
}
void rex6000_state::machine_reset()
{
	address_space& program = m_maincpu->space(AS_PROGRAM);

	program.install_readwrite_handler(0x8000, 0x9fff, 0, 0, read8_delegate(FUNC(rex6000_state::flash_0x8000_r), this), write8_delegate(FUNC(rex6000_state::flash_0x8000_w), this));
	program.install_readwrite_handler(0xa000, 0xbfff, 0, 0, read8_delegate(FUNC(rex6000_state::flash_0xa000_r), this), write8_delegate(FUNC(rex6000_state::flash_0xa000_w), this));

	m_banks[0].type = 0x04;
	m_banks[0].type = 0;
	m_banks[1].type = 0x00;
	m_banks[1].type = 0;

	memset(m_ram_base, 0, m_ram->size());
	memset(m_bank, 0, sizeof(m_bank));
	memset(m_beep_io, 0, sizeof(m_beep_io));
	memset(m_lcd_base, 0, sizeof(m_lcd_base));
	memset(m_touchscreen, 0, sizeof(m_touchscreen));
	m_lcd_enabled = 0;
	m_lcd_cmd = 0;
	m_irq_mask = 0;
	m_irq_flag = 0;
	m_port6 = 0;
	m_beep_mode = 0;
}

UINT32 rex6000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT16 lcd_bank = MAKE_BANK(m_lcd_base[0], m_lcd_base[1]&0x1f);
	UINT8 mem_type = identify_bank_type(lcd_bank);

	if (m_lcd_enabled && mem_type != BANK_UNKNOWN)
	{
		for (int y=0; y<120; y++)
			for (int x=0; x<30; x++)
			{
				UINT8 data = 0;

				if (mem_type == BANK_RAM)
				{
					data = (m_ram_base + ((lcd_bank & 0x03)<<13))[y*30 + x];
				}
				else
				{
					data =  m_flash[mem_type]->space(0).read_byte(((lcd_bank & 0x7f)<<13) | (y*30 + x));
				}


				for (int b=0; b<8; b++)
				{
					bitmap.pix16(y, (x * 8) + b) = BIT(data, 7);
					data <<= 1;
				}
			}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(rex6000_state::irq_timer1)
{
	if (!(m_irq_mask & IRQ_FLAG_IRQ2))
	{
		m_irq_flag |= IRQ_FLAG_IRQ2;

		m_maincpu->set_input_line(0, HOLD_LINE);
	}

}

TIMER_DEVICE_CALLBACK_MEMBER(rex6000_state::irq_timer2)
{
	if (!(m_irq_mask & IRQ_FLAG_IRQ1))
	{
		m_irq_flag |= IRQ_FLAG_IRQ1;

		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(rex6000_state::sec_timer)
{
	if (!(m_irq_mask & IRQ_FLAG_1HZ))
	{
		m_irq_flag |= IRQ_FLAG_1HZ;

		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}

WRITE_LINE_MEMBER( rex6000_state::alarm_irq )
{
	if (!(m_irq_mask & IRQ_FLAG_ALARM) && state)
	{
		m_irq_flag |= IRQ_FLAG_ALARM;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
}


PALETTE_INIT_MEMBER(rex6000_state, rex6000)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

QUICKLOAD_LOAD_MEMBER( rex6000_state,rex6000)
{
	static const char magic[] = "ApplicationName:Addin";
	address_space& flash = machine().device("flash0b")->memory().space(0);
	UINT32 img_start = 0;

	dynamic_buffer data(image.length());
	image.fread(&data[0], image.length());

	if(strncmp((const char*)&data[0], magic, 21))
		return IMAGE_INIT_FAIL;

	img_start = strlen((const char*)&data[0]) + 5;
	img_start += 0xa0;  //skip the icon (40x32 pixel)

	for (UINT32 i=0; i<image.length() - img_start ;i++)
		flash.write_byte(i, data[img_start + i]);

	return IMAGE_INIT_PASS;
}


/* F4 Character Displayer */
static const gfx_layout rex6000_bold_charlayout =
{
	16, 11,                 /* 16 x 11 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
	/* y offsets */
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16 },
	8*24            /* every char takes 24 bytes, first 2 bytes are used for the char size */
};

static const gfx_layout rex6000_tiny_charlayout =
{
	16, 9,                  /* 16 x 9 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
	/* y offsets */
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16 },
	8*20            /* every char takes 20 bytes, first 2 bytes ared use for the char size */
};

static const gfx_layout rex6000_graph_charlayout =
{
	16, 13,                 /* 16 x 13 characters */
	48,                     /* 48 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
	/* y offsets */
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16 , 11*16 , 12*16 },
	8*28            /* every char takes 28 bytes, first 2 bytes are used for the char size */
};

static GFXDECODE_START( rex6000 )
	GFXDECODE_ENTRY( "flash0a", 0x0f0000, rex6000_bold_charlayout,  0, 0 )  //normal
	GFXDECODE_ENTRY( "flash0a", 0x0f2000, rex6000_bold_charlayout,  0, 0 )  //bold
	GFXDECODE_ENTRY( "flash0a", 0x0f4000, rex6000_tiny_charlayout,  0, 0 )  //tiny
	GFXDECODE_ENTRY( "flash0a", 0x0f6000, rex6000_graph_charlayout, 0, 0 )  //graphic
GFXDECODE_END


static MACHINE_CONFIG_START( rex6000, rex6000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz) //Toshiba microprocessor Z80 compatible at 4.3MHz
	MCFG_CPU_PROGRAM_MAP(rex6000_mem)
	MCFG_CPU_IO_MAP(rex6000_io)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("sec_timer", rex6000_state, sec_timer, attotime::from_hz(1))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer1", rex6000_state, irq_timer1, attotime::from_hz(32))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer2", rex6000_state, irq_timer2, attotime::from_hz(4096))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(rex6000_state, screen_update)
	MCFG_SCREEN_SIZE(240, 120)
	MCFG_SCREEN_VISIBLE_AREA(0, 240-1, 0, 120-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(rex6000_state, rex6000)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", rex6000)

	/* quickload */
	MCFG_QUICKLOAD_ADD("quickload", rex6000_state, rex6000, "rex,ds2", 0)

	MCFG_DEVICE_ADD(TC8521_TAG, RP5C01, XTAL_32_768kHz)
	MCFG_RP5C01_OUT_ALARM_CB(WRITELINE(rex6000_state, alarm_irq))

	/*
	Fujitsu 29DL16X have feature which is capability of reading data from one
	bank of memory while a program or erase operation is in progress in the
	other bank of memory (simultaneous operation). This is not supported yet
	by the flash emulation and I have splitted every bank into a separate
	device for have a similar behavior.
	*/
	MCFG_FUJITSU_29DL16X_ADD("flash0a") //bank 0 of first flash
	MCFG_FUJITSU_29DL16X_ADD("flash0b") //bank 1 of first flash
	MCFG_FUJITSU_29DL16X_ADD("flash1a") //bank 0 of second flash
	MCFG_FUJITSU_29DL16X_ADD("flash1b") //bank 1 of second flash

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( rex6000 )
	ROM_REGION( 0x200000, "flash0a", ROMREGION_ERASE )
	ROM_LOAD( "rex6000_0.dat", 0x0000, 0x100000, BAD_DUMP CRC(e7a7324f) SHA1(01adcd469c93984438cbd4bf6e5a46e74da500d0))

	ROM_REGION( 0x200000, "flash0b", ROMREGION_ERASE )
	ROM_LOAD( "rex6000_1.dat", 0x0000, 0x100000, BAD_DUMP CRC(cc4b51db) SHA1(73b655e21ece30e8dc60f2f09719bc446492cc0f))

	ROM_REGION( 0x200000, "flash1a", ROMREGION_ERASE )
	ROM_LOAD( "rex6000_2.dat", 0x0000, 0x100000, NO_DUMP)

	ROM_REGION( 0x200000, "flash1b", ROMREGION_ERASE )
	ROM_LOAD( "rex6000_3.dat", 0x0000, 0x100000, NO_DUMP)
ROM_END

ROM_START( ds2 )
	ROM_REGION( 0x200000, "flash0a", ROMREGION_ERASE )
	ROM_LOAD( "ds2_0.dat", 0x0000, 0x100000, BAD_DUMP CRC(3197bed3) SHA1(9f826fd6d23ced797daae666d88ced14d24b64c3))

	ROM_REGION( 0x200000, "flash0b", ROMREGION_ERASE )
	ROM_LOAD( "ds2_1.dat", 0x0000, 0x100000, BAD_DUMP CRC(a909b755) SHA1(4db8a44539b9f94d418c33bbd794afb9bacbbadd))

	ROM_REGION( 0x200000, "flash1a", ROMREGION_ERASE )
	ROM_LOAD( "ds2_2.dat", 0x0000, 0x100000, BAD_DUMP CRC(a738ea1c) SHA1(3b71f43ff30f4b15b5cd85dd9e95ebc7e84eb5a3))

	ROM_REGION( 0x200000, "flash1b", ROMREGION_ERASE )
	ROM_LOAD( "ds2_3.dat", 0x0000, 0x100000, BAD_DUMP CRC(64f7c189) SHA1(a47d3413ddb879083c3aec03a42fe357d3a3c10a))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 2000, rex6000,  0,       0,   rex6000,    rex6000, driver_device,  0,   "Xircom / Intel",   "REX 6000",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 2000, ds2,      rex6000, 0,   rex6000,    rex6000, driver_device,  0,   "Citizen",          "DataSlim 2",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
