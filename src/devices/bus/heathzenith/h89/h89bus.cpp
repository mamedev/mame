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


#define LOG_PORT_READ     (1U << 1)
#define LOG_PORT_WRITE    (1U << 2)

#define LOG_MEM_READ      (1U << 3)
#define LOG_MEM_WRITE     (1U << 4)

#define VERBOSE (LOG_PORT_READ | LOG_PORT_WRITE)
#include "logmacro.h"

#define LOGPORTREAD(...)     LOGMASKED(LOG_PORT_READ,    __VA_ARGS__)
#define LOGPORTWRITE(...)    LOGMASKED(LOG_PORT_WRITE,   __VA_ARGS__)
#define LOGMEMREAD(...)      LOGMASKED(LOG_MEM_READ,     __VA_ARGS__)
#define LOGMEMWRITE(...)     LOGMASKED(LOG_MEM_WRITE,    __VA_ARGS__)


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

//
//                                     PORT
//         444-43                |  Hex  |  Octal
// ------------------------------+-------+---------
//  Not specified, available     |  0-7B |   0-173
//  Hard-sector disk controller  | 7C-7F | 174-177
//  Not specified, reserved      | 80-CF | 200-317
//  DCE Serial I/O               | D0-D7 | 320-327
//  DTE Serial I/O               | D8-DF | 330-337
//  DCE Serial I/O               | EO-E7 | 340-347
//  Console I/O                  | E8-EF | 350-357
//  NMI                          | F0-F1 | 360-361
//  General purpose port         |    F2 |     362
//  Cassette I/O                 | F8-F9 | 370-371
//  NMI                          | FA-FB | 372-373
//
//                                     PORT
//         444-61                |  Hex  |  Octal
// ------------------------------+-------+---------
//  Not specified, available     |  0-77 |   0-167
//  Disk I/O #1                  | 78-7B | 170-173
//  Disk I/O #2                  | 7C-7F | 174-177
//  Not specified, reserved      | 80-CF | 200-317
//  DCE Serial I/O               | D0-D7 | 320-327
//  DTE Serial I/O               | D8-DF | 330-337
//  DCE Serial I/O               | EO-E7 | 340-347
//  Console I/O                  | E8-EF | 350-357
//  NMI                          | F0-F1 | 360-361
//  General purpose port         |    F2 |     362
//  NMI                          | FA-FB | 372-373
//
ROM_START(h89bus)

	// I/O Decoder
	// -----------
	ROM_REGION(0x100, "io_decode", 0)
	// H88 (Cassette & hard-sectored controller) I/O decoding
	ROM_SYSTEM_BIOS(0, "444-43", "H88 (Cassette & hard-sectored controller)")
	ROMX_LOAD("444-43.u550",  0x0000, 0x0100, CRC(3e0315f4) SHA1(11da9a9145de07f1f3bf1270a10e059dff30c693), ROM_BIOS(0))

	// H89/Z90 (all Heath cards except cassette) I/O decoding
	ROM_SYSTEM_BIOS(1, "444-61", " H89/Z90 (all Heath cards except cassette)")
	ROMX_LOAD("444-61.u550",  0x0000, 0x0100, CRC(0b3c129f) SHA1(92da6484d1339160400d6bc75578a977c5e4d23e), ROM_BIOS(1))

	// MMS (Magnolia Micro Systems) I/O decoding
	ROM_SYSTEM_BIOS(2, "444-61c", "MMS decoding (444-61c)")
	ROMX_LOAD("444-61c.u550", 0x0000, 0x0100, CRC(e7122061) SHA1(33c124f44c0f9cb99c9b17ad15411b4bc6407eae), ROM_BIOS(2))

	// CDR Systems
	ROM_SYSTEM_BIOS(3, "cdr86", "CDR decoding (CDR86)")
	ROMX_LOAD("cdr86.u550",   0x0000, 0x0100, CRC(d35e4063) SHA1(879f9d265d77f8a74c70febd9a80d6896ab8ec7e), ROM_BIOS(3))


	// Primary Memory Decoder
	// ----------------------
	ROM_REGION(0x100, "mem_pri_decode", 0)
	// Newest PROM supports 64k and ORG0
	ROM_LOAD("444-66.u517", 0x0000, 0x0100, CRC(ad94a5df) SHA1(33b478e2c19da1cfa301506866d6810e4171556d))

	// Alternative PROM - 444-42, original does not support 64k or ORG0
	// ROM_LOAD("444-42.u517", 0x0000, 0x0100, CRC(65831fa4) SHA1(ed9865b4b11c292eef3bc7f415de836af8c6bcc7))


	// Secondary Memory Decoder
	// ------------------------
	ROM_REGION(0x20, "mem_sec_decode", 0)
	// Newest PROM, supports 4k Code ROM chips
	ROM_LOAD("444-83.u516", 0x0000, 0x0020, CRC(9160de2b) SHA1(50fb3f9358514f79c407585e3ccd3f3b0aef54df))

	// Alt PROM - 444-41, for 2k Code ROM chips.
	// ROM_LOAD("444-41.u516", 0x0000, 0x0020, CRC(1232f9f6) SHA1(44ee87741e25c51e0f6dc4d5f0408c5d9ec499a5))
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
	m_io_decode_prom(*this, "io_decode"),
	m_mem_primary_decode_prom(*this,"mem_pri_decode"),
	m_mem_secondary_decode_prom(*this,"mem_sec_decode"),
	m_out_int3_cb(*this),
	m_out_int4_cb(*this),
	m_out_int5_cb(*this),
	m_out_fmwe_cb(*this),
	m_out_wait_cb(*this),
	m_out_timer_intr_cb(*this),
	m_out_single_step_cb(*this),
	m_out_cpu_speed_cb(*this),
	m_out_clear_timer_intr(*this),
	m_in_tlb_cb(*this, 0),
	m_in_nmi_cb(*this, 0),
	m_in_gpp_cb(*this, 0),
	m_out_tlb_cb(*this),
	m_out_nmi_cb(*this),
	m_in_bank0_cb(*this, 0),
	m_in_bank1_cb(*this, 0),
	m_in_bank2_cb(*this, 0),
	m_out_bank0_cb(*this),
	m_out_bank1_cb(*this),
	m_out_bank2_cb(*this),
	m_in_sys_rom_cb(*this, 0),
	m_in_opt_rom_cb(*this, 0),
	m_in_opt_ram_cb(*this, 0),
	m_in_flpy_ram_cb(*this, 0),
	m_in_flpy_rom_cb(*this, 0),
	m_out_opt_ram_cb(*this),
	m_out_flpy_ram_cb(*this)
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
	m_program_space->install_readwrite_handler(0x0000, 0xffff, emu::rw_delegate(*this, FUNC(h89bus_device::mem_dispatch_r)), emu::rw_delegate(*this, FUNC(h89bus_device::mem_dispatch_w)));
	m_io_space->install_readwrite_handler(0x0000, 0x00ff, emu::rw_delegate(*this, FUNC(h89bus_device::io_dispatch_r)), emu::rw_delegate(*this, FUNC(h89bus_device::io_dispatch_w)));

	save_item(NAME(m_gpp));
	save_item(NAME(m_mem0));
	save_item(NAME(m_mem1));
	save_item(NAME(m_io0));
	save_item(NAME(m_io1));
	save_item(NAME(m_rsv0));
	save_item(NAME(m_rsv1));
	save_item(NAME(jj501_502));
}

void h89bus_device::device_reset()
{
	update_gpp(0);
}

void h89bus_device::add_h89bus_left_card(device_h89bus_left_card_interface &card)
{
	m_left_device_list.emplace_back(card);
}

void h89bus_device::add_h89bus_right_card(device_h89bus_right_card_interface &card)
{
	m_right_device_list.emplace_back(card);
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

int h89bus_device::get_rsv0()
{
	return m_rsv0;
}

int h89bus_device::get_rsv1()
{
	return m_rsv1;
}

void h89bus_device::set_jj501_502(u8 val)
{
	// place it in the correct position for the PROM lookup
	jj501_502 = val << 5;
}

u8 h89bus_device::mem_m1_r(offs_t offset)
{
	return mem_dispatch_r(offset);
}

//
// General Purpose Port
//
// Bit   OUTPUT
// ---------------------
//  0    Single-step enable
//  1    2 mSec interrupt enable
//  2    Not used with original Heath equipment (RSV 0)
//  3    Not used with original Heath equipment (RSV 1)
//  4    Latched bit MEM 0 H on memory expansion connector (Commonly used for Speed upgrades)
//  5    Latched bit MEM 1 H on memory expansion connector - ORG-0 (CP/M map)
//  6    Latched bit I/O 0 on I/O exp connector
//  7    Latched bit I/O 1 on I/O exp connector
//
void h89bus_device::update_gpp(u8 gpp)
{
	u8 changed_gpp = gpp ^ m_gpp;

	m_gpp = gpp;

	m_mem0 = BIT(m_gpp, GPP_MEM0_BIT);
	m_mem1 = BIT(m_gpp, GPP_MEM1_BIT);
	m_io0 = BIT(m_gpp, GPP_IO0_BIT);
	m_io1 = BIT(m_gpp, GPP_IO1_BIT);
	m_rsv0 = BIT(m_gpp, GPP_RSV0_BIT);
	m_rsv1 = BIT(m_gpp, GPP_RSV1_BIT);

	m_out_clear_timer_intr(0);

	m_out_timer_intr_cb(BIT(m_gpp, GPP_ENABLE_TIMER_INTERRUPT_BIT));

	if (BIT(changed_gpp, GPP_SINGLE_STEP_BIT))
	{
		m_out_single_step_cb(BIT(m_gpp, GPP_SINGLE_STEP_BIT));
	}

	if (BIT(changed_gpp, GPP_MEM0_BIT))
	{
		m_out_cpu_speed_cb(m_mem0);
	}
}

u8 h89bus_device::io_dispatch_r(offs_t offset)
{
	u8 retval = 0;

	u8 decode = m_io_decode_prom[offset] ^ 0xff;

	if (decode)
	{
		if ((decode & H89_IO_GPP) && ((offset & 7) == 2))
		{
			retval = m_in_gpp_cb(offset);
			goto done;
		}
		if (decode & H89_IO_NMI)
		{
			retval = m_in_nmi_cb(offset);
			goto done;
		}
		if (decode & H89_IO_TERM)
		{
			retval = m_in_tlb_cb(offset & 7);
			goto done;
		}

		for (device_h89bus_right_card_interface &entry : m_right_device_list)
		{
			if (entry.m_p506_signals)
			{
				// p506 does not have CASS or LP
				retval |= entry.read(decode & ~(H89_IO_CASS | H89_IO_LP), offset & 7);
			}
			else
			{
				// p504/p505 does not have FLPY
				retval |= entry.read(decode & ~H89_IO_FLPY , offset & 7);
			}
		}
	}

	// service left-slot cards that have a motherboard connection to snoop the I/O space
	for (device_h89bus_left_card_interface &entry : m_left_device_list)
	{
		retval |= entry.read(offset & 0xff);
	}

done:
	LOGPORTREAD("port read - offset: 0x%02x - 0x%02x\n", offset, retval);

	return retval;
}

void h89bus_device::io_dispatch_w(offs_t offset, u8 data)
{
	LOGPORTWRITE("port write - offset: 0x%02x - 0x%02x\n", offset, data);

	u8 decode = m_io_decode_prom[offset] ^ 0xff;

	if (decode)
	{
		if ((decode & H89_IO_GPP) && ((offset & 7) == 2))
		{
			update_gpp(data);
		}
		if (decode & H89_IO_NMI)
		{
			m_out_nmi_cb(offset, data);
			return;
		}
		if (decode & H89_IO_TERM)
		{
			m_out_tlb_cb(offset & 7, data);
			return;
		}

		for (device_h89bus_right_card_interface &entry : m_right_device_list)
		{
			if (entry.m_p506_signals)
			{
				// p506 does not have CASS or LP
				entry.write(decode &  ~(H89_IO_CASS | H89_IO_LP), offset & 7, data);
			}
			else
			{
				// p504/p505 does not have FLPY
				entry.write(decode & ~H89_IO_FLPY, offset & 7, data);
			}
		}
	}

	// service left-slot cards that have a motherboard connection to snoop the I/O space
	for (device_h89bus_left_card_interface &entry : m_left_device_list)
	{
		entry.write(offset & 0xff, data);
	}
}

/*
  The H89 supported 16K, 32K, 48K, or 64K of RAM. The first 8K of address space
  is reserved for the monitor ROM, floppy ROM, and scratch pad RAM. For 16k-48K
  sizes, the upper 8k of memory is remapped to the first 8K when the ROM is disabled.
  For systems with 64K of RAM, the upper half of the expansion board is permanently
  mapped to the lower 8K. Even when ROM is mapped, any writes will still occur
  to the RAM.

  H89 Lower 8K address map

        HDOS Mode                       CP/M Mode
  ------------------- 0x2000 (8k) ----------------
  |   Floppy ROM   |                |            |
  ------------------- 0x1800 (6k)   |            |
  |   Floppy RAM   |                |            |
  ------------------- 0x1400 (5k)   |    RAM     |
  |      Open      |                |            |
  ------------------- 0x1000 (4k)   |            |
  |   MTR-90 ROM   |                |            |
  -................-- 0x0800 (2k)   |            |
  | MTR(88/89) ROM |                |            |
  ------------------- 0x0000 (0k) ----------------


        16K RAM Example

      HDOS                           CP/M
  ------------- 24k
  |    RAM    |  ------+
  ------------- 16k    |         ------------- 16k
  |    RAM    |  ------------->  |    RAM    |
  -------------  8k    |         -------------  8k
  |    ROM    |        +------>  |    RAM    |
  -------------  0k              -------------  0k

*/
u8 h89bus_device::mem_dispatch_r(offs_t offset)
{
	u8 retval = 0;
	bool checkedCards = false;

	u8 val = (m_mem1 << 7) | jj501_502 | 0x08 | BIT(offset, 13, 3);
	u8 decode = m_mem_primary_decode_prom[val] ^ 0xff;

	LOGMEMREAD("mem_r - offset: 0x%04x val: 0x%02x decode: 0x%02x\n", offset, val, decode);

	if (decode)
	{
		if (decode & H89_MEM_PRI_NOMEM)
		{
			LOGMEMREAD("mem_r - PRI_NOMEM\n");
			// return default 0.
			goto done;
		}
		if ((decode & H89_MEM_PRI_U516))
		{
			u8 sec_val = (m_fmwe << 4) | ((BIT(decode, 7) ^ 1) << 3) | BIT(offset, 10, 3);
			u8 sec_decode = m_mem_secondary_decode_prom[sec_val] ^ 0xff;
			LOGMEMREAD("mem_r - PRI_U516 - sec_val: 0x%02x sec_decode: 0x%02x\n", sec_decode, sec_val);

			for (device_h89bus_left_card_interface &entry : m_left_device_list)
			{
				retval |= entry.mem_read(decode, sec_decode, offset);
			}
			checkedCards = true;

			if (sec_decode & H89_MEM_SEC_SYS_ROM)
			{
				LOGMEMREAD("mem_r - SEC_SYS_ROM\n");
				retval = m_in_sys_rom_cb(offset & 0x1fff);
			}
			if (sec_decode & H89_MEM_SEC_OPT_ROM)
			{
				LOGMEMREAD("mem_r - SEC_OPT_ROM\n");
				retval = m_in_opt_rom_cb(offset & 0x1fff);
			}
			if (sec_decode & H89_MEM_SEC_OPT_RAM)
			{
				LOGMEMREAD("mem_r - SEC_OPT_RAM\n");
				retval = m_in_opt_ram_cb(offset & 0x1fff);
			}
			if (sec_decode & H89_MEM_SEC_FPY_RAM)
			{
				LOGMEMREAD("mem_r - SEC_FPY_RAM\n");
				retval = m_in_flpy_ram_cb(offset & 0x1fff);
			}
			if (sec_decode & H89_MEM_SEC_FPY_ROM)
			{
				LOGMEMREAD("mem_r - SEC_FPY_ROM\n");
				retval = m_in_flpy_rom_cb(offset & 0x1fff);
			}
		}

		if (!checkedCards)
		{
			for (device_h89bus_left_card_interface &entry : m_left_device_list)
			{
				u8 sec_select = 0;

				retval |= entry.mem_read(decode, sec_select, offset);
			}
		}

		if (decode & H89_MEM_PRI_RAS0)
		{
			LOGMEMREAD("mem_r - PRI_RAS0\n");
			retval = m_in_bank0_cb(offset & 0x3fff);
		}
		if (decode & H89_MEM_PRI_RAS1)
		{
			LOGMEMREAD("mem_r - PRI_RAS1\n");
			retval = m_in_bank1_cb(offset & 0x3fff);
		}
		if (decode & H89_MEM_PRI_RAS2)
		{
			LOGMEMREAD("mem_r - PRI_RAS2\n");
			retval = m_in_bank2_cb(offset & 0x3fff);
		}
	}

done:
	LOGMEMREAD("mem_r - retval: decode: 0x%02x\n", retval);

	return retval;
}

void h89bus_device::mem_dispatch_w(offs_t offset, u8 data)
{
	u8 val = (m_mem1 << 7) | jj501_502 | 0x18 | BIT(offset, 13, 3);
	u8 decode = m_mem_primary_decode_prom[val] ^ 0xff;
	bool checkedCards = false;

	LOGMEMWRITE("mem_w - offset: 0x%04x decode: 0x%02x data: 0x%02x\n", offset, decode, data);

	if (decode)
	{
		if (decode & H89_MEM_PRI_NOMEM)
		{
			// no memory, do nothing with the write
			return;
		}
		if ((decode & H89_MEM_PRI_U516))
		{
			u8 sec_val = (m_fmwe << 4) | ((BIT(decode, 7) ^ 1) << 3) | BIT(offset, 10, 3);
			u8 sec_decode = m_mem_secondary_decode_prom[sec_val] ^ 0xff;
			LOGMEMWRITE("mem_w - PRI_U516 - sec_decode: 0x%02x sec_decode: 0x%02x\n", sec_decode, sec_val);

			for (device_h89bus_left_card_interface &entry : m_left_device_list)
			{
				entry.mem_write(decode, sec_decode, offset, data);
			}
			checkedCards = true;

			if (sec_decode & H89_MEM_SEC_OPT_RAM)
			{
				LOGMEMWRITE("mem_w - SEC_OPT_RAM\n");
				m_out_opt_ram_cb(offset & 0x1fff, data);
			}
			if (sec_decode & H89_MEM_SEC_FPY_RAM)
			{
				LOGMEMWRITE("mem_w - SEC_FPY_RAM\n");
				m_out_flpy_ram_cb(offset & 0x1fff, data);
			}
		}

		if (!checkedCards)
		{
			for (device_h89bus_left_card_interface &entry : m_left_device_list)
			{
				u8 sec_select = 0;

				entry.mem_write(decode, sec_select, offset, data);
			}
		}

		if (decode & H89_MEM_PRI_RAS0)
		{
			LOGMEMWRITE("mem_w - PRI_RAS0\n");
			m_out_bank0_cb(offset & 0x3fff, data);
		}
		if (decode & H89_MEM_PRI_RAS1)
		{
			LOGMEMWRITE("mem_w - PRI_RAS1\n");
			m_out_bank1_cb(offset & 0x3fff, data);
		}
		if (decode & H89_MEM_PRI_RAS2)
		{
			LOGMEMWRITE("mem_w - PRI_RAS2\n");
			m_out_bank2_cb(offset & 0x3fff, data);
		}
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

void h89bus_device::set_fmwe_line(int state)
{
	m_fmwe = state;

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

