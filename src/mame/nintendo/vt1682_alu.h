// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_NINTENDO_VT1682_ALU_H
#define MAME_NINTENDO_VT1682_ALU_H

#pragma once

DECLARE_DEVICE_TYPE(VT_VT1682_ALU, vrt_vt1682_alu_device)

class vrt_vt1682_alu_device : public device_t
{
public:
	// construction/destruction
	vrt_vt1682_alu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// so that we can filter logging, sound ALU gets used hundreds of times a frame, so logging it is unwise
	void set_sound_alu() { m_is_sound_alu = true; }

	uint8_t alu_out_1_r();
	uint8_t alu_out_2_r();
	uint8_t alu_out_3_r();
	uint8_t alu_out_4_r();
	uint8_t alu_out_5_r();
	uint8_t alu_out_6_r();

	void alu_oprand_1_w(uint8_t data);
	void alu_oprand_2_w(uint8_t data);
	void alu_oprand_3_w(uint8_t data);
	void alu_oprand_4_w(uint8_t data);
	void alu_oprand_5_mult_w(uint8_t data);
	void alu_oprand_6_mult_w(uint8_t data);
	void alu_oprand_5_div_w(uint8_t data);
	void alu_oprand_6_div_w(uint8_t data);


protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	bool m_is_sound_alu = false;

	uint8_t m_alu_oprand[4];
	uint8_t m_alu_oprand_mult[2];
	uint8_t m_alu_oprand_div[2];
	uint8_t m_alu_out[6];
};

#endif // MAME_NINTENDO_VT1682_ALU_H
