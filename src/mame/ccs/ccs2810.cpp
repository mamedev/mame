// license:BSD-3-Clause
// copyright-holders:Robbbert,AJR
/***************************************************************************

CCS Model 2810

2009-12-11 Skeleton driver.
2011-06-02 Connected to a terminal

Chips: INS8250N-B, Z80A, uPD2716D, 82S129. Crystals: 16 MHz, 1.8432MHz

SYSTEM OPERATION:
Press Enter up to three times to start the system.
Supported baud rates include 50, 75, 110, 134.5, 150, 200, 300, 600, 1200,
1800, 2000, 2400, 3600, 7200, 9600, 14400, 38400, 57600 and 115200.
All commands are in uppercase.

A    Assign logical device
Dn,n Dump memory
E    Punch End-of-File to paper tape
F    Fill
G    Go
H    Hex arithmetic
I    In
L    Punch Leader to paper tape
M    Move
O    Out
Q    Query logical devices
R    Read a file from paper tape
S    Edit memory
T    Test memory
V    Verify (compare 2 blocks of memory)
W    Write a file to paper tape
X    Examine Registers
Y    Set Baud rate of i8250
Z    Zleep (lock terminal). Press control+G twice to unlock.

*****************************************************************************

CCS Model 2422B

Chips: UB1793/MB8877, 3 proms and one eprom. Crystal = 16MHz

SYSTEM OPERATION:
Same as above, plus some extra commands

B    Boot from floppy
L    removed
P    Set disk parameters e.g. P0 10 0 = drive A, 10 sectors per track, 1 sided
Q    Set disk position for raw read/write e.g. Q6 0 9 = track 6, side 0, sector 9
Rs f Read absolute disk data (set by P and Q) to memory range s to f
Ws f Write absolute disk data (set by P and Q) from memory range s to f

ToDo:
- fdc hld_r() has no code behind it, to be written
- not sure of the polarities of some of the floppy/fdc signals (not documented)
- no obvious option to change the fdc clock
- currently hard coded for a 20cm drive, not sure how to option select drive sizes
- a few jumpers to be added
- the one disk that exists fails the test:
  Incorrect layout on track 0 head 0, expected_size=41666, current_size=68144

*****************************************************************************

CCS Model 300 / 400

2009-12-11 Skeleton driver.

It requires a floppy disk to boot from.

Early on, it does a read from port F2. If bit 3 is low, the system becomes
a Model 400.

The CPU board appears to be similar to the 2820 System Processor, which has
Z80A CTC, Z80A PIO, Z80A SIO/0 and Z80A DMA peripherals on board. Several
features, including IEI/IEO daisy chain priority, are jumper-configurable.

However, the 2820 has the i/o ports rearranged slightly (even though the
manual says it should work!), and no fdc support.

ToDo:
- Using the 2422's FDC, since the ports are the same
- As before, the only disks that exist cause an unexpected exit.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"
#include "imagedev/floppy.h"
#include "machine/ins8250.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"


namespace {

class ccs_state : public driver_device
{
public:
	ccs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_rom(*this, "maincpu")
		, m_ins8250(*this, "ins8250")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_jump_addr_sel(*this, {"ADDRLO", "ADDRHI"})
		, m_ser_addr_sel(*this, "SERADDR")
		, m_jump_en(*this, "JMPEN")
		, m_rom_en(*this, "ROMEN")
		, m_ser_en(*this, "SEREN")
	{ }

	void ccs2810(machine_config &config);
	void ccs2422(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	u8 port04_r();
	u8 port34_r();
	void port04_w(u8 data);
	void port34_w(u8 data);

	bool m_ss = 0;
	bool m_dden = 0;
	bool m_dsize = 0;
	u8 m_ds = 0U;
	floppy_image_device *m_floppy;

	required_device<z80_device> m_maincpu;
	optional_device<ram_device> m_ram;
	required_region_ptr<u8> m_rom;
	optional_device<ins8250_device> m_ins8250;
	optional_device<mb8877_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_ioport_array<2> m_jump_addr_sel;
	optional_ioport m_ser_addr_sel;
	optional_ioport m_jump_en;
	optional_ioport m_rom_en;
	optional_ioport m_ser_en;

private:
	u8 memory_read(offs_t offset);
	void memory_write(offs_t offset, u8 data);
	u8 io_read(offs_t offset);
	void io_write(offs_t offset, u8 data);

	void port40_w(u8 data);

	void ccs2422_io(address_map &map) ATTR_COLD;
	void ccs2810_io(address_map &map) ATTR_COLD;
	void ccs2810_mem(address_map &map) ATTR_COLD;

	u8 m_power_on_status = 0U;
};

class ccs300_state : public ccs_state
{
public:
	ccs300_state(const machine_config &mconfig, device_type type, const char *tag)
		: ccs_state(mconfig, type, tag)
		, m_ram1(*this, "mainram")
		, m_bank1(*this, "bank1")
	{ }

	void ccs300(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	void ccs300_io(address_map &map) ATTR_COLD;
	void ccs300_mem(address_map &map) ATTR_COLD;
	void port40_w(u8 data);
	required_shared_ptr<u8> m_ram1;
	required_memory_bank    m_bank1;
};

u8 ccs_state::memory_read(offs_t offset)
{
	u8 result = m_ram->read(offset);

	if (!BIT(m_power_on_status, 0))
	{
		if (BIT(m_power_on_status, 1))
			result = m_jump_addr_sel[1]->read();
		else
			result = !BIT(m_power_on_status, 2) ? 0xc3 : m_jump_addr_sel[0]->read();
	}
	else if ((offset & 0xf800) == 0xf000 && m_rom_en->read() == 0)
	{
		result = m_rom[offset & 0x7ff];

		// wait state forced for 4 MHz operation
		if (!machine().side_effects_disabled())
			m_maincpu->adjust_icount(-1);
	}

	if (!machine().side_effects_disabled())
		m_power_on_status |= m_power_on_status >> 1;

	return result;
}

void ccs_state::memory_write(offs_t offset, u8 data)
{
	m_ram->write(offset, data);
}

u8 ccs_state::io_read(offs_t offset)
{
	// A7-A3 are compared against jumper settings
	if (m_ser_en->read() && (offset & 0x00f8) == m_ser_addr_sel->read())
		return m_ins8250->ins8250_r(offset & 7);

	return 0xff;
}

void ccs_state::io_write(offs_t offset, u8 data)
{
	// A7-A3 are compared against jumper settings
	if (m_ser_en->read() && (offset & 0x00f8) == m_ser_addr_sel->read())
		m_ins8250->ins8250_w(offset & 7, data);
}

void ccs_state::ccs2810_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(ccs_state::memory_read), FUNC(ccs_state::memory_write));
}

void ccs_state::ccs2810_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(ccs_state::io_read), FUNC(ccs_state::io_write));
}

void ccs_state::ccs2422_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(ccs_state::io_read), FUNC(ccs_state::io_write));
	map(0x04, 0x04).mirror(0xff00).rw(FUNC(ccs_state::port04_r), FUNC(ccs_state::port04_w));
	map(0x30, 0x33).mirror(0xff00).rw(m_fdc, FUNC(mb8877_device::read), FUNC(mb8877_device::write));
	map(0x34, 0x34).mirror(0xff00).rw(FUNC(ccs_state::port34_r), FUNC(ccs_state::port34_w));
	map(0x40, 0x40).mirror(0xff00).w(FUNC(ccs_state::port40_w));
}

void ccs300_state::ccs300_mem(address_map &map)
{
	map(0x0000, 0xffff).ram().share("mainram");
	map(0x0000, 0x07ff).bankr("bank1");
}

void ccs300_state::ccs300_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x04, 0x04).rw(FUNC(ccs300_state::port04_r), FUNC(ccs300_state::port04_w));
	map(0x10, 0x13).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x14, 0x17).rw("pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x18, 0x1b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x30, 0x33).rw(m_fdc, FUNC(mb8877_device::read), FUNC(mb8877_device::write));
	map(0x34, 0x34).rw(FUNC(ccs300_state::port34_r), FUNC(ccs300_state::port34_w));
	map(0x40, 0x40).w(FUNC(ccs300_state::port40_w));
	map(0xf0, 0xf0).rw("dma", FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0xf2, 0xf2).portr("MODEL"); // dip or jumper? only used by CCS-400
}


/* Input ports */
static INPUT_PORTS_START( ccs2810 )
	PORT_START("ROMEN")
	PORT_DIPNAME(1, 0, "Enable On-board ROM") PORT_DIPLOCATION("ROM EN:1")
	PORT_DIPSETTING(1, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))

	PORT_START("JMPEN")
	PORT_DIPNAME(1, 0, "Enable Power-on Jump") PORT_DIPLOCATION("JMP EN:1")
	PORT_DIPSETTING(1, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))

	PORT_START("ADDRLO")
	PORT_DIPNAME(0xff, 0x00, "Power-on Jump Address (Low)") PORT_DIPLOCATION("JA:!16,!15,!14,!13,!12,!11,!10,!9")
	PORT_DIPSETTING(0x00, "XX00h")
	PORT_DIPSETTING(0x01, "XX01h")
	PORT_DIPSETTING(0x02, "XX02h")
	PORT_DIPSETTING(0x03, "XX03h")
	PORT_DIPSETTING(0x04, "XX04h")
	PORT_DIPSETTING(0x05, "XX05h")
	PORT_DIPSETTING(0x06, "XX06h")
	PORT_DIPSETTING(0x07, "XX07h")
	PORT_DIPSETTING(0x08, "XX08h")
	PORT_DIPSETTING(0x09, "XX09h")
	PORT_DIPSETTING(0x0a, "XX0Ah")
	PORT_DIPSETTING(0x0b, "XX0Bh")
	PORT_DIPSETTING(0x0c, "XX0Ch")
	PORT_DIPSETTING(0x0d, "XX0Dh")
	PORT_DIPSETTING(0x0e, "XX0Eh")
	PORT_DIPSETTING(0x0f, "XX0Fh")
	PORT_DIPSETTING(0x10, "XX10h")
	PORT_DIPSETTING(0x11, "XX11h")
	PORT_DIPSETTING(0x12, "XX12h")
	PORT_DIPSETTING(0x13, "XX13h")
	PORT_DIPSETTING(0x14, "XX14h")
	PORT_DIPSETTING(0x15, "XX15h")
	PORT_DIPSETTING(0x16, "XX16h")
	PORT_DIPSETTING(0x17, "XX17h")
	PORT_DIPSETTING(0x18, "XX18h")
	PORT_DIPSETTING(0x19, "XX19h")
	PORT_DIPSETTING(0x1a, "XX1Ah")
	PORT_DIPSETTING(0x1b, "XX1Bh")
	PORT_DIPSETTING(0x1c, "XX1Ch")
	PORT_DIPSETTING(0x1d, "XX1Dh")
	PORT_DIPSETTING(0x1e, "XX1Eh")
	PORT_DIPSETTING(0x1f, "XX1Fh")
	PORT_DIPSETTING(0x20, "XX20h")
	PORT_DIPSETTING(0x21, "XX21h")
	PORT_DIPSETTING(0x22, "XX22h")
	PORT_DIPSETTING(0x23, "XX23h")
	PORT_DIPSETTING(0x24, "XX24h")
	PORT_DIPSETTING(0x25, "XX25h")
	PORT_DIPSETTING(0x26, "XX26h")
	PORT_DIPSETTING(0x27, "XX27h")
	PORT_DIPSETTING(0x28, "XX28h")
	PORT_DIPSETTING(0x29, "XX29h")
	PORT_DIPSETTING(0x2a, "XX2Ah")
	PORT_DIPSETTING(0x2b, "XX2Bh")
	PORT_DIPSETTING(0x2c, "XX2Ch")
	PORT_DIPSETTING(0x2d, "XX2Dh")
	PORT_DIPSETTING(0x2e, "XX2Eh")
	PORT_DIPSETTING(0x2f, "XX2Fh")
	PORT_DIPSETTING(0x30, "XX30h")
	PORT_DIPSETTING(0x31, "XX31h")
	PORT_DIPSETTING(0x32, "XX32h")
	PORT_DIPSETTING(0x33, "XX33h")
	PORT_DIPSETTING(0x34, "XX34h")
	PORT_DIPSETTING(0x35, "XX35h")
	PORT_DIPSETTING(0x36, "XX36h")
	PORT_DIPSETTING(0x37, "XX37h")
	PORT_DIPSETTING(0x38, "XX38h")
	PORT_DIPSETTING(0x39, "XX39h")
	PORT_DIPSETTING(0x3a, "XX3Ah")
	PORT_DIPSETTING(0x3b, "XX3Bh")
	PORT_DIPSETTING(0x3c, "XX3Ch")
	PORT_DIPSETTING(0x3d, "XX3Dh")
	PORT_DIPSETTING(0x3e, "XX3Eh")
	PORT_DIPSETTING(0x3f, "XX3Fh")
	PORT_DIPSETTING(0x40, "XX40h")
	PORT_DIPSETTING(0x41, "XX41h")
	PORT_DIPSETTING(0x42, "XX42h")
	PORT_DIPSETTING(0x43, "XX43h")
	PORT_DIPSETTING(0x44, "XX44h")
	PORT_DIPSETTING(0x45, "XX45h")
	PORT_DIPSETTING(0x46, "XX46h")
	PORT_DIPSETTING(0x47, "XX47h")
	PORT_DIPSETTING(0x48, "XX48h")
	PORT_DIPSETTING(0x49, "XX49h")
	PORT_DIPSETTING(0x4a, "XX4Ah")
	PORT_DIPSETTING(0x4b, "XX4Bh")
	PORT_DIPSETTING(0x4c, "XX4Ch")
	PORT_DIPSETTING(0x4d, "XX4Dh")
	PORT_DIPSETTING(0x4e, "XX4Eh")
	PORT_DIPSETTING(0x4f, "XX4Fh")
	PORT_DIPSETTING(0x50, "XX50h")
	PORT_DIPSETTING(0x51, "XX51h")
	PORT_DIPSETTING(0x52, "XX52h")
	PORT_DIPSETTING(0x53, "XX53h")
	PORT_DIPSETTING(0x54, "XX54h")
	PORT_DIPSETTING(0x55, "XX55h")
	PORT_DIPSETTING(0x56, "XX56h")
	PORT_DIPSETTING(0x57, "XX57h")
	PORT_DIPSETTING(0x58, "XX58h")
	PORT_DIPSETTING(0x59, "XX59h")
	PORT_DIPSETTING(0x5a, "XX5Ah")
	PORT_DIPSETTING(0x5b, "XX5Bh")
	PORT_DIPSETTING(0x5c, "XX5Ch")
	PORT_DIPSETTING(0x5d, "XX5Dh")
	PORT_DIPSETTING(0x5e, "XX5Eh")
	PORT_DIPSETTING(0x5f, "XX5Fh")
	PORT_DIPSETTING(0x60, "XX60h")
	PORT_DIPSETTING(0x61, "XX61h")
	PORT_DIPSETTING(0x62, "XX62h")
	PORT_DIPSETTING(0x63, "XX63h")
	PORT_DIPSETTING(0x64, "XX64h")
	PORT_DIPSETTING(0x65, "XX65h")
	PORT_DIPSETTING(0x66, "XX66h")
	PORT_DIPSETTING(0x67, "XX67h")
	PORT_DIPSETTING(0x68, "XX68h")
	PORT_DIPSETTING(0x69, "XX69h")
	PORT_DIPSETTING(0x6a, "XX6Ah")
	PORT_DIPSETTING(0x6b, "XX6Bh")
	PORT_DIPSETTING(0x6c, "XX6Ch")
	PORT_DIPSETTING(0x6d, "XX6Dh")
	PORT_DIPSETTING(0x6e, "XX6Eh")
	PORT_DIPSETTING(0x6f, "XX6Fh")
	PORT_DIPSETTING(0x70, "XX70h")
	PORT_DIPSETTING(0x71, "XX71h")
	PORT_DIPSETTING(0x72, "XX72h")
	PORT_DIPSETTING(0x73, "XX73h")
	PORT_DIPSETTING(0x74, "XX74h")
	PORT_DIPSETTING(0x75, "XX75h")
	PORT_DIPSETTING(0x76, "XX76h")
	PORT_DIPSETTING(0x77, "XX77h")
	PORT_DIPSETTING(0x78, "XX78h")
	PORT_DIPSETTING(0x79, "XX79h")
	PORT_DIPSETTING(0x7a, "XX7Ah")
	PORT_DIPSETTING(0x7b, "XX7Bh")
	PORT_DIPSETTING(0x7c, "XX7Ch")
	PORT_DIPSETTING(0x7d, "XX7Dh")
	PORT_DIPSETTING(0x7e, "XX7Eh")
	PORT_DIPSETTING(0x7f, "XX7Fh")
	PORT_DIPSETTING(0x80, "XX80h")
	PORT_DIPSETTING(0x81, "XX81h")
	PORT_DIPSETTING(0x82, "XX82h")
	PORT_DIPSETTING(0x83, "XX83h")
	PORT_DIPSETTING(0x84, "XX84h")
	PORT_DIPSETTING(0x85, "XX85h")
	PORT_DIPSETTING(0x86, "XX86h")
	PORT_DIPSETTING(0x87, "XX87h")
	PORT_DIPSETTING(0x88, "XX88h")
	PORT_DIPSETTING(0x89, "XX89h")
	PORT_DIPSETTING(0x8a, "XX8Ah")
	PORT_DIPSETTING(0x8b, "XX8Bh")
	PORT_DIPSETTING(0x8c, "XX8Ch")
	PORT_DIPSETTING(0x8d, "XX8Dh")
	PORT_DIPSETTING(0x8e, "XX8Eh")
	PORT_DIPSETTING(0x8f, "XX8Fh")
	PORT_DIPSETTING(0x90, "XX90h")
	PORT_DIPSETTING(0x91, "XX91h")
	PORT_DIPSETTING(0x92, "XX92h")
	PORT_DIPSETTING(0x93, "XX93h")
	PORT_DIPSETTING(0x94, "XX94h")
	PORT_DIPSETTING(0x95, "XX95h")
	PORT_DIPSETTING(0x96, "XX96h")
	PORT_DIPSETTING(0x97, "XX97h")
	PORT_DIPSETTING(0x98, "XX98h")
	PORT_DIPSETTING(0x99, "XX99h")
	PORT_DIPSETTING(0x9a, "XX9Ah")
	PORT_DIPSETTING(0x9b, "XX9Bh")
	PORT_DIPSETTING(0x9c, "XX9Ch")
	PORT_DIPSETTING(0x9d, "XX9Dh")
	PORT_DIPSETTING(0x9e, "XX9Eh")
	PORT_DIPSETTING(0x9f, "XX9Fh")
	PORT_DIPSETTING(0xa0, "XXA0h")
	PORT_DIPSETTING(0xa1, "XXA1h")
	PORT_DIPSETTING(0xa2, "XXA2h")
	PORT_DIPSETTING(0xa3, "XXA3h")
	PORT_DIPSETTING(0xa4, "XXA4h")
	PORT_DIPSETTING(0xa5, "XXA5h")
	PORT_DIPSETTING(0xa6, "XXA6h")
	PORT_DIPSETTING(0xa7, "XXA7h")
	PORT_DIPSETTING(0xa8, "XXA8h")
	PORT_DIPSETTING(0xa9, "XXA9h")
	PORT_DIPSETTING(0xaa, "XXAAh")
	PORT_DIPSETTING(0xab, "XXABh")
	PORT_DIPSETTING(0xac, "XXACh")
	PORT_DIPSETTING(0xad, "XXADh")
	PORT_DIPSETTING(0xae, "XXAEh")
	PORT_DIPSETTING(0xaf, "XXAFh")
	PORT_DIPSETTING(0xb0, "XXB0h")
	PORT_DIPSETTING(0xb1, "XXB1h")
	PORT_DIPSETTING(0xb2, "XXB2h")
	PORT_DIPSETTING(0xb3, "XXB3h")
	PORT_DIPSETTING(0xb4, "XXB4h")
	PORT_DIPSETTING(0xb5, "XXB5h")
	PORT_DIPSETTING(0xb6, "XXB6h")
	PORT_DIPSETTING(0xb7, "XXB7h")
	PORT_DIPSETTING(0xb8, "XXB8h")
	PORT_DIPSETTING(0xb9, "XXB9h")
	PORT_DIPSETTING(0xba, "XXBAh")
	PORT_DIPSETTING(0xbb, "XXBBh")
	PORT_DIPSETTING(0xbc, "XXBCh")
	PORT_DIPSETTING(0xbd, "XXBDh")
	PORT_DIPSETTING(0xbe, "XXBEh")
	PORT_DIPSETTING(0xbf, "XXBFh")
	PORT_DIPSETTING(0xc0, "XXC0h")
	PORT_DIPSETTING(0xc1, "XXC1h")
	PORT_DIPSETTING(0xc2, "XXC2h")
	PORT_DIPSETTING(0xc3, "XXC3h")
	PORT_DIPSETTING(0xc4, "XXC4h")
	PORT_DIPSETTING(0xc5, "XXC5h")
	PORT_DIPSETTING(0xc6, "XXC6h")
	PORT_DIPSETTING(0xc7, "XXC7h")
	PORT_DIPSETTING(0xc8, "XXC8h")
	PORT_DIPSETTING(0xc9, "XXC9h")
	PORT_DIPSETTING(0xca, "XXCAh")
	PORT_DIPSETTING(0xcb, "XXCBh")
	PORT_DIPSETTING(0xcc, "XXCCh")
	PORT_DIPSETTING(0xcd, "XXCDh")
	PORT_DIPSETTING(0xce, "XXCEh")
	PORT_DIPSETTING(0xcf, "XXCFh")
	PORT_DIPSETTING(0xd0, "XXD0h")
	PORT_DIPSETTING(0xd1, "XXD1h")
	PORT_DIPSETTING(0xd2, "XXD2h")
	PORT_DIPSETTING(0xd3, "XXD3h")
	PORT_DIPSETTING(0xd4, "XXD4h")
	PORT_DIPSETTING(0xd5, "XXD5h")
	PORT_DIPSETTING(0xd6, "XXD6h")
	PORT_DIPSETTING(0xd7, "XXD7h")
	PORT_DIPSETTING(0xd8, "XXD8h")
	PORT_DIPSETTING(0xd9, "XXD9h")
	PORT_DIPSETTING(0xda, "XXDAh")
	PORT_DIPSETTING(0xdb, "XXDBh")
	PORT_DIPSETTING(0xdc, "XXDCh")
	PORT_DIPSETTING(0xdd, "XXDDh")
	PORT_DIPSETTING(0xde, "XXDEh")
	PORT_DIPSETTING(0xdf, "XXDFh")
	PORT_DIPSETTING(0xe0, "XXE0h")
	PORT_DIPSETTING(0xe1, "XXE1h")
	PORT_DIPSETTING(0xe2, "XXE2h")
	PORT_DIPSETTING(0xe3, "XXE3h")
	PORT_DIPSETTING(0xe4, "XXE4h")
	PORT_DIPSETTING(0xe5, "XXE5h")
	PORT_DIPSETTING(0xe6, "XXE6h")
	PORT_DIPSETTING(0xe7, "XXE7h")
	PORT_DIPSETTING(0xe8, "XXE8h")
	PORT_DIPSETTING(0xe9, "XXE9h")
	PORT_DIPSETTING(0xea, "XXEAh")
	PORT_DIPSETTING(0xeb, "XXEBh")
	PORT_DIPSETTING(0xec, "XXECh")
	PORT_DIPSETTING(0xed, "XXEDh")
	PORT_DIPSETTING(0xee, "XXEEh")
	PORT_DIPSETTING(0xef, "XXEFh")
	PORT_DIPSETTING(0xf0, "XXF0h")
	PORT_DIPSETTING(0xf1, "XXF1h")
	PORT_DIPSETTING(0xf2, "XXF2h")
	PORT_DIPSETTING(0xf3, "XXF3h")
	PORT_DIPSETTING(0xf4, "XXF4h")
	PORT_DIPSETTING(0xf5, "XXF5h")
	PORT_DIPSETTING(0xf6, "XXF6h")
	PORT_DIPSETTING(0xf7, "XXF7h")
	PORT_DIPSETTING(0xf8, "XXF8h")
	PORT_DIPSETTING(0xf9, "XXF9h")
	PORT_DIPSETTING(0xfa, "XXFAh")
	PORT_DIPSETTING(0xfb, "XXFBh")
	PORT_DIPSETTING(0xfc, "XXFCh")
	PORT_DIPSETTING(0xfd, "XXFDh")
	PORT_DIPSETTING(0xfe, "XXFEh")
	PORT_DIPSETTING(0xff, "XXFFh")

	PORT_START("ADDRHI")
	PORT_DIPNAME(0xff, 0xf0, "Power-on Jump Address (High)") PORT_DIPLOCATION("JA:!8,!7,!6,!5,!4,!3,!2,!1")
	PORT_DIPSETTING(0x00, "00XXh")
	PORT_DIPSETTING(0x01, "01XXh")
	PORT_DIPSETTING(0x02, "02XXh")
	PORT_DIPSETTING(0x03, "03XXh")
	PORT_DIPSETTING(0x04, "04XXh")
	PORT_DIPSETTING(0x05, "05XXh")
	PORT_DIPSETTING(0x06, "06XXh")
	PORT_DIPSETTING(0x07, "07XXh")
	PORT_DIPSETTING(0x08, "08XXh")
	PORT_DIPSETTING(0x09, "09XXh")
	PORT_DIPSETTING(0x0a, "0AXXh")
	PORT_DIPSETTING(0x0b, "0BXXh")
	PORT_DIPSETTING(0x0c, "0CXXh")
	PORT_DIPSETTING(0x0d, "0DXXh")
	PORT_DIPSETTING(0x0e, "0EXXh")
	PORT_DIPSETTING(0x0f, "0FXXh")
	PORT_DIPSETTING(0x10, "10XXh")
	PORT_DIPSETTING(0x11, "11XXh")
	PORT_DIPSETTING(0x12, "12XXh")
	PORT_DIPSETTING(0x13, "13XXh")
	PORT_DIPSETTING(0x14, "14XXh")
	PORT_DIPSETTING(0x15, "15XXh")
	PORT_DIPSETTING(0x16, "16XXh")
	PORT_DIPSETTING(0x17, "17XXh")
	PORT_DIPSETTING(0x18, "18XXh")
	PORT_DIPSETTING(0x19, "19XXh")
	PORT_DIPSETTING(0x1a, "1AXXh")
	PORT_DIPSETTING(0x1b, "1BXXh")
	PORT_DIPSETTING(0x1c, "1CXXh")
	PORT_DIPSETTING(0x1d, "1DXXh")
	PORT_DIPSETTING(0x1e, "1EXXh")
	PORT_DIPSETTING(0x1f, "1FXXh")
	PORT_DIPSETTING(0x20, "20XXh")
	PORT_DIPSETTING(0x21, "21XXh")
	PORT_DIPSETTING(0x22, "22XXh")
	PORT_DIPSETTING(0x23, "23XXh")
	PORT_DIPSETTING(0x24, "24XXh")
	PORT_DIPSETTING(0x25, "25XXh")
	PORT_DIPSETTING(0x26, "26XXh")
	PORT_DIPSETTING(0x27, "27XXh")
	PORT_DIPSETTING(0x28, "28XXh")
	PORT_DIPSETTING(0x29, "29XXh")
	PORT_DIPSETTING(0x2a, "2AXXh")
	PORT_DIPSETTING(0x2b, "2BXXh")
	PORT_DIPSETTING(0x2c, "2CXXh")
	PORT_DIPSETTING(0x2d, "2DXXh")
	PORT_DIPSETTING(0x2e, "2EXXh")
	PORT_DIPSETTING(0x2f, "2FXXh")
	PORT_DIPSETTING(0x30, "30XXh")
	PORT_DIPSETTING(0x31, "31XXh")
	PORT_DIPSETTING(0x32, "32XXh")
	PORT_DIPSETTING(0x33, "33XXh")
	PORT_DIPSETTING(0x34, "34XXh")
	PORT_DIPSETTING(0x35, "35XXh")
	PORT_DIPSETTING(0x36, "36XXh")
	PORT_DIPSETTING(0x37, "37XXh")
	PORT_DIPSETTING(0x38, "38XXh")
	PORT_DIPSETTING(0x39, "39XXh")
	PORT_DIPSETTING(0x3a, "3AXXh")
	PORT_DIPSETTING(0x3b, "3BXXh")
	PORT_DIPSETTING(0x3c, "3CXXh")
	PORT_DIPSETTING(0x3d, "3DXXh")
	PORT_DIPSETTING(0x3e, "3EXXh")
	PORT_DIPSETTING(0x3f, "3FXXh")
	PORT_DIPSETTING(0x40, "40XXh")
	PORT_DIPSETTING(0x41, "41XXh")
	PORT_DIPSETTING(0x42, "42XXh")
	PORT_DIPSETTING(0x43, "43XXh")
	PORT_DIPSETTING(0x44, "44XXh")
	PORT_DIPSETTING(0x45, "45XXh")
	PORT_DIPSETTING(0x46, "46XXh")
	PORT_DIPSETTING(0x47, "47XXh")
	PORT_DIPSETTING(0x48, "48XXh")
	PORT_DIPSETTING(0x49, "49XXh")
	PORT_DIPSETTING(0x4a, "4AXXh")
	PORT_DIPSETTING(0x4b, "4BXXh")
	PORT_DIPSETTING(0x4c, "4CXXh")
	PORT_DIPSETTING(0x4d, "4DXXh")
	PORT_DIPSETTING(0x4e, "4EXXh")
	PORT_DIPSETTING(0x4f, "4FXXh")
	PORT_DIPSETTING(0x50, "50XXh")
	PORT_DIPSETTING(0x51, "51XXh")
	PORT_DIPSETTING(0x52, "52XXh")
	PORT_DIPSETTING(0x53, "53XXh")
	PORT_DIPSETTING(0x54, "54XXh")
	PORT_DIPSETTING(0x55, "55XXh")
	PORT_DIPSETTING(0x56, "56XXh")
	PORT_DIPSETTING(0x57, "57XXh")
	PORT_DIPSETTING(0x58, "58XXh")
	PORT_DIPSETTING(0x59, "59XXh")
	PORT_DIPSETTING(0x5a, "5AXXh")
	PORT_DIPSETTING(0x5b, "5BXXh")
	PORT_DIPSETTING(0x5c, "5CXXh")
	PORT_DIPSETTING(0x5d, "5DXXh")
	PORT_DIPSETTING(0x5e, "5EXXh")
	PORT_DIPSETTING(0x5f, "5FXXh")
	PORT_DIPSETTING(0x60, "60XXh")
	PORT_DIPSETTING(0x61, "61XXh")
	PORT_DIPSETTING(0x62, "62XXh")
	PORT_DIPSETTING(0x63, "63XXh")
	PORT_DIPSETTING(0x64, "64XXh")
	PORT_DIPSETTING(0x65, "65XXh")
	PORT_DIPSETTING(0x66, "66XXh")
	PORT_DIPSETTING(0x67, "67XXh")
	PORT_DIPSETTING(0x68, "68XXh")
	PORT_DIPSETTING(0x69, "69XXh")
	PORT_DIPSETTING(0x6a, "6AXXh")
	PORT_DIPSETTING(0x6b, "6BXXh")
	PORT_DIPSETTING(0x6c, "6CXXh")
	PORT_DIPSETTING(0x6d, "6DXXh")
	PORT_DIPSETTING(0x6e, "6EXXh")
	PORT_DIPSETTING(0x6f, "6FXXh")
	PORT_DIPSETTING(0x70, "70XXh")
	PORT_DIPSETTING(0x71, "71XXh")
	PORT_DIPSETTING(0x72, "72XXh")
	PORT_DIPSETTING(0x73, "73XXh")
	PORT_DIPSETTING(0x74, "74XXh")
	PORT_DIPSETTING(0x75, "75XXh")
	PORT_DIPSETTING(0x76, "76XXh")
	PORT_DIPSETTING(0x77, "77XXh")
	PORT_DIPSETTING(0x78, "78XXh")
	PORT_DIPSETTING(0x79, "79XXh")
	PORT_DIPSETTING(0x7a, "7AXXh")
	PORT_DIPSETTING(0x7b, "7BXXh")
	PORT_DIPSETTING(0x7c, "7CXXh")
	PORT_DIPSETTING(0x7d, "7DXXh")
	PORT_DIPSETTING(0x7e, "7EXXh")
	PORT_DIPSETTING(0x7f, "7FXXh")
	PORT_DIPSETTING(0x80, "80XXh")
	PORT_DIPSETTING(0x81, "81XXh")
	PORT_DIPSETTING(0x82, "82XXh")
	PORT_DIPSETTING(0x83, "83XXh")
	PORT_DIPSETTING(0x84, "84XXh")
	PORT_DIPSETTING(0x85, "85XXh")
	PORT_DIPSETTING(0x86, "86XXh")
	PORT_DIPSETTING(0x87, "87XXh")
	PORT_DIPSETTING(0x88, "88XXh")
	PORT_DIPSETTING(0x89, "89XXh")
	PORT_DIPSETTING(0x8a, "8AXXh")
	PORT_DIPSETTING(0x8b, "8BXXh")
	PORT_DIPSETTING(0x8c, "8CXXh")
	PORT_DIPSETTING(0x8d, "8DXXh")
	PORT_DIPSETTING(0x8e, "8EXXh")
	PORT_DIPSETTING(0x8f, "8FXXh")
	PORT_DIPSETTING(0x90, "90XXh")
	PORT_DIPSETTING(0x91, "91XXh")
	PORT_DIPSETTING(0x92, "92XXh")
	PORT_DIPSETTING(0x93, "93XXh")
	PORT_DIPSETTING(0x94, "94XXh")
	PORT_DIPSETTING(0x95, "95XXh")
	PORT_DIPSETTING(0x96, "96XXh")
	PORT_DIPSETTING(0x97, "97XXh")
	PORT_DIPSETTING(0x98, "98XXh")
	PORT_DIPSETTING(0x99, "99XXh")
	PORT_DIPSETTING(0x9a, "9AXXh")
	PORT_DIPSETTING(0x9b, "9BXXh")
	PORT_DIPSETTING(0x9c, "9CXXh")
	PORT_DIPSETTING(0x9d, "9DXXh")
	PORT_DIPSETTING(0x9e, "9EXXh")
	PORT_DIPSETTING(0x9f, "9FXXh")
	PORT_DIPSETTING(0xa0, "A0XXh")
	PORT_DIPSETTING(0xa1, "A1XXh")
	PORT_DIPSETTING(0xa2, "A2XXh")
	PORT_DIPSETTING(0xa3, "A3XXh")
	PORT_DIPSETTING(0xa4, "A4XXh")
	PORT_DIPSETTING(0xa5, "A5XXh")
	PORT_DIPSETTING(0xa6, "A6XXh")
	PORT_DIPSETTING(0xa7, "A7XXh")
	PORT_DIPSETTING(0xa8, "A8XXh")
	PORT_DIPSETTING(0xa9, "A9XXh")
	PORT_DIPSETTING(0xaa, "AAXXh")
	PORT_DIPSETTING(0xab, "ABXXh")
	PORT_DIPSETTING(0xac, "ACXXh")
	PORT_DIPSETTING(0xad, "ADXXh")
	PORT_DIPSETTING(0xae, "AEXXh")
	PORT_DIPSETTING(0xaf, "AFXXh")
	PORT_DIPSETTING(0xb0, "B0XXh")
	PORT_DIPSETTING(0xb1, "B1XXh")
	PORT_DIPSETTING(0xb2, "B2XXh")
	PORT_DIPSETTING(0xb3, "B3XXh")
	PORT_DIPSETTING(0xb4, "B4XXh")
	PORT_DIPSETTING(0xb5, "B5XXh")
	PORT_DIPSETTING(0xb6, "B6XXh")
	PORT_DIPSETTING(0xb7, "B7XXh")
	PORT_DIPSETTING(0xb8, "B8XXh")
	PORT_DIPSETTING(0xb9, "B9XXh")
	PORT_DIPSETTING(0xba, "BAXXh")
	PORT_DIPSETTING(0xbb, "BBXXh")
	PORT_DIPSETTING(0xbc, "BCXXh")
	PORT_DIPSETTING(0xbd, "BDXXh")
	PORT_DIPSETTING(0xbe, "BEXXh")
	PORT_DIPSETTING(0xbf, "BFXXh")
	PORT_DIPSETTING(0xc0, "C0XXh")
	PORT_DIPSETTING(0xc1, "C1XXh")
	PORT_DIPSETTING(0xc2, "C2XXh")
	PORT_DIPSETTING(0xc3, "C3XXh")
	PORT_DIPSETTING(0xc4, "C4XXh")
	PORT_DIPSETTING(0xc5, "C5XXh")
	PORT_DIPSETTING(0xc6, "C6XXh")
	PORT_DIPSETTING(0xc7, "C7XXh")
	PORT_DIPSETTING(0xc8, "C8XXh")
	PORT_DIPSETTING(0xc9, "C9XXh")
	PORT_DIPSETTING(0xca, "CAXXh")
	PORT_DIPSETTING(0xcb, "CBXXh")
	PORT_DIPSETTING(0xcc, "CCXXh")
	PORT_DIPSETTING(0xcd, "CDXXh")
	PORT_DIPSETTING(0xce, "CEXXh")
	PORT_DIPSETTING(0xcf, "CFXXh")
	PORT_DIPSETTING(0xd0, "D0XXh")
	PORT_DIPSETTING(0xd1, "D1XXh")
	PORT_DIPSETTING(0xd2, "D2XXh")
	PORT_DIPSETTING(0xd3, "D3XXh")
	PORT_DIPSETTING(0xd4, "D4XXh")
	PORT_DIPSETTING(0xd5, "D5XXh")
	PORT_DIPSETTING(0xd6, "D6XXh")
	PORT_DIPSETTING(0xd7, "D7XXh")
	PORT_DIPSETTING(0xd8, "D8XXh")
	PORT_DIPSETTING(0xd9, "D9XXh")
	PORT_DIPSETTING(0xda, "DAXXh")
	PORT_DIPSETTING(0xdb, "DBXXh")
	PORT_DIPSETTING(0xdc, "DCXXh")
	PORT_DIPSETTING(0xdd, "DDXXh")
	PORT_DIPSETTING(0xde, "DEXXh")
	PORT_DIPSETTING(0xdf, "DFXXh")
	PORT_DIPSETTING(0xe0, "E0XXh")
	PORT_DIPSETTING(0xe1, "E1XXh")
	PORT_DIPSETTING(0xe2, "E2XXh")
	PORT_DIPSETTING(0xe3, "E3XXh")
	PORT_DIPSETTING(0xe4, "E4XXh")
	PORT_DIPSETTING(0xe5, "E5XXh")
	PORT_DIPSETTING(0xe6, "E6XXh")
	PORT_DIPSETTING(0xe7, "E7XXh")
	PORT_DIPSETTING(0xe8, "E8XXh")
	PORT_DIPSETTING(0xe9, "E9XXh")
	PORT_DIPSETTING(0xea, "EAXXh")
	PORT_DIPSETTING(0xeb, "EBXXh")
	PORT_DIPSETTING(0xec, "ECXXh")
	PORT_DIPSETTING(0xed, "EDXXh")
	PORT_DIPSETTING(0xee, "EEXXh")
	PORT_DIPSETTING(0xef, "EFXXh")
	PORT_DIPSETTING(0xf0, "F0XXh")
	PORT_DIPSETTING(0xf1, "F1XXh")
	PORT_DIPSETTING(0xf2, "F2XXh")
	PORT_DIPSETTING(0xf3, "F3XXh")
	PORT_DIPSETTING(0xf4, "F4XXh")
	PORT_DIPSETTING(0xf5, "F5XXh")
	PORT_DIPSETTING(0xf6, "F6XXh")
	PORT_DIPSETTING(0xf7, "F7XXh")
	PORT_DIPSETTING(0xf8, "F8XXh")
	PORT_DIPSETTING(0xf9, "F9XXh")
	PORT_DIPSETTING(0xfa, "FAXXh")
	PORT_DIPSETTING(0xfb, "FBXXh")
	PORT_DIPSETTING(0xfc, "FCXXh")
	PORT_DIPSETTING(0xfd, "FDXXh")
	PORT_DIPSETTING(0xfe, "FEXXh")
	PORT_DIPSETTING(0xff, "FFXXh")

	PORT_START("SEREN")
	PORT_DIPNAME(1, 1, "Enable Serial Port") PORT_DIPLOCATION("SER EN:!1")
	PORT_DIPSETTING(0, DEF_STR(Off))
	PORT_DIPSETTING(1, DEF_STR(On))

	PORT_START("SERADDR")
	PORT_DIPNAME(0xf8, 0x20, "Serial Address Select") PORT_DIPLOCATION("A7-A3:!5,!4,!3,!2,!1")
	PORT_DIPSETTING(0x00, "00h-07h")
	PORT_DIPSETTING(0x08, "08h-0Fh")
	PORT_DIPSETTING(0x10, "10h-17h")
	PORT_DIPSETTING(0x18, "18h-1Fh")
	PORT_DIPSETTING(0x20, "20h-27h")
	PORT_DIPSETTING(0x28, "28h-2Fh")
	PORT_DIPSETTING(0x30, "30h-37h")
	PORT_DIPSETTING(0x38, "38h-3Fh")
	PORT_DIPSETTING(0x40, "40h-47h")
	PORT_DIPSETTING(0x48, "48h-4Fh")
	PORT_DIPSETTING(0x50, "50h-57h")
	PORT_DIPSETTING(0x58, "58h-5Fh")
	PORT_DIPSETTING(0x60, "60h-67h")
	PORT_DIPSETTING(0x68, "68h-6Fh")
	PORT_DIPSETTING(0x70, "70h-77h")
	PORT_DIPSETTING(0x78, "78h-7Fh")
	PORT_DIPSETTING(0x80, "80h-87h")
	PORT_DIPSETTING(0x88, "88h-8Fh")
	PORT_DIPSETTING(0x90, "90h-97h")
	PORT_DIPSETTING(0x98, "98h-9Fh")
	PORT_DIPSETTING(0xa0, "A0h-A7h")
	PORT_DIPSETTING(0xa8, "A8h-AFh")
	PORT_DIPSETTING(0xb0, "B0h-B7h")
	PORT_DIPSETTING(0xb8, "B8h-BFh")
	PORT_DIPSETTING(0xc0, "C0h-C7h")
	PORT_DIPSETTING(0xc8, "C8h-CFh")
	PORT_DIPSETTING(0xd0, "D0h-D7h")
	PORT_DIPSETTING(0xd8, "D8h-DFh")
	PORT_DIPSETTING(0xe0, "E0h-E7h")
	PORT_DIPSETTING(0xe8, "E8h-EFh")
	PORT_DIPSETTING(0xf0, "F0h-F7h")
	PORT_DIPSETTING(0xf8, "F8h-FFh")
INPUT_PORTS_END

static INPUT_PORTS_START( ccs300 )
	// No information available on this system, but it may be assumed that
	//  the 300 is floppy-only, while the 400 boots off a hard drive.
	//  Plugging in the HDC cable would ground this pin to inform the bios
	//  it should be a 400. This "dip" is so you can see (and trace) what
	//  happens.
	PORT_START("MODEL")
	PORT_DIPNAME(0x08, 0x08, "Model")
	PORT_DIPSETTING(0x00, "CCS-400")
	PORT_DIPSETTING(0x08, "CCS-300")
	PORT_BIT(0xf7, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


//*************************************
//
//  Status / Control ports
//
//*************************************

/* Status 1
d0 : intrq
d1 : ds0
d2 : ds1
d3 : ds2
d4 : ds3
d5 : hld (1=head loaded)
d6 : autoboot (1=go to monitor)
d7 : drq
*/

u8 ccs_state::port34_r()
{
	//return (u8)m_drq | (m_ds << 1) | ((u8)fdc->hld_r() << 5) | 0x40 | ((u8)m_intrq << 7);
	return (u8)m_fdc->drq_r() | (m_ds << 1) | 0x20 | 0x40 | ((u8)m_fdc->intrq_r() << 7); // hld_r doesn't do anything
}

/* Status 2
d0 : trk00 (1=trk00 on 20cm drive; 0=trk00 on 13cm drive)
d1 : drive size (0=20cm, 1=13cm)
d2 : wprt from drive (0=write protected)
d3 : /ss (1=side 0 is selected)
d4 : index hole = 0
d5 : dden (1 = double density)
d6 : double (0 = a double-sided 20cm disk is in the drive)
d7 : drq
*/

u8 ccs_state::port04_r()
{
	bool trk00=1,wprt=0,dside=1;
	int idx=1;
	if (m_floppy)
	{
		trk00 = !m_floppy->trk00_r();
		wprt = !m_floppy->wpt_r();
		idx = m_floppy->idx_r()^1;
		dside = m_floppy->twosid_r();
	}
	return (u8)trk00 | 0 | ((u8)wprt << 2) | ((u8)m_ss << 3) |
		idx << 4 | ((u8)m_dden << 5) | ((u8)dside << 6) | ((u8)m_fdc->drq_r() << 7);
}

/* Control 1
d0 : ds0
d1 : ds1
d2 : ds2
d3 : ds3
d4 : drive size (adjusts fdc clock)
d5 : mon (1=motor on)
d6 : dden
d7 : autowait (0=ignore drq)
*/

void ccs_state::port34_w(u8 data)
{
	m_ds = data & 15;
	m_dsize = BIT(data, 4);
	m_dden = BIT(data, 6);

	m_floppy = nullptr;
	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	m_fdc->set_floppy(m_floppy);
	m_fdc->dden_w(!m_dden);

	if (m_floppy)
	{
		m_floppy->mon_w(!BIT(data, 5));
	}
}

/* Control 2
d2 : remote eject for persci drive (1=eject)
d4 : fast seek mode (1=on)
d6 : /ss (0=side 1 selected)
d7 : rom enable (1=firmware enabled)
other bits not used
*/

void ccs_state::port04_w(u8 data)
{
	m_ss = BIT(data, 6);
	if (m_floppy)
		m_floppy->ss_w(!m_ss);
}


//*************************************
//
//  Machine
//
//*************************************
void ccs_state::port40_w(u8 data)
{
	//possibly a banking control, like ccs300 ?
}

void ccs_state::machine_start()
{
	save_item(NAME(m_power_on_status));
	save_item(NAME(m_ss));
	save_item(NAME(m_dden));
	save_item(NAME(m_dsize));
	save_item(NAME(m_ds));
}

void ccs_state::machine_reset()
{
	// pRESET clears 74LS175 wired as shift register
	m_power_on_status = m_jump_en->read() | 8;
}

void ccs300_state::port40_w(u8 data)
{
	m_bank1->set_entry(BIT(~data, 0));
}

void ccs300_state::machine_reset()
{
	m_bank1->set_entry(1);
}

void ccs300_state::machine_start()
{
	m_bank1->configure_entry(0, m_ram1);
	m_bank1->configure_entry(1, m_rom);

	save_item(NAME(m_ss));
	save_item(NAME(m_dden));
	save_item(NAME(m_dsize));
	save_item(NAME(m_ds));
}

//*************************************
//
//  Config
//
//*************************************

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "sio" },
	{ "pio" },
	{ "dma" },
	{ nullptr }
};

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

static void ccs_floppies(device_slot_interface &device)
{
	device.option_add("8sssd", FLOPPY_8_SSSD);
	//device.option_add("8sssd", FLOPPY_525_DD);
}


void ccs_state::ccs2810(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ccs_state::ccs2810_mem);
	m_maincpu->set_addrmap(AS_IO, &ccs_state::ccs2810_io);

	RAM(config, RAM_TAG).set_default_size("64K");

	/* Devices */
	INS8250(config, m_ins8250, 1.8432_MHz_XTAL);
	m_ins8250->out_tx_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_ins8250->out_dtr_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_ins8250->out_rts_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	m_ins8250->out_out1_callback().set("rs232", FUNC(rs232_port_device::write_spds)); // RLSD

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_ins8250, FUNC(ins8250_device::rx_w));
	rs232.rxd_handler().append(m_ins8250, FUNC(ins8250_device::ri_w));
	rs232.dcd_handler().set(m_ins8250, FUNC(ins8250_device::dcd_w));
	rs232.dsr_handler().set(m_ins8250, FUNC(ins8250_device::dsr_w));
	rs232.cts_handler().set(m_ins8250, FUNC(ins8250_device::cts_w));
}

void ccs_state::ccs2422(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ccs_state::ccs2810_mem);
	m_maincpu->set_addrmap(AS_IO, &ccs_state::ccs2422_io);

	RAM(config, RAM_TAG).set_default_size("64K");

	/* Devices */
	INS8250(config, m_ins8250, 1.8432_MHz_XTAL);
	m_ins8250->out_tx_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_ins8250->out_dtr_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_ins8250->out_rts_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	m_ins8250->out_out1_callback().set("rs232", FUNC(rs232_port_device::write_etc)); // RLSD

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_ins8250, FUNC(ins8250_device::rx_w));
	rs232.rxd_handler().append(m_ins8250, FUNC(ins8250_device::ri_w));
	rs232.dcd_handler().set(m_ins8250, FUNC(ins8250_device::dcd_w));
	rs232.dsr_handler().set(m_ins8250, FUNC(ins8250_device::dsr_w));
	rs232.cts_handler().set(m_ins8250, FUNC(ins8250_device::cts_w));

	MB8877(config, m_fdc, 16_MHz_XTAL / 8); // UB1793 or MB8877
	FLOPPY_CONNECTOR(config, "fdc:0", ccs_floppies, "8sssd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

void ccs300_state::ccs300(machine_config & config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &ccs300_state::ccs300_mem);
	m_maincpu->set_addrmap(AS_IO, &ccs300_state::ccs300_io);
	m_maincpu->set_daisy_config(daisy_chain);

	/* Devices */
	z80sio_device &sio(Z80SIO(config, "sio", 16_MHz_XTAL / 4));
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio.out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	sio.out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	sio.out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	rs232a.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
	rs232a.dcd_handler().set("sio", FUNC(z80sio_device::dcda_w));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal)); // must be exactly here

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("sio", FUNC(z80sio_device::rxb_w));
	rs232b.cts_handler().set("sio", FUNC(z80sio_device::ctsb_w));
	rs232b.dcd_handler().set("sio", FUNC(z80sio_device::dcdb_w));

	z80ctc_device &ctc(Z80CTC(config, "ctc", 16_MHz_XTAL / 4));
	ctc.set_clk<0>(16_MHz_XTAL / 8);     // 153'846
	//ctc.set_clk<1>(16_MHz_XTAL / 8);    // not used
	ctc.set_clk<2>(16_MHz_XTAL / 8);      // 9'615
	//ctc.set_clk<3>(16_MHz_XTAL / 8);   // 2'000'000 - this causes an IRQ storm, hanging the machine
	ctc.zc_callback<0>().set("sio", FUNC(z80sio_device::txca_w));
	ctc.zc_callback<0>().append("sio", FUNC(z80sio_device::rxca_w));
	ctc.zc_callback<2>().append("sio", FUNC(z80sio_device::rxtxcb_w));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80pio_device &pio(Z80PIO(config, "pio", 16_MHz_XTAL / 4));
	pio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dma_device &dma(Z80DMA(config, "dma", 16_MHz_XTAL / 4));
	dma.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	MB8877(config, m_fdc, 16_MHz_XTAL / 8); // UB1793 or MB8877
	FLOPPY_CONNECTOR(config, "fdc:0", ccs_floppies, "8sssd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}


/* ROM definition */
ROM_START( ccs2810 )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "ccs2810.u8",   0x0000, 0x0800, CRC(0c3054ea) SHA1(c554b7c44a61af13decb2785f3c9b33c6fc2bfce))

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD_OPTIONAL( "5623.u9", 0x0000, 0x0100, NO_DUMP ) // actual PROM type may differ
ROM_END

ROM_START( ccs2422 )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "2422.u24",  0x0000, 0x0800, CRC(6b47586b) SHA1(73ba779a659da4a1f0e22a3fa351a2b36d8456a0))

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD_OPTIONAL( "2422.u23",  0x0000, 0x0100, CRC(b279cada) SHA1(6cc6e00ec49ba2245c8836d6f09266b09d6e7648))
	ROM_LOAD_OPTIONAL( "2422.u22",  0x0100, 0x0100, CRC(e41858bb) SHA1(0be53725032ebea16e32cb720f099551a357e761))
	ROM_LOAD_OPTIONAL( "2422.u21",  0x0200, 0x0100, NO_DUMP )
ROM_END

ROM_START( ccs300 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "ccs300.rom", 0x0000, 0x0800, CRC(6cf22e31) SHA1(9aa3327cd8c23d0eab82cb6519891aff13ebe1d0))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT   COMPAT  MACHINE   INPUT    CLASS         INIT        COMPANY                        FULLNAME                    FLAGS
COMP( 1980, ccs2810, 0,       0,      ccs2810,  ccs2810, ccs_state,    empty_init, "California Computer Systems", "CCS Model 2810 CPU card",  MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, ccs2422, ccs2810, 0,      ccs2422,  ccs2810, ccs_state,    empty_init, "California Computer Systems", "CCS Model 2422B FDC card", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1981, ccs300,  ccs2810, 0,      ccs300,   ccs300,  ccs300_state, empty_init, "California Computer Systems", "CCS Model 300",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
