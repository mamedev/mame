// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD SuperCPU v2 + SuperRAM emulation

**********************************************************************/

#ifndef MAME_BUS_C64_SUPERCPU_H
#define MAME_BUS_C64_SUPERCPU_H

#pragma once

#include "exp.h"
#include "cpu/m6502/w65816.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_supercpu_device

class c64_supercpu_device : public device_t,
							public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_supercpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER( unit_changed );
	DECLARE_INPUT_CHANGED_MEMBER( speed_changed );

protected:
	c64_supercpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool c128);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_c64_expansion_card_interface overrides
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override;
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) override;
	virtual void c64_ba_w(int state) override;
	virtual void c64_phi2_w(int state) override;
	virtual void c64_irq_w(int state) override { host_line_w(0, state); }
	virtual void c64_nmi_w(int state) override { host_line_w(1, state); }

private:
	enum memory_region : uint8_t
	{
		SRAM_BANK0, SRAM_BANK1, C64_RAM, ULTIMAX, CART_ROML, CART_ROMH, IO, CHAR_ROM,
		EPROM, KERNAL_SHADOW_HW, KERNAL_SHADOW, COLOUR_RAM, COLOUR_RAM_INTERNAL,
		C128_RAM, C128_ROM_SHADOW, C128_BUS, MMU_REGISTERS, PROCESSOR_PORT, SUPERRAM, OPEN_BUS
	};

	enum class mmu_selection : uint8_t { SYSTEM_ROM, INTERNAL_FUNCTION, EXTERNAL_FUNCTION, RAM };
	enum { ACCESS_START, ACCESS_WAIT };
	static constexpr unsigned LINE_QUEUE = 16;
	struct long_read
	{
		memory_region area;
		offs_t address;
		offs_t mask;
	};

	required_device<w65816_device> m_maincpu;
	required_device<c64_expansion_slot_device> m_exp;
	required_region_ptr<uint8_t> m_rom;
	required_ioport m_front;
	required_ioport m_config;
	output_finder<> m_turbo_led;

	bool const m_is_c128_model;

	memory_share_creator<uint8_t> m_sram;
	memory_share_creator<uint8_t> m_simm;
	emu_timer *m_dma_timer;
	emu_timer *m_cycle_timer;

	// hardware registers
	bool m_sw_1mhz;
	bool m_soft_1mhz;
	bool m_sys_1mhz;
	bool m_hwenable;
	bool m_dosext;
	bool m_ramlink;
	bool m_bootmap;
	uint8_t m_optim;
	uint8_t m_simm_reg;
	uint8_t m_pport;

	// boot through the 6510
	bool m_boot;
	bool m_armed;
	bool m_ultimax;

	// bus ownership
	bool m_host_dma;
	bool m_exp_dma;
	bool m_dma;

	// C128 mode
	bool m_host_is_c128;
	bool m_2mhz;
	bool m_native_c128_mode;
	uint8_t m_mmu[11];
	uint8_t m_p0h_latch;
	uint8_t m_p1h_latch;

	// SuperRAM
	uint32_t m_simm_mask;
	int32_t m_simm_page;
	int32_t m_conf_page;
	uint32_t m_conf_size;
	uint32_t m_simm_cell;

	// bus timing
	// m_clk/m_accu locate the next access in host-cycle/20 MHz phase units.
	// m_dirty requests CPU catch-up; bus synchronization also waits for PHI2.
	// A deferred access keeps m_in_access and m_step so retries cannot repeat
	// waits or start the write buffer twice. Only end_access sets m_inc_pending,
	// which charges the completed 65816 cycle on the next begin_access.
	// Each timing path names its own stages, beginning at ACCESS_START.
	bool m_fast;
	uint64_t m_clk;
	uint32_t m_accu;
	uint64_t m_buffer_finish;
	uint32_t m_buffer_half;
	bool m_buffer_steal;
	bool m_dirty;
	bool m_ba_low;
	uint64_t m_ba_low_start;
	attotime m_phi2_time;
	bool m_in_access;
	bool m_inc_pending;
	int32_t m_step;

	// host IRQ/NMI transitions, by host cycle, until the 65816 reaches them
	emu_timer *m_wake_timer;
	uint64_t m_line_clk[LINE_QUEUE];
	uint8_t m_line_event[LINE_QUEUE];
	uint8_t m_line_count;
	uint8_t m_line_state;
	bool m_rdy;

	void c64_supercpu_map(address_map &map) ATTR_COLD;
	void c64_supercpu_vectors(address_map &map) ATTR_COLD;

	// host ownership
	TIMER_CALLBACK_MEMBER( assert_dma );
	void host_dma_w(bool state);
	bool bus_master() const { return !m_maincpu->suspended(SUSPEND_REASON_DISABLE); }
	void exp_dma_w(int state);
	uint8_t exp_dma_cd_r(offs_t offset);
	void exp_dma_cd_w(offs_t offset, uint8_t data);
	bool probe_c128_mmu();
	void start_maincpu();

	// configuration
	void update_simm(bool reset = false);
	bool slow_mode() const { return m_sys_1mhz || m_soft_1mhz || (m_sw_1mhz && !m_hwenable); }
	void update_speed() { set_fast(!slow_mode()); }
	void set_fast(bool fast);
	void set_2mhz(bool state);
	bool emulation_mode() { return m_maincpu->state_int(W65816_E); }

	// C64 memory mapping
	uint8_t config(offs_t offset);
	memory_region decode(offs_t offset, uint8_t cfg);
	void pport_w(uint8_t data) { m_pport = data & 7; }
	unsigned mirror_mode() const;
	bool mirrored(offs_t offset);

	// C128 MMU and memory mapping
	bool probe_adapter();
	void mmu_w(offs_t offset, uint8_t data);
	memory_region decode_c128(offs_t offset);
	mmu_selection mmu_memory(offs_t offset) const;
	offs_t bank1(offs_t offset);
	offs_t rom_shadow(offs_t offset, bool swap);
	offs_t ram_address(offs_t offset);
	bool mirrored_c128(offs_t offset, offs_t address);
	uint32_t simm_address(offs_t offset);

	// hardware registers
	uint8_t hardware_r(offs_t offset);
	void hardware_w(offs_t offset, uint8_t data);

	// bus timing
	attotime clk_time(uint64_t clk, uint32_t accu);
	void maincpu_inc();
	void advance_phase(uint32_t delta, bool wrap_at_boundary = false);
	void clock_inc();
	bool begin_access();
	void host_line_w(int line, int state);
	bool push_line(uint8_t event);
	bool pop_lines(bool waiting);
	bool line_visible(uint64_t clk) const;
	bool deliver_lines();
	void arm_wake();
	void rdy_w(int state);
	TIMER_CALLBACK_MEMBER( wake );
	void end_access() { m_in_access = false; m_inc_pending = true; }
	bool catch_up();
	bool sync();
	bool steal();
	bool wait_buffer();
	bool stretch_read_io();
	bool stretch_write_io_start();
	bool stretch_write_io_start_cia();
	void stretch_write_io(uint32_t accu);
	bool stretch_write();
	void stretch_io_2mhz();
	void stretch_cycles(int halves);
	void stretch_simm(offs_t offset, bool write);
	bool check_ba_read() { return m_dma || m_fast || steal(); }
	bool check_ba_write() { return m_dma || m_fast || emulation_mode() || steal(); }

	// host bus
	uint8_t bus_r(offs_t offset) { return m_slot->dma_cd_r(offset); }
	void bus_w(offs_t offset, uint8_t data) { m_slot->dma_cd_w(offset, data); }
	uint8_t io_bus_r(offs_t offset);
	void io_bus_w(offs_t offset, uint8_t data);

	// memory access
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	uint8_t bank0_r(offs_t offset);
	void bank0_w(offs_t offset, uint8_t data);
	offs_t bank0_address(offs_t offset, memory_region area);
	void bank0_c128_w(offs_t offset, uint8_t data);
	long_read decode_long(offs_t offset);
	uint8_t rom_r(offs_t offset);
	uint8_t simm_r(offs_t address, offs_t mask);
	uint8_t long_r(offs_t offset);
	void long_w(offs_t offset, uint8_t data);
	uint8_t cpu_r(offs_t offset);
	void cpu_w(offs_t offset, uint8_t data);
	uint8_t vector_r(offs_t offset);
	uint8_t peek(offs_t offset);
};


// ======================> c128_supercpu_device

class c128_supercpu_device : public c64_supercpu_device
{
public:
	// construction/destruction
	c128_supercpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(C64_SUPERCPU, c64_supercpu_device)
DECLARE_DEVICE_TYPE(C128_SUPERCPU, c128_supercpu_device)


#endif // MAME_BUS_C64_SUPERCPU_H
