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
 *   2700   1993  C400I  61 MIPS?/40.1 SPECmark89      sapphire 2   CBUS
 *   6700   1993  C400I  61 MIPS?/40.1 SPECmark89      sapphire 2   SRX
 *   6800   1993  C400I  85 MIPS/67.2 SPECmark89       sapphire 3   SRX
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
 *   C: graphics type (0=none, 2=GT, 3=GT+, 5=GTII, 4=EDGE-1, 8=EDGE-2/2+)
 *   D: usually 0, 6xxx systems have 5, 7 and 9 options (backplane type?)
 *
 * Both the desktop and minicase units supported expansion slots with a variety
 * of cards, although with different profiles and connectors between 2xxx and
 * 6xxx systems. The 2xxx bus is referred to as CBUS, with the 6xxx bus known
 * as SRX bus; SR (shared resource) bus is used to refer to either type. The
 * bus supported a range of add-in cards, ranging from expanded SCSI and serial
 * controllers, to specialised scanning and plotting controllers, a VME bridge,
 * and most importantly various single and dual-headed graphics boards.
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
 * This driver currently supports the Emerald, Turquoise and Sapphire systems
 * (i.e. 2x00 and 6x00 models), but not all of the variants of the Emerald at
 * this point. GT/GTDB or EDGE graphics can be used depending on model, or
 * graphics and keyboard uninstalled and a serial terminal used instead.
 *
 * Key parts lists for the supported models are as follows.
 *
 * 2000 Turquoise (PCB962 rev A, PCB824 rev J)
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
 * 2400 Sapphire (SMT047 rev 0, SMT038?)
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
 * 2700 Sapphire (SMT128 rev 0, SMT104 rev A)
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
 *   U71   LSI L1A7374 CICD094A3     Intergraph I/O gate array
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
 * 6000 (PCB765 rev B, PCB82409 rev D)
 *
 *   Ref   Part                      Function
 *   U264  Xilinx XC3020-50          Plotter control FPGA?
 *   U294  4.9152 MHz crystal
 *   U287  Z8530H-6JC                Serial controller
 *   U288  Z8530H-6JC                Serial controller
 *   U318  24.0 MHz crystal
 *   U323  20.0 MHz crystal
 *   U324  Intel 82586-10            Ethernet controller
 *   U303  CICD84607 TC110G75CY
 *   U311  NCR 53C90A                SCSI controller
 *   U336  27C010 (MPRGG360F)        128Kx8 Boot EPROM MSB
 *   U341  Dallas DS1287             RTC and NVRAM
 *   U349  27C010 (MPRGG350F)        128Kx8 Boot EPROM LSB
 *   U347  Intel 82072               Floppy drive controller
 *
 * 6400 (SMT046 rev B, SMT082)
 *
 * 6800 Sapphire (SMT127 rev B, SMT104 rev C)
 *
 *   Ref   Part                      Function
 *   U53   LSI L1A7751 CICD094A3     Intergraph I/O gate array
 *   U55   Xilinx XC3020-70          Plotter control FPGA?
 *   U57   CYID21704 TC140G54AF
 *   U69   CYID21603 TC150G89AF
 *   U73   20.0 MHz crystal          Clock source for 82596?
 *   U78   29.0 MHz crystal
 *   U88   4.9152 MHz crystal        Clock source for 8530s?
 *   U94   40.0 MHz crystal
 *   U95   CICD 89703 TC110G11AT
 *   U108  24.0 MHz crystal          Clock source for 82077
 *   U117  Intel N28F010             128Kx8 flash memory (Y225 0C40 6010)
 *   U118  Zilog Z85C30 SCC          Keyboard and console serial controller
 *   U119  Zilog Z85230 ESCC         Serial controller for serial port 0 and 1
 *   U130  Intel N28F010             128Kx8 flash memory (Y226 0C40 6280)
 *   U136  NCR 53C94                 SCSI controller
 *   U144  27C1024 (MPRGZ530A)       64Kx16 Boot EPROM
 *   U145  Dallas DS12887            RTC and NVRAM
 *   U159  Intel 82596SX-20          Ethernet controller
 *   U164  Intel N82077SL-1?         Floppy drive controller
 *
 * CPU daughter-boards
 *
 *   PCB824 Rev J (MPCBA5507)
 *   CPCB82409 rev D (MPCB92108) - 6000 C311 + 2xC322 + 80MHz crystal
 *
 *   SMT082 (MSMT0820B, 36MHz?) - 6400 (SMT046 "6400 36-MHz Series System Board")
 *   SMT03? 2400/6400 - (MSMT03804 -> rev 2 cammu, goes with "6400 36-MHz Series System Board", MSMT0380A eco 3+ use rev 3 cammu)
 *   SMT019 (MSMT019 C4E CPU Assembly)
 *   SMT104 Rev A - 2700/6700 (aka MSMT1040A "C4I CPU", C4 CPU REV 3 + C4 FPU REV 3 + C4I CAMMU)
 *
 *   PCB962   2000 System Board                  MPCB824 C300
 *   PCB???   6000 System Board w/?MB            MPCB824 C300
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

#include "cpu/clipper/clipper.h"
#include "machine/cammu.h"

#include "interpro_ioga.h"
#include "interpro_mcga.h"
#include "interpro_sga.h"
#include "interpro_arbga.h"

#include "imagedev/floppy.h"
#include "machine/ram.h"
#include "machine/28fxxx.h"
#include "machine/mc146818.h"
#include "machine/z80scc.h"
#include "machine/upd765.h"
#include "machine/i82586.h"

#include "machine/ncr53c90.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"

#include "bus/rs232/rs232.h"

#include "bus/interpro/sr/sr.h"
#include "bus/interpro/sr/sr_cards.h"
#include "bus/interpro/keyboard/keyboard.h"
#include "bus/interpro/mouse/mouse.h"

#include "softlist.h"

#include "machine/input_merger.h"

#include "debugger.h"

#include "interpro.lh"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class interpro_state : public driver_device
{
public:
	interpro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_mcga(*this, "mcga")
		, m_sga(*this, "sga")
		, m_fdc(*this, "fdc")
		, m_scc1(*this, "scc1")
		, m_scc2(*this, "scc2")
		, m_rtc(*this, "rtc")
		, m_scsibus(*this, "scsi")
		, m_eth(*this, "eth")
		, m_ioga(*this, "ioga")
		, m_eprom(*this, "eprom")
		, m_softlist(*this, "softlist")
		, m_diag_led(*this, "digit0")
	{
	}

	required_device<clipper_device> m_maincpu;
	required_device<ram_device> m_ram;

	required_device<interpro_mcga_device> m_mcga;
	required_device<interpro_sga_device> m_sga;
	required_device<upd765_family_device> m_fdc;
	required_device<z80scc_device> m_scc1;
	required_device<z80scc_device> m_scc2;
	required_device<mc146818_device> m_rtc;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<i82586_base_device> m_eth;
	required_device<interpro_ioga_device> m_ioga;
	required_region_ptr<u16> m_eprom;

	required_device<software_list_device> m_softlist;

	void init_common();

	virtual u32 unmapped_r(address_space &space, offs_t offset);
	virtual void unmapped_w(offs_t offset, u32 data);

	enum error_mask : u16
	{
		ERROR_BPID4    = 0x0001,
		ERROR_SRXMMBE  = 0x0002,
		ERROR_SRXHOG   = 0x0004,
		ERROR_SRXNEM   = 0x0008,
		ERROR_SRXVALID = 0x0010,
		ERROR_CBUSNMI  = 0x0020,
		ERROR_CBUSBG   = 0x00c0,
		ERROR_BG       = 0x0070,
		ERROR_BUSHOG   = 0x0080
	};
	u16 error_r();

	void led_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	enum status_mask : u16
	{
		STATUS_YELLOW_ZONE = 0x0001,
		STATUS_SRNMI       = 0x0002,
		STATUS_PWRLOSS     = 0x0004,
		STATUS_RED_ZONE    = 0x0008,
		STATUS_BP          = 0x00f0
	};
	u16 status_r() { return m_status; }

	virtual u16 ctrl1_r() = 0;
	virtual void ctrl1_w(offs_t offset, u16 data, u16 mem_mask = ~0) = 0;
	virtual u16 ctrl2_r() = 0;
	virtual void ctrl2_w(offs_t offset, u16 data, u16 mem_mask = ~0) = 0;

	u8 nodeid_r(address_space &space, offs_t offset);

	void ioga(machine_config &config);
	void interpro_serial(machine_config &config);
	void interpro(machine_config &config);
	static void interpro_scsi_adapter(device_t *device);
	static void interpro_cdrom(device_t *device);
	void interpro_boot_map(address_map &map) ATTR_COLD;
	void interpro_common_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	output_finder<> m_diag_led;
	emu_timer *m_reset_timer = nullptr;

	u16 m_error = 0;
	u16 m_status = 0;
	u16 m_led = 0;
};

class emerald_state : public interpro_state
{
public:
	emerald_state(const machine_config &mconfig, device_type type, const char *tag)
		: interpro_state(mconfig, type, tag)
		, m_d_cammu(*this, "cammu_d")
		, m_i_cammu(*this, "cammu_i")
		, m_scsi(*this, "scsi:7:host")
		, m_bus(*this, "slot")
	{
	}

	void error_w(u8 data) { m_error = data; }

	enum ctrl1_mask : u16
	{
		CTRL1_FLOPLOW    = 0x0001, // 3.5" floppy select
		CTRL1_FLOPRDY    = 0x0002, // floppy ready enable?
		CTRL1_LEDENA     = 0x0004, // led display enable
		CTRL1_LEDDP      = 0x0008, // led right decimal point enable
		CTRL1_ETHLOOP    = 0x0010, // ethernet loopback enable?
		CTRL1_ETHDTR     = 0x0020, // modem dtr pin enable?
		CTRL1_ETHRMOD    = 0x0040, // remote modem configured (read)?
		CTRL1_CLIPRESET  = 0x0040, // hard reset (write)?
		CTRL1_FIFOACTIVE = 0x0080  // plotter fifo active?
	};
	u16 ctrl1_r() override { return m_ctrl1; }
	void ctrl1_w(offs_t offset, u16 data, u16 mem_mask = ~0) override;

	enum ctrl2_mask : u16
	{
		CTRL2_PWRUP     = 0x0001, // power supply voltage adjust?
		CRTL2_PWRENA    = 0x0002, // ?
		CTRL2_HOLDOFF   = 0x0004, // power supply shut down delay
		CTRL2_EXTNMIENA = 0x0008, // power nmi enable
		CTRL2_COLDSTART = 0x0010, // cold start flag
		CTRL2_RESET     = 0x0020, // soft reset
		CTRL2_BUSENA    = 0x0040, // clear bus grant error
		CTRL2_FRCPARITY = 0x0080, // ?

		CTRL2_WMASK     = 0x000f
	};
	u16 ctrl2_r() override { return m_ctrl2; }
	void ctrl2_w(offs_t offset, u16 data, u16 mem_mask = ~0) override;

	required_device<cammu_c3_device> m_d_cammu;
	required_device<cammu_c3_device> m_i_cammu;
	required_device<ncr53c90a_device> m_scsi;
	required_device<srx_bus_device> m_bus;

	void emerald(machine_config &config);
	void ip6000(machine_config &config);
	void interpro_82586_map(address_map &map) ATTR_COLD;
	void emerald_base_map(address_map &map) ATTR_COLD;
	void emerald_main_map(address_map &map) ATTR_COLD;
	void emerald_io_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u16 m_ctrl1 = 0;
	u16 m_ctrl2 = 0;
};

class turquoise_state : public interpro_state
{
public:
	turquoise_state(const machine_config &mconfig, device_type type, const char *tag)
		: interpro_state(mconfig, type, tag)
		, m_d_cammu(*this, "cammu_d")
		, m_i_cammu(*this, "cammu_i")
		, m_kbd_port(*this, "kbd")
		, m_mse_port(*this, "mse")
		, m_scsi(*this, "scsi:7:host")
		, m_bus(*this, "slot")
	{
	}

	void error_w(u8 data) { m_error = data; }

	enum ctrl1_mask : u16
	{
		CTRL1_FLOPLOW    = 0x0001, // 3.5" floppy select
		CTRL1_FLOPRDY    = 0x0002, // floppy ready enable?
		CTRL1_LEDENA     = 0x0004, // led display enable
		CTRL1_LEDDP      = 0x0008, // led right decimal point enable
		CTRL1_ETHLOOP    = 0x0010, // ethernet loopback enable?
		CTRL1_ETHDTR     = 0x0020, // modem dtr pin enable?
		CTRL1_ETHRMOD    = 0x0040, // remote modem configured (read)?
		CTRL1_CLIPRESET  = 0x0040, // hard reset (write)?
		CTRL1_FIFOACTIVE = 0x0080  // plotter fifo active?
	};
	u16 ctrl1_r() override { return m_ctrl1; }
	void ctrl1_w(offs_t offset, u16 data, u16 mem_mask = ~0) override;

	enum ctrl2_mask : u16
	{
		CTRL2_PWRUP     = 0x0001, // power supply voltage adjust?
		CRTL2_PWRENA    = 0x0002, // ?
		CTRL2_HOLDOFF   = 0x0004, // power supply shut down delay
		CTRL2_EXTNMIENA = 0x0008, // power nmi enable
		CTRL2_COLDSTART = 0x0010, // cold start flag
		CTRL2_RESET     = 0x0020, // soft reset
		CTRL2_BUSENA    = 0x0040, // clear bus grant error
		CTRL2_FRCPARITY = 0x0080, // ?

		CTRL2_WMASK     = 0x000f
	};
	u16 ctrl2_r() override { return m_ctrl2; }
	void ctrl2_w(offs_t offset, u16 data, u16 mem_mask = ~0) override;

	required_device<cammu_c3_device> m_d_cammu;
	required_device<cammu_c3_device> m_i_cammu;
	required_device<interpro_keyboard_port_device> m_kbd_port;
	required_device<interpro_mouse_port_device> m_mse_port;
	required_device<ncr53c90a_device> m_scsi;
	required_device<cbus_bus_device> m_bus;

	void turquoise(machine_config &config);
	void ip2000(machine_config &config);
	void interpro_82586_map(address_map &map) ATTR_COLD;
	void turquoise_base_map(address_map &map) ATTR_COLD;
	void turquoise_main_map(address_map &map) ATTR_COLD;
	void turquoise_io_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u16 m_ctrl1 = 0;
	u16 m_ctrl2 = 0;
};

class sapphire_state : public interpro_state
{
public:
	sapphire_state(const machine_config &mconfig, device_type type, const char *tag)
		: interpro_state(mconfig, type, tag)
		, m_mmu(*this, "cammu")
		, m_scsi(*this, "scsi:7:host")
		, m_arbga(*this, "arbga")
		, m_flash_lsb(*this, "flash_lsb")
		, m_flash_msb(*this, "flash_msb")
	{
	}

	virtual u32 unmapped_r(address_space &space, offs_t offset) override;
	virtual void unmapped_w(offs_t offset, u32 data) override;

	enum ctrl1_mask : u16
	{
		CTRL1_FLOPLOW    = 0x0001, // 3.5" floppy select
								   // unused
		CTRL1_LEDENA     = 0x0004, // led display enable
		CTRL1_LEDDP      = 0x0008, // led right decimal point enable
		CTRL1_MMBE       = 0x0010, // mmbe enable
		CTRL1_ETHDTR     = 0x0020, // modem dtr pin enable
		CTRL1_ETHRMOD    = 0x0040, // 0 = sytem configured for remote modems
		CTRL1_FIFOACTIVE = 0x0080  // 0 = plotter fifos reset
	};
	u16 ctrl1_r() override { return m_ctrl1; }
	void ctrl1_w(offs_t offset, u16 data, u16 mem_mask = ~0) override;

	enum ctrl2_mask : u16
	{
		CTRL2_PWRUP     = 0x0003, // power supply voltage adjust
		CTRL2_HOLDOFF   = 0x0004, // power supply shut down delay
		CTRL2_EXTNMIENA = 0x0008, // power nmi enable
		CTRL2_COLDSTART = 0x0010, // cold start flag
		CTRL2_RESET     = 0x0020, // soft reset
		CTRL2_BUSENA    = 0x0040, // clear bus grant error
		CTRL2_FLASHEN   = 0x0080, // flash eprom write enable
	};
	u16 ctrl2_r() override { return m_ctrl2; }
	void ctrl2_w(offs_t offset, u16 data, u16 mem_mask = ~0) override;

	required_device<cammu_c4_device> m_mmu;
	required_device<ncr53c94_device> m_scsi;
	required_device<interpro_arbga_device> m_arbga;
	required_device<intel_28f010_device> m_flash_lsb;
	required_device<intel_28f010_device> m_flash_msb;

	void sapphire(machine_config &config);

	void interpro_82596_map(address_map &map) ATTR_COLD;
	void sapphire_base_map(address_map &map) ATTR_COLD;
	void sapphire_main_map(address_map &map) ATTR_COLD;
	void sapphire_io_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u16 m_ctrl1 = 0;
	u16 m_ctrl2 = 0;
};

class cbus_sapphire_state : public sapphire_state
{
public:
	cbus_sapphire_state(const machine_config &mconfig, device_type type, const char *tag)
		: sapphire_state(mconfig, type, tag)
		, m_kbd_port(*this, "kbd")
		, m_mse_port(*this, "mse")
		, m_bus(*this, "slot")
	{
	}

	void cbus_sapphire(machine_config &config);

	void ip2500(machine_config &config);
	void ip2400(machine_config &config);
	void ip2700(machine_config &config);
	void ip2800(machine_config &config);

protected:
	required_device<interpro_keyboard_port_device> m_kbd_port;
	required_device<interpro_mouse_port_device> m_mse_port;
	required_device<cbus_bus_device> m_bus;
};

class srx_sapphire_state : public sapphire_state
{
public:
	srx_sapphire_state(const machine_config &mconfig, device_type type, const char *tag)
		: sapphire_state(mconfig, type, tag)
		, m_bus(*this, "slot")
	{
	}

	void srx_sapphire(machine_config &config);

	void ip6400(machine_config &config);
	void ip6700(machine_config &config);
	void ip6800(machine_config &config);

protected:
	required_device<srx_bus_device> m_bus;
};

void interpro_state::machine_start()
{
	m_diag_led.resolve();

	save_item(NAME(m_error));
	save_item(NAME(m_status));
	save_item(NAME(m_led));
}

void interpro_state::machine_reset()
{
	m_error = 0;
	m_status = 0;
	m_led = 0;
}

void emerald_state::machine_start()
{
	interpro_state::machine_start();

	save_item(NAME(m_ctrl1));
	save_item(NAME(m_ctrl2));

	m_ctrl1 = 0;
	// FIXME: disabled for now to avoid cold start diagnostic errors
	m_ctrl2 = 0; // CTRL2_COLDSTART
}

void turquoise_state::machine_start()
{
	interpro_state::machine_start();

	save_item(NAME(m_ctrl1));
	save_item(NAME(m_ctrl2));

	m_ctrl1 = 0;
	// FIXME: disabled for now to avoid cold start diagnostic errors
	m_ctrl2 = 0; // CTRL2_COLDSTART
}

void sapphire_state::machine_start()
{
	interpro_state::machine_start();

	save_item(NAME(m_ctrl1));
	save_item(NAME(m_ctrl2));

	m_ctrl1 = 0;
	// FIXME: disabled for now to avoid cold start diagnostic errors
	m_ctrl2 = 0; // CTRL2_COLDSTART
}

void emerald_state::machine_reset()
{
	interpro_state::machine_reset();

	// deassert floppy ready
	m_fdc->ready_w(true);
}

void turquoise_state::machine_reset()
{
	interpro_state::machine_reset();

	// deassert floppy ready
	m_fdc->ready_w(true);
}

void sapphire_state::machine_reset()
{
	interpro_state::machine_reset();

	// flash rom requires the following
	m_status = 0x400;
}

void interpro_state::init_common()
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

void interpro_state::led_w(offs_t offset, u16 data, u16 mem_mask)
{
	// 7-segment decode patterns (hex digits) borrowed from wico.cpp (mc14495)
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 };

	m_diag_led = patterns[data & 0xf];

	COMBINE_DATA(&m_led);
}

void emerald_state::ctrl1_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("control register 1 data 0x%04x mem_mask 0x%04x (%s)\n", data, mem_mask, machine().describe_context());

	// check if led decimal point changes state
	if ((data ^ m_ctrl1) & CTRL1_LEDDP)
		m_led = (m_led + 0x80) & 0xff;

	// FIXME: select 3.5" or 5.25" floppy drive?
	//if ((data ^ m_sreg_ctrl1) & CTRL1_FLOPLOW)
	//  logerror("floplow %d\n", data & CTRL1_FLOPLOW ? 1 : 0);

	// FIXME: floppy ready line handling - this is strange but working
	if (data & CTRL1_FLOPRDY)
		m_fdc->ready_w(!(data & CTRL1_FLOPRDY));

	COMBINE_DATA(&m_ctrl1);
}

void turquoise_state::ctrl1_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("control register 1 data 0x%04x mem_mask 0x%04x (%s)\n", data, mem_mask, machine().describe_context());

	// check if led decimal point changes state
	if ((data ^ m_ctrl1) & CTRL1_LEDDP)
		m_led = (m_led + 0x80) & 0xff;

	// FIXME: select 3.5" or 5.25" floppy drive?
	//if ((data ^ m_sreg_ctrl1) & CTRL1_FLOPLOW)
	//  logerror("floplow %d\n", data & CTRL1_FLOPLOW ? 1 : 0);

	// FIXME: floppy ready line handling - this is strange but working
	if (data & CTRL1_FLOPRDY)
		m_fdc->ready_w(!(data & CTRL1_FLOPRDY));

	COMBINE_DATA(&m_ctrl1);
}

void sapphire_state::ctrl1_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("control register 1 data 0x%04x mem_mask 0x%04x (%s)\n", data, mem_mask, machine().describe_context());

	// check if led decimal point changes state
	if ((data ^ m_ctrl1) & CTRL1_LEDDP)
		m_led = (m_led + 0x80) & 0xff;

	// FIXME: select 3.5" or 5.25" floppy drive?
	//if ((data ^ m_ctrl1) & CTRL1_FLOPLOW)
	//  logerror("floplow %d\n", data & CTRL1_FLOPLOW ? 1 : 0);

	COMBINE_DATA(&m_ctrl1);
}

void emerald_state::ctrl2_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("control register 2 data 0x%04x mem_mask 0x%04x (%s)\n", data, mem_mask, machine().describe_context());
	if (data & CTRL2_RESET)
	{
		m_ctrl2 &= ~CTRL2_COLDSTART;

		machine().schedule_soft_reset();
	}
	else
		m_ctrl2 = (m_ctrl2 & ~CTRL2_WMASK) | (data & CTRL2_WMASK);
}

void turquoise_state::ctrl2_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("control register 2 data 0x%04x mem_mask 0x%04x (%s)\n", data, mem_mask, machine().describe_context());
	if (data & CTRL2_RESET)
	{
		m_ctrl2 &= ~CTRL2_COLDSTART;

		machine().schedule_soft_reset();
	}
	else
		m_ctrl2 = (m_ctrl2 & ~CTRL2_WMASK) | (data & CTRL2_WMASK);
}

void sapphire_state::ctrl2_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOG("control register 2 data 0x%04x mem_mask 0x%04x (%s)\n", data, mem_mask, machine().describe_context());
	if (data & CTRL2_RESET)
	{
		m_ctrl2 &= ~CTRL2_COLDSTART;

		machine().schedule_soft_reset();
	}
	else
		m_ctrl2 = (m_ctrl2 & ~0x4d) | (data & 0x4d);

	// enable/disable programming power on both flash devices
	m_flash_lsb->vpp(data & CTRL2_FLASHEN ? ASSERT_LINE : CLEAR_LINE);
	m_flash_msb->vpp(data & CTRL2_FLASHEN ? ASSERT_LINE : CLEAR_LINE);
}

u16 interpro_state::error_r()
{
	const u16 result = m_error;

	// clear error register on read
	if (!machine().side_effects_disabled())
		m_error = 0;

	return result;
}

u32 interpro_state::unmapped_r(address_space &space, offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		// flag non-existent memory error in system error register
		m_error |= (ERROR_SRXNEM | ERROR_SRXVALID);

		// tell ioga to raise a bus error
		m_ioga->bus_error(interpro_ioga_device::BINFO_BERR | interpro_ioga_device::BINFO_SNAPOK, offset << 2);
	}

	return space.unmap();
}

void interpro_state::unmapped_w(offs_t offset, u32 data)
{
	if (!machine().side_effects_disabled())
	{
		// flag non-existent memory error in system error register
		m_error |= (ERROR_SRXNEM | ERROR_SRXVALID);

		// tell ioga to raise a bus error
		m_ioga->bus_error(interpro_ioga_device::BINFO_BERR | interpro_ioga_device::BINFO_SNAPOK, offset << 2);
	}
}

u32 sapphire_state::unmapped_r(address_space &space, offs_t offset)
{
	// check if non-existent memory errors are enabled
	if (!machine().side_effects_disabled())
		if (m_arbga->tctrl_r() & interpro_arbga_device::TCTRL_ENNEM)
		{
			// flag non-existent memory error in system error register
			m_error |= (ERROR_SRXNEM | ERROR_SRXVALID);

			// tell ioga to raise a bus error
			m_ioga->bus_error(interpro_ioga_device::BINFO_BERR | interpro_ioga_device::BINFO_SNAPOK, offset << 2);
		}

	return space.unmap();
}

void sapphire_state::unmapped_w(offs_t offset, u32 data)
{
	// check if non-existent memory errors are enabled
	if (m_arbga->tctrl_r() & interpro_arbga_device::TCTRL_ENNEM)
	{
		// flag non-existent memory error in system error register
		m_error |= (ERROR_SRXNEM | ERROR_SRXVALID);

		// tell ioga to raise a bus error
		m_ioga->bus_error(interpro_ioga_device::BINFO_BERR | interpro_ioga_device::BINFO_SNAPOK, offset << 2);
	}
}

u8 interpro_state::nodeid_r(address_space &space, offs_t offset)
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
	//map(0x00000000, 0xffffffff).rw(FUNC(interpro_state::unmapped_r), FUNC(interpro_state::unmapped_w));

	// FIXME: don't know what this range is for
	map(0x08000000, 0x08000fff).noprw();

	map(0x4f007e00, 0x4f007eff).m(m_sga, FUNC(interpro_sga_device::map));

	map(0x7f000100, 0x7f00011f).m(m_fdc, FUNC(upd765_family_device::map)).umask32(0x000000ff);
	map(0x7f000300, 0x7f000301).r(FUNC(interpro_state::error_r));
	map(0x7f000304, 0x7f000305).rw(FUNC(interpro_state::status_r), FUNC(interpro_state::led_w));
	map(0x7f000308, 0x7f000309).rw(FUNC(interpro_state::ctrl1_r), FUNC(interpro_state::ctrl1_w));
	map(0x7f00030c, 0x7f00030d).rw(FUNC(interpro_state::ctrl2_r), FUNC(interpro_state::ctrl2_w));

	map(0x7f000400, 0x7f00040f).rw(m_scc1, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0x000000ff);
	map(0x7f000410, 0x7f00041f).rw(m_scc2, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0x000000ff);
	map(0x7f000500, 0x7f000500).rw(m_rtc, FUNC(mc146818_device::data_r), FUNC(mc146818_device::data_w));
	map(0x7f000600, 0x7f000600).w(m_rtc, FUNC(mc146818_device::address_w));

	// the system board id prom
	map(0x7f000700, 0x7f00077f).rom().region("idprom", 0);

	map(0x7f0fff00, 0x7f0fffff).m(m_ioga, FUNC(interpro_ioga_device::map));
}

void emerald_state::emerald_base_map(address_map &map)
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

	map(0x7f000300, 0x7f000300).w(FUNC(emerald_state::error_w));

	map(0x7f000600, 0x7f00067f).rom().region("nodeid", 0);
}

void emerald_state::emerald_main_map(address_map &map)
{
	//map(0x00000000, 0xffffffff).rw(FUNC(interpro_state::unmapped_r), FUNC(interpro_state::unmapped_w));

	emerald_base_map(map);

	map(0x00000000, 0x00ffffff).ram().share("ram");
	map(0x7f100000, 0x7f13ffff).lr16([this] (offs_t offset) { return m_eprom[offset]; }, "eprom");
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

	map(0x7f000300, 0x7f000300).w(FUNC(turquoise_state::error_w));

	map(0x7f000600, 0x7f00067f).rom().region("nodeid", 0);
}

void turquoise_state::turquoise_main_map(address_map &map)
{
	//map(0x00000000, 0xffffffff).rw(FUNC(interpro_state::unmapped_r), FUNC(interpro_state::unmapped_w));

	turquoise_base_map(map);

	map(0x00000000, 0x00ffffff).ram().share("ram");
	map(0x7f100000, 0x7f13ffff).lr16([this] (offs_t offset) { return m_eprom[offset]; }, "eprom");
}

void sapphire_state::sapphire_base_map(address_map &map)
{
	interpro_common_map(map);

	map(0x40000000, 0x4000004f).m(m_mcga, FUNC(interpro_fmcc_device::map));
	map(0x7f000200, 0x7f0002ff).m(m_arbga, FUNC(interpro_arbga_device::map));

	map(0x7f000600, 0x7f00060f).r(FUNC(sapphire_state::nodeid_r)).umask32(0x000000ff);

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
	//map(0x00000000, 0xffffffff).rw(FUNC(sapphire_state::unmapped_r), FUNC(sapphire_state::unmapped_w));

	sapphire_base_map(map);

	map(0x00000000, 0x00ffffff).ram().share("ram");
	map(0x7f100000, 0x7f11ffff).lr16([this] (offs_t offset) { return m_eprom[offset]; }, "eprom");
	map(0x7f180000, 0x7f1fffff).rw(m_flash_lsb, FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).umask32(0x00ff00ff).mask(0x3ffff);
	map(0x7f180000, 0x7f1fffff).rw(m_flash_msb, FUNC(intel_28f010_device::read), FUNC(intel_28f010_device::write)).umask32(0xff00ff00).mask(0x3ffff);

	// HACK: for SRX bus Sapphire only
	//map(0x8f007f80, 0x8f007fff).rom().region("idprom", 0);
}

void emerald_state::emerald_io_map(address_map &map)
{
	emerald_base_map(map);
}

void turquoise_state::turquoise_io_map(address_map &map)
{
	turquoise_base_map(map);
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

void emerald_state::interpro_82586_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).rw(m_ioga, FUNC(emerald_ioga_device::eth_r), FUNC(emerald_ioga_device::eth_w));
}

void turquoise_state::interpro_82586_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).rw(m_ioga, FUNC(turquoise_ioga_device::eth_r), FUNC(turquoise_ioga_device::eth_w));
}

void sapphire_state::interpro_82596_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(m_ioga, FUNC(sapphire_ioga_device::eth_r), FUNC(sapphire_ioga_device::eth_w));
}

void interpro_state::interpro_serial(machine_config &config)
{
	input_merger_device &scc_int(INPUT_MERGER_ANY_HIGH(config, "scc_int"));

	/*
	 * Documentation states that all three serial ports have RxD, TxD, CTS and
	 * RTS signals connected, and serial port 0 also has RI, DTR and DTS(?).
	 * Serial diagnostics pass all tests (except internal loopback which is not
	 * supported by z80scc_device) when a dec_loopback device is installed. The
	 * diagnostic tests also indicate that DCD is connected on all three ports.
	 *
	 * The documentation consistently refers to a DTS signal on serial port 0,
	 * but this appears to be an error or typo, as it doesn't match any known
	 * RS-232 signal; possibly it should be DSR?
	 */
	// scc1 channel A (serial port 1)
	rs232_port_device &port1(RS232_PORT(config, "serial1", default_rs232_devices, nullptr));
	port1.cts_handler().set(m_scc1, FUNC(z80scc_device::ctsa_w));
	port1.dcd_handler().set(m_scc1, FUNC(z80scc_device::dcda_w));
	port1.rxd_handler().set(m_scc1, FUNC(z80scc_device::rxa_w));
	m_scc1->out_rtsa_callback().set(port1, FUNC(rs232_port_device::write_rts));
	m_scc1->out_txda_callback().set(port1, FUNC(rs232_port_device::write_txd));
	m_scc1->out_wreqa_callback().set(m_ioga, FUNC(interpro_ioga_device::drq_serial1)).invert();

	// scc1 channel B (serial port 2)
	rs232_port_device &port2(RS232_PORT(config, "serial2", default_rs232_devices, nullptr));
	port2.cts_handler().set(m_scc1, FUNC(z80scc_device::ctsb_w));
	port2.dcd_handler().set(m_scc1, FUNC(z80scc_device::dcdb_w));
	port2.rxd_handler().set(m_scc1, FUNC(z80scc_device::rxb_w));
	m_scc1->out_rtsb_callback().set(port2, FUNC(rs232_port_device::write_rts));
	m_scc1->out_txdb_callback().set(port2, FUNC(rs232_port_device::write_txd));
	m_scc1->out_wreqb_callback().set(m_ioga, FUNC(interpro_ioga_device::drq_serial2)).invert();

	m_scc1->out_int_callback().set(scc_int, FUNC(input_merger_device::in_w<0>));

	// scc2 channel B (serial port 0)
	rs232_port_device &port0(RS232_PORT(config, "serial0", default_rs232_devices, nullptr));
	port0.cts_handler().set(m_scc2, FUNC(z80scc_device::ctsb_w));
	port0.dcd_handler().set(m_scc2, FUNC(z80scc_device::dcdb_w));
	port0.ri_handler().set(m_scc2, FUNC(z80scc_device::syncb_w));
	port0.rxd_handler().set(m_scc2, FUNC(z80scc_device::rxb_w));
	m_scc2->out_dtrb_callback().set(port0, FUNC(rs232_port_device::write_dtr));
	m_scc2->out_rtsb_callback().set(port0, FUNC(rs232_port_device::write_rts));
	m_scc2->out_txdb_callback().set(port0, FUNC(rs232_port_device::write_txd));
	m_scc2->out_wreqb_callback().set(m_ioga, FUNC(interpro_ioga_device::drq_serial0)).invert();

	m_scc2->out_int_callback().set(scc_int, FUNC(input_merger_device::in_w<1>));

	scc_int.output_handler().set(m_ioga, FUNC(interpro_ioga_device::ir11_w));
}

static void interpro_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

void interpro_state::interpro_scsi_adapter(device_t *device)
{
	ncr53c90_device &adapter = downcast<ncr53c90_device &>(*device);

	adapter.set_clock(24_MHz_XTAL);

	adapter.irq_handler_cb().set(":ioga", FUNC(interpro_ioga_device::ir0_w));
	adapter.drq_handler_cb().set(":ioga", FUNC(interpro_ioga_device::drq_scsi));
}

void interpro_state::interpro_cdrom(device_t *device)
{
	downcast<nscsi_cdrom_device &>(*device).set_block_size(512);
}

void interpro_state::ioga(machine_config &config)
{
	m_ioga->out_nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_ioga->out_irq_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ioga->out_irq_vector_callback().set(m_maincpu, FUNC(clipper_device::set_ivec));

	// ioga dma and serial dma channels
	//m_ioga->dma_r_callback<0>().set(unknown); // plotter
	m_ioga->dma_r_callback<1>().set("scsi:7:host", FUNC(ncr53c90a_device::dma_r));
	m_ioga->dma_w_callback<1>().set("scsi:7:host", FUNC(ncr53c90a_device::dma_w));
	m_ioga->dma_r_callback<2>().set(m_fdc, FUNC(upd765_family_device::dma_r));
	m_ioga->dma_w_callback<2>().set(m_fdc, FUNC(upd765_family_device::dma_w));
	m_ioga->serial_dma_r_callback<0>().set(m_scc2, FUNC(z80scc_device::db_r));
	m_ioga->serial_dma_w_callback<0>().set(m_scc2, FUNC(z80scc_device::db_w));
	m_ioga->serial_dma_r_callback<1>().set(m_scc1, FUNC(z80scc_device::da_r));
	m_ioga->serial_dma_w_callback<1>().set(m_scc1, FUNC(z80scc_device::da_w));
	m_ioga->serial_dma_r_callback<2>().set(m_scc1, FUNC(z80scc_device::db_r));
	m_ioga->serial_dma_w_callback<2>().set(m_scc1, FUNC(z80scc_device::db_w));

	// ioga floppy terminal count, ethernet channel attention
	m_ioga->fdc_tc_callback().set(m_fdc, FUNC(upd765_family_device::tc_line_w));
	m_ioga->eth_ca_callback().set(m_eth, FUNC(i82586_base_device::ca));
}

void interpro_state::interpro(machine_config &config)
{
	RAM(config, m_ram, 0);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("32M,64M,128M,256M");

	// memory control gate array

	// srx gate array
	INTERPRO_SGA(config, m_sga, 0);
	m_sga->berr_callback().set(m_ioga, FUNC(interpro_ioga_device::bus_error));

	// floppy

	// serial

	// real-time clock/non-volatile memory
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->set_use_utc(true);
	m_rtc->irq().set(m_ioga, FUNC(interpro_ioga_device::ir9_w));

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus, 0);

	nscsi_connector &harddisk(NSCSI_CONNECTOR(config, "scsi:0", 0));
	interpro_scsi_devices(harddisk);
	harddisk.set_default_option("harddisk");

	nscsi_connector &cdrom(NSCSI_CONNECTOR(config, "scsi:4", 0));
	interpro_scsi_devices(cdrom);
	cdrom.set_default_option("cdrom");
	cdrom.set_option_machine_config("cdrom", interpro_cdrom);

	interpro_scsi_devices(NSCSI_CONNECTOR(config, "scsi:1", 0));
	interpro_scsi_devices(NSCSI_CONNECTOR(config, "scsi:2", 0));
	interpro_scsi_devices(NSCSI_CONNECTOR(config, "scsi:3", 0));
	interpro_scsi_devices(NSCSI_CONNECTOR(config, "scsi:5", 0));
	interpro_scsi_devices(NSCSI_CONNECTOR(config, "scsi:6", 0));

	// ethernet

	// i/o gate array

	// system layout
	config.set_default_layout(layout_interpro);

	// software lists
	SOFTWARE_LIST(config, m_softlist).set_original("interpro");
}

void emerald_state::emerald(machine_config &config)
{
	interpro(config);

	CLIPPER_C300(config, m_maincpu, 12.5_MHz_XTAL); // 40MHz?
	m_maincpu->set_addrmap(0, &emerald_state::emerald_main_map);
	m_maincpu->set_addrmap(1, &emerald_state::emerald_io_map);
	m_maincpu->set_addrmap(2, &emerald_state::interpro_boot_map);
	m_maincpu->set_irq_acknowledge_callback(m_ioga, FUNC(interpro_ioga_device::acknowledge_interrupt));

	CAMMU_C3(config, m_i_cammu, 0);
	m_i_cammu->exception_callback().set(m_maincpu, FUNC(clipper_device::set_exception));

	CAMMU_C3(config, m_d_cammu, 0);
	m_d_cammu->exception_callback().set(m_maincpu, FUNC(clipper_device::set_exception));
	m_d_cammu->add_linked(m_i_cammu);

	// boot fails memory test without this
	m_ram->set_default_value(0);

	// memory control gate array
	INTERPRO_MCGA(config, m_mcga, 0);

	// floppy controller
	I82072(config, m_fdc, 24_MHz_XTAL);
	m_fdc->set_ready_line_connected(false);
	m_fdc->intrq_wr_callback().set(m_ioga, FUNC(interpro_ioga_device::ir1_w));
	m_fdc->drq_wr_callback().set(m_ioga, FUNC(interpro_ioga_device::drq_floppy));

	// connect a 3.5" drive at id 3
	//FLOPPY_CONNECTOR(config, "fdc:2", "525hd", FLOPPY_525_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	// serial controllers and ports
	SCC85C30(config, m_scc1, 4.9152_MHz_XTAL);
	SCC85C30(config, m_scc2, 4.9152_MHz_XTAL);
	interpro_serial(config);

	// scsi host adapter
	nscsi_connector &adapter(NSCSI_CONNECTOR(config, "scsi:7", 0));
	adapter.option_add_internal("host", NCR53C90A);
	adapter.set_default_option("host");
	adapter.set_fixed(true);
	adapter.set_option_machine_config("host", interpro_scsi_adapter);

	// ethernet controller
	I82586(config, m_eth, 10_MHz_XTAL);
	m_eth->out_irq_cb().set(m_ioga, FUNC(interpro_ioga_device::ir12_w));
	m_eth->set_addrmap(0, &emerald_state::interpro_82586_map);

	// i/o gate array
	EMERALD_IOGA(config, m_ioga, 0);
	m_ioga->set_memory(m_maincpu, 0);
	ioga(config);

	// srx bus
	SRX_BUS(config, m_bus, 0);
	m_bus->set_main_space(m_maincpu, 0);
	m_bus->set_io_space(m_maincpu, 1);

	m_bus->out_irq0_cb().set(m_ioga, FUNC(interpro_ioga_device::ir3_w));
	m_bus->out_irq1_cb().set(m_ioga, FUNC(interpro_ioga_device::ir4_w));
	m_bus->out_irq2_cb().set(m_ioga, FUNC(interpro_ioga_device::ir5_w));
	m_bus->out_irq3_cb().set(m_ioga, FUNC(interpro_ioga_device::ir6_w));
}

void turquoise_state::turquoise(machine_config &config)
{
	interpro(config);

	CLIPPER_C300(config, m_maincpu, 12.5_MHz_XTAL); // 40Mhz?
	m_maincpu->set_addrmap(0, &turquoise_state::turquoise_main_map);
	m_maincpu->set_addrmap(1, &turquoise_state::turquoise_io_map);
	m_maincpu->set_addrmap(2, &turquoise_state::interpro_boot_map);
	m_maincpu->set_irq_acknowledge_callback(m_ioga, FUNC(interpro_ioga_device::acknowledge_interrupt));

	CAMMU_C3(config, m_i_cammu, 0);
	m_i_cammu->exception_callback().set(m_maincpu, FUNC(clipper_device::set_exception));

	CAMMU_C3(config, m_d_cammu, 0);
	m_d_cammu->exception_callback().set(m_maincpu, FUNC(clipper_device::set_exception));
	m_d_cammu->add_linked(m_i_cammu);

	// boot fails memory test without this
	m_ram->set_default_value(0);

	// memory control gate array
	INTERPRO_MCGA(config, m_mcga, 0);

	// floppy controller
	I82072(config, m_fdc, 24_MHz_XTAL);
	m_fdc->set_ready_line_connected(false);
	m_fdc->intrq_wr_callback().set(m_ioga, FUNC(interpro_ioga_device::ir1_w));
	m_fdc->drq_wr_callback().set(m_ioga, FUNC(interpro_ioga_device::drq_floppy));

	// connect a 3.5" drive at id 3
	FLOPPY_CONNECTOR(config, "fdc:3", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	// serial controllers and ports
	SCC85C30(config, m_scc1, 4.9152_MHz_XTAL);
	SCC85C30(config, m_scc2, 4.9152_MHz_XTAL);
	interpro_serial(config);

	// keyboard port
	INTERPRO_KEYBOARD_PORT(config, m_kbd_port, interpro_keyboard_devices, nullptr);
	m_kbd_port->rxd_handler_cb().set(m_scc2, FUNC(z80scc_device::rxa_w));
	m_scc2->out_txda_callback().set(m_kbd_port, FUNC(interpro_keyboard_port_device::write_txd));

	// mouse port
	INTERPRO_MOUSE_PORT(config, m_mse_port, interpro_mouse_devices, nullptr);
	m_mse_port->state_func().set(m_ioga, FUNC(interpro_ioga_device::mouse_status_w));

	// scsi host adapter
	nscsi_connector &adapter(NSCSI_CONNECTOR(config, "scsi:7", 0));
	adapter.option_add_internal("host", NCR53C90A);
	adapter.set_default_option("host");
	adapter.set_fixed(true);
	adapter.set_option_machine_config("host", interpro_scsi_adapter);

	// ethernet controller
	I82586(config, m_eth, 10_MHz_XTAL);
	m_eth->out_irq_cb().set(m_ioga, FUNC(interpro_ioga_device::ir12_w));
	m_eth->set_addrmap(0, &turquoise_state::interpro_82586_map);

	// i/o gate array
	TURQUOISE_IOGA(config, m_ioga, 0);
	m_ioga->set_memory(m_maincpu, 0);
	ioga(config);

	// cbus bus
	CBUS_BUS(config, m_bus, 0);
	m_bus->set_main_space(m_maincpu, 0);
	m_bus->set_io_space(m_maincpu, 1);

	m_bus->out_irq0_cb().set(m_ioga, FUNC(interpro_ioga_device::ir3_w));
	m_bus->out_irq1_cb().set(m_ioga, FUNC(interpro_ioga_device::ir4_w));
	m_bus->out_irq2_cb().set(m_ioga, FUNC(interpro_ioga_device::ir5_w));
	m_bus->out_irq3_cb().set(m_ioga, FUNC(interpro_ioga_device::ir6_w));
}

void sapphire_state::sapphire(machine_config &config)
{
	interpro(config);

	CLIPPER_C400(config, m_maincpu, 12.5_MHz_XTAL);
	m_maincpu->set_addrmap(0, &sapphire_state::sapphire_main_map);
	m_maincpu->set_addrmap(1, &sapphire_state::sapphire_io_map);
	m_maincpu->set_addrmap(2, &sapphire_state::interpro_boot_map);
	m_maincpu->set_irq_acknowledge_callback(m_ioga, FUNC(interpro_ioga_device::acknowledge_interrupt));

	// FIXME: 2400/6400 should be C4T cammu?
	CAMMU_C4I(config, m_mmu, 0);
	m_mmu->exception_callback().set(m_maincpu, FUNC(clipper_device::set_exception));

	// memory control gate array
	INTERPRO_FMCC(config, m_mcga, 0);

	// floppy controller
	N82077AA(config, m_fdc, 24_MHz_XTAL, n82077aa_device::mode_t::PS2);
	m_fdc->intrq_wr_callback().set(m_ioga, FUNC(interpro_ioga_device::ir1_w));
	m_fdc->drq_wr_callback().set(m_ioga, FUNC(interpro_ioga_device::drq_floppy));

	// connect a 3.5" drive at id 1
	FLOPPY_CONNECTOR(config, "fdc:1", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	// srx arbiter gate array
	INTERPRO_ARBGA(config, m_arbga, 0);

	// serial controllers and ports
	SCC85230(config, m_scc1, 4.9152_MHz_XTAL);
	SCC85C30(config, m_scc2, 4.9152_MHz_XTAL);
	interpro_serial(config);

	// scsi host adapter
	nscsi_connector &adapter(NSCSI_CONNECTOR(config, "scsi:7", 0));
	adapter.option_add_internal("host", NCR53C94);
	adapter.set_default_option("host");
	adapter.set_fixed(true);
	adapter.set_option_machine_config("host", interpro_scsi_adapter);

	// ethernet controller
	I82596_LE16(config, m_eth, 20_MHz_XTAL);
	m_eth->out_irq_cb().set(m_ioga, FUNC(interpro_ioga_device::ir12_w));
	m_eth->set_addrmap(0, &sapphire_state::interpro_82596_map);

	// i/o gate array
	SAPPHIRE_IOGA(config, m_ioga, 0);
	m_ioga->set_memory(m_maincpu, 0);
	ioga(config);

	// flash memory
	INTEL_28F010(config, m_flash_lsb);
	INTEL_28F010(config, m_flash_msb);
}

void turquoise_state::ip2000(machine_config &config)
{
	turquoise(config);

	// default is 2020 with GT graphics
	m_kbd_port->set_default_option("lle_en_us");
	m_mse_port->set_default_option("interpro_mouse");

	CBUS_SLOT(config, "slot:0", 0, m_bus, cbus_cards, "mpcb963", false);
	CBUS_SLOT(config, "slot:1", 0, m_bus, cbus_cards, nullptr, false);

	m_softlist->set_filter("2000");
}

void cbus_sapphire_state::cbus_sapphire(machine_config &config)
{
	sapphire(config);

	// keyboard port
	INTERPRO_KEYBOARD_PORT(config, m_kbd_port, interpro_keyboard_devices, nullptr);
	m_kbd_port->rxd_handler_cb().set(m_scc2, FUNC(z80scc_device::rxa_w));
	m_scc2->out_txda_callback().set(m_kbd_port, FUNC(interpro_keyboard_port_device::write_txd));

	// mouse port
	INTERPRO_MOUSE_PORT(config, m_mse_port, interpro_mouse_devices, nullptr);
	m_mse_port->state_func().set(m_ioga, FUNC(interpro_ioga_device::mouse_status_w));

	// cbus bus
	CBUS_BUS(config, m_bus, 0);
	m_bus->set_main_space(m_maincpu, 0);
	m_bus->set_io_space(m_maincpu, 1);

	m_bus->out_irq0_cb().set(m_ioga, FUNC(interpro_ioga_device::ir3_w));
	m_bus->out_irq1_cb().set(m_ioga, FUNC(interpro_ioga_device::ir4_w));
	m_bus->out_irq2_cb().set(m_ioga, FUNC(interpro_ioga_device::ir5_w));
	m_bus->out_irq3_cb().set(m_ioga, FUNC(interpro_ioga_device::ir6_w));
}

void srx_sapphire_state::srx_sapphire(machine_config &config)
{
	sapphire(config);

	// srx bus
	SRX_BUS(config, m_bus, 0);
	m_bus->set_main_space(m_maincpu, 0);
	m_bus->set_io_space(m_maincpu, 1);

	m_bus->out_irq0_cb().set(m_ioga, FUNC(interpro_ioga_device::ir3_w));
	m_bus->out_irq1_cb().set(m_ioga, FUNC(interpro_ioga_device::ir4_w));
	m_bus->out_irq2_cb().set(m_ioga, FUNC(interpro_ioga_device::ir5_w));
	m_bus->out_irq3_cb().set(m_ioga, FUNC(interpro_ioga_device::ir6_w));
}

void cbus_sapphire_state::ip2400(machine_config &config)
{
	cbus_sapphire(config);

	//m_maincpu->set_clock(50_MHz_XTAL);
	m_mmu->set_cammu_id(cammu_c4i_device::CID_C4IR0);

	// default is 2430 with GT+ graphics
	m_kbd_port->set_default_option("lle_en_us");
	m_mse_port->set_default_option("interpro_mouse");

	CBUS_SLOT(config, "slot:0", 0, m_bus, cbus_cards, "msmt070", false);
	CBUS_SLOT(config, "slot:1", 0, m_bus, cbus_cards, nullptr, false);

	m_softlist->set_filter("2400");
}

void cbus_sapphire_state::ip2500(machine_config &config)
{
	cbus_sapphire(config);

	//m_maincpu->set_clock(?);
	// FIXME: don't know which cammu revision
	m_mmu->set_cammu_id(cammu_c4i_device::CID_C4IR2);

	// default is 2530 with GT+ graphics
	m_kbd_port->set_default_option("lle_en_us");
	m_mse_port->set_default_option("interpro_mouse");

	CBUS_SLOT(config, "slot:0", 0, m_bus, cbus_cards, "msmt070", false);
	CBUS_SLOT(config, "slot:1", 0, m_bus, cbus_cards, nullptr, false);

	m_softlist->set_filter("2500");
}

void cbus_sapphire_state::ip2700(machine_config &config)
{
	cbus_sapphire(config);

	//m_maincpu->set_clock(?);
	m_mmu->set_cammu_id(cammu_c4i_device::CID_C4IR2);

	// default is 2730 with GT+ graphics
	m_kbd_port->set_default_option("lle_en_us");
	m_mse_port->set_default_option("interpro_mouse");

	CBUS_SLOT(config, "slot:0", 0, m_bus, cbus_cards, "msmt070", false);
	CBUS_SLOT(config, "slot:1", 0, m_bus, cbus_cards, nullptr, false);

	m_softlist->set_filter("2700");
}

void cbus_sapphire_state::ip2800(machine_config &config)
{
	cbus_sapphire(config);

	//m_maincpu->set_clock(?);
	// FIXME: don't know which cammu revision
	m_mmu->set_cammu_id(cammu_c4i_device::CID_C4IR2);

	// default is 2830 with GT+ graphics
	m_kbd_port->set_default_option("lle_en_us");
	m_mse_port->set_default_option("interpro_mouse");

	CBUS_SLOT(config, "slot:0", 0, m_bus, cbus_cards, "msmt070", false);
	CBUS_SLOT(config, "slot:1", 0, m_bus, cbus_cards, nullptr, false);

	m_softlist->set_filter("2800");
}

void emerald_state::ip6000(machine_config &config)
{
	emerald(config);

	// default is 6040 with EDGE-1 graphics
	SRX_SLOT(config, "slot:1", 0, m_bus, srx_cards, nullptr, false);
	SRX_SLOT(config, "slot:2", 0, m_bus, srx_cards, nullptr, false);
	SRX_SLOT(config, "slot:3", 0, m_bus, srx_cards, nullptr, false);
	SRX_SLOT(config, "slot:4", 0, m_bus, srx_cards, "mpcb828", false);

	m_softlist->set_filter("6000");
}

void srx_sapphire_state::ip6400(machine_config &config)
{
	srx_sapphire(config);

	//m_maincpu->set_clock(36_MHz_XTAL);
	m_mmu->set_cammu_id(cammu_c4i_device::CID_C4IR0);

	// default is 6450 with GT II graphics
	SRX_SLOT(config, "slot:1", 0, m_bus, srx_cards, nullptr, false);
	SRX_SLOT(config, "slot:2", 0, m_bus, srx_cards, nullptr, false);
	SRX_SLOT(config, "slot:3", 0, m_bus, srx_cards, nullptr, false);
	SRX_SLOT(config, "slot:4", 0, m_bus, srx_cards, "mpcbb68", false);

	// EDGE-2 graphics (6480)
	//SRX_SLOT(config, "slot:3", 0, m_bus, srx_cards, "mpcb030", false);
	//SRX_SLOT(config, "slot:4", 0, m_bus, srx_cards, "mpcba63", false);

	m_softlist->set_filter("6400");
}

void srx_sapphire_state::ip6700(machine_config &config)
{
	srx_sapphire(config);

	//m_maincpu->set_clock(?);
	// FIXME: don't know which cammu revision
	m_mmu->set_cammu_id(cammu_c4i_device::CID_C4IR2);

	// default is 6780 with EDGE-2 Plus graphics
	SRX_SLOT(config, "slot:1", 0, m_bus, srx_cards, nullptr, false);
	SRX_SLOT(config, "slot:2", 0, m_bus, srx_cards, nullptr, false);
	SRX_SLOT(config, "slot:3", 0, m_bus, srx_cards, "msmt094", false);
	SRX_SLOT(config, "slot:4", 0, m_bus, srx_cards, "mpcb896", false);

	m_softlist->set_filter("6700");
}

void srx_sapphire_state::ip6800(machine_config &config)
{
	srx_sapphire(config);

	//m_maincpu->set_clock(?);
	// FIXME: don't know which cammu revision
	m_mmu->set_cammu_id(cammu_c4i_device::CID_C4IR2);

	// default is 6880 with EDGE-2 Plus graphics
	SRX_SLOT(config, "slot:1", 0, m_bus, srx_cards, nullptr, false);
	SRX_SLOT(config, "slot:2", 0, m_bus, srx_cards, nullptr, false);
	SRX_SLOT(config, "slot:3", 0, m_bus, srx_cards, "msmt094", false);
	SRX_SLOT(config, "slot:4", 0, m_bus, srx_cards, "mpcb896", false);

	m_softlist->set_filter("6800");
}

ROM_START(ip2000)
	ROM_REGION32_LE(0x80, "nodeid", 0)
	ROM_LOAD32_BYTE("nodeid.bin", 0x0, 0x20, CRC(a38397a6) SHA1(9f45fb932bbe231c95b3d5470dcd1fa1c846486f))

	ROM_REGION32_LE(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("mpcb962a.bin", 0x0, 0x20, CRC(e391342c) SHA1(02e03aad760b6651b8599c3a41c7c457983ee97d))

	ROM_REGION16_LE(0x40000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "ip2000", "InterPro/InterServe 20x0 EPROM")
	ROMX_LOAD("mprgm530e__26_apr_91k.u171", 0x00001, 0x20000, CRC(e4c470cb) SHA1(ff1917bfa963988d739a9dbf0b8f034fe49f2f8c), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("mprgm540e__06_may_91k.u172", 0x00000, 0x20000, CRC(03225843) SHA1(03cfcd5b8ae0057240ef808a40108cb5d082eb63), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

ROM_START(ip2400)
	// feature[0] & 0x02: C4I cammu if set
	ROM_REGION32_LE(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("msmt0470.bin", 0x0, 0x20, CRC(498c80df) SHA1(18a49732ac9d04b20a77747c1b946c2e88abb087))

	ROM_REGION16_LE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "ip2400", "InterPro/InterServe 24x0 EPROM")
	ROMX_LOAD("mprgw510b__05_16_92.u35", 0x00000, 0x20000, CRC(3b2c4545) SHA1(4e4c98d1cd1035a04be8527223f44d0b687ec3ef), ROM_BIOS(0))

	ROM_REGION(0x20000, "flash_lsb", 0)
	ROM_LOAD_OPTIONAL("y225.u76", 0x00000, 0x20000, CRC(46c0b105) SHA1(7c4a104e4fb3d0e5e8db7c911cdfb3f5c4fb0218))

	ROM_REGION(0x20000, "flash_msb", 0)
	ROM_LOAD_OPTIONAL("y226.u67", 0x00000, 0x20000, CRC(54d95730) SHA1(a4e114dee1567d8aa31eed770f7cc366588f395c))
ROM_END

ROM_START(ip2500)
	ROM_REGION32_LE(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("msmt1000.bin", 0x0, 0x20, CRC(548046c0) SHA1(ce7646e868f3aa35642d7f9348f6b9e91693918e))

	// FIXME: undumped - probably identical to ip2700 eprom
	ROM_REGION16_LE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "ip2500", "InterPro/InterServe 25x0 EPROM")
	ROMX_LOAD("ip2500_eprom.bin", 0x00000, 0x20000, CRC(467ce7bd) SHA1(53faee40d5df311f53b24c930e434cbf94a5c4aa) BAD_DUMP, ROM_BIOS(0))

	ROM_REGION(0x20000, "flash_lsb", 0)
	ROM_LOAD_OPTIONAL("y225.u76", 0x00000, 0x20000, CRC(46c0b105) SHA1(7c4a104e4fb3d0e5e8db7c911cdfb3f5c4fb0218))

	ROM_REGION(0x20000, "flash_msb", 0)
	ROM_LOAD_OPTIONAL("y226.u67", 0x00000, 0x20000, CRC(54d95730) SHA1(a4e114dee1567d8aa31eed770f7cc366588f395c))
ROM_END

ROM_START(ip2700)
	// feature[0] & 0x04: supports RETRY if clear
	ROM_REGION32_LE(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("msmt1280.bin", 0x0, 0x20, CRC(32d833af) SHA1(7225c5f5670fe49d86556a2cb453ae6fe09e3e19))

	ROM_REGION16_LE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "ip2700", "InterPro/InterServe 27x0 EPROM")
	ROMX_LOAD("mprgz530a__9405181.u35", 0x00000, 0x20000, CRC(467ce7bd) SHA1(53faee40d5df311f53b24c930e434cbf94a5c4aa), ROM_BIOS(0))

	ROM_REGION(0x20000, "flash_lsb", 0)
	ROM_LOAD_OPTIONAL("y225.u76", 0x00000, 0x20000, CRC(46c0b105) SHA1(7c4a104e4fb3d0e5e8db7c911cdfb3f5c4fb0218))

	ROM_REGION(0x20000, "flash_msb", 0)
	ROM_LOAD_OPTIONAL("y226.u67", 0x00000, 0x20000, CRC(54d95730) SHA1(a4e114dee1567d8aa31eed770f7cc366588f395c))
ROM_END

ROM_START(ip2800)
	ROM_REGION32_LE(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("msmt1450.bin", 0x0, 0x20, CRC(61c7a305) SHA1(efcd045cbdfda8df44eaad761b0ba99367973cd7))

	ROM_REGION16_LE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "ip2800", "InterPro/InterServe 28x0 EPROM")
	ROMX_LOAD("ip2800_eprom.bin", 0x00000, 0x20000, CRC(467ce7bd) SHA1(53faee40d5df311f53b24c930e434cbf94a5c4aa), ROM_BIOS(0))

	ROM_REGION(0x20000, "flash_lsb", 0)
	ROM_LOAD_OPTIONAL("y225.u76", 0x00000, 0x20000, CRC(46c0b105) SHA1(7c4a104e4fb3d0e5e8db7c911cdfb3f5c4fb0218))

	ROM_REGION(0x20000, "flash_msb", 0)
	ROM_LOAD_OPTIONAL("y226.u67", 0x00000, 0x20000, CRC(54d95730) SHA1(a4e114dee1567d8aa31eed770f7cc366588f395c))
ROM_END

ROM_START(ip6000)
	ROM_REGION32_LE(0x80, "nodeid", 0)
	ROM_LOAD32_BYTE("nodeid.bin", 0x0, 0x20, CRC(a38397a6) SHA1(9f45fb932bbe231c95b3d5470dcd1fa1c846486f))

	// feature[0] & 0x01: 1/4 bus clock if clear
	// feature[0] & 0x02: configurable console port if clear
	ROM_REGION32_LE(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("mpcb765b.bin", 0x0, 0x20, CRC(6da05794) SHA1(fef8a9c17491f3d3ceb35c56a628f47d49166b57))

	ROM_REGION16_LE(0x40000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "ip6000", "InterPro/InterServe 60x0 EPROM")
	ROMX_LOAD("mprgg360f__04_may_90v.u336", 0x00001, 0x20000, CRC(9e8b798b) SHA1(54412e26a468e038fb44ffa322ed3ddfae423c17), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("mprgg350f__04_may_90v.u349", 0x00000, 0x20000, CRC(32ab99fd) SHA1(202a6082bade8a084b8cd25109daff8443f6a5c7), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

ROM_START(ip6400)
	ROM_REGION32_LE(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("msmt046b.bin", 0x0, 0x20, CRC(3e8ffc77) SHA1(719a3f8b01bb506c9cb876506d63d167550bcd0a))

	// FIXME: use 2400 eprom until we have a 6400 dump
	ROM_REGION16_LE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "ip6400", "InterPro/InterServe 64x0 EPROM")
	ROMX_LOAD("ip6400_eprom.bin", 0x00000, 0x20000, BAD_DUMP CRC(3b2c4545) SHA1(4e4c98d1cd1035a04be8527223f44d0b687ec3ef), ROM_BIOS(0))

	ROM_REGION(0x20000, "flash_lsb", 0)
	ROM_LOAD_OPTIONAL("flash.lsb", 0x00000, 0x20000, CRC(46c0b105) SHA1(7c4a104e4fb3d0e5e8db7c911cdfb3f5c4fb0218))

	ROM_REGION(0x20000, "flash_msb", 0)
	ROM_LOAD_OPTIONAL("flash.msb", 0x00000, 0x20000, CRC(54d95730) SHA1(a4e114dee1567d8aa31eed770f7cc366588f395c))
ROM_END

ROM_START(ip6700)
	ROM_REGION32_LE(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("msmt127b.bin", 0x0, 0x20, CRC(cc112f65) SHA1(8533a31b4733fd91bb87effcd276fc93f2858629))

	ROM_REGION16_LE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "ip6700", "InterPro/InterServe 67x0 EPROM")
	ROMX_LOAD("mprgz530a.u144", 0x00000, 0x20000, CRC(467ce7bd) SHA1(53faee40d5df311f53b24c930e434cbf94a5c4aa), ROM_BIOS(0))

	ROM_REGION(0x20000, "flash_lsb", 0)
	ROM_LOAD_OPTIONAL("y225.u117", 0x00000, 0x20000, CRC(46c0b105) SHA1(7c4a104e4fb3d0e5e8db7c911cdfb3f5c4fb0218))

	ROM_REGION(0x20000, "flash_msb", 0)
	ROM_LOAD_OPTIONAL("y226.u130", 0x00000, 0x20000, CRC(54d95730) SHA1(a4e114dee1567d8aa31eed770f7cc366588f395c))
ROM_END

ROM_START(ip6800)
	ROM_REGION32_LE(0x80, "idprom", 0)
	ROM_LOAD32_BYTE("msmt127b.bin", 0x0, 0x20, CRC(cc112f65) SHA1(8533a31b4733fd91bb87effcd276fc93f2858629))

	ROM_REGION16_LE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "ip6800", "InterPro/InterServe 68x0 EPROM")
	ROMX_LOAD("mprgz530a__9406270.u144", 0x00000, 0x20000, CRC(467ce7bd) SHA1(53faee40d5df311f53b24c930e434cbf94a5c4aa), ROM_BIOS(0))

	ROM_REGION(0x20000, "flash_lsb", 0)
	ROM_LOAD_OPTIONAL("y225.u117", 0x00000, 0x20000, CRC(46c0b105) SHA1(7c4a104e4fb3d0e5e8db7c911cdfb3f5c4fb0218))

	ROM_REGION(0x20000, "flash_msb", 0)
	ROM_LOAD_OPTIONAL("y226.u130", 0x00000, 0x20000, CRC(54d95730) SHA1(a4e114dee1567d8aa31eed770f7cc366588f395c))
ROM_END

}

/*    YEAR   NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS                INIT        COMPANY        FULLNAME                    FLAGS */
COMP( 1990,  ip2000,  0,      0,      ip2000,  0,     turquoise_state,     init_common,"Intergraph",  "InterPro/InterServe 20x0", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1992,  ip2400,  0,      0,      ip2400,  0,     cbus_sapphire_state, init_common,"Intergraph",  "InterPro/InterServe 24x0", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1993,  ip2500,  0,      0,      ip2500,  0,     cbus_sapphire_state, init_common,"Intergraph",  "InterPro/InterServe 25x0", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1993,  ip2700,  0,      0,      ip2700,  0,     cbus_sapphire_state, init_common,"Intergraph",  "InterPro/InterServe 27x0", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1994,  ip2800,  0,      0,      ip2800,  0,     cbus_sapphire_state, init_common,"Intergraph",  "InterPro/InterServe 28x0", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1990,  ip6000,  0,      0,      ip6000,  0,     emerald_state,       init_common,"Intergraph",  "InterPro/InterServe 60x0", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1992,  ip6400,  0,      0,      ip6400,  0,     srx_sapphire_state,  init_common,"Intergraph",  "InterPro/InterServe 64x0", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1993,  ip6700,  0,      0,      ip6700,  0,     srx_sapphire_state,  init_common,"Intergraph",  "InterPro/InterServe 67x0", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1993,  ip6800,  0,      0,      ip6800,  0,     srx_sapphire_state,  init_common,"Intergraph",  "InterPro/InterServe 68x0", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
