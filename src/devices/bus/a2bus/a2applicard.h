// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2applicard.h

    Implementation of the PCPI AppliCard Z-80 card

*********************************************************************/

#ifndef __A2BUS_APPLICARD__
#define __A2BUS_APPLICARD__

#include "emu.h"
#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_applicard_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_applicard_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	a2bus_applicard_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	DECLARE_READ8_MEMBER( dma_r );
	DECLARE_WRITE8_MEMBER( dma_w );
	DECLARE_READ8_MEMBER( z80_io_r );
	DECLARE_WRITE8_MEMBER( z80_io_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const rom_entry *device_rom_region() const override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual bool take_c800() override;

	required_device<cpu_device> m_z80;

private:
	bool m_bROMAtZ80Zero;
	bool m_z80stat, m_6502stat;
	UINT8 m_toz80, m_to6502;
	UINT8 m_z80ram[64*1024];
	UINT8 *m_z80rom;
};

// device type definition
extern const device_type A2BUS_APPLICARD;

#endif /* __A2BUS_APPLICARD__ */
