// license:BSD-3-Clause
// copyright-holders:Anthony Kruize,Wilbert Pol
/***************************************************************************

  gb.c

  Driver file to handle emulation of the Nintendo Game Boy.
  By:

  Hans de Goede               1998
  Anthony Kruize              2002
  Wilbert Pol                 2004 (Megaduck/Cougar Boy)

  TODO list:
  - Do correct LCD stat timing
  - Add Game Boy Light (Japan, 1997) - does it differ from gbpocket?
  - SGB should be moved to SNES driver
  - Emulate OAM corruption bug on 16bit inc/dec in $fe** region

***************************************************************************/

#include "emu.h"

#include "bus/gameboy/carts.h"
#include "bus/gameboy/gbslot.h"
#include "bus/gameboy/mdslot.h"
#include "cpu/lr35902/lr35902.h"
#include "machine/ram.h"
#include "sound/gb.h"
#include "video/gb_lcd.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

//#define VERBOSE 1
#include "logmacro.h"


namespace {


#define DMG_FRAMES_PER_SECOND   59.732155
#define SGB_FRAMES_PER_SECOND   61.17


class base_state : public driver_device
{
public:
	base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cartslot(*this, "cartslot"),
		m_maincpu(*this, "maincpu"),
		m_apu(*this, "apu"),
		m_inputs(*this, "INPUTS"),
		m_ppu(*this, "ppu"),
		m_palette(*this, "palette")
	{ }

protected:
	enum
	{
		SIO_ENABLED = 0x80,
		SIO_FAST_CLOCK = 0x02,
		SIO_INTERNAL_CLOCK = 0x01
	};

	static constexpr XTAL MASTER_CLOCK = 4.194304_MHz_XTAL;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void gb_io_w(offs_t offset, uint8_t data);
	uint8_t gb_ie_r();
	void gb_ie_w(uint8_t data);
	uint8_t gb_io_r(offs_t offset);

	void gb_timer_callback(uint8_t data);

	void gb_init_regs();

	uint8_t       m_gb_io[0x10]{};

	/* Timer related */
	uint16_t      m_divcount = 0;
	uint8_t       m_shift = 0;
	uint16_t      m_shift_cycles = 0;
	uint8_t       m_triggering_irq = 0;
	uint8_t       m_reloading = 0;

	/* Serial I/O related */
	uint16_t      m_internal_serial_clock = 0;
	uint16_t      m_internal_serial_frequency = 0;
	uint32_t      m_sio_count = 0;             /* Serial I/O counter */

	required_device<gb_cart_slot_device_base> m_cartslot;

	required_device<lr35902_cpu_device> m_maincpu;
	required_device<gameboy_sound_device> m_apu;
	required_ioport m_inputs;
	required_device<dmg_ppu_device> m_ppu;
	required_device<palette_device> m_palette;

private:
	void gb_timer_increment();
	void gb_timer_check_irq();
	void gb_serial_timer_tick();
};


class gb_state : public base_state
{
public:
	gb_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag),
		m_region_boot(*this, "maincpu"),
		m_boot_view(*this, "boot"),
		m_bios_hack(*this, "SKIP_CHECK")
	{ }

	void gameboy(machine_config &config);
	void gbpocket(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void disable_boot();

	void gb_io2_w(offs_t offset, uint8_t data);

	required_region_ptr<uint8_t> m_region_boot;
	memory_view m_boot_view;

private:
	u8 boot_r(offs_t offset);

	void gb_palette(palette_device &palette) const;
	void gbp_palette(palette_device &palette) const;

	void gameboy_map(address_map &map) ATTR_COLD;

	required_ioport m_bios_hack;
};


class sgb_state : public gb_state
{
public:
	sgb_state(const machine_config &mconfig, device_type type, const char *tag) :
		gb_state(mconfig, type, tag)
	{ }

	void supergb(machine_config &config);
	void supergb2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void sgb_palette(palette_device &palette) const;

	void sgb_io_w(offs_t offset, uint8_t data);

	void sgb_map(address_map &map) ATTR_COLD;

	int8_t m_sgb_packets = 0;
	uint8_t m_sgb_bitcount = 0;
	uint8_t m_sgb_bytecount = 0;
	uint8_t m_sgb_start = 0;
	uint8_t m_sgb_rest = 0;
	uint8_t m_sgb_controller_no = 0;
	uint8_t m_sgb_controller_mode = 0;
	uint8_t m_sgb_data[0x100]{};
};


class gbc_state : public gb_state
{
public:
	gbc_state(const machine_config &mconfig, device_type type, const char *tag) :
		gb_state(mconfig, type, tag),
		m_rambank(*this, "cgb_ram"),
		m_bankedram(*this, "banked_ram", 7 * 0x1000, ENDIANNESS_LITTLE)
	{ }

	void gbcolor(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr XTAL GBC_CLOCK = 8.388_MHz_XTAL;

	void gbc_io_w(offs_t offset, uint8_t data);
	void gbc_io2_w(offs_t offset, uint8_t data);
	uint8_t gbc_io2_r(offs_t offset);

	void gbc_map(address_map &map) ATTR_COLD;

	required_memory_bank m_rambank;
	memory_share_creator<uint8_t> m_bankedram;

	memory_passthrough_handler m_boot_high_tap;
};


class megaduck_state : public base_state
{
public:
	megaduck_state(const machine_config &mconfig, device_type type, const char *tag) :
		base_state(mconfig, type, tag)
	{ }

	void megaduck(machine_config &config);

private:
	uint8_t megaduck_video_r(offs_t offset);
	void megaduck_video_w(offs_t offset, uint8_t data);
	void megaduck_sound_w1(offs_t offset, uint8_t data);
	uint8_t megaduck_sound_r1(offs_t offset);
	void megaduck_sound_w2(offs_t offset, uint8_t data);
	uint8_t megaduck_sound_r2(offs_t offset);
	void megaduck_palette(palette_device &palette) const;
	void megaduck_map(address_map &map) ATTR_COLD;
};



/* RAM layout defines */

#define JOYPAD      m_gb_io[0x00]   // Joystick: 1.1.P15.P14.P13.P12.P11.P10
#define SIODATA     m_gb_io[0x01]   // Serial IO data buffer
#define SIOCONT     m_gb_io[0x02]   // Serial IO control register
#define TIMECNT     m_gb_io[0x05]   // Timer counter. Gen. int. when it overflows
#define TIMEMOD     m_gb_io[0x06]   // New value of TimeCount after it overflows
#define TIMEFRQ     m_gb_io[0x07]   // Timer frequency and start/stop switch


void base_state::gb_init_regs()
{
	/* Initialize the registers */
	SIODATA = 0x00;
	SIOCONT = 0x7E;

	gb_io_w(0x05, 0x00);       /* TIMECNT */
	gb_io_w(0x06, 0x00);       /* TIMEMOD */
}


void base_state::machine_start()
{
	save_item(NAME(m_gb_io));
	save_item(NAME(m_divcount));
	save_item(NAME(m_shift));
	save_item(NAME(m_shift_cycles));
	save_item(NAME(m_triggering_irq));
	save_item(NAME(m_reloading));
	save_item(NAME(m_sio_count));
}

void gb_state::machine_start()
{
	base_state::machine_start();

	m_maincpu->space(AS_PROGRAM).install_view(0x0000, 0x08ff, m_boot_view);
	m_boot_view[0].install_read_handler(0x0000, 0x00ff, read8sm_delegate(*this, NAME(&gb_state::boot_r)));
}

void gbc_state::machine_start()
{
	gb_state::machine_start();

	m_boot_view[0].install_rom(0x0200, 0x08ff, &m_region_boot[0x100]);

	m_rambank->configure_entry(0, &m_bankedram[0]);
	m_rambank->configure_entries(1, 7, &m_bankedram[0], 0x1000);
}

void sgb_state::machine_start()
{
	gb_state::machine_start();

	m_sgb_packets = -1;

	save_item(NAME(m_sgb_packets));
	save_item(NAME(m_sgb_bitcount));
	save_item(NAME(m_sgb_bytecount));
	save_item(NAME(m_sgb_start));
	save_item(NAME(m_sgb_rest));
	save_item(NAME(m_sgb_controller_no));
	save_item(NAME(m_sgb_controller_mode));
	save_item(NAME(m_sgb_data));
}


void base_state::machine_reset()
{
	m_apu->sound_w(0x16, 0x00); // Initialize sound hardware

	m_divcount = 8;
	m_internal_serial_clock = 0;
	m_internal_serial_frequency = 512 / 2;
	m_triggering_irq = 0;
	m_shift = 10; // slowest timer?
	m_shift_cycles = 1 << m_shift;

	// Set registers to default/startup values
	m_gb_io[0x00] = 0xcf;
	m_gb_io[0x01] = 0x00;
	m_gb_io[0x02] = 0x7e;
	m_gb_io[0x03] = 0xff;
	m_gb_io[0x07] = 0xf8;       // Upper bits of TIMEFRQ register are set to 1
}

void gb_state::machine_reset()
{
	base_state::machine_reset();

	m_boot_view.select(0);
}

void gbc_state::machine_reset()
{
	gb_state::machine_reset();

	gb_init_regs();

	std::fill_n(&m_bankedram[0], m_bankedram.length(), 0);
}

void sgb_state::machine_reset()
{
	gb_state::machine_reset();

	gb_init_regs();
}


void gb_state::disable_boot()
{
	m_boot_view.disable();
}


void base_state::gb_io_w(offs_t offset, uint8_t data)
{
	static const uint8_t timer_shifts[4] = {10, 4, 6, 8};

	switch (offset)
	{
	case 0x00:                      /* JOYP - Joypad */
		JOYPAD = 0xCF | data;
		if (!(data & 0x20))
			JOYPAD &= (m_inputs->read() >> 4) | 0xF0;
		if (!(data & 0x10))
			JOYPAD &= m_inputs->read() | 0xF0;
		return;
	case 0x01:                      /* SB - Serial transfer data */
		break;
	case 0x02:                      /* SC - SIO control */
		switch (data & 0x81)
		{
		case 0x00:
		case 0x01:
			m_sio_count = 0;
			break;
		case 0x80:              /* enabled & external clock */
			m_sio_count = 16;
			break;
		case 0x81:              /* enabled & internal clock */
			m_sio_count = 16;
			break;
		}
logerror("SIOCONT write, serial clock is %04x\n", m_internal_serial_clock);
		data |= 0x7E; // unused bits stay high
		break;
	case 0x03:
		return;
	case 0x04:                      /* DIV - Divider register */
		/* Force increment of TIMECNT register when the 'highest' bit is set */
		if ((m_divcount >> (m_shift - 1)) & 1)
		{
			gb_timer_increment();
		}
		LOG("DIV write\n");
		m_divcount = 0;
		return;
	case 0x05:                      /* TIMA - Timer counter */
		/* Check if the counter is being reloaded in this cycle */
		if ((TIMEFRQ & 0x04) && TIMECNT == TIMEMOD && (m_divcount & (m_shift_cycles - 1)) == 4)
		{
			data = TIMEMOD;
		}
		break;
	case 0x06:                      /* TMA - Timer module */
		/* Check if the counter is being reloaded in this cycle */
		if ((TIMEFRQ & 0x04) && TIMECNT == TIMEMOD && (m_divcount & (m_shift_cycles - 1)) == 4)
		{
			TIMECNT = data;
		}
		break;
	case 0x07:                      /* TAC - Timer control */
		data |= 0xF8;
		/* Check if timer is just disabled or the timer frequency is changing */
		if ((!(data & 0x04) && (TIMEFRQ & 0x04)) || ((data & 0x04) && (TIMEFRQ & 0x04) && (data & 0x03) != (TIMEFRQ & 0x03)))
		{
			/* Check if TIMECNT should be incremented */
			if ((m_divcount & (m_shift_cycles - 1)) >= (m_shift_cycles >> 1))
			{
				gb_timer_increment();
			}
		}
		m_shift = timer_shifts[data & 0x03];
		m_shift_cycles = 1 << m_shift;
		break;
	case 0x0F:                      /* IF - Interrupt flag */
		m_ppu->update_state();
		LOG("write if\n");
		data &= 0x1F;
		m_maincpu->set_if(data);
		break;
	}

	m_gb_io[offset] = data;
}

void gb_state::gb_io2_w(offs_t offset, uint8_t data)
{
	if (offset == 0x10)
		disable_boot(); // disable boot ROM
	else
		m_ppu->video_w(offset, data);
}

u8 gb_state::boot_r(offs_t offset)
{
	if (m_bios_hack->read())
	{
		// patch out logo and checksum checks
		// useful to run some pirate carts until properly emulated, or to test homebrew
		if (offset == 0xe9 || offset == 0xea)
			return 0x00;
		if (offset == 0xfa || offset == 0xfb)
			return 0x00;
	}
	return m_region_boot[offset];
}

#ifdef MAME_DEBUG
static const char *const sgbcmds[32] =
{
	/* 0x00 */ "PAL01   ",
	/* 0x01 */ "PAL23   ",
	/* 0x02 */ "PAL03   ",
	/* 0x03 */ "PAL12   ",
	/* 0x04 */ "ATTR_BLK",
	/* 0x05 */ "ATTR_LIN",
	/* 0x06 */ "ATTR_DIV",
	/* 0x07 */ "ATTR_CHR",
	/* 0x08 */ "SOUND   ",
	/* 0x09 */ "SOU_TRN ",
	/* 0x0A */ "PAL_SET ",
	/* 0x0B */ "PAL_TRN ",
	/* 0x0C */ "ATRC_EN ",
	/* 0x0D */ "TEST_EN ",
	/* 0x0E */ "ICON_EN ",
	/* 0x0F */ "DATA_SND",
	/* 0x10 */ "DATA_TRN",
	/* 0x11 */ "MLT_REG ",
	/* 0x12 */ "JUMP    ",
	/* 0x13 */ "CHR_TRN ",
	/* 0x14 */ "PCT_TRN ",
	/* 0x15 */ "ATTR_TRN",
	/* 0x16 */ "ATTR_SET",
	/* 0x17 */ "MASK_EN ",
	/* 0x18 */ "OBJ_TRN ",
	/* 0x19 */ "PAL_PRI ",
	/* 0x1A */ "????????",
	/* 0x1B */ "????????",
	/* 0x1C */ "????????",
	/* 0x1D */ "????????",
	/* 0x1E */ "????????",
	/* 0x1F */ "????????"
};
#endif

void sgb_state::sgb_io_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x00:
			switch (data & 0x30)
			{
			case 0x00:                 /* start condition */
				if (m_sgb_start)
					logerror("SGB: Start condition before end of transfer ??\n");
				m_sgb_bitcount = 0;
				m_sgb_start = 1;
				m_sgb_rest = 0;
				JOYPAD = 0x0F & ((m_inputs->read() >> 4) | m_inputs->read() | 0xF0);
				break;
			case 0x10:                 /* data true */
				if (m_sgb_rest)
				{
					/* We should test for this case , but the code below won't
					   work with the current setup */
#if 0
					if (m_sgb_bytecount == 16)
					{
						logerror("SGB: end of block is not zero!");
						m_sgb_start = 0;
					}
#endif
					m_sgb_data[m_sgb_bytecount] >>= 1;
					m_sgb_data[m_sgb_bytecount] |= 0x80;
					m_sgb_bitcount++;
					if (m_sgb_bitcount == 8)
					{
						m_sgb_bitcount = 0;
						m_sgb_bytecount++;
					}
					m_sgb_rest = 0;
				}
				JOYPAD = 0x1F & ((m_inputs->read() >> 4) | 0xF0);
				break;
			case 0x20:              /* data false */
				if (m_sgb_rest)
				{
					if (m_sgb_bytecount == 16 && m_sgb_packets == -1)
					{
#ifdef MAME_DEBUG
						LOG("SGB: %s (%02X) pkts: %d data: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
								sgbcmds[m_sgb_data[0] >> 3],m_sgb_data[0] >> 3, m_sgb_data[0] & 0x07, m_sgb_data[1], m_sgb_data[2], m_sgb_data[3],
								m_sgb_data[4], m_sgb_data[5], m_sgb_data[6], m_sgb_data[7],
								m_sgb_data[8], m_sgb_data[9], m_sgb_data[10], m_sgb_data[11],
								m_sgb_data[12], m_sgb_data[13], m_sgb_data[14], m_sgb_data[15]);
#endif
						m_sgb_packets = m_sgb_data[0] & 0x07;
						m_sgb_start = 0;
					}
					if (m_sgb_bytecount == (m_sgb_packets << 4))
					{
						switch (m_sgb_data[0] >> 3)
						{
							case 0x11:  /* MLT_REQ - Multi controller request */
								if (m_sgb_data[1] == 0x00)
									m_sgb_controller_mode = 0;
								else if (m_sgb_data[1] == 0x01)
									m_sgb_controller_mode = 2;
								break;
							default:
								dynamic_cast<sgb_ppu_device*>(m_ppu.target())->sgb_io_write_pal(m_sgb_data[0] >> 3, &m_sgb_data[0]);
								break;
						}
						m_sgb_start = 0;
						m_sgb_bytecount = 0;
						m_sgb_packets = -1;
					}
					if (m_sgb_start)
					{
						m_sgb_data[m_sgb_bytecount] >>= 1;
						m_sgb_bitcount++;
						if (m_sgb_bitcount == 8)
						{
							m_sgb_bitcount = 0;
							m_sgb_bytecount++;
						}
					}
					m_sgb_rest = 0;
				}
				JOYPAD = 0x2F & (m_inputs->read() | 0xF0);
				break;
			case 0x30:                 /* rest condition */
				if (m_sgb_start)
					m_sgb_rest = 1;
				if (m_sgb_controller_mode)
				{
					m_sgb_controller_no++;
					if (m_sgb_controller_no == m_sgb_controller_mode)
						m_sgb_controller_no = 0;
					JOYPAD = 0x3F - m_sgb_controller_no;
				}
				else
					JOYPAD = 0x3F;

				/* Hack to let cartridge know it's running on an SGB */
				if ((m_sgb_data[0] >> 3) == 0x1F)
					JOYPAD = 0x3E;
				break;
			}
			return;
		default:
			/* we didn't handle the write, so pass it to the GB handler */
			gb_io_w(offset, data);
			return;
	}

	m_gb_io[offset] = data;
}

/* Interrupt Enable register */
uint8_t base_state::gb_ie_r()
{
	return m_maincpu->get_ie();
}

void base_state::gb_ie_w(uint8_t data)
{
	m_maincpu->set_ie(data);
}

/* IO read */
uint8_t base_state::gb_io_r(offs_t offset)
{
	switch(offset)
	{
		case 0x04:
			LOG("read DIV, divcount = %04x\n", m_divcount);
			return (m_divcount >> 8) & 0xFF;
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x05:
		case 0x06:
		case 0x07:
			return m_gb_io[offset];
		case 0x0F:
			/* Make sure the internal states are up to date */
			m_ppu->update_state();
			LOG("read if\n");
logerror("IF read, serial clock is %04x\n", m_internal_serial_clock);
			return 0xE0 | m_maincpu->get_if();
		default:
			/* Unsupported registers return 0xFF */
			return 0xFF;
	}
}


/* Called when 512 internal cycles are passed */
void base_state::gb_serial_timer_tick()
{
	if (SIOCONT & SIO_ENABLED)
	{
		if (m_sio_count & 1)
		{
			/* Shift in a received bit */
			SIODATA = (SIODATA << 1) | 0x01;
		}
		/* Decrement number of handled bits */
		m_sio_count--;

		LOG("%04x - gb_serial_timer_proc: SIODATA = %02x, sio_count = %u\n", m_maincpu->pc(), SIODATA, m_sio_count);
		/* If all bits done, stop timer and trigger interrupt */
		if (m_sio_count == 0)
		{
			SIOCONT &= ~SIO_ENABLED;
			m_maincpu->set_input_line(lr35902_cpu_device::SIO_INT, ASSERT_LINE);
			// Make sure the state is updated during the current timeslice in case it is read.
			m_maincpu->execute_set_input(lr35902_cpu_device::SIO_INT, ASSERT_LINE);
		}
	}
}


void base_state::gb_timer_check_irq()
{
	m_reloading = 0;
	if (m_triggering_irq)
	{
		m_triggering_irq = 0;
		if (TIMECNT == 0)
		{
			TIMECNT = TIMEMOD;
			m_maincpu->set_input_line(lr35902_cpu_device::TIM_INT, ASSERT_LINE);
			// Make sure the state is updated during the current timeslice in case it is read.
			m_maincpu->execute_set_input(lr35902_cpu_device::TIM_INT, ASSERT_LINE);
			m_reloading = 1;
		}
	}
}

void base_state::gb_timer_increment()
{
	gb_timer_check_irq();

	LOG("increment timer\n");
	TIMECNT += 1;
	if (TIMECNT == 0)
	{
		m_triggering_irq = 1;
	}
}

// This gets called while the cpu is executing instructions to keep the timer state in sync
void base_state::gb_timer_callback(uint8_t data)
{
	uint16_t old_gb_divcount = m_divcount;
	uint16_t old_internal_serial_clock = m_internal_serial_clock;
	m_divcount += data;
	m_internal_serial_clock += data;

	if ((old_gb_divcount >> 8) != (m_divcount >> 8))
	{
		//LOG("DIV became %02x\n", m_divcount >> 8);
	}
	gb_timer_check_irq();

	if (TIMEFRQ & 0x04)
	{
		uint16_t old_count = old_gb_divcount >> m_shift;
		uint16_t new_count = m_divcount >> m_shift;
		if (data > m_shift_cycles)
		{
			gb_timer_increment();
			old_count++;
		}
		if (new_count != old_count)
		{
			gb_timer_increment();
			if (new_count << m_shift < m_divcount)
			{
				gb_timer_check_irq();
			}
		}
	}

	if (((m_internal_serial_clock ^ old_internal_serial_clock) & m_internal_serial_frequency) && (SIOCONT & SIO_INTERNAL_CLOCK))
	{
		gb_serial_timer_tick();
	}
}


void gbc_state::gbc_io_w(offs_t offset, uint8_t data)
{
	gb_io_w(offset, data);

	// On CGB the internal serial transfer clock is selectable
	if (offset == 0x02)
	{
		m_internal_serial_frequency = ((data & SIO_FAST_CLOCK) ? 16 : 512) / 2;
		SIOCONT = (SIOCONT & ~SIO_FAST_CLOCK) | (data & SIO_FAST_CLOCK);
	}
}


void gbc_state::gbc_io2_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0x0D:  // KEY1 - Prepare speed switch
			m_maincpu->set_speed(data);
			return;
		case 0x10:  // BFF - boot ROM disable
			disable_boot();
			return;
		case 0x16:  // RP - Infrared port
			break;
		case 0x30:  // SVBK - RAM bank select
			m_rambank->set_entry(data & 0x07);
			break;
		default:
			break;
	}
	m_ppu->video_w(offset, data);
}

uint8_t gbc_state::gbc_io2_r(offs_t offset)
{
	switch (offset)
	{
	case 0x0D:  // KEY1
		return m_maincpu->get_speed();
	case 0x16:  // RP - Infrared port
		break;
	case 0x30:  // SVBK - RAM bank select
		return m_rambank->entry();
	default:
		break;
	}
	return m_ppu->video_r(offset);
}

/****************************************************************************

  Megaduck routines

 ****************************************************************************/

/*
 Map megaduck video related area on to regular Game Boy video area

 Different locations of the video registers:
 Register      Game Boy   Mega Duck
 LCDC          FF40       FF10  (See different bit order below)
 STAT          FF41       FF11
 SCY           FF42       FF12
 SCX           FF43       FF13
 LY            FF44       FF18
 LYC           FF45       FF19
 DMA           FF46       FF1A
 BGP           FF47       FF1B
 OBP0          FF48       FF14
 OBP1          FF49       FF15
 WY            FF4A       FF16
 WX            FF4B       FF17
 Unused        FF4C       FF4C (?)
 Unused        FF4D       FF4D (?)
 Unused        FF4E       FF4E (?)
 Unused        FF4F       FF4F (?)

 Different LCDC register

 Game Boy       Mega Duck
 0                      6       - BG & Window Display : 0 - Off, 1 - On
 1                      0       - OBJ Display: 0 - Off, 1 - On
 2                      1       - OBJ Size: 0 - 8x8, 1 - 8x16
 3                      2       - BG Tile Map Display: 0 - 9800, 1 - 9C00
 4                      4       - BG & Window Tile Data Select: 0 - 8800, 1 - 8000
 5                      5       - Window Display: 0 - Off, 1 - On
 6                      3       - Window Tile Map Display Select: 0 - 9800, 1 - 9C00
 7                      7       - LCD Operation

 **************/

uint8_t megaduck_state::megaduck_video_r(offs_t offset)
{
	uint8_t data;

	if ((offset & 0x0C) && ((offset & 0x0C) ^ 0x0C))
	{
		offset ^= 0x0C;
	}
	data = m_ppu->video_r(offset);
	if (offset)
		return data;
	return bitswap<8>(data,7,0,5,4,6,3,2,1);
}

void megaduck_state::megaduck_video_w(offs_t offset, uint8_t data)
{
	if (!offset)
	{
		data = bitswap<8>(data,7,3,5,4,2,1,0,6);
	}
	if ((offset & 0x0C) && ((offset & 0x0C) ^ 0x0C))
	{
		offset ^= 0x0C;
	}
	m_ppu->video_w(offset, data);
}

// Map megaduck audio offset to game boy audio offsets
// Envelope and LFSR register nibbles are reversed relative to the game boy

static const uint8_t megaduck_sound_offsets[16] = { 0, 2, 1, 3, 4, 6, 5, 7, 8, 9, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

void megaduck_state::megaduck_sound_w1(offs_t offset, uint8_t data)
{
	if ((offset == 0x01) || (offset == 0x07))
		m_apu->sound_w(megaduck_sound_offsets[offset], ((data & 0x0f)<<4) | ((data & 0xf0)>>4));
	else
		m_apu->sound_w(megaduck_sound_offsets[offset], data);
}

uint8_t megaduck_state::megaduck_sound_r1(offs_t offset)
{
	uint8_t data = m_apu->sound_r(megaduck_sound_offsets[offset]);
	if ((offset == 0x01) || (offset == 0x07))
		return ((data & 0x0f)<<4) | ((data & 0xf0)>>4);
	else
		return data;
}

void megaduck_state::megaduck_sound_w2(offs_t offset, uint8_t data)
{
	if ((offset == 0x01) || (offset == 0x02))
		m_apu->sound_w(0x10 + megaduck_sound_offsets[offset], ((data & 0x0f)<<4) | ((data & 0xf0)>>4));
	else
		m_apu->sound_w(0x10 + megaduck_sound_offsets[offset], data);
}

uint8_t megaduck_state::megaduck_sound_r2(offs_t offset)
{
	uint8_t data = m_apu->sound_r(0x10 + megaduck_sound_offsets[offset]);
	if ((offset == 0x01) || (offset == 0x02))
		return ((data & 0x0f)<<4) | ((data & 0xf0)>>4);
	else
		return data;
}


void gb_state::gameboy_map(address_map &map)
{
	map.unmap_value_high();
	map(0x8000, 0x9fff).rw(m_ppu, FUNC(dmg_ppu_device::vram_r), FUNC(dmg_ppu_device::vram_w));
	map(0xc000, 0xdfff).mirror(0x2000).ram();
	map(0xfe00, 0xfeff).rw(m_ppu, FUNC(dmg_ppu_device::oam_r), FUNC(dmg_ppu_device::oam_w));
	map(0xff00, 0xff0f).rw(FUNC(gb_state::gb_io_r), FUNC(gb_state::gb_io_w));
	map(0xff10, 0xff26).rw(m_apu, FUNC(gameboy_sound_device::sound_r), FUNC(gameboy_sound_device::sound_w));
	map(0xff27, 0xff2f).noprw();
	map(0xff30, 0xff3f).rw(m_apu, FUNC(gameboy_sound_device::wave_r), FUNC(gameboy_sound_device::wave_w));
	map(0xff40, 0xff7f).r(m_ppu, FUNC(dmg_ppu_device::video_r)).w(FUNC(gb_state::gb_io2_w));
	map(0xff80, 0xfffe).ram();
	map(0xffff, 0xffff).rw(FUNC(gb_state::gb_ie_r), FUNC(gb_state::gb_ie_w));
}

void sgb_state::sgb_map(address_map &map)
{
	map.unmap_value_high();
	map(0x8000, 0x9fff).rw(m_ppu, FUNC(sgb_ppu_device::vram_r), FUNC(sgb_ppu_device::vram_w));
	map(0xc000, 0xdfff).mirror(0x2000).ram();
	map(0xfe00, 0xfeff).rw(m_ppu, FUNC(sgb_ppu_device::oam_r), FUNC(sgb_ppu_device::oam_w));
	map(0xff00, 0xff0f).rw(FUNC(sgb_state::gb_io_r), FUNC(sgb_state::sgb_io_w));
	map(0xff10, 0xff26).rw(m_apu, FUNC(gameboy_sound_device::sound_r), FUNC(gameboy_sound_device::sound_w));
	map(0xff27, 0xff2f).noprw();
	map(0xff30, 0xff3f).rw(m_apu, FUNC(gameboy_sound_device::wave_r), FUNC(gameboy_sound_device::wave_w));
	map(0xff40, 0xff7f).r(m_ppu, FUNC(sgb_ppu_device::video_r)).w(FUNC(sgb_state::gb_io2_w));
	map(0xff80, 0xfffe).ram();
	map(0xffff, 0xffff).rw(FUNC(sgb_state::gb_ie_r), FUNC(sgb_state::gb_ie_w));
}

void gbc_state::gbc_map(address_map &map)
{
	map.unmap_value_high();
	map(0x8000, 0x9fff).rw(m_ppu, FUNC(cgb_ppu_device::vram_r), FUNC(cgb_ppu_device::vram_w));
	map(0xc000, 0xcfff).mirror(0x2000).ram();
	map(0xd000, 0xdfff).mirror(0x2000).bankrw(m_rambank);
	map(0xfe00, 0xfeff).rw(m_ppu, FUNC(cgb_ppu_device::oam_r), FUNC(cgb_ppu_device::oam_w));
	map(0xff00, 0xff0f).rw(FUNC(gbc_state::gb_io_r), FUNC(gbc_state::gbc_io_w));
	map(0xff10, 0xff26).rw(m_apu, FUNC(gameboy_sound_device::sound_r), FUNC(gameboy_sound_device::sound_w));
	map(0xff27, 0xff2f).noprw();
	map(0xff30, 0xff3f).rw(m_apu, FUNC(gameboy_sound_device::wave_r), FUNC(gameboy_sound_device::wave_w));
	map(0xff40, 0xff7f).rw(FUNC(gbc_state::gbc_io2_r), FUNC(gbc_state::gbc_io2_w));
	map(0xff80, 0xfffe).ram();
	map(0xffff, 0xffff).rw(FUNC(gbc_state::gb_ie_r), FUNC(gbc_state::gb_ie_w));
}

void megaduck_state::megaduck_map(address_map &map)
{
	map.unmap_value_high();
	map(0x8000, 0x9fff).rw(m_ppu, FUNC(dmg_ppu_device::vram_r), FUNC(dmg_ppu_device::vram_w));
	map(0xc000, 0xfdff).ram();    // 8k or 16k? RAM
	map(0xfe00, 0xfeff).rw(m_ppu, FUNC(dmg_ppu_device::oam_r), FUNC(dmg_ppu_device::oam_w));
	map(0xff00, 0xff0f).rw(FUNC(megaduck_state::gb_io_r), FUNC(megaduck_state::gb_io_w));
	map(0xff10, 0xff1f).rw(FUNC(megaduck_state::megaduck_video_r), FUNC(megaduck_state::megaduck_video_w));
	map(0xff20, 0xff2f).rw(FUNC(megaduck_state::megaduck_sound_r1), FUNC(megaduck_state::megaduck_sound_w1));
	map(0xff30, 0xff3f).rw(m_apu, FUNC(gameboy_sound_device::wave_r), FUNC(gameboy_sound_device::wave_w));
	map(0xff40, 0xff46).rw(FUNC(megaduck_state::megaduck_sound_r2), FUNC(megaduck_state::megaduck_sound_w2));
	map(0xff47, 0xff7f).noprw();
	map(0xff80, 0xfffe).ram();
	map(0xffff, 0xffff).rw(FUNC(megaduck_state::gb_ie_r), FUNC(megaduck_state::gb_ie_w));
}


static INPUT_PORTS_START( megaduck )
	PORT_START("INPUTS")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_NAME("Left")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("Right")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_NAME("Up")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_NAME("Down")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2)        PORT_NAME("Button A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1)        PORT_NAME("Button B")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START)          PORT_NAME("Start")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT)         PORT_NAME("Select")
INPUT_PORTS_END

static INPUT_PORTS_START( gameboy )
	PORT_INCLUDE(megaduck)

	PORT_START("SKIP_CHECK")
	PORT_CONFNAME( 0x01, 0x00, "[HACK] Skip BIOS Logo check" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END



static constexpr rgb_t palette_gb[] =
{
	// Simple black and white palette
	/*  0xff,0xff,0xff,
	 0xb0,0xb0,0xb0,
	 0x60,0x60,0x60,
	 0x00,0x00,0x00 */

	// Possibly needs a little more green in it
	{ 0xff,0xfb,0x87 },     // Background
	{ 0xb1,0xae,0x4e },     // Light
	{ 0x84,0x80,0x4e },     // Medium
	{ 0x4e,0x4e,0x4e },     // Dark

	// Palette for Game Boy Pocket/Light
	{ 0xc4,0xcf,0xa1 },     // Background
	{ 0x8b,0x95,0x6d },     // Light
	{ 0x6b,0x73,0x53 },     // Medium
	{ 0x41,0x41,0x41 },     // Dark
};

static constexpr rgb_t palette_megaduck[] = {
	{ 0x6b, 0xa6, 0x4a }, { 0x43, 0x7a, 0x63 }, { 0x25, 0x59, 0x55 }, { 0x12, 0x42, 0x4c }
};

// Initialise the palettes
void gb_state::gb_palette(palette_device &palette) const
{
	for (int i = 0; i < 4; i++)
		palette.set_pen_color(i, palette_gb[i]);
}

void gb_state::gbp_palette(palette_device &palette) const
{
	for (int i = 0; i < 4; i++)
		palette.set_pen_color(i, palette_gb[i + 4]);
}

void sgb_state::sgb_palette(palette_device &palette) const
{
	for (int i = 0; i < 32768; i++)
	{
		int const r = i & 0x1f;
		int const g = (i >> 5) & 0x1f;
		int const b = (i >> 10) & 0x1f;
		palette.set_pen_color(i, pal5bit(r), pal5bit(g), pal5bit(b));
	}
}


void megaduck_state::megaduck_palette(palette_device &palette) const
{
	for (int i = 0; i < 4; i++)
		palette.set_pen_color(i, palette_megaduck[i]);
}


void gb_state::gameboy(machine_config &config)
{
	// basic machine hardware
	LR35902(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &gb_state::gameboy_map);
	m_maincpu->timer_cb().set(FUNC(gb_state::gb_timer_callback));
	m_maincpu->set_halt_bug(true);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_raw(MASTER_CLOCK, 456, 0, 20 * 8, 154, 0, 18 * 8);
	screen.set_screen_update(m_ppu, FUNC(dmg_ppu_device::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfxdecode_device::empty);
	PALETTE(config, m_palette, FUNC(gb_state::gb_palette), 4);

	DMG_PPU(config, m_ppu, m_maincpu);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DMG_APU(config, m_apu, MASTER_CLOCK);
	m_apu->add_route(0, "lspeaker", 0.50);
	m_apu->add_route(1, "rspeaker", 0.50);

	// cartslot
	GB_CART_SLOT(config, m_cartslot, gameboy_cartridges, nullptr);
	m_cartslot->set_space(m_maincpu, AS_PROGRAM);

	SOFTWARE_LIST(config, "cart_list").set_original("gameboy");
	SOFTWARE_LIST(config, "gbc_list").set_compatible("gbcolor");
}

void sgb_state::supergb(machine_config &config)
{
	// basic machine hardware
	LR35902(config, m_maincpu, 4'295'454); // derived from SNES xtal
	m_maincpu->set_addrmap(AS_PROGRAM, &sgb_state::sgb_map);
	m_maincpu->timer_cb().set(FUNC(sgb_state::gb_timer_callback));
	m_maincpu->set_halt_bug(true);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_physical_aspect(4, 3); // runs on a TV, not an LCD
	screen.set_refresh_hz(SGB_FRAMES_PER_SECOND);
	screen.set_vblank_time(0);
	screen.set_screen_update(m_ppu, FUNC(dmg_ppu_device::screen_update));
	screen.set_palette(m_palette);
	screen.set_size(32*8, 28*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 28*8-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfxdecode_device::empty);
	PALETTE(config, m_palette, palette_device::BGR_555);

	SGB_PPU(config, m_ppu, m_maincpu);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DMG_APU(config, m_apu, 4'295'454);
	m_apu->add_route(0, "lspeaker", 0.50);
	m_apu->add_route(1, "rspeaker", 0.50);

	// cartslot
	GB_CART_SLOT(config, m_cartslot, gameboy_cartridges, nullptr);
	m_cartslot->set_space(m_maincpu, AS_PROGRAM);

	SOFTWARE_LIST(config, "cart_list").set_original("gameboy");
	SOFTWARE_LIST(config, "gbc_list").set_compatible("gbcolor");
}

void sgb_state::supergb2(machine_config &config)
{
	gameboy(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &sgb_state::sgb_map);

	// video hardware
	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_physical_aspect(4, 3); // runs on a TV, not an LCD
	screen.set_size(32*8, 28*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 28*8-1);

	m_palette->set_entries(32'768);
	m_palette->set_init(FUNC(sgb_state::sgb_palette));

	SGB_PPU(config.replace(), m_ppu, m_maincpu);
}

void gb_state::gbpocket(machine_config &config)
{
	gameboy(config);

	// video hardware
	m_palette->set_init(FUNC(gb_state::gbp_palette));

	MGB_PPU(config.replace(), m_ppu, m_maincpu);
}

void gbc_state::gbcolor(machine_config &config)
{
	// basic machine hardware
	LR35902(config, m_maincpu, GBC_CLOCK / 2); // FIXME: make the CPU device divide rather than multiply the clock frequency
	m_maincpu->set_addrmap(AS_PROGRAM, &gbc_state::gbc_map);
	m_maincpu->timer_cb().set(FUNC(gbc_state::gb_timer_callback));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_raw(GBC_CLOCK / 2, 456, 0, 20 * 8, 154, 0, 18 * 8);
	screen.set_screen_update(m_ppu, FUNC(dmg_ppu_device::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfxdecode_device::empty);
	PALETTE(config, m_palette, palette_device::BGR_555);

	CGB_PPU(config, m_ppu, m_maincpu);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	CGB04_APU(config, m_apu, GBC_CLOCK / 2);
	m_apu->add_route(0, "lspeaker", 0.50);
	m_apu->add_route(1, "rspeaker", 0.50);

	// cartslot
	GB_CART_SLOT(config, m_cartslot, gameboy_cartridges, nullptr);
	m_cartslot->set_space(m_maincpu, AS_PROGRAM);

	SOFTWARE_LIST(config, "cart_list").set_original("gbcolor");
	SOFTWARE_LIST(config, "gb_list").set_compatible("gameboy");
}

void megaduck_state::megaduck(machine_config &config)
{
	// basic machine hardware
	LR35902(config, m_maincpu, XTAL(4'194'304));
	m_maincpu->set_addrmap(AS_PROGRAM, &megaduck_state::megaduck_map);
	m_maincpu->timer_cb().set(FUNC(megaduck_state::gb_timer_callback));
	m_maincpu->set_halt_bug(true);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(DMG_FRAMES_PER_SECOND);
	screen.set_vblank_time(0);
	screen.set_screen_update(m_ppu, FUNC(dmg_ppu_device::screen_update));
	screen.set_palette(m_palette);
	screen.set_size(20*8, 18*8);
	screen.set_visarea(0*8, 20*8-1, 0*8, 18*8-1);

	GFXDECODE(config, "gfxdecode", m_palette, gfxdecode_device::empty);
	PALETTE(config, m_palette, FUNC(megaduck_state::megaduck_palette), 4);

	DMG_PPU(config, m_ppu, m_maincpu);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DMG_APU(config, m_apu, XTAL(4'194'304));
	m_apu->add_route(0, "lspeaker", 0.50);
	m_apu->add_route(1, "rspeaker", 0.50);

	// cartslot
	MEGADUCK_CART_SLOT(config, m_cartslot, megaduck_cartridges, nullptr);
	m_cartslot->set_space(m_maincpu, AS_PROGRAM);

	SOFTWARE_LIST(config, "cart_list").set_original("megaduck");
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(gameboy)
	ROM_REGION(0x0100, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "vx", "DMG vX")
	ROMX_LOAD("dmg_boot.bin", 0x0000, 0x0100, CRC(59c8598e) SHA1(4ed31ec6b0b175bb109c0eb5fd3d193da823339f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v0", "DMG v0")
	ROMX_LOAD("dmg_v0.rom", 0x0000, 0x0100, CRC(c2f5cc97) SHA1(8bd501e31921e9601788316dbd3ce9833a97bcbc), ROM_BIOS(1))
ROM_END

ROM_START(supergb)
	ROM_REGION(0x0100, "maincpu", 0)
	ROM_LOAD("sgb_boot.bin", 0x0000, 0x0100, CRC(ec8a83b9) SHA1(aa2f50a77dfb4823da96ba99309085a3c6278515))
ROM_END

ROM_START(supergb2 )
	ROM_REGION(0x0100, "maincpu", 0)
	ROM_LOAD("sgb2_boot.bin", 0x0000, 0x0100, CRC(53d0dd63) SHA1(93407ea10d2f30ab96a314d8eca44fe160aea734))
ROM_END

ROM_START(gbpocket )
	ROM_REGION(0x0100, "maincpu", 0)
	ROM_LOAD("mgb_boot.bin", 0x0000, 0x0100, CRC(e6920754) SHA1(4e68f9da03c310e84c523654b9026e51f26ce7f0))
ROM_END

ROM_START(gbcolor)
	ROM_REGION(0x800, "maincpu", 0)
	ROM_LOAD("gbc_boot.1", 0x0000, 0x0100, CRC(779ea374) SHA1(e4b40c9fd593a97a1618cfb2696f290cf9596a62)) // Bootstrap code part 1
	ROM_LOAD("gbc_boot.2", 0x0100, 0x0700, CRC(f741807d) SHA1(f943b1e0b640cf1d371e1d8f0ada69af03ebb396)) // Bootstrap code part 2
ROM_END

ROM_START(megaduck)
ROM_END

ROM_START(gamefgtr)
	ROM_REGION(0x0100, "maincpu", 0)
	ROM_LOAD("gamefgtr.bin", 0x0000, 0x0100, CRC(908ba8de) SHA1(a4a36f71bf1b3b587df620d48ae940af93a982a5))
ROM_END

/*

Notes from Sean:

The bottom of the ROM PCB says
EW-012 4M Mask ROM
German Version
BB35-E012-0A12
REV.0

The bottom of the 7-pin glob-on-a-board says
EW-012
VOICE CHIP PCB
HT-81300 /
HT-81400
BB35-E012-0A09
REV.1

I couldn't find a data sheet, but I did see

"81300 is described as PCM speech synthesizer 5.6s" and
"81400 as PCM speech synthesizer 8.4s" (which is 50% larger)

I assume it that means it has (undumped) internal ROM.

*/

ROM_START(mduckspa)
	ROM_REGION(0x80000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("megaducksp.bin", 0x0000, 0x80000,  CRC(debd33fd) SHA1(fbf86dffa82f6e469da46623541f6f58f6c8a0d8) )

	ROM_REGION(0x10000, "81x00", ROMREGION_ERASEFF) // unknown size / capacity
	ROM_LOAD("81x00.bin", 0x0000, 0x10000, NO_DUMP )
ROM_END

} // anonymous namespace


//   YEAR  NAME      PARENT   COMPAT   MACHINE   INPUT     STATE           INIT        COMPANY     FULLNAME
CONS(1990, gameboy,  0,       0,       gameboy,  gameboy,  gb_state,       empty_init, "Nintendo", "Game Boy",         MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
CONS(1994, supergb,  gameboy, 0,       supergb,  gameboy,  sgb_state,      empty_init, "Nintendo", "Super Game Boy",   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
CONS(1998, supergb2, gameboy, 0,       supergb2, gameboy,  sgb_state,      empty_init, "Nintendo", "Super Game Boy 2", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
CONS(1996, gbpocket, gameboy, 0,       gbpocket, gameboy,  gb_state,       empty_init, "Nintendo", "Game Boy Pocket",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
CONS(1998, gbcolor,  0,       0,       gbcolor,  gameboy,  gbc_state,      empty_init, "Nintendo", "Game Boy Color",   MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)

// Sound is not 100% yet, it generates some sounds which could be OK. Since we're lacking a real system there's no way to verify.
CONS(1993, megaduck, 0,       0,       megaduck, megaduck, megaduck_state, empty_init, "Welback Holdings (Timlex International) / Creatronic / Videojet / Cougar USA", "Mega Duck / Cougar Boy", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

CONS(199?, mduckspa, 0,       0,       megaduck, megaduck, megaduck_state, empty_init, "Cefa Toys", "Super Quique / Mega Duck (Spain)", MACHINE_NOT_WORKING ) // versions for other regions exist too


// http://blog.gg8.se/wordpress/2012/11/11/gameboy-clone-game-fighter-teardown/
CONS(1993, gamefgtr, gameboy, 0,       gameboy,  gameboy,  gb_state,       empty_init, "bootleg", "Game Fighter (bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
