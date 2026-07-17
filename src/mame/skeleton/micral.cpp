// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************

Bull (Originally R2E) Micral 80-22G

2015-10-01 Skeleton [Robbbert]

http://www.ti99.com/exelvision/website/index.php?page=r2e-micral-8022-g

This expensive, futuristic-looking design featured a motherboard and slots,
much like an ancient PC. The known chip complement is:
Z80A, 4MHz; 64KB RAM, 2KB BIOS ROM, 256x4 prom (7611);
CRT8002, TMS9937 (=CRT5037), 4KB video RAM, 256x4 prom (7611);
2x 5.25 inch floppy drives, one ST506 5MB hard drive;
CDP6402 UART. Sound is a beeper.
The keyboard has a uPD780C (=Z80) and 1KB of ROM.

Reverse-engineered schematics of the whole machine (including the discrete
floppy subsystem) and a commented disassembly of the boot ROM are available:
https://forum.system-cfg.com/viewtopic.php?t=12786 (R2E Micral 8022 G thread)
The HDC is still unknown.

Commands must be in uppercase. Reboot to exit each command.
Bd[,t]: boot from floppy drive d (track t; "B0" boots CP/M from drive 0,
        "B0,8" boots Prologue/QMOS)
Gxxxx : go (writes a jump @FFED then jumps to FFEB)
T     : test
*     : echo keystrokes
enter : boot drive 0 track 8

Using generic keyboard via the uart for now, the real keyboard MCU is not
hooked up yet.

FFF8/9 are used for sending instructions to the screen. FFF9 is command/status,
and FFF8 is data. The codes 0-D seem to be for the CRT5037, but the results don't
make much sense. Code A0 is to write a byte to the current cursor position, and
B0 is to get the status.

Screen needs:
- Proper character generator (currently borrowing the c10 chargen, BAD_DUMP)
- According to the web, graphics are possible. A screenshot shows reverse video
  exists.

Other things...
- Beeper
- The real keyboard MCU hookup
- ST506 hard disk

--------------------
Honeywell Bull Questar/M

The Questar/M is a rebadged Micral 80-22 (R2E had been acquired by Bull).
Both machines boot CP/M 2.2 ("R2E 8021 57k CP/M vers 2.23", type B0 at the
monitor prompt) and Prologue/QMOS (type B0,8) from the floppy images dumped
from a surviving Questar/M.

http://www.histoireinform.com/Histoire/+infos6/chr6inf3.htm
https://www.esocop.org/docs/Questar.pdf

Acknowledgements:
- aquarius (forums.bannister.org) dumped the Questar/M BIOS ROM and the
  CP/M and QMOS floppies, and reverse-engineered the hard-sectored track
  format: https://forums.bannister.org/ubbthreads.php?ubb=showflat&Number=115424
- rfka01 added the Questar/M BIOS to this driver.
- The Micral 80-22G schematics and commented ROM disassembly published on
  forum.system-cfg.com documented the floppy register interface.

*********************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
//#include "sound/beep.h"
#include "video/tms9927.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  Floppy disk image device
//
//  Hard-sectored Tandon TM100-4: 80 cylinders, 2 sides, 16 sectors of
//  256 bytes per side (logical sectors 0-15 = side 1, 16-31 = side 2),
//  640K total.  Image format: raw decoded sector data, cylinder-major,
//  32 x 256 bytes per cylinder (as produced by aquarius' kryoflux decode).
//
//  The FDC is discrete logic (see Micral 80-22G schematics, sheets 07-12):
//  the CPU reads/writes the bit stream serially through 0xFFFF (one bit
//  per access, bit 0, inverted on read), with drive/sector select at
//  0xFFFD and a command register at 0xFFFE.  Media layout per sector:
//  sync (00 00 05), then 00, track, sector, 256 data bytes and two
//  checksum bytes (one's-complement running sum and second-order sum,
//  cf. CP/M BIOS routine at F56C).
//**************************************************************************

class micral_floppy_device : public device_t, public device_image_interface
{
public:
	micral_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	virtual const char *image_type_name() const noexcept override { return "floppydisk"; }
	virtual const char *image_brief_type_name() const noexcept override { return "flop"; }
	virtual bool is_readable() const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "disk,img,bin"; }

	virtual std::pair<std::error_condition, std::string> call_load() override
	{
		if (length() != DISK_SIZE)
			return std::make_pair(image_error::INVALIDLENGTH, "Invalid image size (must be 655360 bytes)");
		fseek(0, SEEK_SET);
		if (fread(m_data.get(), DISK_SIZE) != DISK_SIZE)
			return std::make_pair(image_error::UNSPECIFIED, "Error reading file");
		m_loaded = true;
		return std::make_pair(std::error_condition(), std::string());
	}

	virtual void call_unload() override
	{
		m_loaded = false;
	}

	bool loaded() const { return m_loaded; }

	// 256 bytes of sector data, or nullptr
	u8 *sector(u8 cyl, u8 sec)
	{
		if (!m_loaded || cyl >= 80 || sec >= 32)
			return nullptr;
		return &m_data[cyl * 32 * 256 + sec * 256];
	}

	void write_sector(u8 cyl, u8 sec, const u8 *src)
	{
		u8 *d = sector(cyl, sec);
		if (!d)
			return;
		memcpy(d, src, 256);
		if (!is_readonly())
		{
			fseek(cyl * 32 * 256 + sec * 256, SEEK_SET);
			fwrite(src, 256);
		}
	}

	static constexpr u32 DISK_SIZE = 80 * 32 * 256;

protected:
	virtual void device_start() override
	{
		m_data = make_unique_clear<u8[]>(DISK_SIZE);
		save_pointer(NAME(m_data), DISK_SIZE);
		save_item(NAME(m_loaded));
	}

private:
	std::unique_ptr<u8[]> m_data;
	bool m_loaded = false;
};

} // anonymous namespace

DEFINE_DEVICE_TYPE(MICRAL_FLOPPY, micral_floppy_device, "micral_floppy", "Micral/Questar hard-sectored disk")

namespace {

micral_floppy_device::micral_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MICRAL_FLOPPY, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}


class micral_state : public driver_device
{
public:
	micral_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		//, m_beep(*this, "beeper")
		, m_p_chargen(*this, "chargen")
		, m_uart(*this, "uart")
		, m_crtc(*this, "crtc")
		, m_flop(*this, "flop%u", 0U)
	{ }

	void micral(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	u8 keyin_r();
	u8 status_r();
	u8 unk_r();
	u8 video_r(offs_t offset);
	void video_w(offs_t offset, u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// floppy
	u8 fd_stat_r();
	void fd_sel_w(u8 data);
	void fd_cmd_w(u8 data);
	u8 fd_data_r();
	void fd_data_w(u8 data);
	int fd_selected_drive() const;
	void fd_build_stream();
	void fd_commit_sector();
	static void fd_chk_update(u8 &d, u8 &e, u8 a);

	void io_kbd(address_map &map) ATTR_COLD;
	void mem_kbd(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u16 s_curpos = 0U;
	u8 s_command = 0U;
	u8 s_data = 0U;
	u8 s_reg6 = 23U;                 // CRT reg 6: last displayed data row (scroll offset)

	// floppy state
	u8 m_fd_sel = 0U;                // last write to 0xFFFD
	u8 m_fd_cmd = 0U;                // last write to 0xFFFE
	u8 m_fd_cyl[2] = { 0U, 0U };     // head position per drive
	u8 m_fd_sec_latch = 0U;          // sector+side latched at seek time
	bool m_fd_seek_done = false;     // status bit 0
	u8 m_fd_stream[261]{};           // 00, track, sector, 256 data, chk1, chk2
	int m_fd_bitpos = 0;             // -1 = pad bit, then 8 bits/byte MSB first
	bool m_fd_reading = false;
	bool m_fd_writing = false;       // write-start seen with data mode
	bool m_fd_wr_synced = false;     // sync pattern (...000101) detected
	u16 m_fd_wr_window = 0U;         // last written bits for sync hunt
	u32 m_fd_wr_count = 0U;          // bits collected since sync
	u8 m_fd_wr_buf[261]{};

	std::unique_ptr<u8[]> m_vram;
	memory_passthrough_handler m_rom_shadow_tap;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	//required_device<beep_device> m_beep;
	required_region_ptr<u8> m_p_chargen;
	required_device<ay31015_device> m_uart;
	required_device<crt5037_device> m_crtc;
	required_device_array<micral_floppy_device, 2> m_flop;
};

u8 micral_state::status_r()
{
	return m_uart->dav_r() | 4;
}

u8 micral_state::unk_r()
{
	return 0x96;
}

u8 micral_state::keyin_r()
{
	m_uart->write_rdav(0);
	u8 result = m_uart->receive();
	m_uart->write_rdav(1);
	return result;
}

//**************************************************************************
//  Floppy
//**************************************************************************

// checksum pair update, exactly as CP/M BIOS routine at F56C:
//   add a,d / adc a,0 / ld d,a / add a,e / adc a,0 / ld e,a
void micral_state::fd_chk_update(u8 &d, u8 &e, u8 a)
{
	u16 t = u16(a) + d;
	a = u8(t) + BIT(t, 8);
	d = a;
	t = u16(a) + e;
	a = u8(t) + BIT(t, 8);
	e = a;
}

int micral_state::fd_selected_drive() const
{
	if (BIT(m_fd_sel, 4))
		return 0;
	if (BIT(m_fd_sel, 5))
		return 1;
	return -1;
}

// FFFD read: bit 0 = sector found, bit 1 = track 0, bit 2 = write protected,
// bit 3 = double sided drive, bit 4 = first command flag
u8 micral_state::fd_stat_r()
{
	int drive = fd_selected_drive();
	if (drive < 0 || !m_flop[drive]->loaded())
		return 0x00;

	u8 stat = 0x08;    // Tandon TM100-4 = double sided
	if (m_fd_seek_done)
		stat |= 0x01;
	if (m_fd_cyl[drive] == 0)
		stat |= 0x02;
	if (m_flop[drive]->is_readonly())
		stat |= 0x04;
	return stat;
}

// FFFD write: bits 0-3 sector, bit 4 drive 0, bit 5 drive 1
void micral_state::fd_sel_w(u8 data)
{
	m_fd_sel = data;
}

// FFFE write: bit 0 step direction, bit 1 step pulse, bit 2 ext pinout,
// bit 3 motor, bit 4 data mode / clear bit counter, bit 5 sector seek,
// bit 6 write start, bit 7 side select
void micral_state::fd_cmd_w(u8 data)
{
	u8 prev = m_fd_cmd;
	m_fd_cmd = data;

	int drive = fd_selected_drive();

	// step pulse (rising edge of bit 1)
	if (BIT(data, 1) && !BIT(prev, 1) && drive >= 0)
	{
		if (BIT(data, 0))
		{
			if (m_fd_cyl[drive] < 79)
				m_fd_cyl[drive]++;
		}
		else
		{
			if (m_fd_cyl[drive] > 0)
				m_fd_cyl[drive]--;
		}
	}

	// sector seek: the hard-sector logic finds the requested sector hole.
	// Sector and side are latched here: the boot ROM drops the side bit
	// from the command when it later switches to data mode (writes 1C).
	if (BIT(data, 5))
	{
		m_fd_sec_latch = (m_fd_sel & 0x0f) | (BIT(data, 7) ? 0x10 : 0x00);
		m_fd_seek_done = (drive >= 0 && m_flop[drive]->loaded());
	}
	else if (!BIT(data, 5) && BIT(prev, 5))
		m_fd_seek_done = false;

	// data mode (rising edge of bit 4): reset the serial bit counter
	if (BIT(data, 4) && !BIT(prev, 4))
	{
		if (BIT(data, 6))
		{
			// write: hunt for the software-generated sync pattern
			m_fd_writing = true;
			m_fd_reading = false;
			m_fd_wr_synced = false;
			m_fd_wr_window = 0;
			m_fd_wr_count = 0;
		}
		else
		{
			m_fd_reading = false;
			m_fd_writing = false;
			fd_build_stream();
		}
	}

	// write start dropped: abort any write in progress
	if (!BIT(data, 6) && BIT(prev, 6))
		m_fd_writing = false;
}

// build the post-sync serial stream for the currently selected
// drive / head position / sector / side
void micral_state::fd_build_stream()
{
	int drive = fd_selected_drive();
	if (drive < 0)
		return;

	u8 cyl = m_fd_cyl[drive];
	u8 sec = m_fd_sec_latch;

	u8 const *data = m_flop[drive]->sector(cyl, sec);
	if (!data)
		return;

	m_fd_stream[0] = 0x00;    // rest of the sync field
	m_fd_stream[1] = cyl;
	m_fd_stream[2] = sec;
	memcpy(&m_fd_stream[3], data, 256);

	u8 d = cyl, e = cyl;
	fd_chk_update(d, e, sec);
	for (int i = 0; i < 256; i++)
		fd_chk_update(d, e, data[i]);
	m_fd_stream[259] = d;
	m_fd_stream[260] = e;

	m_fd_bitpos = -1;         // one pad bit before the first byte
	m_fd_reading = true;
}

// FFFF read: one bit per access in bit 0 (inverted), bits 1-7 high
u8 micral_state::fd_data_r()
{
	if (!m_fd_reading)
		return 0xff;

	if (m_fd_bitpos < 0)
	{
		if (!machine().side_effects_disabled())
			m_fd_bitpos++;
		return 0xfe;
	}

	if (m_fd_bitpos >= 261 * 8)
		return 0xff;

	u8 byte = m_fd_stream[m_fd_bitpos >> 3];
	u8 bit = BIT(byte, 7 - (m_fd_bitpos & 7));
	if (!machine().side_effects_disabled())
		m_fd_bitpos++;
	return 0xfe | (bit ^ 1);
}

// FFFF write: one bit per access in bit 0 (not inverted).  The software
// writes a long run of zeros, the sync pattern 101 (making bytes 05 00
// with the preceding zeros), then 00, track, sector, 256 data bytes and
// two checksum bytes.
void micral_state::fd_data_w(u8 data)
{
	if (!m_fd_writing)
		return;

	u8 bit = BIT(data, 0);

	if (!m_fd_wr_synced)
	{
		m_fd_wr_window = (m_fd_wr_window << 1) | bit;
		if ((m_fd_wr_window & 0x3f) == 0x05)    // ...000101
		{
			m_fd_wr_synced = true;
			m_fd_wr_count = 0;
		}
		return;
	}

	if (m_fd_wr_count < 261 * 8)
	{
		u32 idx = m_fd_wr_count >> 3;
		m_fd_wr_buf[idx] = (m_fd_wr_buf[idx] << 1) | bit;
		m_fd_wr_count++;
		if (m_fd_wr_count == 261 * 8)
			fd_commit_sector();
	}
}

void micral_state::fd_commit_sector()
{
	int drive = fd_selected_drive();
	if (drive < 0)
		return;

	u8 cyl = m_fd_wr_buf[1];
	u8 sec = m_fd_wr_buf[2];

	u8 d = cyl, e = cyl;
	fd_chk_update(d, e, sec);
	for (int i = 0; i < 256; i++)
		fd_chk_update(d, e, m_fd_wr_buf[3 + i]);
	if (d != m_fd_wr_buf[259] || e != m_fd_wr_buf[260])
		logerror("fd write: checksum mismatch on C%u S%u (%02X/%02X vs %02X/%02X)\n",
				cyl, sec, m_fd_wr_buf[259], m_fd_wr_buf[260], d, e);

	m_flop[drive]->write_sector(cyl, sec, &m_fd_wr_buf[3]);
}

u8 micral_state::video_r(offs_t offset)
{
	if (offset)
		return 0x07;
	else
		return m_vram[s_curpos];
}

void micral_state::video_w(offs_t offset, u8 data)
{
	if (offset)
	{
		s_command = data;
		if (s_command == 0x0c)
			s_curpos = (s_curpos & 0xff00) | s_data;
		else
		if (s_command == 0x0d)
			s_curpos = (s_curpos & 0xff) | ((s_data & 0x1f) << 8);
		else
		if (s_command == 0xa0)
			m_vram[s_curpos] = s_data;
		else
		if ((s_command & 0xbf) == 0x06)    // reg 6: last displayed data row (hardware scroll)
			s_reg6 = s_data & 0x1f;

		//if (s_command < 0x10)
			//m_crtc->write(s_command, s_data);
	}
	else
	{
		s_data = data;
	}
}


void micral_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffef).ram().share("mainram");
	map(0xf800, 0xfeff).rom().region("maincpu", 0);
	map(0xfff6, 0xfff7); // .nopw(); // unknown ports
	map(0xfff8, 0xfff9).rw(FUNC(micral_state::video_r), FUNC(micral_state::video_w));
	map(0xfffa, 0xfffa).r(FUNC(micral_state::keyin_r));
	map(0xfffb, 0xfffb).r(FUNC(micral_state::unk_r));
	map(0xfffc, 0xfffc).r(FUNC(micral_state::status_r));
	map(0xfffd, 0xfffd).rw(FUNC(micral_state::fd_stat_r), FUNC(micral_state::fd_sel_w));
	map(0xfffe, 0xfffe).w(FUNC(micral_state::fd_cmd_w));
	map(0xffff, 0xffff).rw(FUNC(micral_state::fd_data_r), FUNC(micral_state::fd_data_w));
}

void micral_state::mem_kbd(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).rom();
	map(0x8000, 0x8000).ram(); // byte returned to main cpu after receiving irq
	map(0x8001, 0x8001).portr("X0");
	map(0x8002, 0x8002).portr("X1");
	map(0x8004, 0x8004).portr("X2");
	map(0x8008, 0x8008).portr("X3");
	map(0x8010, 0x8010).portr("X4");
	map(0x8020, 0x8020).portr("X5");
	map(0x8040, 0x8040).portr("X6");
	map(0x8080, 0x8080).portr("X7");
	map(0x8100, 0x8100).portr("X8");
	map(0x8200, 0x8200).portr("X9");
	map(0x8400, 0x8400).portr("X10");
	map(0x8800, 0x8800).portr("X11");
	map(0x9000, 0x9000).portr("X12");
	map(0xa000, 0xa000).portr("X13");
}

void micral_state::io_kbd(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("X14");
}

/* Input ports */
static INPUT_PORTS_START( micral )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 01
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 03
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) // 2A
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) // 22
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) // 28
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) // 94
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) // 90
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 29

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 3E, 3C
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // '^'
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) // 5B
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // OB
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) // 3F
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // ':','/'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 08
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 06

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 02
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) // 91
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) // 27
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) // '-'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) // '_'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) // 8E
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) // '+'

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // '@', '#'
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 9C, '%'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 05
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 7F
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)

	PORT_START("X11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // ';','.'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // '!','&'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 0A
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 95,FE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 97,FC
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 9D,'$'
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 96,'\'
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 99,84
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 9A,92

	PORT_START("X13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) // 00

	PORT_START("X14")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RCONTROL) // ?? don't look for a new keypress
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) // ??
INPUT_PORTS_END

uint32_t micral_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0;
	uint8_t const top = (s_reg6 + 1) % 24;    // first displayed data row

	for (uint8_t y = 0; y < 24; y++)
	{
		uint8_t const row = (top + y) % 24;
		uint16_t const ma = row << 8;

		for (uint8_t ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = 0; x < 80; x++)
			{
				uint8_t gfx = 0;
				if (ra < 9)
				{
					uint8_t chr = m_vram[x+ma];
					gfx = m_p_chargen[(chr<<4) | ra ];
					if (((s_curpos & 0xff)==x) && ((s_curpos >> 8)==row))
						gfx ^= 0xff;
				}
				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
	}
	return 0;
}

void micral_state::machine_reset()
{
	m_fd_sel = 0;
	m_fd_cmd = 0;
	m_fd_seek_done = false;
	m_fd_reading = false;
	m_fd_writing = false;
	s_reg6 = 23;

	// no idea if these are hard-coded, or programmable
	m_uart->write_xr(0);
	m_uart->write_xr(1);
	m_uart->write_swe(0);
	m_uart->write_np(1);
	m_uart->write_tsb(0);
	m_uart->write_nb1(1);
	m_uart->write_nb2(1);
	m_uart->write_eps(1);
	m_uart->write_cs(1);
	m_uart->write_cs(0);

	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xf800, 0xffff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x07ff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

void micral_state::machine_start()
{
	m_vram = make_unique_clear<u8[]>(0x2000);
	save_pointer(NAME(m_vram), 0x2000);
	save_item(NAME(s_curpos));
	save_item(NAME(s_command));
	save_item(NAME(s_data));
	save_item(NAME(s_reg6));
	save_item(NAME(m_fd_sel));
	save_item(NAME(m_fd_cmd));
	save_item(NAME(m_fd_cyl));
	save_item(NAME(m_fd_sec_latch));
	save_item(NAME(m_fd_seek_done));
	save_item(NAME(m_fd_stream));
	save_item(NAME(m_fd_bitpos));
	save_item(NAME(m_fd_reading));
	save_item(NAME(m_fd_writing));
	save_item(NAME(m_fd_wr_synced));
	save_item(NAME(m_fd_wr_window));
	save_item(NAME(m_fd_wr_count));
	save_item(NAME(m_fd_wr_buf));
}

void micral_state::micral(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &micral_state::mem_map);
	// no i/o ports on main cpu
	z80_device &keyboard(Z80(config, "keyboard", XTAL(1'000'000))); // freq unknown
	keyboard.set_addrmap(AS_PROGRAM, &micral_state::mem_kbd);
	keyboard.set_addrmap(AS_IO, &micral_state::io_kbd);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(250));
	screen.set_screen_update(FUNC(micral_state::screen_update));
	screen.set_size(640, 240);
	screen.set_visarea(0, 639, 0, 239);
	screen.set_palette("palette");
	PALETTE(config, "palette", palette_device::MONOCHROME);
	//GFXDECODE(config, "gfxdecode", "palette", gfx_micral);

	CRT5037(config, m_crtc, 4000000 / 8);  // xtal freq unknown
	m_crtc->set_char_width(8);  // unknown
	//m_crtc->vsyn_callback().set(TMS5501_TAG, FUNC(tms5501_device::sens_w));
	m_crtc->set_screen("screen");

	/* sound hardware */
	//SPEAKER(config, "mono").front_center();
	//BEEP(config, m_beep, 2000).add_route(ALL_OUTPUTS, "mono", 0.50);

	AY31015(config, m_uart); // CDP6402
	m_uart->read_si_callback().set("rs232", FUNC(rs232_port_device::rxd_r));
	m_uart->write_so_callback().set("rs232", FUNC(rs232_port_device::write_txd));

	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));
	uart_clock.signal_handler().set(m_uart, FUNC(ay31015_device::write_tcp));
	uart_clock.signal_handler().append(m_uart, FUNC(ay31015_device::write_rcp));

	RS232_PORT(config, "rs232", default_rs232_devices, "keyboard");

	MICRAL_FLOPPY(config, m_flop[0]);
	MICRAL_FLOPPY(config, m_flop[1]);
}

ROM_START( micral )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "8022g.rom",    0x0000, 0x0800, CRC(882732a9) SHA1(3f37b82c450a54aedec209bd46fcbcf131c86313) )

	ROM_REGION( 0x0400, "keyboard", 0 )
	ROM_LOAD( "2010221.rom",  0x0000, 0x0400, CRC(65123378) SHA1(401f0a648b78bf1662a1cd2546e83ba8e3cb7a42) )

	// Using the chargen from 'c10' for now.
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

ROM_START( questarm )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "qm_547_1.rom", 0x0000, 0x0800, CRC(8e6dc953) SHA1(b31375af8c6769578d2000fff3e751e94e7ae4d4) )

	// using the keyboard ROM from 'micral' for now
	ROM_REGION( 0x0400, "keyboard", 0 )
	ROM_LOAD( "2010221.rom",  0x0000, 0x0400, CRC(65123378) SHA1(401f0a648b78bf1662a1cd2546e83ba8e3cb7a42) )

	// Using the chargen from 'c10' for now.
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY           FULLNAME         FLAGS
COMP( 1981, micral,   0,      0,      micral,  micral, micral_state, empty_init, "Bull R2E",       "Micral 80-22G", MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
COMP( 1982, questarm, micral, 0,      micral,  micral, micral_state, empty_init, "Honeywell Bull", "Questar/M",     MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
