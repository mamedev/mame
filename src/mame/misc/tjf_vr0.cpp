// license:BSD-3-Clause
// copyright-holders: Angelo Salese

/****************************************************************************

    Tai Ji Feng (TJF) HW

    driver by Angelo Salese, based off original ddz.cpp by ElSemi

=============================================================================

Scrambling (same scheme on all sets):

The flash address lines A8-A15 are scrambled: A10 is swapped with A13 and A11
with A12, then a per-game set of lines is inverted (A8 always). This applies
to all the flash chips, graphics and samples included.

    set        inverted lines            program        loads at
    kxddz      A8 A9 A14 A15             0x406-0x40000  0x02640000
    crzclass   A8 A9 A10 A14 A15         0x446-0x80000  0x02080000
    baibians   A8 A9 A11 A14             0x446-0x80000  0x02080000
    cjmx       A8 A9 A10 A11 A14 A15     0x446-0x80000  0x02080000
    btlfront   A8 A9 A10 A11 A14 A15     0x3e2-0x20000  0x02710000 (rom5)

Once that is undone, the start of the flash is a boot stub, followed by the
game program. The program is further scrambled in blocks of 27 words: the
first 2 words of each block are in order, the following 25 are reversed (the
stub is not).

The boot stub (vector table, reset at 0x140) sets up the stack and VRender0,
leaves 0x05347000 + program offset in flash at 0x026a0000 and 0x03347000 +
load address at 0x026a0004, then sends ffff ffff 000c through the word
transfers of the I/O device below. It receives one word W and 0x100 more words,
stores them at W << 10 (address kept at 0x026a0008), clears 0x02700000-
0x0270003f and jumps there. So the SE3208 boots from the flash, but the second
stage loader comes from the external device and isn't in the dumps; it
presumably copies and unscrambles the program, which uses the value at
0x026a0004 to restart itself (after initializing the serial memory below).
For now the whole loading is HLEd at reset.

Some code paths (e.g. cjmx at 0x0208b468, crzclass at 0x0208b2b2, taken when
the version word at 0x20 in user zone 0 of the serial memory doesn't match)
jump to the value at 0x026a0004 as is. For crzclass and cjmx that is
0x053c7000, which in the 0x05000000 window is
graphics data (crzclass decompresses stream 3 of its asset table from there),
so the real boards must decode instruction fetches there differently, likely
in the unknown loader. The HLE only replaces what the CPU fetches at that
address while executing there with a jump to the program; data reads still
see the flash.

kxddz reads its graphics from 0x05000000, which holds flash 0x400000 onwards
(rom2/rom3): the asset table copied to 0x02610478 lists packed streams at
0x000000, 0x29e120, 0x3bb8f0, ... from there, and their headers only line up
at that offset.

0x06000000 holds the whole flash from offset 0:
- kxddz reads its samples there: the sample table at 0x0266b104 gives offsets
  from 0x06040000, and each sample then ends exactly where a "LIST" chunk of
  the original WAV file starts;
- baibians copies 0x40000 bytes of graphics from 0x06500000 (flash 0x500000,
  first block after an 0xff filled gap), reads packed streams from 0x068xxxxx
  and builds asset addresses from 0x06080000 (its assets start at flash
  0x80000) and 0x06540000.

btlfront has the asset ROMs on two chip selects instead: rom3 and rom4 at
0x05000000 (memcpy of 0x79580 bytes from 0x05400000 = rom3 0x400000, the start
of a block after an 0xff filled gap; palettes at 0x056e4500 and 0x0589d880 land
on palette starts in rom3 and rom4), rom1 and rom2 at 0x06000000 (0x06400000
and 0x06c60000 are the starts of the second data areas of rom1 and rom2).

crzclass and cjmx also check 0x20071030 at 0x01501f80 (cjmx: 0x020e9dac), along
with the serial memory data, and fall into SYSTEM TEST without it; nothing in
the program writes that value, so it comes from a generated default NVRAM.

0x0154xxxx is an external I/O device (see tjf_io_device below), which also
drives the bus of a serial security memory using the AT88SC153 CryptoMemory
command set (see tjf_cryptomem_device). The baibians PCB has an unmarked SOIC-8
(U1) next to the stickered QFP, which could be it (not verified).

All the games read user zone 2 of the serial memory (command 0xb9) and check
it as 16 dwords against constants of their own, which are in the generated
default contents:
    kxddz        read to 0x02200080, copied to 0x02680ff0, 0x02681ee4, ...
               (table at 0x02669044), checked against the table at
               0x02669084, hangs at 0x026407d0 on mismatch
    crzclass   read to 0x02020080, compared inline from 0x020e80b6
    baibians   read to 0x02020080, compared inline from 0x02084ec6
    cjmx       read to 0x02020080, compared inline from 0x020e9c80
    btlfront   read to 0x02200080, copied to 0x0268000f, 0x0268104e, ...
               and checked in several places (0x0271b546, 0x0271df18, ...);
               the expected values are the ones written by 0x02712628
               and match baibians

btlfront and unaligned accesses: 0x02712628 (constants) and 0x02726fb0 (copy
of user zone 2) store the 16 dwords at deliberately unaligned addresses
(0x0268000f, 0x0268104e, 0x0268208d, 0x026830cc, ...), while the game code
checks them at the aligned ones (e.g. 0x0268000c against 0xc503adb5 at
0x0271df18, 0x0268104c at 0x0271b54c, ...). The SE3208 core splits unaligned
dword accesses into bytes, so most of these checks fail and the game stays on
the stage map once started. Ignoring the low address bits for dword accesses
in the core makes all of them pass and the game gets to the HUD and the
MISSION screen (where it still stops, TODO). That the real CPU ignores the low
bits is deduced from this code, not verified.

=============================================================================

Medal games (xbnz, hzniqiu):

Two kids games with 虹猫蓝兔 (Hongmao Lantu) stickers on a board marked
T071030A (MAH-JONG and JAMMA edges): the main QFP (208 pins, marking sanded
off, programmed as a VRenderZERO+), three 27C322 EPROMs (ROM4 socket empty),
a stickered QFP and the usual SDRAMs. They have the TJF boot stub, I/O device
and program scrambling, with these differences:

- the address lines A8-A15 are reversed (A8 <-> A15, A9 <-> A14, A10 <-> A13,
  A11 <-> A12) before the per-game inversions (raw A12-A15 for xbnz, A10-A15
  for hzniqiu), again on all the EPROMs
- the boot stub copies 0x4ec4 blocks of 27 words from 0x2ce to 0x02080000,
  unscrambling them, and jumps there, so there is no loader to HLE
- the I/O device uses other magic words and has an extra memory (see
  medal_io_device below)
- a FameG FS8806 is connected in SPI mode to PIO 12 (SCK), 13 (MOSI), 14 (CS)
  and 15 (MISO), all in open collector mode. The program takes its DES key
  from the I/O device memory at 0x80 and reads 8 bytes at 0x20 of the FS8806
  EEPROM, whose first dword has to match the one at 0x98 of the I/O device
  memory. xbnz also checks the 16 dwords at 0x00 of the I/O device memory
  against constants of its own and, when anything fails, enables the watchdog
  and waits for it (0x02084768).
  The FS8806 EEPROM and the key and 0x80 areas of the I/O device memory are
  erased in the default contents, which satisfies the checks; the real
  contents are unknown.


    TODO:
    - Dump the second stage loader the external device sends to the boot stub
      (command 000c), the program loading is currently HLEd
    - btlfront doesn't proceed once a game starts (the stage map is shown, nothing moves),
      see the note about unaligned accesses above
    - btlfront resets its settings to the defaults at every boot (same cause, the key check at
      0x0271df18 fails), see the note about unaligned accesses above
    - Key matrix labels (MAH-JONG connector), set as MIC for now
    - 0x01548000 (read once at boot by btlfront and baibians, used as a seed)
    - kxddz's sound is too fast. The game sets the PLL at roughly 55 MHz, but it's currently
      bypassed by the HLE
    - Serial security memory: password, access rights, configuration zone
    - xbnz, hzniqiu: unknown inputs, I/O device registers 0x05, 0x22 and 0x23, FS8806 EEPROM
      and key

****************************************************************************/

#include "emu.h"

#include "cpu/se3208/se3208.h"
#include "machine/fs8806.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/vrender0.h"
#include "sound/vrender0.h"
#include "video/vrender0.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "endianness.h"

#include <algorithm>
#include <vector>


namespace {

/***************************************************************************

    Serial security memory (AT88SC153 compatible)

    The games bit-bang an I2C-like bus through the I/O device below and use
    the command set of the Atmel AT88SC153 CryptoMemory: chip select 1011 in
    the upper nibble, then
        1011 zz00   write zone zz ([address] [data...])
        1011 zz01   read zone zz ([address], then the device sends data)
        1011 rp11   verify password (3 bytes)
    with three 64-byte user zones and a 64-byte configuration zone (zz = 3).
    The password, the access rights and the authentication commands are not
    emulated: every zone can be read and written.

***************************************************************************/

class tjf_cryptomem_device : public device_t, public device_nvram_interface
{
public:
	tjf_cryptomem_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void scl_w(int state);
	void sda_w(int state);
	int sda_r();

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	enum : u8 { STATE_IDLE, STATE_RX, STATE_TX };

	void byte_received(u8 data);
	u8 &cell() { return m_mem[((m_cmd & 0x0c) << 4) | (m_addr & 0x3f)]; }

	optional_region_ptr<u8> m_default;

	u8 m_mem[0x100];
	u8 m_scl;
	u8 m_sda;
	u8 m_state;
	u8 m_bit;
	u8 m_phase;
	u8 m_shift;
	u8 m_cmd;
	u8 m_addr;
	u8 m_tx;
	u32 m_count;
	bool m_ack;
	bool m_txpend;
};

DEFINE_DEVICE_TYPE_PRIVATE(TJF_CRYPTOMEM, tjf_cryptomem_device, tjf_cryptomem_device, "tjf_cryptomem", "TJF serial security memory (AT88SC153 compatible)")

tjf_cryptomem_device::tjf_cryptomem_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, TJF_CRYPTOMEM, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_default(*this, DEVICE_SELF),
	m_scl(0),
	m_sda(1)
{
}

void tjf_cryptomem_device::device_start()
{
	save_item(NAME(m_mem));
	save_item(NAME(m_scl));
	save_item(NAME(m_sda));
	save_item(NAME(m_state));
	save_item(NAME(m_bit));
	save_item(NAME(m_phase));
	save_item(NAME(m_shift));
	save_item(NAME(m_cmd));
	save_item(NAME(m_addr));
	save_item(NAME(m_tx));
	save_item(NAME(m_count));
	save_item(NAME(m_ack));
	save_item(NAME(m_txpend));
}

void tjf_cryptomem_device::device_reset()
{
	m_state = STATE_IDLE;
	m_bit = 0;
	m_phase = 0;
	m_shift = 0;
	m_cmd = 0;
	m_addr = 0;
	m_tx = 0;
	m_count = 0;
	m_ack = false;
	m_txpend = false;
}

void tjf_cryptomem_device::nvram_default()
{
	if (m_default.found() && (m_default.bytes() == sizeof(m_mem)))
		std::copy_n(&m_default[0], sizeof(m_mem), m_mem);
	else
		std::fill(std::begin(m_mem), std::end(m_mem), 0xff);
}

bool tjf_cryptomem_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = util::read(file, m_mem, sizeof(m_mem));
	return !err && (actual == sizeof(m_mem));
}

bool tjf_cryptomem_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, m_mem, sizeof(m_mem));
	return !err;
}

void tjf_cryptomem_device::byte_received(u8 data)
{
	if (!m_count && ((data & 0xf0) != 0xb0))
	{
		// not for us, ignore the rest of the transaction
		logerror("command %02x for another device\n", data);
		m_state = STATE_IDLE;
		return;
	}

	m_ack = true;
	switch (m_count++)
	{
		case 0:
			m_cmd = data;
			if ((m_cmd & 0x03) == 0x02)
				logerror("unsupported command %02x\n", m_cmd);
			break;

		case 1:
			m_addr = data;
			if ((m_cmd & 0x03) == 0x01)
				m_txpend = true; // read zone
			break;

		default:
			if ((m_cmd & 0x03) == 0x00)
			{
				// write zone, the address wraps around in 8-byte pages
				cell() = data;
				m_addr = (m_addr & ~0x07) | ((m_addr + 1) & 0x07);
			}
			break;
	}
}

void tjf_cryptomem_device::scl_w(int state)
{
	if (state && !m_scl && (m_state != STATE_IDLE))
	{
		m_phase = m_bit;
		m_bit = (m_bit == 8) ? 0 : (m_bit + 1);
		if (m_phase == 0)
			m_ack = false;

		if (m_state == STATE_RX)
		{
			if (m_phase < 8)
			{
				m_shift = (m_shift << 1) | m_sda;
				if (m_phase == 7)
					byte_received(m_shift);
			}
			else if (m_txpend)
			{
				m_txpend = false;
				m_state = STATE_TX;
				m_tx = cell();
			}
		}
		else if (m_phase == 8)
		{
			// acknowledged by the host, next byte (the address wraps around in the zone)
			m_addr = (m_addr + 1) & 0x3f;
			m_tx = cell();
		}
	}
	m_scl = state;
}

void tjf_cryptomem_device::sda_w(int state)
{
	if (m_scl && (state != m_sda))
	{
		if (!state)
		{
			// start
			m_state = STATE_RX;
			m_bit = 0;
			m_count = 0;
			m_ack = false;
			m_txpend = false;
		}
		else if (m_state != STATE_IDLE)
		{
			// stop
			if (m_count)
				logerror("command %02x address %02x, %u bytes\n", m_cmd, m_addr, m_count);
			m_state = STATE_IDLE;
		}
	}
	m_sda = state;
}

int tjf_cryptomem_device::sda_r()
{
	if ((m_phase == 8) && m_ack)
		return 0;

	if ((m_state == STATE_TX) && (m_phase < 8))
		return BIT(m_tx, 7 - m_phase);

	return 1;
}


/***************************************************************************

    I/O device at 0x0154xxxx (on the low byte of each dword, one register
    every 0x100 bytes; the offsets below are in bytes / 0x100)

    w 0x00   input matrix row select
    w 0x01   bit 0: hopper motor (TJF card games)
    w 0x02   input shift registers clock (kxddz, btlfront)
    w 0x03   input shift registers load (kxddz, btlfront)
    w 0x04   1 = start transfer, 0 = end transfer
    w 0x10   16-bit word to send, high byte
    w 0x11   16-bit word to send, low byte
    w 0x12   direction, 0 = send, 1 = receive
    w 0x14   serial memory bus, 1 while the CPU reads SDA (SDA released)
    w 0x15   serial memory bus, SDA out
    w 0x16   serial memory bus, SCL
    w 0x17   serial memory bus, unknown (RST?)
    r 0x95   inputs 1, active low (kxddz, btlfront: serial on bit 0)
    r 0x96   inputs 2, active low (kxddz, btlfront: serial on bit 0)
    r 0x97   inputs 3, active low (kxddz, btlfront: serial on bit 0)
    r 0xa0   16-bit word received, low byte
    r 0xa1   16-bit word received, high byte
    r 0xa3   status, low nibble 4 = transfer done
    r 0xa5   bit 0: language (btlfront: 0 = Chinese, 1 = English)
    r 0xa7   serial memory bus, SDA in (bit 0)

    The word transfers access a block storage: ffff ffff 000b 7777 <addr>
    followed by 0xff words writes a block, ffff ffff 000a 6666 <addr> then
    0xff received words reads one. The boot stub uses ffff ffff 000c to get
    the second stage loader (one word, then 0x100 words), not emulated since
    the driver skips the stub. All the games read the blocks at 0x2000
    and 0x2200 at boot; btlfront keeps its settings and high scores in the
    one at 0x2000. The TJF card games compare the first 16 bytes of the one
    at 0x2200 with user zone 1 of the serial memory. What holds the blocks
    (maybe the 44-pin QFP on the baibians PCB), its size and whether <addr>
    is in bytes are unknown; it is emulated as 64KB of byte addresses.

***************************************************************************/

class tjf_io_device : public device_t, public device_nvram_interface
{
public:
	tjf_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// the offset passed to the input callbacks is the selected key matrix row
	template <unsigned N> auto in_callback() { return m_in_cb[N].bind(); }
	auto dsw_callback() { return m_dsw_cb.bind(); }
	auto hopper_callback() { return m_hopper_cb.bind(); }
	auto scl_callback() { return m_scl_cb.bind(); }
	auto sda_callback() { return m_sda_cb.bind(); }
	auto sda_in_callback() { return m_sda_in_cb.bind(); }

	virtual u8 read(offs_t offset);
	virtual void write(offs_t offset, u8 data);

protected:
	tjf_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock,
			u16 read_cmd, u16 read_magic, u16 write_cmd, u16 write_magic);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	enum : u8 { XFER_IDLE, XFER_READ_MAGIC, XFER_READ_ADDR, XFER_READ, XFER_WRITE_MAGIC, XFER_WRITE_ADDR, XFER_WRITE, XFER_DERIVED };

	virtual void word_sent(u8 hi, u8 lo);
	virtual void word_received();
	virtual bool command(u16 word) { return false; }

	u8 m_rx_hi;
	u8 m_rx_lo;
	u8 m_xfer_state;
	u16 m_xfer_count;

private:
	u16 const m_read_cmd;
	u16 const m_read_magic;
	u16 const m_write_cmd;
	u16 const m_write_magic;

	devcb_read8::array<3> m_in_cb;
	devcb_read8 m_dsw_cb;
	devcb_write_line m_hopper_cb;
	devcb_write_line m_scl_cb;
	devcb_write_line m_sda_cb;
	devcb_read_line m_sda_in_cb;

	u8 m_row;
	u8 m_in_shift;
	u8 m_xfer_hi;
	u8 m_xfer_lo;
	u8 m_xfer_dir;
	u8 m_xfer_sync;
	u16 m_xfer_addr;
	u8 m_sda_release;
	u8 m_store[0x10000];
};

DEFINE_DEVICE_TYPE_PRIVATE(TJF_IO, tjf_io_device, tjf_io_device, "tjf_io", "TJF I/O device")

tjf_io_device::tjf_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tjf_io_device(mconfig, TJF_IO, tag, owner, clock, 0x000a, 0x6666, 0x000b, 0x7777)
{
}

tjf_io_device::tjf_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock,
		u16 read_cmd, u16 read_magic, u16 write_cmd, u16 write_magic) :
	device_t(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_read_cmd(read_cmd),
	m_read_magic(read_magic),
	m_write_cmd(write_cmd),
	m_write_magic(write_magic),
	m_in_cb(*this, 0xff),
	m_dsw_cb(*this, 0xff),
	m_hopper_cb(*this),
	m_scl_cb(*this),
	m_sda_cb(*this),
	m_sda_in_cb(*this, 1),
	m_row(0xff),
	m_sda_release(0)
{
}

void tjf_io_device::device_start()
{
	save_item(NAME(m_row));
	save_item(NAME(m_in_shift));
	save_item(NAME(m_xfer_hi));
	save_item(NAME(m_xfer_lo));
	save_item(NAME(m_xfer_dir));
	save_item(NAME(m_rx_hi));
	save_item(NAME(m_rx_lo));
	save_item(NAME(m_xfer_state));
	save_item(NAME(m_xfer_sync));
	save_item(NAME(m_xfer_addr));
	save_item(NAME(m_xfer_count));
	save_item(NAME(m_sda_release));
	save_item(NAME(m_store));
}

void tjf_io_device::device_reset()
{
	m_in_shift = 0;
	m_xfer_hi = 0;
	m_xfer_lo = 0;
	m_xfer_dir = 0;
	m_rx_hi = 0xff;
	m_rx_lo = 0xff;
	m_xfer_state = XFER_IDLE;
	m_xfer_sync = 0;
	m_xfer_addr = 0;
	m_xfer_count = 0;
	m_hopper_cb(0);
}

void tjf_io_device::nvram_default()
{
	std::fill(std::begin(m_store), std::end(m_store), 0xff);
}

bool tjf_io_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = util::read(file, m_store, sizeof(m_store));
	return !err && (actual == sizeof(m_store));
}

bool tjf_io_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, m_store, sizeof(m_store));
	return !err;
}

void tjf_io_device::word_sent(u8 hi, u8 lo)
{
	u16 const word = (hi << 8) | lo;

	switch (m_xfer_state)
	{
		case XFER_WRITE:
			// data words go out in memory order, the first byte in the high register
			m_store[m_xfer_addr] = hi;
			m_store[u16(m_xfer_addr + 1)] = lo;
			m_xfer_addr += 2;
			if (++m_xfer_count == 0xff)
				m_xfer_state = XFER_IDLE;
			return;

		case XFER_READ_MAGIC:
			m_xfer_state = (word == m_read_magic) ? XFER_READ_ADDR : XFER_IDLE;
			return;

		case XFER_WRITE_MAGIC:
			m_xfer_state = (word == m_write_magic) ? XFER_WRITE_ADDR : XFER_IDLE;
			return;

		case XFER_READ_ADDR:
		case XFER_WRITE_ADDR:
			logerror("%s block %04x\n", (m_xfer_state == XFER_READ_ADDR) ? "read" : "write", word);
			m_xfer_addr = word;
			m_xfer_count = 0;
			m_xfer_state = (m_xfer_state == XFER_READ_ADDR) ? XFER_READ : XFER_WRITE;
			return;
	}

	// commands follow two 0xffff words
	m_xfer_state = XFER_IDLE;
	if (word == 0xffff)
	{
		if (m_xfer_sync < 2)
			m_xfer_sync++;
		return;
	}
	if (m_xfer_sync == 2)
	{
		if (word == m_read_cmd)
			m_xfer_state = XFER_READ_MAGIC;
		else if (word == m_write_cmd)
			m_xfer_state = XFER_WRITE_MAGIC;
		else if (!command(word))
			logerror("unknown command %04x\n", word);
	}
	else
	{
		logerror("unexpected word %04x\n", word);
	}
	m_xfer_sync = 0;
}

void tjf_io_device::word_received()
{
	if (m_xfer_state == XFER_READ)
	{
		m_rx_lo = m_store[m_xfer_addr];
		m_rx_hi = m_store[u16(m_xfer_addr + 1)];
		m_xfer_addr += 2;
		if (++m_xfer_count == 0xff)
			m_xfer_state = XFER_IDLE;
	}
	else
	{
		logerror("%s: unexpected receive\n", machine().describe_context());
		m_rx_lo = m_rx_hi = 0xff;
	}
}

u8 tjf_io_device::read(offs_t offset)
{
	switch (offset << 2)
	{
		case 0x9500:
		case 0x9600:
		case 0x9700:
			// kxddz and btlfront clock the inputs out serially on bit 0 after a load, the TJF card games read
			// them as bytes and never load or clock
			return (m_in_cb[((offset << 2) - 0x9500) >> 8](m_row) | 0xff00) >> m_in_shift;

		case 0xa000: // received word, low byte (first byte in memory order)
			return m_rx_lo;

		case 0xa100: // received word, high byte
			return m_rx_hi;

		case 0xa500:
			return m_dsw_cb();

		case 0xa300: // status
			return 0x04; // HLE: transfers always complete

		case 0xa700:
			return m_sda_in_cb();
	}

	if (!machine().side_effects_disabled())
		logerror("%s: unknown read %04x\n", machine().describe_context(), offset << 2);
	return 0xff;
}

void tjf_io_device::write(offs_t offset, u8 data)
{
	switch (offset << 2)
	{
		case 0x0000:
			m_row = data;
			break;

		case 0x0100:
			m_hopper_cb(BIT(data, 0));
			break;

		case 0x0200:
			if (m_in_shift < 8)
				m_in_shift++;
			break;

		case 0x0300:
			m_in_shift = 0;
			break;

		case 0x0400:
			if (BIT(data, 0))
			{
				if (!m_xfer_dir)
					word_sent(m_xfer_hi, m_xfer_lo);
				else
					word_received();
			}
			break;

		case 0x1000: m_xfer_hi = data; break;
		case 0x1100: m_xfer_lo = data; break;
		case 0x1200: m_xfer_dir = BIT(data, 0); break;

		case 0x1400:
			// the CPU releases SDA while it reads it, and always drives it again before changing it
			m_sda_release = BIT(data, 0);
			break;

		case 0x1500:
			if (!m_sda_release)
				m_sda_cb(BIT(data, 0));
			break;

		case 0x1600:
			m_scl_cb(BIT(data, 0));
			break;

		case 0x1700:
			break;

		default:
			logerror("%s: unknown write %04x = %02x\n", machine().describe_context(), offset << 2, data);
			break;
	}
}


/***************************************************************************

    I/O device of the medal games (xbnz, hzniqiu)

    Same registers and word transfers as the TJF one, with these differences
    (from the code of both games):
    - block storage: ffff ffff 000a 3728 <addr> followed by 0xff words writes
      a block, ffff ffff 000b 7514 <addr> then 0xff received words reads one
      (both games read the blocks at 0x2800 and 0x2e00 at boot)
    - ffff ffff 000d <addr> then 0x20 received words reads 0x40 bytes of a
      separate memory, ffff ffff 000e <addr> followed by 0x20 words writes
      them. The games read 0x00, 0x40 and 0x80; xbnz checks the 16 dwords at
      0x00 against constants of its own, which it then uses as variables, and
      takes the FS8806 key from the start of 0x80. The size of this memory is
      unknown, it is emulated as 0x100 bytes
    - 0x05 is written with 0 before every word except the block data
      (purpose unknown)
    - 0x20-0x23 are pulsed while pending counts are decremented (0x02100dc2
      in xbnz): 0x20 for the coins, 0x21 for the tickets, the other two
      unknown. Emulated as coin counters

***************************************************************************/

class medal_io_device : public tjf_io_device
{
public:
	medal_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual void write(offs_t offset, u8 data) override;

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

	virtual void word_sent(u8 hi, u8 lo) override;
	virtual void word_received() override;
	virtual bool command(u16 word) override;

private:
	enum : u8 { XFER_ZONE_READ_ADDR = XFER_DERIVED, XFER_ZONE_READ, XFER_ZONE_WRITE_ADDR, XFER_ZONE_WRITE };

	optional_region_ptr<u8> m_default;

	u8 m_zone_addr;
	u8 m_zone[0x100];
};

DEFINE_DEVICE_TYPE_PRIVATE(MEDAL_IO, medal_io_device, medal_io_device, "medal_io", "Medal I/O device")

medal_io_device::medal_io_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	tjf_io_device(mconfig, MEDAL_IO, tag, owner, clock, 0x000b, 0x7514, 0x000a, 0x3728),
	m_default(*this, DEVICE_SELF),
	m_zone_addr(0)
{
}

void medal_io_device::device_start()
{
	tjf_io_device::device_start();

	save_item(NAME(m_zone_addr));
	save_item(NAME(m_zone));
}

void medal_io_device::nvram_default()
{
	tjf_io_device::nvram_default();

	if (m_default && (m_default.bytes() == sizeof(m_zone)))
		std::copy_n(&m_default[0], sizeof(m_zone), m_zone);
	else
		std::fill(std::begin(m_zone), std::end(m_zone), 0xff);
}

bool medal_io_device::nvram_read(util::read_stream &file)
{
	if (!tjf_io_device::nvram_read(file))
		return false;

	auto const [err, actual] = util::read(file, m_zone, sizeof(m_zone));
	return !err && (actual == sizeof(m_zone));
}

bool medal_io_device::nvram_write(util::write_stream &file)
{
	if (!tjf_io_device::nvram_write(file))
		return false;

	auto const [err, actual] = util::write(file, m_zone, sizeof(m_zone));
	return !err;
}

bool medal_io_device::command(u16 word)
{
	switch (word)
	{
		case 0x000d:
			m_xfer_state = XFER_ZONE_READ_ADDR;
			return true;

		case 0x000e:
			m_xfer_state = XFER_ZONE_WRITE_ADDR;
			return true;
	}
	return false;
}

void medal_io_device::word_sent(u8 hi, u8 lo)
{
	switch (m_xfer_state)
	{
		case XFER_ZONE_READ_ADDR:
		case XFER_ZONE_WRITE_ADDR:
			m_zone_addr = lo;
			m_xfer_count = 0;
			m_xfer_state = (m_xfer_state == XFER_ZONE_READ_ADDR) ? XFER_ZONE_READ : XFER_ZONE_WRITE;
			return;

		case XFER_ZONE_WRITE:
			// same byte order as the block storage
			m_zone[m_zone_addr++] = hi;
			m_zone[m_zone_addr++] = lo;
			if (++m_xfer_count == 0x20)
				m_xfer_state = XFER_IDLE;
			return;
	}

	tjf_io_device::word_sent(hi, lo);
}

void medal_io_device::word_received()
{
	if (m_xfer_state == XFER_ZONE_READ)
	{
		m_rx_lo = m_zone[m_zone_addr++];
		m_rx_hi = m_zone[m_zone_addr++];
		if (++m_xfer_count == 0x20)
			m_xfer_state = XFER_IDLE;
	}
	else
	{
		tjf_io_device::word_received();
	}
}

void medal_io_device::write(offs_t offset, u8 data)
{
	switch (offset << 2)
	{
		case 0x0500:
			break;

		case 0x2000:
		case 0x2100:
		case 0x2200:
		case 0x2300:
			machine().bookkeeping().coin_counter_w(((offset << 2) - 0x2000) >> 8, BIT(data, 0));
			break;

		default:
			tjf_io_device::write(offset, data);
			break;
	}
}


class tjf_state : public driver_device
{
public:
	tjf_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_encdata(*this, "enc_data"),
		m_maincpu(*this, "maincpu"),
		m_io(*this, "io"),
		m_workram(*this, "workram"),
		m_players(*this, "P%u", 1U),
		m_matrix(*this, "KEY%u", 0U),
		m_vr0soc(*this, "vr0soc"),
		m_cryptomem(*this, "cryptomem"),
		m_hopper(*this, "hopper")
	{ }


	void init_kxddz() ATTR_COLD;
	void init_crzclass() ATTR_COLD;
	void init_baibians() ATTR_COLD;
	void init_cjmx() ATTR_COLD;
	void init_btlfront() ATTR_COLD;

	void kxddz(machine_config &config) ATTR_COLD;
	void tjf(machine_config &config) ATTR_COLD;
	void btlfront(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_region_ptr<u8> m_encdata;
	required_device<se3208_device> m_maincpu;
	required_device<tjf_io_device> m_io;

	void tjf_common(machine_config &config) ATTR_COLD;

	void main_map(address_map &map) ATTR_COLD;

	template <unsigned Player> u8 player_r(offs_t offset);

private:
	required_shared_ptr<u32> m_workram;
	required_ioport_array<2> m_players;
	required_ioport_array<5> m_matrix;

	required_device<vrender0soc_device> m_vr0soc;
	required_device<tjf_cryptomem_device> m_cryptomem;
	required_device<hopper_device> m_hopper;

	u32 m_prog_start = 0;
	u32 m_prog_end = 0;
	u32 m_prog_base = 0;
	u32 m_hle_restart[2] = { };
	memory_passthrough_handler m_restart_tap;

	static void descramble_flash(u8 *dst, u8 const *src, u32 len, u16 addrxor) ATTR_COLD;
	static void unscramble_program(u8 *buf, u32 start, u32 end) ATTR_COLD;
	void decrypt(u16 addrxor, u32 prog_start, u32 prog_end, u32 prog_base) ATTR_COLD;

	void btlfront_map(address_map &map) ATTR_COLD;
};

class tjf_medal_state : public tjf_state
{
public:
	tjf_medal_state(const machine_config &mconfig, device_type type, const char *tag) :
		tjf_state(mconfig, type, tag),
		m_fs8806(*this, "fs8806")
	{ }

	void medal(machine_config &config) ATTR_COLD;

	void init_xbnz() ATTR_COLD;
	void init_hzniqiu() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<fs8806_device> m_fs8806;

	u32 m_piomod = 0;
	u32 m_pioldat = 0;

	void descramble(u16 addrxor) ATTR_COLD;

	u32 pio_lines() const;
	void pio_update();
	u32 piomod_r() { return m_piomod; }
	void piomod_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pioldat_r() { return m_pioldat; }
	void pioldat_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pioedat_r();

	void medal_map(address_map &map) ATTR_COLD;
};


template <unsigned Player>
u8 tjf_state::player_r(offs_t offset)
{
	u8 data = m_players[Player]->read();
	if (offset < 5)
	{
		u8 const keys = m_matrix[offset]->read() >> (Player * 3);
		data &= ~0x0b | (keys & 0x03) | ((keys & 0x04) << 1);
	}
	return data;
}

void tjf_state::machine_start()
{
	if (m_prog_end)
	{
		// HLE: LERI / LERI / LDI m_prog_base,%R7 / JR %R7 where the program jumps to restart through the loader
		u32 const er = m_prog_base >> 4;
		m_hle_restart[0] = (0x4000 | ((er >> 14) & 0x3fff)) | ((0x4000 | (er & 0x3fff)) << 16);
		m_hle_restart[1] = (0xef00 | (m_prog_base & 0x0f)) | (0xe087 << 16);

		// only for instruction fetches: crzclass reads graphics data from the same address
		u32 const restart = 0x03347000 + m_prog_base;
		m_restart_tap = m_maincpu->space(AS_PROGRAM).install_read_tap(
				restart, restart + 7, "hle_restart",
				[this, restart] (offs_t offset, u32 &data, u32)
				{
					if ((m_maincpu->pc() & ~3) == (offset & ~3))
						data = m_hle_restart[(offset - restart) >> 2];
				},
				&m_restart_tap);
	}
}

void tjf_state::machine_reset()
{
	if (m_prog_end)
	{
		// HLE: the loader of the real boards is unknown, copy the program to RAM and jump to it
		u8 const *const ipl = memregion("ipl")->base();
		u8 *const ram = reinterpret_cast<u8 *>(&m_workram[0]);
		for (u32 i = m_prog_start; i < m_prog_end; i++)
			ram[BYTE4_XOR_LE(m_prog_base - 0x02000000 + i - m_prog_start)] = ipl[BYTE4_XOR_LE(i)];

		// what the boot stub leaves for the loader, the program reads the load address back to restart itself
		m_workram[0x6a0000 / 4] = 0x05347000 + m_prog_start;
		m_workram[0x6a0004 / 4] = 0x03347000 + m_prog_base;

		m_maincpu->set_state_int(SE3208_PC, m_prog_base);
	}
}

void tjf_medal_state::machine_start()
{
	tjf_state::machine_start();

	save_item(NAME(m_piomod));
	save_item(NAME(m_pioldat));
}

// a pin in open collector mode (PIOMOD bit set) is pulled low by a 1 in PIOLDAT and released by a 0
u32 tjf_medal_state::pio_lines() const
{
	return (m_pioldat & ~m_piomod) | (~m_pioldat & m_piomod);
}

void tjf_medal_state::pio_update()
{
	u32 const lines = pio_lines();
	m_fs8806->spi_mosi_w(BIT(lines, 13));
	m_fs8806->spi_cs_w(BIT(lines, 14));
	m_fs8806->spi_sck_w(BIT(lines, 12));
}

void tjf_medal_state::piomod_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_piomod);
	pio_update();
}

void tjf_medal_state::pioldat_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pioldat);
	pio_update();
}

u32 tjf_medal_state::pioedat_r()
{
	u32 data = pio_lines();
	if (!m_fs8806->spi_miso_r())
		data &= ~(u32(1) << 15);
	return data;
}


void tjf_state::main_map(address_map &map)
{
	map.unmap_value_high();

	map(0x00000000, 0x00ffffff).rom().nopw().region("ipl", 0);

	map(0x01500000, 0x01501fff).ram().share("nvram");

	map(0x01540000, 0x0154ffff).rw(m_io, FUNC(tjf_io_device::read), FUNC(tjf_io_device::write)).umask32(0x000000ff);

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));

	map(0x02000000, 0x027fffff).ram().share(m_workram);

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));

	map(0x05000000, 0x057fffff).rom().region("ipl", 0x400000); // TODO: verify size and mirroring
	map(0x06000000, 0x06ffffff).rom().region("ipl", 0); // TODO: verify mirroring
}

void tjf_state::btlfront_map(address_map &map)
{
	main_map(map);

	map(0x05000000, 0x05ffffff).rom().region("assets", 0);         // rom3, rom4
	map(0x06000000, 0x06ffffff).rom().region("assets", 0x1000000); // rom1, rom2
}

void tjf_medal_state::medal_map(address_map &map)
{
	main_map(map);

	map(0x01802000, 0x01802003).rw(FUNC(tjf_medal_state::piomod_r), FUNC(tjf_medal_state::piomod_w));
	map(0x01802004, 0x01802007).rw(FUNC(tjf_medal_state::pioldat_r), FUNC(tjf_medal_state::pioldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(tjf_medal_state::pioedat_r));
}


// The programs share their input code: each player port is copied to an array in the order
// bit 0, 1, 3, 2, (gap), 4, 5, 6, 7 (btlfront: bit 4, 5, 6, 7, 0, 1, 3, 2), and the system bits go to
// variables laid out the same way in every set. The labels come from the key test of the TJF card
// games (按键测试, in the 测试 menu) and were checked by playing btlfront; ddz uses them by analogy.
#define DDZ_PLAYER(player) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(player) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(player) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(player) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(player) \
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(player) \
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(player) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(player)

// All the games scan rows 0-4 of a key matrix through 0x01540000, reading bits 0, 1 and 3 of the two
// player ports (the PCBs have a separate MAH-JONG connector). Bits 0-2 of each row go to bits 0, 1 and 3
// of player 1, bits 3-5 to the same bits of player 2. TODO: which keys these are.
#define DDZ_MATRIX \
	PORT_START("KEY0") \
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_START("KEY1") \
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_START("KEY2") \
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_START("KEY3") \
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_START("KEY4") \
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNKNOWN )

static INPUT_PORTS_START( tjf )
	PORT_START("P1")
	DDZ_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // 退币

	PORT_START("P2")
	DDZ_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) // 洗分

	PORT_START("SYS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )         // 1P 开始
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )         // 2P 开始
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // HPSW.
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )        // not shown in the key test
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )          // 投币
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   // 开分
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )       // 测试
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Settings") // 设定, held at power on clears the NVRAM in crzclass

	DDZ_MATRIX

	PORT_START("DSW") // jumpers or DIP switches, location unknown
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Chinese ) )
	PORT_DIPSETTING(    0x01, DEF_STR( English ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( btlfront )
	PORT_INCLUDE( tjf )

	PORT_MODIFY("P1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( medal )
	PORT_START("P1") // 0x01549500
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // ticket sensor
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // also moves the cursor there
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )  // held: JAMMED MEDAL SELECTOR
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2") // 0x01549700
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // also confirms in the bookkeeping
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYS") // 0x01549600
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	DDZ_MATRIX

	PORT_START("DSW") // read once at boot
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


void tjf_state::tjf_common(machine_config &config)
{
	SE3208(config, m_maincpu, 14'318'180 * 3); // TODO : dynamic via PLL
	m_maincpu->set_addrmap(AS_PROGRAM, &tjf_state::main_map);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	m_io->in_callback<0>().set(FUNC(tjf_state::player_r<0>));
	m_io->in_callback<1>().set(FUNC(tjf_state::player_r<1>));
	m_io->in_callback<2>().set_ioport("SYS");
	m_io->dsw_callback().set_ioport("DSW");
	m_io->hopper_callback().set(m_hopper, FUNC(hopper_device::motor_w));
	m_io->scl_callback().set(m_cryptomem, FUNC(tjf_cryptomem_device::scl_w));
	m_io->sda_callback().set(m_cryptomem, FUNC(tjf_cryptomem_device::sda_w));
	m_io->sda_in_callback().set(m_cryptomem, FUNC(tjf_cryptomem_device::sda_r));

	TJF_CRYPTOMEM(config, m_cryptomem);

	HOPPER(config, m_hopper, attotime::from_msec(50));

	VRENDER0_SOC(config, m_vr0soc, 14'318'180 * 6); // TODO : dynamic via PLL
	m_vr0soc->set_host_space_tag(m_maincpu, AS_PROGRAM);
	m_vr0soc->int_callback().set_inputline(m_maincpu, se3208_device::SE3208_INT);

	SPEAKER(config, "speaker", 2).front();
	m_vr0soc->add_route(0, "speaker", 1.0, 0);
	m_vr0soc->add_route(1, "speaker", 1.0, 1);
}

void tjf_state::kxddz(machine_config &config)
{
	TJF_IO(config, m_io);

	tjf_common(config);
}

void tjf_state::tjf(machine_config &config)
{
	kxddz(config);

	m_io->in_callback<1>().set_ioport("SYS");
	m_io->in_callback<2>().set(FUNC(tjf_state::player_r<1>));
}

void tjf_state::btlfront(machine_config &config)
{
	kxddz(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &tjf_state::btlfront_map);
}

void tjf_medal_state::medal(machine_config &config)
{
	MEDAL_IO(config, m_io);

	tjf_common(config);

	m_io->in_callback<1>().set_ioport("SYS");
	m_io->in_callback<2>().set(FUNC(tjf_medal_state::player_r<1>));

	m_maincpu->set_addrmap(AS_PROGRAM, &tjf_medal_state::medal_map);

	// the program takes the key from 0x80 of the I/O device memory, erased in the default contents
	FS8806(config, m_fs8806).set_key(0xffffffff'ffffffffU);
}


/*
    Generated files, not dumps:
    - at88sc153.bin: contents of the serial security memory. Only user zone 2 (0x80-0xbf) is known, as
      the 16 dwords the program checks. Zone 0 is left erased (the programs write their version word
      there), zone 1 too (the TJF card games compare its start with the block storage, also erased by
      default), the configuration zone (0xc0-0xff) is unknown.
    - zones.bin: memory of the xbnz I/O device. Only the 16 dwords at 0x00 are known, as the constants
      the program checks them against; the rest is left erased.
    - nvram.bin: crzclass and cjmx check 0x20071030 at 0x01501f80, outside the area the program clears
      on first boot; nothing in the program writes it (only an unreferenced routine does, with garbage).
*/

// 开心斗地主 (Kāixīn Dòu Dìzhǔ) (Happy Dou Dizhu)
ROM_START( kxddz )
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD( "ddz.001.rom",  0x000000, 0x400000, CRC(b379f823) SHA1(531885b35d668d22c75a9759994f4aca6eacb046) )
	ROM_LOAD( "ddz.002.rom",  0x400000, 0x400000, CRC(285c744d) SHA1(2f8bc70825e55e3114015cb263e786df35cde275) )
	ROM_LOAD( "ddz.003.rom",  0x800000, 0x400000, CRC(61c9b5c9) SHA1(0438417398403456a1c49408881797a94aa86f49) )

	ROM_REGION( 0x100, "cryptomem", 0 )
	ROM_LOAD( "at88sc153.bin", 0x000, 0x100, BAD_DUMP CRC(b8482d63) SHA1(0c43c82b556c01ef4852eb26141913fe28c52cf6) ) // AI-generated
ROM_END

/* "Chaoji Fengdou" PCB
     ______________________________________________________________
    |                             _________  ____________________  |
  __|                            HD74HC245P |ROM1 H28F320BDJ-TTL80 |
 |__           RAM3 ________      _________ |____________________| |
 |__ M         K4S641632K-UC60   HD74HC245P   ____________________ |
 |__ A             |________|     _________  |ROM2 H28F320BDJ-TTL80|
 |__ H                           HD74HC245P  |____________________||
 |__ -                    XTAL    _________   ____________________ |
 |__ J       ___________  14.    HD74HC245P  |ROM3 H28F320BDJ-TTL80|
 |__ O      |VRenderZero| 318     ________   |____________________||
 |__ N      |MagicEyes  | MHz     HD74HC00P   ____________________ |
 |__ G      |EISC       |         ________   |ROM4 - EMPTY        ||
 |__        |___________|        |Unknown|   |____________________||
    |                                                              |
    |  RAM1 ________  RAM2 ________    _________   _____           |
    | K4S641632K-UC60 K4S641632K-UC60 | Unknown|  |Unkn|           |
    |      |________|     |________|  |        |  |____|      LED  |
    | ________                        |________|               o   |
  __| ULN2003AN                                         __         |
 |                                                      | |        |
 |__                                        ULN2003AN-> | |  ____  |
 |                                                      |_|  |K1|  |
 |__                                                         |__|  |
    |       _____       ___     JAMMA               _____   Switch |
    |______|     |_|_| |   |_|_|_|_|_|_|_|_|_|_|_|_|     |_________|
*/

// 超级疯斗 (Chāojí Fēngdòu) (Super Crazy Fight)
ROM_START( crzclass )
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD( "tjf-mahjong-rom1.bin",  0x000000, 0x400000, CRC(0a8af816) SHA1(9f292e847873078ed2b7584f463633cf9086c7e8) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD( "tjf-mahjong-rom2.bin",  0x400000, 0x400000, CRC(2a04e84a) SHA1(189b16fd4314fd2a5f8a1214618b5db83f8ac59a) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD( "tjf-mahjong-rom3.bin",  0x800000, 0x400000, CRC(1cacf3f9) SHA1(e6c88c98aeb7df4098f8e20f412018617005724d) ) // SHARP LH28F320BJD-TTL80
	// rom4 not populated

	ROM_REGION( 0x100, "cryptomem", 0 )
	ROM_LOAD( "at88sc153.bin", 0x000, 0x100, BAD_DUMP CRC(e062b61b) SHA1(fa77147fb8a4b48d253de815afec397082b53ff0) ) // AI-generated

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "nvram.bin", 0x0000, 0x2000, BAD_DUMP CRC(e36b21e0) SHA1(9a2d4cefc9c0dbc829b72893a002fd73a7ea2a06) ) // AI-generated
ROM_END

// 百变双扣 (Bǎibiàn Shuāngkòu) (Ever-Changing Shuangkou) - same PCB as crzclass
ROM_START( baibians )
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD( "rom1", 0x000000, 0x400000, CRC(b9a61efe) SHA1(e0114c231bf301779033b357bc65b133f2ab00be) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD( "rom2", 0x400000, 0x400000, CRC(8b17c3a8) SHA1(b7c65fe9c262ec819ecb0d92e885c6b67d1ee938) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD( "rom3", 0x800000, 0x400000, CRC(41b9b943) SHA1(3379ddf851818209cfede641e7bcc4be8b982034) ) // SHARP LH28F320BJD-TTL80
	// rom4 not populated

	ROM_REGION( 0x100, "cryptomem", 0 )
	ROM_LOAD( "at88sc153.bin", 0x000, 0x100, BAD_DUMP CRC(456e82cf) SHA1(f16b213bb45894cfabab570e9c69fd42281f2551) ) // AI-generated
ROM_END

// 超级明星 · 两副牌斗地主 (Chāojí Míngxīng · Liǎng Fù Pái Dòu Dìzhǔ) (Super Star · Two-Deck Dou Dizhu)
// same PCB as crzclass, but this one has an Amazon LF instead of the VRenderZero
ROM_START( cjmx )
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD( "rom1", 0x000000, 0x400000, CRC(518ff760) SHA1(68e3fc3269060e0abd8473abfc141f038d63e4e4) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD( "rom2", 0x400000, 0x400000, CRC(9caa6b70) SHA1(9914793ac772b9ae113f820c181a653ab8a9634d) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD( "rom3", 0x800000, 0x400000, CRC(738db095) SHA1(073c92d8d22cbec5e6f5f5e4c25e532d2ab914d5) ) // SHARP LH28F320BJD-TTL80
	// rom4 not populated

	ROM_REGION( 0x100, "cryptomem", 0 )
	ROM_LOAD( "at88sc153.bin", 0x000, 0x100, BAD_DUMP CRC(3ad5e485) SHA1(c8b63cd6b2d843b7274ffbd0281edaec7f158c01) ) // AI-generated

	ROM_REGION( 0x2000, "nvram", 0 )
	ROM_LOAD( "nvram.bin", 0x0000, 0x2000, BAD_DUMP CRC(e36b21e0) SHA1(9a2d4cefc9c0dbc829b72893a002fd73a7ea2a06) ) // AI-generated
ROM_END

ROM_START( btlfront )
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION32_LE( 0x2000000, "assets", ROMREGION_ERASEFF )

	ROM_REGION( 0x2200000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD( "mxg023338.rom5", 0x0000000, 0x200000, CRC(9d27c8dd) SHA1(9ff30bde6cc8711167876e78319e6ba6a2c12390) ) // 29L1611GPC-10, there are remains of a sticker
	ROM_LOAD( "mxs0331.rom1",   0x0200000, 0x800000, CRC(3bc2a4e8) SHA1(7f8639f721ca4cd55850507fa10e98778da0e2aa) ) // 26L6420MC-10, no sticker
	ROM_LOAD( "mxs0304.rom2",   0x0a00000, 0x800000, CRC(84aa8ad7) SHA1(213e405cce2d204406923df14bc5ebb37b480e3f) ) // 26L6420MC-90, "
	ROM_LOAD( "mxs0239.rom3",   0x1200000, 0x800000, CRC(bfeccc8e) SHA1(7d3faad159a655140210abd98c658ad2f16cbcfc) ) // 26L6420MC-90, "
	ROM_LOAD( "mxs0524.rom4",   0x1a00000, 0x800000, CRC(eec8adb5) SHA1(ea7d2456419f4ae53a0e1bc0c1247390e802416f) ) // 26L6420MC-90, "

	ROM_REGION( 0x100, "cryptomem", 0 )
	ROM_LOAD( "at88sc153.bin", 0x000, 0x100, BAD_DUMP CRC(456e82cf) SHA1(f16b213bb45894cfabab570e9c69fd42281f2551) ) // AI-generated
ROM_END


// 西部牛仔 (Xībù Niúzǎi) (Western Cowboy) - 虹猫蓝兔 (Hóngmāo Lántù) stickers, the bookkeeping shows 80822
ROM_START( xbnz )
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD( "rom1.bin", 0x000000, 0x400000, CRC(5ceaeef7) SHA1(ff79fc0aebfbaeab56e79a29802ff007f5f2b45f) )
	ROM_LOAD( "rom2.bin", 0x400000, 0x400000, CRC(87b86f53) SHA1(141097ee53ebbaa36993dbb841de5a1a5c458798) )
	ROM_LOAD( "rom3.bin", 0x800000, 0x400000, CRC(61a5163b) SHA1(d8c9918142b746c86e9d3cb24179ccabb66d733a) )
	// rom4 not populated

	ROM_REGION( 0x100, "io", 0 )
	ROM_LOAD( "zones.bin", 0x000, 0x100, BAD_DUMP CRC(c46bac53) SHA1(35bfd1468b6b405551830987867a74274adccdd8) ) // AI-generated
ROM_END

// 活捉泥鳅 (Huózhuō Níqiū) (Catch the Loach) - 虹猫蓝兔 (Hóngmāo Lántù) stickers, the bookkeeping shows 80821
ROM_START( hzniqiu )
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )

	ROM_REGION( 0x1000000, "enc_data", ROMREGION_ERASEFF )
	ROM_LOAD( "rom1.bin", 0x000000, 0x400000, CRC(8c92fbb7) SHA1(c259c1597beaf48bf7368b0c0e4d048d56527db7) )
	ROM_LOAD( "rom2.bin", 0x400000, 0x400000, CRC(e5685297) SHA1(c3081abe4645ee502bef0325cd895297e439bf41) )
	ROM_LOAD( "rom3.bin", 0x800000, 0x400000, CRC(33eea853) SHA1(d0d3dd7870575e9099dda106cbc9e480b9f9aff9) )
	// rom4 not populated
ROM_END


void tjf_state::descramble_flash(u8 *dst, u8 const *src, u32 len, u16 addrxor)
{
	// A10<->A13, A11<->A12, then per-game inversions; len must be a multiple of 0x10000
	for (u32 i = 0; i < len; i++)
		dst[i] = src[(i & ~0xffff) | (bitswap<16>(i, 15,14, 10,11,12,13, 9,8, 7,6,5,4,3,2,1,0) ^ addrxor)];
}

void tjf_state::unscramble_program(u8 *buf, u32 start, u32 end)
{
	// blocks of 27 words: first 2 words in order, the following 25 reversed
	for (u32 blk = start; (blk + 27 * 2) <= end; blk += 27 * 2)
		for (int j = 0; j < 12; j++)
			for (int b = 0; b < 2; b++)
				std::swap(buf[blk + 4 + j * 2 + b], buf[blk + 4 + (24 - j) * 2 + b]);
}

void tjf_state::decrypt(u16 addrxor, u32 prog_start, u32 prog_end, u32 prog_base)
{
	u32 const len = m_encdata.bytes();
	std::vector<u8> buf(len);

	descramble_flash(&buf[0], &m_encdata[0], len, addrxor);
	unscramble_program(&buf[0], prog_start, prog_end);

	u8 *const ipl = memregion("ipl")->base();
	for (u32 i = 0; i < len; i++)
		ipl[BYTE4_XOR_LE(i)] = buf[i];

	m_prog_start = prog_start;
	m_prog_end = prog_end;
	m_prog_base = prog_base;
}

void tjf_state::init_kxddz()
{
	decrypt(0xc300, 0x406, 0x40000, 0x02640000);
}

void tjf_state::init_crzclass()
{
	decrypt(0xc700, 0x446, 0x80000, 0x02080000);
}

void tjf_state::init_baibians()
{
	decrypt(0x4b00, 0x446, 0x80000, 0x02080000);
}

void tjf_state::init_cjmx()
{
	decrypt(0xcf00, 0x446, 0x80000, 0x02080000);
}

void tjf_state::init_btlfront()
{
	// rom5: boot stub and program
	std::vector<u8> buf(0x200000);
	descramble_flash(&buf[0], &m_encdata[0], 0x200000, 0xcf00);
	unscramble_program(&buf[0], 0x3e2, 0x20000);

	u8 *const ipl = memregion("ipl")->base();
	for (u32 i = 0; i < 0x200000; i++)
		ipl[BYTE4_XOR_LE(i)] = buf[i];

	// rom1-rom4: same address scrambling
	// rom3 and rom4 are seen at 0x05000000, rom1 and rom2 at 0x06000000
	static constexpr u32 dest[4] = { 0x1000000, 0x1800000, 0x0000000, 0x0800000 };
	u8 *const assets = memregion("assets")->base();
	buf.resize(0x800000);
	for (int rom = 0; rom < 4; rom++)
	{
		descramble_flash(&buf[0], &m_encdata[0x200000 + rom * 0x800000], 0x800000, 0xcf00);
		for (u32 i = 0; i < 0x800000; i++)
			assets[BYTE4_XOR_LE(dest[rom] + i)] = buf[i];
	}

	m_prog_start = 0x3e2;
	m_prog_end = 0x20000;
	m_prog_base = 0x02710000;
}

void tjf_medal_state::descramble(u16 addrxor)
{
	// A8-A15 reversed, then per-game inversions; the boot stub copies and unscrambles the program by itself
	u32 const len = m_encdata.bytes();
	u8 *const ipl = memregion("ipl")->base();
	for (u32 i = 0; i < len; i++)
		ipl[BYTE4_XOR_LE(i)] = m_encdata[(i & ~0xffff) | (bitswap<16>(i, 8,9,10,11,12,13,14,15, 7,6,5,4,3,2,1,0) ^ addrxor)];
}

void tjf_medal_state::init_xbnz()
{
	descramble(0xf000);
}

void tjf_medal_state::init_hzniqiu()
{
	descramble(0xfc00);
}

} // anonymous namespace


GAME( 200?, kxddz,    0, kxddz,    tjf,      tjf_state,       init_kxddz,    ROT0, "TJF", "Kaixin Dou Dizhu",    MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, crzclass, 0, tjf,      tjf,      tjf_state,       init_crzclass, ROT0, "TJF", "Chaoji Fengdou",      MACHINE_IMPERFECT_CONTROLS | MACHINE_NOT_WORKING )
GAME( 200?, baibians, 0, tjf,      tjf,      tjf_state,       init_baibians, ROT0, "TJF", "Baibian Shuangkou",   MACHINE_IMPERFECT_CONTROLS | MACHINE_NOT_WORKING )
GAME( 200?, cjmx,     0, tjf,      tjf,      tjf_state,       init_cjmx,     ROT0, "TJF", "Chaoji Mingxing",     MACHINE_IMPERFECT_CONTROLS | MACHINE_NOT_WORKING )
GAME( 2005, btlfront, 0, btlfront, btlfront, tjf_state,       init_btlfront, ROT0, "TJF", "BattleFront (V133F)", MACHINE_IMPERFECT_CONTROLS | MACHINE_NOT_WORKING )

GAME( 200?, xbnz,     0, medal,    medal,    tjf_medal_state, init_xbnz,     ROT0, "TJF", "Xibu Niuzai",         MACHINE_IMPERFECT_CONTROLS | MACHINE_NOT_WORKING )
GAME( 200?, hzniqiu,  0, medal,    medal,    tjf_medal_state, init_hzniqiu,  ROT0, "TJF", "Huozhuo Niqiu",       MACHINE_IMPERFECT_CONTROLS | MACHINE_NOT_WORKING )
