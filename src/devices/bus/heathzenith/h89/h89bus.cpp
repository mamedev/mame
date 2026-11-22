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
#include <array>
#include <cctype>
#include <string>

#define LOG_SETUP (1U << 1)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGSETUP(...)      LOGMASKED(LOG_SETUP, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

DEFINE_DEVICE_TYPE(H89BUS_LEFT_SLOT, h89bus_left_slot_device, "h89bus_lslot", "H-89 left (memory) slot")

h89bus_left_slot_device::h89bus_left_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: h89bus_left_slot_device(mconfig, H89BUS_LEFT_SLOT, tag, owner, clock)
{
}

h89bus_left_slot_device::h89bus_left_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface(mconfig, *this)
	, m_h89bus(*this, finder_base::DUMMY_TAG)
	, m_h89bus_slottag(nullptr)
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

DEFINE_DEVICE_TYPE(H89BUS_CPU_BOARD_MOD_SLOT, h89bus_cpu_board_mod_slot_device, "h89bus_cpu_board_mod_slot", "H-89 CPU board modification slot")

h89bus_cpu_board_mod_slot_device::h89bus_cpu_board_mod_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: h89bus_cpu_board_mod_slot_device(mconfig, H89BUS_CPU_BOARD_MOD_SLOT, tag, owner, clock)
{
}

h89bus_cpu_board_mod_slot_device::h89bus_cpu_board_mod_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_single_card_slot_interface(mconfig, *this)
	, m_h89bus(*this, finder_base::DUMMY_TAG)
	, m_h89bus_slottag(nullptr)
	, m_out_cpu_clock_cb(*this)
{
}

void h89bus_cpu_board_mod_slot_device::device_start()
{
}

void h89bus_cpu_board_mod_slot_device::device_resolve_objects()
{
	device_h89bus_cpu_board_mod_interface *dev = get_card_device();

	if (dev)
	{
		dev->set_h89bus_tag(m_h89bus.target(), m_h89bus_slottag);
		dev->m_slot = this;
	}
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(H89BUS, h89bus_device, "h89bus", "H-89/Z-90 bus")


ROM_START(444_43)
	ROM_REGION(0x100, "io_decode", 0)
	// H88 I/O decoding
	ROM_LOAD("444-43.u550", 0x000000, 0x000100, CRC(3e0315f4) SHA1(11da9a9145de07f1f3bf1270a10e059dff30c693))
ROM_END

ROM_START(444_61)
	ROM_REGION(0x100, "io_decode", 0)
	// H89 I/O decoding
	ROM_LOAD("444-61.u550", 0x000000, 0x000100, CRC(0b3c129f) SHA1(92da6484d1339160400d6bc75578a977c5e4d23e))
ROM_END

ROM_START(mms_61c)
	ROM_REGION(0x100, "io_decode", 0)
	// MMS (Magnolia Micro Systems) I/O decoding
	ROM_LOAD( "444-61c.u550",  0x000000, 0x000100, CRC(e7122061) SHA1(33c124f44c0f9cb99c9b17ad15411b4bc6407eae))
ROM_END

ROM_START(cdr86)
	ROM_REGION(0x100, "io_decode", 0)
	// CDR Systems to support FDC-880H
	ROM_LOAD( "cdr86.u550",  0x000000, 0x000100, CRC(d35e4063) SHA1(879f9d265d77f8a74c70febd9a80d6896ab8ec7e))
ROM_END


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

h89bus::addr_ranges device_heath_io_decoder_interface::scan_io_decoder_rom(u8 select_bits, u8 *rom)
{
	h89bus::addr_ranges ranges;
	bool found = false;
	u8 first = 0, last = 0;

	for (int i = 0; i < 256; i++)
	{
		u8 val = rom[i] ^ 0xff;

		if ((val & select_bits) == select_bits)
		{
			if (!found)
			{
				found = true;
				first = i;
			}

			last = i;
		}
		else if (found)
		{
			ranges.emplace_back(first, last);
			found = false;
		}
	}

	// make sure to include any ranges that extend to the end.
	if (found)
	{
		ranges.emplace_back(first, last);
	}

	return ranges;
}

heath_io_decoder_444_43::heath_io_decoder_444_43(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H89BUS_IO_DECODER_444_43, tag, owner, clock),
	device_heath_io_decoder_interface(mconfig, *this),
	m_decode_prom(*this, "io_decode")
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
h89bus::addr_ranges heath_io_decoder_444_43::get_address_ranges(u8 select_bits, bool p506_signals)
{
	update_slot_select_bits(select_bits, p506_signals);

	return scan_io_decoder_rom(select_bits, m_decode_prom);
}

const tiny_rom_entry *heath_io_decoder_444_43::device_rom_region() const
{
	return ROM_NAME(444_43);
}

heath_io_decoder_444_61::heath_io_decoder_444_61(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H89BUS_IO_DECODER_444_61, tag, owner, clock),
	device_heath_io_decoder_interface(mconfig, *this),
	m_decode_prom(*this, "io_decode")
{
}

//                                     PORT           Select
//         444-61                |  Hex  |  Octal  |   Line
// ------------------------------+-------+---------------------
//  Not specified, available     |  0-77 |   0-167 |
//  Disk I/O #1                  | 78-7B | 170-173 | IO_CASS
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
h89bus::addr_ranges heath_io_decoder_444_61::get_address_ranges(u8 select_bits, bool p506_signals)
{
	update_slot_select_bits(select_bits, p506_signals);

	return scan_io_decoder_rom(select_bits, m_decode_prom);
}

const tiny_rom_entry *heath_io_decoder_444_61::device_rom_region() const
{
	return ROM_NAME(444_61);
}

heath_io_decoder_mms_61c::heath_io_decoder_mms_61c(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H89BUS_IO_DECODER_MMS_61C, tag, owner, clock),
	device_heath_io_decoder_interface(mconfig, *this),
	m_decode_prom(*this, "io_decode")
{
}

//                                     PORT            Select
//        MMS 444-61C            |  Hex  |  Octal  |   Line(s)
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
h89bus::addr_ranges heath_io_decoder_mms_61c::get_address_ranges(u8 select_bits, bool p506_signals)
{
	update_slot_select_bits(select_bits, p506_signals);

	return scan_io_decoder_rom(select_bits, m_decode_prom);
}

const tiny_rom_entry *heath_io_decoder_mms_61c::device_rom_region() const
{
	return ROM_NAME(mms_61c);
}

heath_io_decoder_cdr86::heath_io_decoder_cdr86(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, H89BUS_IO_DECODER_CDR_86, tag, owner, clock),
	device_heath_io_decoder_interface(mconfig, *this),
	m_decode_prom(*this, "io_decode")
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
h89bus::addr_ranges heath_io_decoder_cdr86::get_address_ranges(u8 select_bits, bool p506_signals)
{
	update_slot_select_bits(select_bits, p506_signals);

	return scan_io_decoder_rom(select_bits, m_decode_prom);
}

const tiny_rom_entry *heath_io_decoder_cdr86::device_rom_region() const
{
	return ROM_NAME(cdr86);
}

DEFINE_DEVICE_TYPE(H89BUS_IO_DECODER_444_43,  heath_io_decoder_444_43,  "h89bus_io_decoder_444_43",  "Heath H89 IO Decoder 444-43 PROM")
DEFINE_DEVICE_TYPE(H89BUS_IO_DECODER_444_61,  heath_io_decoder_444_61,  "h89bus_io_decoder_444_61",  "Heath H89 IO Decoder 444-61 PROM")
DEFINE_DEVICE_TYPE(H89BUS_IO_DECODER_MMS_61C, heath_io_decoder_mms_61c, "h89bus_io_decoder_mms_61c", "Heath H89 IO Decoder MMS 444-61c PROM")
DEFINE_DEVICE_TYPE(H89BUS_IO_DECODER_CDR_86,  heath_io_decoder_cdr86,   "h89bus_io_decoder_cdr86",   "Heath H89 IO Decoder CDR86 PROM")

DEFINE_DEVICE_TYPE(H89BUS_IO_DECODER_SOCKET,  heath_io_decoder_socket,  "h89bus_io_decoder_socket",  "Heath H89 IO Decoder Socket")

h89bus::addr_ranges heath_io_decoder_socket::get_address_ranges(u8 select_bits, bool p506_signals)
{
	if (m_decoder)
	{
		return m_decoder->get_address_ranges(select_bits, p506_signals);
	}

	LOGSETUP("m_decoder not set\n");

	h89bus::addr_ranges ranges;

	return ranges;
}

heath_io_decoder_socket::heath_io_decoder_socket(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H89BUS_IO_DECODER_SOCKET, tag, owner, clock),
	device_single_card_slot_interface(mconfig, *this),
	m_decoder(nullptr)
{
}

void heath_io_decoder_socket::device_start()
{
	m_decoder = get_card_device();

	if (!m_decoder)
		osd_printf_warning("%s: no I/O decoder PROM installed, no I/O ports will be decoded\n", tag());
}


//**************************************************************************
//  MEMORY DECODER PROMS
//**************************************************************************

/*
  Primary Memory Decoder PROM (U517) - 256 x 8

    Input                                 Output
    -----------------------------------   -----------------------------------
    A7 - MEM 1 H  ORG-0 (CP/M) map        D7 - WE L
    A6 - JJ502    memory size jumpers     D6 - RD7 L
    A5 - JJ501                            D5 - RD6 L     16K expansion memory
    A4 - BRD L    board read              D4 - RAS2 L
    A3 - BRFSH L  DRAM refresh            D3 - RAS1 L
    A2 - A15                              D2 - RAS0 L
    A1 - A14                              D1 - No Memory L
    A0 - A13                              D0 - U516 L    secondary decoder

  Secondary Memory Decoder PROM (U516) - 32 x 8

    Input                                 Output
    -----------------------------------   -----------------------------------
    A4 - FMWE H   floppy memory write     D7 - WE L
    A3 - WE L     from U517 D7            D4 - Floppy ROM L
    A2 - A12                              D3 - Floppy RAM L
    A1 - A11                              D2 - Option RAM L
    A0 - A10                              D1 - Option ROM L
                                          D0 - System ROM L

  Magnolia Microsystems 77318 bank select control board (10-77318-1)

  Plugs into the U517 and U552 (general purpose port latch) sockets, with its
  own latch, a key-lock circuit and a 512 x 8 primary decoder PROM. The PROM
  outputs are the same as the Heath PROMs', except D1: it goes to the 128K
  RAM board (10-77318-2) as CTL, which inverts A13 there, so there is no
  DATA=0 output. Its inputs are

    A8 - D2 of port 0xf2, NANDed with the key-lock
    A7 - MEM 1 H  (D5 of port 0xf2)
    A6 - D4 of port 0xf2, NANDed with the key-lock
    A5 - JJ501    (JJ502 and JJ503 are not connected)
    A4 - A0 as on the Heath PROMs

  The key-lock is two 74LS107 J-K flip-flops with K low, cleared only by
  reset. The first is set when D3 falls while D2 is high, the second when D2
  falls while D3 is high, after the first. The documented sequence on D3/D2,
  04 0c 04 08 0c 08, does that with J steady at each edge.

  Until the key-lock is set, only MEM 1 selects the map (A, the power-on map,
  or B, ORG-0). After it, D5, D4 and D2 select one of eight maps until the
  next reset. The RAM board holds the top 16K of the standard 64K (page 0) and
  two 56K pages, which the maps put in low memory:

    map  port 0xf2  low memory
    A    00         system ROM, the floppy RAM and ROM (power on)
    B    20         page 0 (ORG-0)
    C    04         0x0000 - 0x1fff from page 1
    D    24         0x0000 - 0x1fff from page 2
    E    10         0x0000 - 0xbfff from page 1
    F    30         0x0000 - 0xbfff from page 2
    G    14         0x0000 - 0xdfff from page 1
    H    34         0x0000 - 0xdfff from page 2
*/

device_heath_mem_decoder_interface::device_heath_mem_decoder_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "h89bus")
{
}

u32 device_heath_mem_decoder_interface::map_count() const
{
	return 2;
}

offs_t device_heath_mem_decoder_interface::map_inputs(u32 map, u8 jumpers) const
{
	return (BIT(map, 0) << 7) | (jumpers << 5);
}

u32 device_heath_mem_decoder_interface::select_map(u8 gpp)
{
	return (gpp & h89bus::GPP_MEM1) ? 1 : 0;
}

u8 device_heath_mem_decoder_interface::map_gpp_lines() const
{
	return h89bus::GPP_MEM1;
}

u8 device_heath_mem_decoder_interface::card_ctl_line() const
{
	return 0;
}

namespace {

ROM_START(mem_444_42)
	ROM_REGION(0x100, "mem_decode", 0)
	// original primary decoder, supports up to 48K, no ORG-0
	ROM_LOAD("444-42.u517", 0x0000, 0x0100, CRC(65831fa4) SHA1(ed9865b4b11c292eef3bc7f415de836af8c6bcc7))
ROM_END

ROM_START(mem_444_66)
	ROM_REGION(0x100, "mem_decode", 0)
	// replacement primary decoder, adds 64K and ORG-0
	ROM_LOAD("444-66.u517", 0x0000, 0x0100, CRC(ad94a5df) SHA1(33b478e2c19da1cfa301506866d6810e4171556d))
ROM_END

ROM_START(mem_mms_77318)
	ROM_REGION(0x200, "mem_decode", 0)
	// not dumped, decode() uses a table built from the documented memory maps
	ROM_LOAD("77318.u517", 0x0000, 0x0200, NO_DUMP)
ROM_END

ROM_START(mem_444_41)
	ROM_REGION(0x20, "mem_decode", 0)
	// secondary decoder for 2K system ROMs (MTR-88, MTR-89), option ROM at 0x0800
	ROM_LOAD("444-41.u516", 0x0000, 0x0020, CRC(1232f9f6) SHA1(44ee87741e25c51e0f6dc4d5f0408c5d9ec499a5))
ROM_END

ROM_START(mem_444_83)
	ROM_REGION(0x20, "mem_decode", 0)
	// secondary decoder for 4K system ROMs (MTR-90)
	ROM_LOAD("444-83.u516", 0x0000, 0x0020, CRC(9160de2b) SHA1(50fb3f9358514f79c407585e3ccd3f3b0aef54df))
ROM_END

class heath_mem_decoder_prom_device : public device_t, public device_heath_mem_decoder_interface
{
public:
	virtual u8 decode(offs_t inputs) const override { return m_prom[inputs & (m_prom.length() - 1)] ^ 0xff; }

protected:
	heath_mem_decoder_prom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, const tiny_rom_entry *rom) :
		device_t(mconfig, type, tag, owner, clock),
		device_heath_mem_decoder_interface(mconfig, *this),
		m_rom(rom),
		m_prom(*this, "mem_decode")
	{
	}

	virtual void device_start() override ATTR_COLD { }
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD { return m_rom; }

private:
	const tiny_rom_entry *const m_rom;
	required_region_ptr<u8> m_prom;
};

class heath_mem_decoder_444_42 : public heath_mem_decoder_prom_device
{
public:
	heath_mem_decoder_444_42(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) :
		heath_mem_decoder_prom_device(mconfig, H89BUS_MEM_DECODER_444_42, tag, owner, clock, ROM_NAME(mem_444_42))
	{
	}
};

class heath_mem_decoder_444_66 : public heath_mem_decoder_prom_device
{
public:
	heath_mem_decoder_444_66(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) :
		heath_mem_decoder_prom_device(mconfig, H89BUS_MEM_DECODER_444_66, tag, owner, clock, ROM_NAME(mem_444_66))
	{
	}
};

class heath_mem_decoder_mms_77318 : public device_t, public device_heath_mem_decoder_interface
{
public:
	heath_mem_decoder_mms_77318(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) :
		device_t(mconfig, H89BUS_MEM_DECODER_MMS_77318, tag, owner, clock),
		device_heath_mem_decoder_interface(mconfig, *this)
	{
	}

	virtual u8 decode(offs_t inputs) const override ATTR_COLD { return m_map_table[inputs & 0x1ff]; }

	// maps A - H: bit 0 is MEM 1 (D5), bit 1 is D2 and bit 2 is D4 of port 0xf2
	virtual u32 map_count() const override ATTR_COLD { return 8; }
	virtual offs_t map_inputs(u32 map, u8 jumpers) const override ATTR_COLD;
	virtual u32 select_map(u8 gpp) override;

	// D2 and D3 (RSVD 0/1) for the key-lock, D2, D4 (MEM 0) and D5 (MEM 1) for the map
	virtual u8 map_gpp_lines() const override ATTR_COLD { return h89bus::GPP_RSVD0 | h89bus::GPP_RSVD1 | h89bus::GPP_MEM0 | h89bus::GPP_MEM1; }

	virtual u8 card_ctl_line() const override ATTR_COLD { return h89bus::MEM_PRI_CTL; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD { return ROM_NAME(mem_mms_77318); }

private:
	static u8 standard_map(bool org0, bool mem_64k, bool write, u32 block) ATTR_COLD;

	// stands in for the PROM until it is dumped, with a selected line as 1
	std::array<u8, 0x200> m_map_table;

	// key-lock flip-flops, and D3/D2 in the latch
	bool m_key_ff1;
	bool m_key_ff2;
	u8 m_key_bits;
};

offs_t heath_mem_decoder_mms_77318::map_inputs(u32 map, u8 jumpers) const
{
	return (BIT(map, 1) ? 0x000 : 0x100) | (BIT(map, 0) << 7) | (BIT(map, 2) ? 0x00 : 0x40) | (BIT(jumpers, 0) << 5);
}

u32 heath_mem_decoder_mms_77318::select_map(u8 gpp)
{
	// the flip-flops' J and clock both come from the latch, so J is the value from before
	// the write; the second is held clear until the first is set
	u8 const bits = gpp & 0x0c;
	bool const d2_falls = BIT(m_key_bits, 2) && !BIT(bits, 2);
	bool const d3_falls = BIT(m_key_bits, 3) && !BIT(bits, 3);

	if (m_key_ff1 && d2_falls && BIT(m_key_bits, 3))
		m_key_ff2 = true;

	if (d3_falls && BIT(m_key_bits, 2))
		m_key_ff1 = true;

	m_key_bits = bits;

	if (m_key_ff2)
		return bitswap<3>(gpp, 4, 2, 5);

	return BIT(gpp, 5);
}

// map A (ROM) or B (ORG-0); with JJ501 out (no RAM board) the same as a 444-66 with 48K;
// with it, page 0's top 16K is bank A 0xc000 - 0xffff on the RAM board, so ORG-0 moves the
// lower half of RAS2 from 0xc000 to 0x0000, as a 444-66 does with 48K
u8 heath_mem_decoder_mms_77318::standard_map(bool org0, bool mem_64k, bool write, u32 block)
{
	u8 out = 0;

	switch (block)
	{
	case 0:
		if (org0)
			out = h89bus::MEM_PRI_RAS2;
		else if (write && mem_64k)
			out = h89bus::MEM_PRI_U516 | h89bus::MEM_PRI_RAS2; // the RAM ORG-0 puts here, as on a 444-66
		else
			out = h89bus::MEM_PRI_U516;
		break;
	case 1: case 2: out = h89bus::MEM_PRI_RAS0; break;
	case 3: case 4: out = h89bus::MEM_PRI_RAS1; break;
	case 5:         out = h89bus::MEM_PRI_RAS2; break;
	case 6:
		if (mem_64k)
			out = h89bus::MEM_PRI_RD6;
		else if (!org0)
			out = h89bus::MEM_PRI_RAS2;
		break;
	case 7:
		if (mem_64k)
			out = h89bus::MEM_PRI_RD6;
		break;
	}

	// D1 is CTL, so nothing is selected where a 444-66 gives DATA=0
	if (!out)
		return 0;

	return write ? (out | h89bus::MEM_PRI_WE) : out;
}

void heath_mem_decoder_mms_77318::device_start()
{
	// build the table from the documented maps and the RAM board's schematic; page 1 is
	// bank B 0x0000 - 0xdfff (RD7), page 2 is bank A 0x0000 - 0xbfff (RD6) and, with CTL
	// inverting A13, bank B 0xe000 - 0xffff for 0xc000 - 0xdfff
	for (offs_t inputs = 0; inputs < 0x200; inputs++)
	{
		bool const d2 = !BIT(inputs, 8);
		bool const org0 = BIT(inputs, 7);
		bool const d4 = !BIT(inputs, 6);
		bool const mem_64k = BIT(inputs, 5);
		bool const write = BIT(inputs, 4);
		bool const refresh = !BIT(inputs, 3);
		u32 const block = inputs & 0x07;

		u8 out = 0;

		if (!refresh)
		{
			// blocks in low memory from page 1 (D5 = 0) or page 2 (D5 = 1)
			u32 paged = 0;
			if (mem_64k)
			{
				if (d4)
					paged = d2 ? 7 : 6;
				else
					paged = d2 ? 1 : 0;
			}

			if (block < paged)
			{
				if (!org0)
					out = h89bus::MEM_PRI_RD7;
				else if (block < 6)
					out = h89bus::MEM_PRI_RD6;
				else
					out = h89bus::MEM_PRI_RD7 | h89bus::MEM_PRI_CTL;

				if (write)
					out |= h89bus::MEM_PRI_WE;
			}
			else
			{
				out = standard_map(org0, mem_64k, write, block);
			}
		}

		m_map_table[inputs] = out;
	}

	save_item(NAME(m_key_ff1));
	save_item(NAME(m_key_ff2));
	save_item(NAME(m_key_bits));
}

void heath_mem_decoder_mms_77318::device_reset()
{
	// reset clears the key-lock flip-flops and the latch
	m_key_ff1 = false;
	m_key_ff2 = false;
	m_key_bits = 0;
}

class heath_mem_decoder_444_41 : public heath_mem_decoder_prom_device
{
public:
	heath_mem_decoder_444_41(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) :
		heath_mem_decoder_prom_device(mconfig, H89BUS_MEM_DECODER_444_41, tag, owner, clock, ROM_NAME(mem_444_41))
	{
	}
};

class heath_mem_decoder_444_83 : public heath_mem_decoder_prom_device
{
public:
	heath_mem_decoder_444_83(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) :
		heath_mem_decoder_prom_device(mconfig, H89BUS_MEM_DECODER_444_83, tag, owner, clock, ROM_NAME(mem_444_83))
	{
	}
};

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_MEM_DECODER_444_42, device_heath_mem_decoder_interface, heath_mem_decoder_444_42, "h89bus_mem_decoder_444_42", "Heath H89 Primary Memory Decoder 444-42 PROM")
DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_MEM_DECODER_444_66, device_heath_mem_decoder_interface, heath_mem_decoder_444_66, "h89bus_mem_decoder_444_66", "Heath H89 Primary Memory Decoder 444-66 PROM")
DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_MEM_DECODER_MMS_77318, device_heath_mem_decoder_interface, heath_mem_decoder_mms_77318, "h89bus_mem_decoder_mms_77318", "Magnolia Microsystems 77318 Bank Select Control Board")
DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_MEM_DECODER_444_41, device_heath_mem_decoder_interface, heath_mem_decoder_444_41, "h89bus_mem_decoder_444_41", "Heath H89 Secondary Memory Decoder 444-41 PROM")
DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_MEM_DECODER_444_83, device_heath_mem_decoder_interface, heath_mem_decoder_444_83, "h89bus_mem_decoder_444_83", "Heath H89 Secondary Memory Decoder 444-83 PROM")

DEFINE_DEVICE_TYPE(H89BUS_MEM_DECODER_SOCKET, heath_mem_decoder_socket, "h89bus_mem_decoder_socket", "Heath H89 Memory Decoder Socket")

heath_mem_decoder_socket::heath_mem_decoder_socket(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H89BUS_MEM_DECODER_SOCKET, tag, owner, clock),
	device_single_card_slot_interface(mconfig, *this),
	m_decoder(nullptr)
{
}

void heath_mem_decoder_socket::device_start()
{
	m_decoder = get_card_device();

	if (!m_decoder)
		osd_printf_warning("%s: no memory decoder PROM installed, memory it selects will not be mapped\n", tag());
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
	m_pri_mem_decoder(*this, "pri_mem_decoder"),
	m_sec_mem_decoder(*this, "sec_mem_decoder"),
	m_out_int3_cb(*this),
	m_out_int4_cb(*this),
	m_out_int5_cb(*this),
	m_out_wait_cb(*this),
	m_mem_view(*this, "mem_map"),
	m_mem_installed(false),
	m_mem_map(0),
	m_fmwe(false),
	m_ram(nullptr),
	m_ram_size(0),
	m_rom(nullptr),
	m_floppy_ram(nullptr),
	m_sys_rom_bank(*this, "sys_rom"),
	m_sys_rom_card(nullptr)
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
	m_gpp = 0;

	save_item(NAME(m_gpp));
	save_item(NAME(m_mem_map));
	save_item(NAME(m_fmwe));
}

//-------------------------------------------------
//  device_reset - the general purpose port is
//  cleared on reset, every handler is called so
//  devices can take their reset state from it
//-------------------------------------------------

void h89bus_device::device_reset()
{
	m_gpp = 0;

	for (auto &[mask, handler] : m_gpp_handlers)
		handler(0);
}

//-------------------------------------------------
//  device_reset_after_children - select the memory
//  map once the primary memory decoder is reset
//-------------------------------------------------

void h89bus_device::device_reset_after_children()
{
	m_mem_map = m_pri_mem_decoder->select_map(0);
	update_mem_view();
}

//-------------------------------------------------
//  device_clock_changed - let the cards know the
//  2 MHz bus clock line has changed speed
//-------------------------------------------------

void h89bus_device::device_clock_changed()
{
	// the clock is also reported while the bus starts, before the cards are ready; they read it at reset
	if (!started())
		return;

	for (device_h89bus_card_interface &card : m_left_device_list)
		card.bus_clock_changed(clock());

	for (device_h89bus_card_interface &card : m_right_device_list)
		card.bus_clock_changed(clock());
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

void h89bus_device::install_io_device(offs_t start, offs_t end, write8smo_delegate whandler)
{
	m_io_space->install_write_handler(start, end, whandler);
}

h89bus::addr_ranges h89bus_device::get_address_ranges(u8 select_bits, bool p506_signals)
{
	return m_io_decoder_socket->get_address_ranges(select_bits, p506_signals);
}

void h89bus_device::gpp_w(u8 data)
{
	u8 const changed = m_gpp ^ data;

	m_gpp = data;

	for (auto &[mask, handler] : m_gpp_handlers)
	{
		if (changed & mask)
			handler(data);
	}

	// the primary memory decoder selects the memory map, from MEM 1 (ORG-0) on the Heath PROMs
	u32 const map = m_pri_mem_decoder->select_map(data);

	if (map != m_mem_map)
	{
		m_mem_map = map;
		update_mem_view();
	}
}

//-------------------------------------------------
//  install_mem_map - build the memory map from
//  the memory decoder PROMs
//
//  The only PROM inputs that change while running
//  are the ones that select the memory map (MEM 1,
//  ORG-0, on the Heath PROMs) and FMWE (floppy RAM
//  write enable), so a view entry is built for each
//  combination of them.
//-------------------------------------------------

void h89bus_device::install_mem_map(u8 *ram, offs_t ram_size, u8 *rom, u8 *floppy_ram)
{
	m_ram = ram;
	m_ram_size = ram_size;
	m_rom = rom;
	m_floppy_ram = floppy_ram;

	// a device that also switches on the port lines that select the memory map changes with it,
	// such as a speed upgrade on MEM 0 with an MMS 77318
	u8 shared_lines = 0;

	for (auto const &[mask, handler] : m_gpp_handlers)
		shared_lines |= mask & m_pri_mem_decoder->map_gpp_lines();

	if (shared_lines)
		osd_printf_warning("%s: another device also uses port 0xf2 bits %02x, which the memory decoder uses to select the memory map\n",
				tag(), shared_lines);

	// the system ROM comes from the CPU board, or from the first card in a left slot that has one
	m_sys_rom_bank->configure_entry(0, m_rom);

	for (device_h89bus_left_card_interface &card : m_left_device_list)
	{
		u8 *const card_rom = card.sys_rom();

		if (!card_rom)
			continue;

		if (m_sys_rom_card)
		{
			osd_printf_warning("%s: %s and %s both have a system ROM, only %s can select its ROM\n",
					tag(), m_sys_rom_card->device().tag(), card.device().tag(), m_sys_rom_card->device().tag());
			continue;
		}

		m_sys_rom_card = &card;
		m_sys_rom_bank->configure_entry(1, card_rom);
	}

	// warn when more than one device answers a left slot select line
	bool rd6_card = false;

	for (u8 const line : { h89bus::MEM_PRI_RD6, h89bus::MEM_PRI_RD7 })
	{
		std::vector<std::string> names;

		for (device_h89bus_left_card_interface &card : m_left_device_list)
		{
			if (card.mem_select_lines() & line)
				names.emplace_back(card.device().tag());
		}

		if (line == h89bus::MEM_PRI_RD6)
			rd6_card = !names.empty();

		if (cpu_ram_bank(line))
			names.emplace_back("CPU board");

		if (names.size() > 1)
		{
			std::string list;
			for (std::string const &name : names)
				list += (list.empty() ? "" : ", ") + name;

			osd_printf_warning("%s: more than one device answers %s (%s); writes go to all of them, reads come from %s\n",
					tag(), (line == h89bus::MEM_PRI_RD6) ? "RD6" : "RD7", list, names.front());
		}
	}

	// memory size jumpers JJ501/JJ502 match the installed RAM: 16K, 32K, 48K or 64K;
	// a 16K expansion card on RD6 makes a 48K board into a 64K system
	u32 const cpu_banks = m_ram_size / 0x4000;
	u8 jumpers = cpu_banks - 1;

	if (rd6_card)
	{
		if (cpu_banks == 3)
			jumpers = 3;
		else if (cpu_banks < 3)
			osd_printf_warning("%s: the 16K expansion memory (RD6) needs 48K on the CPU board, it will not be selected\n", tag());
	}

	m_program_space->install_view(0x0000, 0xffff, m_mem_view);

	for (u32 map = 0; map < m_pri_mem_decoder->map_count(); map++)
	{
		for (int fmwe = 0; fmwe < 2; fmwe++)
		{
			install_mem_view_entry(m_mem_view[(map << 1) | fmwe], map, fmwe, jumpers);
		}
	}

	m_mem_installed = true;
}

// CPU board 16K RAM bank for a primary memory decoder select line, null if it is not installed
u8 *h89bus_device::cpu_ram_bank(u8 select_line) const
{
	// the CPU board has three banks (RAS0 - RAS2), and a 64K board also has the
	// 16K expansion piggybacked on one of them and wired to RD6; they are filled
	// in that order
	u32 bank;

	switch (select_line)
	{
	case h89bus::MEM_PRI_RAS0: bank = 0; break;
	case h89bus::MEM_PRI_RAS1: bank = 1; break;
	case h89bus::MEM_PRI_RAS2: bank = 2; break;
	case h89bus::MEM_PRI_RD6:  bank = 3; break;
	default: return nullptr;
	}

	if (((bank + 1) * 0x4000) > m_ram_size)
	{
		return nullptr;
	}

	return m_ram + (bank * 0x4000);
}

// RAM at addr for the primary memory decoder's select lines: cards in the left
// slots (RD6/RD7) first, so reads come from them, then the CPU board
std::vector<u8 *> h89bus_device::select_line_targets(u8 pri, offs_t addr) const
{
	std::vector<u8 *> targets;

	u8 const card_lines = pri & (h89bus::MEM_PRI_RD6 | h89bus::MEM_PRI_RD7);

	if (card_lines)
	{
		u8 const ctl = pri & m_pri_mem_decoder->card_ctl_line();

		for (device_h89bus_left_card_interface &card : m_left_device_list)
		{
			if (card.mem_select_lines() & card_lines)
			{
				u8 *const target = card.mem_select(card_lines | ctl, addr);
				if (target)
					targets.push_back(target);
			}
		}
	}

	// RAM banks are 16K, A13 selects the half
	for (u8 const line : { h89bus::MEM_PRI_RAS0, h89bus::MEM_PRI_RAS1, h89bus::MEM_PRI_RAS2, h89bus::MEM_PRI_RD6 })
	{
		if (pri & line)
		{
			u8 *const bank = cpu_ram_bank(line);
			if (bank)
				targets.push_back(bank + (addr & 0x3fff));
		}
	}

	return targets;
}

void h89bus_device::install_mem_view_entry(memory_view::memory_view_entry &entry, u32 map, bool fmwe, u8 jumpers)
{
	// what each 1K page of the address space reads from and writes to
	constexpr u32 PAGES = 0x10000 / 0x400;
	std::array<u8 *, PAGES> read_src{};
	std::array<std::vector<u8 *>, PAGES> write_dst;

	// pages read from the system ROM, through the bank so a card can put its own ROM there
	std::array<bool, PAGES> sys_rom{};

	// a decoder output that goes to the cards isn't DATA=0
	u8 const nomem_line = h89bus::MEM_PRI_NOMEM & ~m_pri_mem_decoder->card_ctl_line();

	for (int write = 0; write < 2; write++)
	{
		for (u32 block = 0; block < 8; block++)
		{
			// A3 is BRFSH L, which is never active here
			u8 const pri = m_pri_mem_decoder->decode(m_pri_mem_decoder->map_inputs(map, jumpers) | (write ? 0x10 : 0x00) | 0x08 | block);

			if (pri & nomem_line)
			{
				continue;
			}

			for (u32 sub = 0; sub < 8; sub++)
			{
				u32 const page = (block << 3) | sub;
				offs_t const addr = page << 10;

				auto const add = [this, &write_dst, &read_src, write, page, addr] (u8 *ptr)
				{
					if (!ptr)
						return;

					if (write)
						write_dst[page].push_back(ptr);
					else if (read_src[page])
						logerror("memory decoder selects more than one device for reads at 0x%04x\n", addr);
					else
						read_src[page] = ptr;
				};

				// writes go to all the RAM that is selected, reads come from the first
				if (!write || (pri & h89bus::MEM_PRI_WE))
				{
					for (u8 *const target : select_line_targets(pri, addr))
					{
						add(target);
						if (!write)
							break;
					}
				}

				if (pri & h89bus::MEM_PRI_U516)
				{
					// the WE L input is the WE L output of the primary decoder
					u8 const sec = m_sec_mem_decoder->decode((fmwe ? 0x10 : 0x00) | ((pri & h89bus::MEM_PRI_WE) ? 0x00 : 0x08) | sub);

					if (write)
					{
						if ((sec & h89bus::MEM_SEC_FPY_RAM) && (sec & h89bus::MEM_SEC_WE))
							add(m_floppy_ram + (addr & 0x3ff));
					}
					else
					{
						// the option ROM and option RAM sockets are not populated
						if (sec & h89bus::MEM_SEC_SYS_ROM)
						{
							add(m_rom + (addr & 0x1fff));
							sys_rom[page] = (read_src[page] == (m_rom + (addr & 0x1fff)));
						}
						if (sec & h89bus::MEM_SEC_FPY_ROM)
							add(m_rom + (addr & 0x1fff));
						if (sec & h89bus::MEM_SEC_FPY_RAM)
							add(m_floppy_ram + (addr & 0x3ff));
					}
				}
			}
		}
	}

	// install runs of pages that are contiguous in the same device
	auto const install_runs = [] (auto const &target, auto const &install)
	{
		for (u32 page = 0; page < PAGES; )
		{
			u8 *const base = target(page);
			u32 end = page + 1;

			if (base)
			{
				while ((end < PAGES) && (target(end) == (base + ((end - page) << 10))))
					end++;

				install(page << 10, (end << 10) - 1, base);
			}

			page = end;
		}
	};

	install_runs(
			[&read_src, &sys_rom] (u32 page) { return sys_rom[page] ? nullptr : read_src[page]; },
			[&entry] (offs_t start, offs_t end, u8 *base) { entry.install_rom(start, end, base); });

	// the bank is indexed from the start of the range it is installed in, and the bank entries
	// start at address 0x0000, where every U516 PROM puts the system ROM; a card's ROM is 4K
	install_runs(
			[&read_src, &sys_rom] (u32 page) { return sys_rom[page] ? read_src[page] : nullptr; },
			[this, &entry] (offs_t start, offs_t end, u8 *base)
			{
				if (start != 0x0000)
				{
					logerror("memory decoder selects the system ROM at 0x%04x, not 0x0000; a card can't replace it there\n", start);
					entry.install_rom(start, end, base);
					return;
				}

				offs_t const bank_end = std::min<offs_t>(end, 0x0fff);

				entry.install_read_bank(start, bank_end, m_sys_rom_bank);

				if (end > bank_end)
				{
					logerror("memory decoder selects the system ROM up to 0x%04x; a card can only replace 0x0000-0x0fff\n", end);
					entry.install_rom(bank_end + 1, end, base + (bank_end + 1));
				}
			});

	install_runs(
			[&write_dst] (u32 page) { return (write_dst[page].size() == 1) ? write_dst[page][0] : nullptr; },
			[&entry] (offs_t start, offs_t end, u8 *base) { entry.install_writeonly(start, end, base); });

	// a write can go to more than one device, such as the floppy RAM and the
	// RAM that shadows the lower 8K in a 64K system; these pages get a handler
	// that writes to all of them (not a write tap, as a view in the bus is
	// destroyed after the CPU's address space that a tap is registered with)
	for (u32 page = 0; page < PAGES; )
	{
		std::vector<u8 *> const &targets = write_dst[page];
		u32 end = page + 1;

		if (targets.size() > 1)
		{
			auto const contiguous = [&write_dst, &targets, page] (u32 next)
			{
				std::vector<u8 *> const &next_targets = write_dst[next];

				if (next_targets.size() != targets.size())
					return false;

				for (size_t i = 0; i < targets.size(); i++)
				{
					if (next_targets[i] != (targets[i] + ((next - page) << 10)))
						return false;
				}

				return true;
			};

			while ((end < PAGES) && contiguous(end))
				end++;

			entry.install_write_handler(page << 10, (end << 10) - 1,
					write8sm_delegate(*this, NAME([targets] (offs_t offset, u8 data) { for (u8 *const target : targets) target[offset] = data; })));
		}

		page = end;
	}
}

void h89bus_device::update_mem_view()
{
	if (m_mem_installed)
		m_mem_view.select((m_mem_map << 1) | (m_fmwe ? 1 : 0));
}

void h89bus_device::select_card_sys_rom(device_h89bus_left_card_interface &card, bool state)
{
	// only the card that got the bank can select it, see install_mem_map
	if (m_mem_installed && (&card == m_sys_rom_card))
		m_sys_rom_bank->set_entry(state ? 1 : 0);
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
	m_fmwe = bool(state);

	update_mem_view();
}

void h89bus_device::set_wait_line(int state)
{
	m_out_wait_cb(state);
}

//**************************************************************************
//  DEVICE H89BUS CARD INTERFACE
//**************************************************************************

device_h89bus_interface::device_h89bus_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "h89bus"),
	m_h89bus_slottag(nullptr),
	m_h89bus(nullptr)
{
}

void device_h89bus_interface::interface_pre_start()
{
	if (!m_h89bus)
	{
		fatalerror("Can't find H-89 bus device\n");
	}
}

//-------------------------------------------------
//  device_h89bus_card_interface - constructor
//-------------------------------------------------

device_h89bus_card_interface::device_h89bus_card_interface(const machine_config &mconfig, device_t &device) :
	device_h89bus_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~device_h89bus_card_interface - destructor
//-------------------------------------------------

device_h89bus_card_interface::~device_h89bus_card_interface()
{
}

void device_h89bus_card_interface::bus_clock_changed(u32 clock)
{
}

device_h89bus_left_card_interface::device_h89bus_left_card_interface(const machine_config &mconfig, device_t &device) :
	device_h89bus_card_interface(mconfig, device)
{
}

device_h89bus_left_card_interface::~device_h89bus_left_card_interface()
{
}

u8 device_h89bus_left_card_interface::mem_select_lines() const
{
	return 0;
}

u8 *device_h89bus_left_card_interface::mem_select(u8 select_lines, offs_t addr)
{
	return nullptr;
}

u8 *device_h89bus_left_card_interface::sys_rom()
{
	return nullptr;
}

device_h89bus_right_card_interface::device_h89bus_right_card_interface(const machine_config &mconfig, device_t &device) :
	device_h89bus_card_interface(mconfig, device),
	m_p506_signals(false)
{
}

device_h89bus_right_card_interface::~device_h89bus_right_card_interface()
{
}

device_h89bus_cpu_board_mod_interface::device_h89bus_cpu_board_mod_interface(const machine_config &mconfig, device_t &device) :
	device_h89bus_interface(mconfig, device),
	m_slot(nullptr)
{
}

device_h89bus_cpu_board_mod_interface::~device_h89bus_cpu_board_mod_interface()
{
}
