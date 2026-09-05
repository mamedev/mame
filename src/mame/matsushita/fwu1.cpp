// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Skeleton driver for Panasonic FW-U1 Personal Word Processors.

    Models list: https://ja.wikipedia.org/wiki/%E3%82%B9%E3%83%A9%E3%83%A9

    Hardware
    --------

    FW-U1P503AI (U1Pro503Ai):

    Main Board Markings: DFUP0334ZAU2 / Copyright Matsushita Electric Ind. 1989
    - IC6: Matsushita MN5602 9N.6
    - IC7: NEC D70116C-8 (V30 CPU)
    - IC8: Toshiba TC8556AF (Floppy Disk Controller)
    - IC12,13: Sharp LH52256N-90 (2 * 32K SRAM)
    - IC14,15: Toshiba TC518128AFL-10 (2 * 128K SRAM)
    - IC16,17,18,32: Toshiba TC51832FL-10 (4 * 32K SRAM)
    - IC31: Matsushita MN5605 9D.1
    - IC35: Matsushita DA534A3L-S P503AIOS (512K Mask ROM)
    - IC37: Fujitsu MB834200A-20 2C8 AA (512K Mask ROM)
    - IC38: Matsushita DA534A1A-S P503AIAPL (512K Mask ROM)
    - IC39: Matsushita DA534A91-S P503AIDIC (512K Mask ROM)
    - IC40: Fujitsu MB834200A-20 2C9 AA (512K Mask ROM)
    - IC51: Ricoh RP5B02W (256B SRAM)
    - IC77: Matsushita EPTS30001
    - IC373: Sanyo LB1634 (Motor Driver)
    - IC601: Mitsubishi M5M27C102K-15 (128K Mask ROM with label: "SYS ROM v2.0 / Copyright MEI / 1990 DAP503AIJ2")
    - IC611: NEC D4991G (RTC)

    FW-U1C70:

    Main Board Markings: DFUP0806ZAM1
    - IC100: Matsushita DA242434431J
    - IC131: Matsushita MN89303A (VGA Controller)
    - IC140: Fujitsu M5M44270AJ (512K DRAM)
    - IC200: UMC UM8663BF (SIO)
    - IC300: MEI DA535KYP-S (4M Mask ROM)
    - IC301: MEI DA23C32003XC (4M Mask ROM)
    - IC302: MEI DA5388WL-S (1M Mask ROM)
    - IC303: Fujitsu 29F400TA-12 (512K Mask ROM)
    - IC330,331: SEC KM416C256BLJ-6 (2 * 512K DRAM)
    - IC332: Panasonic MN414260CSJ-07 (512K DRAM)
    - IC550: Matsushita DA249426191J
    - IC600: Matsushita DA87A134PFVJ
    - IC610,620: Allegro MicroSystems UDN2916LB (Motor Driver)
    - IC880: Mitsubishi MT494FP
    - IC950: VM Technology VM863HLB-S25 (80286 Real Mode only CPU like VM863S? https://web.archive.org/web/20200805232019/http://margo.student.utwente.nl/stefan/chipdir/c/v.htm#vmt)

    CD-ROM Board Markings: DFUP0808ZAJ / (C) M.E.I 1996
    - IC701: Matsushita AN8856SB (CD-ROM Head Amplifier)
    - IC702: Panasonic MN662743CDC1 (CD-ROM Servo 4x Speed)
    - IC703,705: Matsushita AN8387S (2 * Motor Driver)
    - IC706: Matsushita NBC3903M

    Floppy Drive: Matsushita EME219MNG
    - IC1: BH6627FS (R/W Amplifier)
    - IC2: MH012

***************************************************************************/

#include "emu.h"

#include "cpu/i86/i286.h"
#include "cpu/nec/nec.h"
#include "imagedev/cdromimg.h"
#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "machine/upd4991a.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

#define LOG_IO (1U << 1)

//#define VERBOSE (LOG_IO)
#include "logmacro.h"


namespace {

class u1pro_state : public driver_device
{
public:
	u1pro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_banks(*this, "bank%u", 0U)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_rtc(*this, "rtc")
		, m_vram(*this, "vram")
		, m_flop(*this, "fdc")
		, m_fd_softlist(*this, "fd_list")
	{ }

	void u1pro(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_memory_bank_array<4> m_banks;
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<upd4991a_device> m_rtc;
	required_shared_ptr<u16> m_vram;
	required_device<floppy_connector> m_flop;
	required_device<software_list_device> m_fd_softlist;

	static void floppy_formats(format_registration &fr);

	void bank_switch(offs_t offset, u16 entry, u16 mem_mask);
	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

void u1pro_state::machine_start()
{
	u8 *rom = memregion("ic38")->base();
	for (size_t i = 0; i < 4; i++) {
		m_banks[i]->configure_entries(0, 8, rom, 0x10000);
		m_banks[i]->set_entry(0);
	}
}

static void u1pro_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

void u1pro_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
}

void u1pro_state::bank_switch(offs_t offset, u16 entry, u16 mem_mask)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t bank_start = 0xb0000 + (entry % 4) * 0x4000;
	offs_t bank_end = 0xb0000 + ((entry % 4) + 1) * 0x4000 - 1;
	program.unmap_read(bank_start, bank_end);

	u8 *mem;
	u8 num_entries = 8;
	// fwu1p503ai @ f000:0747 accumulates bytes of each mapped 0x10000 chunk,
	// compared against the 16-bit checksum at the end of the chunk. When the ROM
	// is mapped at b000:0000, this function iterates through all 8 chunks.
	if (entry < 0x20) {
		mem = memregion("ic38")->base();
	}
	// fwu1p503ai @ d000:d363 reads 0x20 bytes, matching size of bitmap characters.
	else if (entry >= 0x40 && entry < 0x50) {
		mem = memregion("ic35")->base() + 0x40000;
		num_entries = 4;
	}
	else if (entry >= 0xa0 && entry < 0xc0) {
		mem = memregion("ic37")->base();
	}
	else {
		LOGMASKED(LOG_IO, "%s: unknown bank offset=%04x entry=%04x\n", machine().describe_context(), offset, entry);
		return;
	}
	LOGMASKED(LOG_IO, "%s: bank offset=%04x entry=%04x\n", machine().describe_context(), offset, entry);

	m_banks[offset]->configure_entries(0, num_entries, mem + (entry % 4) * 0x4000, 0x10000);
	m_banks[offset]->set_entry((entry % (num_entries * 4)) / 4);
	program.install_read_bank(bank_start, bank_end, m_banks[offset]);
}

void u1pro_state::io_map(address_map &map)
{
	map.global_mask(0xffff);
	map(0x000e, 0x000f).lr16(
		NAME([this]() {
			if (!machine().side_effects_disabled()) {
				LOGMASKED(LOG_IO, "%s: io_0e_r\n", machine().describe_context());
			}
			return 0x3800; // Avoid halt @ d000:94bf
		}));
	map(0x0068, 0x006f).w(FUNC(u1pro_state::bank_switch));
}

void u1pro_state::prg_map(address_map &map)
{
	map(0x00000, 0x2ffff).ram();
	map(0xa0000, 0xaffff).ram().share("vram");
	map(0xb0000, 0xb3fff).bankr(m_banks[0]);
	map(0xb4000, 0xb7fff).bankr(m_banks[1]);
	map(0xb8000, 0xbbfff).bankr(m_banks[2]);
	map(0xbc000, 0xbffff).bankr(m_banks[3]);
	map(0xc0000, 0xeffff).rom().region("ic35", 0);
	map(0xf0000, 0xfffff).rom().region("ic601", 0x10000);
}

u32 u1pro_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 offset = 0;
	u8 *vram = reinterpret_cast<u8 *>(m_vram.target());
	for (int j = 0; j < m_screen->height(); j++) {
		u16 *const row = &bitmap.pix(j);

		for (int i = 0; i < (m_screen->width() + 1) / 8; i++) {
			int const x = i * 8;

			u8 const data = vram[offset];
			for (int b = 0; b < 8; b++) {
				row[x + (8 - b)] = BIT(data, b);
			}

			offset++;
		}
	}

	return 0;
}

void u1pro_state::u1pro(machine_config &config)
{
	V30(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &u1pro_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &u1pro_state::io_map);

	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(640, 320); // 40 * 20 characters
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(u1pro_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	// Actually UPD4991G.
	// Connected to X611 XTAL with frequency marked on PCB.
	UPD4991A(config, m_rtc, 32'768);

	FLOPPY_CONNECTOR(config, m_flop, u1pro_floppies, "35dd", u1pro_state::floppy_formats);
	SOFTWARE_LIST(config, "fd_list").set_original("fwu1_flop");
}

class u1color_state : public driver_device
{
public:
	u1color_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_banks_b(*this, "bank_b%u", 0U)
		, m_banks_d(*this, "bank_d%u", 0U)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_screen(*this, "screen")
		, m_vram(*this, "vram")
		, m_cdrom(*this, "cdrom")
		, m_cd_softlist(*this, "cd_list")
		, m_flop(*this, "fdc")
		, m_fd_softlist(*this, "fd_list")
	{ }

	void u1color(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_memory_bank_array<4> m_banks_b;
	required_memory_bank_array<4> m_banks_d;
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<screen_device> m_screen;
	required_shared_ptr<u16> m_vram;
	required_device<cdrom_image_device> m_cdrom;
	required_device<software_list_device> m_cd_softlist;
	required_device<floppy_connector> m_flop;
	required_device<software_list_device> m_fd_softlist;

	u16 m_banks_b_regs[4]{};
	u16 m_banks_d_regs[4]{};

	static void floppy_formats(format_registration &fr);

	void bank_switch(memory_bank *bank, offs_t base_addr, u16 entry);
	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
	void palette(palette_device &palette) const ATTR_COLD;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

void u1color_state::machine_start()
{
	u8 *mem = m_ram->pointer();
	for (size_t i = 0; i < 4; i++) {
		m_banks_b[i]->configure_entries(0, 8, mem, 0x10000);
		m_banks_b[i]->set_entry(0);
		m_banks_d[i]->configure_entries(0, 8, mem, 0x10000);
		m_banks_d[i]->set_entry(0);
	}
}

static void u1color_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
}

void u1color_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
}

void u1color_state::bank_switch(memory_bank *bank, offs_t base_addr, u16 entry)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	offs_t bank_start = base_addr + (entry % 4) * 0x4000;
	offs_t bank_end = base_addr + ((entry % 4) + 1) * 0x4000 - 1;
	program.unmap_readwrite(bank_start, bank_end);

	u8 *mem;
	u8 num_entries;
	// fwu1c70 @ f000:23e8 tests "XU" @ b000:0000 on 0x100 bank entries.
	if (entry >= 0x004 && entry < 0x100) {
		mem = memregion("ic300")->base();
		num_entries = 64;
	}
	// fwu1c70 @ f000:925f maps bank entry=024e to read character bitmaps.
	else if (entry >= 0x200 && entry < 0x300) {
		mem = memregion("ic301")->base();
		num_entries = 64;
	}
	else {
		LOGMASKED(LOG_IO, "%s: unknown bank start=%04x entry=%04x\n", machine().describe_context(), bank_start, entry);
		return;
	}
	LOGMASKED(LOG_IO, "%s: bank start=%04x entry=%04x\n", machine().describe_context(), bank_start, entry);

	bank->configure_entries(0, num_entries, mem + (entry % 4) * 0x4000, 0x10000);
	bank->set_entry((entry % (num_entries * 4)) / 4);
	program.install_read_bank(bank_start, bank_end, bank);
}

void u1color_state::io_map(address_map &map)
{
	map.global_mask(0xffff);
	map(0x0032, 0x0039).lrw16(
		NAME([this](offs_t offset) {
			return m_banks_d_regs[offset];
		}),
		NAME([this](offs_t offset, u16 data, u16 mem_mask) {
			m_banks_d_regs[offset] = data;
			bank_switch(m_banks_d[offset], 0xd0000, data);
		}));
	map(0x0068, 0x006f).lrw16(
		NAME([this](offs_t offset) {
			return m_banks_b_regs[offset];
		}),
		NAME([this](offs_t offset, u16 data, u16 mem_mask) {
			m_banks_b_regs[offset] = data;
			bank_switch(m_banks_b[offset], 0xb0000, data);
		}));
	map(0x00f6, 0x00f7).lr16(
		NAME([this]() {
			if (!machine().side_effects_disabled()) {
				LOGMASKED(LOG_IO, "%s: io_f6_r\n", machine().describe_context());
			}
			return 0x0800; // Avoid infinite loop @ f000:0e5f
		}));
}

void u1color_state::prg_map(address_map &map)
{
	map(0x000000, 0x00ffff).ram();
	map(0x070000, 0x07ffff).ram();
	map(0x0a0000, 0x0affff).ram().share("vram");

	map(0x0b0000, 0x0b3fff).bankr(m_banks_b[0]);
	map(0x0b4000, 0x0b7fff).bankr(m_banks_b[1]);
	map(0x0b8000, 0x0bbfff).bankr(m_banks_b[2]);
	map(0x0bc000, 0x0bffff).bankr(m_banks_b[3]);

	// f000:292c  JMPF c000:0020
	// c000:0020  JMPF c000:1020
	// c000:0025  JMPF c000:10a0
	map(0x0c0000, 0x0cffff).rom().region("ic303", 0x60000);

	map(0x0d0000, 0x0d3fff).bankr(m_banks_d[0]);
	map(0x0d4000, 0x0d7fff).bankr(m_banks_d[1]);
	map(0x0d8000, 0x0dbfff).bankr(m_banks_d[2]);
	map(0x0dc000, 0x0dffff).bankr(m_banks_d[3]);

	// f000:2787  CALLF e000:9c00
	map(0x0e0000, 0x0effff).rom().region("ic303", 0x40000);

	map(0x0f0000, 0x0fffff).mirror(0xf00000).rom().region("ic303", 0);
}

void u1color_state::palette(palette_device &palette) const
{
	for (size_t i = 0; i < 0xff; i++) {
		palette.set_pen_color(i, 0xff, 0xff, 0xff);
	}
	palette.set_pen_color(0xff, 0x00, 0x00, 0x00);
}

u32 u1color_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 offset = 0;
	u8 *vram = reinterpret_cast<u8 *>(m_vram.target());
	for (int j = 0; j < m_screen->height(); j++) {
		u16 *const row = &bitmap.pix(j);

		for (int i = 0; i < (m_screen->width() + 1) / 8; i++) {
			int const x = i * 8;

			u8 const data = vram[offset];
			for (int b = 0; b < 8; b++) {
				row[x + (8 - b)] = BIT(data, b) ? 0 : 0xff;
			}

			offset++;
		}
	}

	return 0;
}

void u1color_state::u1color(machine_config &config)
{
	// FIXME: Same clock as VM860S? https://archive.org/details/Micro_Systemes_-_Issue_109_1990-06_SPE_FR/page/40/mode/1up?q=VM860
	I80286(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &u1color_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &u1color_state::io_map);

	// FIXME: Missing banked accesses.
	RAM(config, "ram").set_default_size("512K").set_default_value(0x00);

	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(640, 480);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(u1color_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(u1color_state::palette), 256);

	CDROM(config, m_cdrom).set_interface("cdrom");
	SOFTWARE_LIST(config, m_cd_softlist).set_original("fwu1_cdrom");

	FLOPPY_CONNECTOR(config, m_flop, u1color_floppies, "35hd", u1color_state::floppy_formats);
	SOFTWARE_LIST(config, "fd_list").set_original("fwu1_flop");
}


ROM_START( fwu1p503ai )
	ROM_REGION16_LE(0x20000, "ic601", ROMREGION_ERASEFF)
	// 1st half is 0xFF-filled.
	// 2nd half program is almost identical to IC35 at offset 0x30000 (IC601 has NOPs patched at offset 0x1c5).
	// IC601 footer suggests a later build date (likely 1990-01-15 12:30 vs. 1989-10-04 21:26).
	ROM_LOAD("m5m27c102k-15.dap503aij2.ic601", 0x10000, 0x10000, CRC(e035137b) SHA1(39c30eb749fe722277e5f0a4225367c9577161a9))

	ROM_REGION16_LE(0x80000, "ic35", 0)
	ROM_LOAD("da534a3l-s.p503aios.ic35", 0x00000, 0x80000, CRC(197aaac1) SHA1(ec840d94b37716ccb5ccedbb93f5d2f6547415b5))

	ROM_REGION16_LE(0x80000, "ic37", 0)
	ROM_LOAD("mb834200a-20.2c8_aa.ic37", 0x00000, 0x80000, CRC(499c7056) SHA1(5cc3c9564d582705cfc342052be7b1edad2cd2f1))

	ROM_REGION16_LE(0x80000, "ic38", 0)
	ROM_LOAD("da534aia-s.p503aiapl.ic38", 0x00000, 0x80000, CRC(88a31b15) SHA1(3b7c3af32e86a647b4d236e0e7bae443f239b63e))

	ROM_REGION16_LE(0x80000, "ic39", 0)
	ROM_LOAD("da534a91-s.p503aidic.ic39", 0x00000, 0x80000, CRC(738e2a73) SHA1(4deac7dc92305323fbf4fdad672a1428d2f6b0e7))

	ROM_REGION16_LE(0x80000, "ic40", 0)
	ROM_LOAD("mb834200a-20.2c9_aa.ic40", 0x00000, 0x80000, CRC(3ba56308) SHA1(be8cd709202bfb55a8a6fdb52f2ba30b76bd22d3))
ROM_END

ROM_START( fwu1c70 )
	ROM_REGION16_LE(0x400000, "ic300", 0)
	// IC303 has identical chunks at the following offsets:
	// IC300 [0x00000..0x10000] @ IC303 0x00000;
	// IC300 [0x10000..0x20000] @ IC303 0x40000;
	// IC300 [0x40000..0x50000] @ IC303 0x60000;
	ROM_LOAD("da535kyp-s.ic300", 0x000000, 0x400000, CRC(cfa8039a) SHA1(219fdd008c138cfe309447d9e74506d454d483eb))

	ROM_REGION16_LE(0x400000, "ic301", 0)
	ROM_LOAD("da23c32003xc.ic301", 0x000000, 0x400000, CRC(14a1871d) SHA1(67c1e74ff0e86b07f3af75a35679294cd31c2a5c))

	ROM_REGION16_LE(0x100000, "ic302", 0)
	ROM_LOAD("da5388wl-s.ic302", 0x000000, 0x100000, CRC(272bd3af) SHA1(c8c41d8eb54c6b264b7b2429b2b5246f25c81317))

	ROM_REGION16_LE(0x080000, "ic303", 0)
	ROM_LOAD("29f400ta-12.ic303", 0x000000, 0x080000, CRC(94ee3350) SHA1(409049d7cdaecd2ff1b4f6a4bc209eef1406f7de))
ROM_END

} // anonymous namespace


//    YEAR   NAME        PARENT  COMPAT  MACHINE  INPUT  CLASS          INIT        COMPANY      FULLNAME                                  FLAGS
COMP( 1989,  fwu1p503ai, 0,      0,      u1pro,   0,     u1pro_state,   empty_init, "Panasonic", "FW-U1P503AI Personal Word Processor",    MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
COMP( 1996,  fwu1c70,    0,      0,      u1color, 0,     u1color_state, empty_init, "Panasonic", "FW-U1C70 Slala Personal Word Processor", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
