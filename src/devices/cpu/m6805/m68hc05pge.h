// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_CPU_M6805_M68HC05PGE_H
#define MAME_CPU_M6805_M68HC05PGE_H

#pragma once

#include "emu.h"
#include "m6805.h"

class m68hc05pge_device : public m6805_base_device, public device_nvram_interface
{
public:
	const address_space_config m_program_config;

	static constexpr int PGE_PORTA = 0;
	static constexpr int PGE_PORTB = 1;
	static constexpr int PGE_PORTC = 2;
	static constexpr int PGE_PORTD = 3;
	static constexpr int PGE_PORTE = 4;
	static constexpr int PGE_PORTF = 5;
	static constexpr int PGE_PORTG = 6;
	static constexpr int PGE_PORTH = 7;
	static constexpr int PGE_PORTJ = 8;
	static constexpr int PGE_PORTK = 9;
	static constexpr int PGE_PORTL = 10;

	static constexpr int PGE_PWMA0 = 0;
	static constexpr int PGE_PWMA1 = 1;
	static constexpr int PGE_PWMB0 = 2;
	static constexpr int PGE_PWMB1 = 3;

	m68hc05pge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto read_tbX() { return m_read_tbX.bind(); }       // trackball X
	auto read_tbY() { return m_read_tbY.bind(); }       // trackball Y
	auto read_tbB() { return m_read_tbB.bind(); }       // trackball button
	template <std::size_t Bit> auto read_p() { return m_read_p[Bit].bind(); }
	template <std::size_t Bit> auto write_p() { return m_write_p[Bit].bind(); }
	template <std::size_t Bit> void set_pullups(u8 mask) { m_pullups[Bit] = mask; }

	template <std::size_t ad_port> auto ad_in() { return m_ad_in[ad_port].bind(); }

	template <std::size_t pwm> auto pwm_out() { return m_pwm_out[pwm].bind(); }

	auto spi_mosi_callback() { return write_spi_mosi.bind(); }
	auto spi_clock_callback() { return write_spi_clock.bind(); }
	void spi_miso_w(int state) { m_spi_miso = state; }

protected:
	// construction/destruction
	m68hc05pge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, address_map_constructor internal_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	virtual void interrupt_vector() override;

	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override;
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void m68hc05pge_map(address_map &map) ATTR_COLD;

	u8 ports_r(offs_t offset);
	void ports_w(offs_t offset, u8 data);
	u8 ddrs_r(offs_t offset);
	void ddrs_w(offs_t offset, u8 data);
	u8 read_port(u8 offset);
	void send_port(u8 offset, u8 data);
	u8 pll_r();
	void pll_w(u8 data);
	u8 spi_r(offs_t offset);
	void spi_w(offs_t offset, u8 data);
	u8 cpicsr_r();
	void cpicsr_w(u8 data);
	u8 cscr_r();
	void cscr_w(u8 data);
	u8 kcsr_r();
	void kcsr_w(u8 data);
	u8 trackball_r(offs_t offset);
	void trackball_w(offs_t offset, u8 data);
	u8 adb_r(offs_t offset);
	void adb_w(offs_t offset, u8 data);
	u8 option_r();
	void option_w(u8 data);
	u8 adc_r(offs_t offset);
	void adc_w(offs_t offset, u8 data);
	u8 ports_high_r(offs_t offset);
	void ports_high_w(offs_t offset, u8 data);
	u8 pwm_r(offs_t offset);
	void pwm_w(offs_t offset, u8 data);
	u8 plm_r(offs_t offset);
	void plm_w(offs_t offset, u8 data);
	u8 rtc_r(offs_t offset);
	void rtc_w(offs_t offset, u8 data);
	u8 sram_r(offs_t offset);
	void sram_w(offs_t offset, u8 data);

	TIMER_CALLBACK_MEMBER(seconds_tick);
	TIMER_CALLBACK_MEMBER(cpi_tick);
	TIMER_CALLBACK_MEMBER(spi_tick);
	TIMER_CALLBACK_MEMBER(adb_tick);

	required_shared_ptr<u8> m_internal_ram;

	memory_view m_introm;

	devcb_read8 m_read_tbX, m_read_tbY;
	devcb_read_line m_read_tbB;
	devcb_read8::array<11>  m_read_p;
	devcb_write8::array<11> m_write_p;

	devcb_read8::array<16> m_ad_in;

	devcb_write8::array<4> m_pwm_out;

	devcb_write_line write_spi_mosi, write_spi_clock;

	u8 m_ports[11], m_ddrs[11], m_pullups[11];
	u8 m_pll_ctrl;
	u8 m_timer_ctrl;
	u8 m_onesec;
	u8 m_option, m_cscr;
	u8 m_spi_in, m_spi_out;
	int m_spi_bit, m_spi_clock, m_spi_miso;
	u8 m_spcr, m_spsr;
	u8 m_cpicsr;
	u8 m_adcsr;
	u8 m_adbcr, m_adbsr, m_adbdr;
	u8 m_tbcs;
	u8 m_pwmacr, m_pwma0, m_pwma1;
	u8 m_pwmbcr, m_pwmb0, m_pwmb1;
	u8 m_plmcr, m_plmt1, m_plmt2;
	emu_timer *m_seconds_timer, *m_cpi_timer, *m_spi_timer, *m_adb_timer;
	u32 m_rtc;
	u8 m_sram[0x8000];
};

DECLARE_DEVICE_TYPE(M68HC05PGE, m68hc05pge_device)

#endif // MAME_CPU_M6805_M58HC05PGE_H
