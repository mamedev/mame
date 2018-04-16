// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of the Intergraph InterPro/InterServe family of CLIPPER based
 * UNIX workstations.
 *
 * The first systems were built using the original C100 CLIPPER CPU, and used
 * an additional Intel 80186 as an I/O processor, later upgraded to a C300 and
 * 80386 IOP. Around 1990, the CLIPPER became fast enough to obviate the need
 * for the separate I/O processor, and systems from that point used the main
 * CPU for both compute and I/O, along with some custom ASICs.
 *
 * Over the lifespan of the InterPro, there were five distinct families of
 * systems, varying mainly in terms of the primary CPU, as follows:
 *
 *   Year  Family    Models                   CPU
 *   1986  amethyst  32C/100/200              C100 (80186 IOP)
 *                   300                      C100 (80386 IOP)
 *   1988  topaz     3000/4000/5000           C300/C300Plus (80386 IOP)
 *   1990  emerald   6000/6100/6200/6500      C300/C300Plus
 *   1990  turquoise 2000                     C300
 *   1991  emerald   6600                     C4?
 *   1992  sapphire  2400/6400                C4T
 *   1993  sapphire  2500/2700/6700/6800      C4I
 *   1994  sapphire  2800                     C4I
 *
 * Individual models and some of their specific attributes include:
 *
 *   Model  Year  CPU    Performance           Clock   Family       Bus
 *   6000   1990  C300   10 MIPS               40MHz   emerald      SRX
 *          1991         12 MIPS
 *   6100   1990  C300+? 14 MIPS                       emerald      IOI, 12-slot
            1991         15.5 MIPS
 *   6500   1990  C300+  20 MIPS                       emerald      IOI, QWIC bus?, 12-slot
 *   6200   1990  C300+  14 MIPS               60MHz   emerald
 *          1991         18 MIPS
 *   2000   1990  C300   12.5 MIPS             50MHz?  turquoise    CBUS
 *          1991         16 MIPS
 *   6600   1991  C400   40 MIPS                       emerald      IOI, SRX bus?
 *   2400   1992  C4T    36 MIPS/33 SPECmarks  40MHz?  sapphire     CBUS
 *   6400   1992  C4T    36 MIPS/33 SPECmarks  40MHz   sapphire     SRX
 *   2700   1993  C400I  40.1 SPECmark89               sapphire 2   CBUS
 *   6700   1993  C400I  40.1 SPECmark89               sapphire 2   SRX
 *   6800   1993  C400I  67.2 SPECmark89               sapphire 3   SRX
 *   2500   1993  C400I  19.9 SPECint92                sapphire
 *   2800   1994  C400I                                sapphire 3   CBUS?
 *
 * IOI == I/O Interface (another type of bus?)
 * Sapphire 2 w/CBUS supports RETRY (maybe bus retry?)
 *
 * With some exceptions, system models are numbered ABCD, where:
 *
 *   A: case type (2=desktop, 6=minicase)
 *   B: CPU type (0=C300, 4=C4T, 6=C400?, 7/8/5 = C400I)
 *   C: graphics type (0=none, 3/5=GT, 4=EDGE-1, 8 = EDGE-2)
 *   D: usually 0, 6xxx systems have 5, 7 and 9 options (backplane type?)
 *
 * Graphics type 3 is for GT graphics fitted to a 2xxx system, and type 5 when
 * fitted to a 6xxx system. The latter is possibly also known as GTDB.
 *
 * Both the desktop and minicase units supported expansion slots with a variety
 * of cards, although with different profiles and connectors between 2xxx and
 * 6xxx systems. The bus is referred to as SR, SRX, SR Bus, CBUS and some
 * combinations of these in various places, but all appear to be compatible or
 * identical, possibly with SRX being an enhanced version. The bus supported a
 * range of add-in cards, ranging from expanded SCSI and serial controllers,
 * to specialised scanning and plotting controllers, a VME bridge, and most
 * importantly various single and dual-headed graphics boards.
 *
 * The InterPro graphics options included the GT range, generally fitted to the
 * desktops, and EDGE graphics for the 6xxx systems. Systems with no graphics
 * were operated through a serial terminal on serial port 2, and were branded
 * InterServe rather than InterPro.
 *
 *   Model     Year    Performance
 *   GT        1990?   360k 2D vec/s (in a 2020)
 *   EDGE-1            8 planes + 1 highlight plane, double buffered (6040)
 *   EDGE-2            24 bit, 400k 2D vec/s, 350k 3D vec/s (6280)
 *   GT+               500k 2D vec/s, 300k 2D vec/s (in a 2430)
 *                     760k 2D vec/s, 530k 3D vec/s (in a 2730)
 *   GTII              800k 2D vec/s, 500k 3D vec/s (in a 6450)
 *                     830k 2D vec/s, 640k 3D vec/s (in a 6750)
 *                     900k 2D vec/s, 700k 3D vec/s (in a 6850)
 *   EDGE II+          600k 2D vec/s, 500k 3D vec/s, 50k shaded poly/s (in a 6480)
 *
 * GT graphics are also referred to in various places as Memory Mapped Graphics
 * or MMG. EDGE stands for Extensible Display Geometry Engine.
 *
 * This emulation currently supports the Turquoise and Sapphire desktop systems (i.e. models
 * 2000/2400/2500/2700/2800). Base GT graphics can be used with any of these models, or the
 * graphics and keyboard removed and the systems used with a serial terminal instead.
 *
 * Key parts lists for the supported models are as follows.
 *
 * 2000 Turquoise (C300 @ 50MHz?, main board PCB962)
 *
 *   Ref   Part                      Function
 *   U37   Intel 82072               Floppy drive controller
 *   U39   Intel 82586               Ethernet controller
 *   U40   Zilog 8530 SCC            Keyboard and console serial controller
 *   U41   Zilog 8530 SCC            Serial controller for serial port 0 and 1
 *   U42   Xilinx XC3020-50          Plotter control FPGA?
 *   U43   (MPRGM610C)               Bitstream for XC3020?
 *   U54   4.9152 MHz crystal        Clock source for 8530s?
 *   U55   20.0 MHz crystal
 *   U57   24.0 MHz crystal          Clock source for 53C90A?
 *   U61   NCR 53C90A                SCSI controller
 *   U63   CIDC84607 TC110G75CY-0011 Intergraph I/O gate array?
 *   U116  Dallas DS1287             RTC and NVRAM
 *   U137?                           diagnostic 7-segment LED?
 *   U171  128 kB EPROM (MPRGM530E)  Boot ROM MSB
 *   U172  128 kB EPROM (MPRGM540E)  Boot ROM LSB
 *
 * 2400 Sapphire (C400T @ 40MHz?, main board SMT047)
 *
 *   Ref   Part                      Function
 *   U31   Zilog Z85C30 SCC          Keyboard and console serial controller
 *   U32   Zilog Z85230 ESCC         Serial controller for serial port 0 and 1
 *   U34   Xilinx XC3020-50          Plotter control FPGA?
 *   U35   128 kB EPROM (MPRGW510B)  Boot ROM
 *   U43?  (MPRGM610P)               Bitstream for XC3020?
 *   U44   Intel 82596SX-20          Ethernet controller
 *   U67   Intel N28F010-200         128Kx8 flash memory (Y226 0B03 0592)
 *   U68   CYID21603 TC150G89AF
 *   U71   LSI L1A6104 CICD 95801    Intergraph I/O gate array
 *   U76   Intel N28F010-200         128Kx8 flash memory (Y225 0B?? 27??)
 *   U81   NCR 53C94                 SCSI controller
 *   U86   24.0 MHz crystal          Clock source for 53C94?
 *   U87   4.9152 MHz crystal        Clock source for 8530s?
 *   U88   20.0 MHz crystal          Clock source for 82596?
 *   U91   Intel N82077AA-1          Floppy drive controller
 *   U96   32.0 MHz crystal
 *   U97   40.0 MHz crystal
 *   U112? (MPRG4230A)               node ID prom?
 *   U113? Dallas DS1287             RTC and NVRAM
 *   U117?                           diagnostic 7-segment LED?
 *   U118? (MPRG X510R)
 *   U155  CYID21704 TC140G54AF
 *
 * 2700 Sapphire (C400I @ 70MHz?, main board SMT128)
 *
 *   Ref   Part                      Function
 *   U31   Zilog Z85C30 SCC          Keyboard and console serial controller
 *   U32   Zilog Z85230 ESCC         Serial controller for serial port 0 and 1
 *   U34   Xilinx XC3020-70          Plotter control FPGA?
 *   U35   128 kB EPROM (MPRGZ530A)  Boot ROM
 *   U43?  (MPRGM610P)               Bitstream for XC3020?
 *   U44   Intel 82596SX-20          Ethernet controller
 *   U68   CYID21603 TC150G89AF
 *   U67   Intel N28F010             128Kx8 flash memory (Y226 0C30 4291)
 *   U71   LSI L1A7374 CIDC094A3     Intergraph I/O gate array
 *   U76   Intel N28F010             128Kx8 flash memory (Y225 0C30 4220)
 *   U81   NCR 53C94                 SCSI controller
 *   U86   24.0 MHz crystal          Clock source for 82077
 *   U87   4.9152 MHz crystal        Clock source for 8530s?
 *   U88   20.0 MHz crystal          Clock source for 82596?
 *   U91   Intel N82077SL-1          Floppy drive controller
 *   U96   29.0 MHz crystal
 *   U97   40.0 MHz crystal
 *   U112? (MPRGZ260E)               node ID prom?
 *   U113  Dallas DS12887            RTC and NVRAM
 *   U117?                           diagnostic 7-segment LED?
 *   U118? ()
 *   U155  CYID212?4 TC140G54AF?
 *
 * CPU daughter-boards
 *   PCB824 Rev J - 2000 (also has label MPCBA5507)
 *   SMT082 (MSMT0820B, 36MHz?) - 6400 (SMT046 "6400 36-MHz Series System Board")
 *   SMT03? 2400/6400 - (MSMT03804 -> rev 2 cammu, goes with "6400 36-MHz Series System Board", MSMT0380A eco 3+ use rev 3 cammu)
 *   SMT019 (MSMT019 C4E CPU Assembly)
 *   SMT104 Rev A - 2700/6700 (aka MSMT1040A "C4I CPU", C4 CPU REV 3 + C4 FPU REV 3 + C4I CAMMU)
 *
 *   PCB962   2000 System Board                  MPCB824 C300
 *   SMT046   6400 36-MHz Series System Board    MSMT03804 rev 2 CAMMU/30MHz Kryptonite Rev 3 CAMMU/32MHz Kryptonite Rev 3 CAMMU/MSMT0820B(36MHz)
 *   SMT047   2400 Series System Board           MSMT03804 rev 2 CAMMU/MSMT0380A eco 3+ Rev 3 CAMMU
 *   SMT098A  6400 32-MHz Sapphire System Board
 *   SMT098B  6400 32-MHz Sapphire System Board
 *   SMT127   6700 Series System Board           MSMT1040A C4I: C4 CPU Rev 3 + C4 FPU Rev 3 + C4I CAMMU
 *   SMT128   2700 Series System Board           MSMT1040A C4I: C4 CPU Rev 3 + C4 FPU Rev 3 + C4I CAMMU
 *   SMT144   6800 Series System Board           integrated cpu?
 *   SMT145   2800 Series System Board
 */

#include "emu.h"

#include "includes/interpro.h"

#include "debugger.h"

#include "interpro.lh"

#define VERBOSE 0
#include "logmacro.h"

void interpro_state::machine_start()
{
	m_led.resolve();

	// FIXME: disabled for now to avoid cold start diagnostic errors
	//m_sreg_ctrl2 = CTRL2_COLDSTART | CTRL2_PWRUP;
	m_sreg_ctrl2 = 0;

	save_item(NAME(m_sreg_error));
	save_item(NAME(m_sreg_status));
	save_item(NAME(m_sreg_led));
	save_item(NAME(m_sreg_ctrl1));
	save_item(NAME(m_sreg_ctrl2));

	save_item(NAME(m_sreg_ctrl3));
}

void interpro_state::machine_reset()
{
	// flash rom requires the following values
	m_sreg_error = 0;
	m_sreg_status = 0x400;
	m_sreg_ctrl1 = CTRL1_FLOPLOW;
}

DRIVER_INIT_MEMBER(interpro_state, common)
{
	// FIXME: not all memory sizes are reported properly using fdm "5 inqhw" and
	// "optimum_memory" commands

	// 16 = reports 16M, banks empty?
	// 32 = reports 16M, banks empty?
	// 64 = reports 128M, 16x8
	// 128 = reports 128M, 16x8
	// 256 = reports 256M, 32x8

	// map the configured ram
	m_maincpu->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());
}

WRITE16_MEMBER(interpro_state::sreg_led_w)
{
	// 7-segment decode patterns (hex digits) borrowed from wico.cpp (mc14495)
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 };

	m_led = patterns[data & 0xf];

	COMBINE_DATA(&m_sreg_led);
}

WRITE16_MEMBER(interpro_state::sreg_ctrl1_w)
{
	LOG("control register 1 data 0x%04x mem_mask 0x%04x (%s)\n", data, mem_mask, machine().describe_context());

	// check if led decimal point changes state
	if ((data ^ m_sreg_ctrl1) & CTRL1_LEDDP)
		m_led = (m_led + 0x80) & 0xff;

	COMBINE_DATA(&m_sreg_ctrl1);
}

WRITE16_MEMBER(interpro_state::sreg_ctrl2_w)
{
	LOG("control register 2 data 0x%04x mem_mask 0x%04x (%s)\n", data, mem_mask, machine().describe_context());
	if (data & CTRL2_RESET)
	{
		m_sreg_ctrl2 &= ~CTRL2_COLDSTART;

		machine().schedule_soft_reset();
	}
	else
		m_sreg_ctrl2 = (m_sreg_ctrl2 & ~CTRL2_MASK) | (data & CTRL2_MASK);
}

WRITE16_MEMBER(sapphire_state::sreg_ctrl2_w)
{
	interpro_state::sreg_ctrl2_w(space, offset, data, mem_mask);

	// enable/disable programming power on both flash devices
	m_flash_lo->vpp(data & CTRL2_FLASHEN ? ASSERT_LINE : CLEAR_LINE);
	m_flash_hi->vpp(data & CTRL2_FLASHEN ? ASSERT_LINE : CLEAR_LINE);
}

READ16_MEMBER(interpro_state::sreg_error_r)
{
	const u16 result = m_sreg_error;

	// clear error register on read
	if (!machine().side_effects_disabled())
		m_sreg_error = 0;

	return result;
}

READ32_MEMBER(sapphire_state::unmapped_r)
{
	// check if non-existent memory errors are enabled
	if (!machine().side_effects_disabled())
		if (m_arbga->tctrl_r(space, offset, mem_mask) & interpro_arbga_device::TCTRL_ENNEM)
		{
			// flag non-existent memory error in system error register
			m_sreg_error |= (ERROR_SRXNEM | ERROR_SRXVALID);

			// tell ioga to raise a bus error
			m_ioga->bus_error(space, interpro_ioga_device::BINFO_BERR | interpro_ioga_device::BINFO_SNAPOK, offset << 2);
		}

	return space.unmap();
}

WRITE32_MEMBER(sapphire_state::unmapped_w)
{
	// check if non-existent memory errors are enabled
	if (m_arbga->tctrl_r(space, offset, mem_mask) & interpro_arbga_device::TCTRL_ENNEM)
	{
		// flag non-existent memory error in system error register
		m_sreg_error |= (ERROR_SRXNEM | ERROR_SRXVALID);

		// tell ioga to raise a bus error
		m_ioga->bus_error(space, interpro_ioga_device::BINFO_BERR | interpro_ioga_device::BINFO_SNAPOK, offset << 2);
	}
}

READ8_MEMBER(interpro_state::nodeid_r)
{
	// FIXME: hard coded node id for now
	switch (offset)
	{
		// read system node id prom (contains last 3 bytes of mac address)
	case 0: return 0x12;
	case 1: return 0x34;
	case 2: return 0x56;
	case 3: return 0x9c; // checksum - sum of other bytes
	}

	return space.unmap();
}

void interpro_state::interpro_common_map(address_map &map)
{
	// FIXME: don't know what this range is for
	map(0x08000000, 0x08000fff).noprw();

	map(0x4f007e00, 0x4f007eff).m(m_sga, FUNC(interpro_sga_device::map));

	map(0x7f000100, 0x7f00011f).m(m_fdc, FUNC(upd765_family_device::map)).umask32(0x000000ff);
	map(0x7f000300, 0x7f000301).r(this, FUNC(interpro_state::sreg_error_r));
	map(0x7f000304, 0x7f000305).rw(this, FUNC(interpro_state::sreg_status_r), FUNC(interpro_state::sreg_led_w));
	map(0x7f000308, 0x7f000309).rw(this, FUNC(interpro_state::sreg_ctrl1_r), FUNC(interpro_state::sreg_ctrl1_w));
	map(0x7f00030c, 0x7f00030d).rw(this, FUNC(interpro_state::sreg_ctrl2_r), FUNC(interpro_state::sreg_ctrl2_w));

	map(0x7f00031c, 0x7f00031d).rw(this, FUNC(interpro_state::sreg_ctrl3_r), FUNC(interpro_state::sreg_ctrl3_w));

	map(0x7f000400, 0x7f00040f).rw(m_scc1, FUNC(z80scc_device::ba_cd_inv_r), FUNC(z80scc_device::ba_cd_inv_w)).umask32(0x000000ff);
	map(0x7f000410, 0x7f00041f).rw(m_scc2, FUNC(z80scc_device::ba_cd_inv_r), FUNC(z80scc_device::ba_cd_inv_w)).umask32(0x000000ff);
	map(0x7f000500, 0x7f000503).lrw8("rtc_rw",
									 [this](address_space &space, offs_t offset, u8 mem_mask) {
										 return m_rtc->read(space, offset^1, mem_mask);
									 },
									 [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) {
										 m_rtc->write(space, offset^1, data, mem_mask);
									 }).umask32(0x000000ff);
	map(0x7f000600, 0x7f000600).w(m_rtc, FUNC(mc146818_device::write));

	// the system board id prom
	map(0x7f000700, 0x7f00077f).rom().region(INTERPRO_IDPROM_TAG, 0);

	map(0x7f0fff00, 0x7f0fffff).m(m_ioga, FUNC(interpro_ioga_device::map));
}

void turquoise_state::turquoise_base_map(address_map &map)
{
	interpro_common_map(map);

	map(0x40000000, 0x4000003f).m(m_mcga, FUNC(interpro_mcga_device::map));

	// scsi registers have unusual address mapping
	map(0x7f000201, 0x7f000201).rw(m_scsi, FUNC(ncr53c90a_device::tcounter_lo_r), FUNC(ncr53c90a_device::tcount_lo_w));
	map(0x7f000205, 0x7f000205).rw(m_scsi, FUNC(ncr53c90a_device::tcounter_hi_r), FUNC(ncr53c90a_device::tcount_hi_w));
	map(0x7f000209, 0x7f000209).rw(m_scsi, FUNC(ncr53c90a_device::fifo_r), FUNC(ncr53c90a_device::fifo_w));
	map(0x7f00020d, 0x7f00020d).rw(m_scsi, FUNC(ncr53c90a_device::command_r), FUNC(ncr53c90a_device::command_w));
	map(0x7f000211, 0x7f000211).rw(m_scsi, FUNC(ncr53c90a_device::status_r), FUNC(ncr53c90a_device::bus_id_w));
	map(0x7f000215, 0x7f000215).rw(m_scsi, FUNC(ncr53c90a_device::istatus_r), FUNC(ncr53c90a_device::timeout_w));
	map(0x7f000219, 0x7f000219).rw(m_scsi, FUNC(ncr53c90a_device::seq_step_r), FUNC(ncr53c90a_device::sync_period_w));
	map(0x7f00021d, 0x7f00021d).rw(m_scsi, FUNC(ncr53c90a_device::fifo_flags_r), FUNC(ncr53c90a_device::sync_offset_w));
	map(0x7f000221, 0x7f000221).rw(m_scsi, FUNC(ncr53c90a_device::conf_r), FUNC(ncr53c90a_device::conf_w));
	map(0x7f000225, 0x7f000225).w(m_scsi, FUNC(ncr53c90a_device::clock_w));
	map(0x7f000229, 0x7f000229).w(m_scsi, FUNC(ncr53c90a_device::test_w));
	map(0x7f00022d, 0x7f00022d).rw(m_scsi, FUNC(ncr53c90a_device::conf2_r), FUNC(ncr53c90a_device::conf2_w));

	map(0x7f000300, 0x7f000300).w(this, FUNC(turquoise_state::sreg_error_w));

	map(0x7f000600, 0x7f00067f).rom().region(INTERPRO_NODEID_TAG, 0);
}

void turquoise_state::turquoise_main_map(address_map &map)
{
	turquoise_base_map(map);

	map(0x00000000, 0x00ffffff).ram().share(RAM_TAG);
	map(0x7f100000, 0x7f13ffff).rom().region(INTERPRO_EPROM_TAG, 0);
}

void sapphire_state::sapphire_base_map(address_map &map)
{
	interpro_common_map(map);

	map(0x40000000, 0x4000004f).m(INTERPRO_MCGA_TAG, FUNC(interpro_fmcc_device::map));
	map(0x7f000200, 0x7f0002ff).m(m_arbga, FUNC(interpro_arbga_device::map));

	map(0x7f000600, 0x7f00060f).r(this, FUNC(sapphire_state::nodeid_r)).umask32(0x000000ff);

	// scsi registers have unusual address mapping
	map(0x7f001001, 0x7f001001).rw(m_scsi, FUNC(ncr53c94_device::tcounter_lo_r), FUNC(ncr53c94_device::tcount_lo_w));
	map(0x7f001101, 0x7f001101).rw(m_scsi, FUNC(ncr53c94_device::tcounter_hi_r), FUNC(ncr53c94_device::tcount_hi_w));
	map(0x7f001201, 0x7f001201).rw(m_scsi, FUNC(ncr53c94_device::fifo_r), FUNC(ncr53c94_device::fifo_w));
	map(0x7f001301, 0x7f001301).rw(m_scsi, FUNC(ncr53c94_device::command_r), FUNC(ncr53c94_device::command_w));
	map(0x7f001401, 0x7f001401).rw(m_scsi, FUNC(ncr53c94_device::status_r), FUNC(ncr53c94_device::bus_id_w));
	map(0x7f001501, 0x7f001501).rw(m_scsi, FUNC(ncr53c94_device::istatus_r), FUNC(ncr53c94_device::timeout_w));
	map(0x7f001601, 0x7f001601).rw(m_scsi, FUNC(ncr53c94_device::seq_step_r), FUNC(ncr53c94_device::sync_period_w));
	map(0x7f001701, 0x7f001701).rw(m_scsi, FUNC(ncr53c94_device::fifo_flags_r), FUNC(ncr53c94_device::sync_offset_w));
	map(0x7f001801, 0x7f001801).rw(m_scsi, FUNC(ncr53c94_device::conf_r), FUNC(ncr53c94_device::conf_w));
	map(0x7f001901, 0x7f001901).w(m_scsi, FUNC(ncr53c94_device::clock_w));
	map(0x7f001a01, 0x7f001a01).w(m_scsi, FUNC(ncr53c94_device::test_w));
	map(0x7f001b01, 0x7f001b01).rw(m_scsi, FUNC(ncr53c94_device::conf2_r), FUNC(ncr53c94_device::conf2_w));
	map(0x7f001c01, 0x7f001c01).w(m_scsi, FUNC(ncr53c94_device::conf3_w));
	map(0x7f001f01, 0x7f001f01).w(m_scsi, FUNC(ncr53c94_device::fifo_align_w));
}

void sapphire_state::sapphire_main_map(address_map &map)
{
	sapphire_base_map(map);

	map(0x00000000, 0x00ffffff).ram().share(RAM_TAG);
	map(0x7f100000, 0x7f11ffff).rom().region(INTERPRO_EPROM_TAG, 0);
	map(0x7f180000, 0x7f1fffff).rw(INTERPRO_FLASH_TAG "_lo", FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).umask32(0x00ff00ff).mask(0x3ffff);
	map(0x7f180000, 0x7f1fffff).rw(INTERPRO_FLASH_TAG "_hi", FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).umask32(0xff00ff00).mask(0x3ffff);
}

void turquoise_state::turquoise_io_map(address_map &map)
{
	turquoise_base_map(map);

	map(0x00000800, 0x000009ff).m(INTERPRO_MMU_TAG "_d", FUNC(cammu_c3_device::map));
	map(0x00000a00, 0x00000bff).m(INTERPRO_MMU_TAG "_i", FUNC(cammu_c3_device::map));
	map(0x00000c00, 0x00000dff).m(INTERPRO_MMU_TAG "_d", FUNC(cammu_c3_device::map_global));
}

void sapphire_state::sapphire_io_map(address_map &map)
{
	sapphire_base_map(map);

	map(0x00000000, 0x00001fff).m(m_mmu, FUNC(cammu_c4_device::map));
}

void interpro_state::interpro_boot_map(address_map &map)
{
	// FIXME: the real system may have some initial boot instructions in this boot
	// memory space which jump to the start of the boot eprom code, or there may
	// be some special address decoding logic for boot. For now, we fake it in the
	// CPU by hard-coding the start address to point at the eprom.
	map(0x00000000, 0x00001fff).ram();
}

void turquoise_state::interpro_82586_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).rw(INTERPRO_IOGA_TAG, FUNC(turquoise_ioga_device::eth_r), FUNC(turquoise_ioga_device::eth_w));
}

void sapphire_state::interpro_82596_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(INTERPRO_IOGA_TAG, FUNC(sapphire_ioga_device::eth_r), FUNC(sapphire_ioga_device::eth_w));
}

FLOPPY_FORMATS_MEMBER(interpro_state::floppy_formats)
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START(interpro_floppies)
	SLOT_INTERFACE("525dd", FLOPPY_525_DD)
	SLOT_INTERFACE("35hd", FLOPPY_35_HD)
SLOT_INTERFACE_END

MACHINE_CONFIG_START(interpro_state::interpro_scc1)
	MCFG_DEVICE_MODIFY(INTERPRO_SCC1_TAG)
	MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE(INTERPRO_SERIAL_PORT1_TAG, rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_TXDB_CB(DEVWRITELINE(INTERPRO_SERIAL_PORT2_TAG, rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_INT_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir11_w))
	MCFG_Z80SCC_OUT_WREQA_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, drq_serial1))
	MCFG_Z80SCC_OUT_WREQB_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, drq_serial2))

	MCFG_RS232_PORT_ADD(INTERPRO_SERIAL_PORT1_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, ctsa_w))

	MCFG_RS232_PORT_ADD(INTERPRO_SERIAL_PORT2_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, ctsb_w))
MACHINE_CONFIG_END

MACHINE_CONFIG_START(interpro_state::interpro_scc2)
	MCFG_DEVICE_MODIFY(INTERPRO_SCC2_TAG)
	MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE(INTERPRO_KEYBOARD_PORT_TAG, interpro_keyboard_port_device, write_txd))
	MCFG_Z80SCC_OUT_TXDB_CB(DEVWRITELINE(INTERPRO_SERIAL_PORT0_TAG, rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_INT_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir11_w))
	MCFG_Z80SCC_OUT_WREQB_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, drq_serial0))

	MCFG_INTERPRO_KEYBOARD_PORT_ADD(INTERPRO_KEYBOARD_PORT_TAG, interpro_keyboard_devices, "hle_en_us")
	MCFG_INTERPRO_KEYBOARD_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC2_TAG, z80scc_device, rxa_w))

	MCFG_RS232_PORT_ADD(INTERPRO_SERIAL_PORT0_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC2_TAG, z80scc_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INTERPRO_SCC2_TAG, z80scc_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INTERPRO_SCC2_TAG, z80scc_device, ctsb_w))
MACHINE_CONFIG_END

static SLOT_INTERFACE_START(interpro_scsi_devices)
	SLOT_INTERFACE("harddisk", NSCSI_HARDDISK)
	SLOT_INTERFACE("cdrom", NSCSI_CDROM)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(turquoise_scsi_devices)
	SLOT_INTERFACE_INTERNAL(INTERPRO_SCSI_ADAPTER_TAG, NCR53C90A)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START(sapphire_scsi_devices)
	SLOT_INTERFACE_INTERNAL(INTERPRO_SCSI_ADAPTER_TAG, NCR53C94)
SLOT_INTERFACE_END

void interpro_state::interpro_scsi_adapter(device_t *device)
{
	devcb_base *devcb;
	(void)devcb;
	MCFG_DEVICE_CLOCK(XTAL(24'000'000))
	MCFG_NCR5390_IRQ_HANDLER(DEVWRITELINE(":" INTERPRO_IOGA_TAG, interpro_ioga_device, ir0_w))
	MCFG_NCR5390_DRQ_HANDLER(DEVWRITELINE(":" INTERPRO_IOGA_TAG, interpro_ioga_device, drq_scsi))
}

MACHINE_CONFIG_START(interpro_state::ioga)
	MCFG_DEVICE_MODIFY(INTERPRO_IOGA_TAG)
	MCFG_INTERPRO_IOGA_NMI_CB(INPUTLINE(INTERPRO_CPU_TAG, INPUT_LINE_NMI))
	MCFG_INTERPRO_IOGA_IRQ_CB(INPUTLINE(INTERPRO_CPU_TAG, INPUT_LINE_IRQ0))
	MCFG_INTERPRO_IOGA_IVEC_CB(DEVWRITE8(INTERPRO_CPU_TAG, clipper_device, set_ivec))

	// ioga dma and serial dma channels
	// TODO: check serial dma channels - scc2chanA (keyboard) has no dma
	//MCFG_INTERPRO_IOGA_DMA_CB(0, unknown) // plotter
	MCFG_INTERPRO_IOGA_DMA_CB(1, DEVREAD8(INTERPRO_SCSI_DEVICE_TAG, ncr53c90a_device, mdma_r), DEVWRITE8(INTERPRO_SCSI_DEVICE_TAG, ncr53c90a_device, mdma_w))
	MCFG_INTERPRO_IOGA_DMA_CB(2, DEVREAD8(INTERPRO_FDC_TAG, upd765_family_device, mdma_r), DEVWRITE8(INTERPRO_FDC_TAG, upd765_family_device, mdma_w))
	MCFG_INTERPRO_IOGA_SERIAL_DMA_CB(0, DEVREAD8(INTERPRO_SCC2_TAG, z80scc_device, db_r), DEVWRITE8(INTERPRO_SCC2_TAG, z80scc_device, db_w))
	MCFG_INTERPRO_IOGA_SERIAL_DMA_CB(1, DEVREAD8(INTERPRO_SCC1_TAG, z80scc_device, da_r), DEVWRITE8(INTERPRO_SCC1_TAG, z80scc_device, da_w))
	MCFG_INTERPRO_IOGA_SERIAL_DMA_CB(2, DEVREAD8(INTERPRO_SCC1_TAG, z80scc_device, db_r), DEVWRITE8(INTERPRO_SCC1_TAG, z80scc_device, db_w))

	// ioga floppy terminal count, ethernet channel attention
	MCFG_INTERPRO_IOGA_FDCTC_CB(DEVWRITELINE(INTERPRO_FDC_TAG, upd765_family_device, tc_line_w))
	MCFG_INTERPRO_IOGA_ETH_CA_CB(DEVWRITELINE(INTERPRO_ETH_TAG, i82586_base_device, ca))
MACHINE_CONFIG_END

static INPUT_PORTS_START(interpro)
INPUT_PORTS_END

MACHINE_CONFIG_START(interpro_state::interpro)
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16M")
	MCFG_RAM_EXTRA_OPTIONS("32M,64M,128M,256M")

	// memory control gate array

	// srx gate array
	MCFG_DEVICE_ADD(INTERPRO_SGA_TAG, INTERPRO_SGA, 0)
	MCFG_INTERPRO_SGA_BERR_CB(DEVWRITE32(INTERPRO_IOGA_TAG, interpro_ioga_device, bus_error))

	// floppy

	// serial

	// real-time clock/non-volatile memory
	MCFG_MC146818_ADD(INTERPRO_RTC_TAG, XTAL(32'768))
	MCFG_MC146818_UTC(true)
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir9_w))

	// scsi
	MCFG_NSCSI_BUS_ADD(INTERPRO_SCSI_TAG)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":0", interpro_scsi_devices, "harddisk", false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":1", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":2", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":3", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":4", interpro_scsi_devices, "cdrom", false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":5", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":6", interpro_scsi_devices, nullptr, false)

	// ethernet

	// i/o gate array

	// sr bus and slots
	MCFG_DEVICE_ADD(INTERPRO_SRBUS_TAG, SR, 0)
	MCFG_SR_OUT_IRQ0_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir6_w))
	MCFG_SR_SLOT_ADD(INTERPRO_SRBUS_TAG, INTERPRO_SRBUS_TAG ":0", sr_cards, "mpcb963", false)
	MCFG_SR_SLOT_ADD(INTERPRO_SRBUS_TAG, INTERPRO_SRBUS_TAG ":1", sr_cards, nullptr, false)

	// system layout
	MCFG_DEFAULT_LAYOUT(layout_interpro)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("softlist", "interpro")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(turquoise_state::turquoise)
	interpro(config);
	MCFG_CPU_ADD(INTERPRO_CPU_TAG, CLIPPER_C300, XTAL(12'500'000))
	MCFG_DEVICE_ADDRESS_MAP(0, turquoise_main_map)
	MCFG_DEVICE_ADDRESS_MAP(1, turquoise_io_map)
	MCFG_DEVICE_ADDRESS_MAP(2, interpro_boot_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(INTERPRO_IOGA_TAG, interpro_ioga_device, acknowledge_interrupt)

	MCFG_DEVICE_ADD(INTERPRO_MMU_TAG "_i", CAMMU_C3, 0)
	MCFG_CAMMU_EXCEPTION_CB(DEVWRITE16(INTERPRO_CPU_TAG, clipper_device, set_exception))

	MCFG_DEVICE_ADD(INTERPRO_MMU_TAG "_d", CAMMU_C3, 0)
	MCFG_CAMMU_EXCEPTION_CB(DEVWRITE16(INTERPRO_CPU_TAG, clipper_device, set_exception))
	MCFG_CAMMU_LINK(INTERPRO_MMU_TAG "_i")

	// boot fails memory test without this
	MCFG_DEVICE_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_VALUE(0x00)

	// memory control gate array
	MCFG_DEVICE_ADD(INTERPRO_MCGA_TAG, INTERPRO_MCGA, 0)

	// floppy controller
	MCFG_I82072_ADD(INTERPRO_FDC_TAG, false)
	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir1_w))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, drq_floppy))

	// connect a 3.5" drive at id 3
	MCFG_DEVICE_ADD("fdc:0", FLOPPY_CONNECTOR, 0)
	MCFG_DEVICE_ADD("fdc:1", FLOPPY_CONNECTOR, 0)
	MCFG_DEVICE_ADD("fdc:2", FLOPPY_CONNECTOR, 0)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", interpro_floppies, "35hd", interpro_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(false)

	// serial controllers and ports
	MCFG_SCC85C30_ADD(INTERPRO_SCC1_TAG, XTAL(4'915'200), 0, 0, 0, 0)
	interpro_scc1(config);
	MCFG_SCC85C30_ADD(INTERPRO_SCC2_TAG, XTAL(4'915'200), 0, 0, 0, 0)
	interpro_scc2(config);

	// scsi controller
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":7", turquoise_scsi_devices, INTERPRO_SCSI_ADAPTER_TAG, true)
	MCFG_DEVICE_CARD_MACHINE_CONFIG(INTERPRO_SCSI_ADAPTER_TAG, interpro_scsi_adapter)

	// ethernet controller
	MCFG_DEVICE_ADD(INTERPRO_ETH_TAG, I82586, 0)
	MCFG_I82586_IRQ_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir12_w))
	MCFG_DEVICE_ADDRESS_MAP(0, interpro_82586_map)

	// i/o gate array
	MCFG_DEVICE_ADD(INTERPRO_IOGA_TAG, TURQUOISE_IOGA, 0)
	MCFG_INTERPRO_IOGA_MEMORY(INTERPRO_CPU_TAG, 0)
	ioga(config);

	MCFG_DEVICE_MODIFY(INTERPRO_SRBUS_TAG)
	MCFG_SR_MEMORY(INTERPRO_CPU_TAG, 0, 1)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sapphire_state::sapphire)
	interpro(config);
	MCFG_CPU_ADD(INTERPRO_CPU_TAG, CLIPPER_C400, XTAL(12'500'000))
	MCFG_DEVICE_ADDRESS_MAP(0, sapphire_main_map)
	MCFG_DEVICE_ADDRESS_MAP(1, sapphire_io_map)
	MCFG_DEVICE_ADDRESS_MAP(2, interpro_boot_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(INTERPRO_IOGA_TAG, interpro_ioga_device, acknowledge_interrupt)

	// FIXME: 2400/6400 should be C4T cammu?
	MCFG_DEVICE_ADD(INTERPRO_MMU_TAG, CAMMU_C4I, 0)
	MCFG_CAMMU_EXCEPTION_CB(DEVWRITE16(INTERPRO_CPU_TAG, clipper_device, set_exception))

	// memory control gate array
	MCFG_DEVICE_ADD(INTERPRO_MCGA_TAG, INTERPRO_FMCC, 0)

	// floppy controller
	MCFG_N82077AA_ADD(INTERPRO_FDC_TAG, n82077aa_device::MODE_PS2)
	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir1_w))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, drq_floppy))

	// connect a 3.5" drive at id 1
	MCFG_DEVICE_ADD("fdc:0", FLOPPY_CONNECTOR, 0)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", interpro_floppies, "35hd", interpro_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(false)

	// srx arbiter gate array
	MCFG_DEVICE_ADD(INTERPRO_ARBGA_TAG, INTERPRO_ARBGA, 0)

	// serial controllers and ports
	MCFG_SCC85230_ADD(INTERPRO_SCC1_TAG, XTAL(4'915'200), 0, 0, 0, 0)
	interpro_scc1(config);
	MCFG_SCC85C30_ADD(INTERPRO_SCC2_TAG, XTAL(4'915'200), 0, 0, 0, 0)
	interpro_scc2(config);

	// scsi controller
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":7", sapphire_scsi_devices, INTERPRO_SCSI_ADAPTER_TAG, true)
	MCFG_DEVICE_CARD_MACHINE_CONFIG(INTERPRO_SCSI_ADAPTER_TAG, interpro_scsi_adapter)

	// ethernet controller
	MCFG_DEVICE_ADD(INTERPRO_ETH_TAG, I82596_LE16, XTAL(20'000'000))
	MCFG_I82586_IRQ_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir12_w))
	MCFG_DEVICE_ADDRESS_MAP(0, interpro_82596_map)

	// i/o gate array
	MCFG_DEVICE_ADD(INTERPRO_IOGA_TAG, SAPPHIRE_IOGA, 0)
	MCFG_INTERPRO_IOGA_MEMORY(INTERPRO_CPU_TAG, 0)
	ioga(config);

	// flash memory
	MCFG_DEVICE_ADD(INTERPRO_FLASH_TAG "_lo", INTEL_28F010, 0)
	MCFG_DEVICE_ADD(INTERPRO_FLASH_TAG "_hi", INTEL_28F010, 0)

	// sr bus address spaces
	MCFG_DEVICE_MODIFY(INTERPRO_SRBUS_TAG)
	MCFG_SR_MEMORY(INTERPRO_CPU_TAG, 0, 1)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(turquoise_state::ip2000)
	turquoise(config);
	//MCFG_DEVICE_MODIFY(INTERPRO_CPU_TAG)
	//MCFG_DEVICE_CLOCK(XTAL(40'000'000))

	MCFG_SOFTWARE_LIST_FILTER("softlist", "2000")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sapphire_state::ip2400)
	sapphire(config);
	//MCFG_DEVICE_MODIFY(INTERPRO_CPU_TAG)
	//MCFG_DEVICE_CLOCK(XTAL(50'000'000))

	MCFG_DEVICE_MODIFY(INTERPRO_MMU_TAG)
	MCFG_CAMMU_ID(cammu_c4i_device::CID_C4IR0)

	MCFG_DEVICE_MODIFY(INTERPRO_SRBUS_TAG ":0")
	MCFG_SLOT_DEFAULT_OPTION("mpcb070")

	MCFG_SOFTWARE_LIST_FILTER("softlist", "2400")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sapphire_state::ip2500)
	sapphire(config);
	//MCFG_DEVICE_MODIFY(INTERPRO_CPU_TAG)
	//MCFG_DEVICE_CLOCK(XTAL(?)

	// FIXME: don't know which cammu revision
	MCFG_DEVICE_MODIFY(INTERPRO_MMU_TAG)
	MCFG_CAMMU_ID(cammu_c4i_device::CID_C4IR0)

	MCFG_SOFTWARE_LIST_FILTER("softlist", "2500")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sapphire_state::ip2700)
	sapphire(config);
	//MCFG_DEVICE_MODIFY(INTERPRO_CPU_TAG)
	//MCFG_DEVICE_CLOCK(?)

	MCFG_DEVICE_MODIFY(INTERPRO_MMU_TAG)
	MCFG_CAMMU_ID(cammu_c4i_device::CID_C4IR2)

	MCFG_SOFTWARE_LIST_FILTER("softlist", "2700")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(sapphire_state::ip2800)
	sapphire(config);
	//MCFG_DEVICE_MODIFY(INTERPRO_CPU_TAG)
	//MCFG_DEVICE_CLOCK(?)

	// FIXME: don't know which cammu revision
	MCFG_DEVICE_MODIFY(INTERPRO_MMU_TAG)
	MCFG_CAMMU_ID(cammu_c4i_device::CID_C4IR2)

	MCFG_SOFTWARE_LIST_FILTER("softlist", "2800")
MACHINE_CONFIG_END

ROM_START(ip2000)
	ROM_REGION(0x80, INTERPRO_NODEID_TAG, 0)
	ROM_LOAD32_BYTE("nodeid.bin", 0x0, 0x20, CRC(a38397a6) SHA1(9f45fb932bbe231c95b3d5470dcd1fa1c846486f))

	ROM_REGION(0x80, INTERPRO_IDPROM_TAG, 0)
	ROM_LOAD32_BYTE("mpcb962a.bin", 0x0, 0x20, CRC(e391342c) SHA1(02e03aad760b6651b8599c3a41c7c457983ee97d))

	ROM_REGION(0x0040000, INTERPRO_EPROM_TAG, 0)
	ROM_SYSTEM_BIOS(0, "ip2000", "InterPro 2000 EPROM")
	ROMX_LOAD("mprgm530e__26_apr_91k.u171", 0x00001, 0x20000, CRC(e4c470cb) SHA1(ff1917bfa963988d739a9dbf0b8f034fe49f2f8c), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("mprgm540e__06_may_91k.u172", 0x00000, 0x20000, CRC(03225843) SHA1(03cfcd5b8ae0057240ef808a40108cb5d082eb63), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

ROM_START(ip2400)
	ROM_REGION(0x80, INTERPRO_IDPROM_TAG, 0)
	ROM_LOAD32_BYTE("msmt0470.bin", 0x0, 0x20, CRC(498c80df) SHA1(18a49732ac9d04b20a77747c1b946c2e88abb087))

	ROM_REGION(0x0020000, INTERPRO_EPROM_TAG, 0)
	ROM_SYSTEM_BIOS(0, "ip2400", "InterPro 2400 EPROM")
	ROMX_LOAD("mprgw510b__05_16_92.u35", 0x00000, 0x20000, CRC(3b2c4545) SHA1(4e4c98d1cd1035a04be8527223f44d0b687ec3ef), ROM_BIOS(1))

	ROM_REGION(0x20000, INTERPRO_FLASH_TAG "_lo", 0)
	ROM_LOAD_OPTIONAL("y225.u76", 0x00000, 0x20000, CRC(46c0b105) SHA1(7c4a104e4fb3d0e5e8db7c911cdfb3f5c4fb0218))

	ROM_REGION(0x20000, INTERPRO_FLASH_TAG "_hi", 0)
	ROM_LOAD_OPTIONAL("y226.u67", 0x00000, 0x20000, CRC(54d95730) SHA1(a4e114dee1567d8aa31eed770f7cc366588f395c))
ROM_END

ROM_START(ip2500)
	ROM_REGION(0x80, INTERPRO_IDPROM_TAG, 0)
	ROM_LOAD32_BYTE("msmt1000.bin", 0x0, 0x20, CRC(548046c0) SHA1(ce7646e868f3aa35642d7f9348f6b9e91693918e))

	ROM_REGION(0x0020000, INTERPRO_EPROM_TAG, 0)
	ROM_SYSTEM_BIOS(0, "ip2500", "InterPro 2500 EPROM")
	ROMX_LOAD("ip2500_eprom.bin", 0x00000, 0x20000, NO_DUMP, ROM_BIOS(1))

	ROM_REGION(0x20000, INTERPRO_FLASH_TAG "_lo", 0)
	ROM_LOAD_OPTIONAL("y225.u76", 0x00000, 0x20000, CRC(46c0b105) SHA1(7c4a104e4fb3d0e5e8db7c911cdfb3f5c4fb0218))

	ROM_REGION(0x20000, INTERPRO_FLASH_TAG "_hi", 0)
	ROM_LOAD_OPTIONAL("y226.u67", 0x00000, 0x20000, CRC(54d95730) SHA1(a4e114dee1567d8aa31eed770f7cc366588f395c))
ROM_END

ROM_START(ip2700)
	ROM_REGION(0x80, INTERPRO_IDPROM_TAG, 0)
	ROM_LOAD32_BYTE("msmt1280.bin", 0x0, 0x20, CRC(32d833af) SHA1(7225c5f5670fe49d86556a2cb453ae6fe09e3e19))

	ROM_REGION(0x0020000, INTERPRO_EPROM_TAG, 0)
	ROM_SYSTEM_BIOS(0, "ip2700", "InterPro 2700 EPROM")
	ROMX_LOAD("mprgz530a__9405181.u35", 0x00000, 0x20000, CRC(467ce7bd) SHA1(53faee40d5df311f53b24c930e434cbf94a5c4aa), ROM_BIOS(1))

	ROM_REGION(0x20000, INTERPRO_FLASH_TAG "_lo", 0)
	ROM_LOAD_OPTIONAL("y225.u76", 0x00000, 0x20000, CRC(46c0b105) SHA1(7c4a104e4fb3d0e5e8db7c911cdfb3f5c4fb0218))

	ROM_REGION(0x20000, INTERPRO_FLASH_TAG "_hi", 0)
	ROM_LOAD_OPTIONAL("y226.u67", 0x00000, 0x20000, CRC(54d95730) SHA1(a4e114dee1567d8aa31eed770f7cc366588f395c))
ROM_END

ROM_START(ip2800)
	ROM_REGION(0x80, INTERPRO_IDPROM_TAG, 0)
	ROM_LOAD32_BYTE("msmt1450.bin", 0x0, 0x20, CRC(61c7a305) SHA1(efcd045cbdfda8df44eaad761b0ba99367973cd7))

	ROM_REGION(0x0020000, INTERPRO_EPROM_TAG, 0)
	ROM_SYSTEM_BIOS(0, "ip2800", "InterPro 2800 EPROM")
	ROMX_LOAD("ip2800_eprom.bin", 0x00000, 0x20000, CRC(467ce7bd) SHA1(53faee40d5df311f53b24c930e434cbf94a5c4aa), ROM_BIOS(1))

	ROM_REGION(0x20000, INTERPRO_FLASH_TAG "_lo", 0)
	ROM_LOAD_OPTIONAL("y225.u76", 0x00000, 0x20000, CRC(46c0b105) SHA1(7c4a104e4fb3d0e5e8db7c911cdfb3f5c4fb0218))

	ROM_REGION(0x20000, INTERPRO_FLASH_TAG "_hi", 0)
	ROM_LOAD_OPTIONAL("y226.u67", 0x00000, 0x20000, CRC(54d95730) SHA1(a4e114dee1567d8aa31eed770f7cc366588f395c))
ROM_END

/*    YEAR   NAME        PARENT  COMPAT  MACHINE     INPUT     CLASS            INIT       COMPANY         FULLNAME         FLAGS */
COMP( 1990,  ip2000,     0,      0,      ip2000,     interpro, turquoise_state, common,    "Intergraph",   "InterPro 2000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1992,  ip2400,     0,      0,      ip2400,     interpro, sapphire_state,  common,    "Intergraph",   "InterPro 2400", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1993,  ip2500,     0,      0,      ip2500,     interpro, sapphire_state,  common,    "Intergraph",   "InterPro 2500", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1993,  ip2700,     0,      0,      ip2700,     interpro, sapphire_state,  common,    "Intergraph",   "InterPro 2700", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1994,  ip2800,     0,      0,      ip2800,     interpro, sapphire_state,  common,    "Intergraph",   "InterPro 2800", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
