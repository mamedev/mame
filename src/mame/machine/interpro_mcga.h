// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_INTERPRO_MCGA_H
#define MAME_MACHINE_INTERPRO_MCGA_H

#pragma once

class interpro_mcga_device : public device_t
{
public:
	interpro_mcga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map);

	DECLARE_READ16_MEMBER(reg00_r) { return m_reg[0]; }
	DECLARE_WRITE16_MEMBER(reg00_w) { m_reg[0] = data; }

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
	DECLARE_READ16_MEMBER(control_r) { return m_control; }
	virtual DECLARE_WRITE16_MEMBER(control_w);

	enum error_mask
	{
		ERROR_SYND  = 0x00ff,
		ERROR_MMBE  = 0x0100,
		ERROR_MSBE  = 0x0200,
		ERROR_ADDR  = 0x1c00,
		ERROR_VALID = 0x8000
	};
	DECLARE_READ16_MEMBER(error_r) { return m_error; }
	DECLARE_WRITE16_MEMBER(error_w) { m_error = data; }
	DECLARE_READ8_MEMBER(frcrd_r) { return m_frcrd; }
	DECLARE_WRITE8_MEMBER(frcrd_w) { m_frcrd = data; }
	DECLARE_READ8_MEMBER(cbsub_r) { return m_cbsub; }
	DECLARE_WRITE8_MEMBER(cbsub_w) { m_cbsub = data; }
	DECLARE_READ16_MEMBER(reg28_r) { return m_reg[1]; }
	DECLARE_WRITE16_MEMBER(reg28_w) { m_reg[1] = data; }
	DECLARE_READ16_MEMBER(reg30_r) { return m_reg[2]; }
	DECLARE_WRITE16_MEMBER(reg30_w) { m_reg[2] = data; }

	enum memsize_mask
	{
		MEMSIZE_ADDR = 0x007f
	};
	DECLARE_READ16_MEMBER(memsize_r) { return m_memsize; }
	DECLARE_WRITE16_MEMBER(memsize_w) { m_memsize = data; }

protected:
	interpro_mcga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	u16 m_control, m_error, m_memsize;
	u8 m_frcrd, m_cbsub;

	u16 m_reg[3];

private:

};

class interpro_fmcc_device : public interpro_mcga_device
{
public:
	interpro_fmcc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

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
	DECLARE_WRITE16_MEMBER(control_w) override;

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
	DECLARE_READ16_MEMBER(error_control_r) { return m_error_control; }
	DECLARE_WRITE16_MEMBER(error_control_w) { m_error_control = data; }

private:
	u16 m_error_control;
};

// device type definition
DECLARE_DEVICE_TYPE(INTERPRO_MCGA, interpro_mcga_device)
DECLARE_DEVICE_TYPE(INTERPRO_FMCC, interpro_fmcc_device)

#endif // MAME_MACHINE_INTERPRO_MCGA_H
