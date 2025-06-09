// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Bell-Fruit/ATD RasterSpeed hardware

    driver by Phil Bennett

    Games supported:
        * Rise of the Robots (prototype)
        * Football Crazy

    ROMs wanted:
        * Zool (prototype)
        * Wiggle

****************************************************************************/

#include "emu.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "cpu/i386/i386.h"
#include "cpu/tms32031/tms32031.h"
#include "machine/53c7xx.h"
#include "machine/bacta_datalogger.h"
#include "machine/mc146818.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "machine/z80scc.h"
#include "sound/dac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

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
		, m_duart(*this, "duart")
		, m_dram(*this, "dram")
		, m_ldac(*this, "ldac")
		, m_rdac(*this, "rdac")
		, m_palette(*this, "palette")
		, m_nvram(*this, "nvram")
		, m_watchdog(*this, "watchdog")
		, m_tms_timer1(nullptr)
		, m_tms_tx_timer(nullptr)
	{
	}

	void rastersp(machine_config &config);
	void rs_config_base(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint8_t interrupt_ctrl_r(offs_t offset);
	void interrupt_ctrl_w(offs_t offset, uint8_t data);
	uint8_t nvram_r(offs_t offset);
	void nvram_w(offs_t offset, uint8_t data);

	void cpu_map_base(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void dsp_map_base(address_map &map) ATTR_COLD;

	required_device<i486_device>     m_maincpu;
	required_device<tms3203x_device> m_dsp;
	required_device<z80scc_device>   m_duart;

	void update_irq(uint32_t which, uint32_t state);

	enum irq_status
	{
		IRQ_RTC     = 1,
		IRQ_UART    = 2,
		IRQ_SCSI    = 3,
		IRQ_DSP     = 4,
		IRQ_VBLANK  = 5,
	};

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

	required_shared_ptr<uint32_t>    m_dram;
	required_device<dac_16bit_r2r_twos_complement_device> m_ldac;
	required_device<dac_16bit_r2r_twos_complement_device> m_rdac;
	required_device<palette_device>  m_palette;
	required_device<nvram_device>    m_nvram;
	required_device<watchdog_timer_device> m_watchdog;

	emu_timer *m_tms_timer1;
	emu_timer *m_tms_tx_timer;

	void cyrix_cache_w(uint32_t data);
	void port1_w(uint32_t data);
	void port2_w(uint32_t data);
	void port3_w(uint32_t data);
	void dpylist_w(uint32_t data);
	uint32_t tms32031_control_r(offs_t offset);
	void tms32031_control_w(offs_t offset, uint32_t data);
	void dsp_unk_w(uint32_t data);
	void dsp_ctrl_w(uint32_t data);
	void dsp_486_int_w(uint32_t data);
	uint32_t dsp_speedup_r();
	void dsp_speedup_w(uint32_t data);
	uint32_t ncr53c700_read(offs_t offset, uint32_t mem_mask = ~0);
	void ncr53c700_write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void scsi_irq(int state);

	TIMER_CALLBACK_MEMBER(tms_timer1);
	TIMER_CALLBACK_MEMBER(tms_tx_timer);
	void vblank_irq(int state);
	void duart_irq(int state);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void upload_palette(uint32_t word1, uint32_t word2);
	IRQ_CALLBACK_MEMBER(irq_callback);
	nscsi_connector &add_rastersp_scsi_slot(machine_config &config, const char *tag, const char *default_slot);
	static void ncr53c700_config(device_t *device);
	void cpu_map(address_map &map) ATTR_COLD;
	void dsp_map(address_map &map) ATTR_COLD;
	uint8_t interrupt_status_r(offs_t offset);

	std::unique_ptr<uint8_t[]>   m_nvram8;
	uint8_t   m_irq_status = 0;
	uint32_t  m_dlba = 0;
	std::unique_ptr<uint16_t[]> m_paletteram;
	uint8_t m_palette_number = 0;
	uint32_t  m_speedup_count = 0;
	uint32_t  m_tms_io_regs[0x80]{};
	bitmap_ind16 m_update_bitmap;

	uint8_t m_port2_data = 0;
	int m_left_volume = 0;
	int m_right_volume = 0;
	uint8_t m_interrupt_mask = 0;
	uint8_t m_dsp_ctrl_data = 0;
};

class fbcrazy_state : public rastersp_state
{
public:
	fbcrazy_state(const machine_config &mconfig, device_type type, const char *tag)
		: rastersp_state(mconfig, type, tag)
		, m_io_track_x(*this, "TRACK_X")
		, m_io_track_y(*this, "TRACK_Y")
		, m_trackball_timer(nullptr)
	{
	}

	void fbcrazy(machine_config &config);

	int meter_pulse_r();

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	required_ioport m_io_track_x;
	required_ioport m_io_track_y;
	emu_timer *m_trackball_timer;

	void aux_port0_w(offs_t offset, uint8_t data);
	void aux_port1_w(offs_t offset, uint8_t data);
	void aux_port2_w(offs_t offset, uint8_t data);
	void aux_port3_w(offs_t offset, uint8_t data);
	void aux_port4_w(offs_t offset, uint8_t data);

	void cpu_map(address_map &map) ATTR_COLD;
	void dsp_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(trackball_timer);
	void trackball_rts(int state);

	uint8_t m_aux_port3_data;
	uint8_t m_trackball_ctr;
	uint8_t m_trackball_data[3];
	uint16_t m_oldxpos;
	uint16_t m_oldypos;
	uint8_t m_trackball_enabled;
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
	//membank("bank3")->set_base(&m_dram[0x300000/4]);

	if (!m_tms_timer1)
		m_tms_timer1 = timer_alloc(FUNC(rastersp_state::tms_timer1), this);

	if (!m_tms_tx_timer)
		m_tms_tx_timer = timer_alloc(FUNC(rastersp_state::tms_tx_timer), this);

#if USE_SPEEDUP_HACK
	m_dsp->space(AS_PROGRAM).install_read_handler(0x809923, 0x809923, read32smo_delegate(*this, FUNC(rastersp_state::dsp_speedup_r)));
	m_dsp->space(AS_PROGRAM).install_write_handler(0x809923, 0x809923, write32smo_delegate(*this, FUNC(rastersp_state::dsp_speedup_w)));
#endif

	save_item(NAME(m_irq_status));
	save_item(NAME(m_dlba));
	save_pointer(NAME(m_paletteram), 0x8000);
	save_item(NAME(m_palette_number));
	save_item(NAME(m_speedup_count));
	save_item(NAME(m_tms_io_regs));
	save_item(NAME(m_port2_data));
	save_item(NAME(m_left_volume));
	save_item(NAME(m_right_volume));
	save_item(NAME(m_interrupt_mask));
	save_item(NAME(m_dsp_ctrl_data));
}

void fbcrazy_state::machine_start()
{
	rastersp_state::machine_start();

	m_trackball_timer = timer_alloc(FUNC(fbcrazy_state::trackball_timer), this);

	save_item(NAME(m_aux_port3_data));
	save_item(NAME(m_trackball_ctr));
	save_item(NAME(m_trackball_data));
	save_item(NAME(m_oldxpos));
	save_item(NAME(m_oldypos));
	save_item(NAME(m_trackball_enabled));
}

void rastersp_state::machine_reset()
{
	m_irq_status = 0;
	m_dlba = 0;
	m_palette_number = 0;

	m_port2_data = 0;
	m_left_volume = 0;
	m_right_volume = 0;
	m_interrupt_mask = 0;

	m_dsp_ctrl_data = 0;


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

void fbcrazy_state::machine_reset()
{
	attotime period;

	rastersp_state::machine_reset();

	m_aux_port3_data = 0;

	m_trackball_ctr = 0;
	m_trackball_data[0] = m_trackball_data[1] = m_trackball_data[2] = 0;
	m_oldxpos = 0;
	m_oldypos = 0;
	m_trackball_enabled = 0;

	period = attotime::from_hz(1200); // 1200 baud
	m_trackball_timer->adjust(period, 0, period);
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


void rastersp_state::dpylist_w(uint32_t data)
{
	uint32_t word1 = 0;
	m_dlba = data;

	// Update the video now
	// TODO: This should probably be done in sync with the video scan
	if (!BIT(m_dlba, 7))
	{
		m_update_bitmap.fill(m_palette->black_pen());
		return;
	}

	uint32_t dpladdr = (m_dlba & ~0xff) >> 6;

	int y = 0;
	int x = 0;
	uint16_t *bmpptr = &m_update_bitmap.pix(0, 0);

	// Note that you can have a control word (top bit setp) without an address if the control word doesn't need data
	// You can also have a single address word which is used by multiple control words.
	while (y < 240)
	{
		uint32_t word2 = m_dram[dpladdr/4];
		dpladdr += 4;

		if (!(word2 & 0x80000000))
		{
			// Not a control word so it's the data address
			word1 = word2;
		}
		else
		{
			// You're allowed to change the palette for 8 bit pixels even if the
			// control word is for 16 bit pixels!
			if (word2 & 0x40000000)
			{
				m_palette_number &= 0x0f;
				m_palette_number |= ((word2 >> 8) & 0xf0);
			}
			else
			{
				m_palette_number &= 0xf0;
				m_palette_number |= ((word2 >> 12) & 0x0f);
			}

			if ((word2 & 0xbe000000) == 0x94000000)
			{
				// 8 bit pixels using palette
				uint32_t srcaddr = word1 >> 8;
				uint32_t pixels = (word2 >> 16) & 0x1ff;

				uint16_t* palptr = &m_paletteram[m_palette_number*256];
				uint8_t* srcptr = reinterpret_cast<uint8_t*>(&m_dram[0]);

				uint32_t acc = srcaddr << 8;

				int32_t incr = word2 & 0xfff;

				// Sign extend for our convenience
				incr |= ~((incr & 0x800) - 1) & ~0xff;

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
			else if ((word2 & 0xbe000000) == 0x84000000)
			{
				// 16-bit pixels
				uint32_t srcaddr = word1 >> 8;
				uint32_t pixels = (word2 >> 16) & 0x1ff;

				uint16_t* srcptr = reinterpret_cast<uint16_t*>(&m_dram[0]);

				uint32_t acc = srcaddr << 8;

				int32_t incr = word2 & 0xfff;

				// Sign extend for our convenience
				incr |= ~((incr & 0x800) - 1) & ~0xff;

				while (y < 240 && pixels)
				{
					while (x < 320 && pixels)
					{
						*bmpptr++ = srcptr[WORD_XOR_LE(acc >> 9)];
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
			else if ((word2 & 0xbe000000) == 0x8c000000)
			{
				upload_palette(word1, word2);
			}
			else
			{
				fatalerror("Unknown control words! (%x, %x)\n", word1, word2);
			}
		}
	}
}


void rastersp_state::upload_palette(uint32_t word1, uint32_t word2)
{
	/*
	    Palette entry format:
	    ........ ........ ........ ...xxxxx - Blue
	    ........ ........ .....xxx xxx..... - Green
	    ........ ........ xxxxx... ........ - Red
	    .......x xxxxxxxx ........ ........ - Entry number
	*/
	if (word1 & 3)
		fatalerror("Unalligned palette address! (%x, %x)\n", word1, word2);

	uint32_t addr = word1 >> 8;
	uint32_t entries = (word2 >> 16) & 0x1ff;
	uint32_t index = m_palette_number * 256;

	while (entries--)
	{
		uint32_t data = m_dram[addr / 4];
		m_paletteram[index + ((data >> 16) & 0xff)] = data & 0xffff;
		addr += 4;
	}
}


/*******************************************************************************

    Display pointer word:

    xxxxxxxx xxxxxxxx xxxxxxxx ........ DLBA pointer (DWORD aligned)
    ........ ........ ........ x....... Display enable
    ........ ........ ........ ......xx Mode:
                                       00: 384x288 50Hz overscan
                                       01: 320x240 64Hz overscan
                                       02: 320x240 50Hz underscan
                                       03: invalid

    Control words:

    Set address
    0.xxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  Address

    Palette update:
    1P0..... ........ ........ ........  P specifies whether palette entry is the bottom (when 0) or top (when 1) 4 bits of palette number
    ...01... ........ ........ ........  Palette upload
    .....10. ........ ........ ........  X address only
    .......x xxxxxxxx ........ ........  Entry count
    ........ ........ xxxx.... ........  Palette (top 4 bits if 'P' is 1 or bottom 4 bits if 'P' is 0
    ........ ........ ....xxxx xxxxxxxx  Scale (4.8 signed fixed point)

    Pixel data, 8-bit palettized:
    1P0..... ........ ........ ........  P specifies whether palette entry is the bottom (when 0) or top (when 1) 4 bits of palette number
    ...10... ........ ........ ........  8 bit pixels with palette
    .....10. ........ ........ ........  X address only
    .......x xxxxxxxx ........ ........  Pixel count
    ........ ........ xxxx.... ........  Palette (top 4 bits if 'P' is 1 or bottom 4 bits if 'P' is 0)
    ........ ........ ....xxxx xxxxxxxx  Scale (4.8 signed fixed point)

    Pixel data, 16-bit:
    1P0..... ........ ........ ........  P specifies whether palette entry is the bottom (when 0) or top (when 1) 4 bits of palette number
    ...00... ........ ........ ........  8 bit pixels with palette
    .....10. ........ ........ ........  X address only
    .......x xxxxxxxx ........ ........  Pixel count
    ........ ........ xxxx.... ........  Unused
    ........ ........ ....xxxx xxxxxxxx  Scale (4.8 signed fixed point)

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

	if (m_irq_status & (1 << IRQ_UART))
	{
		vector = 10;
	}
	else if (m_irq_status & (1 << IRQ_SCSI))
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


void rastersp_state::scsi_irq(int state)
{
	update_irq(IRQ_SCSI, state);

	if (state && BIT(m_dsp_ctrl_data, 5))
	{
		m_dsp->set_input_line(TMS3203X_IRQ0, ASSERT_LINE);
		m_dsp->set_input_line(TMS3203X_IRQ0, CLEAR_LINE);
	}
}


void rastersp_state::vblank_irq(int state)
{
	if (state)
		update_irq(IRQ_VBLANK, ASSERT_LINE);
}

void rastersp_state::duart_irq(int state)
{
	update_irq(IRQ_UART, state);
}


void fbcrazy_state::trackball_rts(int state)
{
	m_trackball_enabled = state;
}

/*
    Mimic trackball unit. This uses a small PIC based interface board
    which converts the trackball movement into RS232 data.
    The data format is as follows:
    Byte 0 : 0100abAB bits 7 & 6 = 01 signifies 1st byte
    Byte 1 : 00CDEFGH bits 7 & 6 = 00 signifies 2 or 3rd byte (not identified)
    Byte 2 : 00cdefgh

    The trackball X movement is ABCDEFGH and Y movement abcdefgh
    We do not have a PCB nor a dump of the PIC :(
*/
TIMER_CALLBACK_MEMBER(fbcrazy_state::trackball_timer)
{
	if (m_trackball_enabled)
	{
		if (m_trackball_ctr == 0)
		{
			uint16_t newxpos, newypos;

			newxpos = m_io_track_x->read();
			newypos = m_io_track_y->read();

			int diffx, diffy;
			diffx = newxpos - m_oldxpos;
			diffy = newypos - m_oldypos;

			if (diffx<-3900)
			{
				diffx += 4096;
			}
			else if (diffx>3900)
			{
				diffx -= 4096;
			}
			if (diffx>127)
			{
				diffx = 127;
			}
			else if (diffx <-127)
			{
				diffx = -127;
			}

			if (diffy<-3900)
			{
				diffy += 4096;
			}
			else if (diffy>3900)
			{
				diffy -= 4096;
			}
			if (diffy>127)
			{
				diffy = 127;
			}
			else if (diffy <-127)
			{
				diffy = -127;
			}

			m_oldxpos = newxpos;
			m_oldypos = newypos;

			//Format movement into 3 bytes as described above for tx
			m_trackball_data[0] = 0x40 | ((diffy>>4)&0x0c) | ((diffx>>6)&0x03);
			m_trackball_data[1] = diffx & 0x3f;
			m_trackball_data[2] = diffy & 0x3f;
		}

		if (m_trackball_ctr < 36)
		{
			//Transmitting data
			uint8_t bitnum = m_trackball_ctr % 12 ;
			if (bitnum == 0)
			{
				//start bit
				m_duart->rxb_w(0);
			}
			else if (bitnum < 9)
			{
				//Data bits
				m_duart->rxb_w(BIT(m_trackball_data[m_trackball_ctr / 12], bitnum - 1));
			}
			else
			{
				//Stop bit and small gap
				m_duart->rxb_w(1);
			}
			m_trackball_ctr++;
		}
		else
		{
			m_trackball_ctr = 0;
		}
	}
	else
	{
		m_trackball_ctr = 0;
		m_trackball_data[0] = 0;
		m_trackball_data[1] = 0;
		m_trackball_data[2] = 0;
		m_duart->rxb_w(1);
	}
}

/*************************************
 *
 *  486 I/O
 *
 *************************************/

void rastersp_state::port1_w(uint32_t data)
{
	/*
	  x... .... - Watchdog kick
	  ..x. .... - DSP IRQ2 line
	  .... x... - Coin lockout A on ROTR and payout on Football Crazy
	  .... .x.. - Coin lockout B
	  .... ..x. - Coin meter A
	  .... ...x - Coin meter B
	*/
	if (BIT(data, 5) && BIT(m_dsp_ctrl_data, 5))
	{
		m_dsp->set_input_line(TMS3203X_IRQ2, ASSERT_LINE);
		m_dsp->set_input_line(TMS3203X_IRQ2, CLEAR_LINE);
	}

	m_watchdog->reset_line_w(BIT(data, 7));
}


void rastersp_state::port2_w(uint32_t data)
{
	/*
	  .x.. .... - X9313WP /INC
	  ..x. .... - X9313WP U/#D
	  ...x .... - Left audio volume CS (X9313)
	  .... x... - Right audio volume CS (X9313)
	  .... .xxx - Audio bitrate select
	*/
	uint8_t changed = m_port2_data ^ data;

	m_port2_data = data;

	if (changed & 0x40)
	{
		// Digital volume clock line changed
		if (!(data & 0x40))
		{
			if (data & 0x20)
			{
				if (!(data & 0x10) && (m_left_volume < 31)) m_left_volume++;
				if (!(data & 0x08) && (m_right_volume < 31)) m_right_volume++;
			}
			else
			{
				if (!(data & 0x10) && (m_left_volume > 0)) m_left_volume--;
				if (!(data & 0x08) && (m_right_volume > 0)) m_right_volume--;
			}

			m_ldac->set_output_gain(ALL_OUTPUTS, m_left_volume / 32.0f);
			m_rdac->set_output_gain(ALL_OUTPUTS, m_right_volume / 32.0f);
		}
	}

}

void rastersp_state:: port3_w(uint32_t data)
{
	// xxxx xxxx - 8 LED cluster?
}



/*************************************
 *
 *  NVRAM
 *
 *************************************/

void rastersp_state::nvram_w(offs_t offset, uint8_t data)
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


uint8_t rastersp_state::nvram_r(offs_t offset)
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

void fbcrazy_state::aux_port0_w(offs_t offset, uint8_t data)
{
	// Lamps 1-8 on bits 0-7
	// Bit 6, lamp 6, is Start lamp
}

void fbcrazy_state::aux_port1_w(offs_t offset, uint8_t data)
{
	// Lamps 9-16 on bits 0-7
	// Bit 3, lamp 12, is SELECT button lamp
}

void fbcrazy_state::aux_port2_w(offs_t offset, uint8_t data)
{
	// Lamps 17-22 on bits 0-5
	// Bit 6 Coin Validator lamp ?
	// Bit 7 unused
}

void fbcrazy_state::aux_port3_w(offs_t offset, uint8_t data)
{
	m_aux_port3_data = data;
	// Bit 0 - unused
	// Bit 1 - unused
	// Bit 2 - Meter 3
	// Bit 3 - Meter 1
	// Bit 4 - Meter 2
	// Bit 5 - unused
	// Bit 6 - unused
	// Bit 7 - unused
}

void fbcrazy_state::aux_port4_w(offs_t offset, uint8_t data)
{
	// Bit 0 - unused
	// Bit 1 - unused
	// Bit 2 - unused
	// Bit 3 - 10p lockout
	// Bit 4 - 20p lockout
	// Bit 5 - 50p lockout
	// Bit 6 - £1 lockout
	// Bit 7 - 5p lockout
}

int fbcrazy_state::meter_pulse_r()
{
	return m_aux_port3_data & 0xbc ? 1 : 0;
}



void rastersp_state::cyrix_cache_w(uint32_t data)
{
	// TODO?
}

uint8_t rastersp_state::interrupt_ctrl_r(offs_t offset)
{
	return m_interrupt_mask;
}

void rastersp_state::interrupt_ctrl_w(offs_t offset, uint8_t data)
{
	m_interrupt_mask = data;
}

uint8_t rastersp_state::interrupt_status_r(offs_t offset)
{
	// TODO: This returns something to do with interrupts
	// Is it a bit pattern of the current interrupt or a
	// bit pattern of all interrupts pending?
	// Either way, 0 works anyway!
	return 0;
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


uint32_t rastersp_state::tms32031_control_r(offs_t offset)
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


void rastersp_state::tms32031_control_w(offs_t offset, uint32_t data)
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


void rastersp_state::dsp_unk_w(uint32_t data)
{
	// TODO: Looks like a debug port?
}


void rastersp_state::dsp_486_int_w(uint32_t data)
{
	update_irq(IRQ_DSP, ASSERT_LINE);
}


void rastersp_state::dsp_ctrl_w(uint32_t data)
{
	// x... .... - Something to do with 486 bus access?
	// .x.. .... - 486 reset control
	// ..x. .... - Enable SCSI, UART, and 486 int to DSP

	m_maincpu->set_input_line(INPUT_LINE_RESET, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);

	m_dsp_ctrl_data = data;
}


void rastersp_state::dsp_speedup_w(uint32_t data)
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


uint32_t rastersp_state::dsp_speedup_r()
{
	return m_speedup_count;
}


/*************************************
 *
 *  Memory maps
 *
 *  The FPGA limits access to the 1st 1MB of DRAM
 *  when A25 and A24 are both high
 *
 *************************************/

void rastersp_state::cpu_map_base(address_map &map)
{
	map(0x02200000, 0x022fffff).rw(FUNC(rastersp_state::nvram_r), FUNC(rastersp_state::nvram_w)).umask32(0x000000ff);
	map(0x02200800, 0x02200803).rw(FUNC(rastersp_state::interrupt_ctrl_r),FUNC(rastersp_state::interrupt_ctrl_w));
	map(0x02208000, 0x02208fff).rw("scsibus:7:ncr53c700", FUNC(ncr53c7xx_device::read), FUNC(ncr53c7xx_device::write));
	map(0x0220e000, 0x0220e003).w(FUNC(rastersp_state::dpylist_w));
	map(0xfff00000, 0xffffffff).bankrw("bank1");//was 3
}

void rastersp_state::cpu_map(address_map &map)
{
	cpu_map_base(map);
	map(0x00000000, 0x003fffff).ram().share("dram"); // 4MB DRAM on original board
	map(0x01000000, 0x010bffff).noprw(); // External ROM
}

void fbcrazy_state::cpu_map(address_map &map)
{
	cpu_map_base(map);
	map(0x00000000, 0x007fffff).ram().share("dram"); // 8MB DRAM on Football Crazy board
	map(0x01000000, 0x01000003).w( FUNC(fbcrazy_state::aux_port0_w)).umask32(0x000000ff);
	map(0x01000000, 0x01000003).portr("AUX_PORT0");
	map(0x01000004, 0x01000007).w( FUNC(fbcrazy_state::aux_port1_w)).umask32(0x000000ff);
	map(0x01000004, 0x01000007).portr("AUX_PORT1");
	map(0x01000008, 0x0100000b).w( FUNC(fbcrazy_state::aux_port2_w)).umask32(0x000000ff);
	map(0x0100000c, 0x0100000f).w( FUNC(fbcrazy_state::aux_port3_w)).umask32(0x000000ff);
	map(0x01000010, 0x01000013).w( FUNC(fbcrazy_state::aux_port4_w)).umask32(0x000000ff);
}

void rastersp_state::io_map(address_map &map)
{
	map(0x0000, 0x0000).r(FUNC(rastersp_state::interrupt_status_r));
	map(0x0020, 0x0023).w(FUNC(rastersp_state::cyrix_cache_w));
	map(0x1000, 0x1003).portr("P1").w(FUNC(rastersp_state::port1_w));
	map(0x1004, 0x1007).portr("P2").w(FUNC(rastersp_state::port2_w));
	map(0x1008, 0x100b).portr("COMMON").w(FUNC(rastersp_state::port3_w));
	map(0x100c, 0x100f).portr("DSW2");
	map(0x1010, 0x1013).portr("DSW1");
	map(0x1014, 0x1017).portr("EXTRA");
	map(0x4000, 0x4000).w("rtc", FUNC(mc146818_device::address_w));
	map(0x4004, 0x4004).rw("rtc", FUNC(mc146818_device::data_r), FUNC(mc146818_device::data_w));
	map(0x6000, 0x6000).rw( m_duart, FUNC(z80scc_device::cb_r), FUNC(z80scc_device::cb_w));
	map(0x6004, 0x6004).rw( m_duart, FUNC(z80scc_device::db_r), FUNC(z80scc_device::db_w));
	map(0x6008, 0x6008).rw( m_duart, FUNC(z80scc_device::ca_r), FUNC(z80scc_device::ca_w));
	map(0x600c, 0x600c).rw( m_duart, FUNC(z80scc_device::da_r), FUNC(z80scc_device::da_w));
}

void rastersp_state::dsp_map_base(address_map &map)
{
	map(0x400000, 0x40ffff).rom().region("dspboot", 0);
	map(0x808000, 0x80807f).rw(FUNC(rastersp_state::tms32031_control_r), FUNC(rastersp_state::tms32031_control_w));
	map(0x880402, 0x880402).w(FUNC(rastersp_state::dsp_unk_w));
	map(0x883c00, 0x883c00).w(FUNC(rastersp_state::dsp_486_int_w));
	map(0xc00000, 0xc03fff).bankrw("bank2");
	map(0xc80000, 0xc80000).w(FUNC(rastersp_state::dsp_ctrl_w));
	map(0xfc0000, 0xffffff).bankrw("bank1");//was3
}

void rastersp_state::dsp_map(address_map &map)
{
	dsp_map_base(map);
	map(0x000000, 0x0fffff).bankrw("bank1");//4 meg
}

void fbcrazy_state::dsp_map(address_map &map)
{
	rastersp_state::dsp_map_base(map);
	map(0x000000, 0x1fffff).bankrw("bank1");//8 meg
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

static INPUT_PORTS_START( fbcrazy )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COMMON")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )   PORT_NAME("Select") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )   PORT_NAME("Refill Key")   PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_DIPNAME( 0x20, 0x00, "4 6 Cashbox Door" )
	PORT_DIPSETTING(    0x00, "CLOSED" )
	PORT_DIPSETTING(    0x20, "OPEN" )
	PORT_DIPNAME( 0x40, 0x00, "4 7 Back Door" )
	PORT_DIPSETTING(    0x00, "CLOSED" )
	PORT_DIPSETTING(    0x40, "OPEN" )
	PORT_DIPNAME( 0x80, 0x00, "4 8 Front Door" )
	PORT_DIPSETTING(    0x00, "CLOSED" )
	PORT_DIPSETTING(    0x80, "OPEN" )

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

	PORT_START("AUX_PORT0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3) PORT_NAME("10p")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3) PORT_NAME("1 Pound")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3) PORT_NAME("5P")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Pay tube level") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AUX_PORT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x00, "Pay unit fitted" )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(fbcrazy_state::meter_pulse_r))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TRACK_X")  // Fake trackball input port
	PORT_BIT( 0xfff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("TRACK_Y")  // Fake trackball input port
	PORT_BIT( 0xfff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)
INPUT_PORTS_END


/*************************************
 *
 *  SCSI
 *
 *************************************/

uint32_t rastersp_state::ncr53c700_read(offs_t offset, uint32_t mem_mask)
{
	return m_maincpu->space(AS_PROGRAM).read_dword(offset, mem_mask);
}

void rastersp_state::ncr53c700_write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_maincpu->space(AS_PROGRAM).write_dword(offset, data, mem_mask);
}

void rastersp_state::ncr53c700_config(device_t *device)
{
	auto *state = device->subdevice<rastersp_state>(":");
	ncr53c7xx_device &scsictrl = downcast<ncr53c7xx_device &>(*device);
	scsictrl.set_clock(66'000'000);
	scsictrl.irq_handler().set(*state, FUNC(rastersp_state::scsi_irq));
	scsictrl.host_read().set(*state, FUNC(rastersp_state::ncr53c700_read));
	scsictrl.host_write().set(*state, FUNC(rastersp_state::ncr53c700_write));
}

void rastersp_state::rs_config_base(machine_config &config)
{
	I486(config, m_maincpu, 33'330'000);
	m_maincpu->set_irq_acknowledge_callback(FUNC(rastersp_state::irq_callback));
	m_maincpu->set_addrmap(AS_IO, &rastersp_state::io_map);

	TMS32031(config, m_dsp, 33'330'000);
	m_dsp->set_mcbl_mode(true); // Boot-loader mode

	MC146818(config, "rtc", 32.768_kHz_XTAL);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	NSCSI_BUS(config, "scsibus", 0);

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_seconds(1));

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
	SPEAKER(config, "speaker", 2).front();

	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_ldac, 0);
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_rdac, 0);
	m_ldac->add_route(ALL_OUTPUTS, "speaker", 0.5, 0); // unknown DAC
	m_rdac->add_route(ALL_OUTPUTS, "speaker", 0.5, 1); // unknown DAC

	SCC85C30(config, m_duart, 8'000'000);
	m_duart->configure_channels(1'843'200, 0, 1'843'200, 0);
	m_duart->out_int_callback().set(FUNC(rastersp_state::duart_irq));
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void rastersp_state::rastersp(machine_config &config)
{
	rs_config_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &rastersp_state::cpu_map);

	m_dsp->set_addrmap(AS_PROGRAM, &rastersp_state::dsp_map);

	nscsi_connector &connector0(NSCSI_CONNECTOR(config, "scsibus:0", 0));
	connector0.option_add("harddisk", NSCSI_HARDDISK);
	connector0.option_add_internal("ncr53c700", NCR53C7XX);
	connector0.set_default_option("harddisk");
	connector0.set_fixed(true);
}


void fbcrazy_state::fbcrazy(machine_config &config)
{
	rs_config_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &fbcrazy_state::cpu_map);

	m_dsp->set_addrmap(AS_PROGRAM, &fbcrazy_state::dsp_map);

	nscsi_connector &connector3(NSCSI_CONNECTOR(config, "scsibus:3", 0));
	connector3.option_add("cdrom", NSCSI_CDROM);
	connector3.option_add_internal("ncr53c700", NCR53C7XX);
	connector3.set_default_option("cdrom");
	connector3.set_fixed(true);

	bacta_datalogger_device &bacta(BACTA_DATALOGGER(config, "bacta", 0));

	m_duart->out_txda_callback().set("bacta", FUNC(bacta_datalogger_device::write_txd));
	bacta.rxd_handler().set(m_duart, FUNC(z80scc_device::rxa_w));
	m_duart->out_rtsa_callback().set(FUNC(fbcrazy_state::trackball_rts));

}

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( rotr )
	ROM_REGION32_LE(0x1000000, "dspboot", 0)
	ROM_LOAD32_BYTE( "rasterspeed2.1_bootrom4.u10", 0x00000, 0x10000, CRC(6da142d1) SHA1(e2dbd479034677726fc26fd1ba85c4458d89286c) )

	ROM_REGION(0x8000, "proms", 0) /* Xilinx FPGA PROMs */
	ROM_LOAD( "17128dpc.u72", 0x0000, 0x4000, CRC(5ddf6ee3) SHA1(f3d15b649b5641374a9e14877cea84ba9c57ef3c) )
	ROM_LOAD( "17128dpc.u73", 0x2000, 0x4000, CRC(9e274cea) SHA1(e974cad4e4b965bf2c9df7d3d0b4eec64629eeb0) )

	ROM_REGION(0x8000, "nvram", 0) /* Default NVRAM */
	ROM_LOAD( "rotr.nv", 0x0000, 0x8000, CRC(62543517) SHA1(a4bf3431cdab956839bb155c4a8c140d30e5c7ec) )

	DISK_REGION( "scsibus:0:harddisk" )
	DISK_IMAGE( "rotr", 0, SHA1(d67d7feb52d8c7ba1d2a190a40d97e84871f2d80) )
ROM_END


ROM_START( rotra )
	ROM_REGION32_LE(0x1000000, "dspboot", 0)
	ROM_LOAD32_BYTE( "rasterspeed2.1_bootrom4.u10", 0x00000, 0x10000, CRC(6da142d1) SHA1(e2dbd479034677726fc26fd1ba85c4458d89286c) )

	ROM_REGION(0x8000, "proms", 0) /* Xilinx FPGA PROMs */
	ROM_LOAD( "17128dpc.u72", 0x0000, 0x4000, CRC(5ddf6ee3) SHA1(f3d15b649b5641374a9e14877cea84ba9c57ef3c) )
	ROM_LOAD( "17128dpc.u73", 0x2000, 0x4000, CRC(9e274cea) SHA1(e974cad4e4b965bf2c9df7d3d0b4eec64629eeb0) )

	ROM_REGION(0x8000, "nvram", 0) /* Default NVRAM */
	ROM_LOAD( "rotr.nv", 0x0000, 0x8000, CRC(62543517) SHA1(a4bf3431cdab956839bb155c4a8c140d30e5c7ec) )

	DISK_REGION( "scsibus:0:harddisk" )
	DISK_IMAGE( "rotra", 0, SHA1(570d402e5e9bba123edf7dfa9db7a0e6bdb23823) )
ROM_END

/*
  Football Crazy runs on the (c)1997 "RASTERSPEED 2.1 31-599-001 ISS 4" PCB, which seems to be a more modern production version.
  a PCB photo with the ROM sticker showing the text below has also been seen

  Football Crazy Cashflow
  95 750 956
  STANDARD UK 64K
  VER. FOOT 3.2 BFM

  Bell Fruit issue roms in pairs.
  Roms with the 5th digit 0 do not require a Bacta Dataport unit to operate although they still send out the data
  Roms with the 5th digit 1 do required a Dataport unit and the machine will lock if one isn't fitted when the doors are closed.
  Football Crazy is wrong in that 95751937 still requires the Dataport unit even when the doors are open :(
  95751937 shows RS Mk 2.16 P Uk Boot ROM on power up while 95750937 shows RS Mk 2.16 N Uk Boot ROM
  The 'N' and 'P' referring to non-protocol and protocol.

*/

// the rom also exists in some odd hex format like this
// ROM_LOAD( "95751937.hex", 0x0000, 0x025f91, CRC(8f412e97) SHA1(a5ff924fbc327114e59d75de644ed0d5cd7fa6b3) )
ROM_START( fbcrazy )
	ROM_REGION32_LE(0x1000000, "dspboot", 0)
	ROM_LOAD32_BYTE( "95751937.bin", 0x0000, 0x010000, CRC(4a99ee11) SHA1(335398ebc64bbfe86e2652ac080a5943dd413928) )
	//ROM_LOAD32_BYTE( "95750937.bin", 0x0000, 0x010000, CRC(15364c8b) SHA1(c36903fd18ff8abfccca5dbbad8acda25d78e8f6) )

	ROM_REGION(0x8000, "proms", ROMREGION_ERASEFF )
	// Not on this PCB type?

	ROM_REGION(0x8000, "nvram", ROMREGION_ERASEFF )

	DISK_REGION( "scsibus:3:cdrom" )
	DISK_IMAGE_READONLY( "95100303", 0, SHA1(9ba47c96de27ec2bea9c6624d78d309b67705406)  )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1994, rotr,    0,    rastersp, rotr,    rastersp_state, empty_init, ROT0, "BFM/Mirage", "Rise of the Robots (prototype)",        MACHINE_SUPPORTS_SAVE )
GAME( 1994, rotra,   rotr, rastersp, rotr,    rastersp_state, empty_init, ROT0, "BFM/Mirage", "Rise of the Robots (prototype, older)", MACHINE_SUPPORTS_SAVE )

GAME( 1997, fbcrazy, 0,    fbcrazy,  fbcrazy, fbcrazy_state,  empty_init, ROT0, "BFM",        "Football Crazy (Video Quiz)",           MACHINE_SUPPORTS_SAVE )
