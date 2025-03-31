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

#define LOG_SETUP (1U << 1)

//#define VERBOSE (LOG_SETUP)

#include "logmacro.h"

#define LOGSETUP(...)      LOGMASKED(LOG_SETUP, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

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


device_heath_io_decoder_interface::device_heath_io_decoder_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "h89bus")
{
}

void device_heath_io_decoder_interface::update_slot_select_bits(u8 &select_bits, bool p506_signals)
{
	if (p506_signals)
	{
		select_bits &= ~(h89bus::IO_CASS | h89bus::IO_LP);
	}
	else
	{
		select_bits &= ~(h89bus::IO_FLPY);
	}
}

heath_io_decoder_444_43::heath_io_decoder_444_43(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H89BUS_IO_DECODE_444_43, tag, owner, clock),
	device_heath_io_decoder_interface(mconfig, *this)
{
}

//
//                                     PORT           Select
//         444-43                |  Hex  |  Octal  |   Line
// ------------------------------+-------+---------------------
//  Not specified, available     |  0-7B |   0-173 |
//  Hard-sector disk controller  | 7C-7F | 174-177 | IO_FLPY
//  Not specified, reserved      | 80-CF | 200-317 |
//  DCE Serial I/O               | D0-D7 | 320-327 | IO_SER0
//  DTE Serial I/O               | D8-DF | 330-337 | IO_SER1
//  DCE Serial I/O               | EO-E7 | 340-347 | IO_LP
//  Console I/O                  | E8-EF | 350-357 | IO_TERM
//  NMI                          | F0-F1 | 360-361 | IO_NMI
//  General purpose port         |    F2 |     362 | IO_GPP
//  Cassette I/O                 | F8-F9 | 370-371 | IO_CASS
//  NMI                          | FA-FB | 372-373 | IO_NMI
//
std::pair<u8, u8> heath_io_decoder_444_43::get_address_range(u8 select_bits, bool p506_signals)
{
	u8 start_addr = 0;
	u8 end_addr = 0;

	update_slot_select_bits(select_bits, p506_signals);

	// For select lines, IO_NMI, IO_TERM and actual GPP are always fixed across all PROMs, so
	// they are hard-coded in the h89.cpp and not included here.
	switch (select_bits)
	{
		case h89bus::IO_CASS:
			start_addr = 0xf8;
			end_addr   = 0xf9;
			break;
		case h89bus::IO_LP:
			start_addr = 0xe0;
			end_addr   = 0xe7;
			break;
		case h89bus::IO_FLPY:
			start_addr = 0x7c;
			end_addr   = 0x7f;
			break;
		case h89bus::IO_SER0:
			start_addr = 0xd0;
			end_addr   = 0xd7;
			break;
		case h89bus::IO_SER1:
			start_addr = 0xd8;
			end_addr   = 0xdf;
			break;
	}

	LOGSETUP("%s: start: 0x%02x end: 0x%02x\n", FUNCNAME, start_addr, end_addr);

	return std::make_pair(start_addr, end_addr);
}

heath_io_decoder_444_61::heath_io_decoder_444_61(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H89BUS_IO_DECODE_444_61, tag, owner, clock),
	device_heath_io_decoder_interface(mconfig, *this)
{
}

//                                     PORT           Select
//         444-61                |  Hex  |  Octal  |   Line
// ------------------------------+-------+---------------------
//  Not specified, available     |  0-77 |   0-167 |
//  Disk I/O #1                  | 78-7B | 170-173 | IO_FLPY
//  Disk I/O #2                  | 7C-7F | 174-177 | IO_CASS
//  Not specified, reserved      | 80-CF | 200-317 |
//  DCE Serial I/O               | D0-D7 | 320-327 | IO_SER0
//  DTE Serial I/O               | D8-DF | 330-337 | IO_SER1
//  DCE Serial I/O               | EO-E7 | 340-347 | IO_LP
//  Console I/O                  | E8-EF | 350-357 | IO_TERM
//  NMI                          | F0-F1 | 360-361 | IO_NMI
//  General purpose port         |    F2 |     362 | IO_GPP
//  NMI                          | FA-FB | 372-373 | IO_NMI
//
std::pair<u8, u8> heath_io_decoder_444_61::get_address_range(u8 select_bits, bool p506_signals)
{
	u8 start_addr = 0;
	u8 end_addr = 0;

	update_slot_select_bits(select_bits, p506_signals);

	// For select lines, IO_NMI, IO_TERM and actual GPP are always fixed across all PROMs, so
	// they are hard-coded in the h89.cpp and not included here.
	switch (select_bits)
	{
		case h89bus::IO_CASS:
			start_addr = 0x78;
			end_addr   = 0x7b;
			break;
		case h89bus::IO_LP:
			start_addr = 0xe0;
			end_addr   = 0xe7;
			break;
		case h89bus::IO_FLPY:
			start_addr = 0x7c;
			end_addr   = 0x7f;
			break;
		case h89bus::IO_SER0:
			start_addr = 0xd0;
			end_addr   = 0xd7;
			break;
		case h89bus::IO_SER1:
			start_addr = 0xd8;
			end_addr   = 0xdf;
			break;
	}

	LOGSETUP("%s: start: 0x%02x end: 0x%02x\n", FUNCNAME, start_addr, end_addr);
	return std::make_pair(start_addr, end_addr);
}

heath_io_decoder_mms_61c::heath_io_decoder_mms_61c(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H89BUS_IO_DECODE_MMS_61C, tag, owner, clock),
	device_heath_io_decoder_interface(mconfig, *this)
{
}

//                                     PORT           Select
//        MMS 444-61C            |  Hex  |  Octal  |   Line
// ------------------------------+-------+---------------------
//  Not specified, available     |  0-37 |   0- 67 |
//  MMS 77316 DD FDC             | 38-3F |  70- 77 | IO_GPP
//  MMS Internal test fixtures   | 40-47 | 100-107 | IO_LP,IO_SER0,IO_SER1
//  MMS 77317 ACT/XCOMP I/O      | 48-4F | 110-117 | IO_LP,IO_SER1
//  MMS 77315 CAMEO I/O          | 50-56 | 120-126 | IO_LP,IO_SER0
//  Unused                       |    57 |     127 | IO_LP,IO_SER0
//  MMS 77314 Corvus I/O         | 58-59 | 130-131 | IO_SER0,IO_SER1
//  MMS 77314 REMEX I/O          | 5A-5B | 132-133 | IO_SER0,IO_SER1
//  MMS 77314,15,17 Conf Port    |    5C |     134 | IO_SER0,IO_SER1
//  Unused                       | 5D-77 | 135-167 | IO_SER0,IO_SER1
//  Disk I/O #1                  | 78-7B | 170-173 | IO_CASS
//  Disk I/O #2                  | 7C-7F | 174-177 | IO_FLPY
//  HDOS reserved                | 80-CF | 200-317 |
//  DCE Serial I/O               | D0-D7 | 320-327 | IO_SER0
//  DTE Serial I/O               | D8-DF | 330-337 | IO_SER1
//  DCE Serial I/O               | EO-E7 | 340-347 | IO_LP
//  Console I/O                  | E8-EF | 350-357 | IO_TERM
//  NMI                          | F0-F1 | 360-361 | IO_NMI
//  General purpose port         |    F2 |     362 | IO_GPP
//  NMI                          | FA-FB | 372-373 | IO_NMI
//
std::pair<u8, u8> heath_io_decoder_mms_61c::get_address_range(u8 select_bits, bool p506_signals)
{
	u8 start_addr = 0;
	u8 end_addr = 0;

	update_slot_select_bits(select_bits, p506_signals);

	// For select lines, IO_NMI, IO_TERM and actual GPP are always fixed across all PROMs, so
	// they are hard-coded in the h89.cpp and not included here. For the MMS PROM, GPP was also
	// used for the 77316 Double-density controller, so it is included here.
	// MMS also had several boards that utilized multiple bits of the serial select lines to
	// signal selection of the board.
	switch (select_bits)
	{
		case h89bus::IO_CASS:
			start_addr = 0x78;
			end_addr   = 0x7b;
			break;
		case h89bus::IO_LP:
			start_addr = 0xe0;
			end_addr   = 0xe7;
			break;
		case h89bus::IO_FLPY:
			start_addr = 0x7c;
			end_addr   = 0x7f;
			break;
		case h89bus::IO_SER0 | h89bus::IO_SER1 | h89bus::IO_LP:
			start_addr = 0x40;
			end_addr   = 0x47;
			break;
		case h89bus::IO_SER1 | h89bus::IO_LP:
			start_addr = 0x48;
			end_addr   = 0x4f;
			break;
		case h89bus::IO_SER0 | h89bus::IO_LP:
			start_addr = 0x50;
			end_addr   = 0x57;
			break;
		case h89bus::IO_SER0 | h89bus::IO_SER1:
			start_addr = 0x58;
			end_addr   = 0x5f;
			break;
		case h89bus::IO_SER0:
			start_addr = 0xd0;
			end_addr   = 0xd7;
			break;
		case h89bus::IO_SER1:
			start_addr = 0xd8;
			end_addr   = 0xdf;
			break;
		case h89bus::IO_GPP:
			start_addr = 0x38;
			end_addr   = 0x3f;
			break;
	}
	LOGSETUP("%s: start: 0x%02x end: 0x%02x\n", FUNCNAME, start_addr, end_addr);

	return std::make_pair(start_addr, end_addr);
}


heath_io_decoder_cdr86::heath_io_decoder_cdr86(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, H89BUS_IO_DECODE_CDR_86, tag, owner, clock),
	device_heath_io_decoder_interface(mconfig, *this)
{
}


//
//                                     PORT           Select
//        CDR86                  |  Hex  |  Octal  |   Line
// ------------------------------+-------+---------------------
//  Not specified, available     |  0-37 |   0- 67 |
//  Disk I/O #1                  | 38-3f |  70- 77 | IO_CASS
//  Not specified, available     | 40-7B |  80-173 |
//  Disk I/O #2                  | 7C-7F | 174-177 | IO_FLPY
//  Not specified, reserved      | 80-CF | 200-317 |
//  DCE Serial I/O               | D0-D7 | 320-327 | IO_SER0
//  DTE Serial I/O               | D8-DF | 330-337 | IO_SER1
//  DCE Serial I/O               | EO-E7 | 340-347 | IO_LP
//  Console I/O                  | E8-EF | 350-357 | IO_TERM
//  NMI                          | F0-F1 | 360-361 | IO_NMI
//  General purpose port         |    F2 |     362 | IO_GPP
//  NMI                          | FA-FB | 372-373 | IO_NMI
//
std::pair<u8, u8> heath_io_decoder_cdr86::get_address_range(u8 select_bits, bool p506_signals)
{
	u8 start_addr = 0;
	u8 end_addr = 0;

	update_slot_select_bits(select_bits, p506_signals);

	// For select lines, IO_NMI, IO_TERM and actual GPP are always fixed across all PROMs, so
	// they are hard-coded in the h89.cpp and not included here.
	switch (select_bits)
	{
		case h89bus::IO_CASS:
			start_addr = 0x38;
			end_addr   = 0x3f;
			break;
		case h89bus::IO_LP:
			start_addr = 0xe0;
			end_addr   = 0xe7;
			break;
		case h89bus::IO_FLPY:
			start_addr = 0x7c;
			end_addr   = 0x7f;
			break;
		case h89bus::IO_SER0:
			start_addr = 0xd0;
			end_addr   = 0xd7;
			break;
		case h89bus::IO_SER1:
			start_addr = 0xd8;
			end_addr   = 0xdf;
			break;
	}
	LOGSETUP("%s: start: 0x%02x end: 0x%02x\n", FUNCNAME, start_addr, end_addr);

	return std::make_pair(start_addr, end_addr);
}

DEFINE_DEVICE_TYPE(H89BUS_IO_DECODE_444_43,  heath_io_decoder_444_43,  "h89bus_io_decoder_444_43",  "Heath H89 IO Decoder 444-43")
DEFINE_DEVICE_TYPE(H89BUS_IO_DECODE_444_61,  heath_io_decoder_444_61,  "h89bus_io_decoder_444_61",  "Heath H89 IO Decoder 444-61")
DEFINE_DEVICE_TYPE(H89BUS_IO_DECODE_MMS_61C, heath_io_decoder_mms_61c, "h89bus_io_decoder_mms_61c", "Heath H89 IO Decoder MMS 444-61c")
DEFINE_DEVICE_TYPE(H89BUS_IO_DECODE_CDR_86,  heath_io_decoder_cdr86,   "h89bus_io_decoder_cdr86",   "Heath H89 IO Decoder CDR86")

DEFINE_DEVICE_TYPE(H89BUS_IO_DECODE_SOCKET,  heath_io_decoder_socket,  "h89bus_io_decoder_socket",  "Heath H89 IO Decoder Socket")

std::pair<u8, u8> heath_io_decoder_socket::get_address_range(u8 select_bits, bool p506_signals)
{
	if (m_decoder)
	{
		return m_decoder->get_address_range(select_bits);
	}
	else
	{
		LOGSETUP("m_decoder not set\n");
	}

	return std::make_pair(0, 0);
}

heath_io_decoder_socket::heath_io_decoder_socket(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H89BUS_IO_DECODE_SOCKET, tag, owner, clock),
	device_single_card_slot_interface(mconfig, *this),
	m_decoder(nullptr)
{
}

void heath_io_decoder_socket::device_start()
{
	m_decoder = get_card_device();
}


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
	m_io_decoder_socket(*this, "io_decoder"),
	m_out_int3_cb(*this),
	m_out_int4_cb(*this),
	m_out_int5_cb(*this),
	m_out_fmwe_cb(*this),
	m_out_wait_cb(*this)
{
}

h89bus_device::~h89bus_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void h89bus_device::device_start()
{
}

void h89bus_device::add_h89bus_left_card(device_h89bus_left_card_interface &card)
{
	m_left_device_list.emplace_back(card);
}

void h89bus_device::add_h89bus_right_card(device_h89bus_right_card_interface &card)
{
	m_right_device_list.emplace_back(card);
}

void h89bus_device::install_io_device(offs_t start, offs_t end, read8sm_delegate rhandler, write8sm_delegate whandler)
{
	m_io_space->install_readwrite_handler(start, end, rhandler, whandler);
}

void h89bus_device::install_io_device(offs_t start, offs_t end, read8smo_delegate rhandler, write8smo_delegate whandler)
{
	m_io_space->install_readwrite_handler(start, end, rhandler, whandler);
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

