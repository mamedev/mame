// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_INTERPRO_MCGA_H
#define MAME_MACHINE_INTERPRO_MCGA_H

#pragma once

// mcga control register
#define MCGA_CTRL_OPTMASK    0x0003
#define MCGA_CTRL_CBITFRCRD  0x0004
#define MCGA_CTRL_CBITFRCSUB 0x0008
#define MCGA_CTRL_ENREFRESH  0x0010
#define MCGA_CTRL_ENMSBE     0x0100
#define MCGA_CTRL_ENMMBE     0x0200
#define MCGA_CTRL_ENECC      0x0400
#define MCGA_CTRL_WRPROT     0x8000

// rom writes bit 0x80 to test if fmcc or mcga
#define MCGA_CTRL_MASK       0x871f
#define FMCC_CTRL_MASK       0x8fff

// mcga error register
#define MCGA_ERROR_VALID     0x00008000

class interpro_mcga_device : public device_t
{
public:
	interpro_mcga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_ADDRESS_MAP(map, 32);

	DECLARE_READ16_MEMBER(reg00_r) { return m_reg[0]; }
	DECLARE_WRITE16_MEMBER(reg00_w) { m_reg[0] = data; }
	DECLARE_READ16_MEMBER(control_r) { return m_control; }
	virtual DECLARE_WRITE16_MEMBER(control_w);
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

	virtual DECLARE_ADDRESS_MAP(map, 32) override;

	DECLARE_WRITE16_MEMBER(control_w) override;

	DECLARE_READ16_MEMBER(error_control_r) { return m_error_control; }
	DECLARE_WRITE16_MEMBER(error_control_w) { m_error_control = data; }

private:
	u16 m_error_control;
};

// device type definition
DECLARE_DEVICE_TYPE(INTERPRO_MCGA, interpro_mcga_device)
DECLARE_DEVICE_TYPE(INTERPRO_FMCC, interpro_fmcc_device)

#endif // MAME_MACHINE_INTERPRO_MCGA_H
