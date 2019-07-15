// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs Disk Jockey/DMA floppy controller board emulation

**********************************************************************/

#include "emu.h"
#include "djdma.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define Z80_TAG     "14a"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(S100_DJDMA, s100_djdma_device, "s100_djdma", "Morrow Disk Jockey/DMA FDC")


//-------------------------------------------------
//  ROM( djdma )
//-------------------------------------------------

ROM_START( djdma )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "djdma 2.5 26c2.16d", 0x0000, 0x1000, CRC(71ff1924) SHA1(6907575954836364826b8fdef3c108bb93bf3d25) )

	ROM_REGION( 0x200, "paddr", 0 )
	ROM_LOAD( "djdma2x.3d", 0x000, 0x200, CRC(f9b1648b) SHA1(1ebe6dc8ccfbfa6c7dc98cb65fbc9fa21e3b687f) ) // 6306

	ROM_REGION( 0x100, "zaddr", 0 )
	ROM_LOAD( "dja-12b.12b", 0x000, 0x100, CRC(040044af) SHA1(d069dc0e6b680cb8848d165aff6681ed2d750961) ) // 6301

	ROM_REGION( 0x200, "cmdaddr", 0 )
	ROM_LOAD( "dj-11c-a.11c", 0x000, 0x200, CRC(0c6c4af0) SHA1(8fdcd34e3d07add793ff9ba27c77af864e1731bb) ) // 6305

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "djdma-2b.2b", 0x000, 0x104, CRC(d6925f2c) SHA1(1e58dfb7b8a2a5bbaa6589d4018042626fd5ceaf) ) // PAL16R4
	ROM_LOAD( "djdma 2c 81d5.2c", 0x0000, 0x10, NO_DUMP ) // 82S105
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *s100_djdma_device::device_rom_region() const
{
	return ROM_NAME( djdma );
}


//-------------------------------------------------
//  ADDRESS_MAP( djdma_mem )
//-------------------------------------------------

void s100_djdma_device::djdma_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("14a", 0);
	map(0x1000, 0x13ff).mirror(0xc00).ram();
	map(0x4000, 0x4000).mirror(0xff8).w(FUNC(s100_djdma_device::reset_int_w));
	map(0x4001, 0x4001).mirror(0xff8).rw(FUNC(s100_djdma_device::disk_di_r), FUNC(s100_djdma_device::disk_do_w));
	map(0x4002, 0x4002).mirror(0xff8).rw(FUNC(s100_djdma_device::bus_di_r), FUNC(s100_djdma_device::bus_hi_addr_w));
	map(0x4003, 0x4003).mirror(0xff8).rw(FUNC(s100_djdma_device::disk_status_r), FUNC(s100_djdma_device::bus_status_w));
	map(0x4004, 0x4004).mirror(0xff8).rw(FUNC(s100_djdma_device::bus_request_r), FUNC(s100_djdma_device::dro3_w));
	map(0x4005, 0x4005).mirror(0xff8).rw(FUNC(s100_djdma_device::bus_release_r), FUNC(s100_djdma_device::dro2_w));
	map(0x4006, 0x4006).mirror(0xff8).w(FUNC(s100_djdma_device::dro1_w));
	map(0x4007, 0x4007).mirror(0xff8).w(FUNC(s100_djdma_device::dro0_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( djdma_io )
//-------------------------------------------------

void s100_djdma_device::djdma_io(address_map &map)
{
	map(0x0000, 0xffff).w(FUNC(s100_djdma_device::bus_stb_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void s100_djdma_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_diskcpu, 4_MHz_XTAL);
	m_diskcpu->set_addrmap(AS_PROGRAM, &s100_djdma_device::djdma_mem);
	m_diskcpu->set_addrmap(AS_IO, &s100_djdma_device::djdma_io);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s100_djdma_device - constructor
//-------------------------------------------------

s100_djdma_device::s100_djdma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, S100_DJDMA, tag, owner, clock)
	, device_s100_card_interface(mconfig, *this)
	, m_diskcpu(*this, Z80_TAG)
	, m_cmdaddr(*this, "cmdaddr")
	, m_bus_hold(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void s100_djdma_device::device_start()
{
	save_item(NAME(m_bus_hold));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void s100_djdma_device::device_reset()
{
	dro0_w(0);
	dro1_w(0);

	// Bus hold should also be set here for bootstrapping on the Decision I (jumper option)
}


//-------------------------------------------------
//  s100_sout_w - I/O write
//-------------------------------------------------

void s100_djdma_device::s100_sout_w(offs_t offset, uint8_t data)
{
	// O4 = /ATTN (responds to address EF only)
	if (!BIT(m_cmdaddr[offset & 0xff], 3))
		m_diskcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


//-------------------------------------------------
//  reset_int_w - reset interrupt flip-flop for
//  the onboard Z80
//-------------------------------------------------

void s100_djdma_device::reset_int_w(u8 data)
{
	m_diskcpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}


//-------------------------------------------------
//  disk_di_r - read one byte of disk data
//-------------------------------------------------

u8 s100_djdma_device::disk_di_r()
{
	return 0;
}


//-------------------------------------------------
//  disk_do_w - write one byte of disk data
//-------------------------------------------------

void s100_djdma_device::disk_do_w(u8 data)
{
	// What does this do precisely? The schematic doesn't cover it...
}


//-------------------------------------------------
//  bus_di_r - read command off the S-100 bus
//-------------------------------------------------

u8 s100_djdma_device::bus_di_r()
{
	return 0xff;
}


//-------------------------------------------------
//  bus_hi_addr_w - preload A16-A23 for extended
//  memory addressing
//-------------------------------------------------

void s100_djdma_device::bus_hi_addr_w(u8 data)
{
}


//-------------------------------------------------
//  bus_status_w - preload S-100 status signals
//-------------------------------------------------

void s100_djdma_device::bus_status_w(u8 data)
{
	// D7 = SMEMR
	// D6 = SM1
	// D5 = /SWO
	// D4 = SOUT
	// D3 = SINP
	// D2 = SINTA
	// D1 = SHLTA
	// D0 = /ERROR
}


//-------------------------------------------------
//  bus_request_r - set the bus hold flip-flop
//-------------------------------------------------

u8 s100_djdma_device::bus_request_r()
{
	if (!machine().side_effects_disabled())
		m_bus_hold = true;

	return 0xff;
}


//-------------------------------------------------
//  bus_release_r - reset the bus hold flip-flop
//-------------------------------------------------

u8 s100_djdma_device::bus_release_r()
{
	if (!machine().side_effects_disabled())
		m_bus_hold = false;

	return 0xff;
}


//-------------------------------------------------
//  bus_stb_w - initiate a read/write cycle as
//  temporary S-100 bus master
//-------------------------------------------------

void s100_djdma_device::bus_stb_w(offs_t offset, u8 data)
{
}


//-------------------------------------------------
//  disk_status_r - read drive status
//-------------------------------------------------

u8 s100_djdma_device::disk_status_r()
{
	// D7 = READY (P2:22)
	// D6 = WPROT (P1:44, P2:28)
	// D5 = TRACK 0 (P1:42, P2:26)
	// D4 = INDEX/SECTOR (P1:20, P2:8)
	// D3 = DISK CHANGE (P2:12)
	// D2 = TWO SIDED (P2:10)
	// D1 = SERIN (P3:2 RS232 IN)
	// D0 = ATTN/ERROR
	// All signals are active low, but read through 81LS96 inverting buffer.

	return 0xff;
}


//-------------------------------------------------
//  dro0_w - set the first disk control register
//-------------------------------------------------

void s100_djdma_device::dro0_w(u8 data)
{
	// D7 = WR CNTL (P1:40, P2:24 WRITE GATE)
	// D6 = RD CNTL
	// D5 = M1
	// D4 = M0
	// D3 = TYPE 1
	// D2 = TYPE 0
	// D1 = LEN 1
	// D0 = LEN 0
}


//-------------------------------------------------
//  dro1_w - set the second disk control register
//-------------------------------------------------

void s100_djdma_device::dro1_w(u8 data)
{
	// D7 = W/R
	// D6 = PM
	// D5 = MINI
	// D4 = PRE COMP
	// D3 = SEROUT (P3:3 RS232 OUT)
	// D2 = ENBL DVRS (0 disables drive select latches)
	// D1 = CLR
	// D0 = INT RQ (jumpered to any of VI0-7 or PINT)
}


//-------------------------------------------------
//  dro2_w - select drives on 8″ floppy port
//-------------------------------------------------

void s100_djdma_device::dro2_w(u8 data)
{
	// D7 = /DRIVE 1 (P1:26)
	// D6 = /DRIVE 2 (P1:28)
	// D5 = /DRIVE 3 (P1:30)
	// D4 = /DRIVE 4 (P1:34)
	// D3 = /DIRECTION (P1:32)
	// D2 = /STEP (P1:36)
	// D1 = /SIDE SELECT (P1:14)
	// D0 = /LOW CURRENT (P1:2)
}


//-------------------------------------------------
//  dro3_w - select drives on 5¼″ floppy port
//-------------------------------------------------

void s100_djdma_device::dro3_w(u8 data)
{
	// D7 = /DRIVE 1 (P2:10)
	// D6 = /DRIVE 2 (P2:12)
	// D5 = /DRIVE 3 (P2:14)
	// D4 = /DRIVE 4 (P2:6)
	// D3 = /DIRECTION (P2:18)
	// D2 = /STEP (P2:20)
	// D1 = /SIDE SELECT (P2:36)
	// D0 = /MOTOR ON (P2:16)
}
