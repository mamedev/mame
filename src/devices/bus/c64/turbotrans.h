// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rossmöller TurboTrans cartridge emulation

**********************************************************************/

#ifndef MAME_BUS_C64_TURBOTRANS_H
#define MAME_BUS_C64_TURBOTRANS_H

#pragma once

#include "bn1541.h"
#include "exp.h"
#include "machine/6522via.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_turbotrans_device

class c64_turbotrans_device : public device_t, public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_turbotrans_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_c64_expansion_card_interface implementation
	virtual uint8_t c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual void c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2) override;
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) override;
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) override;
	virtual void c64_ba_w(int state) override;
	virtual void c64_phi2_w(int state) override;
	virtual void c64_irq_w(int state) override;
	virtual void c64_nmi_w(int state) override;

private:
	class drive_port : public device_c64_floppy_parallel_interface
	{
	public:
		drive_port(const machine_config &mconfig, c64_turbotrans_device &host, int index) :
			device_c64_floppy_parallel_interface(mconfig, host), m_host(host), m_index(index) { }

		virtual void parallel_data_w(uint8_t data) override;
		virtual void parallel_strobe_w(int state) override;

	private:
		c64_turbotrans_device &m_host;
		int const m_index;
	};

	TIMER_CALLBACK_MEMBER(drive_data_sync);

	template <int Drive> void drive_data_w(uint8_t data);
	template <int Drive> void drive_strobe_w(int state);

	required_device<via6522_device> m_via;
	required_device<c64_expansion_slot_device> m_exp;
	required_memory_region m_kernal;
	required_ioport m_switch;
	drive_port m_drive[2];

	bool m_kernal_enable;
	u8 m_drive_data[2];
};


// device type definition
DECLARE_DEVICE_TYPE(C64_TURBOTRANS, c64_turbotrans_device)


#endif // MAME_BUS_C64_TURBOTRANS_H
