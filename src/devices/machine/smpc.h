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



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SMPC_HLE_ADD(tag) \
		MCFG_DEVICE_ADD((tag), SMPC_HLE, (0))

#define MCFG_SMPC_HLE_PDR1_IN_CB(_devcb) \
	devcb = &smpc_hle_device::set_pdr1_in_handler(*device, DEVCB_##_devcb);

#define MCFG_SMPC_HLE_PDR2_IN_CB(_devcb) \
	devcb = &smpc_hle_device::set_pdr2_in_handler(*device, DEVCB_##_devcb);
		
#define MCFG_SMPC_HLE_PDR1_OUT_CB(_devcb) \
	devcb = &smpc_hle_device::set_pdr1_out_handler(*device, DEVCB_##_devcb);

#define MCFG_SMPC_HLE_PDR2_OUT_CB(_devcb) \
	devcb = &smpc_hle_device::set_pdr2_out_handler(*device, DEVCB_##_devcb);


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
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	// TODO: public stuff that should be internal to the device
	void sr_set(uint8_t data);
	void sr_ack();
	void sf_ack(bool cd_enable);
	void sf_set();
	bool get_iosel(bool which);
	uint8_t get_ddr(bool which);
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
	
	template <class Object> static devcb_base &set_pdr1_in_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_pdr1_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_pdr2_in_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_pdr2_read.set_callback(std::forward<Object>(cb)); }

	template <class Object> static devcb_base &set_pdr1_out_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_pdr1_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_pdr2_out_handler(device_t &device, Object &&cb) { return downcast<smpc_hle_device &>(device).m_pdr2_write.set_callback(std::forward<Object>(cb)); }

	
protected:
	// device-level overrides
//	virtual void device_validity_check(validity_checker &valid) const override;
//	virtual void device_add_mconfig() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	
private:

	const address_space_config      m_space_config;

	bool m_sf;
	bool m_cd_sf;
	uint8_t m_sr;
	uint8_t m_ddr1, m_ddr2;
	uint8_t m_pdr1_readback, m_pdr2_readback;
	bool m_iosel1, m_iosel2;
	bool m_exle1, m_exle2;
	devcb_read8 m_pdr1_read;
	devcb_read8 m_pdr2_read;
	devcb_write8 m_pdr1_write;
	devcb_write8 m_pdr2_write;
};


// device type definition
DECLARE_DEVICE_TYPE(SMPC_HLE, smpc_hle_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_SMPC_HLE_H

