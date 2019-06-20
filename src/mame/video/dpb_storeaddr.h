// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_storeaddr.h
    DPB-7000/1 - Store Address Card

***************************************************************************/

#ifndef MAME_VIDEO_DPB_STOREADDR_H
#define MAME_VIDEO_DPB_STOREADDR_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dpb7000_storeaddr_card_device

class dpb7000_storeaddr_card_device : public device_t
{
public:
	// construction/destruction
	dpb7000_storeaddr_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void reg_w(uint16_t data);
	void s_type_w(int state);
	void cen_w(int state);

	void cxd_w(int state);
	void cxen_w(int state);
	void cxld_w(int state);
	void cxck_w(int state);
	void cxod_w(int state);
	void cxoen_w(int state);

	void cyd_w(int state);
	void cyen_w(int state);
	void cyld_w(int state);
	void cyck_w(int state);
	void cyod_w(int state);
	void cyoen_w(int state);

	void clrc_w(int state);
	void selvideo_w(int state);
	void creq_w(int state);
	void cr_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	uint16_t m_rhscr;
	uint16_t m_rvscr;
	uint16_t m_rzoom;
	uint16_t m_fld_sel;
	uint16_t m_cxpos;
	uint16_t m_cypos;

	int m_s_type;

	bool m_cen;

	bool m_cxd;
	bool m_cxen;
	bool m_cxld;
	bool m_cxck;
	bool m_cxod;
	bool m_cxoen;

	bool m_cyd;
	bool m_cyen;
	bool m_cyld;
	bool m_cyck;
	bool m_cyod;
	bool m_cyoen;

	bool m_clrc;
	bool m_selvideo;
	bool m_creq;
	bool m_cread;
};

// device type definition
DECLARE_DEVICE_TYPE(DPB7000_STOREADDR, dpb7000_storeaddr_card_device)

#endif // MAME_VIDEO_DPB_STOREADDR_H
