// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    JAFA Mode 7 Display Unit

**********************************************************************/

#ifndef MAME_BUS_ELECTRON_MODE7_H
#define MAME_BUS_ELECTRON_MODE7_H

#include "exp.h"
#include "video/mc6845.h"
#include "video/saa5050.h"
#include "screen.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class electron_mode7_device:
	public device_t,
	public device_electron_expansion_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	// construction/destruction
	electron_mode7_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(break_button);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t expbus_r(offs_t offset) override;
	virtual void expbus_w(offs_t offset, uint8_t data) override;

private:
	required_device<screen_device> m_screen;
	required_device<mc6845_device> m_hd6845;
	required_device<saa5050_device> m_trom;
	required_device<electron_expansion_slot_device> m_exp;
	required_memory_region m_exp_rom;

	std::unique_ptr<uint8_t[]> m_videoram;

	MC6845_UPDATE_ROW(crtc_update_row);
	void vsync_changed(int state);

	uint8_t m_romsel;
};


// device type definition
DECLARE_DEVICE_TYPE(ELECTRON_MODE7, electron_mode7_device)


#endif // MAME_BUS_ELECTRON_MODE7_H
