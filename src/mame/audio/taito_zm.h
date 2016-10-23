// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
/***************************************************************************

    Taito Zoom ZSG-2 sound board

***************************************************************************/

#include "cpu/mn10200/mn10200.h"
#include "cpu/tms57002/tms57002.h"
#include "sound/zsg2.h"

ADDRESS_MAP_EXTERN(taitozoom_mn_map, 16);

class taito_zoom_device : public device_t

{
public:
	taito_zoom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~taito_zoom_device() {}

	void sound_irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sound_irq_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void reg_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void reg_address_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	uint8_t shared_ram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void shared_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tms_ctrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tms_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// inherited devices/pointers
	required_device<mn10200_device> m_soundcpu;
	required_device<zsg2_device> m_zsg2;

	// internal state
	uint16_t m_reg_address;
	uint8_t m_tms_ctrl;
	std::unique_ptr<uint8_t[]> m_snd_shared_ram;
};

extern const device_type TAITO_ZOOM;

MACHINE_CONFIG_EXTERN( taito_zoom_sound );

#define MCFG_TAITO_ZOOM_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TAITO_ZOOM, 0)
