// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

  IBM PS/2 Planar FDC.

***************************************************************************/

#include "emu.h"
#include "planar_fdc.h"
#include "formats/pc_dsk.h"
#include "formats/naslite_dsk.h"
#include "formats/ibmxdf_dsk.h"

//#define VERBOSE 1
#include "logmacro.h"

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(MCA16_PLANAR_FDC, mca16_planar_fdc_device, "mca16_planar_fdc", "IBM PS/2 Planar FDC")

void mca16_planar_fdc_device::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
	fr.add(FLOPPY_IBMXDF_FORMAT);
}

static void pc_hd_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void mca16_planar_fdc_device::device_add_mconfig(machine_config &config)
{
	N82077AA(config, m_fdc, 24'000'000);
	m_fdc->set_mode(n82077aa_device::mode_t::PS2);
	m_fdc->intrq_wr_callback().set(FUNC(mca16_planar_fdc_device::irq_w));
	m_fdc->drq_wr_callback().set(FUNC(mca16_planar_fdc_device::drq_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", mca16_planar_fdc_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", mca16_planar_fdc_device::floppy_formats).enable_sound(true);
}

//-------------------------------------------------
//  isa8_com_device - constructor
//-------------------------------------------------

mca16_planar_fdc_device::mca16_planar_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mca16_planar_fdc_device(mconfig, MCA16_PLANAR_FDC, tag, owner, clock)
{
}

mca16_planar_fdc_device::mca16_planar_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_mca16_card_interface(mconfig, *this, 0xffff),
	m_fdc(*this, "fdc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mca16_planar_fdc_device::device_start()
{
	set_mca_device();

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mca16_planar_fdc_device::device_reset()
{
	m_fdc->reset();
}

void mca16_planar_fdc_device::enable()
{
	m_mca->install_device(0x3f0, 0x3f7,
		read8sm_delegate(*this, FUNC(mca16_planar_fdc_device::io8_r)),
    	write8sm_delegate(*this, FUNC(mca16_planar_fdc_device::io8_w)));
	m_mca->set_dma_channel(2, this, true);
}

void mca16_planar_fdc_device::disable()
{
	m_mca->unmap_device(0x3f0, 0x3f7);
}

void mca16_planar_fdc_device::irq_w(int state)
{
	LOG("%s: FDC: IRQ 6 now %d\n", FUNCNAME, state);
	m_mca->ireq_w<6>(state ? ASSERT_LINE : CLEAR_LINE);
}

void mca16_planar_fdc_device::drq_w(int state)
{
	LOG("%s: FDC: DMA 2 now %d\n", FUNCNAME, state);
	m_mca->dreq_w<2>(state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t mca16_planar_fdc_device::dack_r(int line)
{
	LOG("%s: line %d\n", FUNCNAME, line);
	return m_fdc->dma_r();
}

void mca16_planar_fdc_device::dack_w(int line, uint8_t data)
{
	LOG("%s: line %d\n", FUNCNAME, line);
	return m_fdc->dma_w(data);
}

void mca16_planar_fdc_device::eop_w(int state)
{
	LOG("%s: FDC: terminal count %d\n", FUNCNAME, state);
	m_fdc->tc_w(state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t mca16_planar_fdc_device::io8_r(offs_t offset)
{
	//assert_card_feedback();
	uint8_t data;
	
	switch(offset)
	{
		case 0: data = m_fdc->sra_r(); break;
		case 1: data = m_fdc->srb_r(); break;
		case 2: data = m_fdc->dor_r(); break;
		case 3: data = m_fdc->tdr_r(); break;
		case 4: data = m_fdc->msr_r(); break;
		case 5: data = m_fdc->fifo_r(); break;
		case 7: data = m_fdc->dir_r(); break;
		default: data = 0xFF; break;
	}

	//LOG("%s: O:%02X D:%02X\n", FUNCNAME, offset, data);

	return data;
}

void mca16_planar_fdc_device::io8_w(offs_t offset, uint8_t data)
{
	//LOG("%s: O:%02X D:%02X\n", FUNCNAME, offset, data);

	//assert_card_feedback();
	
	switch(offset)
	{
		case 2: m_fdc->dor_w(data); break;
		case 3: m_fdc->tdr_w(data); break;
		case 4: m_fdc->dsr_w(data); break;
		case 5: m_fdc->fifo_w(data); break;
		case 7: m_fdc->ccr_w(data); break;
		default: break;
	}
}