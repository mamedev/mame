// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    saa7191.h

    Philips SAA7191B Digital Multistandard Colour Decoder (DMSD)

    TODO:
    - Actual functionality

*********************************************************************/

#ifndef MAME_MACHINE_SAA7191_H
#define MAME_MACHINE_SAA7191_H

#pragma once

class saa7191_device : public device_t
{
public:
	saa7191_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	void i2c_data_w(uint8_t data);
	uint8_t i2c_data_r();
	void i2c_stop_w(int state);

	void iicsa_w(int state);

	auto chr_in() { return m_chr_in.bind(); }
	auto cvbs_in() { return m_cvbs_in.bind(); }

	auto y_out() { return m_y_out.bind(); }
	auto uv_out() { return m_uv_out.bind(); }
	auto hs_out() { return m_hs_out.bind(); }
	auto vs_out() { return m_vs_out.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void reg_w(uint8_t data);

	enum
	{
		REG_IDEL = 0x00,
		REG_HSYB = 0x01,
		REG_HSYS = 0x02,
		REG_HCLB = 0x03,
		REG_HCLS = 0x04,
		REG_HPHI = 0x05,
		REG_LUMC = 0x06,
		REG_HUEC = 0x07,
		REG_CKTQ = 0x08,
		REG_CKTS = 0x09,
		REG_PLSE = 0x0a,
		REG_SESE = 0x0b,
		REG_GAIN = 0x0c,
		REG_STDC = 0x0d,
		REG_IOCK = 0x0e,
		REG_CTL1 = 0x0f,
		REG_CTL2 = 0x10,
		REG_CHCV = 0x11,
		REG_HS6B = 0x14,
		REG_HS6S = 0x15,
		REG_HC6B = 0x16,
		REG_HC6S = 0x17,
		REG_HP6I = 0x18,
		REG_COUNT,

		LUMC_APER_SHIFT = 0,
		LUMC_APER_MASK = (3 << LUMC_APER_SHIFT),
		LUMC_CORI_SHIFT = 2,
		LUMC_CORI_MASK = (3 << LUMC_CORI_SHIFT),
		LUMC_BPSS_SHIFT = 4,
		LUMC_BPSS_MASK = (3 << LUMC_BPSS_SHIFT),
		LUMC_PREF_BIT = 6,
		LUMC_PREF_MASK = (1 << LUMC_PREF_BIT),
		LUMC_BYPS_BIT = 7,
		LUMC_BYPS_MASK = (1 << LUMC_BYPS_BIT),

		CKTQ_SHIFT = 3,
		CKTQ_MASK = (0x1f << CKTQ_SHIFT),
		CKTS_SHIFT = 3,
		CKTS_MASK = (0x1f << CKTS_SHIFT),

		GAIN_LFIS_SHIFT = 5,
		GAIN_LFIS_MASK = (3 << GAIN_LFIS_SHIFT),
		GAIN_COLO_BIT = 7,
		GAIN_COLO_MASK = (1 << GAIN_COLO_BIT),
		GAIN_MASK = GAIN_LFIS_MASK | GAIN_COLO_MASK,

		STDC_SECS_BIT = 0,
		STDC_SECS_MASK = (1 << STDC_SECS_BIT),
		STDC_GPSW0_BIT = 1,
		STDC_GPSW0_MASK = (1 << STDC_GPSW0_BIT),
		STDC_HRMV_BIT = 2,
		STDC_HRMV_MASK = (1 << STDC_HRMV_BIT),
		STDC_NFEN_BIT = 3,
		STDC_NFEN_MASK = (1 << STDC_NFEN_BIT),
		STDC_VTRC_BIT = 7,
		STDC_VTRC_MASK = (1 << STDC_VTRC_BIT),
		STDC_MASK = STDC_SECS_MASK | STDC_GPSW0_MASK | STDC_HRMV_MASK | STDC_NFEN_MASK | STDC_VTRC_MASK,

		IOCK_GPSW1_BIT = 0,
		IOCK_GPSW1_MASK = (1 << IOCK_GPSW1_BIT),
		IOCK_GPSW2_BIT = 1,
		IOCK_GPSW2_MASK = (1 << IOCK_GPSW2_BIT),
		IOCK_CHRS_BIT = 2,
		IOCK_CHRS_MASK = (1 << IOCK_CHRS_BIT),
		IOCK_OEDY_BIT = 3,
		IOCK_OEDY_MASK = (1 << IOCK_OEDY_BIT),
		IOCK_OEVS_BIT = 4,
		IOCK_OEVS_MASK = (1 << IOCK_OEVS_BIT),
		IOCK_OEHS_BIT = 5,
		IOCK_OEHS_MASK = (1 << IOCK_OEHS_BIT),
		IOCK_OEDC_BIT = 6,
		IOCK_OEDC_MASK = (1 << IOCK_OEDC_BIT),
		IOCK_HPLL_BIT = 7,
		IOCK_HPLL_MASK = (1 << IOCK_HPLL_BIT),

		CTL1_YDEL_SHIFT = 0,
		CTL1_YDEL_MASK = (7 << CTL1_YDEL_SHIFT),
		CTL1_OFTS_BIT = 3,
		CTL1_OFTS_MASK = (1 << CTL1_OFTS_BIT),
		CTL1_SCEN_BIT = 4,
		CTL1_SCEN_MASK = (1 << CTL1_SCEN_BIT),
		CTL1_SXCR_BIT = 5,
		CTL1_SXCR_MASK = (1 << CTL1_SXCR_BIT),
		CTL1_FSEL_BIT = 6,
		CTL1_FSEL_MASK = (1 << CTL1_FSEL_BIT),
		CTL1_AUFD_BIT = 7,
		CTL1_AUFD_MASK = (1 << CTL1_AUFD_BIT),

		CTL2_VNOI_SHIFT = 0,
		CTL2_VNOI_MASK = (3 << CTL2_VNOI_SHIFT),
		CTL2_HRFS_SHIFT = 2,
		CTL2_HRFS_MASK = (1 << CTL2_HRFS_SHIFT),
		CTL2_MASK = CTL2_VNOI_MASK | CTL2_HRFS_MASK
	};

	enum
	{
		I2C_STATE_IDLE,
		I2C_STATE_SUBADDR_READ,
		I2C_STATE_SUBADDR_WRITE,
		I2C_STATE_DATA_READ,
		I2C_STATE_DATA_WRITE
	};

	uint8_t m_status;
	uint8_t m_regs[REG_COUNT];

	uint8_t m_i2c_write_addr;
	uint8_t m_i2c_read_addr;
	uint8_t m_i2c_subaddr;
	int m_i2c_state;

	devcb_read8 m_chr_in;
	devcb_read8 m_cvbs_in;

	devcb_write8 m_y_out;
	devcb_write8 m_uv_out;
	devcb_write_line m_hs_out;
	devcb_write_line m_vs_out;
};

DECLARE_DEVICE_TYPE(SAA7191, saa7191_device)

#endif // MAME_MACHINE_SAA7191_H
