// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Printer interface for ROM BASIC printer driver and compatibles

    Simulates IRPR protocol (inverted data lines, no /ACK signal).

    OSBK 4.1 settings: SET LP IRPR,BK10,NOTRANSL ; TIMER.SAV OFF

***************************************************************************/

#include "emu.h"
#include "printer.h"

#include "bus/centronics/ctronics.h"
#include "machine/output_latch.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bk_printer_device

class bk_printer_device : public device_t, public device_bk_parallel_interface
{
public:
	// construction/destruction
	bk_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD { m_data = 0; };

	virtual uint16_t io_r() override;
	virtual void io_w(uint16_t data, bool word) override;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;

	uint16_t m_data;
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_printer_device - constructor
//-------------------------------------------------

bk_printer_device::bk_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_PRINTER, tag, owner, clock)
	, device_bk_parallel_interface(mconfig, *this)
	, m_centronics(*this, "centronics")
	, m_cent_data_out(*this, "cent_data_out")
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bk_printer_device::device_add_mconfig(machine_config &config)
{
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_output_latch(OUTPUT_LATCH(config, m_cent_data_out));
	m_centronics->busy_handler().set([this] (int state) { m_data = (state << 8); });
}

void bk_printer_device::device_start()
{
	save_item(NAME(m_data));
}

uint16_t bk_printer_device::io_r()
{
	return m_data;
}

void bk_printer_device::io_w(uint16_t data, bool word)
{
	if (!BIT(data, 8)) m_cent_data_out->write(~data);
	m_centronics->write_strobe(BIT(data, 8));
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(BK_PRINTER, device_qbus_card_interface, bk_printer_device, "bk_printer", "BK Printer Interface")
