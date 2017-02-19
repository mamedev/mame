// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#pragma once

#ifndef INTERPRO_MCGA_H_
#define INTERPRO_MCGA_H_


// mcga control register
#define MCGA_CTRL_OPTMASK    0x00000003
#define MCGA_CTRL_CBITFRCRD  0x00000004
#define MCGA_CTRL_CBITFRCSUB 0x00000008
#define MCGA_CTRL_ENREFRESH  0x00000010
#define MCGA_CTRL_ENMSBE     0x00000100
#define MCGA_CTRL_ENMMBE     0x00000200
#define MCGA_CTRL_ENECC      0x00000400
#define MCGA_CTRL_WRPROT     0x00008000

// mcga error register
#define MCGA_ERROR_VALID    0x00008000

class interpro_mcga_device : public device_t
{
public:
	interpro_mcga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_ADDRESS_MAP(map, 32);

	DECLARE_WRITE16_MEMBER(write);
	DECLARE_READ16_MEMBER(read);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint16_t m_reg[32];
};

// device type definition
extern const device_type INTERPRO_MCGA;

#endif
