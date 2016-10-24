// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
#include "imagedev/chd_cd.h"
#include "machine/gdrom.h"
#include "machine/ataintf.h"
#include "machine/intelfsh.h"

class dc_cons_state : public dc_state
{
public:
	dc_cons_state(const machine_config &mconfig, device_type type, const char *tag)
		: dc_state(mconfig, type, tag),
		m_ata(*this, "ata")
//        m_dcflash(*this, "dcflash")
		{ }

	required_device<ata_interface_device> m_ata;
//  required_device<macronix_29lv160tmc_device> m_dcflash;

	void init_dc();
	void init_dcus();
	void init_dcjp();

	uint64_t dcus_idle_skip_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t dcjp_idle_skip_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));

	void machine_reset_dc_console();
	uint64_t dc_pdtra_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void dc_pdtra_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t dc_arm_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void dc_arm_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	void aica_irq(int state);
	void sh4_aica_irq(int state);
	void ata_interrupt(int state);

	void atapi_xfer_end(void *ptr, int32_t param);

	void dreamcast_atapi_init();
	uint32_t dc_mess_g1_ctrl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void dc_mess_g1_ctrl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
//  uint8_t dc_flash_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
//  void dc_flash_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

private:
	uint64_t PDTRA, PCTRA;
	emu_timer *atapi_timer;
	int atapi_xferlen, atapi_xferbase;
};
