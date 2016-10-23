// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina

#include "seibucop/seibucop.h"



class seibu_cop_bootleg_device : public device_t,
								public device_memory_interface
{
public:
	seibu_cop_bootleg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t copdxbl_0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void copdxbl_0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint16_t reg_lo_addr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t reg_hi_addr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dist_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t angle_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void reg_lo_addr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void reg_hi_addr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cmd_trigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t d104_move_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void d104_move_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

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
	inline uint16_t read_word(offs_t address);
	inline void write_word(offs_t address, uint16_t data);

	uint32_t m_reg[8];
	uint16_t m_angle,m_dist,m_status;
	int m_dx,m_dy;
	uint32_t m_d104_move_offset;
	//required_device<raiden2cop_device> m_raiden2cop;
};

extern const device_type SEIBU_COP_BOOTLEG;

#define MCFG_DEVICE_SEIBUCOP_BOOTLEG_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEIBU_COP_BOOTLEG, 0)
