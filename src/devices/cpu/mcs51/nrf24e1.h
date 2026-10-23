// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_CPU_MCS51_NRF24E1_H
#define MAME_CPU_MCS51_NRF24E1_H

#pragma once

#include "i8052.h"
#include "machine/nrf2401.h"

class nrf24e1_device : public i8052_device
{
public:
	nrf24e1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	void set_eeprom_region(const char *tag) { m_eeprom.set_tag(*this, tag); }
	auto eeprom_read_cb() { return m_eeprom_read_cb.bind(); }
	template <unsigned Port> auto gpio_out_cb() { return m_gpio_out_cb[Port].bind(); }
	auto adc_read_cb() { return m_adc_read_cb.bind(); }
	auto spi_clock_cb() { return m_spi_clock_cb.bind(); }
	auto spi_mosi_cb() { return m_spi_mosi_cb.bind(); }
	auto spi_miso_cb() { return m_spi_miso_cb.bind(); }
	nrf2401_device &radio() { return *m_radio.lookup(); }
	void power_w(int state);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;

	// i8052_device implementation
	virtual void sfr_map(address_map &map) override ATTR_COLD;
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override;
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override;
	virtual u32 execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual offs_t external_ram_iaddr(offs_t offset, offs_t mem_mask) override;

private:
	// Offsets in the extended SFR backing store (address minus 0x80).
	enum : u8
	{
		SFR_DPL0 = 0x02,      // Data pointer 0 low byte
		SFR_DPH1 = 0x05,      // Data pointer 1 high byte
		SFR_DPS = 0x06,       // Active data pointer selection
		SFR_CKCON = 0x0e,     // Timer clock dividers and MOVX stretch
		SFR_EXIF = 0x11,      // ADC, SPI and radio interrupt flags
		SFR_MPAGE = 0x12,     // High address byte for MOVX through R0/R1
		SFR_P0_DIR = 0x14,    // Port 0 pin directions
		SFR_P0_ALT = 0x15,    // Port 0 alternate pin functions
		SFR_P1_DIR = 0x16,    // Port 1 pin directions
		SFR_P1_ALT = 0x17,    // Port 1 alternate pin functions
		SFR_ADCCON = 0x21,    // ADC channel, reference, power and conversion control
		SFR_ADCDATAH = 0x22,  // ADC result upper bits
		SFR_ADCDATAL = 0x23,  // ADC result lower bits and range flags
		SFR_ADCSTATIC = 0x24, // ADC resolution, clock, bias and input mode
		SFR_REGX_MSB = 0x2b,  // RTC/watchdog interface data high byte
		SFR_REGX_LSB = 0x2c,  // RTC/watchdog interface data low byte
		SFR_REGX_CTRL = 0x2d, // RTC/watchdog interface command and busy flag
		SFR_RSTREAS = 0x31,   // Power-on/watchdog reset reason
		SFR_SPI_DATA = 0x32,  // SPI transmit/receive byte
		SFR_SPI_CTRL = 0x33,  // SPI destination selection
		SFR_SPICLK = 0x34,    // SPI clock divider
		SFR_TICK_DV = 0x35,   // Low-power oscillator divider for RTC/watchdog ticks
		SFR_CK_CTRL = 0x36,   // Oscillator and CPU clock control
		SFR_EICON = 0x58,     // RTC wakeup interrupt flag
		SFR_EIE = 0x68,       // Extended interrupt enables
		SFR_EIP = 0x78,       // Extended interrupt priorities
		SFR_HWREV = 0x7e      // Hardware revision identifier
	};
	static const u8 INSTRUCTION_CYCLES[256];

	void ram_map(address_map &map) ATTR_COLD;
	void reset_registers();
	void bootstrap() ATTR_COLD;
	void sample_gpio();
	void check_interrupts();
	void advance_cycles(unsigned cycles);
	bool timer01_enabled(unsigned timer) const;
	void timer01_tick(unsigned timer);
	void timer2_tick(unsigned cycles);
	void clock_control_w(u8 data);
	u8 extra_r(offs_t offset);
	void extra_w(offs_t offset, u8 data);
	u8 gpio_r(unsigned port);
	void gpio_w(unsigned port, u8 data);
	void update_gpio(unsigned port);
	u8 gpio_drive_mask(unsigned port) const;
	u8 radio_r();
	void radio_w(u8 data);
	void adc_start(bool continuous = false);
	void spi_start(u8 data);
	void regx_start(u8 data);
	void radio_dr1_w(int state);
	void radio_dr2_w(int state);
	TIMER_CALLBACK_MEMBER(adc_complete);
	TIMER_CALLBACK_MEMBER(spi_tick);
	TIMER_CALLBACK_MEMBER(regx_complete);
	TIMER_CALLBACK_MEMBER(low_power_tick);

	required_device<nrf2401_device> m_radio;
	required_region_ptr<u8> m_eeprom;
	required_shared_ptr<u8> m_program_ram;
	devcb_write8::array<2> m_gpio_out_cb;
	devcb_read8 m_eeprom_read_cb;
	devcb_read16 m_adc_read_cb;
	devcb_write_line m_spi_clock_cb;
	devcb_write_line m_spi_mosi_cb;
	devcb_read_line m_spi_miso_cb;
	emu_timer *m_adc_timer;
	emu_timer *m_spi_timer;
	emu_timer *m_regx_timer;
	emu_timer *m_low_power_timer;

	u8 m_extra[128];
	u16 m_other_dptr;
	u8 m_timer_divider;
	u8 m_previous_gpio[2];
	u8 m_irq_delay;
	u16 m_adc_sample;
	u8 m_adc_resolution;
	u8 m_spi_tx;
	u8 m_spi_rx;
	u8 m_spi_edges;
	u8 m_spi_target;
	u8 m_spi_divider;
	u16 m_regx_data;
	u16 m_rtc_reload;
	u16 m_rtc_counter;
	u16 m_watchdog_counter;
	u16 m_tick_counter;
	u8 m_low_power_phase;
	bool m_rtc_enabled;
	bool m_watchdog_enabled;
	bool m_powered;
	bool m_booted;
	attotime m_adc_remaining;
	attotime m_spi_remaining;
};

DECLARE_DEVICE_TYPE(NRF24E1, nrf24e1_device)

#endif // MAME_CPU_MCS51_NRF24E1_H
