// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Acorn FileStore E01/E01S network hard disk emulation

**********************************************************************/

#pragma once

#ifndef __E01__
#define __E01__

#include "econet.h"
#include "bus/centronics/ctronics.h"
#include "bus/scsi/scsi.h"
#include "cpu/m6502/m65c02.h"
#include "machine/6522via.h"
#include "machine/buffer.h"
#include "machine/latch.h"
#include "machine/mc146818.h"
#include "machine/mc6854.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"

class e01_device : public device_t,
	public device_econet_interface
{
public:
	// construction/destruction
	e01_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	e01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	enum
	{
		TYPE_E01 = 0,
		TYPE_E01S
	};

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( ram_select_r );
	DECLARE_WRITE8_MEMBER( floppy_w );
	DECLARE_READ8_MEMBER( network_irq_disable_r );
	DECLARE_WRITE8_MEMBER( network_irq_disable_w );
	DECLARE_READ8_MEMBER( network_irq_enable_r );
	DECLARE_WRITE8_MEMBER( network_irq_enable_w );
	DECLARE_READ8_MEMBER( hdc_data_r );
	DECLARE_WRITE8_MEMBER( hdc_data_w );
	DECLARE_READ8_MEMBER( hdc_status_r );
	DECLARE_WRITE8_MEMBER( hdc_select_w );
	DECLARE_WRITE8_MEMBER( hdc_irq_enable_w );
	DECLARE_READ8_MEMBER( rtc_address_r );
	DECLARE_WRITE8_MEMBER( rtc_address_w );
	DECLARE_READ8_MEMBER( rtc_data_r );
	DECLARE_WRITE8_MEMBER( rtc_data_w );
	DECLARE_WRITE_LINE_MEMBER( rtc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( adlc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( econet_data_w );
	DECLARE_WRITE_LINE_MEMBER( via_irq_w );
	DECLARE_WRITE_LINE_MEMBER( clk_en_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_bsy_w );
	DECLARE_WRITE_LINE_MEMBER( scsi_req_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// device_econet_interface overrides
	virtual void econet_data(int state) override;
	virtual void econet_clk(int state) override;

	required_device<m65c02_device> m_maincpu;
	required_device<wd2793_t> m_fdc;
	required_device<mc6854_device> m_adlc;
	required_device<mc146818_device> m_rtc;
	required_device<ram_device> m_ram;
	required_device<SCSI_PORT_DEVICE> m_scsibus;
	required_device<output_latch_device> m_scsi_data_out;
	required_device<input_buffer_device> m_scsi_data_in;
	required_device<input_buffer_device> m_scsi_ctrl_in;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_memory_region m_rom;
	required_device<centronics_device> m_centronics;

	inline void update_interrupts();
	inline void network_irq_enable(int enabled);
	inline void hdc_irq_enable(int enabled);

	// interrupt state
	int m_adlc_ie;
	int m_hdc_ie;
	int m_rtc_irq;
	int m_via_irq;
	int m_hdc_irq;
	int m_fdc_irq;
	bool m_fdc_drq;
	int m_adlc_irq;
	int m_clk_en;
	bool m_ram_en;

	int m_variant;

	// timers
	emu_timer *m_clk_timer;
};


// ======================> e01s_device

class e01s_device :  public e01_device
{
public:
	// construction/destruction
	e01s_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type E01;
extern const device_type E01S;



#endif
