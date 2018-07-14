// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
// TODO: make separate device when code is decoupled better
//DECLARE_WRITE8_MEMBER( stv_SMPC_w );
//DECLARE_READ8_MEMBER( stv_SMPC_r );
//DECLARE_WRITE8_MEMBER( saturn_SMPC_w );
//DECLARE_READ8_MEMBER( saturn_SMPC_r );

#ifndef MAME_MACHINE_SMPC_HLE_H
#define MAME_MACHINE_SMPC_HLE_H

#pragma once

#include "screen.h"
#include "bus/sat_ctrl/ctrl.h"
#include "machine/nvram.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SMPC_HLE_ADD(tag, clock) \
	MCFG_DEVICE_ADD((tag), SMPC_HLE, (clock))

#define MCFG_SMPC_HLE_SCREEN(screen_tag) \
	downcast<smpc_hle_device &>(*device).set_screen_tag(screen_tag);

#define MCFG_SMPC_HLE_CONTROL_PORTS(ctrl1_tag, ctrl2_tag) \
	downcast<smpc_hle_device &>(*device).set_control_port_tags(ctrl1_tag, ctrl2_tag);

#define MCFG_SMPC_HLE_PDR1_IN_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_pdr1_in_handler(DEVCB_##_devcb);

#define MCFG_SMPC_HLE_PDR2_IN_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_pdr2_in_handler(DEVCB_##_devcb);

#define MCFG_SMPC_HLE_PDR1_OUT_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_pdr1_out_handler(DEVCB_##_devcb);

#define MCFG_SMPC_HLE_PDR2_OUT_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_pdr2_out_handler(DEVCB_##_devcb);

#define MCFG_SMPC_HLE_MASTER_RESET_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_master_reset_handler(DEVCB_##_devcb);

#define MCFG_SMPC_HLE_MASTER_NMI_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_master_nmi_handler(DEVCB_##_devcb);

#define MCFG_SMPC_HLE_SLAVE_RESET_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_slave_reset_handler(DEVCB_##_devcb);

#define MCFG_SMPC_HLE_SOUND_RESET_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_sound_reset_handler(DEVCB_##_devcb);

#define MCFG_SMPC_HLE_SYSTEM_RESET_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_system_reset_handler(DEVCB_##_devcb);

#define MCFG_SMPC_HLE_SYSTEM_HALT_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_system_halt_handler(DEVCB_##_devcb);

#define MCFG_SMPC_HLE_DOT_SELECT_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_dot_select_handler(DEVCB_##_devcb);

// set_irq_handler doesn't work in Saturn driver???
#define MCFG_SMPC_HLE_IRQ_HANDLER_CB(_devcb) \
	downcast<smpc_hle_device &>(*device).set_interrupt_handler(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> smpc_hle_device

class smpc_hle_device : public device_t,
						public device_memory_interface
{
public:
	// construction/destruction
	smpc_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
//  void io_map(address_map &map);
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_INPUT_CHANGED_MEMBER( trigger_nmi_r );

	void m68k_reset_trigger();

	bool get_iosel(bool which);

	uint8_t get_ddr(bool which);

//  system delegation
	template<class Object>
	devcb_base &set_master_reset_handler(Object &&cb)
	{ return m_mshres.set_callback(std::forward<Object>(cb)); }

	template<class Object>
	devcb_base &set_master_nmi_handler(Object &&cb)
	{ return m_mshnmi.set_callback(std::forward<Object>(cb)); }

	template<class Object>
	devcb_base &set_slave_reset_handler(Object &&cb)
	{ return m_sshres.set_callback(std::forward<Object>(cb)); }

	template<class Object>
	devcb_base &set_sound_reset_handler(Object &&cb)
	{ return m_sndres.set_callback(std::forward<Object>(cb)); }

	template<class Object>
	devcb_base &set_system_reset_handler(Object &&cb)
	{ return m_sysres.set_callback(std::forward<Object>(cb)); }

	template<class Object>
	devcb_base &set_system_halt_handler(Object &&cb)
	{ return m_syshalt.set_callback(std::forward<Object>(cb)); }

	template<class Object>
	devcb_base &set_dot_select_handler(Object &&cb)
	{ return m_dotsel.set_callback(std::forward<Object>(cb)); }


//  PDR delegation
	template<class Object>
	devcb_base &set_pdr1_in_handler(Object &&cb)
	{ return m_pdr1_read.set_callback(std::forward<Object>(cb)); }

	template<class Object>
	devcb_base &set_pdr2_in_handler(Object &&cb)
	{ return m_pdr2_read.set_callback(std::forward<Object>(cb)); }

	template<class Object>
	devcb_base &set_pdr1_out_handler(Object &&cb)
	{ return m_pdr1_write.set_callback(std::forward<Object>(cb)); }

	template<class Object>
	devcb_base &set_pdr2_out_handler(Object &&cb)
	{ return m_pdr2_write.set_callback(std::forward<Object>(cb)); }

	// interrupt handler
	template<class Object>
	devcb_base &set_interrupt_handler(Object &&cb)
	{ return m_irq_line.set_callback(std::forward<Object>(cb)); }

	void set_region_code(uint8_t rgn) { m_region_code = rgn; }
	void set_screen_tag(const char *tag) { m_screen.set_tag(tag); }
	void set_control_port_tags(const char *tag1, const char *tag2)
	{
		m_ctrl1_tag = tag1;
		m_ctrl2_tag = tag2;
		// TODO: checking against nullptr still returns a device!?
		m_has_ctrl_ports = true;
	}

protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual space_config_vector memory_space_config() const override;

private:

	const address_space_config      m_space_config;
	enum {
		COMMAND_ID = 1,
		RTC_ID,
		INTBACK_ID,
		SNDRES_ID
	};

	emu_timer *m_cmd_timer;
	emu_timer *m_rtc_timer;
	emu_timer *m_intback_timer;
	emu_timer *m_sndres_timer;
	const char *m_ctrl1_tag;
	const char *m_ctrl2_tag;
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
	void intback_continue_request();
	void handle_rtc_increment();
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
	saturn_control_port_device *m_ctrl1;
	saturn_control_port_device *m_ctrl2;

	required_device<screen_device> m_screen;

	void smpc_regs(address_map &map);

	DECLARE_WRITE8_MEMBER( ireg_w );
	DECLARE_WRITE8_MEMBER( command_register_w );
	DECLARE_READ8_MEMBER( oreg_r );
	DECLARE_READ8_MEMBER( status_register_r );
	DECLARE_WRITE8_MEMBER( status_flag_w );
	DECLARE_READ8_MEMBER( status_flag_r );
	DECLARE_READ8_MEMBER( pdr1_r );
	DECLARE_READ8_MEMBER( pdr2_r );
	DECLARE_WRITE8_MEMBER( pdr1_w );
	DECLARE_WRITE8_MEMBER( pdr2_w );
	DECLARE_WRITE8_MEMBER( ddr1_w );
	DECLARE_WRITE8_MEMBER( ddr2_w );
	DECLARE_WRITE8_MEMBER( iosel_w );
	DECLARE_WRITE8_MEMBER( exle_w );
};


// device type definition
DECLARE_DEVICE_TYPE(SMPC_HLE, smpc_hle_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_SMPC_HLE_H

