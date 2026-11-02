// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    REX Data Technology ProLogic-DOS Classic emulation

**********************************************************************/

#ifndef MAME_BUS_C64_PROLOGICDOS_H
#define MAME_BUS_C64_PROLOGICDOS_H

#pragma once


#include "bn1541.h"
#include "exp.h"
#include "user.h"

#include "machine/6821pia.h"
#include "machine/input_merger.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_prologic_dos_classic_device

class c64_prologic_dos_classic_device : public device_t, public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_prologic_dos_classic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
		drive_port(const machine_config &mconfig, c64_prologic_dos_classic_device &host, int index) :
			device_c64_floppy_parallel_interface(mconfig, host), m_host(host), m_index(index) { }

		virtual void parallel_data_w(uint8_t data) override;
		virtual void parallel_strobe_w(int state) override;

	private:
		c64_prologic_dos_classic_device &m_host;
		int const m_index;
	};

	void pia_irq_w(int state) { m_slot->irq_w(state); }
	uint8_t pia_pa_r() { return m_drive_data[0] & m_drive_data[1]; }
	void pis_in_w(int drive, int state);
	TIMER_CALLBACK_MEMBER(drive_data_sync);
	uint8_t pia_pb_r() { return m_user_pb; }
	void pia_pb_w(uint8_t data);
	void pia_pa_w(uint8_t data);
	void pia_ca2_w(int state);

	template<int Bit> void pb_w(int state) { if (state) m_user_pb |= (1 << Bit); else m_user_pb &= ~(1 << Bit); }

	required_device<pia6821_device> m_pia;
	required_device<input_merger_all_high_device> m_pis_in;
	required_device<c64_expansion_slot_device> m_exp;
	required_device<pet_user_port_device> m_user;
	required_memory_region m_romh;
	required_ioport m_dip;

	drive_port m_drive[2];

	bool m_dip1;
	bool m_dip2;
	u8 m_user_pb;
	u8 m_drive_data[2];
};


// device type definition
DECLARE_DEVICE_TYPE(C64_PROLOGIC_DOS_CLASSIC, c64_prologic_dos_classic_device)


#endif // MAME_BUS_C64_PROLOGICDOS_H
