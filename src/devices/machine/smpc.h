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


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SMPC_HLE_ADD(tag, clock) \
		MCFG_DEVICE_ADD((tag), SMPC_HLE, (clock))

#define MCFG_SMPC_HLE_PDR1_IN_CB(_devcb) \
	devcb = &smpc_hle_device::set_pdr1_in_handler(*device, DEVCB_##_devcb);

#define MCFG_SMPC_HLE_PDR2_IN_CB(_devcb) \
	devcb = &smpc_hle_device::set_pdr2_in_handler(*device, DEVCB_##_devcb);
		
#define MCFG_SMPC_HLE_PDR1_OUT_CB(_devcb) \
	devcb = &smpc_hle_device::set_pdr1_out_handler(*device, DEVCB_##_devcb);

#define MCFG_SMPC_HLE_PDR2_OUT_CB(_devcb) \
	devcb = &smpc_hle_device::set_pdr2_out_handler(*device, DEVCB_##_devcb);

#define MCFG_SMPC_HLE_MASTER_RESET_CB(_devcb) \
	devcb = &smpc_hle_device::set_master_reset_handler(*device, DEVCB_##_devcb);

#define MCFG_SMPC_HLE_MASTER_NMI_CB(_devcb) \
	devcb = &smpc_hle_device::set_master_nmi_handler(*device, DEVCB_##_devcb);

#define MCFG_SMPC_HLE_SLAVE_RESET_CB(_devcb) \
	devcb = &smpc_hle_device::set_slave_reset_handler(*device, DEVCB_##_devcb);

#define MCFG_SMPC_HLE_SOUND_RESET_CB(_devcb) \
	devcb = &smpc_hle_device::set_sound_reset_handler(*device, DEVCB_##_devcb);

#define MCFG_SMPC_HLE_SYSTEM_RESET_CB(_devcb) \
	devcb = &smpc_hle_device::set_system_reset_handler(*device, DEVCB_##_devcb);

#define MCFG_SMPC_HLE_SYSTEM_HALT_CB(_devcb) \
	devcb = &smpc_hle_device::set_system_halt_handler(*device, DEVCB_##_devcb);

#define MCFG_SMPC_HLE_DOT_SELECT_CB(_devcb) \
	devcb = &smpc_hle_device::set_dot_select_handler(*device, DEVCB_##_devcb);

// set_irq_handler doesn't work in Saturn driver???
#define MCFG_SMPC_HLE_IRQ_HANDLER_CB(_devcb) \
	devcb = &smpc_hle_device::set_interrupt_handler(*device, DEVCB_##_devcb);


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

	virtual space_config_vector memory_space_config() const override;

	// I/O operations
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

	
	// TODO: public stuff & trampolines that should be internal to the device
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	void sr_set(uint8_t data);
	void sr_ack();
	void sf_ack(bool cd_enable);
	void sf_set();
	void master_sh2_reset(bool state);
	void master_sh2_nmi();
	void slave_sh2_reset(bool state);
	void sound_reset(bool state);
	void system_reset(bool state);
	void dot_select_request(bool state);
	void system_halt_request(bool state);
	void irq_request();
	bool get_nmi_status();
	
	bool get_iosel(bool which);
	uint8_t get_ddr(bool which);
	uint8_t get_ireg(uint8_t offset);
	void set_oreg(uint8_t offset,uint8_t data);
	
//	system delegation
	template <class Object> static devcb_base &set_master_reset_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_mshres.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_master_nmi_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_mshnmi.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_slave_reset_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_sshres.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_sound_reset_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_sndres.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_system_reset_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_sysres.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_system_halt_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_syshalt.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_dot_select_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_dotsel.set_callback(std::forward<Object>(cb)); }


//	PDR delegation
	template <class Object> static devcb_base &set_pdr1_in_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_pdr1_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_pdr2_in_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_pdr2_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_pdr1_out_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_pdr1_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_pdr2_out_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_pdr2_write.set_callback(std::forward<Object>(cb)); }

	// interrupt handler
	template <class Object> static devcb_base &set_interrupt_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_irq_line.set_callback(std::forward<Object>(cb)); }

	static void static_set_screentag(device_t &device, const char *tag);
	
protected:
	// device-level overrides
//	virtual void device_validity_check(validity_checker &valid) const override;
//	virtual void device_add_mconfig() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:

	enum {
		COMMAND_ID = 1,
		RTC_ID,
		INTBACK_ID
	};

	const address_space_config      m_space_config;
	emu_timer *m_cmd_timer;
	emu_timer *m_rtc_timer;
	emu_timer *m_intback_timer;

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
	void resolve_intback();
	void intback_continue_request();
	void handle_rtc_increment();
	int DectoBCD(int num);
	int m_intback_stage;
	int m_pmode;

	devcb_write_line m_mshres;
	devcb_write_line m_mshnmi;
	devcb_write_line m_sshres;
//	devcb_write_line m_sshnmi;
	devcb_write_line m_sndres;
	devcb_write_line m_sysres;
//	devcb_write_line m_cdres;
	devcb_write_line m_syshalt;
	devcb_write_line m_dotsel;
	devcb_read8 m_pdr1_read;
	devcb_read8 m_pdr2_read;
	devcb_write8 m_pdr1_write;
	devcb_write8 m_pdr2_write;
	devcb_write_line m_irq_line;
	
	screen_device *m_screen;
};


// device type definition
DECLARE_DEVICE_TYPE(SMPC_HLE, smpc_hle_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_SMPC_HLE_H

