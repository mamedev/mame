// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/********************************************************************

Driver file to handle emulation of the Nintendo Pokemon Mini handheld
by Wilbert Pol.

The LCD is likely to be a SSD1828 LCD.

********************************************************************/

#include "emu.h"
#include "sound/speaker.h"
#include "machine/i2cmem.h"
#include "cpu/minx/minx.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "rendlay.h"
#include "softlist.h"

struct PRC
{
	UINT8       colors_inverted;
	UINT8       background_enabled;
	UINT8       sprites_enabled;
	UINT8       copy_enabled;
	UINT8       map_size;
	UINT8       map_size_x;
	UINT8       frame_count;
	UINT8       max_frame_count;
	UINT32      bg_tiles;
	UINT32      spr_tiles;
	UINT8       count;
	emu_timer   *count_timer;
};


struct TIMERS
{
	emu_timer   *seconds_timer;
	emu_timer   *hz256_timer;
	emu_timer   *timer1;                /* Timer 1 low or 16bit */
	emu_timer   *timer1_hi;             /* Timer 1 hi */
	emu_timer   *timer2;                /* Timer 2 low or 16bit */
	emu_timer   *timer2_hi;             /* Timer 2 high */
	emu_timer   *timer3;                /* Timer 3 low or 16bit */
	emu_timer   *timer3_hi;             /* Timer 3 high */
};


class pokemini_state : public driver_device
{
public:
	pokemini_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_p_ram(*this, "p_ram"),
		m_speaker(*this, "speaker"),
		m_i2cmem(*this, "i2cmem"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "INPUTS")
	{ }

	UINT8 m_pm_reg[0x100];
	PRC m_prc;
	TIMERS m_timers;
	bitmap_ind16 m_bitmap;
	virtual void video_start() override;
	virtual void machine_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(pokemini);
	DECLARE_WRITE8_MEMBER(hwreg_w);
	DECLARE_READ8_MEMBER(hwreg_r);
	DECLARE_READ8_MEMBER(rom_r);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(pokemini_cart);

protected:
	enum
	{
		TIMER_SECONDS,
		TIMER_256HZ,
		TIMER_1,
		TIMER_1_HI,
		TIMER_2,
		TIMER_2_HI,
		TIMER_3,
		TIMER_3_HI,
		TIMER_PRC
	};

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_p_ram;
	required_device<speaker_sound_device> m_speaker;
	required_device<i2cmem_device> m_i2cmem;
	required_device<generic_slot_device> m_cart;
	required_ioport m_inputs;

	void check_irqs();
	void update_sound();
	void seconds_timer_callback();
	void timer_256hz_callback();
	void timer1_callback();
	void timer1_hi_callback();
	void timer2_callback();
	void timer2_hi_callback();
	void timer3_callback();
	void timer3_hi_callback();
	void prc_counter_callback();

};


READ8_MEMBER( pokemini_state::rom_r )
{
	offset += 0x2100;
	return m_cart->read_rom(space, offset & 0x1fffff);
}

static ADDRESS_MAP_START( pokemini_mem_map, AS_PROGRAM, 8, pokemini_state )
	AM_RANGE( 0x000000, 0x000fff )  AM_ROM                            /* bios */
	AM_RANGE( 0x001000, 0x001fff )  AM_RAM AM_SHARE("p_ram")          /* VRAM/RAM */
	AM_RANGE( 0x002000, 0x0020ff )  AM_READWRITE(hwreg_r, hwreg_w)    /* hardware registers */
	AM_RANGE( 0x002100, 0x1fffff )  AM_READ(rom_r)                    /* cartridge area */
ADDRESS_MAP_END


static INPUT_PORTS_START( pokemini )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Button A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Button B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("Button C")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_NAME("Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_NAME("Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_NAME("Left")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("Right")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1) PORT_NAME("Power")
INPUT_PORTS_END


PALETTE_INIT_MEMBER(pokemini_state, pokemini)
{
	palette.set_pen_color(0, rgb_t(0xff, 0xfb, 0x87));
	palette.set_pen_color(1, rgb_t(0xb1, 0xae, 0x4e));
	palette.set_pen_color(2, rgb_t(0x84, 0x80, 0x4e));
	palette.set_pen_color(3, rgb_t(0x4e, 0x4e, 0x4e));
}


void pokemini_state::check_irqs()
{
	int irq_set[4] = { 1, 0, 0, 0 };
	int prio, vector;

	/* Check IRQ $03-$04 */
	prio = ( m_pm_reg[0x20] >> 6 ) & 0x03;
	if ( ! irq_set[prio] )
	{
		if ( m_pm_reg[0x23] & m_pm_reg[0x27] & 0x40 )
			irq_set[prio] = 0x04;

		if ( m_pm_reg[0x23] & m_pm_reg[0x27] & 0x80 )
			irq_set[prio] = 0x03;
	}

	/* Check IRQ $05-$06 */
	prio = ( m_pm_reg[0x20] >> 4 ) & 0x03;
	if ( ! irq_set[prio] )
	{
		if ( m_pm_reg[0x23] & m_pm_reg[0x27] & 0x10 )
			irq_set[prio] = 0x06;

		if ( m_pm_reg[0x23] & m_pm_reg[0x27] & 0x20 )
			irq_set[prio] = 0x05;
	}

	/* Check IRQ $07-$08 */
	prio = ( m_pm_reg[0x20] >> 2 ) & 0x03;
	if ( ! irq_set[prio] )
	{
		if ( m_pm_reg[0x23] & m_pm_reg[0x27] & 0x04 )
			irq_set[prio] = 0x08;

		if ( m_pm_reg[0x23] & m_pm_reg[0x27] & 0x08 )
			irq_set[prio] = 0x07;
	}

	/* Check IRQ $09-$0A */
	prio = ( m_pm_reg[0x20] >> 0 ) & 0x03;
	if ( ! irq_set[prio] )
	{
		if ( m_pm_reg[0x23] & m_pm_reg[0x27] & 0x01 )
			irq_set[prio] = 0x0A;

		if ( m_pm_reg[0x23] & m_pm_reg[0x27] & 0x02 )
			irq_set[prio] = 0x09;
	}

	/* Check IRQ $0B-$0E */
	prio = ( m_pm_reg[0x21] >> 6 ) & 0x03;
	if ( ! irq_set[prio] )
	{
		if ( m_pm_reg[0x24] & m_pm_reg[0x28] & 0x04 )
			irq_set[prio] = 0x0E;

		if ( m_pm_reg[0x24] & m_pm_reg[0x28] & 0x08 )
			irq_set[prio] = 0x0D;

		if ( m_pm_reg[0x24] & m_pm_reg[0x28] & 0x10 )
			irq_set[prio] = 0x0C;

		if ( m_pm_reg[0x24] & m_pm_reg[0x28] & 0x20 )
			irq_set[prio] = 0x0B;
	}

	/* Check IRQ $0F-$10 */
	prio = ( m_pm_reg[0x22] >> 0 ) & 0x03;
	if ( ! irq_set[prio] )
	{
		if ( m_pm_reg[0x26] & m_pm_reg[0x2A] & 0x40 )
			irq_set[prio] = 0x10;

		if ( m_pm_reg[0x26] & m_pm_reg[0x2A] & 0x80 )
			irq_set[prio] = 0x0F;
	}

	/* Check IRQ $13-$14 */
	prio = ( m_pm_reg[0x21] >> 4 ) & 0x03;
	if ( ! irq_set[prio] )
	{
		if ( m_pm_reg[0x24] & m_pm_reg[0x28] & 0x01 )
			irq_set[prio] = 0x14;

		if ( m_pm_reg[0x24] & m_pm_reg[0x28] & 0x02 )
			irq_set[prio] = 0x13;
	}

	/* Check IRQ $15-$1C */
	prio = ( m_pm_reg[0x21] >> 2 ) & 0x03;
	if ( ! irq_set[prio] )
	{
		if ( m_pm_reg[0x25] & m_pm_reg[0x29] & 0x01 )
			irq_set[prio] = 0x1C;

		if ( m_pm_reg[0x25] & m_pm_reg[0x29] & 0x02 )
			irq_set[prio] = 0x1B;

		if ( m_pm_reg[0x25] & m_pm_reg[0x29] & 0x04 )
			irq_set[prio] = 0x1A;

		if ( m_pm_reg[0x25] & m_pm_reg[0x29] & 0x08 )
			irq_set[prio] = 0x19;

		if ( m_pm_reg[0x25] & m_pm_reg[0x29] & 0x10 )
			irq_set[prio] = 0x18;

		if ( m_pm_reg[0x25] & m_pm_reg[0x29] & 0x20 )
			irq_set[prio] = 0x17;

		if ( m_pm_reg[0x25] & m_pm_reg[0x29] & 0x40 )
			irq_set[prio] = 0x16;

		if ( m_pm_reg[0x25] & m_pm_reg[0x29] & 0x80 )
			irq_set[prio] = 0x15;
	}

	/* Check IRQ $1D-$1F */
	prio = ( m_pm_reg[0x21] >> 0 ) & 0x03;
	if ( ! irq_set[prio] && ( m_pm_reg[0x26] & m_pm_reg[0x2A] & 0x07 ) )
	{
		if ( m_pm_reg[0x26] & m_pm_reg[0x2A] & 0x01 )
			irq_set[prio] = 0x1F;

		if ( m_pm_reg[0x26] & m_pm_reg[0x2A] & 0x02 )
			irq_set[prio] = 0x1E;

		if ( m_pm_reg[0x26] & m_pm_reg[0x2A] & 0x04 )
			irq_set[prio] = 0x1D;
	}

	/* Determine vector */
	vector = 0;
	if ( irq_set[1] )
		vector = irq_set[1];
	if ( irq_set[2] )
		vector = irq_set[2];
	if ( irq_set[3] )
		vector = irq_set[3];

	if ( vector )
	{
		//logerror("Triggering IRQ with vector %02x\n", vector );
		/* Trigger interrupt and set vector */
		m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, vector );
	}
	else
	{
		m_maincpu->set_input_line(0, CLEAR_LINE );
	}
}


void pokemini_state::update_sound()
{
	/* Check if sound should be muted */
	if ( m_pm_reg[0x70] & 0x03 )
	{
		m_speaker->level_w(0);
	}
	else
	{
		///static const int levels[4] = { 0, 1, 1, 2 };
		int level; /// silence clang warning/// = levels[ m_pm_reg[0x71] & 0x03 ];

//      if ( ( ( m_pm_reg[0x48] & 0x80 ) && ( m_pm_reg[0x4E] | ( m_pm_reg[0x4F] << 8 ) ) > ( m_pm_reg[0x4C] | ( m_pm_reg[0x4D] << 8 ) ) )
//        || ( ( m_pm_reg[0x48] & 0x80 ) && m_pm_reg[0x4F] > m_pm_reg[0x4D] ) )
//      {
			level = 0;
//      }

		m_speaker->level_w(level);
	}
}


void pokemini_state::seconds_timer_callback()
{
	if ( m_pm_reg[0x08] & 0x01 )
	{
		m_pm_reg[0x09] += 1;
		if ( ! m_pm_reg[0x09] )
		{
			m_pm_reg[0x0A] += 1;
			if ( ! m_pm_reg[0x0A] )
			{
				m_pm_reg[0x0B] += 1;
			}
		}
	}
}


void pokemini_state::timer_256hz_callback()
{
	if ( m_pm_reg[0x40] & 0x01 )
	{
		m_pm_reg[0x41] += 1;
		/* Check if the 32Hz IRQ should be triggered */
		if ( ! ( m_pm_reg[0x41] & 0x07 ) )
		{
			m_pm_reg[0x28] |= 0x20;

			/* Check if the 8Hz IRQ should be triggered */
			if ( ! ( m_pm_reg[0x41] & 0x1F ) )
			{
				m_pm_reg[0x28] |= 0x10;

				/* Check if the 2Hz IRQ should be triggered */
				if ( ! ( m_pm_reg[0x41] & 0x7F ) )
				{
					m_pm_reg[0x28] |= 0x08;

					/* Check if the 1Hz IRQ should be triggered */
					if ( ! m_pm_reg[0x41] )
					{
						m_pm_reg[0x28] |= 0x04;
					}
				}
			}

			check_irqs();
		}
	}
}


void pokemini_state::timer1_callback()
{
	m_pm_reg[0x36] -= 1;
	/* Check for underflow of timer */
	if ( m_pm_reg[0x36] == 0xFF )
	{
		/* Check if timer1 is running in 16bit mode */
		if ( m_pm_reg[0x30] & 0x80 )
		{
			m_pm_reg[0x37] -= 1;
			if ( m_pm_reg[0x37] == 0xFF )
			{
				m_pm_reg[0x27] |= 0x08;
				check_irqs();
				m_pm_reg[0x36] = m_pm_reg[0x32];
				m_pm_reg[0x37] = m_pm_reg[0x33];
			}
		}
		else
		{
			m_pm_reg[0x27] |= 0x04;
			check_irqs();
			m_pm_reg[0x36] = m_pm_reg[0x32];
		}
	}
}


void pokemini_state::timer1_hi_callback()
{
	m_pm_reg[0x37] -= 1;
	/* Check for underflow of timer */
	if ( m_pm_reg[0x37] == 0xFF )
	{
		m_pm_reg[0x27] |= 0x08;
		check_irqs();
		m_pm_reg[0x37] = m_pm_reg[0x33];
	}
}


void pokemini_state::timer2_callback()
{
	m_pm_reg[0x3E] -= 1;
	/* Check for underflow of timer */
	if ( m_pm_reg[0x3E] == 0xFF )
	{
		/* Check if timer2 is running in 16bit mode */
		if ( m_pm_reg[0x38] & 0x80 )
		{
			m_pm_reg[0x3F] -= 1;
			if ( m_pm_reg[0x3F] == 0xFF )
			{
				m_pm_reg[0x27] |= 0x20;
				check_irqs();
				m_pm_reg[0x3E] = m_pm_reg[0x3A];
				m_pm_reg[0x3F] = m_pm_reg[0x3B];
			}
		}
		else
		{
			m_pm_reg[0x27] |= 0x10;
			check_irqs();
			m_pm_reg[0x3E] = m_pm_reg[0x3A];
		}
	}
}


void pokemini_state::timer2_hi_callback()
{
	m_pm_reg[0x3F] -= 1;
	/* Check for underfow of timer */
	if ( m_pm_reg[0x3F] == 0xFF )
	{
		m_pm_reg[0x27] |= 0x20;
		check_irqs();
		m_pm_reg[0x3F] = m_pm_reg[0x3B];
	}
}


void pokemini_state::timer3_callback()
{
	m_pm_reg[0x4E] -= 1;
	/* Check for underflow of timer */
	if ( m_pm_reg[0x4E] == 0xFF )
	{
		/* Check if timer3 is running in 16bit mode */
		if ( m_pm_reg[0x48] & 0x80 )
		{
			m_pm_reg[0x4F] -= 1;
			if ( m_pm_reg[0x4F] == 0xFF )
			{
				m_pm_reg[0x27] |= 0x02;
				check_irqs();
				m_pm_reg[0x4E] = m_pm_reg[0x4A];
				m_pm_reg[0x4F] = m_pm_reg[0x4B];
			}
		}
		else
		{
			m_pm_reg[0x4E] = m_pm_reg[0x4A];
		}
	}

	if ( m_pm_reg[0x48] & 0x80 )
	{
		if (  ( m_pm_reg[0x4E] == m_pm_reg[0x4C] ) && ( m_pm_reg[0x4F] == m_pm_reg[0x4D] ) )
		{
			m_pm_reg[0x27] |= 0x01;
			check_irqs();
		}
		update_sound();
	}
}


void pokemini_state::timer3_hi_callback()
{
	m_pm_reg[0x4F] -= 1;
	/* Check for underflow of timer */
	if ( m_pm_reg[0x4F] == 0xFF )
	{
		m_pm_reg[0x27] |= 0x02;
		check_irqs();
		m_pm_reg[0x4F] = m_pm_reg[0x4B];
	}

	if ( ! ( m_pm_reg[0x48] & 0x80 ) )
	{
		if( m_pm_reg[0x4F] == m_pm_reg[0x4D] )
		{
			m_pm_reg[0x27] |= 0x01;
			check_irqs();
		}
		update_sound();
	}
}


WRITE8_MEMBER(pokemini_state::hwreg_w)
{
	static const int timer_to_cycles_fast[8] = { 2, 8, 32, 64, 128, 256, 1024, 4096 };
	static const int timer_to_cycles_slow[8] = { 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };

	//logerror( "%0X: Write to hardware address: %02X, %02X\n", space.device() .safe_pc( ), offset, data );

	switch( offset )
	{
	case 0x00:  /* start-up contrast
               Bit 0-1 R/W Must be 1(?)
               Bit 2-7 R/W Start up contrast (doesn't affect contrast until after reboot)
            */
	case 0x01:  /* CPU related?
               Bit 0-7 R/W Unknown
            */
	case 0x02:  /* CPU related?
               Bit 0-7 R/W Unknown
            */
		logerror( "%0X: Write to unknown hardware address: %02X, %02X\n", m_maincpu->pc(), offset, data );
		break;
	case 0x08:  /* Seconds-timer control
               Bit 0   R/W Timer enable
               Bit 1   W   Timer reset
               Bit 2-7     Unused
            */
		if ( data & 0x02 )
		{
			m_pm_reg[0x09] = 0x00;
			m_pm_reg[0x0A] = 0x00;
			m_pm_reg[0x0B] = 0x00;
			data &= ~0x02;
		}
		break;
	case 0x09:  /* Seconds-timer (low), read only
               Bit 0-7 R   Seconds timer bit 0-7
            */
		return;
	case 0x0A:  /* Seconds-timer (mid), read only
               Bit 0-7 R   Seconds timer bit 8-15
            */
		return;
	case 0x0B:  /* Seconds-timer (high), read only
               Bit 0-7 R   Seconds timer bit 16-23
            */
		return;
	case 0x10:  /* Low power detector
               Bit 0-4 R/W Unknown
               Bit 5   R   Battery status: 0 - battery OK, 1 - battery low
               Bit 6-7     Unused
            */
		logerror( "%0X: Write to unknown hardware address: %02X, %02X\n", m_maincpu->pc(), offset, data );
		break;
	case 0x18:  /* Timer 1 pre-scale + enable
               Bit 0-2 R/W low timer 1 prescaler select
                           000 - 2 or 128 cycles
                           001 - 8 or 256 cycles
                           010 - 32 or 512 cycles
                           011 - 64 or 1024 cycles
                           100 - 128 or 2048 cycles
                           101 - 256 or 4096 cycles
                           110 - 1024 or 8192 cycles
                           111 - 4096 or 16384 cycles
               Bit 3   R/W Enable low counting
               Bit 4-6 R/W high timer 1 prescaler select
               Bit 7   R/W Enable high counting
            */
		/* Check for prescaler change for the low counter */
		if ( ( data & 0x07 ) != ( m_pm_reg[0x18] & 0x07 ) )
		{
			int index = data & 0x07;
			int cycles = ( m_pm_reg[0x19] & 0x01 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer1->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));
		}

		/* Check for prescaler change for the high counter */
		if ( ( data & 0x70 ) != ( m_pm_reg[0x18] & 0x70 ) )
		{
			int index = ( data >> 4 ) & 0x07;
			int cycles = ( m_pm_reg[0x19] & 0x02 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer1_hi->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));
		}

		/* Check if timer1 low should be enabled */
		if ( ( data & 0x08 ) && ( m_pm_reg[0x30] & 0x04 ) &&
				( ( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x19] & 0x01 ) ) ||
				( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x19] & 0x01 ) ) ) )
		{
			m_timers.timer1->enable( 1 );
		}
		else
		{
			m_timers.timer1->enable( 0 );
		}

		/* Check if timer1 high should be enabled */
		if ( ( data & 0x80 ) && ( m_pm_reg[0x31] & 0x04 ) && ! ( m_pm_reg[0x30] & 0x80 ) &&
				( ( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x19] & 0x02 ) ) ||
				( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x19] & 0x02 ) ) ) )
		{
			m_timers.timer1_hi->enable( 1 );
		}
		else
		{
			m_timers.timer1_hi->enable( 0 );
		}
		break;
	case 0x19:  /* Timers 1 speed
               Bit 0   R/W Select slow timer for timer 1 lo
               Bit 1   R/W Select slow timer for timer 1 hi
               Bit 2-3     Unused
               Bit 4   R/W Enable slow timers
               Bit 5   R/W Enable fast timers
               Bit 6-7     Unused
            */
		/* Check for prescaler change for the high counter */
		if ( ( data & 0x01 ) != ( m_pm_reg[0x19] & 0x01 ) )
		{
			int index = m_pm_reg[0x18] & 0x07;
			int cycles = ( data & 0x01 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer1->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));
		}

		/* Check for prescaler change for the low counter */
		if ( ( data & 0x02 ) != ( m_pm_reg[0x19] & 0x02 ) )
		{
			int index = ( m_pm_reg[0x18] >> 4 ) & 0x07;
			int cycles = ( data & 0x02 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer1_hi->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));
		}

		{
			int timer1_enable = 0, timer1_hi_enable = 0;
			int timer2_enable = 0, timer2_hi_enable = 0;
			int timer3_enable = 0, timer3_hi_enable = 0;

			/* Check which fast timers should be enabled */
			if ( data & 0x20 )
			{
				if ( ( m_pm_reg[0x18] & 0x08 ) && ( m_pm_reg[0x30] & 0x04 ) && ! ( data & 0x01 ) )
					timer1_enable = 1;

				if ( ( m_pm_reg[0x18] & 0x80 ) && ( m_pm_reg[0x31] & 0x04 ) && ! ( m_pm_reg[0x30] & 0x80 ) && ! ( data & 0x02 ) )
					timer1_hi_enable = 1;

				if ( ( m_pm_reg[0x1A] & 0x08 ) && ( m_pm_reg[0x38] & 0x04 ) && ! ( m_pm_reg[0x1B] & 0x01 ) )
					timer2_enable = 1;

				if ( ( m_pm_reg[0x1A] & 0x80 ) && ( m_pm_reg[0x39] & 0x04 ) && ! ( m_pm_reg[0x38] & 0x80 ) && ! ( m_pm_reg[0x1B] & 0x02 ) )
					timer2_hi_enable = 1;

				if ( ( m_pm_reg[0x1C] & 0x08 ) && ( m_pm_reg[0x48] & 0x04 ) && ! ( m_pm_reg[0x1D] & 0x01 ) )
					timer3_enable = 1;

				if ( ( m_pm_reg[0x1C] & 0x80 ) && ( m_pm_reg[0x49] & 0x04 ) && ! ( m_pm_reg[0x48] & 0x80 ) && ! ( m_pm_reg[0x1D] & 0x02 ) )
					timer3_hi_enable = 1;
			}

			/* Check which slow timers should be enabled */
			if ( data & 0x10 )
			{
				if ( ( m_pm_reg[0x18] & 0x08 ) && ( data & 0x01 ) )
					timer1_enable = 1;

				if ( ( m_pm_reg[0x1A] & 0x08 ) && ( m_pm_reg[0x1B] & 0x01 ) )
					timer2_enable = 1;

				if ( ( m_pm_reg[0x1C] & 0x08 ) && ( m_pm_reg[0x1D] & 0x01 ) )
					timer3_enable = 1;
			}
			m_timers.timer1->enable( timer1_enable );
			m_timers.timer1_hi->enable( timer1_hi_enable );
			m_timers.timer2->enable( timer2_enable );
			m_timers.timer2_hi->enable( timer2_hi_enable );
			m_timers.timer3->enable( timer3_enable );
			m_timers.timer3_hi->enable( timer3_hi_enable );
		}
		break;
	case 0x1A:  /* Timer 2 pre-scale + enable
               Bit 0-2 R/W low timer 2 prescaler select
                           000 - 2 or 128 cycles
                           001 - 8 or 256 cycles
                           010 - 32 or 512 cycles
                           011 - 64 or 1024 cycles
                           100 - 128 or 2048 cycles
                           101 - 256 or 4096 cycles
                           110 - 1024 or 8192 cycles
                           111 - 4096 or 16384 cycles
               Bit 3   R/W Enable low counting
               Bit 4-6 R/W high timer 2 prescaler select
               Bit 7   R/W Enable high counting
            */
		/* Check for prescaler change for the low counter */
		if ( ( data & 0x07 ) != ( m_pm_reg[0x1A] & 0x07 ) )
		{
			int index = data & 0x07;
			int cycles = ( m_pm_reg[0x1B] & 0x01 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer2->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));
		}

		/* Check for prescaler change for the high counter */
		if ( ( data & 0x70 ) != ( m_pm_reg[0x1A] & 0x70 ) )
		{
			int index = ( data >> 4 ) & 0x07;
			int cycles = ( m_pm_reg[0x1B] & 0x02 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer2_hi->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));
		}

		/* Check if timer2 low should be enabled */
		if ( ( data & 0x08 ) && ( m_pm_reg[0x38] & 0x04 ) &&
				( ( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x1B] & 0x01 ) ) ||
				( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x1B] & 0x01 ) ) ) )
		{
			m_timers.timer2->enable( 1 );
		}
		else
		{
			m_timers.timer2->enable( 0 );
		}

		/* Check if timer2 high should be enabled */
		if ( ( data & 0x80 ) && ( m_pm_reg[0x39] & 0x04 ) && ! ( m_pm_reg[0x38] & 0x80 ) &&
				( ( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x1B] & 0x02 ) ) ||
				( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x1B] & 0x02 ) ) ) )
		{
			m_timers.timer2_hi->enable( 1 );
		}
		else
		{
			m_timers.timer2_hi->enable( 0 );
		}
		break;
	case 0x1B:  /* Timer 2 speeds
               Bit 0   R/W Select slow timer for timer 2 lo
               Bit 1   R/W Select slow timer for timer 2 hi
            */
		/* Check for prescaler change for the high counter */
		if ( ( data & 0x01 ) != ( m_pm_reg[0x1B] & 0x01 ) )
		{
			int index = m_pm_reg[0x1A] & 0x07;
			int cycles = ( data & 0x01 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer2->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));

			if ( ( m_pm_reg[0x1A] & 0x08 ) && ( m_pm_reg[0x38] & 0x04 ) &&
					( ( ( m_pm_reg[0x19] & 0x10 ) && ( data & 0x01 ) ) ||
					( ( m_pm_reg[0x19] & 0x20 ) && ! ( data & 0x01 ) ) ) )
			{
				m_timers.timer2->enable( 1 );
			}
			else
			{
				m_timers.timer2->enable( 0 );
			}
		}

		/* Check for prescaler change for the low counter */
		if ( ( data & 0x02 ) != ( m_pm_reg[0x1B] & 0x02 ) )
		{
			int index = ( m_pm_reg[0x1A] >> 4 ) & 0x07;
			int cycles = ( data & 0x02 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer2_hi->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));

			if ( ( m_pm_reg[0x1A] & 0x80 ) && ( m_pm_reg[0x39] & 0x04 ) && ! ( m_pm_reg[0x38] & 0x80 ) &&
					( ( ( m_pm_reg[0x19] & 0x10 ) && ( data & 0x02 ) ) ||
					( ( m_pm_reg[0x19] & 0x20 ) && ! ( data & 0x02 ) ) ) )
			{
				m_timers.timer2_hi->enable( 1 );
			}
			else
			{
				m_timers.timer2_hi->enable( 0 );
			}
		}
		break;
	case 0x1C:  /* Timer 3 pre-scale + enable
               Bit 0-2 R/W low timer 3 prescaler select
                           000 - 2 or 128 cycles
                           001 - 8 or 256 cycles
                           010 - 32 or 512 cycles
                           011 - 64 or 1024 cycles
                           100 - 128 or 2048 cycles
                           101 - 256 or 4096 cycles
                           110 - 1024 or 8192 cycles
                           111 - 4096 or 16384 cycles
               Bit 3   R/W Enable low counting
               Bit 4-6 R/W high timer 3 prescaler select
               Bit 7   R/W Enable high counting
            */
		/* Check for prescaler change for the low counter */
		if ( ( data & 0x07 ) != ( m_pm_reg[0x1C] & 0x07 ) )
		{
			int index = data & 0x07;
			int cycles = ( m_pm_reg[0x1D] & 0x01 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer3->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));
		}

		/* Check for prescaler change for the high counter */
		if ( ( data & 0x70 ) != ( m_pm_reg[0x1C] & 0x70 ) )
		{
			int index = ( data >> 4 ) & 0x07;
			int cycles = ( m_pm_reg[0x1D] & 0x02 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer3_hi->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));
		}

		/* Check if timer2 low should be enabled */
		if ( ( data & 0x08 ) && ( m_pm_reg[0x48] & 0x04 ) &&
				( ( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x1D] & 0x01 ) ) ||
				( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x1D] & 0x01 ) ) ) )
		{
			m_timers.timer3->enable( 1 );
		}
		else
		{
			m_timers.timer3->enable( 0 );
		}

		/* Check if timer2 high should be enabled */
		if ( ( data & 0x80 ) && ( m_pm_reg[0x49] & 0x04 ) && ! ( m_pm_reg[0x48] & 0x80 ) &&
				( ( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x1D] & 0x02 ) ) ||
				( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x1D] & 0x02 ) ) ) )
		{
			m_timers.timer3_hi->enable( 1 );
		}
		else
		{
			m_timers.timer3_hi->enable( 0 );
		}
		break;
	case 0x1D:  /* Timer 3 speeds
               Bit 0   R/W Select slow timer for timer 3 lo
               Bit 1   R/W Select slow timer for timer 3 hi
            */
		/* Check for prescaler change for the high counter */
		if ( ( data & 0x01 ) != ( m_pm_reg[0x1D] & 0x01 ) )
		{
			int index = m_pm_reg[0x1C] & 0x07;
			int cycles = ( data & 0x01 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer3->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));

			if ( ( m_pm_reg[0x1C] & 0x08 ) && ( m_pm_reg[0x48] & 0x04 ) &&
					( ( ( m_pm_reg[0x19] & 0x10 ) && ( data & 0x01 ) ) ||
					( ( m_pm_reg[0x19] & 0x20 ) && ! ( data & 0x01 ) ) ) )
			{
				m_timers.timer3->enable( 1 );
			}
			else
			{
				m_timers.timer3->enable( 0 );
			}
		}

		/* Check for prescaler change for the low counter */
		if ( ( data & 0x02 ) != ( m_pm_reg[0x1D] & 0x02 ) )
		{
			int index = ( m_pm_reg[0x1C] >> 4 ) & 0x07;
			int cycles = ( data & 0x02 ) ? timer_to_cycles_slow[index] : timer_to_cycles_fast[index];

			m_timers.timer3_hi->adjust(attotime::zero, 0, m_maincpu->cycles_to_attotime(cycles));

			if ( ( m_pm_reg[0x1C] & 0x80 ) && ( m_pm_reg[0x49] & 0x04 ) && ! ( m_pm_reg[0x48] & 0x80 ) &&
					( ( ( m_pm_reg[0x19] & 0x10 ) && ( data & 0x02 ) ) ||
					( ( m_pm_reg[0x19] & 0x20 ) && ! ( data & 0x02 ) ) ) )
			{
				m_timers.timer3_hi->enable( 1 );
			}
			else
			{
				m_timers.timer3_hi->enable( 0 );
			}
		}
		break;
	case 0x20:  /* Event #1-#8 priority
               Bit 0-1 R/W Timer 3 overflow Interrupt #7-#8
               Bit 2-3 R/W Timer 1 overflow Interrupt #5-#6
               Bit 4-5 R/W Timer 2 overflow Interrupt #3-#4
               Bit 6-7 R/W VDraw/VBlank trigger Interrupt #1-#2
            */
		m_pm_reg[0x20] = data;
		check_irqs();
		break;
	case 0x21:  /* Event #15-#22 priority
               Bit 0-1 R/W Unknown
               Bit 2-3 R/W All keypad interrupts - Interrupt #15-#22
               Bit 4-7 R/W Unknown
            */
		m_pm_reg[0x21] = data;
		check_irqs();
		break;
	case 0x22:  /* Event #9-#14 priority
               Bit 0-1 R/W All #9 - #14 events - Interrupt #9-#14
               Bit 2-7     Unused
            */
		m_pm_reg[0x22] = data;
		check_irqs();
		break;
	case 0x23:  /* Event #1-#8 enable
               Bit 0   R/W Timer 3 overflow (mirror) - Enable Interrupt #8
               Bit 1   R/W Timer 3 overflow - Enable Interrupt #7
               Bit 2   R/W Not called... - Enable Interrupt #6
               Bit 3   R/W Timer 1 overflow - Enable Interrupt #5
               Bit 4   R/W Not called... - Enable Interrupt #4
               Bit 5   R/W Timer 2 overflow - Enable Interrupt #3
               Bit 6   R/W V-Draw trigger - Enable Interrupt #2
               Bit 7   R/W V-Blank trigger - Enable Interrupt #1
            */
		m_pm_reg[0x23] = data;
		check_irqs();
		break;
	case 0x24:  /* Event #9-#12 enable
               Bit 0-5 R/W Unknown
               Bit 6-7     Unused
            */
		m_pm_reg[0x24] = data;
		check_irqs();
		break;
	case 0x25:  /* Event #15-#22 enable
               Bit 0   R/W Press key "A" event - Enable interrupt #22
               Bit 1   R/W Press key "B" event - Enable interrupt #21
               Bit 2   R/W Press key "C" event - Enable interrupt #20
               Bit 3   R/W Press D-pad up key event - Enable interrupt #19
               Bit 4   R/W Press D-pad down key event - Enable interrupt #18
               Bit 5   R/W Press D-pad left key event - Enable interrupt #17
               Bit 6   R/W Press D-pad right key event - Enable interrupt #16
               Bit 7   R/W Press power button event - Enable interrupt #15
            */
		m_pm_reg[0x25] = data;
		check_irqs();
		break;
	case 0x26:  /* Event #13-#14 enable
               Bit 0-2 R/W Unknown
               Bit 3       Unused
               Bit 4-5 R/W Unknown
               Bit 6   R/W Shock detector trigger - Enable interrupt #14
               Bit 7   R/W IR receiver - low to high trigger - Enable interrupt #13
            */
		m_pm_reg[0x26] = data;
		check_irqs();
		break;
	case 0x27:  /* Interrupt active flag #1-#8
               Bit 0       Timer 3 overflow (mirror) / Clear interrupt #8
               Bit 1       Timer 3 overflow / Clear interrupt #7
               Bit 2       Not called ... / Clear interrupt #6
               Bit 3       Timer 1 overflow / Clear interrupt #5
               Bit 4       Not called ... / Clear interrupt #4
               Bit 5       Timer 2 overflow / Clear interrupt #3
               Bit 6       VDraw trigger / Clear interrupt #2
               Bit 7       VBlank trigger / Clear interrupt #1
            */
		m_pm_reg[0x27] &= ~data;
		check_irqs();
		return;
	case 0x28:  /* Interrupt active flag #9-#12
               Bit 0-1     Unknown
               Bit 2       Unknown / Clear interrupt #12
               Bit 3       Unknown / Clear interrupt #11
               Bit 4       Unknown / Clear interrupt #10
               Bit 5       Unknown / Clear interrupt #9
               Bit 6-7     Unknown
            */
		m_pm_reg[0x28] &= ~data;
		check_irqs();
		return;
	case 0x29:  /* Interrupt active flag #15-#22
               Bit 0       Press key "A" event / Clear interrupt #22
               Bit 1       Press key "B" event / Clear interrupt #21
               Bit 2       Press key "C" event / Clear interrupt #20
               Bit 3       Press D-pad up key event / Clear interrupt #19
               Bit 4       Press D-pad down key event / Clear interrupt #18
               Bit 5       Press D-pad left key event / Clear interrupt #17
               Bit 6       Press D-pad right key event / Clear interrupt #16
               Bit 7       Press power button event / Clear interrupt #15
            */
		m_pm_reg[0x29] &= ~data;
		check_irqs();
		return;
	case 0x2A:  /* Interrupt active flag #13-#14
               Bit 0-5     Unknown
               Bit 6       Shock detector trigger / Clear interrupt #14
               Bit 7       Unknown / Clear interrupt #13
            */
		m_pm_reg[0x2A] &= ~data;
		check_irqs();
		return;
	case 0x30:  /* Timer 1 control 1
               Bit 0   R/W Unknown
               Bit 1   W   Reset low counter
               Bit 2   R/W Enable high counter
               Bit 3   R/W Unknown
               Bit 4-6     Unused
               Bit 7   R/W Enable 16bit mode
            */
		if ( data & 0x02 )
		{
			m_pm_reg[0x36] = m_pm_reg[0x32];
			data &= ~0x02;
		}

		if ( ( data & 0x04 ) && ( m_pm_reg[0x18] & 0x08 ) &&
				( ( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x19] & 0x01 ) ) ||
				( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x19] & 0x01 ) ) ) )
		{
			m_timers.timer1->enable( 1 );
		}
		else
		{
			m_timers.timer1->enable( 0 );
		}

		if ( ( m_pm_reg[0x31] & 0x04 ) && ! ( data & 0x80 ) && ( m_pm_reg[0x18] & 0x80 ) &&
				( ( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x19] & 0x02 ) ) ||
				( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x19] & 0x02 ) ) ) )
		{
			m_timers.timer1_hi->enable( 1 );
		}
		else
		{
			m_timers.timer1_hi->enable( 0 );
		}
		break;
	case 0x31:  /* Timer 1 control 2
               Bit 0   R/W Unknown
               Bit 1   W   Reset hi counter
               Bit 2   R/W Enable high counter
               Bit 3   R/W Unknown
               Bit 4-7     Unused
            */
		if ( data & 0x02 )
		{
			m_pm_reg[0x37] = m_pm_reg[0x33];
			data &= ~0x02;
		}

		if ( ( data & 0x04 ) && ! ( m_pm_reg[0x30] & 0x80 ) && ( m_pm_reg[0x18] & 0x80 ) &&
				( ( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x19] & 0x02 ) ) ||
				( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x19] & 0x02 ) ) ) )
		{
			m_timers.timer1_hi->enable( 1 );
		}
		else
		{
			m_timers.timer1_hi->enable( 0 );
		}
		break;
	case 0x32:  /* Timer 1 preset value (low)
               Bit 0-7 R/W Timer 1 preset value bit 0-7
            */
		break;
	case 0x33:  /* Timer 1 preset value (high)
               Bit 0-7 R/W Timer 1 preset value bit 8-15
            */
		break;
	case 0x34:  /* Timer 1 sound-pivot (low, unused)
            */
	case 0x35:  /* Timer 1 sound-pivot (high, unused)
            */
		logerror( "%0X: Write to unknown hardware address: %02X, %02X\n", m_maincpu->pc(), offset, data );
		break;
	case 0x36:  /* Timer 1 counter (low), read only
            */
		return;
	case 0x37:  /* Timer 1 counter (high), read only
            */
		return;
	case 0x38:  /* Timer 2 control 1
               Bit 0   R/W Unknown
               Bit 1   W   Reset low counter
               Bit 2   R/W Enable high counter
               Bit 3   R/W Unknown
               Bit 4-6     Unused
               Bit 7   R/W Enable 16bit mode
            */
		if ( data & 0x02 )
		{
			m_pm_reg[0x3E] = m_pm_reg[0x3A];
			data &= ~0x02;
		}

		if ( ( data & 0x04 ) && ( m_pm_reg[0x1A] & 0x08 ) &&
				( ( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x1A] & 0x01 ) ) ||
				( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x1A] & 0x01 ) ) ) )
		{
			m_timers.timer2->enable( 1 );
		}
		else
		{
			m_timers.timer2->enable( 0 );
		}
		if ( ( m_pm_reg[0x39] & 0x04 ) && ! ( data & 0x80 ) && ( m_pm_reg[0x1A] & 0x80 ) &&
				( ( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x1B] & 0x02 ) ) ||
				( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x1B] & 0x02 ) ) ) )
		{
			m_timers.timer2_hi->enable( 1 );
		}
		else
		{
			m_timers.timer2_hi->enable( 0 );
		}
		break;
	case 0x39:  /* Timer 2 control 2
               Bit 0   R/W Unknown
               Bit 1   W   Reset hi counter
               Bit 2   R/W Enable high counter
               Bit 3   R/W Unknown
               Bit 4-7     Unused
            */
		if ( data & 0x02 )
		{
			m_pm_reg[0x3F] = m_pm_reg[0x3A];
			data &= ~0x02;
		}

		if ( ( data & 0x04 ) && ! ( m_pm_reg[0x38] & 0x80 ) && ( m_pm_reg[0x1A] & 0x80 ) &&
				( ( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x1B] & 0x02 ) ) ||
				( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x1B] & 0x02 ) ) ) )
		{
			m_timers.timer2_hi->enable( 1 );
		}
		else
		{
			m_timers.timer2_hi->enable( 0 );
		}
		break;
	case 0x3A:  /* Timer 2 preset value (low)
               Bit 0-7 R/W Timer 2 preset value bit 0-7
            */
		break;
	case 0x3B:  /* Timer 2 preset value (high)
               Bit 0-7 R/W Timer 2 preset value bit 8-15
            */
		break;
	case 0x3C:  /* Timer 2 sound-pivot (low, unused)
            */
	case 0x3D:  /* Timer 2 sound-pivot (high, unused)
            */
		logerror( "%0X: Write to unknown hardware address: %02X, %02X\n", m_maincpu->pc(), offset, data );
		break;
	case 0x3E:  /* Timer 2 counter (low), read only
               Bit 0-7 R/W Timer 2 counter value bit 0-7
            */
		return;
	case 0x3F:  /* Timer 2 counter (high), read only
               Bit 0-7 R/W Timer 2 counter value bit 8-15
            */
		return;
	case 0x40:  /* 256Hz timer control
               Bit 0   R/W Enable Timer
               Bit 1   W   Reset Timer
               Bit 2-7     Unused
            */
		if ( data & 0x02 )
		{
			m_pm_reg[0x41] = 0;
			data &= ~0x02;
		}
		break;
	case 0x41:  /* 256Hz timer counter
               Bit 0-7 R   256Hz timer counter
            */
		return;
	case 0x48:  /* Timer 3 control 1
               Bit 0   R/W Unknown
               Bit 1   W   Reset low counter
               Bit 2   R/W Enable high counter
               Bit 3   R/W Unknown
               Bit 4-6     Unused
               Bit 7   R/W Enable 16bit mode
            */
		if ( data & 0x02 )
		{
			m_pm_reg[0x4E] = m_pm_reg[0x4A];
			data &= ~0x02;
		}

		if ( ( data & 0x04 ) && ( m_pm_reg[0x1C] & 0x08 ) &&
				( ( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x1D] & 0x01 ) ) ||
				( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x1D] & 0x01 ) ) ) )
		{
			m_timers.timer3->enable( 1 );
		}
		else
		{
			m_timers.timer3->enable( 0 );
		}
		if ( ( m_pm_reg[0x49] & 0x04 ) && ! ( data & 0x80 ) && ( m_pm_reg[0x1C] & 0x80 ) &&
				( ( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x1D] & 0x02 ) ) ||
				( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x1D] & 0x02 ) ) ) )
		{
			m_timers.timer3_hi->enable( 1 );
		}
		else
		{
			m_timers.timer3_hi->enable( 0 );
		}
		m_pm_reg[0x48] = data;
		update_sound();
		break;
	case 0x49:  /* Timer 3 control 2
               Bit 0   R/W Unknown
               Bit 1   W   Reset hi counter
               Bit 2   R/W Enable high counter
               Bit 3   R/W Unknown
               Bit 4-7     Unused
            */
		if ( data & 0x02 )
		{
			m_pm_reg[0x4F] = m_pm_reg[0x4B];
			data &= ~0x02;
		}

		if ( ( data & 0x04 ) && ! ( m_pm_reg[0x48] & 0x80 ) && ( m_pm_reg[0x1C] & 0x80 ) &&
				( ( ( m_pm_reg[0x19] & 0x20 ) && ! ( m_pm_reg[0x1D] & 0x02 ) ) ||
				( ( m_pm_reg[0x19] & 0x10 ) && ( m_pm_reg[0x1D] & 0x02 ) ) ) )
		{
			m_timers.timer3_hi->enable( 1 );
		}
		else
		{
			m_timers.timer3_hi->enable( 0 );
		}
		m_pm_reg[0x49] = data;
		update_sound();
		break;
	case 0x4A:  /* Timer 3 preset value (low)
               Bit 0-7 R/W Timer 3 preset value bit 0-7
            */
		m_pm_reg[0x4A] = data;
		update_sound();
		break;
	case 0x4B:  /* Timer 3 preset value (high)
               Bit 0-7 R/W Timer 3 preset value bit 8-15
            */
		m_pm_reg[0x4B] = data;
		update_sound();
		break;
	case 0x4C:  /* Timer 3 sound-pivot (low)
               Bit 0-7 R/W Timer 3 sound-pivot value bit 0-7
            */
		m_pm_reg[0x4C] = data;
		update_sound();
		break;
	case 0x4D:  /* Timer 3 sound-pivot (high)
               Bit 0-7 R/W Timer 3 sound-pivot value bit 8-15

               Sound-pivot location:
               Pulse-Width of 0% = 0x0000
               Pulse-Width of 50% = Half of preset-value
               Pulse-Width of 100% = Same as preset-value
            */
		m_pm_reg[0x4D] = data;
		update_sound();
		break;
	case 0x4E:  /* Timer 3 counter (low), read only
               Bit 0-7 R/W Timer 3 counter value bit 0-7
            */
		return;
	case 0x4F:  /* Timer 3 counter (high), read only
               Bit 0-7 R/W Timer 3 counter value bit 8-15
            */
		return;
	case 0x52:  /* Keypad status
               Bit 0   R   Key "A"
               Bit 1   R   Key "B"
               Bit 2   R   Key "C"
               Bit 3   R   D-pad up
               Bit 4   R   D-pad down
               Bit 5   R   D-pad left
               Bit 6   R   D-pad right
               Bit 7   R   Power button
            */
		return;
	case 0x60:  /* I/O peripheral circuit select
               Bit 0   R/W Unknown
               bit 1   R/W IR receive / transmit
               Bit 2   R/W EEPROM / RTC data
               Bit 3   R/W EEPROM / RTC clock
               Bit 4   R/W Rumble controller
               Bit 5   R/W IR enable/disable
               Bit 6   R/W Unknown
               Bit 7   R/W Unknown
            */
		break;
	case 0x61:  /* I/O peripheral status control
               Bit 0   R/W IR received bit (if device not selected: 0)
               Bit 1   R/W IR transmit (if device not selected: 0)
               Bit 2   R/W EEPROM / RTC data (if device not selected: 1)
               Bit 3   R/W EEPROM / RTC clock (if device not selected: 0)
               Bit 4   R/W Rumble on/off (if device not selected: 0)
               Bit 5   R/W IR disable (receive & transmit) (if device not selected: 1)
               Bit 6       Always 1
               Bit 7   R/W IR received bit (mirror, if device not selected: 0)
            */
		if ( m_pm_reg[0x60] & 0x04 )
			m_i2cmem->write_sda( ( data & 0x04 ) ? 1 : 0 );

		if ( m_pm_reg[0x60] & 0x08 )
			m_i2cmem->write_scl( ( data & 0x08 ) ? 1 : 0 );
		break;
	case 0x70:  /* Sound related */
		m_pm_reg[0x70] = data;
		update_sound();
		break;
	case 0x71:  /* Sound volume
               Bit 0-1 R/W Sound volume
                           00 - 0%
                           01 - 50%
                           10 - 50%
                           11 - 100%
               Bit 2   R/W Always set to 0
               Bit 3-7     Unused
            */
		m_pm_reg[0x71] = data;
		update_sound();
		break;
	case 0x80:  /* LCD control
               Bit 0   R/W Invert colors; 0 - normal, 1 - inverted
               Bit 1   R/W Enable rendering of background
               Bit 2   R/W Enable rendering of sprites
               Bit 3   R/W Enable copy to LCD ram
               Bit 4-5 R/W Map size
                           00 - 12x16
                           01 - 16x12
                           10 - 24x8
                           11 - 24x8 (prohibited code)
              Bit 6-7      Unused
            */
		m_prc.colors_inverted = ( data & 0x01 ) ? 1 : 0;
		m_prc.background_enabled = ( data & 0x02 ) ? 1 : 0;
		m_prc.sprites_enabled = ( data & 0x04 ) ? 1 : 0;
		m_prc.copy_enabled = ( data & 0x08 ) ? 1 : 0;
		m_prc.map_size = ( data >> 4 ) & 0x03;
		switch( m_prc.map_size )
		{
		case 0:
			m_prc.map_size_x = 12; break;
		case 1:
			m_prc.map_size_x = 16; break;
		case 2:
		case 3:
			m_prc.map_size_x = 24; break;
		}
		break;
	case 0x81:  /* LCD render refresh rate
               Bit 0   R/W Unknown
               Bit 1-3 R/W LCD refresh rate divider
                           000 - 60Hz / 3 = 20Hz (0 - 2)
                           001 - 60Hz / 6 = 10Hz (0 - 5)
                           010 - 60Hz / 9 = 6,6Hz (0 - 8)
                           011 - 60Hz / 12 = 5Hz (0 - B)
                           100 - 60Hz / 2 = 30Hz (0 - 1)
                           101 - 60Hz / 4 = 15Hz (0 - 3)
                           110 - 60Hz / 6 = 10Hz (0 - 5)
                           111 - 60Hz / 8 = 7,5Hz (0 - 7)
               Bit 4-7 R   Divider position, when overflow the LCD is updated
            */
		switch ( data & 0x0E )
		{
		case 0x00:  m_prc.max_frame_count = 3; break;
		case 0x02:  m_prc.max_frame_count = 6; break;
		case 0x04:  m_prc.max_frame_count = 9; break;
		case 0x06:  m_prc.max_frame_count = 12; break;
		case 0x08:  m_prc.max_frame_count = 2; break;
		case 0x0A:  m_prc.max_frame_count = 4; break;
		case 0x0C:  m_prc.max_frame_count = 6; break;
		case 0x0E:  m_prc.max_frame_count = 8; break;
		}
		break;
	case 0x82:  /* BG tile data memory offset (low)
               Bit 0-2     Always "0"
               Bit 3-7 R/W BG tile data memory offset bit 3-7
            */
		data &= 0xF8;
		m_prc.bg_tiles = ( m_prc.bg_tiles & 0xFFFF00 ) | data;
		break;
	case 0x83:  /* BG tile data memory offset (mid)
               Bit 0-7 R/W BG tile data memory offset bit 8-15
            */
		m_prc.bg_tiles = ( m_prc.bg_tiles & 0xFF00FF ) | ( data << 8 );
		break;
	case 0x84:  /* BG tile data memory offset (high)
               Bit 0-4 R/W BG tile data memory offset bit 16-20
               Bit 5-7     Unused
            */
		data &= 0x1F;
		m_prc.bg_tiles = ( m_prc.bg_tiles & 0x00FFFF ) | ( data << 16 );
		break;
	case 0x85:  /* BG vertical move
               Bit 0-6 R/W Move the background up, move range:
                           Map size 0: 0x00 to 0x40
                           Map size 1: 0x00 to 0x20
                           Map size 2: move ignored
               Bit 7       Unused
            */
	case 0x86:  /* BG horizontal move
               Bit 0-6 R/W Move the background left, move range:
                           Map size 0: move ignored
                           Map size 1: 0x00 to 0x20
                           Map size 2: 0x00 to 0x60
               Bit 7       Unused
            */
		logerror( "%0X: Write to unknown hardware address: %02X, %02X\n", m_maincpu->pc(), offset, data );
		break;
	case 0x87:  /* Sprite tile data memory offset (low)
               Bit 0-5     Always "0"
               Bit 6-7 R/W Sprite tile data memory offset bit 6-7
            */
		data &= 0xC0;
		m_prc.spr_tiles = ( m_prc.spr_tiles & 0xFFFF00 ) | data;
		break;
	case 0x88:  /* Sprite tile data memory offset (med)
               Bit 0-7 R/W Sprite tile data memory offset bit 8-15
            */
		m_prc.spr_tiles = ( m_prc.spr_tiles & 0xFF00FF ) | ( data << 8 );
		break;
	case 0x89:  /* Sprite tile data memory offset (high)
               Bit 0-4 R/W Sprite tile data memory offset bit 16-20
               Bit 5-7     Unused
            */
		data &= 0x1F;
		m_prc.spr_tiles = ( m_prc.spr_tiles & 0x00FFFF ) | ( data << 16 );
		break;
	case 0x8A:  /* LCD status
               Bit 0   R   Unknown
               Bit 1   R   Unknown
               Bit 2   R   Unknown
               Bit 3   R   Unknown
               Bit 4   R   LCD during V-Sync / Rendering circuitry active or not ( 1 = not active)
               Bit 5   R   Unknown
               Bit 6-7     Unused
            */
	case 0xFE:  /* Direct LCD control / data
               Bit 0-7 R/W Direct LCD command or data
            */
//      lcd_command_w( data );
		break;
	case 0xFF:  /* Direct LCD data
               Bit 0-7 R/W Direct LCD data
            */
//      lcd_data_w( data );
		break;
	default:
		logerror( "%0X: Write to unknown hardware address: %02X, %02X\n", m_maincpu->pc(), offset, data );
		break;
	}
	m_pm_reg[offset] = data;
}

READ8_MEMBER(pokemini_state::hwreg_r)
{
	UINT8 data = m_pm_reg[offset];

	switch( offset )
	{
	case 0x52:  return m_inputs->read();
	case 0x61:
		if ( ! ( m_pm_reg[0x60] & 0x04 ) )
		{
			data = ( data & ~ 0x04 ) | ( m_i2cmem->read_sda() ? 0x04 : 0x00 );
		}

		if ( ! ( m_pm_reg[0x60] & 0x08 ) )
		{
			data &= ~0x08;
		}
		break;
	case 0x81:  return ( m_pm_reg[offset] & 0x0F ) | ( m_prc.frame_count << 4 );
	case 0x8A:  return m_prc.count;
	}
	return data;
}

DEVICE_IMAGE_LOAD_MEMBER( pokemini_state, pokemini_cart )
{
	UINT32 size = m_cart->common_get_size("rom");

	/* Verify that the image is big enough */
	if (size <= 0x2100)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid ROM image: ROM image is too small");
		return IMAGE_INIT_FAIL;
	}

	/* Verify that the image is not too big */
	if (size > 0x1fffff)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid ROM image: ROM image is too big");
		return IMAGE_INIT_FAIL;
	}

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}


void pokemini_state::prc_counter_callback()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );
	m_prc.count++;

	/* Check for overflow */
	if ( m_prc.count >= 0x42 )
	{
		m_prc.count = 0;
		m_prc.frame_count++;
	}
	else
	{
		if ( m_prc.count == 0x18 && m_prc.frame_count >= m_prc.max_frame_count )
		{
			m_prc.frame_count = 0;

			/* Check if the background should be drawn */
			if ( m_prc.background_enabled )
			{
				int x, y;
				for ( y = 0; y < 8; y++ ) {
					for ( x = 0; x < 12; x++ ) {
						UINT8 tile = m_p_ram[ 0x360 + ( y * m_prc.map_size_x ) + x ];
						int i;
						for( i = 0; i < 8; i++ ) {
							m_p_ram[ ( y * 96 ) + ( x * 8 ) + i ] = space.read_byte( m_prc.bg_tiles + ( tile * 8 ) + i );
						}
					}
				}
			}

			/* Check if the sprites should be drawn */
			if ( m_prc.sprites_enabled )
			{
				UINT16  spr;

				for ( spr = 0x35C; spr >= 0x300; spr -= 4 )
				{
					int     spr_x = ( m_p_ram[ spr + 0 ] & 0x7F ) - 16;
					int     spr_y = ( m_p_ram[ spr + 1 ] & 0x7F ) - 16;
					UINT8   spr_tile = m_p_ram[ spr + 2 ];
					UINT8   spr_flag = m_p_ram[ spr + 3 ];

					if ( spr_flag & 0x08 )
					{
						UINT16  gfx, mask;
						UINT32  spr_base = m_prc.spr_tiles + spr_tile * 64;
						int     i, j;

						for ( i = 0; i < 16; i++ )
						{
							if ( spr_x + i >= 0 && spr_x + i < 96 )
							{
								int rel_x = ( spr_flag & 0x01 ) ? 15 - i : i;
								UINT32  s = spr_base + ( ( rel_x & 0x08 ) << 2 ) + ( rel_x & 0x07 );

								mask = ~ ( space.read_byte( s ) | ( space.read_byte( s + 8 ) << 8 ) );
								gfx = space.read_byte( s + 16 ) | ( space.read_byte( s + 24 ) << 8 );

								/* Are the colors inverted? */
								if ( spr_flag & 0x04 )
								{
									gfx = ~gfx;
								}

								for ( j = 0; j < 16; j++ )
								{
									if ( spr_y + j >= 0 && spr_y + j < 64 )
									{
										UINT16  ram_addr = ( ( ( spr_y + j ) >> 3 ) * 96 ) + spr_x + i;

										if ( spr_flag & 0x02 )
										{
											if ( mask & 0x8000 )
											{
												m_p_ram[ ram_addr ] &= ~ ( 1 << ( ( spr_y + j ) & 0x07 ) );
												if ( gfx & 0x8000 )
												{
													m_p_ram[ ram_addr ] |= ( 1 << ( ( spr_y + j ) & 0x07 ) );
												}
											}
											mask <<= 1;
											gfx <<= 1;
										}
										else
										{
											if ( mask & 0x0001 )
											{
												m_p_ram[ ram_addr ] &= ~ ( 1 << ( ( spr_y + j ) & 0x07 ) );
												if ( gfx & 0x0001 )
												{
													m_p_ram[ ram_addr ] |= ( 1 << ( ( spr_y + j ) & 0x07 ) );
												}
											}
											mask >>= 1;
											gfx >>= 1;
										}
									}
								}
							}
						}
					}
				}
			}

			/* Set PRC Render interrupt */
			m_pm_reg[0x27] |= 0x40;
			check_irqs();

			/* Check if the rendered data should be copied to the LCD */
			if ( m_prc.copy_enabled )
			{
				int x, y;

				for( y = 0; y < 64; y += 8 ) {
					for( x = 0; x < 96; x++ ) {
						UINT8 data = m_p_ram[ ( y * 12 ) + x ];

						m_bitmap.pix16(y + 0, x) = ( data & 0x01 ) ? 3 : 0;
						m_bitmap.pix16(y + 1, x) = ( data & 0x02 ) ? 3 : 0;
						m_bitmap.pix16(y + 2, x) = ( data & 0x04 ) ? 3 : 0;
						m_bitmap.pix16(y + 3, x) = ( data & 0x08 ) ? 3 : 0;
						m_bitmap.pix16(y + 4, x) = ( data & 0x10 ) ? 3 : 0;
						m_bitmap.pix16(y + 5, x) = ( data & 0x20 ) ? 3 : 0;
						m_bitmap.pix16(y + 6, x) = ( data & 0x40 ) ? 3 : 0;
						m_bitmap.pix16(y + 7, x) = ( data & 0x80 ) ? 3 : 0;
					}
				}

				/* Set PRC Copy interrupt */
				m_pm_reg[0x27] |= 0x80;
				check_irqs();
			}
		}

		/* Set possible input irqs */
		m_pm_reg[0x29] |= ~ m_inputs->read();
	}
}


void pokemini_state::machine_start()
{
	/* Clear internal structures */
	memset( &m_prc, 0, sizeof(m_prc) );
	memset( &m_timers, 0, sizeof(m_timers) );
	memset( m_pm_reg, 0, sizeof(m_pm_reg) );

	/* Set up timers */
	m_timers.seconds_timer = timer_alloc(TIMER_SECONDS);
	m_timers.seconds_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));

	m_timers.hz256_timer = timer_alloc(TIMER_256HZ);
	m_timers.hz256_timer->adjust(attotime::zero, 0, attotime::from_hz(256));

	m_timers.timer1 = timer_alloc(TIMER_1);
	m_timers.timer1_hi = timer_alloc(TIMER_1_HI);
	m_timers.timer2 = timer_alloc(TIMER_2);
	m_timers.timer2_hi = timer_alloc(TIMER_2_HI);
	m_timers.timer3 = timer_alloc(TIMER_3);
	m_timers.timer3_hi = timer_alloc(TIMER_3_HI);

	/* Set up the PRC */
	m_prc.max_frame_count = 2;
	m_prc.count_timer = timer_alloc(TIMER_PRC);
	m_prc.count_timer->adjust( attotime::zero, 0, m_maincpu->cycles_to_attotime(55640 / 65) );
}


void pokemini_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_SECONDS:
			seconds_timer_callback();
			break;

		case TIMER_256HZ:
			timer_256hz_callback();
			break;

		case TIMER_1:
			timer1_callback();
			break;

		case TIMER_1_HI:
			timer1_hi_callback();
			break;

		case TIMER_2:
			timer2_callback();
			break;

		case TIMER_2_HI:
			timer2_hi_callback();
			break;

		case TIMER_3:
			timer3_callback();
			break;

		case TIMER_3_HI:
			timer3_hi_callback();
			break;

		case TIMER_PRC:
			prc_counter_callback();
			break;
	}
}


static const INT16 speaker_levels[] = {-32768, 0, 32767};

void pokemini_state::video_start()
{
	machine().first_screen()->register_screen_bitmap(m_bitmap);
}


UINT32 pokemini_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


static MACHINE_CONFIG_START( pokemini, pokemini_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", MINX, 4000000)
	MCFG_CPU_PROGRAM_MAP(pokemini_mem_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_I2CMEM_ADD("i2cmem")
	MCFG_I2CMEM_DATA_SIZE(0x2000)

	/* This still needs to be improved to actually match the hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_UPDATE_DRIVER(pokemini_state, screen_update)
	MCFG_SCREEN_SIZE( 96, 64 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 95, 0, 63 )
	MCFG_SCREEN_REFRESH_RATE( 72 )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_PALETTE_ADD("palette", 4)
	MCFG_PALETTE_INIT_OWNER(pokemini_state, pokemini)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SPEAKER_LEVELS(3, speaker_levels)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "pokemini_cart")
	MCFG_GENERIC_EXTENSIONS("bin,min")
	MCFG_GENERIC_LOAD(pokemini_state, pokemini_cart)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "pokemini")
MACHINE_CONFIG_END

ROM_START( pokemini )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "bios.min", 0x0000, 0x1000, CRC(aed3c14d) SHA1(daad4113713ed776fbd47727762bca81ba74915f) )
ROM_END


CONS( 2001, pokemini, 0, 0, pokemini, pokemini, driver_device, 0, "Nintendo", "Pokemon Mini", MACHINE_NO_SOUND )
