// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina
#ifndef MAME_MACHINE_SEICOP_H
#define MAME_MACHINE_SEICOP_H

#pragma once

#include "seibucop/seibucop.h"



class seibu_cop_bootleg_device : public device_t, public device_memory_interface
{
public:
	seibu_cop_bootleg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER( copdxbl_0_r );
	DECLARE_WRITE16_MEMBER( copdxbl_0_w );

	DECLARE_READ16_MEMBER( reg_lo_addr_r );
	DECLARE_READ16_MEMBER( reg_hi_addr_r );
	DECLARE_READ16_MEMBER( status_r );
	DECLARE_READ16_MEMBER( dist_r );
	DECLARE_READ16_MEMBER( angle_r );
	DECLARE_WRITE16_MEMBER( reg_lo_addr_w );
	DECLARE_WRITE16_MEMBER( reg_hi_addr_w );
	DECLARE_WRITE16_MEMBER( cmd_trigger_w );
	DECLARE_READ16_MEMBER( d104_move_r );
	DECLARE_WRITE16_MEMBER( d104_move_w );

	void seibucopbl_map(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual space_config_vector memory_space_config() const override;

private:
	cpu_device *m_host_cpu;      /**< reference to the host cpu */
	address_space *m_host_space;                            /**< reference to the host cpu space */
	const address_space_config      m_space_config;
	inline uint16_t read_word(offs_t address);
	inline void write_word(offs_t address, uint16_t data);

	uint32_t m_reg[8];
	uint16_t m_angle,m_dist,m_status;
	int m_dx,m_dy;
	uint32_t m_d104_move_offset;
	//required_device<raiden2cop_device> m_raiden2cop;
};

DECLARE_DEVICE_TYPE(SEIBU_COP_BOOTLEG, seibu_cop_bootleg_device)

#define MCFG_DEVICE_SEIBUCOP_BOOTLEG_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEIBU_COP_BOOTLEG, 0)

#endif // MAME_MACHINE_SEICOP_H
