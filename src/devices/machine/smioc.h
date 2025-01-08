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

enum smioc_dma_parameter_t
{
	smiocdma_sendaddress=0, // Send to SMIOC - For Serial TX data
	smiocdma_sendlength,
	smiocdma_recvaddress, // Recv from SMIOC - For Serial RX data
	smiocdma_recvlength
};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> smioc_device

class smioc_device : public device_t
{
public:
	/* Constructor and Destructor */
	smioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto m68k_r_callback() { return m_m68k_r_cb.bind(); }
	auto m68k_w_callback() { return m_m68k_w_cb.bind(); }


	u8 ram2_mmio_r(offs_t offset);
	void ram2_mmio_w(offs_t offset, u8 data);

	u8 dma68k_r(offs_t offset);
	void dma68k_w(offs_t offset, u8 data);

	u8 dma8237_2_dmaread(offs_t offset);
	void dma8237_2_dmawrite(offs_t offset, u8 data);

	u8 boardlogic_mmio_r(offs_t offset);
	void boardlogic_mmio_w(offs_t offset, u8 data);


	int m_activePortIndex;

	u16 m_status;
	u16 m_status2;
	u16 m_shadowstatus; // RAM backing for SMIOC's status writes
	u16 m_shadowstatus2;
	bool m_statusvalid; // Status field has a valid value in it
	bool m_statusvalid2;
	bool m_statusrequest; // SMIOC has requested to queue a new status value
	bool m_statusrequest2;

	u16 m_wordcount;
	u16 m_wordcount2;

	u8 m_deviceBusy;

	u8 m_requestFlags_11D;
	u16 m_commandValue;
	u16 m_commandValue2;

	void SoftReset();

	void SendCommand(u16 command);
	void SendCommand2(u16 command);
	void SetCommandParameter(u16 parameter);
	void SetCommandParameter2(u16 parameter);
	u16 GetStatus();
	u16 GetStatus2();
	void ClearStatus();
	void ClearStatus2();
	void ClearParameter();
	void ClearParameter2();

	bool m_enable_hacky_status;
	int m_status_hack_counter;

	void AdvanceStatus();
	void AdvanceStatus2();

	void SetDmaParameter(smioc_dma_parameter_t param, u16 value);


	void WriteRamParameter(const char* function, const char* register_name, int address, int value);

protected:
	/* Device-level overrides */
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	/* Optional information overrides */
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(raise_drq);
	TIMER_CALLBACK_MEMBER(raise_int1);

	u16 ReadDmaParameter(smioc_dma_parameter_t param);
	int DmaParameterAddress(smioc_dma_parameter_t param);

private:
	/* Attached devices */
	required_device<i80188_cpu_device> m_smioccpu;

	required_device_array<am9517a_device, 5> m_dma8237;

	required_device_array<rs232_port_device, 8> m_rs232_p;

	required_device<scc2698b_device> m_scc2698b;

	required_shared_ptr<uint8_t> m_smioc_ram;

	u8 m_logic_ram[4096]; // 4kb of ram in the 0x4xxxx window, mainly used by the board's logic to proxy command parameters and data.

	void smioc_mem(address_map &map) ATTR_COLD;

	void update_and_log(u16& reg, u16 newValue, const char* register_name);

	emu_timer *m_dma_timer;
	emu_timer *m_451_timer;

	devcb_read8 m_m68k_r_cb;
	devcb_write8 m_m68k_w_cb;
};

/* Device type */
DECLARE_DEVICE_TYPE(SMIOC, smioc_device)

#endif // MAME_MACHINE_SMIOC_H
