// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    dpb_framestore.h
    DPB-7000/1 - Framestore Card

***************************************************************************/

#ifndef MAME_VIDEO_DPB_FRAMESTORE_H
#define MAME_VIDEO_DPB_FRAMESTORE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dpb7000_framestore_card_device

class dpb7000_framestore_card_device : public device_t
{
public:
	// construction/destruction
	dpb7000_framestore_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void vopen_w(int state);
	void copen_w(int state);
	void csel_w(int state);
	void rck_w(int state);
	void cck_w(int state);

	void opstr_w(int state);
	void opwa_w(int state);
	void opwb_w(int state);
	void opra_w(int state);
	void oprb_w(int state);

	void ras_w(int state);
	void cas_w(int state);
	void write_w(int state);
	void ipen_w(int state);
	void whp_w(int state);
	void lumen_w(int state);
	void bdsel_w(int state);
	void ipsel_w(int state);

	void a_w(uint8_t data);
	void d_w(uint8_t data);
	void ra_w(uint8_t data);
	void wa_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	void update_cdata_latches(const uint8_t stripe);
	void update_cdata_out();
	void update_vdata_latches(const uint8_t stripe);
	void update_vdata_out();

	void update_pals();

	bool m_vopen;
	bool m_copen;
	bool m_csel;
	bool m_rck;
	bool m_cck;

	bool m_openx[5];
	bool m_opstr;
	bool m_opwa;
	bool m_opwb;
	uint8_t m_opw;
	bool m_opra;
	bool m_oprb;
	uint8_t m_opr;

	bool m_ras_in;
	bool m_ras[10];
	bool m_cas;
	bool m_write;
	bool m_ipen;
	bool m_whp;
	bool m_lumen;
	bool m_bdsel;
	uint8_t m_ipsel;

	bool m_ipenx[10];
	uint8_t m_rav[10];
	uint8_t m_cav;

	uint8_t *m_front_pal_base;
	uint16_t m_front_pal_addr;
	uint8_t m_front_pal_out;
	uint8_t *m_back_pal_base;
	uint16_t m_back_pal_addr;
	uint8_t m_back_pal_out;

	uint8_t m_a;
	uint8_t m_d_in[10][2];
	uint8_t m_d_out[10][4];
	uint8_t m_ra;
	uint8_t m_wa;
	uint8_t m_stripe;
	uint8_t m_cdata_front;
	uint8_t m_cdata_back;
	uint8_t m_cdata;
	uint8_t m_vdata_front;
	uint8_t m_vdata_back;
	uint8_t m_vdata;

	std::unique_ptr<uint8_t[]> m_stripes[10];

	// Output Lines
	devcb_write8 m_cdata_out;
	devcb_write8 m_vdata_out;
	devcb_write_line m_cbusy_out;

	// Devices
	required_memory_region m_back_pal;
	required_memory_region m_front_pal;
};

// device type definition
DECLARE_DEVICE_TYPE(DPB7000_FRAMESTORE, dpb7000_framestore_card_device)

#endif // MAME_VIDEO_DPB_FRAMESTORE_H
