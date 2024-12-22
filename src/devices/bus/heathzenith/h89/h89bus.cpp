// license:BSD-3-Clause
// copyright-holders:R. Belmont, Mark Garlanger
/***************************************************************************

  h89bus.cpp - Heath/Zenith H-89/Z-90 bus

  by R. Belmont

  This system is weird.  There are 3 left-hand slots.  These are intended
  for RAM expansion and map into the Z80's memory space.  They get A0-A12
  and some select signals decoded by a pair of PROMs at U516 and U517.
  Notably, these slots don't get the Z80's /RD or /WR signals so actual
  working boards for these slots always run a cable or jumper(s) to pick
  signals from the motherboard.  These slots also have no way to signal
  an interrupt.

  There are also 3 right-hand slots.  These connect to the Z80's I/O address
  space and are addressed by a 3-bit offset A0/A1/A2 plus select lines /SER0,
  /SER1, /LP1 and /CASS (on P504/P510 and P505/P511), or /FLPY (on P506/P512).
  P506/P512 replaces the /LP1 select line with the /FMWE signal which write-enables
  the "floppy RAM" at 0x1400.  These slots have 3 standard interrupt outputs
  /INT3, /INT4, and /INT5.

  Notable real-world use cases that we support:
  - The Sigmasoft parallel card plugs into a left slot but picks the I/O space
    select signal off the motherboard.  This plus the left slots' nearly full
    set of address lines let it decode the I/O space arbitrarily without
    having to change the I/O decoder PROM.

  - The MMS 77316 floppy controller has jumpers to replace U553 so that it can
    intercept the GPP select line output by the PROM and further decode it.
    This decode appears to check if A0/A1/A2 = b010.  The partial schematic for
    the card doesn't include that logic, but based on what signals it has access
    to and the fact that the card doesn't respond to A0/A1/A2 = b010 that seems
    to be a reasonable guess.

***************************************************************************/

#include "emu.h"
#include "h89bus.h"

#include <algorithm>
#include <cctype>

DEFINE_DEVICE_TYPE(H89BUS_LEFT_SLOT, h89bus_left_slot_device, "h89bus_lslot", "H-89 left (memory) slot")

h89bus_left_slot_device::h89bus_left_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h89bus_left_slot_device(mconfig, H89BUS_LEFT_SLOT, tag, owner, clock)
{
}

h89bus_left_slot_device::h89bus_left_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_single_card_slot_interface(mconfig, *this),
	m_h89bus(*this, finder_base::DUMMY_TAG),
	m_h89bus_slottag(nullptr)
{
}

void h89bus_left_slot_device::device_start()
{
}

void h89bus_left_slot_device::device_resolve_objects()
{
	device_h89bus_left_card_interface *dev = get_card_device();

	if (dev)
	{
		dev->set_h89bus_tag(m_h89bus.target(), m_h89bus_slottag);
		m_h89bus->add_h89bus_left_card(*dev);
	}
}

DEFINE_DEVICE_TYPE(H89BUS_RIGHT_SLOT, h89bus_right_slot_device, "h89bus_rslot", "H-89 right (I/O) slot")

h89bus_right_slot_device::h89bus_right_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h89bus_right_slot_device(mconfig, H89BUS_RIGHT_SLOT, tag, owner, clock)
{
}

h89bus_right_slot_device::h89bus_right_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_single_card_slot_interface(mconfig, *this),
	m_h89bus(*this, finder_base::DUMMY_TAG),
	m_h89bus_slottag(nullptr),
	m_p506_signals(false)
{
}

void h89bus_right_slot_device::device_start()
{
}

void h89bus_right_slot_device::device_resolve_objects()
{
	device_h89bus_right_card_interface *dev = get_card_device();

	if (dev)
	{
		dev->set_h89bus_tag(m_h89bus.target(), m_h89bus_slottag);
		dev->set_p506_signalling(m_p506_signals);
		m_h89bus->add_h89bus_right_card(*dev);
	}
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(H89BUS, h89bus_device, "h89bus", "H-89/Z-90 bus")

ROM_START(h89bus)
	ROM_REGION(0x100, "iodecode", 0)
	// H88 I/O decoding
	ROM_SYSTEM_BIOS(0, "444-43", "Heath/Zenith stock decoding (444-43)")
	ROMX_LOAD("444-43.bin", 0x000000, 0x000100, CRC(3e0315f4) SHA1(11da9a9145de07f1f3bf1270a10e059dff30c693), ROM_BIOS(0))

	// H89 I/O decoding
	ROM_SYSTEM_BIOS(1, "444-61", "Z-37 decoding (444-61)")
	ROMX_LOAD("444-61.bin", 0x000000, 0x000100, CRC(0b3c129f) SHA1(92da6484d1339160400d6bc75578a977c5e4d23e), ROM_BIOS(1))

	// MMS (Magnolia Micro Systems) I/O decoding
	ROM_SYSTEM_BIOS(2, "444-61c", "MMS decoding (444-61c)")
	ROMX_LOAD( "444-61c.bin",  0x000000, 0x000100, CRC(e7122061) SHA1(33c124f44c0f9cb99c9b17ad15411b4bc6407eae), ROM_BIOS(2))

	// CDR Systems
	ROM_SYSTEM_BIOS(3, "cdr86", "CDR decoding (CDR86)")
	ROMX_LOAD( "cdr86.bin",  0x000000, 0x000100, CRC(d35e4063) SHA1(879f9d265d77f8a74c70febd9a80d6896ab8ec7e), ROM_BIOS(3))
ROM_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  h89bus_device - constructor
//-------------------------------------------------

h89bus_device::h89bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	h89bus_device(mconfig, H89BUS, tag, owner, clock)
{
}

h89bus_device::h89bus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_program_space(*this, finder_base::DUMMY_TAG, -1),
	m_io_space(*this, finder_base::DUMMY_TAG, -1),
	m_decode_prom(*this, "iodecode"),
	m_out_int3_cb(*this),
	m_out_int4_cb(*this),
	m_out_int5_cb(*this),
	m_out_fdcirq_cb(*this),
	m_out_fdcdrq_cb(*this),
	m_out_blockirq_cb(*this),
	m_out_fmwe_cb(*this),
	m_out_wait_cb(*this),
	m_in_tlb_cb(*this, 0),
	m_in_nmi_cb(*this, 0),
	m_in_gpp_cb(*this, 0),
	m_out_tlb_cb(*this),
	m_out_nmi_cb(*this),
	m_out_gpp_cb(*this)
{
}

h89bus_device::~h89bus_device()
{
}

const tiny_rom_entry *h89bus_device::device_rom_region() const
{
	return ROM_NAME(h89bus);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void h89bus_device::device_start()
{
	m_io_space->install_readwrite_handler(0x0000, 0x00ff, emu::rw_delegate(*this, FUNC(h89bus_device::io_dispatch_r)), emu::rw_delegate(*this, FUNC(h89bus_device::io_dispatch_w)));
}

void h89bus_device::add_h89bus_left_card(device_h89bus_left_card_interface &card)
{
	m_left_device_list.emplace_back(card);
}

void h89bus_device::add_h89bus_right_card(device_h89bus_right_card_interface &card)
{
	m_right_device_list.emplace_back(card);
}

void h89bus_device::set_io0(int state)
{
	m_io0 = state;
}

void h89bus_device::set_io1(int state)
{
	m_io1 = state;
}

void h89bus_device::set_mem0(int state)
{
	m_mem0 = state;
}

void h89bus_device::set_mem1(int state)
{
	m_mem1 = state;
}

int h89bus_device::get_io0()
{
	return m_io0;
}

int h89bus_device::get_io1()
{
	return m_io1;
}

int h89bus_device::get_mem0()
{
	return m_mem0;
}

int h89bus_device::get_mem1()
{
	return m_mem1;
}

u8 h89bus_device::read_gpp()
{
	return m_in_gpp_cb(0);
}
void h89bus_device::write_gpp(u8 data)
{
	m_out_gpp_cb(0, data);
}

u8 h89bus_device::io_dispatch_r(offs_t offset)
{
	u8 retval = 0;

	u16 decode = m_decode_prom[offset] ^ 0xff;

	if (decode)
	{
		if ((decode & H89_GPP) && ((offset & 7) == 2)) return m_in_gpp_cb(offset);
		if (decode & H89_NMI) return m_in_nmi_cb(offset);
		if (decode & H89_TERM) return m_in_tlb_cb(offset & 7);

		for (device_h89bus_right_card_interface &entry : m_right_device_list)
		{
			if (entry.m_p506_signals)
			{
				// p506 does not have CASS or LP
				retval |= entry.read(decode & ~(H89_CASS | H89_LP), offset & 7);
			}
			else
			{
				// p504/p505 does not have FLPY
				retval |= entry.read(decode & ~H89_FLPY , offset & 7);
			}
		}
	}

	// service left-slot cards that have a motherboard connection to snoop the I/O space
	for (device_h89bus_left_card_interface &entry : m_left_device_list)
	{
		retval |= entry.read(H89_IO, offset & 0x1fff);
	}

	return retval;
}

void h89bus_device::io_dispatch_w(offs_t offset, u8 data)
{
	u16 decode = m_decode_prom[offset] ^ 0xff;

	if (decode)
	{
		if (decode & H89_GPP) m_out_gpp_cb(offset, data);
		if (decode & H89_NMI) { m_out_nmi_cb(offset, data); return; }
		if (decode & H89_TERM) { m_out_tlb_cb(offset & 7, data); return; }

		for (device_h89bus_right_card_interface &entry : m_right_device_list)
		{
			if (entry.m_p506_signals)
			{
				// p506 does not have CASS or LP
				entry.write(decode &  ~(H89_CASS | H89_LP), offset & 7, data);
			}
			else
			{
				// p504/p505 does not have FLPY
				entry.write(decode & ~H89_FLPY, offset & 7, data);
			}
		}
	}

	// service left-slot cards that have a motherboard connection to snoop the I/O space
	for (device_h89bus_left_card_interface &entry : m_left_device_list)
	{
		entry.write(H89_IO, offset, data);
	}
}

void h89bus_device::set_int3_line(int state)
{
	m_out_int3_cb(state);
}

void h89bus_device::set_int4_line(int state)
{
	m_out_int4_cb(state);
}

void h89bus_device::set_int5_line(int state)
{
	m_out_int5_cb(state);
}

void h89bus_device::set_fdcirq_line(int state)
{
	m_out_fdcirq_cb(state);
}

void h89bus_device::set_fdcdrq_line(int state)
{
	m_out_fdcdrq_cb(state);
}

void h89bus_device::set_blockirq_line(int state)
{
	m_out_blockirq_cb(state);
}

void h89bus_device::set_fmwe_line(int state)
{
	m_out_fmwe_cb(state);
}

void h89bus_device::set_wait_line(int state)
{
	m_out_wait_cb(state);
}

//**************************************************************************
//  DEVICE H89BUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_h89bus_card_interface - constructor
//-------------------------------------------------

device_h89bus_card_interface::device_h89bus_card_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "h89bus"),
	m_h89bus(nullptr)
{
}


//-------------------------------------------------
//  ~device_h89bus_card_interface - destructor
//-------------------------------------------------

device_h89bus_card_interface::~device_h89bus_card_interface()
{
}

void device_h89bus_card_interface::interface_pre_start()
{
	if (!m_h89bus)
	{
		fatalerror("Can't find H-89 bus device\n");
	}
}

device_h89bus_left_card_interface::device_h89bus_left_card_interface(const machine_config &mconfig, device_t &device) :
	device_h89bus_card_interface(mconfig, device)
{
}

device_h89bus_left_card_interface::~device_h89bus_left_card_interface()
{
}

device_h89bus_right_card_interface::device_h89bus_right_card_interface(const machine_config &mconfig, device_t &device) :
	device_h89bus_card_interface(mconfig, device),
	m_p506_signals(false)
{
}

device_h89bus_right_card_interface::~device_h89bus_right_card_interface()
{
}

