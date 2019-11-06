// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cumana 68008 2nd processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Cumana_680082ndProcessor.html

**********************************************************************/


#ifndef MAME_BUS_BBC_INTERNAL_CUMANA68K_H
#define MAME_BUS_BBC_INTERNAL_CUMANA68K_H

#include "internal.h"
#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/mc146818.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "bus/scsi/scsi.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_cumana68k_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_cumana68k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER(write_sasi_sel);
	DECLARE_WRITE_LINE_MEMBER(write_sasi_msg);
	DECLARE_WRITE_LINE_MEMBER(write_sasi_bsy);
	DECLARE_WRITE_LINE_MEMBER(write_sasi_req);
	DECLARE_WRITE_LINE_MEMBER(write_sasi_io);
	DECLARE_WRITE_LINE_MEMBER(write_sasi_cd);
	DECLARE_READ8_MEMBER(pia_sasi_pa_r);
	DECLARE_WRITE8_MEMBER(pia_sasi_pa_w);
	DECLARE_READ8_MEMBER(pia_sasi_pb_r);
	DECLARE_READ_LINE_MEMBER(pia_sasi_cb1_r);
	DECLARE_WRITE8_MEMBER(pia_rtc_pb_w);

	DECLARE_READ8_MEMBER(dma_r);
	DECLARE_WRITE8_MEMBER(dma_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	DECLARE_WRITE_LINE_MEMBER(intrq_w);
	DECLARE_WRITE_LINE_MEMBER(drq_w);
	DECLARE_WRITE_LINE_MEMBER(reset68008_w);
	IRQ_CALLBACK_MEMBER(irq_callback);

	required_device<cpu_device> m_maincpu;
	required_device<m68000_base_device> m_m68008;
	required_device<pia6821_device> m_pia_sasi;
	required_device<pia6821_device> m_pia_rtc;
	required_device<input_merger_device> m_irqs;
	required_device<mc146818_device> m_rtc;
	required_device<wd2797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
	//required_device<scsi_port_device> m_scsibus;
	//required_device<output_latch_device> m_scsi_data_out;
	//required_device<input_buffer_device> m_scsi_data_in;

	void cumana68k_mem(address_map &map);

	bool m_bEnabled;
	uint8_t m_pia_sasi_pb;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_CUMANA68K, bbc_cumana68k_device)


#endif // MAME_BUS_BBC_INTERNAL_CUMANA68K_H
