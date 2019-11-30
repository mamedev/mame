// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Bell-Fruit/ATD RasterSpeed hardware

    driver by Phil Bennett

    Games supported:
        * Rise of the Robots (prototype)

    ROMs wanted:
        * Zool (prototype)
        * Football Crazy (need HDD/CD image)

****************************************************************************/

#include "emu.h"
#include "bus/nscsi/hd.h"
#include "cpu/i386/i386.h"
#include "cpu/tms32031/tms32031.h"
#include "machine/53c7xx.h"
#include "machine/mc146818.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

/*************************************
 *
 *  Defines
 *
 *************************************/

#define SOUND_CLOCK             XTAL(12'288'000)
#define PLL_CLOCK               XTAL(14'318'181)
#define NVRAM_SIZE              0x8000

#define USE_SPEEDUP_HACK        1


/*************************************
 *
 *  Driver class
 *
 *************************************/

class rastersp_state : public driver_device
{
public:
	rastersp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp")
		, m_dram(*this, "dram")
		, m_ldac(*this, "ldac")
		, m_rdac(*this, "rdac")
		, m_palette(*this, "palette")
		, m_nvram(*this, "nvram")
		, m_tms_timer1(nullptr)
		, m_tms_tx_timer(nullptr)
	{
	}

	void rastersp(machine_config &config);

private:
	#define VIDEO_ADDR_MASK     0x3fffffff

	enum tms_regs
	{
		DMA_GLOBAL_CTL      =   0x00,
		DMA_SOURCE_ADDR     =   0x04,
		DMA_DEST_ADDR       =   0x06,
		DMA_TRANSFER_COUNT  =   0x08,

		TIMER0_GLOBAL_CTL   =   0x20,
		TIMER0_COUNTER      =   0x24,
		TIMER0_PERIOD       =   0x28,

		TIMER1_GLOBAL_CTL   =   0x30,
		TIMER1_COUNTER      =   0x34,
		TIMER1_PERIOD       =   0x38,

		SPORT_GLOBAL_CTL    =   0x40,
		SPORT_TX_CTL        =   0x42,
		SPORT_RX_CTL        =   0x43,
		SPORT_TIMER_CTL     =   0x44,
		SPORT_TIMER_COUNTER =   0x45,
		SPORT_TIMER_PERIOD  =   0x46,
		SPORT_DATA_TX       =   0x48,
		SPORT_DATA_RX       =   0x4c
	};

	enum irq_status
	{
		IRQ_RTC     = 1,
		IRQ_UART    = 2,
		IRQ_DSP     = 4,
		IRQ_VBLANK  = 5,
		IRQ_SCSI    = 7
	};

	required_device<i486_device>     m_maincpu;
	required_device<tms3203x_device> m_dsp;
	required_shared_ptr<uint32_t>    m_dram;
	required_device<dac_16bit_r2r_twos_complement_device> m_ldac;
	required_device<dac_16bit_r2r_twos_complement_device> m_rdac;
	required_device<palette_device>  m_palette;
	required_device<nvram_device>    m_nvram;

	emu_timer *m_tms_timer1;
	emu_timer *m_tms_tx_timer;

	DECLARE_WRITE32_MEMBER(cyrix_cache_w);
	DECLARE_READ8_MEMBER(nvram_r);
	DECLARE_WRITE8_MEMBER(nvram_w);
	DECLARE_WRITE32_MEMBER(port1_w);
	DECLARE_WRITE32_MEMBER(port2_w);
	DECLARE_WRITE32_MEMBER(port3_w);
	DECLARE_WRITE32_MEMBER(dpylist_w);
	DECLARE_READ32_MEMBER(tms32031_control_r);
	DECLARE_WRITE32_MEMBER(tms32031_control_w);
	DECLARE_WRITE32_MEMBER(dsp_unk_w);
	DECLARE_WRITE32_MEMBER(dsp_ctrl_w);
	DECLARE_WRITE32_MEMBER(dsp_486_int_w);
	DECLARE_READ32_MEMBER(dsp_speedup_r);
	DECLARE_WRITE32_MEMBER(dsp_speedup_w);
	DECLARE_READ32_MEMBER(ncr53c700_read);
	DECLARE_WRITE32_MEMBER(ncr53c700_write);
	DECLARE_WRITE_LINE_MEMBER(scsi_irq);

	TIMER_CALLBACK_MEMBER(tms_timer1);
	TIMER_CALLBACK_MEMBER(tms_tx_timer);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	std::unique_ptr<uint8_t[]>   m_nvram8;
	uint8_t   m_irq_status;
	uint32_t  m_dpyaddr;
	std::unique_ptr<uint16_t[]> m_paletteram;
	uint32_t  m_speedup_count;
	uint32_t  m_tms_io_regs[0x80];
	bitmap_ind16 m_update_bitmap;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_irq(uint32_t which, uint32_t state);
	void upload_palette(uint32_t word1, uint32_t word2);
	IRQ_CALLBACK_MEMBER(irq_callback);
	nscsi_connector &add_rastersp_scsi_slot(machine_config &config, const char *tag, const char *default_slot);
	static void ncr53c700_config(device_t *device);
	void cpu_map(address_map &map);
	void dsp_map(address_map &map);
	void io_map(address_map &map);

	// driver_device overrides
	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void video_start() override;
};



/*************************************
 *
 *  Initialisation
 *
 *************************************/

void rastersp_state::machine_start()
{
	m_nvram8 = std::make_unique<uint8_t[]>(NVRAM_SIZE);
	m_nvram->set_base(m_nvram8.get(),NVRAM_SIZE);
	m_paletteram = std::make_unique<uint16_t[]>(0x8000);

	membank("bank1")->set_base(m_dram);
	membank("bank2")->set_base(&m_dram[0x10000/4]);
	membank("bank3")->set_base(&m_dram[0x300000/4]);

	if (!m_tms_timer1)
		m_tms_timer1 = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(rastersp_state::tms_timer1), this));

	if (!m_tms_tx_timer)
		m_tms_tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(rastersp_state::tms_tx_timer), this));

#if USE_SPEEDUP_HACK
	m_dsp->space(AS_PROGRAM).install_read_handler(0x809923, 0x809923, read32_delegate(*this, FUNC(rastersp_state::dsp_speedup_r)));
	m_dsp->space(AS_PROGRAM).install_write_handler(0x809923, 0x809923, write32_delegate(*this, FUNC(rastersp_state::dsp_speedup_w)));
#endif
}


void rastersp_state::machine_reset()
{
	m_irq_status = 0;
	m_dpyaddr = 0;

	// Halt the 486 on reset - the DSP will release it from reset
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// Set IRQ1 line to cause DSP to boot from 0x400000
	m_dsp->set_input_line(TMS3203X_IRQ1, ASSERT_LINE);
	m_dsp->set_input_line(TMS3203X_IRQ1, CLEAR_LINE);

	// Reset DSP internal registers
	m_tms_io_regs[SPORT_GLOBAL_CTL] = 0;

	m_tms_timer1->adjust(attotime::never);
	m_tms_tx_timer->adjust(attotime::never);
}



/*************************************
 *
 *  Video hardware
 *
 *************************************/

void rastersp_state::video_start()
{
	m_update_bitmap.allocate(320, 240);
}


WRITE32_MEMBER( rastersp_state::dpylist_w )
{
	m_dpyaddr = data;

	// Update the video now
	// TODO: This should probably be done in sync with the video scan
	if (m_dpyaddr == 0)
	{
		m_update_bitmap.fill(m_palette->black_pen());
		return;
	}

	uint32_t dpladdr = (m_dpyaddr & ~0xff) >> 6;

	if ((m_dpyaddr & 0xff) != 0xb2 && (m_dpyaddr & 0xff) != 0xf2)
		logerror("Unusual display list data: %x\n", m_dpyaddr);

	int y = 0;
	int x = 0;
	uint16_t *bmpptr = &m_update_bitmap.pix16(0, 0);

	while (y < 240)
	{
		uint32_t word1 = m_dram[dpladdr/4];

		if (word1 & 0x80000000)
		{
			// TODO: What does this signify?
			dpladdr += 4;
		}
		else
		{
			uint32_t word2 = m_dram[(dpladdr + 4)/4];

			dpladdr += 8;

			if (word2 & 0x10000000)
			{
				if ((word2 & 0xfe000000) != 0x94000000)
					logerror("Unusual display list entry: %x %x\n", word1, word2);

				uint32_t srcaddr = word1 >> 8;
				uint32_t pixels = (word2 >> 16) & 0x1ff;
				uint32_t palbase = (word2 >> 4) & 0xf00;

				uint16_t* palptr = &m_paletteram[palbase];
				uint8_t* srcptr = reinterpret_cast<uint8_t*>(&m_dram[0]);

				uint32_t acc = srcaddr << 8;

				int32_t incr = word2 & 0xfff;

				// Sign extend for our convenience
				incr |= ~((incr & 0x800) - 1) & ~0xff;

				// TODO: Assumes 8-bit palettized mode - the hardware also supports 16-bit direct
				while (y < 240 && pixels)
				{
					while (x < 320 && pixels)
					{
						*bmpptr++ = palptr[srcptr[BYTE_XOR_LE(acc >> 8)]];
						acc = (acc + incr) & VIDEO_ADDR_MASK;

						--pixels;
						++x;
					}

					// Advance to the next scanline
					if (x >= 320)
					{
						x = 0;
						++y;
					}
				}
			}
			else
			{
				if ((word2 & 0x0c000000) != 0x0c000000)
					logerror("Unknown palette upload: %.8x %.8x\n", word1, word2);

				upload_palette(word1, word2);
			}
		}
	}
}


void rastersp_state::upload_palette(uint32_t word1, uint32_t word2)
{
	if (word1 & 3)
		fatalerror("Unalligned palette address! (%x, %x)\n", word1, word2);

	uint32_t addr = word1 >> 8;
	uint32_t entries = (word2 >> 16) & 0x1ff;
	uint32_t index = ((word2 >> 12) & 0x1f) * 256;

	// The third byte of each entry in RAM appears to contain an index
	// but appears to be discared when written to palette RAM
	while (entries--)
	{
		uint32_t data = m_dram[addr / 4];
		m_paletteram[index++] = data & 0xffff;
		addr += 4;
	}
}


/*******************************************************************************

    Display list register:

    ..xxxxxx xxxxxxxx xxxxxxxx ........  Display list base (DWORD granularity)
    ........ ........ ........ xxxxxxxx  ? (number of entries? Only seems to be valid for basic stuff)

    Display list format:

[0] ..xxxxxx xxxxxxxx xxxxxxxx ........  Source address (4MB mask?)

Palette update: (8d000400)
[1] 1....... ........ ........ ........  ?
    ...0.... ........ ........ ........  0 for palette upload?
    ....11.. ........ ........ ........  ?
    .......x xxxxxxxx ........ ........  Entry count
    ........ ........ .....1.. ........  ? (Usually 1)

Pixel data: (94040100)
[2] 1....... ........ ........ ........  ?
    ...1.... ........ ........ ........  1 for video update?
    .....1.. ........ ........ ........  ?
    .......x xxxxxxxx ........ ........  Pixel count
    ........ ........ xxxx.... ........  Palette
    ........ ........ ....xxxx xxxxxxxx  Scale (4.8 signed fixed point)

Unknown: (D4000100) - Present at start of a list
[3] 1....... ........ ........ .........
    .1...... ........ ........ ......... ?
    ..1..... ........ ........ .........
    .....1.. ........ ........ ......... ?
    ........ ........ .......1 ......... ?


    TODO: I'm not sure about bit 28. When creating the display list if it's 0,
    0x1000 is added to the source address.

*******************************************************************************/

uint32_t rastersp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_update_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

IRQ_CALLBACK_MEMBER(rastersp_state::irq_callback)
{
	uint8_t vector = 0;

	if (m_irq_status & (1 << IRQ_SCSI))
	{
		vector = 11;
	}
	else if (m_irq_status & (1 << IRQ_DSP))
	{
		update_irq(IRQ_DSP, CLEAR_LINE);
		vector = 12;
	}
	else if (m_irq_status & (1 << IRQ_VBLANK))
	{
		update_irq(IRQ_VBLANK, CLEAR_LINE);
		vector = 13;
	}
	else
	{
		fatalerror("Unknown x86 IRQ (m_irq_status = %x)", m_irq_status);
	}

	return vector;
}


void rastersp_state::update_irq(uint32_t which, uint32_t state)
{
	uint32_t mask = 1 << which;

	if (state)
		m_irq_status |= mask;
	else
		m_irq_status &= ~mask;

	m_maincpu->set_input_line(0, m_irq_status ? HOLD_LINE : CLEAR_LINE);
}


WRITE_LINE_MEMBER( rastersp_state::scsi_irq )
{
	update_irq(IRQ_SCSI, state);
}


WRITE_LINE_MEMBER( rastersp_state::vblank_irq )
{
	if (state)
		update_irq(IRQ_VBLANK, ASSERT_LINE);
}



/*************************************
 *
 *  486 I/O
 *
 *************************************/

WRITE32_MEMBER( rastersp_state::port1_w )
{
	// x... .... - LED?
	// ..x. .... - DSP IRQ2 line
	// .... ..xx - LEDs?

	if (data & 0x20)
	{
		m_dsp->set_input_line(TMS3203X_IRQ2, ASSERT_LINE);
		m_dsp->set_input_line(TMS3203X_IRQ2, CLEAR_LINE);
	}
}


WRITE32_MEMBER( rastersp_state::port2_w )
{
	// .x.. .... - X9313WP /INC
	// ..x. .... - X9313WP U/#D
}


WRITE32_MEMBER( rastersp_state:: port3_w )
{
	// xxxx xxxx - 8 LED cluster?
}



/*************************************
 *
 *  NVRAM
 *
 *************************************/

WRITE8_MEMBER( rastersp_state::nvram_w )
{
	offset *= 4;

	if ((offset & 0xc000) || !(offset & 0x2000))
	{
		logerror("Unmapped NVRAM write to offset: %x", offset);
	}

	offset &= ~0x2000;

	uint32_t addr = ((offset & 0x00f0000) >> 5) | ((offset & 0x1fff) / 4);

	m_nvram8[addr] = data & 0xff;
}


READ8_MEMBER( rastersp_state::nvram_r )
{
	offset *= 4;

	if ((offset & 0xc000) || !(offset & 0x2000))
	{
		logerror("Unmapped NVRAM read from offset: %x", offset);
	}

	offset &= ~0x2000;

	uint32_t addr = ((offset & 0x00f0000) >> 5) | ((offset & 0x1fff) / 4);

	return m_nvram8[addr];
}


WRITE32_MEMBER( rastersp_state::cyrix_cache_w )
{
	// TODO?
}



/*************************************
 *
 *  DSP
 *
 *************************************/

TIMER_CALLBACK_MEMBER(rastersp_state::tms_tx_timer)
{
	// Is the transmit shifter full?
	if (m_tms_io_regs[SPORT_GLOBAL_CTL] & (1 << 3))
	{
		uint32_t data = m_tms_io_regs[SPORT_DATA_TX];

		m_ldac->write(data & 0xffff);
		m_rdac->write(data >> 16);
	}

	// Set XSREMPTY
	m_tms_io_regs[SPORT_GLOBAL_CTL] &= ~(1 << 3);

	// Set XRDY
	m_tms_io_regs[SPORT_GLOBAL_CTL] |= (1 << 1);

	// Signal a transmit interrupt
	if (m_tms_io_regs[SPORT_GLOBAL_CTL] & (1 << 23))
	{
		m_dsp->set_input_line(TMS3203X_XINT0, ASSERT_LINE);
		m_dsp->set_input_line(TMS3203X_XINT0, CLEAR_LINE);
	}
}


TIMER_CALLBACK_MEMBER(rastersp_state::tms_timer1)
{
}


READ32_MEMBER( rastersp_state::tms32031_control_r )
{
	uint32_t val = m_tms_io_regs[offset];

	switch (offset)
	{
		case TIMER1_COUNTER:
		{
			attotime elapsed = m_tms_timer1->elapsed();
			val = m_tms_io_regs[TIMER1_PERIOD] - (elapsed.as_ticks(m_dsp->clock() / 2 / 2));

			break;
		}
		default:
		{
			logerror("TMS32031: Unhandled I/O read: %x\n", offset);
		}
	}

	return val;
}


WRITE32_MEMBER( rastersp_state::tms32031_control_w )
{
	uint32_t old = m_tms_io_regs[offset];

	m_tms_io_regs[offset] = data;

	switch (offset)
	{
		case TIMER1_GLOBAL_CTL:
		{
			// Calculate the DSP clocks
			attotime period = attotime::from_hz(m_dsp->clock() / 2 / 2);

			period *= m_tms_io_regs[TIMER1_PERIOD];

			m_tms_timer1->adjust(period, 0, period);

			break;
		}
		case SPORT_GLOBAL_CTL:
		{
			if (!(data & (1 << 26)))
			{
				// Reset transmitter
				m_tms_tx_timer->adjust(attotime::never);
			}
			else if (!(old & (1 << 26)) && (data & (1 << 26)))
			{
				// Sample rate is 24KHz
				attotime period = attotime::from_hz(SOUND_CLOCK / 512);

				// Set XRDY
				m_tms_io_regs[SPORT_GLOBAL_CTL] |= (1 << 1);

				// Set XSREMPTY
				m_tms_io_regs[SPORT_GLOBAL_CTL] &= ~(1 << 3);

				// Run transmitter
				m_tms_tx_timer->adjust(period, 0, period);
			}

			break;
		}
		case SPORT_DATA_TX:
		{
			// Clear XRDY
			m_tms_io_regs[SPORT_GLOBAL_CTL] &= ~(1 << 1);

			// Clear XSREMPTY
			m_tms_io_regs[SPORT_GLOBAL_CTL] |= (1 << 3);

			break;
		}
		default:
		{
			logerror("TMS32031: Unhandled I/O write: %x %x\n", offset, data);
		}
	}
}


WRITE32_MEMBER( rastersp_state::dsp_unk_w )
{
	// TODO: Looks like a debug port?
}


WRITE32_MEMBER( rastersp_state::dsp_486_int_w )
{
	update_irq(IRQ_DSP, ASSERT_LINE);
}


WRITE32_MEMBER( rastersp_state::dsp_ctrl_w )
{
	// x... .... LED?
	// .xx. .... 486 reset control?

	m_maincpu->set_input_line(INPUT_LINE_RESET, (data & 0x60) == 0x60 ? CLEAR_LINE : ASSERT_LINE);
}


WRITE32_MEMBER( rastersp_state::dsp_speedup_w )
{
	// 809e90  48fd, 48d5
	if (m_dsp->pc() == 0x809c23)
	{
		int32_t cycles_left = m_dsp->cycles_remaining();
		data += cycles_left / 6;
		m_dsp->spin();
	}

	m_speedup_count = data;
}


READ32_MEMBER( rastersp_state::dsp_speedup_r )
{
	return m_speedup_count;
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void rastersp_state::cpu_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram().share("dram");
	map(0x01000000, 0x010bffff).noprw(); // External ROM
	map(0x010c0000, 0x010cffff).rom().region("bios", 0);
	map(0x02200000, 0x022fffff).rw(FUNC(rastersp_state::nvram_r), FUNC(rastersp_state::nvram_w)).umask32(0x000000ff);
	map(0x02200800, 0x02200803).nopw(); // ?
	map(0x02208000, 0x02208fff).rw("scsibus:7:ncr53c700", FUNC(ncr53c7xx_device::read), FUNC(ncr53c7xx_device::write));
	map(0x0220e000, 0x0220e003).w(FUNC(rastersp_state::dpylist_w));
	map(0xfff00000, 0xffffffff).bankrw("bank3");
}

void rastersp_state::io_map(address_map &map)
{
	map(0x0020, 0x0023).w(FUNC(rastersp_state::cyrix_cache_w));
	map(0x1000, 0x1003).portr("P1").w(FUNC(rastersp_state::port1_w));
	map(0x1004, 0x1007).portr("P2").w(FUNC(rastersp_state::port2_w));
	map(0x1008, 0x100b).portr("COMMON").w(FUNC(rastersp_state::port3_w));
	map(0x100c, 0x100f).portr("DSW2");
	map(0x1010, 0x1013).portr("DSW1");
	map(0x1014, 0x1017).portr("EXTRA");
	map(0x4000, 0x4007).rw("rtc", FUNC(mc146818_device::read), FUNC(mc146818_device::write)).umask32(0x000000ff);
	map(0x6008, 0x600b).nopr().nopw(); // RS232
}


void rastersp_state::dsp_map(address_map &map)
{
	map(0x000000, 0x0fffff).bankrw("bank1");
	map(0x400000, 0x40ffff).rom().region("dspboot", 0);
	map(0x808000, 0x80807f).rw(FUNC(rastersp_state::tms32031_control_r), FUNC(rastersp_state::tms32031_control_w));
	map(0x880402, 0x880402).w(FUNC(rastersp_state::dsp_unk_w));
	map(0x883c00, 0x883c00).w(FUNC(rastersp_state::dsp_486_int_w));
	map(0xc00000, 0xc03fff).bankrw("bank2");
	map(0xc80000, 0xc80000).w(FUNC(rastersp_state::dsp_ctrl_w));
	map(0xfc0000, 0xffffff).bankrw("bank3");
}



/*************************************
 *
 *  Inputs
 *
 *************************************/

static INPUT_PORTS_START( rotr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("COMMON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Setup Disk" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )

	PORT_DIPNAME( 0x3c, 0x00, "Load PROG" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x14, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPSETTING(    0x1c, "7" )
	PORT_DIPSETTING(    0x20, "8" )
	PORT_DIPSETTING(    0x24, "9" )
	PORT_DIPSETTING(    0x28, "A" )
	PORT_DIPSETTING(    0x2c, "B" )
	PORT_DIPSETTING(    0x30, "C" )
	PORT_DIPSETTING(    0x34, "D" )
	PORT_DIPSETTING(    0x38, "E" )
	PORT_DIPSETTING(    0x3c, "F" )

	PORT_DIPNAME( 0x40, 0x00, "Reserved" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )

	PORT_DIPNAME( 0x80, 0x00, "Enable Cache" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Debug Screen" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_DIPNAME( 0x0e, 0x00, "FPGA File Source" )
	PORT_DIPSETTING(    0x00, "Serial PROMs" )
	PORT_DIPSETTING(    0x0e, "Cable" )

	PORT_DIPNAME( 0x70, 0x50, "Clock speed" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x20, "16" )
	PORT_DIPSETTING(    0x30, "20" )
	PORT_DIPSETTING(    0x40, "25" )
	PORT_DIPSETTING(    0x50, "33" )
	PORT_DIPSETTING(    0x60, "40" )
	PORT_DIPSETTING(    0x70, "50" )

	PORT_DIPNAME( 0x80, 0x00, "SCSI bus terminated" )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
INPUT_PORTS_END



/*************************************
 *
 *  SCSI
 *
 *************************************/

READ32_MEMBER(rastersp_state::ncr53c700_read)
{
	return m_maincpu->space(AS_PROGRAM).read_dword(offset, mem_mask);
}

WRITE32_MEMBER(rastersp_state::ncr53c700_write)
{
	m_maincpu->space(AS_PROGRAM).write_dword(offset, data, mem_mask);
}

void rastersp_state::ncr53c700_config(device_t *device)
{
	auto *state = device->subdevice<rastersp_state>(":");
	ncr53c7xx_device &scsictrl = downcast<ncr53c7xx_device &>(*device);
	scsictrl.set_clock(66000000);
	scsictrl.irq_handler().set(*state, FUNC(rastersp_state::scsi_irq));
	scsictrl.host_read().set(*state, FUNC(rastersp_state::ncr53c700_read));
	scsictrl.host_write().set(*state, FUNC(rastersp_state::ncr53c700_write));
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void rastersp_state::rastersp(machine_config &config)
{
	I486(config, m_maincpu, 33330000);
	m_maincpu->set_addrmap(AS_PROGRAM, &rastersp_state::cpu_map);
	m_maincpu->set_addrmap(AS_IO, &rastersp_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(rastersp_state::irq_callback));

	TMS32031(config, m_dsp, 33330000);
	m_dsp->set_addrmap(AS_PROGRAM, &rastersp_state::dsp_map);
	m_dsp->set_mcbl_mode(true); // Boot-loader mode

	MC146818(config, "rtc", 32.768_kHz_XTAL);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	NSCSI_BUS(config, "scsibus", 0);

	nscsi_connector &connector0(NSCSI_CONNECTOR(config, "scsibus:0", 0));
	connector0.option_add("harddisk", NSCSI_HARDDISK);
	connector0.option_add_internal("ncr53c700", NCR53C7XX);
	connector0.set_default_option("harddisk");
	connector0.set_fixed(true);

	nscsi_connector &connector7(NSCSI_CONNECTOR(config, "scsibus:7", 0));
	connector7.option_add("harddisk", NSCSI_HARDDISK);
	connector7.option_add_internal("ncr53c700", NCR53C7XX);
	connector7.set_default_option("ncr53c700");
	connector7.set_fixed(true);
	connector7.set_option_machine_config("ncr53c700", ncr53c700_config);

	/* Video */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(320, 240);
	screen.set_visarea(0, 320-1, 0, 240-1);
	screen.set_screen_update(FUNC(rastersp_state::screen_update));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(rastersp_state::vblank_irq));

	PALETTE(config, m_palette, palette_device::RGB_565);

	/* Sound */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_ldac, 0);
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_rdac, 0);
	m_ldac->add_route(ALL_OUTPUTS, "lspeaker", 0.5); // unknown DAC
	m_rdac->add_route(ALL_OUTPUTS, "rspeaker", 0.5); // unknown DAC

	voltage_regulator_device &vreg(VOLTAGE_REGULATOR(config, "vref"));
	vreg.add_route(0, "ldac",  1.0, DAC_VREF_POS_INPUT);
	vreg.add_route(0, "rdac",  1.0, DAC_VREF_POS_INPUT);
	vreg.add_route(0, "ldac", -1.0, DAC_VREF_NEG_INPUT);
	vreg.add_route(0, "rdac", -1.0, DAC_VREF_NEG_INPUT);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( rotr )
	ROM_REGION32_LE(0x100000, "bios", 0)
	ROM_LOAD( "rasterspeed2.1_bootrom4.u10", 0x00000, 0x10000, CRC(6da142d1) SHA1(e2dbd479034677726fc26fd1ba85c4458d89286c) )

	ROM_REGION32_LE(0x1000000, "dspboot", 0)
	ROM_LOAD32_BYTE( "rasterspeed2.1_bootrom4.u10", 0x00000, 0x10000, CRC(6da142d1) SHA1(e2dbd479034677726fc26fd1ba85c4458d89286c) )

	ROM_REGION(0x8000, "proms", 0) /* Xilinx FPGA PROMs */
	ROM_LOAD( "17128dpc.u72", 0x0000, 0x4000, CRC(5ddf6ee3) SHA1(f3d15b649b5641374a9e14877cea84ba9c57ef3c) )
	ROM_LOAD( "17128dpc.u73", 0x2000, 0x4000, CRC(9e274cea) SHA1(e974cad4e4b965bf2c9df7d3d0b4eec64629eeb0) )

	ROM_REGION(0x8000, "nvram", 0) /* Default NVRAM */
	ROM_LOAD( "rotr.nv", 0x0000, 0x8000, CRC(62543517) SHA1(a4bf3431cdab956839bb155c4a8c140d30e5c7ec) )

	DISK_REGION( "scsibus:0:harddisk:image" )
	DISK_IMAGE( "rotr", 0, SHA1(d67d7feb52d8c7ba1d2a190a40d97e84871f2d80) )
ROM_END


ROM_START( rotra )
	ROM_REGION32_LE(0x100000, "bios", 0)
	ROM_LOAD( "rasterspeed2.1_bootrom4.u10", 0x00000, 0x10000, CRC(6da142d1) SHA1(e2dbd479034677726fc26fd1ba85c4458d89286c) )

	ROM_REGION32_LE(0x1000000, "dspboot", 0)
	ROM_LOAD32_BYTE( "rasterspeed2.1_bootrom4.u10", 0x00000, 0x10000, CRC(6da142d1) SHA1(e2dbd479034677726fc26fd1ba85c4458d89286c) )

	ROM_REGION(0x8000, "proms", 0) /* Xilinx FPGA PROMs */
	ROM_LOAD( "17128dpc.u72", 0x0000, 0x4000, CRC(5ddf6ee3) SHA1(f3d15b649b5641374a9e14877cea84ba9c57ef3c) )
	ROM_LOAD( "17128dpc.u73", 0x2000, 0x4000, CRC(9e274cea) SHA1(e974cad4e4b965bf2c9df7d3d0b4eec64629eeb0) )

	ROM_REGION(0x8000, "nvram", 0) /* Default NVRAM */
	ROM_LOAD( "rotr.nv", 0x0000, 0x8000, CRC(62543517) SHA1(a4bf3431cdab956839bb155c4a8c140d30e5c7ec) )

	DISK_REGION( "scsibus:0:harddisk:image" )
	DISK_IMAGE( "rotra", 0, SHA1(570d402e5e9bba123edf7dfa9db7a0e6bdb23823) )
ROM_END

/*
  Football Crazy runs on the (c)1997 "RASTERSPEED 2.1 31-599-001 ISS 4" PCB, which seems to be a more modern production version.
  a PCB photo with the rom sticker showing the text below has also been seen

  Football Crazy Cashflow
  95 750 956
  STANDARD UK 64K
  VER. FOOT 3.2 BFM



*/

// the rom also exists in some odd hex format like this
// ROM_LOAD( "95751937.hex", 0x0000, 0x025f91, CRC(8f412e97) SHA1(a5ff924fbc327114e59d75de644ed0d5cd7fa6b3) )
ROM_START( fbcrazy )
	ROM_REGION32_LE(0x100000, "bios", 0)
	ROM_LOAD( "95751937.bin", 0x0000, 0x010000, CRC(4a99ee11) SHA1(335398ebc64bbfe86e2652ac080a5943dd413928) )

	ROM_REGION32_LE(0x1000000, "dspboot", 0)
	ROM_LOAD32_BYTE( "95751937.bin", 0x0000, 0x010000, CRC(4a99ee11) SHA1(335398ebc64bbfe86e2652ac080a5943dd413928) )

	ROM_REGION(0x8000, "proms", ROMREGION_ERASEFF )
	/* not on this PCB type? */

	ROM_REGION(0x8000, "nvram", ROMREGION_ERASEFF )

	DISK_REGION( "scsibus:0:harddisk:image" )
	DISK_IMAGE( "fbcrazy_hdd", 0, NO_DUMP )
ROM_END

/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1994, rotr,    0,    rastersp, rotr, rastersp_state, empty_init, ROT0, "BFM/Mirage", "Rise of the Robots (prototype)",        0 )
GAME( 1994, rotra,   rotr, rastersp, rotr, rastersp_state, empty_init, ROT0, "BFM/Mirage", "Rise of the Robots (prototype, older)", 0 )
GAME( 1997, fbcrazy, 0,    rastersp, rotr, rastersp_state, empty_init, ROT0, "BFM",        "Football Crazy (Video Quiz)",           MACHINE_NOT_WORKING )
