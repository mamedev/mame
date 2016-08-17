// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina

#include "seibucop/seibucop.h"



class seibu_cop_bootleg_device : public device_t,
								public device_memory_interface
{
public:
	seibu_cop_bootleg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

private:
	cpu_device *m_host_cpu;      /**< reference to the host cpu */
	address_space *m_host_space;                            /**< reference to the host cpu space */
	const address_space_config      m_space_config;
	inline UINT16 read_word(offs_t address);
	inline void write_word(offs_t address, UINT16 data);

	UINT32 m_reg[8];
	UINT16 m_angle,m_dist,m_status;
	int m_dx,m_dy;
	UINT32 m_d104_move_offset;
	//required_device<raiden2cop_device> m_raiden2cop;
};

extern const device_type SEIBU_COP_BOOTLEG;

#define MCFG_DEVICE_SEIBUCOP_BOOTLEG_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEIBU_COP_BOOTLEG, 0)
