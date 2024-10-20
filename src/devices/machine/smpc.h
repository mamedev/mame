// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
// TODO: make separate device when code is decoupled better
//void stv_SMPC_w(offs_t offset, uint8_t data);
//uint8_t stv_SMPC_r(offs_t offset);
//void saturn_SMPC_w(offs_t offset, uint8_t data);
//uint8_t saturn_SMPC_r(offs_t offset);

#ifndef MAME_MACHINE_SMPC_HLE_H
#define MAME_MACHINE_SMPC_HLE_H

#pragma once

#include "screen.h"
#include "bus/sat_ctrl/ctrl.h"
#include "machine/nvram.h"
#include "dirtc.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> smpc_hle_device

class smpc_hle_device : public device_t,
						public device_memory_interface,
						public device_rtc_interface
{
public:
	// construction/destruction
	smpc_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
//  void io_map(address_map &map) ATTR_COLD;
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi_r );

	void m68k_reset_trigger();

	bool get_iosel(bool which);

	uint8_t get_ddr(bool which);

//  system delegation
	auto master_reset_handler() { return m_mshres.bind(); }

	auto master_nmi_handler() { return m_mshnmi.bind(); }

	auto slave_reset_handler() { return m_sshres.bind(); }

	auto sound_reset_handler() { return m_sndres.bind(); }

	auto system_reset_handler() { return m_sysres.bind(); }

	auto system_halt_handler() { return m_syshalt.bind(); }

	auto dot_select_handler() { return m_dotsel.bind(); }


//  PDR delegation
	auto pdr1_in_handler() { return m_pdr1_read.bind(); }

	auto pdr2_in_handler() { return m_pdr2_read.bind(); }

	auto pdr1_out_handler() { return m_pdr1_write.bind(); }

	auto pdr2_out_handler() { return m_pdr2_write.bind(); }

	// interrupt handler, doesn't work in Saturn driver???
	auto interrupt_handler() { return m_irq_line.bind(); }

	void set_region_code(uint8_t rgn) { m_region_code = rgn; }
	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename T, typename U> void set_control_port_tags(T &&tag1, U &&tag2)
	{
		m_ctrl1.set_tag(std::forward<T>(tag1));
		m_ctrl2.set_tag(std::forward<U>(tag2));
		// TODO: checking against nullptr still returns a device!?
		m_has_ctrl_ports = true;
	}

protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	// device_rtc_interface overrides
	virtual bool rtc_feature_y2k() const override { return true; }
	virtual bool rtc_feature_leap_year() const override { return true; }
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:

	const address_space_config      m_space_config;

	emu_timer *m_cmd_timer;
	emu_timer *m_rtc_timer;
	emu_timer *m_intback_timer;
	emu_timer *m_sndres_timer;
	bool m_has_ctrl_ports;

	bool m_sf;
	bool m_cd_sf;
	uint8_t m_sr;
	uint8_t m_ddr1, m_ddr2;
	uint8_t m_pdr1_readback, m_pdr2_readback;
	bool m_iosel1, m_iosel2;
	bool m_exle1, m_exle2;
	uint8_t m_ireg[7];
	uint8_t m_intback_buf[3];
	uint8_t m_oreg[32];
	uint8_t m_rtc_data[7];
	uint8_t m_smem[5];
	uint8_t m_comreg;
	// in usec
	// timing table, from manual in usec
	const uint32_t m_cmd_table_timing[0x20] =
	{
		30, 30, // MASTER ON / OFF
		30, 30, // SLAVE ON / OFF
		10, 10, // <unknown>
		30, 30, // SOUND ON / OFF
		40, 40, // CD ON / OFF
		30, 30, 30, // NETLINK ON / OFF / <unknown>
		100*1000, 100*1000, 100*1000, // SYSTEM RESET / ClocK CHaNGe 352 / 320
		320*1000, // INTBACK
		30, 30, 30, 30, 30, // <unknown>
		70, 40, 30, 30, 30, // SETTIME / SETSMEM / NMIREQ / RESENAB / RESDISA
		30, 30, 30, 30 // <unknown>
	};
	bool m_command_in_progress;
	bool m_NMI_reset;
	bool m_cur_dotsel;

	void master_sh2_nmi();
	void irq_request();

	void resolve_intback();
	TIMER_CALLBACK_MEMBER(intback_continue_request);
	TIMER_CALLBACK_MEMBER(handle_rtc_increment);
	TIMER_CALLBACK_MEMBER(sound_reset);
	TIMER_CALLBACK_MEMBER(handle_command);
	void read_saturn_ports();

	void sr_set(uint8_t data);
	void sr_ack();
	void sf_ack(bool cd_enable);
	void sf_set();
	int DectoBCD(int num);
	int m_intback_stage;
	int m_pmode;
	uint8_t m_region_code;

	required_device<nvram_device> m_mini_nvram;
	devcb_write_line m_mshres;
	devcb_write_line m_mshnmi;
	devcb_write_line m_sshres;
//  devcb_write_line m_sshnmi;
	devcb_write_line m_sndres;
	devcb_write_line m_sysres;
//  devcb_write_line m_cdres;
	devcb_write_line m_syshalt;
	devcb_write_line m_dotsel;
	devcb_read8 m_pdr1_read;
	devcb_read8 m_pdr2_read;
	devcb_write8 m_pdr1_write;
	devcb_write8 m_pdr2_write;
	devcb_write_line m_irq_line;
	optional_device<saturn_control_port_device> m_ctrl1;
	optional_device<saturn_control_port_device> m_ctrl2;

	required_device<screen_device> m_screen;

	void smpc_regs(address_map &map) ATTR_COLD;

	void ireg_w(offs_t offset, uint8_t data);
	void command_register_w(uint8_t data);
	uint8_t oreg_r(offs_t offset);
	uint8_t status_register_r();
	void status_flag_w(uint8_t data);
	uint8_t status_flag_r();
	uint8_t pdr1_r();
	uint8_t pdr2_r();
	void pdr1_w(uint8_t data);
	void pdr2_w(uint8_t data);
	void ddr1_w(uint8_t data);
	void ddr2_w(uint8_t data);
	void iosel_w(uint8_t data);
	void exle_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(SMPC_HLE, smpc_hle_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_SMPC_HLE_H

