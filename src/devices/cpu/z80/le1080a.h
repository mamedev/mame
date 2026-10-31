// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/***************************************************************************

    L.E.Tech 'StarNovax' LE1080A-PA(V1) Security CPU device

    Z80-compatible CPU with additional internal features:

      - internal battery-backed RAM
      - system register block
      - internal interrupt controller
      - internal hardware RNG / random input source

***************************************************************************/

#ifndef MAME_CPU_Z80_LE1080A_H
#define MAME_CPU_Z80_LE1080A_H

#pragma once

#include <array>
#include <optional>

#include "z80.h"


// LE1080A interrupt input lines.
// IRQ0 / IRQ1 appear to be internal timer-driven interrupt sources.
// IRQ2 is treated as an external power-fail interrupt input.
// Current V1 observations suggest PFI/IRQ2 may be NMI-like in one important
// respect: software expects the pre-interrupt IFF2 state to remain available
// via LD A,I during the power-fail handler and restore path.
enum
{
	LE1080A_INPUT_LINE_IRQ0 = INPUT_LINE_IRQ0, // internal timer interrupt source
	LE1080A_INPUT_LINE_IRQ1,                   // internal timer interrupt source
	LE1080A_INPUT_LINE_IRQ2,                   // external power-fail interrupt input
};

// LE1080A is largely Z80 compatible but adds internal RAM, a system
// register block at FF00h-FFFFh, an internal interrupt controller and one
// or more pseudo-random / internal RNG data sources.
//
// Current analysis notes:
//   * 8000h-81FFh behaves like battery-backed state.
//   * FF00h-FF3Fh behaves more like volatile control/configuration state
//     that programs reinitialize at boot.
//   * Some manufacturers appear to use mixed-width system registers.
//     Yamasa V1 code performs 16-bit writes to locations such as FF1Ch,
//     FF25h, FF2Ah, FF2Ch, FF31h and FF3Eh.
//   * Some FFxx locations may also act as program-visible external I/O
//     windows rather than simple internal control bits.
//   * The original LE1080A method for configuring interrupt vector
//     addresses and interrupt periods remains unconfirmed.
class le1080a_device : public z80_device, public device_nvram_interface
{
public:
	le1080a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// Internal battery-backed RAM used by games for persistent state and
	// power-fail recovery data.
	static constexpr u32 INTERNAL_RAM_BASE = 0x8000;
	static constexpr u32 INTERNAL_RAM_SIZE = 0x0200;

	u8 *internal_ram_ptr() { return &m_internal_ram[0]; }

	static constexpr int IRQ_PULSE_WIDTH_USEC = 32;

	void set_irq0_vector(u8 vector) { m_irq0_vector = vector; }
	void set_irq1_vector(u8 vector) { m_irq1_vector = vector; }
	void set_irq2_vector(u8 vector) { m_irq2_vector = vector; }

	void trigger_power_fail();

	// Compatibility aliases for earlier driver-side setup code.
	void set_irq0_address(u16 addr) { m_irq0_vector = u8(addr); }
	void set_irq1_address(u16 addr) { m_irq1_vector = u8(addr); }
	void set_irq2_address(u16 addr) { m_irq2_vector = u8(addr); }

	void set_irq0_rate_hz(u32 hz) { m_irq0_rate_hz = hz; }
	void set_irq1_rate_hz(u32 hz) { m_irq1_rate_hz = hz; }

	void install_sysreg_handler(u8 start, u8 end, read8sm_delegate read, write8sm_delegate write);

	// Approximate LE1080A internal hardware RNG input source.
	// This intentionally keeps rolling internal state instead of returning
	// raw machine().rand() output directly.
	u8 internal_rng_r(offs_t offset);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	// Internal memory and system-register installation
	void install_internal_ram_and_sysregs();

	// System-register accessors
	u8   sysreg_r(offs_t offset);
	void sysreg_w(offs_t offset, u8 data);

	// Handlers for the currently unmapped LE1080A internal area (FF40h-FFFFh).
	// Accesses are logged to help identify additional internal registers.
	u8   unmapped_internal_r(offs_t offset);
	void unmapped_internal_w(offs_t offset, u8 data);

	// Interrupt input handling and timer sources
	void update_irq_attention();
	void pulse_irq_line(int line);
 
	bool irq0_enabled() const;
	bool irq1_enabled() const;
	bool irq2_enabled() const;

	TIMER_CALLBACK_MEMBER(irq0_tick);
	TIMER_CALLBACK_MEMBER(irq1_tick);
	TIMER_CALLBACK_MEMBER(irq0_clear);
	TIMER_CALLBACK_MEMBER(irq1_clear);
	TIMER_CALLBACK_MEMBER(irq2_clear);

	emu_timer *m_irq0_timer = nullptr;
	emu_timer *m_irq1_timer = nullptr;
	emu_timer *m_irq0_off_timer = nullptr;
	emu_timer *m_irq1_off_timer = nullptr;
	emu_timer *m_irq2_off_timer = nullptr;

	std::array<u8, INTERNAL_RAM_SIZE> m_internal_ram{};

	u8  m_irq0_vector = 0x38;
	u8  m_irq1_vector = 0x66;
	u8  m_irq2_vector = 0x00;

	u32 m_irq0_rate_hz = 0;
	u32 m_irq1_rate_hz = 0;

	int  m_irq0_line_state = CLEAR_LINE;
	int  m_irq1_line_state = CLEAR_LINE;
	int  m_irq2_line_state = CLEAR_LINE;

	// Internal rolling state for the LE1080A pseudo-random source.
	static constexpr int INTERNAL_RNG_INPUT_COUNT = 5;

	std::array<u8, INTERNAL_RNG_INPUT_COUNT> m_internal_rng_state{};
	u32 m_internal_rng_counter = 0;

	struct sysreg_entry
	{
		std::optional<read8sm_delegate> r;
		std::optional<write8sm_delegate> w;
	};
	std::array<sysreg_entry, 256> m_sysreg{};

	// Mirror of the currently known LE1080A system register block (FF00h-FF3Fh).
	std::array<u8, 256> m_sysreg_shadow{};
};

DECLARE_DEVICE_TYPE(LE1080A, le1080a_device)

#endif // MAME_CPU_Z80_LE1080A_H
