// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
/***************************************************************************

    Nokia N-Gage (NEM-4)

    The 4.03 ROM set is assembled from Nokia service files nem440.3c1 and
    nem440.326, with the PPM ranges overlaid. The images are in chip-select
    order and preserve the scrambled NOS bytes and erased writable areas.

    UPP_WD2 behavior and NOR commands are based on the NEM-4 FPGA study.
    The ASIC ROM is undumped. Only the observed startup call at 0x03de
    has a return stub. CBUS and flash programming use transaction-level
    models. The UPP DSP provides MDI version replies and signed mono PCM. D550 models
    audio DSP startup and diagnostics over EUART1. The UPP LCD DMA path
    supports packed writes to the panel. SIMIF can use a synthetic GSM test
    card. The native MMC path models the host and card at transaction level.

***************************************************************************/

#include "emu.h"

#include "ngage_adsp.h"
#include "ngage_bt.h"
#include "ngage_mmc.h"
#include "ngage_pcm.h"
#include "ngage_rtc.h"
#include "ngage_sim.h"

#include "cpu/arm7/arm7.h"
#include "machine/nvram.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "multibyte.h"

#include <algorithm>
#include <bit>
#include <bitset>
#include <span>

namespace {

constexpr u32 NOR_SIZE = 0x800000;
constexpr u32 PMM_SIZE = 0x10000;
constexpr u32 CDRIVE_SIZE = 0x400000;
constexpr u32 UPP_BASE = 0x0c100000;
constexpr u32 UPP_WORDS = 0x100000 / 4;
constexpr u32 ARM_SDRAM_CLOCK = 104'000'000;
constexpr u32 GENERAL_TIMER_CLOCK = 26'000'000;
constexpr u32 CTSI_FAST_CLOCK = 32'768;
constexpr u32 CTSI_NOS_CLOCK = 64;
constexpr u32 CTSI_EVENT_MASK = 0x3680;
constexpr u32 CTSI_SOFTWARE_EVENTS = 0x3000;
constexpr u32 KEYPAD_EVENTS = 0x00ee0004;
constexpr u32 BLUETOOTH_EVENT = 0x00100000;
constexpr u32 GENIO_ENABLE_MASK = ((KEYPAD_EVENTS | BLUETOOTH_EVENT) >> 16) * 0x10001U;
constexpr u8 BLUETOOTH_CBUS_DEVICE = 5;
constexpr u16 UEM_ADC_DEFAULT = 512;
constexpr ioport_value BATTERY_LEVEL_MASK = 0x70;
// UEM channel 12 samples that produce seven through one bars in NEM-4 4.03.
constexpr u16 BATTERY_VOLTAGE_ADC[] = { UEM_ADC_DEFAULT, 470, 465, 460, 450, 440, 430 };
constexpr unsigned MDI_RING_WORDS = 250;
constexpr u32 MDI_TX_POINTERS = 0x3e8 / 4;
constexpr u32 MDI_RX_POINTERS = 0x3ec / 4;
constexpr u32 MDI_CONTROL = 0x4b004 / 4;
constexpr unsigned LCD_WIDTH = 176;
constexpr unsigned LCD_HEIGHT = 208;
constexpr unsigned LCD_ROW_PAIRS = LCD_WIDTH / 2;
constexpr unsigned LCD_PAIRS = LCD_ROW_PAIRS * LCD_HEIGHT;

// UPP flash read-side equations from Andrea Bogazzi's ngage_descramble.v.
u16 descramble_nos(u16 cipher, u32 address)
{
	struct equation
	{
		u16 cipher_mask;
		u32 address_mask;
		u8 a, b, c, d;
		bool invert;
	};
	static constexpr equation equations[] = {
		{ 0x0101, 0x020102,  4, 10,  8, 17, false },
		{ 0x0202, 0x000000,  3, 13,  9, 17, false },
		{ 0x0044, 0x102628,  9, 10,  5, 13, false },
		{ 0x0108, 0x040008, 11, 12,  5, 14, true  },
		{ 0x0012, 0x004c84,  7, 11, 10, 14, false },
		{ 0x8020, 0x240188,  3,  8,  7, 18, true  },
		{ 0x0840, 0x041042,  6, 12,  4, 15, true  },
		{ 0x00a0, 0x010802,  2,  5, 11, 16, true  },
		{ 0x0140, 0x202204,  9, 13,  7, 16, true  },
		{ 0x2200, 0x088012,  4, 15,  8, 16, false },
		{ 0x0401, 0x002280,  5, 11,  7, 13, false },
		{ 0x0800, 0x084980,  0,  0,  0,  0, true  },
		{ 0x5000, 0x008122,  3,  6,  8, 15, true  },
		{ 0x2004, 0x001004,  6, 11,  0,  0, true  },
		{ 0x4400, 0x000006,  4, 15,  6, 16, false },
		{ 0x8008, 0x100444,  6, 10, 12, 17, false },
	};

	u16 plain = 0;
	for (unsigned i = 0; i < std::size(equations); ++i)
	{
		auto const &eq = equations[i];
		unsigned const parity = std::popcount(unsigned(cipher & eq.cipher_mask))
			^ std::popcount(address & eq.address_mask)
			^ (BIT(address, eq.a) & BIT(address, eq.b))
			^ (BIT(address, eq.c) & BIT(address, eq.d))
			^ eq.invert;
		plain |= (parity & 1) << i;
	}
	return plain;
}

class ngage_state : public driver_device
{
public:
	ngage_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_flash(*this, "flash")
		, m_pmm_nvram(*this, "pmm")
		, m_cdrive_nvram(*this, "cdrive")
		, m_adsp(*this, "adsp")
		, m_bt(*this, "bluetooth")
		, m_mmc(*this, "mmc")
		, m_rtc(*this, "rtc")
		, m_pcm(*this, "pcm")
		, m_sim(*this, "sim")
		, m_config(*this, "CONFIG")
	{
	}

	void nem4(machine_config &config) ATTR_COLD;
	DECLARE_INPUT_CHANGED_MEMBER(key_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void nem4_map(address_map &map) ATTR_COLD;
	template <unsigned Chip> u16 flash_r(offs_t offset);
	template <unsigned Chip> void flash_w(offs_t offset, u16 data, u16 mem_mask);
	template <unsigned Chip> u8 *flash_storage(u32 addr);
	u32 cdrive_r(offs_t offset, u32 mem_mask);
	void finish_cdrive_erase();
	u32 upp_r(offs_t offset, u32 mem_mask);
	void upp_w(offs_t offset, u32 data, u32 mem_mask);
	u16 uem_r(u8 reg) const;
	void uem_w(u8 reg, u16 data);
	void bt_event(int state);
	bool bt_irq_active() const;
	void recover_calendar_mirror();
	void mdi_update();
	void pcm_reply_ready(int state);
	u16 mdi_word(unsigned index) const;
	void mdi_publish(std::span<u16 const> words);
	u16 euart_status() const;
	void euart_update();
	void dma_start(unsigned channel);
	void lcd_dma_start(unsigned channel);
	void mmc_dma_start(unsigned channel);
	void mmc_event(int state);
	TIMER_CALLBACK_MEMBER(lcd_dma_complete);
	struct keypad_levels { u8 rows, columns; u16 genio; };
	keypad_levels keypad_read() const;
	void keypad_update();
	void lcd_header_w(u32 data);
	void lcd_data_w(u32 data);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);
	void sti_w(offs_t offset, u8 data);
	void nos_trace_w(u32 data, u32 mem_mask);
	u32 interrupt_sources(unsigned bank, bool fiq) const;
	void update_interrupts();
	u16 ctsi_count() const;
	u16 ctsi_window() const;
	u32 timer_count(unsigned n) const;
	void set_timer_count(unsigned n, u32 count);
	TIMER_CALLBACK_MEMBER(timer_expired);
	TIMER_CALLBACK_MEMBER(ctsi_compare_expired);
	TIMER_CALLBACK_MEMBER(mdi_announce);
	TIMER_CALLBACK_MEMBER(uem_tick);

	enum : u8 { ARRAY, IDENTIFY, STATUS, PROTECTION };
	enum : u8 { IDLE, AMD_PROGRAM, AMD_ERASE, INTEL_PROGRAM, INTEL_ERASE };

	required_device<arm920t_cpu_device> m_maincpu;
	required_region_ptr<u8> m_flash;
	required_device<nvram_device> m_pmm_nvram;
	required_device<nvram_device> m_cdrive_nvram;
	required_device<ngage_adsp_device> m_adsp;
	required_device<ngage_bt_device> m_bt;
	required_device<ngage_mmc_device> m_mmc;
	required_device<ngage_rtc_device> m_rtc;
	required_device<ngage_pcm_device> m_pcm;
	required_device<ngage_sim_device> m_sim;
	required_ioport m_config;

	std::vector<u16> m_decoded_nos;
	u32 m_nos_start = 0;
	u32 m_nos_end = 0;
	u8 m_flash_mode[3]{};
	u8 m_flash_unlock[3]{};
	u8 m_flash_operation[3]{};
	u8 m_flash_protection_bank[3]{};
	u32 m_flash_erase_sector[3]{};
	bool m_flash_amd_tail[3]{};
	u8 m_pmm[PMM_SIZE]{};
	u8 m_cdrive[CDRIVE_SIZE]{};
	u32 m_cdrive_erase_base = 0;
	u32 m_cdrive_erase_size = 0;
	bool m_cdrive_erase_polled = false;

	u32 m_upp[UPP_WORDS]{};
	std::bitset<UPP_WORDS> m_upp_reads;
	std::bitset<UPP_WORDS> m_upp_writes;
	u32 m_irq_enable[3]{};
	u32 m_fiq_enable[3]{};
	u32 m_irq_pending[3]{};

	emu_timer *m_timer[6]{};
	u32 m_timer_stopped_count[6]{};
	emu_timer *m_ctsi_compare_timer = nullptr;
	u64 m_ctsi_fast_start = 0;
	u64 m_ctsi_nos_start = 0;

	u16 m_uem_regs[256]{};
	u16 m_uem_adc_result = UEM_ADC_DEFAULT;
	emu_timer *m_uem_timer = nullptr;
	bool m_uem_irq = false;
	bool m_uem_rtc_calibration_written = false;
	u16 m_cbus_command = 0;
	u16 m_cbus_bt_data = 0;
	bool m_bt_irq_pending = false;
	u8 m_uem_alarm_status = 8;
	bool m_cbus_complete = true;
	bool m_power_key_released = false;
	emu_timer *m_mdi_timer = nullptr;
	u8 m_mdi_pointers_seen = 0;
	bool m_mdi_announce_due = false;
	bool m_mdi_announced = false;

	u8 m_adsp_controls_written = 0;
	u16 m_euart_enable = 0;
	u8 m_euart_batch_left = 0;
	u8 m_euart_tx_index = 0;
	bool m_euart_tx_done = false;
	u8 m_euart_rx[16]{};
	u8 m_euart_rx_producer = 0;
	u8 m_euart_rx_consumer = 0;

	u32 m_keys = 0;
	u32 m_key_pending = 0;
	u8 m_key_row_drive = 0x3f;
	u8 m_key_column_drive = 0;
	u8 m_key_row_enable = 0;
	u8 m_key_column_enable = 0;
	u8 m_ctsi_genio_enable = 0;
	u8 m_key_last_columns = 0x3f;
	u16 m_key_last_genio = 0x3c06;
	bool m_key_gpio30 = false;

	emu_timer *m_lcd_dma_timer[8]{};
	u16 m_lcd_pixels[LCD_WIDTH * LCD_HEIGHT]{};
	u32 m_lcd_window_first = 0;
	unsigned m_lcd_cursor = 0;
	unsigned m_lcd_band_first = 0;
	unsigned m_lcd_band_end = LCD_PAIRS;
	unsigned m_lcd_limit = LCD_PAIRS;
	u8 m_lcd_command = 0;
	u8 m_lcd_window_words = 0;
	bool m_lcd_columns_valid = true;
	bool m_lcd_rows_valid = true;
	bool m_lcd_write = false;
	bool m_lcd_pixel_header = false;
	bool m_lcd_on = false;
	std::string m_sti_line;
	std::string m_nos_trace_line;
};

void ngage_state::machine_start()
{
	m_pcm->set_memory(m_maincpu->space(AS_PROGRAM));
	auto const word32 = [this](u32 offset)
	{
		return u32(m_flash[offset]) | (u32(m_flash[offset + 1]) << 8)
			| (u32(m_flash[offset + 2]) << 16) | (u32(m_flash[offset + 3]) << 24);
	};

	u32 const first = word32(0x174);
	u32 const end = word32(0x178);
	if (!((first | end) & 0xfff) && first < end && end <= NOR_SIZE)
	{
		m_nos_start = first;
		m_nos_end = end;
		m_decoded_nos.reserve((end - first) / 2);
		for (u32 addr = first; addr < end; addr += 2)
			m_decoded_nos.push_back(descramble_nos(m_flash[addr] | (m_flash[addr + 1] << 8), addr));
	}

	for (unsigned n = 0; n < std::size(m_timer); ++n)
		m_timer[n] = timer_alloc(FUNC(ngage_state::timer_expired), this);
	m_ctsi_compare_timer = timer_alloc(FUNC(ngage_state::ctsi_compare_expired), this);
	m_mdi_timer = timer_alloc(FUNC(ngage_state::mdi_announce), this);
	m_uem_timer = timer_alloc(FUNC(ngage_state::uem_tick), this);
	for (auto &timer : m_lcd_dma_timer)
		timer = timer_alloc(FUNC(ngage_state::lcd_dma_complete), this);
	m_pmm_nvram->set_base(m_pmm, sizeof(m_pmm));
	m_cdrive_nvram->set_base(m_cdrive, sizeof(m_cdrive));

	save_item(NAME(m_flash_mode));
	save_item(NAME(m_flash_unlock));
	save_item(NAME(m_flash_operation));
	save_item(NAME(m_flash_protection_bank));
	save_item(NAME(m_flash_erase_sector));
	save_item(NAME(m_flash_amd_tail));
	save_item(NAME(m_pmm));
	save_item(NAME(m_cdrive));
	save_item(NAME(m_cdrive_erase_base));
	save_item(NAME(m_cdrive_erase_size));
	save_item(NAME(m_cdrive_erase_polled));
	save_item(NAME(m_upp));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_fiq_enable));
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_timer_stopped_count));
	save_item(NAME(m_ctsi_fast_start));
	save_item(NAME(m_ctsi_nos_start));
	save_item(NAME(m_uem_regs));
	save_item(NAME(m_uem_adc_result));
	save_item(NAME(m_uem_irq));
	save_item(NAME(m_uem_rtc_calibration_written));
	save_item(NAME(m_cbus_command));
	save_item(NAME(m_cbus_bt_data));
	save_item(NAME(m_bt_irq_pending));
	save_item(NAME(m_uem_alarm_status));
	save_item(NAME(m_cbus_complete));
	save_item(NAME(m_power_key_released));
	save_item(NAME(m_mdi_pointers_seen));
	save_item(NAME(m_mdi_announce_due));
	save_item(NAME(m_mdi_announced));
	save_item(NAME(m_adsp_controls_written));
	save_item(NAME(m_euart_enable));
	save_item(NAME(m_euart_batch_left));
	save_item(NAME(m_euart_tx_index));
	save_item(NAME(m_euart_tx_done));
	save_item(NAME(m_euart_rx));
	save_item(NAME(m_euart_rx_producer));
	save_item(NAME(m_euart_rx_consumer));
	save_item(NAME(m_keys));
	save_item(NAME(m_key_pending));
	save_item(NAME(m_key_row_drive));
	save_item(NAME(m_key_column_drive));
	save_item(NAME(m_key_row_enable));
	save_item(NAME(m_key_column_enable));
	save_item(NAME(m_ctsi_genio_enable));
	save_item(NAME(m_key_last_columns));
	save_item(NAME(m_key_last_genio));
	save_item(NAME(m_key_gpio30));
	save_item(NAME(m_lcd_pixels));
	save_item(NAME(m_lcd_window_first));
	save_item(NAME(m_lcd_cursor));
	save_item(NAME(m_lcd_band_first));
	save_item(NAME(m_lcd_band_end));
	save_item(NAME(m_lcd_limit));
	save_item(NAME(m_lcd_command));
	save_item(NAME(m_lcd_window_words));
	save_item(NAME(m_lcd_columns_valid));
	save_item(NAME(m_lcd_rows_valid));
	save_item(NAME(m_lcd_write));
	save_item(NAME(m_lcd_pixel_header));
	save_item(NAME(m_lcd_on));
}

void ngage_state::machine_reset()
{
	ioport_field::user_settings clock_config;
	m_config->field(0x0e)->get_user_settings(clock_config);
	u32 const half_steps = (clock_config.value >> 1) & 7;
	m_maincpu->set_unscaled_clock(ARM_SDRAM_CLOCK + (half_steps <= 4 ? half_steps : 0) * (ARM_SDRAM_CLOCK / 2));

	finish_cdrive_erase();
	recover_calendar_mirror();
	std::fill(std::begin(m_flash_mode), std::end(m_flash_mode), ARRAY);
	std::fill(std::begin(m_flash_unlock), std::end(m_flash_unlock), 0);
	std::fill(std::begin(m_flash_operation), std::end(m_flash_operation), IDLE);
	std::fill(std::begin(m_flash_amd_tail), std::end(m_flash_amd_tail), false);
	std::fill(std::begin(m_upp), std::end(m_upp), 0);
	std::fill(std::begin(m_irq_enable), std::end(m_irq_enable), 0);
	std::fill(std::begin(m_fiq_enable), std::end(m_fiq_enable), 0);
	std::fill(std::begin(m_irq_pending), std::end(m_irq_pending), 0);
	for (unsigned n = 0; n < std::size(m_timer); ++n)
		set_timer_count(n, 0);
	m_ctsi_fast_start = machine().time().as_ticks(CTSI_FAST_CLOCK);
	m_ctsi_nos_start = machine().time().as_ticks(CTSI_NOS_CLOCK);
	m_ctsi_compare_timer->enable(false);

	std::fill(std::begin(m_uem_regs), std::end(m_uem_regs), 0);
	m_uem_adc_result = UEM_ADC_DEFAULT;
	m_uem_irq = false;
	m_uem_rtc_calibration_written = false;
	m_uem_timer->adjust(attotime::from_seconds(1), 0, attotime::from_seconds(1));
	m_cbus_command = m_cbus_bt_data = 0;
	m_bt_irq_pending = false;
	m_uem_alarm_status = 8;
	m_cbus_complete = true;
	m_power_key_released = false;
	m_mdi_pointers_seen = 0;
	m_mdi_announce_due = m_mdi_announced = false;
	m_mdi_timer->enable(false);
	m_adsp_controls_written = 0;
	m_euart_enable = 0;
	m_euart_batch_left = m_euart_tx_index = 0;
	m_euart_tx_done = false;
	m_euart_rx_producer = m_euart_rx_consumer = 0;
	m_adsp->power_w(false);
	m_sim->set_present(BIT(m_config->read(), 0));
	m_key_pending = 0;
	m_key_row_drive = 0x3f;
	m_key_column_drive = m_key_row_enable = m_key_column_enable = m_ctsi_genio_enable = 0;
	m_key_gpio30 = false;
	auto const levels = keypad_read();
	m_key_last_columns = levels.columns;
	m_key_last_genio = levels.genio;

	for (auto *timer : m_lcd_dma_timer)
		timer->enable(false);
	m_lcd_cursor = 0;
	m_lcd_band_first = 0;
	m_lcd_limit = m_lcd_band_end = LCD_PAIRS;
	m_lcd_command = m_lcd_window_words = 0;
	m_lcd_columns_valid = m_lcd_rows_valid = true;
	m_lcd_write = m_lcd_pixel_header = m_lcd_on = false;
	m_sti_line.clear();
	m_nos_trace_line.clear();
	update_interrupts();
}

template <unsigned Chip>
u8 *ngage_state::flash_storage(u32 addr)
{
	if (Chip == 0 && addr >= NOR_SIZE - PMM_SIZE)
		return &m_pmm[addr - (NOR_SIZE - PMM_SIZE)];
	if (Chip == 1 && addr >= NOR_SIZE - CDRIVE_SIZE)
		return &m_cdrive[addr - (NOR_SIZE - CDRIVE_SIZE)];
	return nullptr;
}

template <unsigned Chip>
u16 ngage_state::flash_r(offs_t offset)
{
	u32 const addr = offset * 2;
	switch (m_flash_mode[Chip])
	{
	case IDENTIFY:
		switch (Chip == 1 && addr >= NOR_SIZE - CDRIVE_SIZE ? addr & 0x0f : addr)
		{
		case 0x00: return 0x0001;
		case 0x02: return 0x257e;
		case 0x06: return 0x0042;
		case 0x1c: return 0x2502;
		case 0x1e: return 0x2500;
		default: return 0;
		}
	case STATUS:
		return 0x0080;
	case PROTECTION:
		if ((addr >> 21) == m_flash_protection_bank[Chip])
			return 1;
		break;
	}

	if (Chip == 0 && addr >= m_nos_start && addr < m_nos_end)
		return m_decoded_nos[(addr - m_nos_start) / 2];

	if (u8 const *const storage = flash_storage<Chip>(addr))
		return storage[0] | (storage[1] << 8);

	u32 const index = Chip * NOR_SIZE + addr;
	return m_flash[index] | (m_flash[index + 1] << 8);
}

u32 ngage_state::cdrive_r(offs_t offset, u32 mem_mask)
{
	u32 const addr = NOR_SIZE - CDRIVE_SIZE + offset * 4;
	if (m_flash_mode[1] == ARRAY && m_cdrive_erase_size && addr >= m_cdrive_erase_base
			&& addr - m_cdrive_erase_base < m_cdrive_erase_size)
	{
		// NOS first waits for DQ3 set and DQ7 clear. The FPGA model
		// completes on a second status read or an erase suspend command.
		if (!m_cdrive_erase_polled)
		{
			if (!machine().side_effects_disabled())
				m_cdrive_erase_polled = true;
			return 0x00080008;
		}
		if (!machine().side_effects_disabled())
			finish_cdrive_erase();
		return 0xffffffff;
	}

	u32 const low = (mem_mask & 0xffff) ? flash_r<1>(addr / 2) : 0;
	u32 const high = (mem_mask & 0xffff0000) ? flash_r<1>(addr / 2 + 1) : 0;
	return low | (high << 16);
}

void ngage_state::finish_cdrive_erase()
{
	if (m_cdrive_erase_size)
	{
		std::fill_n(flash_storage<1>(m_cdrive_erase_base), m_cdrive_erase_size, 0xff);
		m_cdrive_erase_size = 0;
		m_cdrive_erase_polled = false;
	}
}

template <unsigned Chip>
void ngage_state::flash_w(offs_t offset, u16 data, u16 mem_mask)
{
	u32 const addr = offset * 2;
	u8 const unlock = m_flash_unlock[Chip];
	u8 &mode = m_flash_mode[Chip];
	u8 &operation = m_flash_operation[Chip];
	u8 *const storage = flash_storage<Chip>(addr);
	u32 const sector_size = Chip == 0 || (addr & 0x1fffff) >= 0x1f0000 ? 0x4000 : 0x10000;
	u32 const sector = addr & ~(sector_size - 1);
	u32 const command_address = addr & 0x7ff;

	if (mem_mask != 0xffff)
		return;

	m_flash_unlock[Chip] = 0;
	if (operation == AMD_PROGRAM || operation == INTEL_PROGRAM)
	{
		if (storage)
		{
			storage[0] &= u8(data);
			storage[1] &= u8(data >> 8);
		}
		mode = operation == AMD_PROGRAM ? ARRAY : STATUS;
		m_flash_amd_tail[Chip] = operation == AMD_PROGRAM;
		operation = IDLE;
	}
	else if ((operation == AMD_ERASE && unlock == 2 && data == 0x30)
		|| (operation == INTEL_ERASE && data == 0xd0 && m_flash_erase_sector[Chip] == sector))
	{
		if (Chip == 1 && storage && operation == AMD_ERASE)
		{
			finish_cdrive_erase();
			m_cdrive_erase_base = sector;
			m_cdrive_erase_size = sector_size;
		}
		else if (storage)
			std::fill_n(flash_storage<Chip>(sector), sector_size, 0xff);
		mode = operation == AMD_ERASE ? ARRAY : STATUS;
		operation = IDLE;
	}
	else if (Chip == 1 && storage && data == 0xb0)
		finish_cdrive_erase();
	else if (data == 0xf0 || data == 0xff)
	{
		mode = ARRAY;
		operation = IDLE;
		m_flash_amd_tail[Chip] = false;
	}
	else if (command_address == 0x2aa && data == 0xaa)
		m_flash_unlock[Chip] = 1;
	else if (unlock == 1 && command_address == 0x554 && data == 0x55)
		m_flash_unlock[Chip] = 2;
	else if (unlock == 2 && command_address == 0x2aa && data == 0xa0)
		operation = AMD_PROGRAM;
	else if (unlock == 2 && command_address == 0x2aa && data == 0x80)
		operation = AMD_ERASE;
	else if (data == 0x90 && ((unlock == 2 && command_address == 0x2aa) || (!unlock && (addr == 0 || addr == 0xaaaa))))
		mode = IDENTIFY;
	else if (unlock == 2 && (offset & 0xfff) == 0x555 && data == 0xc0)
	{
		mode = PROTECTION;
		m_flash_protection_bank[Chip] = addr >> 21;
	}
	else if (data == 0x70)
		mode = STATUS;
	else if (data == 0x50)
		mode = ARRAY;
	else if (data == 0x60)
	{
		mode = PROTECTION;
		m_flash_protection_bank[Chip] = addr >> 21;
	}
	else if (mode == PROTECTION && (data == 0x01 || data == 0xd0))
		mode = ARRAY;
	else if (data == 0x40 && storage)
		operation = INTEL_PROGRAM;
	else if (data == 0x20)
	{
		// The native AMD programmer issues 0x20 after programming.
		if (m_flash_amd_tail[Chip])
			m_flash_amd_tail[Chip] = false;
		else if (storage)
		{
			operation = INTEL_ERASE;
			m_flash_erase_sector[Chip] = sector;
		}
	}
	else
		logerror("%s: unimplemented NOR CS%u write %04x at %06x\n", machine().describe_context(), Chip, data, addr);
}

u32 ngage_state::interrupt_sources(unsigned bank, bool fiq) const
{
	if (fiq)
		return bank == 0 ? (m_irq_pending[0] & 0x3f) | ((m_sim->status() || (euart_status() & m_euart_enable)) ? (1U << 26) : 0) : 0;

	return m_irq_pending[bank] | (bank == 0 ? m_mmc->irq_sources() | (m_uem_irq ? 2 : 0) : 0);
}

void ngage_state::bt_event(int state)
{
	if (state)
	{
		m_bt_irq_pending = true;
		update_interrupts();
	}
}

bool ngage_state::bt_irq_active() const
{
	return m_bt_irq_pending && (m_ctsi_genio_enable & (BLUETOOTH_EVENT >> 16));
}

void ngage_state::update_interrupts()
{
	if (m_key_pending || bt_irq_active() || (m_upp[0x50100 / 4] & (m_upp[0x50010 / 4] >> 16) & CTSI_EVENT_MASK))
		m_irq_pending[0] |= 1U << 25;
	else
		m_irq_pending[0] &= ~(1U << 25);

	bool irq = false;
	bool fiq = false;
	for (unsigned n = 0; n < std::size(m_irq_pending); ++n)
	{
		irq |= bool(interrupt_sources(n, false) & m_irq_enable[n]);
		fiq |= bool(interrupt_sources(n, true) & m_fiq_enable[n]);
	}
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_IRQ_LINE, irq ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, fiq ? ASSERT_LINE : CLEAR_LINE);
}

u16 ngage_state::ctsi_count() const
{
	return machine().time().as_ticks(CTSI_NOS_CLOCK) - m_ctsi_nos_start;
}

u16 ngage_state::ctsi_window() const
{
	u16 const programmed = m_upp[0x50114 / 4];
	return programmed ? programmed : 0x80;
}

TIMER_CALLBACK_MEMBER(ngage_state::ctsi_compare_expired)
{
	m_upp[0x50100 / 4] |= 0x80;
	update_interrupts();
	m_ctsi_compare_timer->adjust(attotime::from_ticks(0x10000, CTSI_NOS_CLOCK));
}

u32 ngage_state::timer_count(unsigned n) const
{
	if (!m_timer[n]->enabled())
		return m_timer_stopped_count[n];

	// Round up so the polling register reaches zero at the expiry event.
	return (m_timer[n]->remaining() + attotime::from_ticks(1, GENERAL_TIMER_CLOCK)).as_ticks(GENERAL_TIMER_CLOCK);
}

void ngage_state::set_timer_count(unsigned n, u32 count)
{
	u32 const control = m_upp[(0x20a04 + n * 0x20) / 4];
	m_timer_stopped_count[n] = count;
	if (control & 3)
		m_timer[n]->adjust(attotime::from_ticks(count ? u64(count) : u64(1) << 32, GENERAL_TIMER_CLOCK), n);
	else
		m_timer[n]->enable(false);
}

TIMER_CALLBACK_MEMBER(ngage_state::timer_expired)
{
	unsigned const n = param;
	u32 const base = (0x20a00 + n * 0x20) / 4;
	m_upp[base + 2] = 1;
	m_irq_pending[0] |= 1U << n;
	if (BIT(m_upp[base + 1], 1))
		set_timer_count(n, m_upp[base + 6]);
	else
		m_timer_stopped_count[n] = 0;
	update_interrupts();
}

void ngage_state::recover_calendar_mirror()
{
	// Stock 4.03 can persist the two RTC calendar mirrors a minute apart.
	// Its restore guard rejects that pair. Repair the older mirror only
	// when both active records are unique and in the same native catalog.
	static constexpr unsigned SECTOR_SIZE = 0x4000;
	static constexpr unsigned HEADER_SIZE = 12;
	u8 *records[2]{};
	unsigned record_bank = PMM_SIZE;
	for (unsigned bank = 0; bank < PMM_SIZE; bank += SECTOR_SIZE)
	{
		u8 *const sector = &m_pmm[bank];
		if (std::memcmp(&sector[6], "PMMCAT", 6))
			continue;
		for (unsigned offset = 32; offset <= SECTOR_SIZE - HEADER_SIZE; )
		{
			u8 *const entry = &sector[offset];
			if (entry[0] == 0xff)
				break;
			unsigned const length = get_u16le(&entry[8]);
			unsigned const next = get_u16le(&entry[10]);
			if (next != offset + HEADER_SIZE + length || next > SECTOR_SIZE)
				return;
			u16 const record = get_u16le(&entry[2]);
			if (entry[0] == 0xf4 && entry[1] == 0xd4 && entry[4] == 0x55 && length == 8 && record < 2)
			{
				if (records[record] || (record_bank != PMM_SIZE && record_bank != bank))
					return;
				records[record] = entry + HEADER_SIZE;
				record_bank = bank;
			}
			offset = next;
		}
	}

	if (records[0] && records[1])
	{
		u64 const primary = get_u64le(records[0]);
		u64 const mirror = get_u64le(records[1]);
		if (mirror && primary > mirror && primary - mirror == 60'000'000)
		{
			put_u64le(records[1], primary);
			logerror("UEM RTC: reconciled the saved calendar mirror\n");
		}
	}
}

u16 ngage_state::uem_r(u8 reg) const
{
	// The RTC read aliases share addresses with control writes. The guest
	// maintains the date base in PMM and can set the counter's eight-bit day.
	if (u16(m_upp[0x50420 / 4]) == 7)
	{
		u32 const time = m_rtc->counter();
		switch (reg)
		{
		case 0x21: return u8(time / 86400);
		case 0x31: return time % 60;
		case 0x32: return (time / 60) % 60;
		case 0x33: return (time / 3600) % 24;
		}
	}
	switch (reg)
	{
	case 0x24: return m_uem_rtc_calibration_written ? m_uem_regs[reg] : 1;
	case 0x35: return m_uem_alarm_status | ((!m_power_key_released || BIT(m_keys, 26)) ? 0x10 : 0);
	case 0x3a: return m_uem_adc_result;
	case 0x3f: return (!m_power_key_released || BIT(m_keys, 26)) ? 2 : 0;
	case 0x80: return 0x0400;
	default:   return m_uem_regs[reg];
	}
}

TIMER_CALLBACK_MEMBER(ngage_state::uem_tick)
{
	m_uem_irq = true;
	update_interrupts();
}

void ngage_state::uem_w(u8 reg, u16 data)
{
	m_uem_regs[reg] = data;
	if (reg == 0x2f || reg == 0x30)
	{
		m_adsp_controls_written |= reg == 0x2f ? 1 : 2;
		euart_update();
	}
	if (reg == 0x21 && u16(m_upp[0x50420 / 4]) == 7)
		m_rtc->set_day(u8(data));
	if (reg == 0x24)
		m_uem_rtc_calibration_written = true;
	if (reg == 0x35)
		m_uem_alarm_status &= ~data;
	if (reg == 0x39 && BIT(data, 11))
	{
		m_uem_adc_result = UEM_ADC_DEFAULT;
		if ((data >> 12) == 12)
		{
			ioport_field::user_settings battery;
			m_config->field(BATTERY_LEVEL_MASK)->get_user_settings(battery);
			unsigned const level = (battery.value & BATTERY_LEVEL_MASK) >> 4;
			if (level < std::size(BATTERY_VOLTAGE_ADC))
				m_uem_adc_result = BATTERY_VOLTAGE_ADC[level];
		}
	}
	if (reg == 0x36 && (m_upp[0x50420 / 4] & 0xffff) == 15 && (data & 0xfff8) == 0xfff0)
		m_power_key_released = true;
	if (reg == 0x30 && BIT(m_upp[0x20a04 / 4], 1) && m_timer[0]->enabled())
		uem_tick(0);
}

u16 ngage_state::mdi_word(unsigned index) const
{
	return m_upp[index / 2] >> ((index & 1) * 16);
}

void ngage_state::mdi_publish(std::span<u16 const> words)
{
	unsigned producer = u16(m_upp[MDI_RX_POINTERS]);
	for (u16 const word : words)
	{
		unsigned const index = MDI_RING_WORDS + producer;
		unsigned const shift = (index & 1) * 16;
		m_upp[index / 2] = (m_upp[index / 2] & ~(0xffffU << shift)) | (u32(word) << shift);
		producer = (producer + 1) % MDI_RING_WORDS;
	}
	m_upp[MDI_RX_POINTERS] = (m_upp[MDI_RX_POINTERS] & 0xffff0000) | producer;
	m_upp[0x50100 / 4] |= 0x0400;
}

TIMER_CALLBACK_MEMBER(ngage_state::mdi_announce)
{
	m_mdi_announce_due = true;
	mdi_update();
}

void ngage_state::pcm_reply_ready(int state)
{
	if (state)
		mdi_update();
}

void ngage_state::mdi_update()
{
	if (u16(m_upp[MDI_CONTROL]) != 0x0500)
	{
		m_mdi_timer->enable(false);
		m_mdi_announce_due = m_mdi_announced = false;
		m_pcm->disconnect();
		return;
	}
	if (m_mdi_pointers_seen != 3)
		return;
	if (!m_mdi_announced && !m_mdi_announce_due && !m_mdi_timer->enabled())
		m_mdi_timer->adjust(attotime::from_ticks(20, CTSI_NOS_CLOCK));

	unsigned const producer = u16(m_upp[MDI_TX_POINTERS]);
	unsigned consumer = m_upp[MDI_TX_POINTERS] >> 16;
	unsigned const read_ptr = m_upp[MDI_RX_POINTERS] >> 16;
	unsigned const write_ptr = u16(m_upp[MDI_RX_POINTERS]);
	if (std::max({producer, consumer, read_ptr, write_ptr}) >= MDI_RING_WORDS)
		return;
	unsigned free = MDI_RING_WORDS - 1 - (write_ptr + MDI_RING_WORDS - read_ptr) % MDI_RING_WORDS;

	if (m_mdi_announce_due)
	{
		static constexpr u16 ready[] = { 0x0000, 0x7cd0, 0x0001, 0x0400 };
		if (free < std::size(ready))
			return;
		mdi_publish(ready);
		free -= std::size(ready);
		m_mdi_announce_due = false;
		m_mdi_announced = true;
		logerror("UPP MDI: published DSP startup announcement\n");
	}

	auto const publish_pcm = [this, &free]()
	{
		for (auto reply = m_pcm->reply(); !reply.empty() && reply.size() <= free; reply = m_pcm->reply())
		{
			mdi_publish(reply);
			free -= reply.size();
			m_pcm->reply_consumed();
		}
	};
	publish_pcm();

	while ((producer + MDI_RING_WORDS - consumer) % MDI_RING_WORDS >= 3)
	{
		auto const word = [this, consumer](unsigned index)
		{
			return mdi_word((consumer + index) % MDI_RING_WORDS);
		};
		unsigned const length = word(2) + 6;
		unsigned const words = (length + 1) / 2;
		if (words > (producer + MDI_RING_WORDS - consumer) % MDI_RING_WORDS)
			break;

		// Native F1/A0 RF and software version requests use a synthetic
		// revision and version string.
		u8 const media = word(0) >> 8;
		bool const version_request = length == 12 && (media == 0x12 || media == 0)
			&& (word(0) & 0xfc) == 0x7c && (word(1) & 0xff) == 0xf1 && (word(4) & 0xff) == 0xa0;
		bool const rf_version = version_request && (word(4) >> 8) == 0x20 && (word(5) >> 8) == 2;
		bool const sw_version = version_request && (word(4) >> 8) == 0x40 && (word(5) >> 8) == 1;
		if ((media == 0x12 || media == 0) && (word(0) & 0xfc) == 0x7c && (word(1) & 0xff) == 0xe8
			&& length >= 10 && length <= 24)
		{
			if (!m_pcm->reply().empty() || free < ngage_pcm_device::REPLY_WORDS)
				break;
			u8 packet[24];
			for (unsigned i = 0; i < words; ++i)
			{
				u16 const value = word(i);
				packet[i * 2] = value >> 8;
				packet[i * 2 + 1] = value;
			}
			m_pcm->receive(std::span(packet).first(length));
			publish_pcm();
		}
		else if (rf_version || sw_version)
		{
			unsigned const reply_words = rf_version ? 6 : 8;
			if (free < reply_words)
				break;
			u16 const reply[] = {
				u16((word(0) & 0xff00) | (word(1) >> 8)),
				u16((word(0) << 8) | (word(1) & 0xff)),
				u16(rf_version ? 6 : 10),
				u16((word(3) << 8) | (word(3) >> 8)),
				u16((word(4) & 0xff00) | 0xa1),
				u16(rf_version ? 0x0103 : 0x0548), 0x444c, 0x3031,
			};
			mdi_publish(std::span(reply).first(reply_words));
			free -= reply_words;
			logerror("UPP MDI: replied to DSP %s version query\n", rf_version ? "RF" : "software");
		}
		consumer = (consumer + words) % MDI_RING_WORDS;
		m_upp[MDI_TX_POINTERS] = (consumer << 16) | producer;
		m_upp[0x50100 / 4] |= 0x0200;
	}
	update_interrupts();
}

u16 ngage_state::euart_status() const
{
	return (m_euart_rx_consumer != m_euart_rx_producer ? 8 : 0) | (m_euart_tx_done ? 16 : 0);
}

void ngage_state::euart_update()
{
	bool const powered = m_adsp_controls_written == 3 && !(m_uem_regs[0x2f] & 0xf0) && BIT(m_uem_regs[0x30], 5);
	bool const receive_enabled = BIT(m_euart_enable, 3) && !(m_upp[0x50560 / 4] & 7);
	m_adsp->power_w(powered && receive_enabled);
	while (m_adsp->rx_pending() && ((m_euart_rx_producer + 1) & 15) != m_euart_rx_consumer)
	{
		m_euart_rx[m_euart_rx_producer] = m_adsp->rx_r();
		m_euart_rx_producer = (m_euart_rx_producer + 1) & 15;
	}
	update_interrupts();
}

void ngage_state::dma_start(unsigned channel)
{
	u32 const request = (m_upp[0x20064 / 4] >> (channel * 4)) & 15;
	if (request == 2)
		mmc_dma_start(channel);
	else if (request == 7)
		lcd_dma_start(channel);
}

void ngage_state::mmc_event(int state)
{
	if (!state)
		return;
	for (unsigned channel = 0; channel < 8; ++channel)
		if (((m_upp[0x20064 / 4] >> (channel * 4)) & 15) == 2)
			mmc_dma_start(channel);
	update_interrupts();
}

void ngage_state::mmc_dma_start(unsigned channel)
{
	u32 *const reg = &m_upp[(0x20300 + channel * 0x40) / 4];
	if (reg[1] != 1 || (reg[2] != 2 && reg[2] != 3))
		return;

	static constexpr u32 fifo = 0x0c120624;
	static constexpr u32 fifo_alias = 0x10120624;
	bool const receive = (reg[5] == fifo || reg[5] == fifo_alias) && reg[6] == 0 && reg[7] == 8
		&& reg[9] == 4 && reg[10] == 32;
	bool const transmit = (reg[8] == fifo || reg[8] == fifo_alias) && reg[9] == 0 && reg[10] == 8
		&& reg[6] == 4 && reg[7] == 32;
	u32 const address = receive ? reg[8] : reg[5];
	u64 const bytes = u64(reg[11]) + 1;
	if ((!receive && !transmit) || !m_mmc->dma_ready(transmit) || (address & 3) || (bytes & 3)
			|| bytes > 4096 || address < 0x0a000000 || u64(address) + bytes > 0x0b000000)
		return;

	// Clear the run register before the FIFO's last word raises DataIdle.
	// The callback can then see other armed channels without reentering this one.
	reg[2] = 0;
	address_space &program = m_maincpu->space(AS_PROGRAM);
	for (unsigned n = 0; n < bytes; n += 4)
		if (receive)
			program.write_dword(address + n, m_mmc->read(0x24 / 4, 0xffffffff));
		else
			m_mmc->write(0x24 / 4, program.read_dword(address + n), 0xffffffff);
	m_irq_pending[0] |= 1U << (channel + 6);
	update_interrupts();
}

void ngage_state::lcd_dma_start(unsigned channel)
{
	if (m_lcd_dma_timer[channel]->enabled())
		return;

	u32 const *const registers = &m_upp[(0x20300 + channel * 0x40) / 4];
	u32 const request = (m_upp[0x20064 / 4] >> (channel * 4)) & 15;
	u32 const source = registers[5];
	u32 const destination = registers[8];
	u64 const words = u64(registers[11]) + 1;
	bool const sdram = source >= 0x0a000000 && u64(source) + words * 4 <= 0x0b000000;
	if (registers[2] != 2 || registers[1] != 1 || request != 7 || !sdram || (source & 3)
			|| registers[6] != 4 || registers[7] != 32 || registers[9] != 0 || registers[10] != 32
			|| (destination != 0x10121208 && destination != 0x0c121208)
			|| !m_lcd_pixel_header || !m_lcd_write || m_lcd_cursor >= m_lcd_limit
			|| words > m_lcd_limit - m_lcd_cursor)
	{
		logerror("%s: unsupported UPP DMA channel %u transfer %08x to %08x (%u words)\n",
			machine().describe_context(), channel, source, destination, words);
		return;
	}

	// Deliver the FIFO data in a batch and pace completion by the minimum
	// transfer time of the 16-bit SDRAM interface, clocked 1:1 with the ARM.
	address_space &program = m_maincpu->space(AS_PROGRAM);
	for (u32 n = 0; n < words; ++n)
		lcd_data_w(program.read_dword(source + n * 4));
	m_lcd_dma_timer[channel]->adjust(attotime::from_ticks(words * 2, m_maincpu->unscaled_clock()), channel);
}

TIMER_CALLBACK_MEMBER(ngage_state::lcd_dma_complete)
{
	m_upp[(0x20308 + param * 0x40) / 4] = 0;
	m_irq_pending[0] |= 1U << (param + 6);
	update_interrupts();
}

ngage_state::keypad_levels ngage_state::keypad_read() const
{
	u8 const row_data = m_upp[0x50400 / 4] & 0x1f;
	u8 const column_data = m_upp[0x5040c / 4] & 0x3f;
	u8 const row_low = m_key_row_drive & ~row_data & 0x1f;
	u8 const column_low = m_key_column_drive & ~column_data;
	u8 rows = ~row_low & 0x1f;
	u8 columns = ~column_low & 0x3f;
	for (unsigned column = 0; column < 6; ++column)
	{
		unsigned const first = column < 2 ? 5 + column * 2 : 9 + (column - 2) * 3;
		unsigned const first_row = column < 2 ? 0 : 2;
		unsigned const row_count = column < 2 ? 2 : 3;
		for (unsigned i = 0; i < row_count; ++i)
		{
			if (!BIT(m_keys, first + i))
				continue;
			unsigned const row = first_row + i;
			if (BIT(column_low, column))
				rows &= ~(1U << row);
			if (BIT(row_low, row))
				columns &= ~(1U << column);
		}
		if (column >= 2 && BIT(m_keys, 19 + column) && BIT(m_key_row_drive, 5) && !m_key_gpio30)
			columns &= ~(1U << column);
	}
	rows = (rows & ~m_key_row_drive) | (row_data & m_key_row_drive);
	columns = (columns & ~m_key_column_drive) | (column_data & m_key_column_drive);

	static constexpr unsigned rocker_pins[] = { 1, 10, 13, 2, 11 };
	u16 inputs = 0x3c06;
	for (unsigned i = 0; i < std::size(rocker_pins); ++i)
		if (BIT(m_keys, i))
			inputs &= ~(1U << rocker_pins[i]);
	if (BIT(m_uem_regs[0x2a], 3) && !BIT(m_upp[0x50200 / 4], 16))
		inputs &= ~2;

	u16 const direction = m_upp[0x5020c / 4] >> 16;
	u16 genio = (inputs & ~direction) | (m_upp[0x50200 / 4] >> 16 & direction);
	bool const music_output = BIT(m_upp[0x50210 / 4], 12);
	bool const music_level = music_output ? BIT(m_upp[0x50204 / 4], 28) : !BIT(m_keys, 25);
	genio = (genio & ~0x1000) | (music_level ? 0x1000 : 0);
	return { rows, columns, genio };
}

void ngage_state::keypad_update()
{
	auto const levels = keypad_read();
	if (m_key_last_columns & ~levels.columns & m_key_column_enable & ~m_key_column_drive)
		m_key_pending |= 4;
	u16 const direction = m_upp[0x5020c / 4] >> 16;
	u16 const falling = m_key_last_genio & ~levels.genio & ~direction;
	static constexpr unsigned pins[] = { 0, 10, 11, 13, 0, 1, 12, 2 };
	for (unsigned i = 1; i < std::size(pins); ++i)
		if (BIT(m_ctsi_genio_enable & (KEYPAD_EVENTS >> 16), i) && BIT(falling, pins[i]))
			m_key_pending |= 1U << (16 + i);
	m_key_last_columns = levels.columns;
	m_key_last_genio = levels.genio;
	update_interrupts();
}

INPUT_CHANGED_MEMBER(ngage_state::key_changed)
{
	u32 const key = 1U << param;
	m_keys = (m_keys & ~key) | (newval ? key : 0);
	keypad_update();
}

u32 ngage_state::upp_r(offs_t offset, u32 mem_mask)
{
	u32 const addr = offset * 4;
	if (addr >= 0x20600 && addr < 0x20640)
		return m_mmc->read((addr - 0x20600) / 4, mem_mask);
	if (addr >= 0x20100 && addr < 0x20280)
	{
		unsigned const plane = (addr - 0x20100) / 0xc0;
		unsigned const bank = ((addr - 0x20100) % 0xc0) / 0x40;
		u32 const enabled = plane ? m_fiq_enable[bank] : m_irq_enable[bank];
		u32 const sources = interrupt_sources(bank, plane != 0);
		u32 const pending = sources & enabled;
		switch (addr & 0x3f)
		{
		case 0x08:
		case 0x0c: return enabled;
		case 0x10: return sources;
		case 0x14: return pending ? std::countr_zero(pending) + 1 : 0;
		}
	}
	if (addr >= 0x20a00 && addr < 0x20ac0 && (addr & 0x1f) == 0x10)
		return timer_count((addr - 0x20a00) / 0x20);

	switch (addr)
	{
	case 0x20050: return 0; // Fast status is separate from the CTSI FIQ source.
	case 0x20068: return m_upp[0x20064 / 4];
	case 0x20c04:
	{
		u8 const driven_low = m_key_column_drive & ~(m_upp[0x5040c / 4] & 0x3f);
		bool const pressed = ((m_keys >> 19) & driven_low & 0x3c) != 0;
		bool const level = BIT(m_key_row_drive, 5) ? m_key_gpio30 : !pressed;
		return (m_upp[offset] & ~0x00080000) | (level ? 0x00080000 : 0);
	}
	case 0x20020:
		switch (m_upp[0x20004 / 4])
		{
		case 0x0c: return 0x04000000;
		case 0x03:
		case 0x27: return 0x08000000;
		case 0x20:
		case 0x40: return 0x06000000;
		default: break;
		}
		break;
	case 0x21200: return m_upp[offset] == 0x49200000 ? 0x40000000 : 0;
	case 0x21204: return m_upp[offset] == 1 ? 4 : 0; // LCD FIFO ready
	case 0x21208: return 0;
	case 0x48000:
	case 0x49000: return m_upp[offset] | 1; // DPLL lock, with settling time unmeasured.
	case 0x4b000: return m_upp[offset] | 0x80;
	case 0x50000: return m_upp[offset] | 0x00400000; // GSM board strap.
	case 0x50014: return (m_upp[offset] & ~GENIO_ENABLE_MASK) | ((m_ctsi_genio_enable * 0x10001U) & GENIO_ENABLE_MASK);
	case 0x500b8: return (u32(euart_status()) << 16) | (m_sim->status() ? 4 : 0);
	case 0x50100: return (m_upp[offset] & ~(KEYPAD_EVENTS | BLUETOOTH_EVENT)) | m_key_pending
		| (bt_irq_active() ? BLUETOOTH_EVENT : 0);
	case 0x50110:
	{
		u32 const window = ctsi_window();
		u64 const ticks = machine().time().as_ticks(CTSI_FAST_CLOCK) - m_ctsi_fast_start;
		return ((window - (ticks % (window + 1))) << 16) | (m_upp[offset] & 0xffff);
	}
	case 0x50114: return (u32(ctsi_count()) << 16) | ctsi_window();
	case 0x50208: // UEM-routed ear data and TX loopbacks
		return (m_upp[offset] & ~0x2c0e0000)
			| (u32(keypad_read().genio & 0x2c06) << 16)
			| ((BIT(m_uem_regs[0x3b], 10) && BIT(m_upp[0x50200 / 4], 18)) ? 0x00080000 : 0);
	case 0x5020c: // UEM-routed SIM control loopback
		return (m_upp[offset] & ~0x1002) | (keypad_read().genio & 0x1000)
			| ((BIT(m_uem_regs[0x2c], 5) && (m_upp[0x50204 / 4] & 0x00050000) == 0x00050000) ? 2 : 0);
	case 0x5021c: // MBUS strobes are self-clearing; startup tests the loopback.
		return (m_upp[offset] & 0xffbf0000) | ((m_upp[offset] & 0x00800000) >> 1);
	case 0x50420: return u32((m_cbus_command & ~0x0400) | (m_cbus_complete ? 0x0400 : 0)) << 16;
	case 0x50424:
		if (!BIT(m_cbus_command, 8) || !m_cbus_complete)
			return 0;
		if (((m_cbus_command >> 5) & 7) == BLUETOOTH_CBUS_DEVICE)
			return u32(m_cbus_bt_data) << 16;
		return u32(uem_r(u8(m_cbus_command))) << 16;
	case 0x50400: return (m_upp[offset] & ~0x001f001f) | (u32(m_key_row_drive & 0x1f) << 16) | keypad_read().rows;
	case 0x50404: return (m_upp[offset] & ~0x1f) | m_key_row_enable;
	case 0x5040c: return (m_upp[offset] & ~0x003f003f) | (u32(m_key_column_drive) << 16) | keypad_read().columns;
	case 0x50410: return (m_upp[offset] & ~0x3f) | m_key_column_enable;
	case 0x50800: return (m_upp[offset] & 0xffff0000) | m_sim->status();
	case 0x50804:
	{
		u32 value = m_upp[offset];
		if (mem_mask & 0xffff0000)
		{
			if ((value >> 16) == 0x79)
				value &= ~0x003a0000;
			else if ((value >> 16) == 0x7e)
				value &= ~0x00390000;
			if (!machine().side_effects_disabled())
				m_upp[offset] = value;
		}
		return value;
	}
	case 0x5080c:
	{
		u32 const data = (mem_mask & 0xffff) ? m_sim->read() : 0;
		if ((mem_mask & 0xffff) && !machine().side_effects_disabled())
			update_interrupts();
		return (m_upp[offset] & 0xffff0000) | data;
	}
	case 0x50504: return m_upp[offset] | 0x1d0; // FBUS TX empty.
	case 0x50514: return (m_upp[offset] & ~4) | (m_euart_batch_left ? 0 : 4);
	case 0x50554:
		if ((mem_mask & 0xffff0000) && m_euart_rx_consumer != m_euart_rx_producer)
		{
			u32 const data = u32(m_euart_rx[m_euart_rx_consumer]) << 24;
			if (!machine().side_effects_disabled())
			{
				m_euart_rx_consumer = (m_euart_rx_consumer + 1) & 15;
				euart_update();
			}
			return data;
		}
		return 0;
	case 0x5055c: return (m_euart_tx_index << 4) | m_euart_tx_index;
	case 0x50560: return (m_upp[offset] & 0xffff) | (u32(m_euart_rx_producer) << 20) | (u32(m_euart_rx_consumer) << 16);
	case 0x50568: return m_upp[offset] | 0x00060000;
	}

	if (!machine().side_effects_disabled() && !m_upp_reads.test(offset))
	{
		m_upp_reads.set(offset);
		logerror("%s: UPP read %08x mask %08x = %08x\n",
			machine().describe_context(), UPP_BASE + addr, mem_mask, m_upp[offset]);
	}
	return m_upp[offset];
}

void ngage_state::upp_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 const addr = offset * 4;
	if (addr >= 0x20600 && addr < 0x20640)
	{
		m_mmc->write((addr - 0x20600) / 4, data, mem_mask);
		return;
	}
	u32 const previous = m_upp[offset];
	COMBINE_DATA(&m_upp[offset]);

	if (offset == MDI_TX_POINTERS || offset == MDI_RX_POINTERS)
	{
		m_mdi_pointers_seen |= offset == MDI_TX_POINTERS ? 1 : 2;
		mdi_update();
		return;
	}
	if (offset == MDI_CONTROL)
		mdi_update();

	switch (addr)
	{
	case 0x20c0c:
		if (BIT(data & mem_mask, 19))
			m_key_gpio30 = true;
		break;
	case 0x20c10:
		if (BIT(data & mem_mask, 19))
			m_key_gpio30 = false;
		break;
	case 0x20c14:
		if (mem_mask == 0xffffffff)
			m_key_row_drive = (m_key_row_drive & 0x1f) | (BIT(data, 19) ? 0 : 0x20);
		break;
	case 0x20064:
		m_upp[offset] = previous | (data & mem_mask);
		break;
	case 0x20068:
		m_upp[0x20064 / 4] &= ~(data & mem_mask);
		break;
	case 0x50800:
		m_sim->acknowledge(data & mem_mask);
		update_interrupts();
		break;
	case 0x50808:
		if (mem_mask & 0xffff)
			m_sim->start(data & mem_mask);
		update_interrupts();
		break;
	case 0x5080c:
		if (mem_mask & 0xffff)
			m_sim->write(data & mem_mask);
		update_interrupts();
		break;
	case 0x5000c:
		m_euart_enable = (m_euart_enable & ~(data & mem_mask)) | ((data & mem_mask) >> 16);
		euart_update();
		break;
	case 0x500b8:
		if (BIT(data & mem_mask, 20))
		{
			m_euart_tx_done = false;
			update_interrupts();
		}
		break;
	case 0x50010:
	{
		u32 const commands = data & mem_mask;
		u16 const enable = ((previous >> 16) | (commands >> 16)) & ~commands;
		m_upp[offset] = (u32(enable) << 16) | (m_upp[offset] & 0xffff);
		// Enabling software sources 12/13 also sets their pending status.
		u32 &status = m_upp[0x50100 / 4];
		status = (status | ((commands >> 16) & CTSI_SOFTWARE_EVENTS))
			& ~(commands & CTSI_SOFTWARE_EVENTS);
		update_interrupts();
		break;
	}
	case 0x50100:
		m_key_pending &= ~(data & mem_mask);
		if (data & mem_mask & BLUETOOTH_EVENT)
			m_bt_irq_pending = false;
		m_upp[offset] = (m_upp[offset] & ~CTSI_EVENT_MASK) | (previous & CTSI_EVENT_MASK & ~(data & mem_mask));
		update_interrupts();
		break;
	case 0x50114:
		if (mem_mask & 0xffff)
			m_ctsi_fast_start = machine().time().as_ticks(CTSI_FAST_CLOCK);
		break;
	case 0x50118:
		if (mem_mask & 0xffff)
		{
			u16 const delta = m_upp[offset] - ctsi_count();
			m_ctsi_compare_timer->adjust(attotime::from_ticks(delta, CTSI_NOS_CLOCK));
		}
		break;
	case 0x50200:
		m_upp[offset] |= previous & 0xffff0000;
		m_upp[0x50204 / 4] &= ~((data & mem_mask) >> 16);
		break;
	case 0x50204:
		m_upp[offset] |= previous & 0xffff0000;
		m_upp[0x50200 / 4] &= ~((data & mem_mask) << 16);
		m_upp[0x50208 / 4] &= ~((data & mem_mask) >> 16);
		break;
	case 0x50208:
		m_upp[0x50204 / 4] &= ~((data & mem_mask) << 16);
		break;
	case 0x50554:
		if (mem_mask & 0xffff)
		{
			m_adsp->tx_w(data >> 8);
			m_euart_tx_index = (m_euart_tx_index + 1) & 15;
			if (m_euart_batch_left && !--m_euart_batch_left)
				m_euart_tx_done = true;
			euart_update();
		}
		break;
	case 0x50558:
		if (mem_mask & 0xffff0000)
		{
			m_euart_batch_left = ((m_upp[offset] >> 16) & 15) + 1;
			m_euart_tx_done = false;
			update_interrupts();
		}
		break;
	case 0x50560:
		if (mem_mask & 0xffff)
		{
			if (m_upp[offset] & 7)
				m_euart_rx_producer = m_euart_rx_consumer = 0;
			euart_update();
		}
		break;
	}

	u32 const bits = data & mem_mask;
	switch (addr)
	{
	case 0x50014:
		m_ctsi_genio_enable = (m_ctsi_genio_enable | (bits >> 16)) & ~bits;
		keypad_update();
		break;
	case 0x50400:
		m_key_row_drive = (m_key_row_drive | ((bits >> 16) & 0x1f)) & ~((bits >> 24) & 0x1f);
		keypad_update();
		break;
	case 0x50404:
		m_key_row_enable = (m_key_row_enable | (bits & 0x1f)) & ~((bits >> 8) & 0x1f);
		keypad_update();
		break;
	case 0x5040c:
		m_key_column_drive = (m_key_column_drive | ((bits >> 16) & 0x3f)) & ~((bits >> 24) & 0x3f);
		keypad_update();
		break;
	case 0x50410:
		m_key_column_enable = (m_key_column_enable | (bits & 0x3f)) & ~((bits >> 8) & 0x3f);
		keypad_update();
		break;
	case 0x20c0c: case 0x20c10: case 0x20c14:
	case 0x50200: case 0x50204: case 0x50208: case 0x5020c: case 0x50210:
		keypad_update();
		break;
	}

	if (addr == 0x50420)
	{
		if (mem_mask == 0x0000ffff)
			m_cbus_complete = false;
		else if (mem_mask == 0xffff0000)
		{
			m_cbus_command = data >> 16;
			m_cbus_complete = BIT(m_cbus_command, 8);
			if (m_cbus_complete && ((m_cbus_command >> 5) & 7) == BLUETOOTH_CBUS_DEVICE)
				m_cbus_bt_data = m_bt->read(m_cbus_command & 0x1f);
		}
		return;
	}
	if (addr == 0x50424)
	{
		if (mem_mask == 0xffff0000 && !m_cbus_complete && !BIT(m_cbus_command, 8))
		{
			if (((m_cbus_command >> 5) & 7) == BLUETOOTH_CBUS_DEVICE)
				m_bt->write(m_cbus_command & 0x1f, data >> 16);
			else
				uem_w(u8(m_cbus_command), data >> 16);
			m_cbus_complete = true;
		}
		return;
	}

	if (addr >= 0x21200 && addr <= 0x21208 && mem_mask == 0xffffffff)
	{
		if (addr == 0x21204)
		{
			if (data == 2)
			{
				m_upp[0x21200 / 4] = 0;
				m_lcd_window_words = 0;
				m_lcd_pixel_header = false;
			}
		}
		else if (m_upp[0x21204 / 4] == 1)
		{
			if (addr == 0x21200)
				lcd_header_w(data);
			else
				lcd_data_w(data);
		}
		return;
	}

	if (addr >= 0x20100 && addr < 0x20280)
	{
		unsigned const plane = (addr - 0x20100) / 0xc0;
		unsigned const bank = ((addr - 0x20100) % 0xc0) / 0x40;
		u32 &enabled = plane ? m_fiq_enable[bank] : m_irq_enable[bank];
		if ((addr & 0x3f) == 8)
			enabled |= data & mem_mask;
		else if ((addr & 0x3f) == 12)
			enabled &= ~(data & mem_mask);
		else if (!plane && bank == 0 && (addr & 0x3f) == 0x30 && BIT(data & mem_mask, 1))
		{
			m_irq_pending[0] &= ~2;
			m_uem_irq = false;
		}
		update_interrupts();
	}
	if (addr >= 0x20300 && addr < 0x20500)
	{
		unsigned const channel = (addr - 0x20300) / 0x40;
		unsigned const reg = addr & 0x3f;
		if ((reg == 4 || reg == 8) && !m_upp[offset])
		{
			m_lcd_dma_timer[channel]->enable(false);
			m_upp[(0x20308 + channel * 0x40) / 4] = 0;
			m_irq_pending[0] &= ~(1U << (channel + 6));
			update_interrupts();
		}
		else if (reg == 8)
			dma_start(channel);
	}
	if (addr >= 0x20a00 && addr < 0x20ac0)
	{
		unsigned const n = (addr - 0x20a00) / 0x20;
		switch (addr & 0x1f)
		{
		case 0x04:
			if ((previous ^ m_upp[offset]) & 3)
				set_timer_count(n, timer_count(n));
			break;
		case 0x08:
			m_irq_pending[0] &= ~(1U << n);
			update_interrupts();
			break;
		case 0x10:
			m_upp[offset - 2] = 0;
			set_timer_count(n, m_upp[offset]);
			break;
		}
	}

	if (!m_upp_writes.test(offset))
	{
		m_upp_writes.set(offset);
		logerror("%s: UPP write %08x mask %08x = %08x\n",
			machine().describe_context(), UPP_BASE + addr, mem_mask, data);
	}
}

void ngage_state::lcd_header_w(u32 data)
{
	m_lcd_window_words = 0;
	if (data == 0x09240003 && (m_lcd_command == 0x15 || m_lcd_command == 0x75))
		m_lcd_window_words = 2;

	bool const valid_window = m_lcd_rows_valid && m_lcd_columns_valid;
	bool const full_upload = valid_window && m_lcd_band_first == 0 && data == 0x8924d67f
		&& (m_lcd_band_end == LCD_PAIRS || m_lcd_band_end == LCD_PAIRS - LCD_ROW_PAIRS);
	m_lcd_limit = full_upload ? LCD_PAIRS : m_lcd_band_end;
	m_lcd_pixel_header = full_upload || (valid_window && (data >> 16) == 0x8924
		&& ((data & 0xffff) + 1) == (m_lcd_band_end - m_lcd_band_first) * 3);
}

void ngage_state::lcd_data_w(u32 data)
{
	u32 const header = m_upp[0x21200 / 4];
	if (header == 0x09200000)
	{
		m_lcd_command = u8(data);
		m_lcd_write = false;
		switch (data)
		{
		case 0x15: m_lcd_columns_valid = false; break;
		case 0x75: m_lcd_rows_valid = false; break;
		case 0x5c:
			m_lcd_write = m_lcd_rows_valid && m_lcd_columns_valid;
			m_lcd_cursor = m_lcd_band_first;
			break;
		case 0xaf: m_lcd_on = true; break;
		}
	}
	else if (header == 0x09240003 && m_lcd_window_words)
	{
		if (--m_lcd_window_words)
			m_lcd_window_first = data;
		else if (m_lcd_command == 0x15)
			m_lcd_columns_valid = data == 0 && m_lcd_window_first == 0x0b100002;
		else
		{
			unsigned const start = m_lcd_window_first & 0xff;
			unsigned const end = (m_lcd_window_first >> 20) & 0xff;
			m_lcd_rows_valid = data == 0 && !(m_lcd_window_first & 0xf00fff00)
				&& start < end && end <= LCD_HEIGHT;
			if (m_lcd_rows_valid)
			{
				m_lcd_band_first = start * LCD_ROW_PAIRS;
				m_lcd_band_end = end * LCD_ROW_PAIRS;
			}
		}
	}
	else if (m_lcd_pixel_header && m_lcd_write && m_lcd_cursor < m_lcd_limit)
	{
		m_lcd_pixels[2 * m_lcd_cursor] = data & 0x0fff;
		m_lcd_pixels[2 * m_lcd_cursor + 1] = (data >> 16) & 0x0fff;
		if (++m_lcd_cursor == m_lcd_limit)
			m_lcd_write = false;
	}
}

u32 ngage_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	if (m_lcd_on)
	{
		for (int y = cliprect.top(); y <= cliprect.bottom(); ++y)
		{
			for (int x = cliprect.left(); x <= cliprect.right(); ++x)
			{
				u16 const color = m_lcd_pixels[y * LCD_WIDTH + x];
				bitmap.pix(y, x) = rgb_t(pal4bit(color >> 8), pal4bit(color >> 4), pal4bit(color));
			}
		}
	}
	else
		bitmap.fill(rgb_t::black(), cliprect);
	return 0;
}

void ngage_state::sti_w(offs_t offset, u8 data)
{
	if ((offset == 0 && data == 0xc0) || (offset == 0x1d0000 && data == 0xe0) || m_sti_line.size() == 256)
	{
		if (!m_sti_line.empty())
			logerror("STI: %s\n", m_sti_line);
		m_sti_line.clear();
	}
	if ((data >= 0x20 && data < 0x7f) || data == '\t')
		m_sti_line += char(data);
}

void ngage_state::nos_trace_w(u32 data, u32 mem_mask)
{
	for (unsigned lane = 0; lane < 4; ++lane)
	{
		if (!(mem_mask & (0xffU << (8 * lane))))
			continue;
		u8 const byte = data >> (8 * lane);
		if (!byte || byte == 0xc3 || m_nos_trace_line.size() == 256)
		{
			if (!m_nos_trace_line.empty())
				logerror("NOS: %s\n", m_nos_trace_line);
			m_nos_trace_line.clear();
		}
		if (byte >= 0x20 && byte < 0x7f)
			m_nos_trace_line += char(byte);
	}
}

void ngage_state::nem4_map(address_map &map)
{
	map(0x00000000, 0x007fffff).rw(FUNC(ngage_state::flash_r<0>), FUNC(ngage_state::flash_w<0>));
	map(0x02000000, 0x023fffff).r(FUNC(ngage_state::flash_r<1>));
	map(0x02400000, 0x027fffff).r(FUNC(ngage_state::cdrive_r));
	map(0x02000000, 0x027fffff).w(FUNC(ngage_state::flash_w<1>));
	map(0x04000000, 0x047fffff).rw(FUNC(ngage_state::flash_r<2>), FUNC(ngage_state::flash_w<2>));
	map(0x08000000, 0x08000fff).ram(); // External memory controller latches
	map(0x0a000000, 0x0affffff).ram();
	map(0x0c000000, 0x0c000fff).nopr(); // Undumped UPP ASIC ROM
	map(0x0c0003dc, 0x0c0003df).lr32(NAME([] () { return 0x47700000; })); // Thumb BX LR at 0x03de
	map(0x0c001000, 0x0c0fffff).ram(); // On-chip RAM, including bootstrap scratch and DSP upload
	map(0x0c100000, 0x0c1fffff).rw(FUNC(ngage_state::upp_r), FUNC(ngage_state::upp_w));
	map(0x0d6d0000, 0x0d6d0003).w(FUNC(ngage_state::nos_trace_w));
	map(0x0de00000, 0x0dffffff).w(FUNC(ngage_state::sti_w));
}

void ngage_state::nem4(machine_config &config)
{
	ARM920T(config, m_maincpu, ARM_SDRAM_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &ngage_state::nem4_map);
	NVRAM(config, m_pmm_nvram, nvram_device::DEFAULT_ALL_1);
	NVRAM(config, m_cdrive_nvram, nvram_device::DEFAULT_ALL_1);
	NGAGE_ADSP(config, m_adsp, 0);
	NGAGE_BT(config, m_bt, 0);
	m_bt->irq_cb().set(FUNC(ngage_state::bt_event));
	NGAGE_MMC(config, m_mmc, 8'666'667);
	NGAGE_RTC(config, m_rtc, 0);
	m_mmc->event_cb().set(FUNC(ngage_state::mmc_event));
	NGAGE_SIM(config, m_sim, 0);

	SPEAKER(config, "mono").front_center();
	NGAGE_PCM(config, m_pcm, 0).add_route(ALL_OUTPUTS, "mono", 1.0);
	m_pcm->reply_ready().set(FUNC(ngage_state::pcm_reply_ready));

	screen_device &screen(SCREEN(config, "screen").set_lcd());
	screen.set_raw(6'500'000, 400, 0, LCD_WIDTH, 262, 0, LCD_HEIGHT); // Timing from the FPGA model
	screen.set_screen_update(FUNC(ngage_state::screen_update));
}

INPUT_PORTS_START(ngage)
	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x00, "SIM Card")
	PORT_CONFSETTING(0x00, DEF_STR(None))
	PORT_CONFSETTING(0x01, "Test SIM")
	PORT_CONFNAME(0x0e, 0x00, "ARM / SDRAM Clock")
	PORT_CONFSETTING(0x00, "104 MHz (1x)")
	PORT_CONFSETTING(0x02, "156 MHz (1.5x)")
	PORT_CONFSETTING(0x04, "208 MHz (2x)")
	PORT_CONFSETTING(0x06, "260 MHz (2.5x)")
	PORT_CONFSETTING(0x08, "312 MHz (3x)")
	PORT_CONFNAME(BATTERY_LEVEL_MASK, 0x00, "Battery Level")
	PORT_CONFSETTING(0x00, "Full (7 bars)")
	PORT_CONFSETTING(0x10, "6 bars")
	PORT_CONFSETTING(0x20, "5 bars")
	PORT_CONFSETTING(0x30, "4 bars")
	PORT_CONFSETTING(0x40, "3 bars")
	PORT_CONFSETTING(0x50, "2 bars")
	PORT_CONFSETTING(0x60, "1 bar")

	PORT_START("KEYS")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_8WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 0)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 1)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 2)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 3)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("OK") PORT_CODE(KEYCODE_ENTER) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 4)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Left Soft") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 5)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Menu") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 6)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Clear") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 7)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Radio") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 8)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Right Soft") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 9)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 10)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("#") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 11)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 12)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 13)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 14)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 15)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 16)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 17)
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 18)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 19)
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 20)
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 21)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 22)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Send") PORT_CODE(KEYCODE_S) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 23)
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("End") PORT_CODE(KEYCODE_END) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 24)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Music") PORT_CODE(KEYCODE_C) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 25)
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Power") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(ngage_state::key_changed), 26)
INPUT_PORTS_END

ROM_START(ngage)
	ROM_REGION(0x1800000, "flash", 0)
	ROM_LOAD("nem4_403_cs0.bin", 0x0000000, 0x800000, CRC(848f1c33) SHA1(1c420c64b15579706e9c3374f6dafc863825e9d7))
	ROM_LOAD("nem4_403_cs1.bin", 0x0800000, 0x800000, CRC(8e8ace41) SHA1(33448698969d26af0984ae6a6716cfd9841c7fa5))
	ROM_LOAD("nem4_403_cs2.bin", 0x1000000, 0x800000, CRC(436711c7) SHA1(21141142b373098c0871a3daa10238c694766ef4))
ROM_END

} // anonymous namespace

SYST(2003, ngage, 0, 0, nem4, ngage, ngage_state, empty_init, "Nokia", "N-Gage (NEM-4, firmware 4.03)", MACHINE_NOT_WORKING)
