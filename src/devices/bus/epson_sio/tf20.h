// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    EPSON TF-20

    Dual 5.25" floppy drive with HX-20 factory option

**********************************************************************/

#ifndef MAME_BUS_EPSON_SIO_TF20_H
#define MAME_BUS_EPSON_SIO_TF20_H

#pragma once

#include "epson_sio.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/z80sio.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class epson_tf20_device : public device_t,
							public device_epson_sio_interface
{
public:
	// construction/destruction
	epson_tf20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_epson_sio_interface overrides
	virtual void tx_w(int level) override;
	virtual void pout_w(int level) override;

private:
	IRQ_CALLBACK_MEMBER( irq_callback );
	DECLARE_WRITE_LINE_MEMBER( txda_w );
	DECLARE_WRITE_LINE_MEMBER( dtra_w );

	// from sio output
	DECLARE_WRITE_LINE_MEMBER( rxc_w );
	DECLARE_WRITE_LINE_MEMBER( pinc_w );

	uint8_t rom_disable_r();
	uint8_t upd765_tc_r();
	void fdc_control_w(uint8_t data);

	TIMER_CALLBACK_MEMBER( serial_tick );
	TIMER_CALLBACK_MEMBER( tc_tick );

	void cpu_io(address_map &map);
	void cpu_mem(address_map &map);

	required_device<cpu_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<upd765a_device> m_fdc;
	required_device<upd7201_device> m_mpsc;
	required_device<epson_sio_device> m_sio_output;
	required_device_array<floppy_connector, 2> m_fd;

	emu_timer *m_timer_serial;
	emu_timer *m_timer_tc;

	int m_rxc;
	int m_txda;
	int m_dtra;
	int m_pinc;

	epson_sio_device *m_sio_input;

	static constexpr XTAL XTAL_CR1 = XTAL(8'000'000);
	static constexpr XTAL XTAL_CR2 = XTAL(4'915'200);
};


// device type definition
DECLARE_DEVICE_TYPE(EPSON_TF20, epson_tf20_device)


#endif // MAME_BUS_EPSON_SIO_TF20_H
