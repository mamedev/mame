// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, R. Belmont
/***************************************************************************

  nds.cpp

  Preliminary driver for first-generation Nintendo DS.

  Tech info: http://problemkaputt.de/gbatek.htm

  Notes:
    Timers and DMAs 0-3 are ARM9's, 4-7 are ARM7's.
    Interrupt registers [0] is ARM9, [1] is ARM7.

***************************************************************************/

#include "emu.h"
#include "nds.h"

#include "bus/gba/rom.h"
#include "bus/nds/rom.h"
#include "layout/generic.h"

#include "crsshair.h"
#include "multibyte.h"
#include "screen.h"
#include "softlist_dev.h"

#define LOG_UNK_RD      (1U << 1)
#define LOG_UNK_WR      (1U << 2)
#define LOG_TIMER       (1U << 3)
#define LOG_TIMER_EXTRA (1U << 4)
#define LOG_SPI         (1U << 5)
#define LOG_GAMECARD    (1U << 6)
#define LOG_INTERRUPT   (1U << 7)
#define LOG_DMA         (1U << 8)
#define LOG_IPC         (1U << 9)

#define VERBOSE         (0)
#include "logmacro.h"

// Measured value from GBATEK.  Actual crystal unknown.
#define MASTER_CLOCK (33513982)

#define INT_VBL                 0x00000001
#define INT_HBL                 0x00000002
#define INT_VCNT                0x00000004
#define INT_TM0_OVERFLOW        0x00000008
#define INT_TM1_OVERFLOW        0x00000010
#define INT_TM2_OVERFLOW        0x00000020
#define INT_TM3_OVERFLOW        0x00000040
#define INT_SIO                 0x00000080  // also RCNT/RTC (arm7 only)
#define INT_DMA0                0x00000100
#define INT_DMA1                0x00000200
#define INT_DMA2                0x00000400
#define INT_DMA3                0x00000800
#define INT_KEYPAD              0x00001000
#define INT_GAMEPAK             0x00002000  // GBA slot IRQ line (never used?)
#define INT_NA1                 0x00004000  // unused
#define INT_NA2                 0x00008000  // unused
#define INT_IPCSYNC             0x00010000
#define INT_IPCSENDEMPTY        0x00020000
#define INT_IPCRECVNOTEMPTY     0x00040000
#define INT_CARDXFERCOMPLETE    0x00080000
#define INT_CARDIREQ            0x00100000
#define INT_GEOCMDFIFO          0x00200000  // arm9 only
#define INT_SCREENUNFOLD        0x00400000  // arm7 only
#define INT_SPIBUS              0x00800000  // arm7 only
#define INT_WIFI                0x01000000  // arm7 only - also DSP on DSi
#define INT_CAMERA              0x02000000  // DSi only
#define INT_NA3                 0x04000000
#define INT_NA4                 0x08000000
#define INT_NEWDMA0             0x10000000  // DSi only
#define INT_NEWDMA1             0x20000000  // DSi only
#define INT_NEWDMA2             0x40000000  // DSi only
#define INT_NEWDMA3             0x80000000  // DSi only

// DMA startup modes.  The ARM9 has 3 mode bits, the ARM7 only 2 (and its
// modes 2 and 3 do not match the ARM9's).
enum
{
	DMA_MODE_IMMEDIATE = 0,
	DMA_MODE_VBLANK,
	DMA_MODE_HBLANK,
	DMA_MODE_DISPLAY_START,
	DMA_MODE_MAIN_MEMORY_DISPLAY,
	DMA_MODE_GAMECARD,
	DMA_MODE_GBA_SLOT,
	DMA_MODE_GEOMETRY_FIFO,
	DMA_MODE_SPECIAL        // GBA mode: sound FIFO / video capture
};

static const uint32_t timer_clks[4] = { MASTER_CLOCK, MASTER_CLOCK / 64, MASTER_CLOCK / 256, MASTER_CLOCK / 1024 };

// in GBA mode the ARM7 and its timers run at the GBA clock
#define GBA_CLOCK (16777216)
static const uint32_t gba_timer_clks[4] = { GBA_CLOCK, GBA_CLOCK / 64, GBA_CLOCK / 256, GBA_CLOCK / 1024 };

// SPI device select (SPICNT bits 8-9)
enum
{
	SPI_DEVICE_POWERMAN = 0,
	SPI_DEVICE_FIRMWARE,
	SPI_DEVICE_TOUCHSCREEN,
	SPI_DEVICE_RESERVED
};

nds_state::nds_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_vram(make_unique_clear<uint32_t[]>(VRAM_WORDS)),
	m_vram_page_off{},
	m_vram_page_count{},
	m_vram_bank_mapped{},
	m_arm7(*this, "arm7"),
	m_arm9(*this, "arm9"),
	m_firmware(*this, "firmware"),
	m_gbabios(*this, "gbabios"),
	m_arm7wrambnk(*this, "nds7wram"),
	m_arm9wrambnk(*this, "nds9wram"),
	m_arm7ram(*this, "arm7ram"),
	m_palette(*this, "palette"),
	m_oam(*this, "oam"),
	m_screen(*this, "screen%u", 0U),
	m_ppu(*this, "ppu_%c", 'a'),
	m_ndssound(*this, "ndssound"),
	m_rtc(*this, "rtc"),
	m_gbsound(*this, "gbsound"),
	m_gba_ldac(*this, "gba_ldac%c", 'a'),
	m_gba_rdac(*this, "gba_rdac%c", 'a'),
	m_ndscart(*this, "ndscart"),
	m_gbacart(*this, "gbacart"),
	m_keys(*this, "KEYS"),
	m_extkeys(*this, "EXTKEYS"),
	m_touch_x(*this, "TOUCH_X"),
	m_touch_y(*this, "TOUCH_Y"),
	m_arm7_postflg(0), m_arm9_postflg(0),
	m_ime{ 0, 0 }, m_ie{ 0, 0 }, m_if{ 0, 0 }, m_ipcsync{ 0, 0 },
	m_WRAM{},
	m_wramcnt(0),
	m_vramcnta(0), m_vramcntb(0), m_vramcntc(0), m_vramcntd(0), m_vramcnte(0), m_vramcntf(0), m_vramcntg(0), m_vramcnth(0), m_vramcnti(0),
	m_exmemcnt(0x6000),
	m_powcnt{ 0x0001, 0x0001 },
	m_biosprot(0),
	m_halted{ 0, 0 },
	m_sioregs{ 0, 0, 0, 0 },
	m_rcnt(0x8000),
	m_card_seed{ 0, 0, 0 },
	m_dispstat{ 0, 0 },
	m_vcount(0),
	m_scanline_timer(nullptr), m_hblank_timer(nullptr),
	m_keycnt{},
	m_ipcfifocnt{},	m_ipcfifo{}, m_ipcfifo_last{}, m_ipcfifo_head{}, m_ipcfifo_count{},
	m_spi_cnt(0), m_spi_data(0),
	m_fw_ram(std::make_unique<uint8_t[]>(0x40000)),
	m_fw_cmd(0), m_fw_stat(0), m_fw_addr(0), m_fw_bytes(0), m_fw_powerdown(false),
	m_pm_regs{ 0x0d, 0, 0, 0, 0, 0, 0, 0 },	m_pm_index(0), m_pm_have_index(false),
	m_tsc_result(0), m_tsc_pos(0),
	m_rtc_io(0),
	m_gba_mode(false),
	m_total_lines(TOTAL_LINES), m_visible_lines(VISIBLE_LINES),
	m_gba_soundregs{}, m_gba_waitcnt(0), m_gba_bios_prefetch(0), m_gba_fifo{},
	m_auxspicnt(0),	m_auxspidata(0),
	m_romctrl(0),
	m_card_command{}, m_cartdata_len(0), m_card_cpu(1),
	m_dma_timer{}, m_dma_srcreg{}, m_dma_dstreg{}, m_dma_ctrl{}, m_dma_src{}, m_dma_dst{}, m_dma_cnt{}, m_dma_fill{},
	m_timer_regs{},	m_timer_reload{}, m_timer_hz{},	m_timer_start{}, m_tmr_timer{},
	m_divcnt(0), m_sqrtcnt(0), m_div_numer(0), m_div_denom(0), m_div_result(0), m_divrem_result(0), m_sqrt_param(0), m_sqrt_result(0)
	{ }

void nds_state::update_irqs(int cpu)
{
	const bool pending = (m_ie[cpu] & m_if[cpu]) != 0;

	// HALT is left as soon as any enabled interrupt is pending, regardless of IME
	if (pending && m_halted[cpu])
	{
		set_halted(cpu, false);
	}

	const int state = (pending && (m_ime[cpu] & 1)) ? ASSERT_LINE : CLEAR_LINE;

	if (cpu == 0)
	{
		m_arm9->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, state);
	}
	else
	{
		m_arm7->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, state);
	}
}

void nds_state::request_irq(int cpu, uint32_t int_type)
{
	LOGMASKED(LOG_INTERRUPT, "request IRQ %08x on CPU %d\n", int_type, cpu);

	m_if[cpu] |= int_type;
	update_irqs(cpu);
}

void nds_state::set_halted(int cpu, bool halted)
{
	// halt applies if (IE & IF) is clear, so a pending IRQ means no halt at all
	if (halted && ((m_ie[cpu] & m_if[cpu]) != 0))
	{
		return;
	}

	if (halted == m_halted[cpu])
	{
		return;
	}

	m_halted[cpu] = halted;

	arm7_cpu_device *const cpudev = (cpu == 0) ? static_cast<arm7_cpu_device *>(m_arm9.target()) : m_arm7.target();
	if (halted)
	{
		cpudev->suspend(SUSPEND_REASON_HALT, 1);
	}
	else
	{
		cpudev->resume(SUSPEND_REASON_HALT);
	}
}

TIMER_CALLBACK_MEMBER(nds_state::scanline_tick)
{
	m_vcount++;
	if (m_vcount >= m_total_lines)
	{
		m_vcount = 0;
	}

	for (int cpu = 0; cpu < 2; cpu++)
	{
		// clear HBL at the start of the scanline
		m_dispstat[cpu] &= ~0x0002;

		// Nocash docs: VBL is *not* set on the last scanline (off-by-one error in the hardware?)
		if (m_vcount == m_visible_lines)
		{
			m_dispstat[cpu] |= 0x0001;
			if (BIT(m_dispstat[cpu], 3))
			{
				request_irq(cpu, INT_VBL);
			}
		}
		else if (m_vcount == (m_total_lines - 1))
		{
			m_dispstat[cpu] &= ~0x0001;
		}

		// handle raster interrupts
		const uint16_t lyc = ((m_dispstat[cpu] >> 8) & 0xff) | (BIT(m_dispstat[cpu], 7) << 8);
		if (m_vcount == lyc)
		{
			m_dispstat[cpu] |= 0x0004;
			if (BIT(m_dispstat[cpu], 5))
			{
				request_irq(cpu, INT_VCNT);
			}
		}
		else
		{
			m_dispstat[cpu] &= ~0x0004;
		}
	}

	if (m_vcount == m_visible_lines)
	{
		// GBA mode: the frame is complete, copy it to the DS screen
		if (m_gba_mode)
		{
			copybitmap(m_engine_bitmap[0], m_gba_work, 0, 0, 0, 0, m_engine_bitmap[0].cliprect());
		}

		dma_trigger(-1, DMA_MODE_VBLANK);

		update_keypad_irq(0);
		update_keypad_irq(1);
	}
}

TIMER_CALLBACK_MEMBER(nds_state::hblank_tick)
{
	if (m_vcount < m_visible_lines)
	{
		draw_scanline(m_vcount);
	}

	for (int cpu = 0; cpu < 2; cpu++)
	{
		m_dispstat[cpu] |= 0x0002;
		if (BIT(m_dispstat[cpu], 4))
		{
			request_irq(cpu, INT_HBL);
		}
	}

	// H-blank DMA is paused during V-blank; it is an ARM9-only mode on the DS, ARM7 in GBA mode
	if (m_vcount < m_visible_lines)
	{
		dma_trigger(m_gba_mode ? 1 : 0, DMA_MODE_HBLANK);
	}
}

/***************************************************************************
    Keypad
***************************************************************************/

void nds_state::update_keypad_irq(int cpu)
{
	if (!BIT(m_keycnt[cpu], 14))
	{
		return;
	}

	const uint16_t mask = m_keycnt[cpu] & 0x03ff;
	const uint16_t pressed = (~m_keys->read()) & 0x03ff;

	// bit 15: 0 = logical OR (any of the selected keys), 1 = logical AND (all of them)
	const bool fire = BIT(m_keycnt[cpu], 15) ? ((pressed & mask) == mask) : ((pressed & mask) != 0);

	if (fire)
	{
		request_irq(cpu, INT_KEYPAD);
	}
}

uint16_t nds_state::ipcfifo_cnt_r(int cpu)
{
	uint16_t data = m_ipcfifocnt[cpu] & 0xc404;

	if (m_ipcfifo_count[cpu] == 0)
	{
		data |= 0x0001;             // send FIFO empty
	}
	if (m_ipcfifo_count[cpu] == 16)
	{
		data |= 0x0002;             // send FIFO full
	}

	if (m_ipcfifo_count[cpu ^ 1] == 0)
	{
		data |= 0x0100;             // receive FIFO empty
	}
	if (m_ipcfifo_count[cpu ^ 1] == 16)
	{
		data |= 0x0200;             // receive FIFO full
	}

	return data;
}

void nds_state::ipcfifo_cnt_w(int cpu, uint16_t data)
{
	const bool old_send_empty_irq = BIT(m_ipcfifocnt[cpu], 2) && (m_ipcfifo_count[cpu] == 0);
	const bool old_recv_irq = BIT(m_ipcfifocnt[cpu], 10) && (m_ipcfifo_count[cpu ^ 1] != 0);

	if (BIT(data, 3))   // flush our send FIFO
	{
		m_ipcfifo_count[cpu] = 0;
		m_ipcfifo_head[cpu] = 0;
		m_ipcfifo_last[cpu] = 0;
	}

	// auto-acknowledge if bit 14 is set
	if (BIT(data, 14))
	{
		m_ipcfifocnt[cpu] &= ~0x4000;
	}

	m_ipcfifocnt[cpu] = (m_ipcfifocnt[cpu] & 0x4000) | (data & 0x8404);

	if (!old_send_empty_irq && BIT(m_ipcfifocnt[cpu], 2) && (m_ipcfifo_count[cpu] == 0))
	{
		request_irq(cpu, INT_IPCSENDEMPTY);
	}

	if (!old_recv_irq && BIT(m_ipcfifocnt[cpu], 10) && (m_ipcfifo_count[cpu ^ 1] != 0))
	{
		request_irq(cpu, INT_IPCRECVNOTEMPTY);
	}
}

void nds_state::ipcfifo_send(int cpu, uint32_t data)
{
	if (!BIT(m_ipcfifocnt[cpu], 15))
	{
		return;
	}

	// indicate an error if the send FIFO is full
	if (m_ipcfifo_count[cpu] == 16)
	{
		m_ipcfifocnt[cpu] |= 0x4000;
		return;
	}

	const bool was_empty = (m_ipcfifo_count[cpu] == 0);

	m_ipcfifo[cpu][(m_ipcfifo_head[cpu] + m_ipcfifo_count[cpu]) & 15] = data;
	m_ipcfifo_count[cpu]++;

	LOGMASKED(LOG_IPC, "IPC: CPU%d sends %08x (%d words queued)\n", cpu, data, m_ipcfifo_count[cpu]);

	if (was_empty && BIT(m_ipcfifocnt[cpu ^ 1], 10))
	{
		request_irq(cpu ^ 1, INT_IPCRECVNOTEMPTY);
	}
}

uint32_t nds_state::ipcfifo_recv(int cpu)
{
	const int remote = cpu ^ 1;

	// is the FIFO empty?
	if (m_ipcfifo_count[remote] == 0)
	{
		m_ipcfifocnt[cpu] |= 0x4000;
		return m_ipcfifo_last[remote];
	}

	const uint32_t data = m_ipcfifo[remote][m_ipcfifo_head[remote]];
	m_ipcfifo_last[remote] = data;

	// when the FIFO is disabled reads do not remove the word
	if (!BIT(m_ipcfifocnt[cpu], 15))
	{
		return data;
	}

	m_ipcfifo_head[remote] = (m_ipcfifo_head[remote] + 1) & 15;
	m_ipcfifo_count[remote]--;

	if ((m_ipcfifo_count[remote] == 0) && BIT(m_ipcfifocnt[remote], 2))
	{
		request_irq(remote, INT_IPCSENDEMPTY);
	}

	return data;
}

uint8_t nds_state::firmware_spi_transfer(uint8_t data)
{
	if (m_fw_cmd == 0)
	{
		m_fw_addr = 0;
		m_fw_bytes = 0;

		// while in power-down the chip ignores everything but RDP
		if (m_fw_powerdown && (data != 0xab))
		{
			return 0;
		}

		m_fw_cmd = data;

		switch (data)
		{
			case 0x06:  // WREN
				m_fw_stat |= 0x02;
				break;

			case 0x04:  // WRDI
				m_fw_stat &= ~0x02;
				break;

			case 0xb9:  // DP
				m_fw_powerdown = true;
				break;

			case 0xab:  // RDP
				m_fw_powerdown = false;
				break;
		}
		return 0;
	}

	switch (m_fw_cmd)
	{
		case 0x03:  // READ
		case 0x0b:  // FAST READ (one extra dummy byte after the address)
			if (m_fw_bytes < 3)
			{
				m_fw_addr = (m_fw_addr << 8) | data;
				m_fw_bytes++;
				return 0;
			}
			if ((m_fw_cmd == 0x0b) && (m_fw_bytes == 3))
			{
				m_fw_bytes++;
				return 0;
			}
			if (m_fw_bytes == 3)
			{
				m_fw_bytes++;
				LOGMASKED(LOG_SPI, "firmware: starting read at %05x\n", m_fw_addr);
			}
			return m_fw_ram[m_fw_addr++ & 0x3ffff];

		case 0x05:  // RDSR
			return m_fw_stat;

		case 0x9f:  // RDID
		{
			// TODO: we're emulating a ST M25P20-style 2Mbit part.  it works, but is that right?
			static const uint8_t id[3] = { 0x20, 0x40, 0x12 };
			return id[m_fw_bytes++ % 3];
		}

		case 0x02:  // PP (program: can only clear bits)
		case 0x0a:  // PW (page write: erases first, so it can set bits too)
			if (m_fw_bytes < 3)
			{
				m_fw_addr = (m_fw_addr << 8) | data;
				m_fw_bytes++;
				return 0;
			}
			if (BIT(m_fw_stat, 1))
			{
				// writes wrap within the 256-byte page they started in
				const uint32_t offset = (m_fw_addr & 0x3ff00) | ((m_fw_addr + m_fw_bytes - 3) & 0xff);
				if (m_fw_cmd == 0x02)
				{
					m_fw_ram[offset] &= data;
				}
				else
				{
					m_fw_ram[offset] = data;
				}
			}
			m_fw_bytes++;
			return 0;
	}

	return 0;
}

uint8_t nds_state::powerman_spi_transfer(uint8_t data)
{
	if (!m_pm_have_index)
	{
		m_pm_index = data;
		m_pm_have_index = true;
		return 0;
	}

	const int reg = m_pm_index & 3;

	if (BIT(m_pm_index, 7))
	{
		return m_pm_regs[reg];
	}

	// don't allow writes to the battery status
	if (reg == 1)
	{
		return 0;
	}

	m_pm_regs[reg] = data;

	if (BIT(m_pm_regs[0], 6))
	{
		LOGMASKED(LOG_SPI, "powerman: system shutdown requested\n");
	}

	return 0;
}

bool nds_state::touch_pressed() const
{
	return (m_extkeys->read() & 0x40) == 0;
}

uint8_t nds_state::tsc_spi_transfer(uint8_t data)
{
	if (BIT(data, 7))   // start bit: this is a control byte
	{
		const int channel = (data >> 4) & 7;
		const bool touched = touch_pressed();

		switch (channel)
		{
			case 1:     // Y position
				m_tsc_result = touched ? m_touch_y->read() : 0xfff;
				break;

			case 3:     // Z1
				m_tsc_result = touched ? 0x100 : 0x000;
				break;

			case 4:     // Z2
				m_tsc_result = touched ? 0xf00 : 0xfff;
				break;

			case 5:     // X position
				m_tsc_result = touched ? m_touch_x->read() : 0x000;
				break;

			case 2:     // battery voltage: tied to ground on the DS
				m_tsc_result = 0x000;
				break;

			default:    // temperature sensors and the microphone AUX input
				m_tsc_result = 0x800;
				break;
		}

		m_tsc_pos = 0;
		return 0;
	}

	// one dummy bit followed by the 12-bit result, MSB first
	m_tsc_pos++;
	if (m_tsc_pos == 1)
	{
		return (m_tsc_result >> 5) & 0xff;
	}
	else if (m_tsc_pos == 2)
	{
		return (m_tsc_result << 3) & 0xff;
	}

	return 0;
}

uint8_t nds_state::spi_transfer(uint8_t data)
{
	switch ((m_spi_cnt >> 8) & 3)
	{
		case SPI_DEVICE_POWERMAN:
			return powerman_spi_transfer(data);

		case SPI_DEVICE_FIRMWARE:
			return firmware_spi_transfer(data);

		case SPI_DEVICE_TOUCHSCREEN:
			return tsc_spi_transfer(data);
	}

	return 0;
}

void nds_state::spi_deselect()
{
	m_fw_cmd = 0;
	m_pm_have_index = false;
	m_tsc_pos = 0;
}

void nds_state::rtc_w(uint8_t data)
{
	m_rtc_io = data;

	m_rtc->data_w(BIT(data, 0));
	m_rtc->cs_w(BIT(data, 2));
	m_rtc->sck_w(BIT(data, 1));
}

uint8_t nds_state::rtc_r()
{
	if (BIT(m_rtc_io, 4))
	{
		return m_rtc_io;
	}

	return (m_rtc_io & ~0x01) | m_rtc->data_r();
}

/***************************************************************************
    Game card interface
***************************************************************************/

void nds_state::gamecard_start_transfer()
{
	m_cartdata_len = (m_romctrl >> 24) & 7;
	if (m_cartdata_len == 7)
	{
		m_cartdata_len = 4;
	}
	else if (m_cartdata_len != 0)
	{
		m_cartdata_len = 0x100 << m_cartdata_len;
	}

	LOGMASKED(LOG_GAMECARD, "nds: card command %02x%02x%02x%02x%02x%02x%02x%02x, %x bytes\n",
		m_card_command[0], m_card_command[1], m_card_command[2], m_card_command[3],
		m_card_command[4], m_card_command[5], m_card_command[6], m_card_command[7], m_cartdata_len);

	m_ndscart->command_start(m_card_command, m_cartdata_len);

	if (m_cartdata_len > 0)
	{
		m_romctrl |= GAMECARD_DATA_READY;

		// A DMA channel in DS card mode is triggered once per data word: the BIOS programs
		// it with a count of 1 and repeat, so keep triggering until the block is drained
		// (or until the channel stops taking it and the CPU has to read the port itself).
		for (uint32_t guard = (m_cartdata_len + 3) / 4; (guard > 0) && (m_romctrl & GAMECARD_DATA_READY); guard--)
		{
			if (!dma_trigger(m_card_cpu, DMA_MODE_GAMECARD))
			{
				break;
			}
		}
	}
	else
	{
		gamecard_end_transfer();
	}
}

void nds_state::gamecard_end_transfer()
{
	LOGMASKED(LOG_GAMECARD, "NDS: xfer over\n");

	m_romctrl &= ~(GAMECARD_DATA_READY | GAMECARD_BLOCK_BUSY);

	if (BIT(m_auxspicnt, 14))
	{
		request_irq(m_card_cpu, INT_CARDXFERCOMPLETE);
	}
}

uint32_t nds_state::gamecard_data_r()
{
	if (!(m_romctrl & GAMECARD_DATA_READY))
	{
		return 0xffffffff;
	}

	if (m_cartdata_len >= 4)
	{
		m_cartdata_len -= 4;
	}
	else
	{
		m_cartdata_len = 0;
	}

	if (m_cartdata_len == 0)
	{
		gamecard_end_transfer();
	}

	// with no card the bus floats high, which is what the slot returns then
	return m_ndscart->data_r();
}

template <int Cpu> uint32_t nds_state::gba_rom_r(offs_t offset)
{
	if (BIT(m_exmemcnt, 7) != Cpu)
	{
		return 0xffffffff;
	}
	return m_gbacart->read_rom(offset);
}

template <int Cpu> uint32_t nds_state::gba_ram_r(offs_t offset, uint32_t mem_mask)
{
	if (BIT(m_exmemcnt, 7) != Cpu)
	{
		return 0xffffffff;
	}
	return m_gbacart->read_ram(offset, mem_mask);
}

template <int Cpu> void nds_state::gba_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (BIT(m_exmemcnt, 7) == Cpu)
	{
		m_gbacart->write_ram(offset, data, mem_mask);
	}
}

/***************************************************************************
    DMA
***************************************************************************/

void nds_state::dma_control_w(int ch, uint32_t data, uint32_t mem_mask)
{
	const uint32_t old = m_dma_ctrl[ch];
	COMBINE_DATA(&m_dma_ctrl[ch]);

	if (!BIT(old, 31) && BIT(m_dma_ctrl[ch], 31))
	{
		// latch the address and count registers when the channel is enabled
		m_dma_src[ch] = m_dma_srcreg[ch];
		m_dma_dst[ch] = m_dma_dstreg[ch];
		m_dma_cnt[ch] = m_dma_ctrl[ch] & ((ch < 4) ? 0x1fffff : ((ch == 7) ? 0xffff : 0x3fff));

		LOGMASKED(LOG_DMA, "DMA%d: enable %08x -> %08x, %x units, ctrl %08x\n",
			ch, m_dma_src[ch], m_dma_dst[ch], m_dma_cnt[ch], m_dma_ctrl[ch]);

		// the ARM9 has three mode bits, the ARM7 only two
		const int mode = (ch < 4) ? ((m_dma_ctrl[ch] >> 27) & 7) : ((m_dma_ctrl[ch] >> 28) & 3);
		if (mode == DMA_MODE_IMMEDIATE)
		{
			dma_exec(ch);
		}
	}
}

bool nds_state::dma_trigger(int cpu, int mode)
{
	bool ran = false;
	for (int ch = 0; ch < 8; ch++)
	{
		if ((cpu >= 0) && ((ch >> 2) != cpu))
		{
			continue;
		}

		if (!BIT(m_dma_ctrl[ch], 31))
		{
			continue;
		}

		// the ARM7's two mode bits map onto the ARM9's numbering as 0, 1, 5 (DS card) and 6 (GBA slot / wireless);
		static constexpr int arm7_modes[4] = { DMA_MODE_IMMEDIATE, DMA_MODE_VBLANK, DMA_MODE_GAMECARD, DMA_MODE_GBA_SLOT };
		static constexpr int gba_modes[4] = { DMA_MODE_IMMEDIATE, DMA_MODE_VBLANK, DMA_MODE_HBLANK, DMA_MODE_SPECIAL };
		const int chmode = (ch < 4) ? ((m_dma_ctrl[ch] >> 27) & 7) : (m_gba_mode ? gba_modes : arm7_modes)[(m_dma_ctrl[ch] >> 28) & 3];
		if (chmode == mode)
		{
			dma_exec(ch);
			ran = true;
		}
	}
	return ran;
}

void nds_state::dma_exec(int ch)
{
	address_space &space = (ch < 4) ? m_arm9->space(AS_PROGRAM) : m_arm7->space(AS_PROGRAM);
	const uint32_t ctrl = m_dma_ctrl[ch];
	const int srcadd = (ctrl >> 23) & 3;
	int dstadd = (ctrl >> 21) & 3;
	bool wide = BIT(ctrl, 26);

	uint32_t src = m_dma_src[ch];
	uint32_t dst = m_dma_dst[ch];
	uint32_t cnt = m_dma_cnt[ch];

	if (cnt == 0)
	{
		cnt = (ch < 4) ? 0x200000 : ((ch == 7) ? 0x10000 : 0x4000);
	}

	// a GBA-mode direct sound DMA always transfers four 32-bit words
	if (m_gba_mode && ((ch == 5) || (ch == 6)) && (((ctrl >> 28) & 3) == 3))
	{
		cnt = 4;
		wide = true;
		dstadd = 2;
	}

	const int step = wide ? 4 : 2;

	LOGMASKED(LOG_DMA, "DMA%d: %08x -> %08x, %x %d-bit units\n", ch, src, dst, cnt, wide ? 32 : 16);

	src &= ~(step - 1);
	dst &= ~(step - 1);

	for (uint32_t i = 0; i < cnt; i++)
	{
		if (wide)
		{
			space.write_dword(dst, space.read_dword(src));
		}
		else
		{
			space.write_word(dst, space.read_word(src));
		}

		switch (dstadd)
		{
			case 0: dst += step; break;     // increment
			case 1: dst -= step; break;     // decrement
			case 2: break;                  // fixed
			case 3: dst += step; break;     // increment and reload
		}

		switch (srcadd)
		{
			case 0: src += step; break;     // increment
			case 1: src -= step; break;     // decrement
			case 2: break;                  // fixed
			case 3: break;                  // prohibited: treat as fixed
		}
	}

	m_dma_src[ch] = src;
	m_dma_dst[ch] = dst;

	const int mode = (ch < 4) ? ((ctrl >> 27) & 7) : ((ctrl >> 28) & 3);

	// repeat mode leaves the channel enabled so the next trigger restarts it
	if (BIT(ctrl, 25) && (mode != DMA_MODE_IMMEDIATE))
	{
		m_dma_cnt[ch] = ctrl & ((ch < 4) ? 0x1fffff : ((ch == 7) ? 0xffff : 0x3fff));
		if (dstadd == 3)
		{
			m_dma_dst[ch] = m_dma_dstreg[ch];
		}
	}
	else
	{
		m_dma_ctrl[ch] &= ~0x80000000;
	}

	if (BIT(ctrl, 30))
	{
		request_irq((ch < 4) ? 0 : 1, INT_DMA0 << (ch & 3));
	}
}

// TODO: make DMA take some time
TIMER_CALLBACK_MEMBER(nds_state::dma_complete)
{
}

/***************************************************************************
    Timers
***************************************************************************/

uint16_t nds_state::timer_count_r(int timer)
{
	if (!BIT(m_timer_regs[timer], 23) || BIT(m_timer_regs[timer], 18))
	{
		return m_timer_regs[timer] & 0xffff;
	}

	const uint32_t rate = (m_gba_mode ? gba_timer_clks : timer_clks)[(m_timer_regs[timer] >> 16) & 3];
	const uint64_t ticks = (machine().time() - m_timer_start[timer]).as_ticks(rate);

	return uint16_t(m_timer_reload[timer] + ticks);
}

void nds_state::timer_start(int timer, uint32_t old_regs, uint32_t data)
{
	// the reload value is copied into the counter when the start bit goes 0 -> 1
	if (!BIT(old_regs, 23))
	{
		m_timer_regs[timer] = (m_timer_regs[timer] & 0xffff0000) | m_timer_reload[timer];
	}

	if (!BIT(m_timer_regs[timer], 23) || BIT(m_timer_regs[timer], 18))
	{
		// stopped, or clocked by the preceding timer rather than by the prescaler
		m_tmr_timer[timer]->adjust(attotime::never);
		return;
	}

	const uint32_t rate = (m_gba_mode ? gba_timer_clks : timer_clks)[(m_timer_regs[timer] >> 16) & 3];
	const uint32_t ticks = 0x10000 - (m_timer_regs[timer] & 0xffff);

	m_timer_hz[timer] = double(rate) / double(ticks);
	m_timer_start[timer] = machine().time();
	m_tmr_timer[timer]->adjust(attotime::from_ticks(ticks, rate), timer);

	LOGMASKED(LOG_TIMER_EXTRA, "Enabling timer %d @ %f Hz regs %08x\n", timer, m_timer_hz[timer], m_timer_regs[timer]);
}

void nds_state::timer_tick_countup(int timer)
{
	const int cpu = (timer >= 4) ? 1 : 0;

	m_timer_regs[timer] = (m_timer_regs[timer] & 0xffff0000) | m_timer_reload[timer];
	m_timer_start[timer] = machine().time();

	if (BIT(m_timer_regs[timer], 22))
	{
		request_irq(cpu, INT_TM0_OVERFLOW << (timer & 3));
	}

	// GBA mode: timers 0 and 1 (ARM7 channels 4 and 5) clock the DirectSound FIFOs
	if (m_gba_mode && (timer == 4 || timer == 5))
	{
		const uint16_t cnt_h = m_gba_soundregs[(0x80 - 0x60)/4] >> 16;
		const int gbatimer = timer - 4;
		if (int((cnt_h >> 10) & 1) == gbatimer)
		{
			gba_audio_tick(0);
		}
		if (int((cnt_h >> 14) & 1) == gbatimer)
		{
			gba_audio_tick(1);
		}
	}

	// clock the next timer along if it is in count-up mode
	if ((timer & 3) == 3)
	{
		return;
	}

	const int next = timer + 1;
	if (!BIT(m_timer_regs[next], 23) || !BIT(m_timer_regs[next], 18))
	{
		return;
	}

	const uint16_t count = (m_timer_regs[next] + 1) & 0xffff;
	m_timer_regs[next] = (m_timer_regs[next] & 0xffff0000) | count;
	if (count == 0)
	{
		timer_tick_countup(next);
	}
}

TIMER_CALLBACK_MEMBER(nds_state::timer_expire)
{
	const int tmr = param;

	LOGMASKED(LOG_TIMER_EXTRA, "Timer %d expired\n", tmr);

	timer_tick_countup(tmr);

	// reschedule using the value that was just reloaded
	const uint32_t rate = (m_gba_mode ? gba_timer_clks : timer_clks)[(m_timer_regs[tmr] >> 16) & 3];
	const uint32_t ticks = 0x10000 - (m_timer_regs[tmr] & 0xffff);
	m_tmr_timer[tmr]->adjust(attotime::from_ticks(ticks, rate), tmr);
}

/***************************************************************************
    ARM9 divider and square root units
***************************************************************************/

void nds_state::div_calculate()
{
	m_divcnt &= ~0x4000;

	int64_t numer, denom;
	switch (m_divcnt & 3)
	{
		case 0:     // 32bit / 32bit
			numer = int32_t(uint32_t(m_div_numer));
			denom = int32_t(uint32_t(m_div_denom));
			break;

		case 1:     // 64bit / 32bit
		case 3:     // prohibited, acts as mode 1
			numer = int64_t(m_div_numer);
			denom = int32_t(uint32_t(m_div_denom));
			break;

		default:    // 64bit / 64bit
			numer = int64_t(m_div_numer);
			denom = int64_t(m_div_denom);
			break;
	}

	if (denom == 0)
	{
		m_divcnt |= 0x4000;         // division by zero
		m_divrem_result = uint64_t(numer);
		m_div_result = (numer < 0) ? 1 : uint64_t(-1);
		if ((m_divcnt & 3) == 0)
		{
			m_div_result ^= 0xffffffff00000000ULL;
		}
	}
	else if ((denom == -1) && (uint64_t(numer) == 0x8000000000000000ULL))
	{
		m_div_result = uint64_t(numer);
		m_divrem_result = 0;
	}
	else
	{
		m_div_result = uint64_t(numer / denom);
		m_divrem_result = uint64_t(numer % denom);
	}
}

void nds_state::sqrt_calculate()
{
	uint64_t val = BIT(m_sqrtcnt, 0) ? m_sqrt_param : uint32_t(m_sqrt_param);

	// integer square root by restoring shift-and-subtract
	uint64_t rem = 0, root = 0;
	for (int i = 0; i < 32; i++)
	{
		root <<= 1;
		rem = (rem << 2) | ((val >> 62) & 3);
		val <<= 2;
		if (rem > root)
		{
			rem -= root | 1;
			root += 2;
		}
	}

	m_sqrt_result = uint32_t(root >> 1);
}

/***************************************************************************
    Common I/O
***************************************************************************/

uint32_t nds_state::common_io_r(int cpu, offs_t offset, uint32_t mem_mask, bool &handled)
{
	handled = true;

	// DMA channels: SAD, DAD and CNT for each of four channels
	if ((offset >= DMA_OFFSET) && (offset < (DMA_OFFSET + 12)))
	{
		const int ch = ((offset - DMA_OFFSET) / 3) + (cpu * 4);
		switch ((offset - DMA_OFFSET) % 3)
		{
			case 0: return m_dma_srcreg[ch];
			case 1: return m_dma_dstreg[ch];
			default: return m_dma_ctrl[ch];
		}
	}

	if ((offset >= TIMER_OFFSET) && (offset < (TIMER_OFFSET + 4)))
	{
		const int timer = (offset - TIMER_OFFSET) + (cpu * 4);
		LOGMASKED(LOG_TIMER, "Read timer reg %x\n", timer);
		return (m_timer_regs[timer] & 0xffff0000) | timer_count_r(timer);
	}

	switch (offset)
	{
		case DISPSTAT_OFFSET:
			return m_dispstat[cpu] | (uint32_t(m_vcount) << 16);

		case KEYINPUT_OFFSET:
			return (m_keys->read() & 0x03ff) | (uint32_t(m_keycnt[cpu]) << 16);

		case IPCSYNC_OFFSET:
			return m_ipcsync[cpu];

		case IPCFIFOCNT_OFFSET:
			return ipcfifo_cnt_r(cpu);

		case EXMEMCNT_OFFSET:
			return m_exmemcnt;

		case IME_OFFSET:
			return m_ime[cpu];

		case IE_OFFSET:
			return m_ie[cpu];

		case IF_OFFSET:
			return m_if[cpu];

		case AUX_SPI_CNT_OFFSET:
			LOGMASKED(LOG_GAMECARD, "cpu%d: read AUXSPICNT mask %08x\n", cpu, mem_mask);
			return m_auxspicnt | (uint32_t(m_auxspidata) << 16);

		case GAMECARD_BUS_CTRL_OFFSET:
			LOGMASKED(LOG_GAMECARD, "cpu%d: read ROMCTRL (%08x) mask %08x\n", cpu, m_romctrl, mem_mask);
			return m_romctrl;

		case GAMECARD_DATA_OFFSET:
			return (m_card_command[0] << 24) | (m_card_command[1] << 16) | (m_card_command[2] << 8) | m_card_command[3];

		case GAMECARD_DATA_2_OFFSET:
			return (m_card_command[4] << 24) | (m_card_command[5] << 16) | (m_card_command[6] << 8) | m_card_command[7];

		case IPCFIFORECV_OFFSET:
			return ipcfifo_recv(cpu);

		case GAMECARD_DATA_IN_OFFSET:
			return gamecard_data_r();
	}

	handled = false;
	return 0;
}

void nds_state::common_io_w(int cpu, offs_t offset, uint32_t data, uint32_t mem_mask, bool &handled)
{
	handled = true;

	if ((offset >= DMA_OFFSET) && (offset < (DMA_OFFSET + 12)))
	{
		const int ch = ((offset - DMA_OFFSET) / 3) + (cpu * 4);
		switch ((offset - DMA_OFFSET) % 3)
		{
			// the ARM7's DMA reaches internal memory only (27 bit addresses) - except in GBA mode,
			// where channels 1-3 can read the Game Pak and channel 3 can write it
			case 0:
				COMBINE_DATA(&m_dma_srcreg[ch]);
				m_dma_srcreg[ch] &= (!cpu || (m_gba_mode && (ch != 4))) ? 0x0ffffffe : 0x07fffffe;
				return;

			case 1:
				COMBINE_DATA(&m_dma_dstreg[ch]);
				m_dma_dstreg[ch] &= (!cpu || (m_gba_mode && (ch == 7))) ? 0x0ffffffe : 0x07fffffe;
				return;

			default:
				dma_control_w(ch, data, mem_mask);
				return;
		}
	}

	if ((offset >= TIMER_OFFSET) && (offset < (TIMER_OFFSET + 4)))
	{
		const int timer = (offset - TIMER_OFFSET) + (cpu * 4);
		const uint32_t old_timer_regs = m_timer_regs[timer];

		LOGMASKED(LOG_TIMER, "%08x to timer %d (mask %08x)\n", data, timer, mem_mask);

		if (ACCESSING_BITS_0_15)
		{
			m_timer_reload[timer] = (m_timer_reload[timer] & ~mem_mask) | (data & mem_mask);
		}

		if (ACCESSING_BITS_16_31)
		{
			m_timer_regs[timer] = (m_timer_regs[timer] & ~(mem_mask & 0xffff0000)) | (data & mem_mask & 0xffff0000);
			timer_start(timer, old_timer_regs, data);
		}
		return;
	}

	switch (offset)
	{
		case DISPSTAT_OFFSET:
			if (ACCESSING_BITS_0_15)
			{
				// bits 0-2 are read-only status flags
				const uint16_t mask = mem_mask & 0xfff8;
				m_dispstat[cpu] = (m_dispstat[cpu] & ~mask) | (data & mask);
			}
			return;

		case KEYINPUT_OFFSET:
			if (ACCESSING_BITS_16_31)
			{
				m_keycnt[cpu] = (m_keycnt[cpu] & ~(mem_mask >> 16)) | ((data & mem_mask) >> 16);
				update_keypad_irq(cpu);
			}
			return;

		case IPCSYNC_OFFSET:
		{
			const int remote = cpu ^ 1;

			LOGMASKED(LOG_IPC, "CPU%d: %04x to IPCSYNC\n", cpu, data & 0xffff);

			// our output nibble becomes the remote CPU's input nibble
			m_ipcsync[remote] = (m_ipcsync[remote] & ~0x000f) | ((data >> 8) & 0x000f);
			m_ipcsync[cpu] = (m_ipcsync[cpu] & 0x000f) | (data & 0x4f00);

			if (BIT(data, 13) && BIT(m_ipcsync[remote], 14))
			{
				request_irq(remote, INT_IPCSYNC);
			}
			return;
		}

		case IPCFIFOCNT_OFFSET:
			if (ACCESSING_BITS_0_15)
			{
				ipcfifo_cnt_w(cpu, data & 0xffff);
			}
			return;

		case IPCFIFOSEND_OFFSET:
			ipcfifo_send(cpu, data);
			return;

		case EXMEMCNT_OFFSET:
			if (ACCESSING_BITS_0_15)
			{
				// the ARM7 can only change the low seven bits
				const uint16_t mask = cpu ? (mem_mask & 0x007f) : (mem_mask & 0xe8ff);
				m_exmemcnt = (m_exmemcnt & ~mask) | (data & mask);
			}
			return;

		case IME_OFFSET:
			LOGMASKED(LOG_INTERRUPT, "CPU%d: %08x to IME\n", cpu, data);
			COMBINE_DATA(&m_ime[cpu]);
			update_irqs(cpu);
			return;

		case IE_OFFSET:
			LOGMASKED(LOG_INTERRUPT, "CPU%d: %08x to IE\n", cpu, data);
			COMBINE_DATA(&m_ie[cpu]);
			update_irqs(cpu);
			return;

		case IF_OFFSET:
			// writing a 1 acknowledges that interrupt
			m_if[cpu] &= ~(data & mem_mask);
			update_irqs(cpu);
			return;

		case AUX_SPI_CNT_OFFSET:
			if (ACCESSING_BITS_0_15)
			{
				LOGMASKED(LOG_GAMECARD, "cpu%d: %04x to AUXSPICNT\n", cpu, data & 0xffff);
				m_auxspicnt = (m_auxspicnt & ~(mem_mask & 0xe043)) | (data & mem_mask & 0xe043);
			}
			if (ACCESSING_BITS_16_31)
			{
				// AUXSPIDATA: a transfer with the backup chip on the card
				m_auxspidata = BIT(m_auxspicnt, 13) ? m_ndscart->spi_transfer((data >> 16) & 0xff) : 0xff;
				if (!BIT(m_auxspicnt, 6))
				{
					m_ndscart->spi_deselect();
				}
			}
			return;

		case GAMECARD_BUS_CTRL_OFFSET:
		{
			LOGMASKED(LOG_GAMECARD, "cpu%d: %08x to ROMCTRL mask %08x\n", cpu, data, mem_mask);

			const uint32_t old = m_romctrl;
			// bit 23 is a read-only status flag, and bit 29 can only ever be set
			const uint32_t mask = mem_mask & ~GAMECARD_DATA_READY;
			m_romctrl = (m_romctrl & ~mask) | (data & mask) | (old & 0x20000000);

			if (BIT(m_auxspicnt, 15) && !BIT(old, 31) && BIT(m_romctrl, 31))
			{
				m_card_cpu = cpu;
				gamecard_start_transfer();
			}
			return;
		}

		case GAMECARD_DATA_OFFSET:
			if (ACCESSING_BITS_0_7)
			{
				m_card_command[0] = data & 0xff;
			}
			if (ACCESSING_BITS_8_15)
			{
				m_card_command[1] = (data >> 8) & 0xff;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_card_command[2] = (data >> 16) & 0xff;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_card_command[3] = (data >> 24) & 0xff;
			}
			return;

		case GAMECARD_DATA_2_OFFSET:
			if (ACCESSING_BITS_0_7)
			{
				m_card_command[4] = data & 0xff;
			}
			if (ACCESSING_BITS_8_15)
			{
				m_card_command[5] = (data >> 8) & 0xff;
			}
			if (ACCESSING_BITS_16_23)
			{
				m_card_command[6] = (data >> 16) & 0xff;
			}
			if (ACCESSING_BITS_24_31)
			{
				m_card_command[7] = (data >> 24) & 0xff;
			}
			return;
	}

	handled = false;
}

/***************************************************************************
    ARM7 I/O
***************************************************************************/

uint32_t nds_state::arm7_io_r(offs_t offset, uint32_t mem_mask)
{
	bool handled = false;
	const uint32_t data = common_io_r(1, offset, mem_mask, handled);
	if (handled)
	{
		return data;
	}

	if ((offset >= SOUND_OFFSET) && (offset < SOUND_END_OFFSET))
	{
		return m_ndssound->read(offset - SOUND_OFFSET, mem_mask);
	}

	if ((offset >= GAMECARD_SEED_OFFSET) && (offset < (GAMECARD_SEED_OFFSET + 3)))
	{
		return m_card_seed[offset - GAMECARD_SEED_OFFSET];
	}

	if ((offset >= SIO_OFFSET) && (offset < (SIO_OFFSET + 4)))
	{
		return m_sioregs[offset - SIO_OFFSET];
	}

	switch (offset)
	{
		case RCNT_OFFSET:
			// RCNT in the low half, EXTKEYIN in the high half
			return m_rcnt | ((m_extkeys->read() & 0x00ff) << 16);

		case RTC_OFFSET:
			return rtc_r();

		case SPI_CTRL_OFFSET:
			LOGMASKED(LOG_SPI, "arm7: read SPICNT (%04x) / SPIDATA (%02x)\n", m_spi_cnt, m_spi_data);
			return m_spi_cnt | (uint32_t(m_spi_data) << 16);

		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*/
			return m_arm7_postflg;

		case POWCNT_OFFSET:
			return m_powcnt[1];

		case WRAMSTAT_OFFSET:
		{
			const uint8_t temp1 = (((m_vramcntc & 3) == 2) && BIT(m_vramcntc, 7)) ? 1 : 0;
			const uint8_t temp2 = (((m_vramcntd & 3) == 2) && BIT(m_vramcntd, 7)) ? 2 : 0;
			return (m_wramcnt << 8) | temp1 | temp2;
		}

		default:
			LOGMASKED(LOG_UNK_RD, "[ARM7] [IO] Unknown read: %08x (%08x)\n", offset*4, mem_mask);
			break;
	}

	return 0;
}

void nds_state::arm7_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	bool handled = false;
	common_io_w(1, offset, data, mem_mask, handled);
	if (handled)
	{
		return;
	}

	if ((offset >= SOUND_OFFSET) && (offset < SOUND_END_OFFSET))
	{
		m_ndssound->write(offset - SOUND_OFFSET, data, mem_mask);
		return;
	}

	if ((offset >= GAMECARD_SEED_OFFSET) && (offset < (GAMECARD_SEED_OFFSET + 3)))
	{
		COMBINE_DATA(&m_card_seed[offset - GAMECARD_SEED_OFFSET]);
		return;
	}

	if ((offset >= SIO_OFFSET) && (offset < (SIO_OFFSET + 4)))
	{
		COMBINE_DATA(&m_sioregs[offset - SIO_OFFSET]);
		return;
	}

	switch (offset)
	{
		case RCNT_OFFSET:
			if (ACCESSING_BITS_0_15)
			{
				m_rcnt = (m_rcnt & ~(mem_mask & 0xffff)) | (data & mem_mask & 0xffff);
			}
			break;

		case BIOSPROT_OFFSET:
			// write-once BIOS read protection boundary
			if (m_biosprot == 0)
			{
				COMBINE_DATA(&m_biosprot);
			}
			break;

		case RTC_OFFSET:
			if (ACCESSING_BITS_0_7)
			{
				rtc_w(data & 0xff);
			}
			break;

		case SPI_CTRL_OFFSET:
			if (ACCESSING_BITS_0_15)
			{
				LOGMASKED(LOG_SPI, "arm7: %04x to SPICNT\n", data & 0xffff);

				const uint16_t old = m_spi_cnt;
				m_spi_cnt = (m_spi_cnt & ~(mem_mask & 0xcf03)) | (data & mem_mask & 0xcf03);

				// changing device or turning the bus off drops the chipselect
				if (!BIT(m_spi_cnt, 15) || (((old ^ m_spi_cnt) & 0x0300) != 0))
				{
					spi_deselect();
				}
			}

			if (ACCESSING_BITS_16_31)
			{
				if (BIT(m_spi_cnt, 15))
				{
					m_spi_data = spi_transfer((data >> 16) & 0xff);

					// with "chipselect hold" clear the device is deselected after the transfer
					if (!BIT(m_spi_cnt, 11))
					{
						spi_deselect();
					}

					if (BIT(m_spi_cnt, 14))
					{
						request_irq(1, INT_SPIBUS);
					}
				}
				else
				{
					m_spi_data = 0;
				}

				LOGMASKED(LOG_SPI, "arm7: SPI xfer %02x -> %02x\n", (data >> 16) & 0xff, m_spi_data);
			}
			break;

		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*/
			if (ACCESSING_BITS_0_7)
			{
				if (!(m_arm7_postflg & POSTFLG_PBF_MASK) && m_arm7->pc() < 0x4000)
				{
					m_arm7_postflg &= ~POSTFLG_PBF_MASK;
					m_arm7_postflg |= data & POSTFLG_PBF_MASK;
				}
			}

			if (ACCESSING_BITS_8_15)
			{
				// HALTCNT: bits 6-7 select GBA mode / halt / sleep
				switch ((data >> 14) & 3)
				{
					case 1:
						enter_gba_mode();
						break;

					case 2:
						LOGMASKED(LOG_INTERRUPT, "arm7: HALT\n");
						set_halted(1, true);
						break;

					case 3:
						LOGMASKED(LOG_INTERRUPT, "arm7: SLEEP\n");
						set_halted(1, true);
						break;
				}
			}
			break;

		case POWCNT_OFFSET:
			if (ACCESSING_BITS_0_15)
			{
				m_powcnt[1] = (m_powcnt[1] & ~(mem_mask & 0x0003)) | (data & mem_mask & 0x0003);
			}
			break;

		default:
			LOGMASKED(LOG_UNK_WR, "[ARM7] [IO] Unknown write: %08x = %08x (%08x)\n", offset*4, data, mem_mask);
			break;
	}
}

/***************************************************************************
    ARM9 I/O
***************************************************************************/

uint32_t nds_state::arm9_io_r(offs_t offset, uint32_t mem_mask)
{
	bool handled = false;
	const uint32_t data = common_io_r(0, offset, mem_mask, handled);
	if (handled)
	{
		return data;
	}

	if ((offset >= DMAFILL_OFFSET) && (offset < (DMAFILL_OFFSET + 4)))
	{
		return m_dma_fill[offset - DMAFILL_OFFSET];
	}

	if (offset < ENGINE_A_END_OFFSET)
	{
		return m_ppu[0]->regs_r(offset);
	}

	if ((offset >= ENGINE_B_OFFSET) && (offset < ENGINE_B_END_OFFSET))
	{
		return m_ppu[1]->regs_r(offset - ENGINE_B_OFFSET);
	}

	switch (offset)
	{
		case DIVCNT_OFFSET:
			return m_divcnt;

		case DIV_NUMER_OFFSET:
			return uint32_t(m_div_numer);
		case DIV_NUMER_OFFSET+1:
			return uint32_t(m_div_numer >> 32);

		case DIV_DENOM_OFFSET:
			return uint32_t(m_div_denom);
		case DIV_DENOM_OFFSET+1:
			return uint32_t(m_div_denom >> 32);

		case DIV_RESULT_OFFSET:
			return uint32_t(m_div_result);
		case DIV_RESULT_OFFSET+1:
			return uint32_t(m_div_result >> 32);

		case DIVREM_RESULT_OFFSET:
			return uint32_t(m_divrem_result);
		case DIVREM_RESULT_OFFSET+1:
			return uint32_t(m_divrem_result >> 32);

		case SQRTCNT_OFFSET:
			return m_sqrtcnt;

		case SQRT_RESULT_OFFSET:
			return m_sqrt_result;

		case SQRT_PARAM_OFFSET:
			return uint32_t(m_sqrt_param);
		case SQRT_PARAM_OFFSET+1:
			return uint32_t(m_sqrt_param >> 32);

		case VRAMCNT_A_OFFSET:
			return m_vramcnta | (m_vramcntb << 8) | (m_vramcntc << 16) | (m_vramcntd << 24);

		case WRAMCNT_OFFSET:
			return m_vramcnte | (m_vramcntf << 8) | (m_vramcntg << 16) | (m_wramcnt << 24);

		case VRAMCNT_H_OFFSET:
			return m_vramcnth | (m_vramcnti << 8);

		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*  1     RAM
			*/
			return m_arm9_postflg;

		case POWCNT_OFFSET:
			return m_powcnt[0];

		default:
			LOGMASKED(LOG_UNK_RD, "[ARM9] [IO] Unknown read: %08x (%08x)\n", offset*4, mem_mask);
			break;
	}

	return 0;
}

void nds_state::arm9_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	bool handled = false;
	common_io_w(0, offset, data, mem_mask, handled);
	if (handled)
	{
		return;
	}

	if ((offset >= DMAFILL_OFFSET) && (offset < (DMAFILL_OFFSET + 4)))
	{
		COMBINE_DATA(&m_dma_fill[offset - DMAFILL_OFFSET]);
		return;
	}

	if (offset < ENGINE_A_END_OFFSET)
	{
		m_ppu[0]->regs_w(offset, data, mem_mask);
		return;
	}

	if ((offset >= ENGINE_B_OFFSET) && (offset < ENGINE_B_END_OFFSET))
	{
		m_ppu[1]->regs_w(offset - ENGINE_B_OFFSET, data, mem_mask);
		return;
	}

	switch (offset)
	{
		case DIVCNT_OFFSET:
			if (ACCESSING_BITS_0_15)
			{
				m_divcnt = (m_divcnt & ~(mem_mask & 0x0003)) | (data & mem_mask & 0x0003);
			}
			div_calculate();
			break;

		case DIV_NUMER_OFFSET:
			m_div_numer = (m_div_numer & ~uint64_t(mem_mask)) | (uint64_t(data) & mem_mask);
			div_calculate();
			break;
		case DIV_NUMER_OFFSET+1:
			m_div_numer = (m_div_numer & ~(uint64_t(mem_mask) << 32)) | ((uint64_t(data) & mem_mask) << 32);
			div_calculate();
			break;

		case DIV_DENOM_OFFSET:
			m_div_denom = (m_div_denom & ~uint64_t(mem_mask)) | (uint64_t(data) & mem_mask);
			div_calculate();
			break;
		case DIV_DENOM_OFFSET+1:
			m_div_denom = (m_div_denom & ~(uint64_t(mem_mask) << 32)) | ((uint64_t(data) & mem_mask) << 32);
			div_calculate();
			break;

		case SQRTCNT_OFFSET:
			if (ACCESSING_BITS_0_15)
			{
				m_sqrtcnt = (m_sqrtcnt & ~(mem_mask & 0x0001)) | (data & mem_mask & 0x0001);
			}
			sqrt_calculate();
			break;

		case SQRT_PARAM_OFFSET:
			m_sqrt_param = (m_sqrt_param & ~uint64_t(mem_mask)) | (uint64_t(data) & mem_mask);
			sqrt_calculate();
			break;
		case SQRT_PARAM_OFFSET+1:
			m_sqrt_param = (m_sqrt_param & ~(uint64_t(mem_mask) << 32)) | ((uint64_t(data) & mem_mask) << 32);
			sqrt_calculate();
			break;

		case VRAMCNT_A_OFFSET:
			if (ACCESSING_BITS_0_7) // VRAMCNT_A
			{
				m_vramcnta = data & 0xff;
			}
			if (ACCESSING_BITS_8_15) // VRAMCNT_B
			{
				m_vramcntb = (data >> 8) & 0xff;
			}
			if (ACCESSING_BITS_16_23) // VRAMCNT_C
			{
				m_vramcntc = (data >> 16) & 0xff;
			}
			if (ACCESSING_BITS_24_31) // VRAMCNT_D
			{
				m_vramcntd = (data >> 24) & 0xff;
			}
			update_vram_mapping();
			break;

		case WRAMCNT_OFFSET:
			if (ACCESSING_BITS_0_7) // VRAMCNT_E
			{
				m_vramcnte = data & 0xff;
			}
			if (ACCESSING_BITS_8_15) // VRAMCNT_F
			{
				m_vramcntf = (data >> 8) & 0xff;
			}
			if (ACCESSING_BITS_16_23) // VRAMCNT_G
			{
				m_vramcntg = (data >> 16) & 0xff;
			}
			if (ACCESSING_BITS_24_31) // WRAMCNT
			{
				m_wramcnt = (data>>24) & 0x3;
				m_arm7wrambnk->set_bank(m_wramcnt);
				m_arm9wrambnk->set_bank(m_wramcnt);
			}
			update_vram_mapping();
			break;

		case VRAMCNT_H_OFFSET:
			if (ACCESSING_BITS_0_7) // VRAMCNT_H
			{
				m_vramcnth = data & 0xff;
			}
			if (ACCESSING_BITS_8_15) // VRAMCNT_I
			{
				m_vramcnti = (data >> 8) & 0xff;
			}
			update_vram_mapping();
			break;

		case POSTFLG_OFFSET:
			/* Bit   Use
			*  0     0=Booting, 1=Booted (set by BIOS/firmware)
			*  1     RAM
			*/
			if (!(m_arm9_postflg & POSTFLG_PBF_MASK))
			{
				m_arm9_postflg &= ~POSTFLG_PBF_MASK;
				m_arm9_postflg |= data & POSTFLG_PBF_MASK;
			}
			m_arm9_postflg &= ~POSTFLG_RAM_MASK;
			m_arm9_postflg |= data & POSTFLG_RAM_MASK;
			break;

		case POWCNT_OFFSET:
			if (ACCESSING_BITS_0_15)
			{
				m_powcnt[0] = (m_powcnt[0] & ~(mem_mask & 0x820f)) | (data & mem_mask & 0x820f);
			}
			break;

		default:
			LOGMASKED(LOG_UNK_WR, "[ARM9] [IO] Unknown write: %08x = %08x (%08x)\n", offset*4, data, mem_mask);
			break;
	}
}

/***************************************************************************
    Address maps
***************************************************************************/

void nds_state::nds_arm7_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom().region("arm7", 0);
	map(0x02000000, 0x023fffff).ram().mirror(0x00400000).share("mainram");
	map(0x03000000, 0x0300ffff).mirror(0x007f0000).m(m_arm7wrambnk, FUNC(address_map_bank_device::amap32));
	map(0x03800000, 0x0380ffff).ram().mirror(0x007f0000).share("arm7ram");
	map(0x04000000, 0x0410ffff).rw(FUNC(nds_state::arm7_io_r), FUNC(nds_state::arm7_io_w));
	map(0x06000000, 0x0603ffff).mirror(0x00fc0000).rw(FUNC(nds_state::vram_region_r<VRAM_REGION_ARM7>), FUNC(nds_state::vram_region_w<VRAM_REGION_ARM7>));
	map(0x08000000, 0x09ffffff).r(FUNC(nds_state::gba_rom_r<1>)).nopw();
	map(0x0a000000, 0x0a00ffff).mirror(0x00ff0000).rw(FUNC(nds_state::gba_ram_r<1>), FUNC(nds_state::gba_ram_w<1>));
}

void nds_state::nds_arm9_map(address_map &map)
{
	map(0x02000000, 0x023fffff).ram().mirror(0x00400000).share("mainram");
	map(0x03000000, 0x03007fff).mirror(0x00ff8000).m("nds9wram", FUNC(address_map_bank_device::amap32));
	map(0x04000000, 0x0410ffff).rw(FUNC(nds_state::arm9_io_r), FUNC(nds_state::arm9_io_w));
	map(0x05000000, 0x050007ff).mirror(0x00fff800).ram().share("palette");
	map(0x06000000, 0x0607ffff).mirror(0x00180000).rw(FUNC(nds_state::vram_region_r<VRAM_REGION_BG_A>), FUNC(nds_state::vram_region_w<VRAM_REGION_BG_A>));
	map(0x06200000, 0x0621ffff).mirror(0x001e0000).rw(FUNC(nds_state::vram_region_r<VRAM_REGION_BG_B>), FUNC(nds_state::vram_region_w<VRAM_REGION_BG_B>));
	map(0x06400000, 0x0643ffff).mirror(0x001c0000).rw(FUNC(nds_state::vram_region_r<VRAM_REGION_OBJ_A>), FUNC(nds_state::vram_region_w<VRAM_REGION_OBJ_A>));
	map(0x06600000, 0x0661ffff).mirror(0x001e0000).rw(FUNC(nds_state::vram_region_r<VRAM_REGION_OBJ_B>), FUNC(nds_state::vram_region_w<VRAM_REGION_OBJ_B>));
	map(0x06800000, 0x068a3fff).rw(FUNC(nds_state::vram_lcdc_r), FUNC(nds_state::vram_lcdc_w));
	map(0x07000000, 0x070007ff).mirror(0x00fff800).ram().share("oam");
	map(0x08000000, 0x09ffffff).r(FUNC(nds_state::gba_rom_r<0>)).nopw();
	map(0x0a000000, 0x0a00ffff).mirror(0x00ff0000).rw(FUNC(nds_state::gba_ram_r<0>), FUNC(nds_state::gba_ram_w<0>));
	map(0xffff0000, 0xffff0fff).rom().mirror(0x1000).region("arm9", 0);
}

/***************************************************************************
    VRAM

	The VRAM mapping on this machine is crazy so we use an indirection table
	to track what pages are mapped where.
***************************************************************************/

void nds_state::map_vram_bank(int region, int page, int pages, uint32_t bankoff)
{
	for (int i = 0; i < pages; i++)
	{
		const int target = page + i;
		if ((target < 0) || (target >= VRAM_MAX_PAGES))
		{
			continue;
		}

		uint8_t &count = m_vram_page_count[region][target];
		if (count < 4)
		{
			m_vram_page_off[region][target][count++] = bankoff + (i * VRAM_PAGE_WORDS);
		}

		// the 2D engines fetch from the first bank mapped to a page
		static constexpr struct { int8_t engine, region; } engine_region[VRAM_REGION_COUNT] =
		{
			{ 0, gba_ppu_device::VRAM_BG },
			{ 0, gba_ppu_device::VRAM_OBJ },
			{ 1, gba_ppu_device::VRAM_BG },
			{ 1, gba_ppu_device::VRAM_OBJ },
			{ -1, 0 },
			{ 0, gba_ppu_device::VRAM_BG_EXTPAL },
			{ 0, gba_ppu_device::VRAM_OBJ_EXTPAL },
			{ 1, gba_ppu_device::VRAM_BG_EXTPAL },
			{ 1, gba_ppu_device::VRAM_OBJ_EXTPAL }
		};

		if ((count == 1) && (engine_region[region].engine >= 0))
		{
			m_ppu[engine_region[region].engine]->set_vram_page(engine_region[region].region, target, &m_vram[bankoff + (i * VRAM_PAGE_WORDS)]);
		}
	}
}

void nds_state::update_vram_mapping()
{
	static constexpr uint32_t bank_off[9] =
	{
		0x00000/4, 0x20000/4, 0x40000/4, 0x60000/4, 0x80000/4,
		0x90000/4, 0x94000/4, 0x98000/4, 0xa0000/4
	};

	std::memset(m_vram_page_count, 0, sizeof(m_vram_page_count));
	std::fill(std::begin(m_vram_bank_mapped), std::end(m_vram_bank_mapped), false);
	m_ppu[0]->unmap_vram();
	m_ppu[1]->unmap_vram();

	if (m_gba_mode)
	{
		// bank A holds the GBA's 96K of VRAM
		map_vram_bank(VRAM_REGION_BG_A, 0, 4, 0);
		map_vram_bank(VRAM_REGION_OBJ_A, 0, 2, 0x10000/4);
		return;
	}

	const uint8_t cnt[9] =
	{
		m_vramcnta, m_vramcntb, m_vramcntc, m_vramcntd, m_vramcnte,
		m_vramcntf, m_vramcntg, m_vramcnth, m_vramcnti
	};

	for (int bank = 0; bank < 9; bank++)
	{
		if (!BIT(cnt[bank], 7))
		{
			continue;
		}

		const int mst = cnt[bank] & ((bank <= 1) ? 3 : 7);
		const int ofs = (cnt[bank] >> 3) & 3;
		const uint32_t base = bank_off[bank];

		// F and G are 16K, and both their BG/OBJ offsets skip in a 4K/64K pattern
		const int fg_page = (ofs & 1) + ((ofs & 2) * 4);

		switch (bank)
		{
			case 0:     // A, 128K
			case 1:     // B, 128K
				switch (mst)
				{
					case 0: m_vram_bank_mapped[bank] = true; break;
					case 1: map_vram_bank(VRAM_REGION_BG_A, ofs * 8, 8, base); break;
					case 2: map_vram_bank(VRAM_REGION_OBJ_A, (ofs & 1) * 8, 8, base); break;
					case 3: break;      // 3D texture memory, not visible to the CPU
				}
				break;

			case 2:     // C, 128K
				switch (mst)
				{
					case 0: m_vram_bank_mapped[bank] = true; break;
					case 1: map_vram_bank(VRAM_REGION_BG_A, ofs * 8, 8, base); break;
					case 2: map_vram_bank(VRAM_REGION_ARM7, (ofs & 1) * 8, 8, base); break;
					case 3: break;      // 3D texture
					case 4: map_vram_bank(VRAM_REGION_BG_B, 0, 8, base); break;
				}
				break;

			case 3:     // D, 128K
				switch (mst)
				{
					case 0: m_vram_bank_mapped[bank] = true; break;
					case 1: map_vram_bank(VRAM_REGION_BG_A, ofs * 8, 8, base); break;
					case 2: map_vram_bank(VRAM_REGION_ARM7, (ofs & 1) * 8, 8, base); break;
					case 3: break;      // 3D texture
					case 4: map_vram_bank(VRAM_REGION_OBJ_B, 0, 8, base); break;
				}
				break;

			case 4:     // E, 64K
				switch (mst)
				{
					case 0: m_vram_bank_mapped[bank] = true; break;
					case 1: map_vram_bank(VRAM_REGION_BG_A, 0, 4, base); break;
					case 2: map_vram_bank(VRAM_REGION_OBJ_A, 0, 4, base); break;
					case 3: break;      // texture palette
					case 4: map_vram_bank(VRAM_REGION_BGEXTPAL_A, 0, 2, base); break;     // all four 8K slots
					default: break;
				}
				break;

			case 5:     // F, 16K
			case 6:     // G, 16K
				switch (mst)
				{
					case 0: m_vram_bank_mapped[bank] = true; break;
					case 1: map_vram_bank(VRAM_REGION_BG_A, fg_page, 1, base); break;
					case 2: map_vram_bank(VRAM_REGION_OBJ_A, fg_page, 1, base); break;
					case 3: break;      // texture palette
					case 4: map_vram_bank(VRAM_REGION_BGEXTPAL_A, ofs & 1, 1, base); break;    // slots 0-1 or 2-3
					case 5: map_vram_bank(VRAM_REGION_OBJEXTPAL_A, 0, 1, base); break;
					default: break;
				}
				break;

			case 7:     // H, 32K
				switch (mst)
				{
					case 0: m_vram_bank_mapped[bank] = true; break;
					case 1: map_vram_bank(VRAM_REGION_BG_B, 0, 2, base); break;
					case 2: map_vram_bank(VRAM_REGION_BGEXTPAL_B, 0, 2, base); break;
					default: break;
				}
				break;

			case 8:     // I, 16K
				switch (mst)
				{
					case 0: m_vram_bank_mapped[bank] = true; break;
					case 1: map_vram_bank(VRAM_REGION_BG_B, 2, 1, base); break;
					case 2: map_vram_bank(VRAM_REGION_OBJ_B, 0, 1, base); break;
					case 3: map_vram_bank(VRAM_REGION_OBJEXTPAL_B, 0, 1, base); break;
					default: break;
				}
				break;
		}
	}
}

uint32_t nds_state::vram_r(int region, offs_t offset)
{
	const int page = offset / VRAM_PAGE_WORDS;
	const uint32_t sub = offset % VRAM_PAGE_WORDS;

	uint32_t data = 0;
	for (int i = 0; i < m_vram_page_count[region][page]; i++)
	{
		data |= m_vram[m_vram_page_off[region][page][i] + sub];
	}

	return data;
}

void nds_state::vram_w(int region, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const int page = offset / VRAM_PAGE_WORDS;
	const uint32_t sub = offset % VRAM_PAGE_WORDS;

	for (int i = 0; i < m_vram_page_count[region][page]; i++)
	{
		COMBINE_DATA(&m_vram[m_vram_page_off[region][page][i] + sub]);
	}
}

// The LCDC window exposes the banks in their natural order, but only the ones
// that are enabled and allocated to LCDC.
uint32_t nds_state::vram_lcdc_r(offs_t offset)
{
	static constexpr uint32_t bank_end[9] =
	{
		0x20000/4, 0x40000/4, 0x60000/4, 0x80000/4, 0x90000/4,
		0x94000/4, 0x98000/4, 0xa0000/4, 0xa4000/4
	};

	for (int bank = 0; bank < 9; bank++)
	{
		if (offset < bank_end[bank])
		{
			return m_vram_bank_mapped[bank] ? m_vram[offset] : 0;
		}
	}

	return 0;
}

void nds_state::vram_lcdc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	static constexpr uint32_t bank_end[9] =
	{
		0x20000/4, 0x40000/4, 0x60000/4, 0x80000/4, 0x90000/4,
		0x94000/4, 0x98000/4, 0xa0000/4, 0xa4000/4
	};

	for (int bank = 0; bank < 9; bank++)
	{
		if (offset < bank_end[bank])
		{
			if (m_vram_bank_mapped[bank])
			{
				COMBINE_DATA(&m_vram[offset]);
			}
			return;
		}
	}
}

// ARM7 views of WRAM.  The window is 64K wide because with WRAMCNT=0 the ARM7
// sees a mirror of all of its own 64K of WRAM here.
void nds_state::nds7_wram_map(address_map &map)
{
	map(0x00000, 0x0ffff).rw(FUNC(nds_state::wram_arm7mirror_r), FUNC(nds_state::wram_arm7mirror_w));
	map(0x10000, 0x13fff).mirror(0x0c000).rw(FUNC(nds_state::wram_first_half_r), FUNC(nds_state::wram_first_half_w));
	map(0x20000, 0x23fff).mirror(0x0c000).rw(FUNC(nds_state::wram_second_half_r), FUNC(nds_state::wram_second_half_w));
	map(0x30000, 0x37fff).mirror(0x08000).rw(FUNC(nds_state::wram_first_half_r), FUNC(nds_state::wram_first_half_w));
}

// ARM9 views of WRAM
void nds_state::nds9_wram_map(address_map &map)
{
	map(0x00000, 0x07fff).rw(FUNC(nds_state::wram_first_half_r), FUNC(nds_state::wram_first_half_w));
	map(0x08000, 0x0bfff).mirror(0x04000).rw(FUNC(nds_state::wram_second_half_r), FUNC(nds_state::wram_second_half_w));
	map(0x10000, 0x13fff).mirror(0x04000).rw(FUNC(nds_state::wram_first_half_r), FUNC(nds_state::wram_first_half_w));
	map(0x18000, 0x1ffff).noprw().nopw();       // probably actually open bus?  GBATEK describes as "random"
}

// note that these all take 32-bit word offsets
uint32_t nds_state::wram_first_half_r(offs_t offset) { return m_WRAM[offset]; }
uint32_t nds_state::wram_second_half_r(offs_t offset) { return m_WRAM[offset+0x1000]; }
void nds_state::wram_first_half_w(offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_WRAM[offset]); }
void nds_state::wram_second_half_w(offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_WRAM[offset+0x1000]); }
uint32_t nds_state::wram_arm7mirror_r(offs_t offset) { return m_arm7ram[offset]; }
void nds_state::wram_arm7mirror_w(offs_t offset, uint32_t data, uint32_t mem_mask) { COMBINE_DATA(&m_arm7ram[offset]); }

/***************************************************************************
    Video

    The two 2D engines (video/gba_ppu.cpp) each render a line at a time into
    their own bitmap at the start of H-blank; POWCNT1 routes the bitmaps to
    the LCDs.
***************************************************************************/

void nds_state::video_start()
{
	for (int i = 0; i < 2; i++)
	{
		m_engine_bitmap[i].allocate(VISIBLE_DOTS, VISIBLE_LINES);
	}
	m_gba_work.allocate(VISIBLE_DOTS, VISIBLE_LINES);
}

void nds_state::draw_scanline(int line)
{
	uint16_t buf[gba_ppu_device::MAX_WIDTH];

	for (int engine = 0; engine < (m_gba_mode ? 1 : 2); engine++)
	{
		const int width = m_ppu[engine]->width();

		// in GBA mode the 240x160 picture sits in the middle of the LCD, and is assembled off screen
		uint32_t *const dest = m_gba_mode ? (&m_gba_work.pix(line + 16) + 8) : &m_engine_bitmap[engine].pix(line);

		// POWCNT1 bits 1 and 9 power the engines; one that is off shows white
		if (!BIT(m_powcnt[0], engine ? 9 : 1))
		{
			std::fill_n(dest, width, rgb_t::white());
			continue;
		}

		m_ppu[engine]->draw_scanline(line, buf);
		for (int x = 0; x < width; x++)
		{
			dest[x] = pal555(buf[x], 0, 5, 10);
		}
	}
}

uint32_t nds_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// POWCNT1 bit 0 powers both LCDs; bit 15 puts engine A on the top screen
	if (!BIT(m_powcnt[0], 0))
	{
		bitmap.fill(rgb_t::white(), cliprect);
		return 0;
	}

	const bool top = (&screen == m_screen[0].target());
	const int engine = (top == BIT(m_powcnt[0], 15)) ? 0 : 1;
	copybitmap(bitmap, m_engine_bitmap[engine], 0, 0, 0, 0, cliprect);
	return 0;
}

/***************************************************************************
    Machine
***************************************************************************/

static INPUT_PORTS_START( nds )
	PORT_START("KEYS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SELECT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("R")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("L")
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTKEYS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("X")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Y")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )   // DEBUG button, not fitted
	PORT_BIT( 0x0030, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Touch")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )  // hinge: 0 = open
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	// raw 12-bit touchscreen controller readings
	PORT_START("TOUCH_X")
	PORT_BIT( 0x0fff, 0x0800, IPT_LIGHTGUN_X ) PORT_MINMAX(0x0100, 0x0ed0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CROSSHAIR(X, 1.0, 0.0, 0)
	PORT_START("TOUCH_Y")
	PORT_BIT( 0x0fff, 0x0800, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x00b0, 0x0f20) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CROSSHAIR(Y, 1.0, 0.0, 0)
INPUT_PORTS_END

void nds_state::machine_reset()
{
	for (int i = 0; i < 2; i++)
	{
		m_gba_ldac[i]->write(0);
		m_gba_rdac[i]->write(0);
	}
	m_gba_fifo[0] = gba_fifo_t{};
	m_gba_fifo[1] = gba_fifo_t{};

	m_arm7_postflg = 0;
	m_arm9_postflg = 0;
	m_wramcnt = 0;
	m_arm7wrambnk->set_bank(0);
	m_arm9wrambnk->set_bank(0);
	m_halted[0] = m_halted[1] = false;

	m_vramcnta = m_vramcntb = m_vramcntc = m_vramcntd = m_vramcnte = 0;
	m_vramcntf = m_vramcntg = m_vramcnth = m_vramcnti = 0;
	update_vram_mapping();

	m_vcount = 0;
	m_dispstat[0] = m_dispstat[1] = 0;
	m_ime[0] = m_ime[1] = 0;
	m_ie[0] = m_ie[1] = 0;
	m_if[0] = m_if[1] = 0;
	m_ipcsync[0] = m_ipcsync[1] = 0;
	m_ipcfifocnt[0] = m_ipcfifocnt[1] = 0;
	m_ipcfifo_head[0] = m_ipcfifo_head[1] = 0;
	m_ipcfifo_count[0] = m_ipcfifo_count[1] = 0;
	m_keycnt[0] = m_keycnt[1] = 0;
	m_exmemcnt = 0x6000;
	m_powcnt[0] = 0x0001;
	m_powcnt[1] = 0x0001;

	m_spi_cnt = 0;
	m_spi_data = 0;
	m_fw_cmd = 0;
	m_fw_stat = 0;
	m_fw_powerdown = false;
	m_pm_have_index = false;
	std::fill(std::begin(m_pm_regs), std::end(m_pm_regs), 0);
	m_pm_regs[0] = 0x0d;    // sound amp on, both backlights on
	m_tsc_pos = 0;

	m_auxspicnt = 0;
	m_auxspidata = 0;
	m_romctrl = 0;
	m_cartdata_len = 0;

	std::fill(std::begin(m_dma_ctrl), std::end(m_dma_ctrl), 0);
	std::fill(std::begin(m_timer_regs), std::end(m_timer_regs), 0);

	for (int i = 0; i < 8; i++)
	{
		m_tmr_timer[i]->adjust(attotime::never);
	}

	// a reset after GBA mode puts the ARM7 back into DS mode
	if (m_gba_mode)
	{
		m_gba_mode = false;
		install_ds_arm7_map();
		m_arm7->set_unscaled_clock(MASTER_CLOCK);
		m_arm9->resume(SUSPEND_REASON_DISABLE);
		downcast<gba_ppu_nds_a_device &>(*m_ppu[0]).set_gba_mode(false);
		update_vram_mapping();
	}

	set_lcd_timing();
}

void nds_state::set_lcd_timing()
{
	attotime line, hblank;
	if (m_gba_mode)
	{
		// 308 dots of 4 cycles per line, 240 visible, 228 lines
		line = attotime::from_ticks(1232, GBA_CLOCK);
		hblank = attotime::from_ticks(960, GBA_CLOCK);
		m_total_lines = 228;
		m_visible_lines = 160;
	}
	else
	{
		line = attotime::from_ticks(TOTAL_DOTS, MASTER_CLOCK / 6);
		hblank = attotime::from_ticks(VISIBLE_DOTS, MASTER_CLOCK / 6);
		m_total_lines = TOTAL_LINES;
		m_visible_lines = VISIBLE_LINES;
	}
	m_vcount = 0;
	m_scanline_timer->adjust(line, 0, line);
	m_hblank_timer->adjust(hblank, 0, line);
}

/***************************************************************************
    GBA mode

    HALTCNT = 0x40 turns the DS into a GBA: the ARM9 stops, the ARM7 drops to
    16.78 MHz and restarts in the GBA BIOS with the GBA memory map, engine A
    takes GBA register semantics at 240x160 and the LCD runs GBA timing.
    Only a reset leaves it again.
***************************************************************************/

void nds_state::enter_gba_mode()
{
	if (m_gba_mode)
	{
		return;
	}

	if (!m_gbabios.found())
	{
		logerror("GBA mode requested, but there is no GBA BIOS\n");
		return;
	}

	LOGMASKED(LOG_INTERRUPT, "arm7: entering GBA mode\n");
	m_gba_mode = true;

	m_arm9->suspend(SUSPEND_REASON_DISABLE, true);

	install_gba_map();
	downcast<gba_ppu_nds_a_device &>(*m_ppu[0]).set_gba_mode(true);
	update_vram_mapping();
	m_arm7->set_unscaled_clock(GBA_CLOCK);
	set_lcd_timing();
	m_gba_work.fill(rgb_t::black());     // the border around the 240x160 picture
	m_gba_fifo[0] = gba_fifo_t{};
	m_gba_fifo[1] = gba_fifo_t{};
	m_ndssound->mute_all();              // the DS channels do not carry into GBA mode

	std::fill(std::begin(m_gba_soundregs), std::end(m_gba_soundregs), 0);
	m_gba_soundregs[(0x88 - 0x60)/4] = 0x200;       // SOUNDBIAS
	m_gba_waitcnt = 0;
	m_ime[1] = m_ie[1] = m_if[1] = 0;
	update_irqs(1);
	m_arm7_postflg = 0;

	// restart in the GBA BIOS once the current instruction is done
	m_arm7->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void nds_state::install_ds_arm7_map()
{
	address_space &space = m_arm7->space(AS_PROGRAM);
	space.unmap_readwrite(0x00000000, 0xffffffff);
	space.install_rom(0x00000000, 0x00003fff, memregion("arm7")->base());
	space.install_ram(0x02000000, 0x023fffff, 0x00400000, memshare("mainram")->ptr());
	space.install_readwrite_handler(0x03000000, 0x0300ffff, 0, 0x007f0000, 0,
			read32s_delegate(*m_arm7wrambnk, FUNC(address_map_bank_device::read32)),
			write32s_delegate(*m_arm7wrambnk, FUNC(address_map_bank_device::write32)));
	space.install_ram(0x03800000, 0x0380ffff, 0x007f0000, m_arm7ram.target());
	space.install_readwrite_handler(0x04000000, 0x0410ffff,
			read32s_delegate(*this, FUNC(nds_state::arm7_io_r)), write32s_delegate(*this, FUNC(nds_state::arm7_io_w)));
	space.install_readwrite_handler(0x06000000, 0x0603ffff, 0, 0x00fc0000, 0,
			read32sm_delegate(*this, FUNC(nds_state::vram_region_r<VRAM_REGION_ARM7>)),
			write32s_delegate(*this, FUNC(nds_state::vram_region_w<VRAM_REGION_ARM7>)));
	space.install_read_handler(0x08000000, 0x09ffffff, read32sm_delegate(*this, FUNC(nds_state::gba_rom_r<1>)));
	space.nop_write(0x08000000, 0x09ffffff);
	space.install_readwrite_handler(0x0a000000, 0x0a00ffff, 0, 0x00ff0000, 0,
			read32s_delegate(*this, FUNC(nds_state::gba_ram_r<1>)), write32s_delegate(*this, FUNC(nds_state::gba_ram_w<1>)));
}

void nds_state::install_gba_map()
{
	address_space &space = m_arm7->space(AS_PROGRAM);
	space.unmap_readwrite(0x00000000, 0xffffffff);

	space.install_read_handler(0x00000000, 0x00003fff, read32s_delegate(*this, FUNC(nds_state::gba_bios_r)));
	space.nop_write(0x00000000, 0x00003fff);
	space.install_read_handler(0x00004000, 0x01ffffff, read32s_delegate(*this, FUNC(nds_state::gba_open_bus_r)));
	space.install_read_handler(0x10000000, 0xffffffff, read32s_delegate(*this, FUNC(nds_state::gba_open_bus_r)));
	space.install_ram(0x02000000, 0x0203ffff, 0x00fc0000, memshare("mainram")->ptr());                  // EWRAM: 256K of main RAM
	space.install_ram(0x03000000, 0x03007fff, 0x00ff8000, m_arm7ram.target());                          // IWRAM: 32K of the ARM7's WRAM
	space.install_readwrite_handler(0x04000000, 0x04ffffff,
			read32s_delegate(*this, FUNC(nds_state::gba_io_r)), write32s_delegate(*this, FUNC(nds_state::gba_io_w)));
	space.install_ram(0x05000000, 0x050003ff, 0x00fffc00, m_palette.target());                          // engine A palette
	space.install_ram(0x06000000, 0x0600ffff, 0x00fe0000, &m_vram[0]);                                  // VRAM bank A, GBA layout
	space.install_ram(0x06010000, 0x06017fff, 0x00fe8000, &m_vram[0x10000/4]);                          // OBJ half, mirrored at +0x8000
	space.install_ram(0x07000000, 0x070003ff, 0x00fffc00, m_oam.target());                              // engine A OAM

	for (offs_t base = 0x08000000; base < 0x0e000000; base += 0x02000000)
	{
		space.install_read_handler(base, base + 0x01ffffff, read32sm_delegate(*this, FUNC(nds_state::gba_pak_r)));
	}

	if (m_gbacart->exists())
	{
		space.install_readwrite_handler(0x080000c4, 0x080000cb,
				read32s_delegate(*m_gbacart, FUNC(gba_cart_slot_device::read_gpio)), write32s_delegate(*m_gbacart, FUNC(gba_cart_slot_device::write_gpio)));

		const int type = m_gbacart->get_type();
		if (type == GBA_SRAM || type == GBA_DRILLDOZ || type == GBA_WARIOTWS)
		{
			space.install_readwrite_handler(0x0e000000, 0x0e01ffff,
					read32s_delegate(*m_gbacart, FUNC(gba_cart_slot_device::read_ram)), write32s_delegate(*m_gbacart, FUNC(gba_cart_slot_device::write_ram)));
		}
		if (type == GBA_EEPROM || type == GBA_EEPROM4 || type == GBA_EEPROM64 || type == GBA_BOKTAI || type == GBA_YOSHIUG)
		{
			space.install_readwrite_handler(0x0d000000, 0x0dffffff,
					read32s_delegate(*m_gbacart, FUNC(gba_cart_slot_device::read_ram)), write32s_delegate(*m_gbacart, FUNC(gba_cart_slot_device::write_ram)));
		}
		if (type == GBA_YOSHIUG)
		{
			space.install_readwrite_handler(0x0e008000, 0x0e0085ff,
					read32s_delegate(*m_gbacart, FUNC(gba_cart_slot_device::read_tilt)), write32sm_delegate(*m_gbacart, FUNC(gba_cart_slot_device::write_tilt)));
		}
		if (type == GBA_FLASH || type == GBA_FLASH512 || type == GBA_FLASH_RTC)
		{
			space.install_readwrite_handler(0x0e000000, 0x0e00ffff,
					read32s_delegate(*m_gbacart, FUNC(gba_cart_slot_device::read_ram)), write32s_delegate(*m_gbacart, FUNC(gba_cart_slot_device::write_ram)));
		}
		if (type == GBA_FLASH1M || type == GBA_FLASH1M_RTC)
		{
			space.install_readwrite_handler(0x0e000000, 0x0e01ffff,
					read32s_delegate(*m_gbacart, FUNC(gba_cart_slot_device::read_ram)), write32s_delegate(*m_gbacart, FUNC(gba_cart_slot_device::write_ram)));
		}
		if (type == GBA_3DMATRIX)
		{
			space.install_write_handler(0x08800000, 0x088001ff, write32sm_delegate(*m_gbacart, FUNC(gba_cart_slot_device::write_mapper)));
		}
	}
}

uint32_t nds_state::gba_pak_r(offs_t offset)
{
	return m_gbacart->read_rom(offset);
}

uint32_t nds_state::gba_bios_r(offs_t offset, uint32_t mem_mask)
{
	const uint32_t pc = m_arm7->pc();
	if (pc >= 0x4000)
	{
		// The BIOS can only be read while executing from it; other reads return the most recently
		// fetched BIOS opcode as per GBATEK.
		return m_gba_bios_prefetch;
	}

	const uint32_t data = get_u32le(&m_gbabios[offset << 2]);

	// The core prefetches 2 words ahead of the PC like the real pipeline, so PC+8
	if (((offset << 2) == ((pc & ~3) + 8)) && !machine().side_effects_disabled())
	{
		m_gba_bios_prefetch = data;
	}

	return data;
}

// Reads from unused address space return the most recently prefetched opcode.
// Justice League Chronicles and The Pinball of the Dead both do null pointer
// reads and work by accident.
uint32_t nds_state::gba_open_bus_r(offs_t offset, uint32_t mem_mask)
{
	address_space &mspace = m_arm7->space(AS_PROGRAM);
	const uint32_t pc = m_arm7->pc();

	// don't recurse if we're somehow executing from unused space
	if ((pc >= 0x10000000) || ((pc >= 0x4000) && (pc < 0x02000000)))
	{
		return 0;
	}

	uint32_t data;
	if (BIT(m_arm7->state_int(arm7_cpu_device::ARM7_CPSR), 5))    // Thumb
	{
		// Thumb: the two halves depend on where the code is and how it's aligned
		switch (pc >> 24)
		{
			case 0x00: // BIOS
			case 0x03: // IWRAM
			case 0x07: // OAM
				if (pc & 2)
				{
					data = mspace.read_word(pc + 2) | (mspace.read_word(pc + 4) << 16);
				}
				else
				{
					data = mspace.read_word(pc + 4) | (mspace.read_word(pc + 6) << 16);
				}
				break;

			default: // EWRAM, palette, VRAM, cartridge ROM
				data = mspace.read_word(pc + 4);
				data |= data << 16;
				break;
		}
	}
	else
	{
		data = mspace.read_dword(pc + 8);
	}
	logerror("%s: unmapped program memory read = %08X & %08X\n", machine().describe_context(), data, mem_mask);
	return data;
}

// The GBA I/O map: LCD registers are engine A's, DMA/timers/keys/serial are
// the ARM7's own, IE/IF are 16-bit at 0x200 and HALTCNT means halt on any write.
/***************************************************************************
    GBA mode sound

    The four PSG channels and wave RAM are the AGB_APU (the same device the
    GBA driver uses); the two DirectSound channels are FIFOs fed by DMA and
    drained one byte at a time to a pair of DACs on each timer 0/1 overflow.
***************************************************************************/

uint32_t nds_state::gba_sound_r(offs_t offset)
{
	switch (offset)
	{
		case 0x60/4: return m_gbsound->sound_r(0) | (m_gbsound->sound_r(1) << 16) | (m_gbsound->sound_r(2) << 24);
		case 0x64/4: return m_gbsound->sound_r(3) | (m_gbsound->sound_r(4) << 8);
		case 0x68/4: return m_gbsound->sound_r(6) | (m_gbsound->sound_r(7) << 8);
		case 0x6c/4: return m_gbsound->sound_r(8) | (m_gbsound->sound_r(9) << 8);
		case 0x70/4: return m_gbsound->sound_r(0xa) | (m_gbsound->sound_r(0xb) << 16) | (m_gbsound->sound_r(0xc) << 24);
		case 0x74/4: return m_gbsound->sound_r(0xd) | (m_gbsound->sound_r(0xe) << 8);
		case 0x78/4: return m_gbsound->sound_r(0x10) | (m_gbsound->sound_r(0x11) << 8);
		case 0x7c/4: return m_gbsound->sound_r(0x12) | (m_gbsound->sound_r(0x13) << 8);
		case 0x80/4: return m_gbsound->sound_r(0x14) | (m_gbsound->sound_r(0x15) << 8) | (m_gba_soundregs[(0x80 - 0x60)/4] & 0xffff0000);
		case 0x84/4: return m_gbsound->sound_r(0x16);
	}

	if ((offset >= (0x90/4)) && (offset < (0xa0/4)))
	{
		const int base = (offset - (0x90/4)) * 4;
		return m_gbsound->wave_r(base) | (m_gbsound->wave_r(base + 1) << 8) | (m_gbsound->wave_r(base + 2) << 16) | (m_gbsound->wave_r(base + 3) << 24);
	}

	return m_gba_soundregs[offset - (0x60/4)];
}

void nds_state::gba_sound_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	static const double dac_gain[2] = { 0.5, 1.0 };
	static const double psg_gain[4] = { 0.25, 0.5, 1.0, 1.0 };

	const uint16_t old_x = m_gba_soundregs[(0x84 - 0x60)/4] & 0xffff;
	COMBINE_DATA(&m_gba_soundregs[offset - (0x60/4)]);

	auto reset_fifo = [this] (int ref)
	{
		m_gba_fifo[ref] = gba_fifo_t{};
		m_gba_ldac[ref]->write(0);
		m_gba_rdac[ref]->write(0);
	};

	switch (offset)
	{
		case 0x60/4:
			if (ACCESSING_BITS_0_7)
			{
				m_gbsound->sound_w(0, data);
			}
			if (ACCESSING_BITS_16_23)
			{
				m_gbsound->sound_w(1, data >> 16);
			}
			if (ACCESSING_BITS_24_31)
			{
				m_gbsound->sound_w(2, data >> 24);
			}
			break;

		case 0x64/4:
			if (ACCESSING_BITS_0_7)
			{
				m_gbsound->sound_w(3, data);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_gbsound->sound_w(4, data >> 8);
			}
			break;

		case 0x68/4:
			if (ACCESSING_BITS_0_7)
			{
				m_gbsound->sound_w(6, data);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_gbsound->sound_w(7, data >> 8);
			}
			break;

		case 0x6c/4:
			if (ACCESSING_BITS_0_7)
			{
				m_gbsound->sound_w(8, data);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_gbsound->sound_w(9, data >> 8);
			}
			break;

		case 0x70/4:
			if (ACCESSING_BITS_0_7)
			{
				m_gbsound->sound_w(0xa, data);
			}
			if (ACCESSING_BITS_16_23)
			{
				m_gbsound->sound_w(0xb, data >> 16);
			}
			if (ACCESSING_BITS_24_31)
			{
				m_gbsound->sound_w(0xc, data >> 24);
			}
			break;

		case 0x74/4:
			if (ACCESSING_BITS_0_7)
			{
				m_gbsound->sound_w(0xd, data);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_gbsound->sound_w(0xe, data >> 8);
			}
			break;

		case 0x78/4:
			if (ACCESSING_BITS_0_7)
			{
				m_gbsound->sound_w(0x10, data);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_gbsound->sound_w(0x11, data >> 8);
			}
			break;

		case 0x7c/4:
			if (ACCESSING_BITS_0_7)
			{
				m_gbsound->sound_w(0x12, data);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_gbsound->sound_w(0x13, data >> 8);
			}
			break;

		case 0x80/4:
			if (ACCESSING_BITS_0_7)
			{
				m_gbsound->sound_w(0x14, data);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_gbsound->sound_w(0x15, data >> 8);
			}
			if (ACCESSING_BITS_16_31)
			{
				m_gbsound->set_output_gain(ALL_OUTPUTS, psg_gain[(data >> 16) & 3]);
				m_gba_ldac[0]->set_output_gain(ALL_OUTPUTS, dac_gain[BIT(data, 18)]);
				m_gba_rdac[0]->set_output_gain(ALL_OUTPUTS, dac_gain[BIT(data, 18)]);
				m_gba_ldac[1]->set_output_gain(ALL_OUTPUTS, dac_gain[BIT(data, 19)]);
				m_gba_rdac[1]->set_output_gain(ALL_OUTPUTS, dac_gain[BIT(data, 19)]);
				if (BIT(data, 27))
				{
					reset_fifo(0);
				}
				if (BIT(data, 31))
				{
					reset_fifo(1);
				}
			}
			break;

		case 0x84/4:
			if (ACCESSING_BITS_0_7)
			{
				m_gbsound->sound_w(0x16, data);
				if (BIT(data, 7) && !BIT(old_x, 7))
				{
					reset_fifo(0);
					reset_fifo(1);
				}
			}
			break;

		case 0x90/4:
		case 0x94/4:
		case 0x98/4:
		case 0x9c/4:
		{
			const int base = (offset - (0x90/4)) * 4;
			if (ACCESSING_BITS_0_7)
			{
				m_gbsound->wave_w(base + 0, data);
			}
			if (ACCESSING_BITS_8_15)
			{
				m_gbsound->wave_w(base + 1, data >> 8);
			}
			if (ACCESSING_BITS_16_23)
			{
				m_gbsound->wave_w(base + 2, data >> 16);
			}
			if (ACCESSING_BITS_24_31)
			{
				m_gbsound->wave_w(base + 3, data >> 24);
			}
			break;
		}

		case 0xa0/4:
		case 0xa4/4:
		{
			gba_fifo_t &fifo = m_gba_fifo[offset & 1];
			if (fifo.size < 8)
			{
				COMBINE_DATA(&fifo.word[fifo.in]);
				fifo.in = (fifo.in + 1) & 7;
				fifo.size++;
			}
			break;
		}
	}
}

// one timer-0/1 overflow feeds one byte to a DirectSound channel; the FIFO
// is refilled by its DMA when it runs half empty
void nds_state::gba_audio_tick(int ref)
{
	if (!BIT(m_gba_soundregs[(0x84 - 0x60)/4], 7))       // SOUNDCNT_X master enable
	{
		return;
	}

	gba_fifo_t &fifo = m_gba_fifo[ref];
	const uint16_t cnt_h = m_gba_soundregs[(0x80 - 0x60)/4] >> 16;

	if ((fifo.size > 0) && (fifo.remains == 0))
	{
		fifo.sample = fifo.word[fifo.ptr];
		fifo.ptr = (fifo.ptr + 1) & 7;
		fifo.remains = 4;
		fifo.size--;
	}

	const int lbit = ref ? 13 : 9;
	const int rbit = ref ? 12 : 8;
	const uint8_t out = ((fifo.size == 0) && (fifo.remains == 0)) ? 0 : uint8_t(fifo.sample);
	if (BIT(cnt_h, lbit))
	{
		m_gba_ldac[ref]->write(out);
	}
	if (BIT(cnt_h, rbit))
	{
		m_gba_rdac[ref]->write(out);
	}

	if (fifo.size <= 4)
	{
		// GBA DMA 1 and 2 feed the FIFOs; those are ARM7 channels 5 and 6
		const uint32_t fifo_addr = ref ? 0x040000a4 : 0x040000a0;
		for (int ch = 5; ch <= 6; ch++)
		{
			if (BIT(m_dma_ctrl[ch], 31) && (((m_dma_ctrl[ch] >> 28) & 3) == 3) && (m_dma_dstreg[ch] == fifo_addr))
			{
				dma_exec(ch);
			}
		}
	}

	if (fifo.remains > 0)
	{
		fifo.sample >>= 8;
		fifo.remains--;
	}
}

uint32_t nds_state::gba_io_r(offs_t offset, uint32_t mem_mask)
{
	if ((offset & 0x3fff) == (0x800/4))
	{
		return 0x0d000020;      // internal memory control
	}
	if (offset >= 0x400)
	{
		return 0;
	}

	bool handled = false;
	if (offset < GBA_LCD_END_OFFSET)
	{
		if (offset == DISPSTAT_OFFSET)
		{
			return common_io_r(1, offset, mem_mask, handled);
		}
		return m_ppu[0]->regs_r(offset);
	}
	if ((offset >= (0x60/4)) && (offset < (0xb0/4)))
	{
		return gba_sound_r(offset);
	}
	if (((offset >= DMA_OFFSET) && (offset < (0xe0/4))) || ((offset >= TIMER_OFFSET) && (offset < (0x110/4)))
		|| ((offset >= SIO_OFFSET) && (offset < (0x130/4))) || (offset == KEYINPUT_OFFSET) || (offset == RCNT_OFFSET) || (offset == IME_OFFSET))
	{
		const uint32_t data = common_io_r(1, offset, mem_mask, handled);
		return handled ? data : 0;
	}
	if (offset == (0x200/4))
	{
		return (m_ie[1] & 0xffff) | ((m_if[1] & 0xffff) << 16);
	}
	if (offset == (0x204/4))
	{
		return m_gba_waitcnt;
	}
	if (offset == POSTFLG_OFFSET)
	{
		return m_arm7_postflg;
	}
	return 0;
}

void nds_state::gba_io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset >= 0x400)
	{
		return;
	}

	bool handled = false;
	if (offset < GBA_LCD_END_OFFSET)
	{
		if (offset == DISPSTAT_OFFSET)
		{
			common_io_w(1, offset, data, mem_mask, handled);
			return;
		}
		m_ppu[0]->regs_w(offset, data, mem_mask);
		return;
	}
	if ((offset >= (0x60/4)) && (offset < (0xb0/4)))
	{
		gba_sound_w(offset, data, mem_mask);
		return;
	}
	if (((offset >= DMA_OFFSET) && (offset < (0xe0/4))) || ((offset >= TIMER_OFFSET) && (offset < (0x110/4)))
		|| ((offset >= SIO_OFFSET) && (offset < (0x130/4))) || (offset == KEYINPUT_OFFSET) || (offset == RCNT_OFFSET) || (offset == IME_OFFSET))
	{
		common_io_w(1, offset, data, mem_mask, handled);
		return;
	}
	switch (offset)
	{
		case 0x200/4:   // IE / IF
			if (ACCESSING_BITS_0_15)
			{
				m_ie[1] = (m_ie[1] & 0xffff0000) | (data & 0xffff);
			}
			if (ACCESSING_BITS_16_31)
			{
				m_if[1] &= ~(data >> 16);
			}
			update_irqs(1);
			break;

		case 0x204/4:   // WAITCNT
			if (ACCESSING_BITS_0_15)
			{
				m_gba_waitcnt = data & 0xffff;
			}
			break;

		case POSTFLG_OFFSET:
			if (ACCESSING_BITS_0_7)
			{
				m_arm7_postflg = data & 0xff;
			}
			if (ACCESSING_BITS_8_15)
			{
				set_halted(1, true);        // HALTCNT: 0x00 halt, 0x80 stop
			}
			break;
	}
}


void nds_state::machine_start()
{
	for (int i = 0; i < 8; i++)
	{
		m_dma_timer[i] = timer_alloc(FUNC(nds_state::dma_complete), this);
		m_dma_timer[i]->adjust(attotime::never, i);

		m_tmr_timer[i] = timer_alloc(FUNC(nds_state::timer_expire), this);
		m_tmr_timer[i]->adjust(attotime::never, i);
	}

	m_scanline_timer = timer_alloc(FUNC(nds_state::scanline_tick), this);
	m_hblank_timer = timer_alloc(FUNC(nds_state::hblank_tick), this);

	save_item(NAME(m_gba_mode));
	save_item(NAME(m_total_lines));
	save_item(NAME(m_visible_lines));
	save_item(NAME(m_gba_soundregs));
	save_item(STRUCT_MEMBER(m_gba_fifo, ptr));
	save_item(STRUCT_MEMBER(m_gba_fifo, in));
	save_item(STRUCT_MEMBER(m_gba_fifo, size));
	save_item(STRUCT_MEMBER(m_gba_fifo, remains));
	save_item(STRUCT_MEMBER(m_gba_fifo, sample));
	save_item(STRUCT_MEMBER(m_gba_fifo, word));
	save_item(NAME(m_gba_waitcnt));
	save_item(NAME(m_gba_bios_prefetch));

	// the card decrypts KEY1 commands with the same Blowfish tables the BIOS encrypts them with
	m_ndscart->set_key1_table(memregion("arm7")->base() + 0x30);

	// the sound channels fetch their samples from the ARM7 bus
	m_ndssound->set_hostspace(m_arm7->space(AS_PROGRAM));

	// the touch screen is the bottom one
	machine().crosshair().get_crosshair(0).set_screen(m_screen[1]);

	// the firmware serial flash is writable, so run from a RAM copy of the dump
	std::copy_n(&m_firmware[0], 0x40000, &m_fw_ram[0]);

	// the 2D engines fetch straight from the palette, OAM and the VRAM pages
	// mapped to them, so the page tables are rebuilt after a state load
	for (int engine = 0; engine < 2; engine++)
	{
		m_ppu[engine]->set_palette_ram(&m_palette[engine * 0x100]);
		m_ppu[engine]->set_oam(&m_oam[engine * 0x100]);
	}
	m_ppu[0]->set_display_vram(&m_vram[0]);
	machine().save().register_postload(save_prepost_delegate(FUNC(nds_state::update_vram_mapping), this));

	save_pointer(NAME(m_fw_ram), 0x40000);
	save_pointer(NAME(m_vram), VRAM_WORDS);
	save_item(NAME(m_vram_page_off));
	save_item(NAME(m_vram_page_count));
	save_item(NAME(m_vram_bank_mapped));
	save_item(NAME(m_arm7_postflg));
	save_item(NAME(m_arm9_postflg));
	save_item(NAME(m_ime));
	save_item(NAME(m_ie));
	save_item(NAME(m_if));
	save_item(NAME(m_ipcsync));
	save_item(NAME(m_WRAM));
	save_item(NAME(m_wramcnt));
	save_item(NAME(m_vramcnta));
	save_item(NAME(m_vramcntb));
	save_item(NAME(m_vramcntc));
	save_item(NAME(m_vramcntd));
	save_item(NAME(m_vramcnte));
	save_item(NAME(m_vramcntf));
	save_item(NAME(m_vramcntg));
	save_item(NAME(m_vramcnth));
	save_item(NAME(m_vramcnti));
	save_item(NAME(m_exmemcnt));
	save_item(NAME(m_powcnt));
	save_item(NAME(m_biosprot));
	save_item(NAME(m_halted));
	save_item(NAME(m_sioregs));
	save_item(NAME(m_rcnt));
	save_item(NAME(m_card_seed));
	save_item(NAME(m_dispstat));
	save_item(NAME(m_vcount));
	save_item(NAME(m_keycnt));
	save_item(NAME(m_ipcfifocnt));
	save_item(NAME(m_ipcfifo));
	save_item(NAME(m_ipcfifo_last));
	save_item(NAME(m_ipcfifo_head));
	save_item(NAME(m_ipcfifo_count));
	save_item(NAME(m_spi_cnt));
	save_item(NAME(m_spi_data));
	save_item(NAME(m_fw_cmd));
	save_item(NAME(m_fw_stat));
	save_item(NAME(m_fw_addr));
	save_item(NAME(m_fw_bytes));
	save_item(NAME(m_fw_powerdown));
	save_item(NAME(m_pm_regs));
	save_item(NAME(m_pm_index));
	save_item(NAME(m_pm_have_index));
	save_item(NAME(m_tsc_result));
	save_item(NAME(m_tsc_pos));
	save_item(NAME(m_rtc_io));
	save_item(NAME(m_auxspicnt));
	save_item(NAME(m_auxspidata));
	save_item(NAME(m_romctrl));
	save_item(NAME(m_card_command));
	save_item(NAME(m_cartdata_len));
	save_item(NAME(m_card_cpu));
	save_item(NAME(m_dma_srcreg));
	save_item(NAME(m_dma_dstreg));
	save_item(NAME(m_dma_ctrl));
	save_item(NAME(m_dma_src));
	save_item(NAME(m_dma_dst));
	save_item(NAME(m_dma_cnt));
	save_item(NAME(m_dma_fill));
	save_item(NAME(m_timer_regs));
	save_item(NAME(m_timer_reload));
	save_item(NAME(m_timer_start));
	save_item(NAME(m_divcnt));
	save_item(NAME(m_sqrtcnt));
	save_item(NAME(m_div_numer));
	save_item(NAME(m_div_denom));
	save_item(NAME(m_div_result));
	save_item(NAME(m_divrem_result));
	save_item(NAME(m_sqrt_param));
	save_item(NAME(m_sqrt_result));
}

static void nds_cart(device_slot_interface &device)
{
	device.option_add_internal("nds_rom", NDS_ROM_STD);
}

static void gba_cart(device_slot_interface &device)
{
	device.option_add_internal("gba_rom",          GBA_ROM_STD);
	device.option_add_internal("gba_sram",         GBA_ROM_SRAM);
	device.option_add_internal("gba_drilldoz",     GBA_ROM_DRILLDOZ);
	device.option_add_internal("gba_wariotws",     GBA_ROM_WARIOTWS);
	device.option_add_internal("gba_eeprom",       GBA_ROM_EEPROM);
	device.option_add_internal("gba_eeprom_4k",    GBA_ROM_EEPROM);
	device.option_add_internal("gba_yoshiug",      GBA_ROM_YOSHIUG);
	device.option_add_internal("gba_eeprom_64k",   GBA_ROM_EEPROM64);
	device.option_add_internal("gba_boktai",       GBA_ROM_BOKTAI);
	device.option_add_internal("gba_flash",        GBA_ROM_FLASH);
	device.option_add_internal("gba_flash_rtc",    GBA_ROM_FLASH_RTC);
	device.option_add_internal("gba_flash_512",    GBA_ROM_FLASH);
	device.option_add_internal("gba_flash_1m",     GBA_ROM_FLASH1M);
	device.option_add_internal("gba_flash_1m_rtc", GBA_ROM_FLASH1M_RTC);
	device.option_add_internal("gba_3dmatrix",     GBA_ROM_3DMATRIX);
}

void nds_state::nds(machine_config &config)
{
	ARM7(config, m_arm7, MASTER_CLOCK);
	m_arm7->set_addrmap(AS_PROGRAM, &nds_state::nds_arm7_map);

	ARM946ES(config, m_arm9, MASTER_CLOCK*2);
	m_arm9->set_high_vectors();
	m_arm9->set_addrmap(AS_PROGRAM, &nds_state::nds_arm9_map);

	// the two CPUs communicate through shared RAM and the IPC registers,
	// so they need to be kept in lockstep
	config.set_perfect_quantum(m_arm9);

	// WRAM
	ADDRESS_MAP_BANK(config, "nds7wram").set_map(&nds_state::nds7_wram_map).set_options(ENDIANNESS_LITTLE, 32, 32, 0x10000);
	ADDRESS_MAP_BANK(config, "nds9wram").set_map(&nds_state::nds9_wram_map).set_options(ENDIANNESS_LITTLE, 32, 32, 0x8000);

	// 2D graphics engines
	NDS_PPU_A(config, m_ppu[0], 0);
	NDS_PPU_B(config, m_ppu[1], 0);

	for (int i = 0; i < 2; i++)
	{
		SCREEN(config, m_screen[i]);
		m_screen[i]->set_lcd();
		m_screen[i]->set_raw(MASTER_CLOCK / 6, TOTAL_DOTS, 0, VISIBLE_DOTS, TOTAL_LINES, 0, VISIBLE_LINES);
		m_screen[i]->set_screen_update(FUNC(nds_state::screen_update));
	}

	config.set_default_layout(layout_dualhovu);

	// battery-backed clock
	S35180(config, m_rtc, 32'768);

	// DS native sound: 16 channels mixed to a stereo speaker
	SPEAKER(config, "speaker", 2).front();
	NDS_SOUND(config, m_ndssound, 0);
	m_ndssound->add_route(0, "speaker", 1.0, 0);
	m_ndssound->add_route(1, "speaker", 1.0, 1);

	// GBA-mode sound: the AGB APU (4 PSG channels + wave) and the two
	// DirectSound FIFO channels' DACs
	AGB_APU(config, m_gbsound, 4.194304_MHz_XTAL);
	m_gbsound->add_route(0, "speaker", 0.5, 0);
	m_gbsound->add_route(1, "speaker", 0.5, 1);
	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_gba_ldac[0], 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_gba_rdac[0], 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 1);
	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_gba_ldac[1], 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 0);
	DAC_8BIT_R2R_TWOS_COMPLEMENT(config, m_gba_rdac[1], 0).add_route(ALL_OUTPUTS, "speaker", 0.5, 1);

	// game card slot
	NDS_CART_SLOT(config, m_ndscart, nds_cart, nullptr);

	// GBA slot: GBA mode is not emulated, so this is for the firmware's "Game Pak" panel and DS expansion paks
	GBA_CART_SLOT(config, m_gbacart, gba_cart, nullptr);
	SOFTWARE_LIST(config, "gba_list").set_compatible("gba");
}

// Help identifying the region and revisions of the main set would be greatly appreciated!
ROM_START( nds )
	ROM_REGION( 0x1000, "arm9", 0 )
	ROM_LOAD( "biosnds9.rom", 0x0000, 0x1000, CRC(2ab23573) SHA1(bfaac75f101c135e32e2aaf541de6b1be4c8c62d) )

	ROM_REGION( 0x4000, "arm7", 0 )
	ROM_LOAD( "biosnds7.rom", 0x0000, 0x4000, CRC(1280f0d5) SHA1(24f67bdea115a2c847c8813a262502ee1607b7df) )

	// the GBA BIOS is the same as in the GBA; the DS runs it in GBA mode
	ROM_REGION( 0x4000, "gbabios", 0 )
	ROM_LOAD( "gba.bin", 0x0000, 0x4000, CRC(81977335) SHA1(300c20df6731a33952ded8c436f7f186d25d3492) )

	ROM_REGION( 0x40000, "firmware", 0 )
	ROM_SYSTEM_BIOS( 0, "nds", "Nintendo DS" )
	ROMX_LOAD( "firmware.bin", 0x0000, 0x40000, CRC(945f9dc9) SHA1(cfe072921ee3fb93f688743f8beef89043c3e9ad), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "40820d", "Nintendo DS ver.40820D prototype" ) // from X4 prototype unit
	ROMX_LOAD( "fw0802d6.bin", 0x0000, 0x40000, CRC(18e137df) SHA1(d51be561a6538941f8f43d6db9cdb964a383080a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "040615", "Nintendo DS ver.040615 prototype" )
	ROMX_LOAD( "fw64b19d.bin", 0x0000, 0x40000, CRC(93487f12) SHA1(3af896a05736cc0385d0f858b431ff719164caf6), ROM_BIOS(2) )
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY     FULLNAME  FLAGS
CONS( 2004, nds,  0,      0,      nds,     nds,   nds_state, empty_init, "Nintendo", "DS",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
