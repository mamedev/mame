// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Skeleton driver for Toshiba Rupo (ルポ) Personal Word Processors.

    Models list: https://dynabook.com/assistpc/rupo/page/list.htm

    Hardware
    --------

    JW-V860 (JWP2860A):

    Main Board Markings: KS-112 F8FHC1 B36017091014
    - IC1: AMD Am486 DE2-66V8TYC (x86 IA-32 CPU)
    - IC2: Toshiba TC203G14CF
    - IC4: Toshiba TC170G35AF
    - IC5: Toshiba SLA9092FF0B
    - IC19: Toshiba 5GN1 KM23V32000BG (4M Mask ROM)
    - IC21: Toshiba LHMV5GN2 (4M Mask ROM)
    - IC24: Toshiba LHMNOPN9 (8M Mask ROM)
    - IC26: Toshiba OPNA KM23C64000G (8M Mask ROM)
    - IC27: Toshiba P46008004022 (4M Mask ROM)
    - IC30,31: Fujitsu 81V16165B-60LPFTN (2 * 4M DRAM)
    - IC34: NEC D482444GW-70 (4M-Bit Dual Port Graphics Buffer)
    - IC35: National Semiconductor NS9718AH PC87312VF (SIO Floppy Disk Controller)
    - IC36: Maxim MAX3241CAI (RS-232 Transceiver)
    - IC39: Alps CBK0002ZA-PTCTS10 BC4482.1
    - IC40: RTC3511 A777363
    - IC43: National Semiconductor P74AB LVXC4245
    - IC47: AMD Am29202-16KC (29k CPU)
    - IC48: Toshiba TC170G21AF
    - IC49: Toshiba G36460330036 M531655E-03 (2M Mask ROM)
    - IC50: Oki M531655E-08 (2M Mask ROM)
    - IC51: Toshiba KFW3Z2J KM23C32000AG (4M Mask ROM)
    - IC52: Oki M514260CSL-60J (512K DRAM)
    - IC53: NEC 42S4260-60 (512K DRAM)
    - IC63: Sanyo LC83086M
    - IC64: 88346B 9722 M78 (D/A Converter)
    - IC66,67: Allegro MicroSystems A3953SLB (2 * Motor Driver)
    - IC68: Allegro MicroSystems UDN2916LB (Motor Driver)
    - IC69: LB1836 7P9 (Motor Driver)

    Floppy Drive: Mitsumi D353T5
    - Mitsumi NCL016 712 101B
    - Mitsumi NCL017 702 B31G
    - LB1813 6M4
    - LB1838 6Z6

***************************************************************************/

#include "emu.h"

#include "cpu/am29000/am29000.h"
#include "cpu/i386/i386.h"
#include "imagedev/floppy.h"
#include "machine/ram.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

#define LOG_IO (1U << 1)

//#define VERBOSE (LOG_IO)
#include "logmacro.h"


namespace {

class jwcolor_state : public driver_device
{
public:
	jwcolor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_am29k(*this, "am29k")
		, m_am486(*this, "am486")
		, m_screen(*this, "screen")
		, m_flop(*this, "fdc")
		, m_fd_softlist(*this, "fd_list")
	{ }

	void jwcolor(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_am29k;
	required_device<cpu_device> m_am486;
	required_device<screen_device> m_screen;
	required_device<floppy_connector> m_flop;
	required_device<software_list_device> m_fd_softlist;

	static void floppy_formats(format_registration &fr);

	void am29k_data_map(address_map &map) ATTR_COLD;
	void am29k_prg_map(address_map &map) ATTR_COLD;
	void am486_io_map(address_map &map) ATTR_COLD;
	void am486_prg_map(address_map &map) ATTR_COLD;
	void palette(palette_device &palette) const ATTR_COLD { }
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u16 m_io_00d0;
	u8 m_io_0398;
};

void jwcolor_state::machine_reset()
{
	// FIXME: Fatal error: Am29000 instruction MMU translation enabled!
	m_am29k->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_io_00d0 = 0;
	m_io_0398 = 0;
}

static void jwcolor_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
}

void jwcolor_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
}

void jwcolor_state::am29k_data_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom().region("ic49", 0);
}

void jwcolor_state::am29k_prg_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom().region("ic49", 0);
}

void jwcolor_state::am486_io_map(address_map &map)
{
	map(0x0398, 0x0398).lr8(
		NAME([this]() {
			if (!machine().side_effects_disabled()) {
				m_io_0398 = m_io_0398 == 0 ? 0x88 : 0;
				LOGMASKED(LOG_IO, "%s: io_0398_r = %02x\n", machine().describe_context(), m_io_0398);
			}
			return m_io_0398;
		}));
	map(0x03bd, 0x03bd).lr8(
		NAME([this]() {
			if (!machine().side_effects_disabled()) {
				LOGMASKED(LOG_IO, "%s: io_03bd_r = 0\n", machine().describe_context());
			}
			return 0;
		}));
	map(0x00d0, 0x00d1).lr16(
		NAME([this](offs_t offset) {
			if (!machine().side_effects_disabled()) {
				m_io_00d0 += 100;
				LOGMASKED(LOG_IO, "%s: io_00d0_r = %02x\n", machine().describe_context(), m_io_00d0);
			}
			return m_io_00d0;
		}));
}

void jwcolor_state::am486_prg_map(address_map &map)
{
	map(0x00000000, 0x000001ff).ram();
	map(0x00020000, 0x0002ffff).rom().region("ic19_ic21", 0x20000);
	map(0x000ffff0, 0x000fffff).mirror(0xfff00000).rom().region("ic19_ic21", 0x100);
}

u32 jwcolor_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void jwcolor_state::jwcolor(machine_config &config)
{
	AM29000(config, m_am29k, 20_MHz_XTAL);
	m_am29k->set_addrmap(AS_PROGRAM, &jwcolor_state::am29k_prg_map);
	m_am29k->set_addrmap(AS_DATA, &jwcolor_state::am29k_data_map);

	I486(config, m_am486, 66_MHz_XTAL);
	m_am486->set_addrmap(AS_PROGRAM, &jwcolor_state::am486_prg_map);
	m_am486->set_addrmap(AS_IO, &jwcolor_state::am486_io_map);

	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(640, 480);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(jwcolor_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(jwcolor_state::palette), 256);

	FLOPPY_CONNECTOR(config, m_flop, jwcolor_floppies, "35hd", jwcolor_state::floppy_formats);
	SOFTWARE_LIST(config, "fd_list").set_original("rupo_flop");
}

ROM_START( jwv860 )
	ROM_REGION32_LE(0x800000, "ic19_ic21", 0)
	ROM_LOAD32_WORD("km23v32000bg.5gn1.ic19", 0x000000, 0x400000, CRC(0961da04) SHA1(acdca402a8fdfb6b784864b9f61eb85c78d7ad64))
	ROM_LOAD32_WORD("lhmv5gn2.ic21", 0x000002, 0x400000, CRC(6bf1d1d0) SHA1(c19f38e13931af6b137462532ebdb692f727e654))

	ROM_REGION32_LE(0x800000, "ic24", 0)
	ROM_LOAD("lhmnopn9.ic24", 0x000000, 0x800000, CRC(3083f6bb) SHA1(e938342745805f7626903244b4a04bd31455d1fe))

	ROM_REGION32_LE(0x800000, "ic26", 0)
	ROM_LOAD("km23c64000g.opna.ic26", 0x000000, 0x800000, CRC(760433fa) SHA1(a7965c90efa06e00aacfd4aaa6ef0fef66075f2e))

	ROM_REGION32_LE(0x400000, "ic27", 0)
	ROM_LOAD("p46008004022.ic27", 0x000000, 0x400000, CRC(bab3ded2) SHA1(ba25df3fe2c0ba368ee793cc160d03871495038c))

	ROM_REGION32_BE(0x200000, "ic49", 0)
	ROMX_LOAD("m531655e-03.g36460330036.ic49", 0x000000, 0x200000, CRC(8218c3d8) SHA1(0fb1a7a5fe375360f57c3b4a95612bd20d3b4004), ROM_GROUPDWORD | ROM_REVERSE)

	ROM_REGION32_BE(0x200000, "ic50", 0)
	ROMX_LOAD("m531655e-08.ic50", 0x000000, 0x200000, CRC(46f5dabf) SHA1(837d59e1c546b59ab14c5218339f14bbade72557), ROM_GROUPDWORD | ROM_REVERSE)

	ROM_REGION32_BE(0x400000, "ic51", 0)
	ROM_LOAD16_WORD_SWAP("km23c32000ag.kfw3z2j.ic51", 0x000000, 0x400000, CRC(f437bd92) SHA1(50472ed58a9385f0da08efb78147c5669f679872))
ROM_END

} // anonymous namespace


//    YEAR   NAME       PARENT  COMPAT  MACHINE  INPUT  CLASS          INIT        COMPANY    FULLNAME                                FLAGS
COMP( 1997,  jwv860,    0,      0,      jwcolor, 0,     jwcolor_state, empty_init, "Toshiba", "JW-V860 Rupo Personal Word Processor", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
