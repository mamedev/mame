// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_SPEAKBOARD_H
#define MAME_BUS_PC98_CBUS_SPEAKBOARD_H

#include "slot.h"

#include "bus/msx/ctrl/ctrl.h"
#include "sound/ymopn.h"

class speakboard_device : public device_t
					    , public device_pc98_cbus_slot_interface
{
public:
	// construction/destruction
	speakboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;
	void io_map(address_map &map) ATTR_COLD;
	void opna_map(address_map &map) ATTR_COLD;
private:
	required_device<ym2608_device>  m_opna;
	required_memory_region m_bios;
	required_device<msx_general_purpose_port_device> m_joy;

	u8 m_joy_sel;
};

DECLARE_DEVICE_TYPE(SPEAKBOARD, speakboard_device)

#endif // MAME_BUS_PC98_CBUS_SPEAKBOARD_H
