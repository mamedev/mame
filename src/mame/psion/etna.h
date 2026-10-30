// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Ash Wolf
/***************************************************************************

    Psion ASIC12 (ETNA) peripheral

****************************************************************************/

#ifndef MAME_PSION_ETNA_H
#define MAME_PSION_ETNA_H

#pragma once


class etna_device : public device_t
{
public:
	etna_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	auto porta_r() { return m_porta_r.bind(); }
	auto porta_w() { return m_porta_w.bind(); }

	void regs_w(offs_t offset, uint8_t data);
	uint8_t regs_r(offs_t offset);

	//void write_pc1_bvd1(int state) { set_input_level(PC1_BVD1, state); }
	//void write_pc1_bvd2(int state) { set_input_level(PC1_BVD2, state); }
	//void write_pc1_cd1(int state)  { set_input_level(PC1_CD1, state); }
	//void write_pc1_cd2(int state)  { set_input_level(PC1_CD2, state); }
	//void write_pc1_wp(int state)   { set_input_level(PC1_WP, state); }
	//void write_pc2_bvd1(int state) { set_input_level(PC2_BVD1, state); }
	//void write_pc2_bvd2(int state) { set_input_level(PC2_BVD2, state); }
	//void write_pc2_cd1(int state)  { set_input_level(PC2_CD1, state); }
	//void write_pc2_cd2(int state)  { set_input_level(PC2_CD2, state); }
	//void write_pc2_wp(int state)   { set_input_level(PC2_WP, state); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	//void set_input_level(uint16_t mask, int state);

	enum
	{
		REG_UNK0,
		REG_UNK1,
		REG_UART_INT_STATUS,
		REG_UART_INT_MASK,
		REG_UART_BAUD_LO,
		REG_UART_BAUD_HI,
		REG_PCCD_INT_STATUS,
		REG_PCCD_INT_MASK,
		REG_INT_CLEAR,
		REG_SKT_VAR_A0,
		REG_SKT_VAR_A1,
		REG_SKT_CTRL,
		REG_PORTA,
		REG_SKT_VAR_B0,
		REG_SKT_VAR_B1,
		REG_WAKE2
	};

	devcb_read8 m_porta_r;
	devcb_write8 m_porta_w;

	uint8_t m_pending_ints;

	uint8_t m_regs[0x10];
};

DECLARE_DEVICE_TYPE(ETNA, etna_device)

#endif // MAME_PSION_ETNA_H
