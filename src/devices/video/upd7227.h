// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    uPD7227 Intelligent Dot-Matrix LCD Controller/Driver emulation

**********************************************************************/

#ifndef MAME_VIDEO_UPD7227_H
#define MAME_VIDEO_UPD7227_H

#pragma once



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> upd7227_device

class upd7227_device : public device_t, public device_memory_interface
{
public:
	// construction/destruction
	upd7227_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	upd7227_device(const machine_config &mconfig, const char *tag, device_t *owner, int sx, int sy) : upd7227_device(mconfig, tag, owner, 0)
	{ set_offsets(sx, sy); }

	// inline configuration helpers
	void set_offsets(int sx, int sy) { m_sx = sx; m_sy = sy; }

	void cs_w(int state);
	void cd_w(int state);
	void sck_w(int state);
	void si_w(int state);
	int so_r();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	address_space_config        m_space_config;

private:
	enum
	{
		CMD_SMM         = 0x18,
		CMD_SFF         = 0x10,
		CMD_LDPI        = 0x80,
		CMD_SWM         = 0x64,
		CMD_SRM         = 0x60,
		CMD_SANDM       = 0x6c,
		CMD_SORM        = 0x68,
		CMD_SCM         = 0x72,
		CMD_BSET        = 0x40,
		CMD_BRESET      = 0x20,
		CMD_DISP_ON     = 0x09,
		CMD_DISP_OFF    = 0x08
	};

	int m_sx;
	int m_sy;

	int m_cs;
	int m_cd;
	int m_sck;
	int m_si;
	int m_so;

	void upd7227_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(UPD7227, upd7227_device)

#endif // MAME_VIDEO_UPD7227_H
