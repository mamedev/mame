// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Technology Research Beta Disk interface & clones

	There are multiple versions of this

	'hand made' PCB with V2 ROM and 1771 Disk controller
	https://www.youtube.com/watch?v=gSJIuZjbFYs

	Original Beta Disk release with V3 ROM (FORMAT, COPY etc. must
	be loaded from a disk to be used) uses a 1793 controller

	Re-release dubbed "Beta Disk plus" with V4 ROM (many operations
	moved into a larger capacity ROM rather than requiring a utility
	disk) also uses a 1793 controller?

	Many clones, some specific to the various Spectrum clones.

*********************************************************************/

#include "emu.h"
#include "beta.h"


/***************************************************************************
    DEVICE DEFINITIONS
***************************************************************************/

DEFINE_DEVICE_TYPE(SPECTRUM_BETA, spectrum_beta_device, "spectrum_beta", "TR Beta Disk Interface")

//-------------------------------------------------
//  SLOT_INTERFACE( beta_floppies )
//-------------------------------------------------

static void beta_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

//-------------------------------------------------
//  floppy_format_type floppy_formats
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER(spectrum_beta_device::floppy_formats)
	FLOPPY_TRD_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  ROM( beta )
//-------------------------------------------------

ROM_START(beta)
	ROM_REGION(0x4000, "rom", 0)
	ROM_DEFAULT_BIOS("trd20")
	ROM_SYSTEM_BIOS(0, "trd20", "TR-DOS v2.0")
	ROMX_LOAD("trd20.bin", 0x0000, 0x1000, CRC(dd269fb2) SHA1(ab394a19461f314fffd592645a273b85e76fadec), ROM_BIOS(0))
	ROM_RELOAD(0x1000,0x1000)
	ROM_RELOAD(0x2000,0x1000)
	ROM_RELOAD(0x3000,0x1000)
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void spectrum_beta_device::device_add_mconfig(machine_config &config)
{
	FD1793(config, m_fdc, 4_MHz_XTAL / 4);

	FLOPPY_CONNECTOR(config, "fdc:0", beta_floppies, "525qd", spectrum_beta_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", beta_floppies, "525qd", spectrum_beta_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", beta_floppies, nullptr, spectrum_beta_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", beta_floppies, nullptr, spectrum_beta_device::floppy_formats).enable_sound(true);

	// passthru
	SPECTRUM_EXPANSION_SLOT(config, m_exp, spectrum_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(spectrum_expansion_slot_device::nmi_w));
}

const tiny_rom_entry *spectrum_beta_device::device_rom_region() const
{
	return ROM_NAME(beta);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spectrum_beta_device - constructor
//-------------------------------------------------

spectrum_beta_device::spectrum_beta_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_spectrum_expansion_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0)
	, m_exp(*this, "exp")
{
}

spectrum_beta_device::spectrum_beta_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: spectrum_beta_device(mconfig, SPECTRUM_BETA, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spectrum_beta_device::device_start()
{
	save_item(NAME(m_romcs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spectrum_beta_device::device_reset()
{
	// always paged in on boot?
	m_romcs = 1;
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ_LINE_MEMBER(spectrum_beta_device::romcs)
{
	return m_romcs | m_exp->romcs();
}


void spectrum_beta_device::opcode_fetch(offs_t offset)
{
	m_exp->opcode_fetch(offset);

	if (!machine().side_effects_disabled())
	{
		if ((offset & 0xff00) == 0x3c00)
			m_romcs = 1;
	
		// how does the ROM get disabled on these older beta units
		// there are no RETs that end up in RAM as with the 128
		// so it looks like jumps to the 1xxx region, but that
		// doesn't work?
	}
}

uint8_t spectrum_beta_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

#if 0 // this is the Beta 128 logic, it may or may not be the same here
	if (m_romcs)
	{
		switch (offset & 0xff)
		{
		case 0x1f: case 0x3f: case 0x5f: case 0x7f:
			data = m_fdc->read((offset >> 5) & 0x03);
			break;

		case 0xff:
			data &= 0x3f; // actually open bus
			data |= m_fdc->drq_r() ? 0x40 : 0;
			data |= m_fdc->intrq_r() ? 0x80 : 0;
			break;
		}
	}
#endif

	return data;
}

void spectrum_beta_device::iorq_w(offs_t offset, uint8_t data)
{
#if 0 // this is the Beta 128 logic, it may or may not be the same here
	if (m_romcs)
	{
		switch (offset & 0xff)
		{
		case 0x1f: case 0x3f: case 0x5f: case 0x7f:
			m_fdc->write((offset >> 5) & 0x03, data);
			break;

		case 0xff:
			floppy_image_device* floppy = m_floppy[data & 3]->get_device();

			m_fdc->set_floppy(floppy);
			if (floppy)
				floppy->ss_w(BIT(data, 4) ? 0 : 1);
			m_fdc->dden_w(BIT(data, 6));

			// bit 3 connected to pin 23 "HLT" of FDC and via diode to INDEX
			//m_fdc->hlt_w(BIT(data, 3)); // not handled in current wd_fdc

			if (BIT(data, 2) == 0) // reset
			{
				m_fdc->reset();
				if (floppy)
					floppy->mon_w(ASSERT_LINE);
			}
			else
			{
				// TODO: implement correct motor control, FDD motor and RDY FDC pin controlled by HLD pin of FDC
				if (floppy)
				 floppy->mon_w(CLEAR_LINE);
			}
			break;
		}
	}
#endif 
	m_exp->iorq_w(offset, data);
}

uint8_t spectrum_beta_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_romcs)
	{
		data = m_rom->base()[offset & 0x3fff];
		data = bitswap<8>(data,0,6,5,4,3,2,1,7); // proper dumps have bits 0 and 7 swapped?
	}

	if (m_exp->romcs())
		data &= m_exp->mreq_r(offset);

	return data;
}

void spectrum_beta_device::mreq_w(offs_t offset, uint8_t data)
{
	if (m_exp->romcs())
		m_exp->mreq_w(offset, data);
}
