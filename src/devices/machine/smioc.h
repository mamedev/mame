// license:GPL-2.0+
// copyright-holders:Brandon Munger
/**********************************************************************

    ROLM 9751 9005 System Monitor Input/Ouput Card emulation

**********************************************************************/

#ifndef MAME_MACHINE_SMIOC_H
#define MAME_MACHINE_SMIOC_H

#pragma once

#include "cpu/i86/i186.h"
#include "machine/am9517a.h"
#include "bus/rs232/rs232.h"
#include "machine/scc2698b.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> smioc_device

class smioc_device : public device_t
{
public:
	/* Constructor and Destructor */
	smioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &m68k_r_callback(device_t &device, Object &&cb) { return downcast<smioc_device &>(device).m_m68k_r_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &m68k_w_callback(device_t &device, Object &&cb) { return downcast<smioc_device &>(device).m_m68k_w_cb.set_callback(std::forward<Object>(cb)); }


	DECLARE_READ8_MEMBER(ram2_mmio_r);
	DECLARE_WRITE8_MEMBER(ram2_mmio_w);

	DECLARE_READ8_MEMBER(dma68k_r);
	DECLARE_WRITE8_MEMBER(dma68k_w);

	DECLARE_READ8_MEMBER(dma8237_2_dmaread);
	DECLARE_WRITE8_MEMBER(dma8237_2_dmawrite);

	DECLARE_READ8_MEMBER(boardlogic_mmio_r);
	DECLARE_WRITE8_MEMBER(boardlogic_mmio_w);


	u16 m_status;
	u16 m_status2;
	u16 m_wordcount;
	u16 m_wordcount2;

	u8 m_deviceBusy;

	u16 m_dmaSendAddress;
	u16 m_dmaSendLength;


	u8 m_requestFlags_11D;
	u16 m_commandValue;

	void SendCommand(u16 command);
	void ClearStatus();
	void ClearStatus2();

protected:
	/* Device-level overrides */
	virtual void device_start() override;
	virtual void device_reset() override;
	/* Optional information overrides */
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	/* Attached devices */
	required_device<i80188_cpu_device> m_smioccpu;

	required_device_array<am9517a_device, 5> m_dma8237;

	required_device<rs232_port_device> m_rs232_p1;
	required_device_array<rs232_port_device, 7> m_rs232_p;

	required_device<scc2698b_device> m_scc2698b;

	required_shared_ptr<uint8_t> m_smioc_ram;

	void smioc_mem(address_map &map);

	void update_and_log(u16& reg, u16 newValue, const char* register_name);

	emu_timer *m_dma_timer;

	devcb_read8 m_m68k_r_cb;
	devcb_write8 m_m68k_w_cb;
};

/* Device type */
DECLARE_DEVICE_TYPE(SMIOC, smioc_device)

/* MCFG defines */
#define MCFG_SMIOC_R_CB(_devcb) \
	devcb = &smioc_device::m68k_r_callback(*device, DEVCB_##_devcb);
#define MCFG_SMIOC_W_CB(_devcb) \
	devcb = &smioc_device::m68k_w_callback(*device, DEVCB_##_devcb);

#endif // MAME_MACHINE_SMIOC_H
