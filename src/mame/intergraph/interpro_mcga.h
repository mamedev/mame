// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_INTERGRAPH_INTERPRO_MCGA_H
#define MAME_INTERGRAPH_INTERPRO_MCGA_H

#pragma once

class interpro_mcga_device : public device_t
{
public:
	interpro_mcga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) ATTR_COLD;

	u16 reg00_r() { return m_reg[0]; }
	void reg00_w(u16 data) { m_reg[0] = data; }

	enum control_mask
	{
		CONTROL_OPTMASK    = 0x0003,
		CONTROL_CBITFRCRD  = 0x0004,
		CONTROL_CBITFRCSUB = 0x0008,
		CONTROL_ENREFRESH  = 0x0010,
		CONTROL_ENMSBE     = 0x0100,
		CONTROL_ENMMBE     = 0x0200,
		CONTROL_ENECC      = 0x0400,
		CONTROL_WRPROT     = 0x8000,

		CONTROL_MASK       = 0x871f
	};
	u16 control_r() { return m_control; }
	virtual void control_w(u16 data);

	enum error_mask
	{
		ERROR_SYND  = 0x00ff,
		ERROR_MMBE  = 0x0100,
		ERROR_MSBE  = 0x0200,
		ERROR_ADDR  = 0x1c00,
		ERROR_VALID = 0x8000
	};
	u16 error_r() { return m_error; }
	void error_w(u16 data) { m_error = data; }
	u8 frcrd_r() { return m_frcrd; }
	void frcrd_w(u8 data) { m_frcrd = data; }
	u8 cbsub_r() { return m_cbsub; }
	void cbsub_w(u8 data) { m_cbsub = data; }
	u16 reg28_r() { return m_reg[1]; }
	void reg28_w(u16 data) { m_reg[1] = data; }
	u16 reg30_r() { return m_reg[2]; }
	void reg30_w(u16 data) { m_reg[2] = data; }

	enum memsize_mask
	{
		MEMSIZE_ADDR = 0x007f
	};
	u16 memsize_r() { return m_memsize; }
	void memsize_w(u16 data) { m_memsize = data; }

protected:
	interpro_mcga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	u16 m_control = 0, m_error = 0, m_memsize = 0;
	u8 m_frcrd = 0, m_cbsub = 0;

	u16 m_reg[3]{};

private:

};

class interpro_fmcc_device : public interpro_mcga_device
{
public:
	interpro_fmcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;

	enum control_mask
	{
		CONTROL_CBITFRCRD  = 0x0004,
		CONTROL_CBITFRCSUB = 0x0008,
		CONTROL_ENREFRESH  = 0x0010,
		CONTROL_ENMSBENMI  = 0x0020,
		CONTROL_ENMMBENMI  = 0x0040,
		CONTROL_ENSTICKY   = 0x0080,
		CONTROL_ENMERR     = 0x0100,
		CONTROL_ENMMBE     = 0x0200,
		CONTROL_ENECC      = 0x0400,
		CONTROL_ENRMWCOR   = 0x0800,
		CONTROL_WRPROT     = 0x8000,

		CONTROL_MASK       = 0x8fff
	};
	void control_w(u16 data) override;

	enum error_mask
	{
		ERROR_SYND  = 0x00ff,
		ERROR_MMBE  = 0x0100,
		ERROR_MSBE  = 0x0200,
		ERROR_MTYPE = 0x0400,
		ERROR_VALID = 0x8000
	};

	enum error_control_mask
	{
		ERROR_CONTROL_CYCLE = 0x003f,
		ERROR_CONTROL_TAG   = 0x01c0
	};
	u16 error_control_r() { return m_error_control; }
	void error_control_w(u16 data) { m_error_control = data; }

private:
	u16 m_error_control = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(INTERPRO_MCGA, interpro_mcga_device)
DECLARE_DEVICE_TYPE(INTERPRO_FMCC, interpro_fmcc_device)

#endif // MAME_INTERGRAPH_INTERPRO_MCGA_H
