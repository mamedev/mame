// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Archimedes Expansion Bus emulation

**********************************************************************/

#include "emu.h"
#include "slot.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ARCHIMEDES_PODULE_SLOT, archimedes_podule_slot_device, "archimedes_exp_slot", "Acorn Archimedes Podule slot")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  archimedes_podule_slot_device - constructor
//-------------------------------------------------
archimedes_podule_slot_device::archimedes_podule_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARCHIMEDES_PODULE_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_archimedes_podule_interface>(mconfig, *this)
	, m_exp(*this, finder_base::DUMMY_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void archimedes_podule_slot_device::device_resolve_objects()
{
	device_archimedes_podule_interface *const intf(get_card_device());
	if (intf)
		intf->set_archimedes_exp(m_exp, tag());
}

void archimedes_podule_slot_device::device_start()
{
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ARCHIMEDES_EXPANSION_BUS, archimedes_exp_device, "archimedes_exp", "Acorn Archimedes Expansion Bus")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  archimedes_exp_device - constructor
//-------------------------------------------------

archimedes_exp_device::archimedes_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARCHIMEDES_EXPANSION_BUS, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_ioc_config("podule_ioc", ENDIANNESS_LITTLE, 32, 16, 0, address_map_constructor(FUNC(archimedes_exp_device::ioc_map), this))
	, m_memc_config("podule_memc", ENDIANNESS_LITTLE, 32, 16, 0, address_map_constructor(FUNC(archimedes_exp_device::memc_map), this))
	, m_out_pirq_cb(*this)
	, m_out_pfiq_cb(*this)
{
}

device_memory_interface::space_config_vector archimedes_exp_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_memc_config),
		std::make_pair(AS_IO, &m_ioc_config)
	};
}

void archimedes_exp_device::ioc_map(address_map &map)
{
	map.unmap_value_high();
}

void archimedes_exp_device::memc_map(address_map &map)
{
	map.unmap_value_high();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void archimedes_exp_device::device_resolve_objects()
{
	// resolve callbacks
	m_out_pirq_cb.resolve_safe();
	m_out_pfiq_cb.resolve_safe();
}

void archimedes_exp_device::device_start()
{
	m_ioc = &space(AS_IO);
	m_memc = &space(AS_PROGRAM);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void archimedes_exp_device::device_reset()
{
	m_pirq_state = 0x00;
	m_pirq_mask = 0xff;
}

void archimedes_exp_device::pirq_w(int state, int slot)
{
	if (state == ASSERT_LINE)
	{
		m_pirq_state |= (1 << slot);
	}
	else
	{
		m_pirq_state &= ~(1 << slot);
	}

	m_out_pirq_cb(m_pirq_state & m_pirq_mask ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  ps - simple podule select
//-------------------------------------------------

u16 archimedes_exp_device::ps4_r(offs_t offset, u16 mem_mask)
{
	return m_ioc->read_word(offset << 2, mem_mask);
}

void archimedes_exp_device::ps4_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_ioc->write_word(offset << 2, data, mem_mask);
}


u16 archimedes_exp_device::ps6_r(offs_t offset, u16 mem_mask)
{
	switch ((offset << 2) & 0xfc)
	{
	case 0x00:
		return m_pirq_state & m_pirq_mask;

	case 0x04:
		return m_pirq_mask;
	}

	return 0xffff;
}

void archimedes_exp_device::ps6_w(offs_t offset, u16 data, u16 mem_mask)
{
	switch ((offset << 2) & 0xfc)
	{
	case 0x00:
		break;

	case 0x04:
		m_pirq_mask = data & 0xff;
		m_out_pirq_cb(m_pirq_state & m_pirq_mask ? ASSERT_LINE : CLEAR_LINE);
		break;
	}
}

//-------------------------------------------------
//  ms - memc select
//-------------------------------------------------

u16 archimedes_exp_device::ms_r(offs_t offset, u16 mem_mask)
{
	return m_memc->read_word(offset << 2, mem_mask);
}

void archimedes_exp_device::ms_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_memc->write_word(offset << 2, data, mem_mask);
}


//**************************************************************************
//  DEVICE ARCHIMEDES EXPANSION INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_archimedes_podule_interface - constructor
//-------------------------------------------------

device_archimedes_podule_interface::device_archimedes_podule_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "arcexp")
	, m_exp(nullptr)
	, m_exp_slottag(nullptr)
	, m_slot(-1)
{
}


//-------------------------------------------------
//  ~device_archimedes_podule_interface - destructor
//-------------------------------------------------

device_archimedes_podule_interface::~device_archimedes_podule_interface()
{
}


void device_archimedes_podule_interface::interface_pre_start()
{
	if (!m_exp->started())
		throw device_missing_dependencies();

	// extract the slot number from the last digit of the slot tag
	size_t const tlen = strlen(m_exp_slottag);

	m_slot = (m_exp_slottag[tlen - 1] - '0');
	if (m_slot < 0 || m_slot > 3)
		fatalerror("Podule %d out of range for Archimedes expansion bus\n", m_slot);
}

void device_archimedes_podule_interface::interface_post_start()
{
	m_exp->install_ioc_map(m_slot, *this, &device_archimedes_podule_interface::ioc_map);
	m_exp->install_memc_map(m_slot, *this, &device_archimedes_podule_interface::memc_map);
}


// slot devices
#include "a448.h"
#include "acejoy.h"
//#include "archdigi.h"
//#include "archscan.h"
#include "armadeus.h"
//#include "discbuffer.h"
//#include "colourcard.h"
#include "eaglem2.h"
#include "ether1.h"
//#include "ether2.h"
//#include "ether3.h"
//#include "ether5.h"
//#include "ethera.h"
#include "etherd.h"
#include "etherr.h"
#include "faxpack.h"
//#include "g8.h"
#include "greyhawk.h"
//#include "hdfc_rdev.h"
#include "hdisc.h"
#include "hdisc_cw.h"
#include "hdisc_morley.h"
#include "hdisc_we.h"
#include "ide_be.h"
//#include "ide_castle.h"
//#include "ide_dt.h"
//#include "ide_hccs.h"
//#include "ide_ics.h"
#include "ide_rdev.h"
//#include "ide_we.h"
#include "io.h"
#include "io_hccs.h"
#include "io_morley.h"
#include "io_we.h"
#include "lark.h"
#include "laserd.h"
#include "midi_emr.h"
#include "midimax.h"
#include "nexus.h"
//#include "prisma3.h"
#include "rom.h"
//#include "rom_cc.h"
#include "rs423.h"
#include "scan256.h"
#include "scanlight.h"
//#include "scsi_a500.h"
//#include "scsi_acorn.h"
//#include "scsi_ling.h"
//#include "scsi_morley.h"
//#include "scsi_oak.h"
#include "scsi_vti.h"
#include "serial.h"
#include "spectra.h"
#include "tube.h"
//#include "ultimate.h"
//#include "ultimatecd.h"

//-------------------------------------------------
//  archimedes_exp_devices (A3x0/A4x0/A5x0/A5000)
//-------------------------------------------------

void archimedes_exp_devices(device_slot_interface &device)
{
	device.option_add("a448", ARC_A448);                  // Armadillo Systems A448 Sound Sampler
	device.option_add("a448b", ARC_A448B);                // Armadillo Systems A448b Stereo MIDI Sound Sampler
	device.option_add("acejoy", ARC_ACEJOY);              // ACE Joy Connect
	//device.option_add("archdigi", ARC_ARCHDIGI);          // Watford Electronics Archimedes Video Digitiser
	//device.option_add("archscan", ARC_ARCHSCAN);          // Watford Electronics Archimedes Scanner
	device.option_add("armadeus", ARC_ARMADEUS);          // Clares Armadeus Sampler Board
	device.option_add("bbcio_aka10", ARC_BBCIO_AKA10);    // Acorn AKA10 BBC I/O Podule
	device.option_add("bbcio_we", ARC_BBCIO_WE);          // Watford BBC User I/O Card
	//device.option_add("cc", ARC_CC);                      // Wild Vision/Computer Concepts Colour Card
	//device.option_add("ccgold", ARC_CCGOLD);              // Wild Vision/Computer Concepts Colour Card Gold
	device.option_add("eaglem2", ARC_EAGLEM2);            // Wild Vision/Computer Concepts Eagle M2
	device.option_add("ether1", ARC_ETHER1_AKA25);        // Acorn AKA25 Ethernet
	//device.option_add("ether2", ARC_ETHER2_AEH50);        // Acorn AEH50 Ethernet II
	//device.option_add("ether3_aeh54", ARC_ETHER3_AEH54);  // Acorn AEH54 10Base2 Ethernet Podule
	//device.option_add("ether5", ARC_ETHER5);              // Atomwide Ethernet V Podule
	device.option_add("etherr", ARC_ETHERR);              // RISC Developments Ethernet Card
	device.option_add("faxpack", ARC_FAXPACK);            // Computer Concepts Fax-Pack
	//device.option_add("g8", ARC_G8);                      // State Machine G8 Graphic Accelerator
	//device.option_add("g16", ARC_G16);                    // State Machine G16 Graphic Accelerator
	device.option_add("greyhawk", ARC_GREYHAWK);          // Computer Concepts GreyHawk Video Digitiser
	//device.option_add("hdfc_rdev", ARC_HDFC_RDEV);        // RISC Developments High Density Floppy Controller
	device.option_add("hdisc_akd52", ARC_HDISC_AKD52);    // Acorn AKD52 Hard Disc Podule
	device.option_add("hdisc_cw", ARC_HDISC_CW);          // Computerware Hard Disk Podule
	device.option_add("hdisc_morley", ARC_HDISC_MORLEY);  // Morley Electronics Hard Disc Podule
	device.option_add("hdisc_we", ARC_HDISC_WE);          // Watford Electronics Archimedes Hard Disk Podule
	device.option_add("ide_be", ARC_IDE_BE);              // Baildon Electronics IDE HD Interface
	//device.option_add("ide_dt", ARC_IDE_DT);              // D.T. Software IDE Interface
	//device.option_add("ide_hccs", ARC_IDE_HCCS);          // HCCS IDE Interface
	device.option_add("ide_rdev", ARC_IDE_RDEV);          // RISC Developments IDE Hard Disc System
	//device.option_add("ide1v4_ics", ARC_IDE1V4_ICS);      // ICS ideA Hard Disc System
	device.option_add("iomidi_aka15", ARC_IOMIDI_AKA15);  // Acorn AKA15 MIDI and BBC I/O Podule
	device.option_add("lark", ARC_LARK);                  // Wild Vision/Computer Concepts Lark A16
	device.option_add("lbp4", ARC_LBP4);                  // Computer Concepts LaserDirect LBP-4
	device.option_add("midi_aka16", ARC_MIDI_AKA16);      // Acorn AKA16 MIDI Podule
	device.option_add("midi2", ARC_MIDI2_EMR);            // EMR MIDI 2 Interface
	device.option_add("midi4", ARC_MIDI4_EMR);            // EMR MIDI 4 Interface
	device.option_add("midimax", ARC_MIDIMAX);            // Wild Vision MidiMax
	device.option_add("midimax2", ARC_MIDIMAX2);          // Wild Vision MidiMax 2
	device.option_add("nexus_a500", ARC_NEXUS_A500);      // SJ Research Nexus Interface (A500)
	//device.option_add("prisma3", ARC_PRISMA3);            // Millipede PRISMA-3 Podule
	//device.option_add("prisma3p", ARC_PRISMA3P);          // Millipede PRISMA-3 Plus Podule
	device.option_add("rom_aka05", ARC_ROM_AKA05);        // Acorn AKA05 ROM Podule
	//device.option_add("rom_cc", ARC_ROM_CC);              // Computer Concepts ROM/RAM Podule
	device.option_add("rs423", ARC_RS423);                // Intelligent Interfaces Dual RS423 Serial Interface
	device.option_add("scan256", ARC_SCAN256);            // Watford Electronics 256 Grey-Scale Scanner
	device.option_add("scanlight", ARC_SCANLIGHT);        // Computer Concepts ScanLight
	device.option_add("scanjunior", ARC_SCANJUNIOR);      // Computer Concepts ScanLight Junior
	device.option_add("scanjunior3", ARC_SCANJUNIOR3);    // Computer Concepts ScanLight Junior Mk3
	device.option_add("scanvideo", ARC_SCANVIDEO);        // Computer Concepts ScanLight Video 256
	//device.option_add("scsi_a500", ARC_SCSI_A500);        // Acorn A500 SCSI Interface
	//device.option_add("scsi_aka31", ARC_SCSI_AKA31);      // Acorn AKA31 SCSI Expansion Card
	//device.option_add("scsi_aka32", ARC_SCSI_AKA32);      // Acorn AKA32 CDFS & SCSI Expansion Card
	//device.option_add("scsi_ling", ARC_SCSI_LING);        // Lingenuity SCSI Podule
	//device.option_add("scsi_morley", ARC_SCSI_MORLEY);    // Morley Electronics 16bit Cached SCSI card
	//device.option_add("scsi_oak", ARC_SCSI_OAK);          // Oak Solutions SCSI Interface
	device.option_add("scsi_vti", ARC_SCSI_VTI);          // VTI User Port and SCSI Podule
	device.option_add("serial", ARC_SERIAL);              // Atomwide Serial Expansion Card
	device.option_add("spectra", ARC_SPECTRA);            // Beebug Spectra Colour Scanner
	device.option_add("tube", ARC_TUBE);                  // Acorn Tube Podule
	device.option_add("ua_morley", ARC_UA_MORLEY);        // Morley Electronics Analogue and User Interface
	//device.option_add("ultcd", ARC_ULTCD);                // HCCS Ultimate CD-ROM
}

//-------------------------------------------------
//  archimedes_mini_exp_devices (A30x0/A4000)
//-------------------------------------------------

void archimedes_mini_exp_devices(device_slot_interface &device)
{
	device.option_add("bbcio_aga30", ARC_BBCIO_AGA30);    // Acorn AGA30 BBC I/O Podule
	device.option_add("bbcio_we", ARC_BBCIO_WE);          // Watford BBC User I/O Card
	//device.option_add("disc_a3k6", ARC_DISC_A3K6);        // PRES A3K6 Disc Buffer
	//device.option_add("ethera", ARC_ETHERA);              // ANT Ethernet 10base2 mini-podule
	device.option_add("etherd", ARC_ETHERD);              // Digital Services Ethernet Podule
	//device.option_add("ide_a3k_hccs", ARC_IDE_A3K_HCCS);  // HCCS IDE A3000 Interface
	//device.option_add("ide_castle", ARC_IDE_CASTLE);      // Castle Technology A3000 IDE Expansion Card
	//device.option_add("ide1v5_ics", ARC_IDE1V5_ICS);      // ICS ideA Hard Disc System
	//device.option_add("ide3v5_ics", ARC_IDE3V5_ICS);      // ICS ideA Hard Disc System
	//device.option_add("ide_we", ARC_IDE_WE);              // Watford Electronics A3000 IDE Interface
	device.option_add("scanlight", ARC_SCANLIGHT);        // Computer Concepts A3000 ScanLight
	//device.option_add("ult3010", ARC_ULT3010);            // HCCS A3010 Ultimate Expansion System
	device.option_add("uma_morley", ARC_UMA_MORLEY);      // Morley Electronics User/MIDI/Analogue Interface
	device.option_add("upa_hccs", ARC_UPA_HCCS);          // HCCS User/Analogue Podule
	device.option_add("upmidi_aka12", ARC_UPMIDI_AKA12);  // Acorn AKA12 User Port/MIDI Upgrade
}
