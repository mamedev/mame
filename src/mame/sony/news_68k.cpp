// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay,Brice Onken

/*
   Sony NEWS single-processor 68k systems

   Unlike the original model of the Sony NEWS (NWS-800 series), most of the NWS-1xxx series (with the exception of the
   NWS-18xx/19xx) are single-processor 68030-based workstations. Sony developed a new family of workstations, popNEWS,
   that included extra software including NEWS Desk, a suite of X11 applications. By the time NEWS-OS 4 came out, NEWS
   Desk had proliferated across the entire lineup.

   The NWS-12xx series uses the MPU-13 motherboard. The NWS-14xx, NWS-15xx, and PWS-15xx series all use the MPU-7
   motherboard. The PWS-16xx and NWS-17xx series use the MPU-8 motherboard.

   Supported:
    - NWS-1250
    - NWS-1580

   Not supported yet:
    - NWS-1230
    - All other NWS/PWS-15xx workstations
    - NWS-14xx/17xx series
    - PWS-16xx series

   Known NWS-1200 Series Base Configurations
    - NWS-1230: Aug 1990, 4MB RAM (up to 12MB), 200MB HDD
    - NWS-1250: Jul 1990, 8MB RAM (up to 12MB), 240MB HDD

   Known NWS-1500 Series Base Configurations
    - NWS-1510: ??? 19??, 4MB RAM (up to 16MB), 40MB HDD, NEWS-OS, optional display
    - NWS-1520: ??? 19??, 4MB RAM (up to 16MB), diskless, NEWS-OS, monochrome display
    - PWS-1520: Jan 1989, 4MB RAM (up to 16MB), 40MB HDD, POP-OS, monochrome display
    - NWS-1530: ??? 19??, 4MB RAM (up to 16MB), 40MB HDD, NEWS-OS, color display
    - PWS-1550: Nov 1988, 4MB RAM (up to 16MB), 91MB HDD, POP-OS + Media Bank, monochrome display
    - PWS-1560: Apr 1989, 4MB RAM (up to 16MB), 91MB HDD, POP-OS + Media Bank, color display
    - NWS-1580: ??? 1988, 4MB RAM (up to 16MB), 170MB HDD, NEWS-OS, color display

   Sources:
     - http://wiki.netbsd.org/ports/news68k/
     - http://bitsavers.org/pdf/sony/news/Sony_NEWS_Technical_Manual_3ed_199103.pdf
     - https://katsu.watanabe.name/doc/sonynews/model.html
     - https://goodoldbits.wordpress.com/2016/05/25/news-5-pop-news/

   TODO:
     - Desktop mouse/keyboard
     - Desktop graphics
     - Sound
     - Expansion slots

   The Sony NEWS Portable Workstation NWS-1250 is a "laptop" (weight of more than 8 Kg) with a black-and-white LCD (1120×780),
   a keyboard, a 3.5″ floppy disk drive, a (SCSI) harddisk, and interfaces for mouse, audio (phones, line in, mic in), SCSI,
   Ethernet (AUI), serial (DB9), and parallel (proprietary).

   This is its main PCB layout:
   _____________________________________________________________________________________________________________________________________________
  |               __________   _____________________   _______  _______               ____________________________    _____                    |
  |              |MB834200A|  | EPROM AM27C1024    |  |HC257_| |HC257_|              |||||||||||||||||||||||||||||   |·····|                   |
  |              |         |  |                    |   _______  _______            __________       _________       _________                  |
  |              |_________|  |____________________|  |HC257_| |HC257_|           |         | Xtal | Sony   | Xtal | Sony   |  ___             |
  |                            _______                                            |HD64646FS|  19  |WSC-AIF2|  741 |CDX1123 | |  |<-74HC244A   |__
  |               __________  |74HC32A  __________   _______                      |_________|      |        |      |        | |__|              __|_
  |              |MB834200A|   _______ |Intel    |  ACT11004                     ___ ___           |________|      |________|                  |____|
__|              |         |  |ALS05A| |N82077   |   __             6 x 74F00J->|  | |  |   _________________________________________          __ |
|   ________     |_________|   _______ |         |  |-|                         |__| |__|   |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|         | | |
|  |74ACT139                  |74LS14| |_________|  |-| __________  __________   ___ ___    _________________________________________ DIPSx8->| | |
|   ________                                        |-||HM62256LFP |HM62256LFP  |  | |  |   |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|         |_| |
|  74ACT11244               ____________   ___      |-||_________| |_________|  |__| |__|   _________________________________________        ___  |
|   ________               | Sony      |  |  |      |-|                          ___ ___    |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|       T7705 |
|  74ACT11244              |WSC-MEMPAK |  |  |      |-|    ____________         |  | |  |   _________________________________________         __  |
|   ________               |9030EK712  |  |__|      |_|   | Sony      |         |__| |__|   |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_| Switch->__| |
|  74ACT11244       Xtal   |           | SG51KH           |WSC-LCMC   |               __    _________________________________________         __  |
|                 4915.2   |___________| 50 MHz           |9025EK442  |              |..|   |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_| Switch->__| |
|   ________                    ________                  |           |              |..|   _________________________________________         __  |
|  |74ACT139                   |ACT11244   ________       |___________|              |..|   |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_| Switch->__| |
|   ________                              |ACT11245                    :|            |..|   _________________________________________        ___  |
|  |ACT1124             ________________   ________                 :| :|            |..|   |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|      74AS00 |
|   ________           | MK48T02B-25   |  |ACT11245                 :| :|            |..|   _________________________________________             |
|  |AM27S21PC                                                                        |__|   |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|             |
|  _____________________________________       ________  ________  ________            ________   ________   ________   ________   ______________ |
|_|                                    |____  |ACT11353 |ACT11353 |ACT11020           |AC11004|  |AC11004|  |AC11004|  |AC11004|  |o o o o o o o ||
  |____________________________________|    |  ________  ________                ___    ___    ___    ___    ___    _________     ________        |
                                            | |ACT11353 |ACT11353               |  |   |  |   |  |   |  |   |  |   |Sony    |    |AC11004|        |
                                            |  ________  ________  ________    BCT245 BCT245 BCT245 BCT245 BCT245  |WSC-LANCE   __________        |
                                            | |ACT11353 |ACT11353 |ACT11027     |__|   |__|   |__|   |__|   |__|   |________| HM6264ALFP-12T      |
                                            |                                                                                   __________    ___ |
                                            | _____________       _____________      _____________                            HM6264ALFP-12T |__<-SG51KH 32 MHz
                                            || Motorola   |      | Motorola   |     | Sony       |                  __________________________    |
                                            ||MC68882FN25A|      |XC68030FE25B|     |L7A0266     |                 | AMD                     |    |
                                            ||            |      |            |     |WSC-ICKDMAC |                 | AM7990PC                |    |
                                            ||            |      |            |     |9019        |                 |_________________________|    |
                                            ||____________|      |____________|     |____________|          ___________    ___________            |
                                            |  ___________   ________   ________                           |Sony      |   |AM7992BDC_|            |
                                            | | Zilog    |  |74HC374|  74HCT244A       ________   ________ |CXD1185Q  |    _________              |
                                            | |Z85C3008VSC                ________    |74ACT139  |ACT11002 |__________|   SG51K 20 MHz            |
                                            |                            |MC1489A|                                                                |
                                            |             ________        _____________________________________                                   |
                                            |            |MC145406       |::::::::::::::::::::::::::::::::::::|                                   |
                                            |  __________   __________   _______________________                 __________________               |
                                            |_|         |__|         |__|                      |________________|                 |_______________|
                                              |_________|  |_________|  |______________________|                |_________________|

 On the other side of the PCB there are a few components too:
  - 3 x HM62256LFP-12T
  - 1 x DS1000S-50

 NWS-1250 came bundled with a Sony mouse based on a Fujitsu MB88201H MCU (undumped mask ROM 512 x 8 bits).

 */

#include "emu.h"

#include "cpu/m68000/m68030.h"

// memory
#include "machine/ram.h"

// various hardware
#include "machine/timekpr.h"
#include "machine/z80scc.h"
#include "machine/am79c90.h"
#include "machine/upd765.h"
#include "dmac_0266.h"
#include "news_hid.h"
#include "machine/ncr5380.h"
#include "machine/cxd1185.h"

// laptop video
#include "news_lcdfb.h"

// desktop video
#define DESKTOP_GRAPHICS 0
#if DESKTOP_GRAPHICS
#include "screen.h"
#include "video/bt45x.h"
#endif

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/centronics/ctronics.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"
#include "bus/rs232/rs232.h"

#include "machine/input_merger.h"
#include "imagedev/floppy.h"

#define LOG_INTERRUPT (1U << 1)
#define LOG_TIMER (1U << 2)
#define LOG_PARALLEL (1U << 3)
#define VERBOSE 0
#include "logmacro.h"

namespace {

class news_68k_base_state : public driver_device
{
public:
	news_68k_base_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_dma(*this, "dma")
		, m_rtc(*this, "rtc")
		, m_scc(*this, "scc")
		, m_net(*this, "net")
		, m_fdc(*this, "fdc")
		, m_hid(*this, "hid")
		, m_scsibus(*this, "scsi")
		, m_serial(*this, "serial%u", 0U)
		, m_parallel(*this, "parallel")
		, m_parallel_data(*this, "parallel_data")
		, m_irq5(*this, "irq5")
		, m_irq7(*this, "irq7")
		, m_sw1(*this, "SW1")
		, m_eprom(*this, "eprom")
		, m_led(*this, "led%u", 0U)
	{
	}

	void init_common() ATTR_COLD;

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;

	// address maps
	virtual void cpu_autovector_map(address_map &map) ATTR_COLD;

	// machine config
	void common(machine_config &config);
	void config_scc(machine_config &config, char const *default_device_name, int scc_clock);

	// IRQ setup
	enum irq_number : unsigned
	{
		IPIRQ1     = 0,
		IPIRQ3     = 1,
		LANCE      = 2,
		VME4_AUDIO = 3, // Desktop = VME4, Laptop = Audio
		VME2       = 4,
		FDC        = 5,
		PRINTER    = 6,
		SCSI       = 7,
	};
	template <irq_number Number> void irq_w(int state);
	void int_check();

	// platform hardware
	u32 bus_error_r();
	u8 intst_r();
	void ast_w(u8 data);
	void irq2_w(u8 data);
	void led_w(u8 data);
	void poweron_w(u8 data);
	void ram_enable_w(u8 data);
	void parallel_irq_enable_w(u8 data);
	void parallel_data_w(u8 data);
	void parallel_strobe_w(u8 data);

	// devices
	required_device<m68030_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<dmac_0266_device> m_dma;
	required_device<m48t02_device> m_rtc;
	required_device<z80scc_device> m_scc;
	required_device<am7990_device> m_net;
	required_device<upd765_family_device> m_fdc;
	required_device<news_hid_hle_device> m_hid;
	required_device<nscsi_bus_device> m_scsibus;

	required_device_array<rs232_port_device, 2> m_serial;
	required_device<centronics_device> m_parallel;
	optional_device<output_latch_device> m_parallel_data;

	required_device<input_merger_device> m_irq5;
	required_device<input_merger_device> m_irq7;
	required_ioport m_sw1;

	required_region_ptr<u32> m_eprom;
	std::unique_ptr<u16[]> m_net_ram;
	output_finder<2> m_led;

	emu_timer *m_timer;

	// platform hardware state
	u8 m_intst = 0;
	bool m_int_state[2] = {false, false};
	bool m_scc_irq_state = false;
	bool m_parallel_irq_enabled = false;
	bool m_parallel_busy = false;
	bool m_parallel_fault = false;
};

class news_68k_desktop_state : public news_68k_base_state
{
public:
	news_68k_desktop_state(machine_config const &mconfig, device_type type, char const *tag)
		: news_68k_base_state(mconfig, type, tag)
		, m_scsi(*this, "cxd1180")
#if DESKTOP_GRAPHICS
		, m_screen(*this, "screen")
		, m_ramdac(*this, "ramdac")
		, m_vram(*this, "vram")
#endif
	{
	}

	void nws1580(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void desktop_cpu_map(address_map &map);
	virtual void cpu_autovector_map(address_map &map) override ATTR_COLD;
	TIMER_CALLBACK_MEMBER(timer);
	void timer_w(u8 data);

#if DESKTOP_GRAPHICS
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect) { return 0; }
#endif

	// Desktop-specific devices
	required_device<cxd1180_device> m_scsi;

#if DESKTOP_GRAPHICS
	required_device<screen_device> m_screen;
	required_device<bt458_device> m_ramdac;
	required_device<ram_device> m_vram;
#endif

	// Desktop-specific platform hardware
	bool m_parity_irq_state = false;
	u8 m_parity_vector = 0;
};

class news_68k_laptop_state : public news_68k_base_state
{
public:
	news_68k_laptop_state(machine_config const &mconfig, device_type type, char const *tag)
	: news_68k_base_state(mconfig, type, tag)
	, m_scsi(*this, "cxd1185")
	, m_lcd(*this, "lcd")
	, m_vram(*this, "vram")
	{
	}

	void nws1250(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void laptop_cpu_map(address_map &map);
	virtual void cpu_autovector_map(address_map &map) override ATTR_COLD;
	TIMER_CALLBACK_MEMBER(timer);
	void timer_w(offs_t offset, u8 data);

	// Laptop-specific devices
	required_device<cxd1185_device> m_scsi;
	required_device<news_lcd_device> m_lcd;
	required_shared_ptr<u32> m_vram;

	// Laptop-specific platform hardware
	u16 m_timer_rate = 0;
};

void news_68k_base_state::machine_start()
{
	// Initialize state
	u32 constexpr NET_RAM_SIZE = 8192;
	m_net_ram = std::make_unique<u16[]>(NET_RAM_SIZE);

	m_scc_irq_state = false;
	m_intst = 0;
	m_parallel_irq_enabled = false;

	for (bool &int_state : m_int_state)
		int_state = false;

	// Save state support
	save_pointer(NAME(m_net_ram), NET_RAM_SIZE);
	save_item(NAME(m_scc_irq_state));
	save_item(NAME(m_intst));
	save_item(NAME(m_int_state));
	save_item(NAME(m_parallel_irq_enabled));
	save_item(NAME(m_parallel_busy));
	save_item(NAME(m_parallel_fault));
}

void news_68k_desktop_state::machine_start()
{
	news_68k_base_state::machine_start();
	m_timer = timer_alloc(FUNC(news_68k_desktop_state::timer), this);

	m_parity_irq_state = false;
	m_parity_vector = 0;

	save_item(NAME(m_parity_irq_state));
	save_item(NAME(m_parity_vector));
}

void news_68k_laptop_state::machine_start()
{
	news_68k_base_state::machine_start();

	m_timer = timer_alloc(FUNC(news_68k_laptop_state::timer), this);

	m_timer_rate = 0;
	save_item(NAME(m_timer_rate));
}

void news_68k_desktop_state::machine_reset()
{
	// eprom is mapped at 0 after reset
	m_cpu->space(0).install_rom(0x00000000, 0x0000ffff, m_eprom);
}

void news_68k_laptop_state::machine_reset()
{
	// eprom is mapped at 0 after reset
	m_cpu->space(0).install_rom(0x00000000, 0x0001ffff, m_eprom);
}

void news_68k_base_state::init_common()
{
	// HACK: hardwire the rate
	m_fdc->set_rate(500000);
}

void news_68k_desktop_state::desktop_cpu_map(address_map &map)
{
	map(0xe0000000, 0xe000ffff).rom().region("eprom", 0);

	map(0xe0c40000, 0xe0c40000).w(FUNC(news_68k_desktop_state::parallel_data_w));
	map(0xe0c40001, 0xe0c40001).w(FUNC(news_68k_desktop_state::parallel_strobe_w));
	map(0xe0c40002, 0xe0c40002).lw8([this] (u8) { irq_w<PRINTER>(0); }, "parallel_irq_clear");
	map(0xe0c40003, 0xe0c40003).w(FUNC(news_68k_desktop_state::parallel_irq_enable_w));

	map(0xe0c80000, 0xe0c80003).m(m_fdc, FUNC(upd72067_device::map));
	map(0xe0c80100, 0xe0c80100).rw(m_fdc, FUNC(upd72067_device::dma_r), FUNC(upd72067_device::dma_w));
	map(0xe0cc0000, 0xe0cc0007).m(m_scsi, FUNC(ncr5380_device::map));

	map(0xe0d00000, 0xe0d00007).m(m_hid, FUNC(news_hid_hle_device::map_68k));
	map(0xe0d40000, 0xe0d40003).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));
	map(0xe0d80000, 0xe0d807ff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write));
	map(0xe0dc0000, 0xe0dc0000).lw8([this](u8 data) { m_led[0] = BIT(data, 0); m_led[1] = BIT(data, 1); }, "led_w");

	map(0xe0e00000, 0xe0e03fff).lrw16(
		[this](offs_t offset) { return m_net_ram[offset]; }, "net_ram_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset]); }, "net_ram_w");
	map(0xe0e80000, 0xe0e80017).m(m_dma, FUNC(dmac_0266_device::map));
	// e0ec0000 // sound board

	map(0xe0f00000, 0xe0f00003).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	map(0xe0f40000, 0xe0f40000).lr8(
		[this] { return (m_parallel_busy ? 0x10 : 0x0) | (m_intst & 1 << PRINTER ? 0x8 : 0x0); },
		"parallel_status_r");

	//map(0xe0f40000, 0xe0f40000).lr8([]() { return 0xfb; }, "scc_ridsr_r");

	map(0xe1000000, 0xe1000000).w(FUNC(news_68k_desktop_state::timer_w));
	map(0xe1080000, 0xe1080000).lw8([this](u8 data) { LOG("parity check enable 0x%02x\n", data); }, "parity_check_enable_w");
	map(0xe1180000, 0xe1180000).w(FUNC(news_68k_desktop_state::irq2_w));
	map(0xe1200000, 0xe1200000).w(FUNC(news_68k_desktop_state::ram_enable_w));
	map(0xe1280000, 0xe1280000).w(FUNC(news_68k_desktop_state::ast_w));
	map(0xe1300000, 0xe1300000).lw8([this](u8 data) { LOG("cache enable 0x%02x (%s)\n", data, machine().describe_context()); }, "cache_enable_w");
	map(0xe1380000, 0xe1380000).w(FUNC(news_68k_desktop_state::poweron_w));
	map(0xe1900000, 0xe1900000).lw8([this](u8 data) { LOG("cache clear 0x%02x\n", data); }, "cache_clear_w");
	map(0xe1a00000, 0xe1a00000).lw8([this](u8 data) { LOG("parity interrupt clear 0x%02x\n", data); }, "parity_interrupt_clear_w");
	// 0xe1b00000 // fdc vfo external/internal
	map(0xe1c00000, 0xe1c000ff).rom().region("idrom", 0);
	map(0xe1c00100, 0xe1c00103).lr8([this]() { return u8(m_sw1->read()); }, "sw1_r");
	map(0xe1c00200, 0xe1c00200).r(FUNC(news_68k_desktop_state::intst_r));
	// HACK: disable fdc irq for NetBSD
	map(0xe1c00200, 0xe1c00200).lw8([this](u8 data) { irq_w<FDC>(0); m_parity_vector = data; }, "parity_vector_w");
	map(0xe1c00300, 0xe1c00300).lr8([this] { return m_parallel_fault ? 0x40 : 0x0; }, "parallel_fault_r");

	// external I/O
	map(0xf0000000, 0xffffffff).r(FUNC(news_68k_desktop_state::bus_error_r));

#if DESKTOP_GRAPHICS
	// POPC
	//map(0xf0fc0000, 0xf0fc0003).unmaprw();
	// f0fc0000 & 0x40 == 0x00 -> popm
	// f0fc0000 & 0xc0 == 0xc0 -> popc
	map(0xf0fc0000, 0xf0fc0001).lr16([]() {return 0x00c0; }, "popc_probe"); // lower 2 bits give busy state
	map(0xf0fc4000, 0xf0fc4007).m(m_ramdac, FUNC(bt458_device::map)).umask32(0x00ff00ff);

	//map(0xf0fc0000, 0xf10bffff).rom().region("krom", 0);
#endif

	// 0xf0c30000 expansion lance #1
	// 0xf0c20000   lance #1 memory
	// 0xf0c38000   lance #1 etherid
	// 0xf0c70000 expansion lance #2
	// 0xf0c60000   lance #2 memory
	// 0xf0c78000   lance #2 etherid

	// 0xf0d04000 isdn?

	// 0xf0f00000 nwb512_base
	// 0xf0fc0000 nwb512krom_base
	// 0xf0700000 nwb225_base
	// 0xf0600000 nwb225krom_base
}

void news_68k_laptop_state::laptop_cpu_map(address_map &map)
{
	map(0xe0000000, 0xe001ffff).rom().region("eprom", 0);

	map(0xe1000000, 0xe1000000).w(FUNC(news_68k_laptop_state::poweron_w));
	map(0xe1040000, 0xe1040000).w(FUNC(news_68k_laptop_state::ram_enable_w)); // This is an educated guess
	map(0xe10c0000, 0xe10c0000).w(FUNC(news_68k_laptop_state::irq2_w));
	map(0xe1100000, 0xe1100000).w(FUNC(news_68k_laptop_state::ast_w));
	map(0xe1140000, 0xe1140003).w(FUNC(news_68k_laptop_state::timer_w));
	map(0xe11c0000, 0xe11c000f).nopw(); // ABORTCTL
	map(0xe1200000, 0xe1200000).r(FUNC(news_68k_laptop_state::intst_r));

	map(0xe1240000, 0xe1240007).m(m_hid, FUNC(news_hid_hle_device::map_nws12xx_keyboard));
	map(0xe1280000, 0xe1280007).m(m_hid, FUNC(news_hid_hle_device::map_nws12xx_mouse));

	map(0xe12c0000, 0xe12c0000).lr8([this] {
			return (m_parallel_fault ? 0x4 : 0x0) | (m_parallel_busy ? 0x2 : 0x0) | (m_intst & 1 << PRINTER ? 0x1 : 0x0);
		}, "parallel_status_r");
	map(0xe12c0001, 0xe12c0001).w(FUNC(news_68k_laptop_state::parallel_strobe_w));
	map(0xe12c0002, 0xe12c0002).w(FUNC(news_68k_laptop_state::parallel_irq_enable_w));
	map(0xe12c0003, 0xe12c0003).lw8([this] (u8) { irq_w<PRINTER>(0); }, "parallel_irq_clear");

	map(0xe1400000, 0xe14000ff).rom().region("idrom", 0);
	map(0xe1420000, 0xe14207ff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write));
	map(0xe1580000, 0xe1580007).m(m_fdc, FUNC(n82077aa_device::map));
	map(0xe15c0000, 0xe15c0000).rw(m_fdc, FUNC(n82077aa_device::dma_r), FUNC(n82077aa_device::dma_w));

	map(0xe1600000, 0xe1600000).w(FUNC(news_68k_laptop_state::parallel_data_w));

	// The 0-3 is important here - the monitor ROM and NEWS-OS use different bytes to read the switch values.
	map(0xe1680000, 0xe1680003).lr8([this] { return m_sw1->read(); }, "sw1_r");

	map(0xe1780000, 0xe1780003).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));
	map(0xe1900000, 0xe190000f).m(m_scsi, FUNC(cxd1185_device::map));
	map(0xe1c00000, 0xe1c00017).m(m_dma, FUNC(dmac_0266_device::map));

	map(0xe1a00000, 0xe1a03fff).lrw16(
		[this](offs_t offset) { return m_net_ram[offset]; }, "net_ram_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset]); }, "net_ram_w");
	map(0xe1a40000, 0xe1a40003).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));

	map(0xe2000000, 0xe20fffff).rom().region("krom", 0);
	map(0xe4000000, 0xe401ffff).ram().share("vram");

	map(0xe1480000, 0xe148001f).m(m_lcd, FUNC(news_lcd_device::map_lctc));

	// TODO: Unsure what 0xe1500000 is - the monitor ROM sets it to 0x2 before memory probe, and clears it afterwards.
	map(0xe1500001, 0xe1500001).w(FUNC(news_68k_laptop_state::led_w));
	map(0xe1500002, 0xe1500002).w(m_lcd, FUNC(news_lcd_device::lcd_enable_w));

	// external I/O
	map(0xf0000000, 0xffffffff).r(FUNC(news_68k_laptop_state::bus_error_r));
}

void news_68k_base_state::cpu_autovector_map(address_map &map)
{
	map(0xfffffff3, 0xfffffff3).lr8(NAME([]() { return m68000_base_device::autovector(1); }));
	map(0xfffffff5, 0xfffffff5).lr8(NAME([]() { return m68000_base_device::autovector(2); }));
	map(0xfffffff7, 0xfffffff7).lr8(NAME([]() { return m68000_base_device::autovector(3); }));
	map(0xfffffff9, 0xfffffff9).lr8(NAME([]() { return m68000_base_device::autovector(4); }));
	map(0xfffffffb, 0xfffffffb).lr8(NAME([this]() { return m_scc_irq_state ? m_scc->m1_r() : m68000_base_device::autovector(5); }));
	map(0xfffffffd, 0xfffffffd).lr8(NAME([]() { return m68000_base_device::autovector(6); }));
}

void news_68k_desktop_state::cpu_autovector_map(address_map &map)
{
	news_68k_base_state::cpu_autovector_map(map);
	map(0xffffffff, 0xffffffff).lr8(NAME([this]() { return m_parity_irq_state ? m_parity_vector : m68000_base_device::autovector(7); }));
}

void news_68k_laptop_state::cpu_autovector_map(address_map &map)
{
	news_68k_base_state::cpu_autovector_map(map);
	map(0xffffffff, 0xffffffff).lr8(NAME([]() { return m68000_base_device::autovector(7); }));
}

template <news_68k_base_state::irq_number Number> void news_68k_base_state::irq_w(int state)
{
	LOGMASKED(LOG_INTERRUPT, "irq number %d state %d\n", Number, state);

	if (state)
		m_intst |= 1U << Number;
	else
		m_intst &= ~(1U << Number);

	int_check();
}

void news_68k_base_state::int_check()
{
	// TODO: assume 43334443, masking?
	static int constexpr int_line[] = { INPUT_LINE_IRQ3, INPUT_LINE_IRQ4 };
	u8 const int_mask[] = { u8(0x31 | (m_parallel_irq_enabled ? 0x40 : 0x0)), 0x8e};

	for (unsigned i = 0; i < std::size(m_int_state); i++)
	{
		bool const int_state = m_intst & int_mask[i];

		if (m_int_state[i] != int_state)
		{
			m_int_state[i] = int_state;
			m_cpu->set_input_line(int_line[i], int_state);
		}
	}
}

u32 news_68k_base_state::bus_error_r()
{
	if (!machine().side_effects_disabled())
		m_cpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);

	return 0;
}

u8 news_68k_base_state::intst_r()
{
	return m_intst;
}

void news_68k_base_state::ast_w(u8 data)
{
	// TODO: The NWS-800/3800 only trigger AST once the system transitions to user mode (FC) - is the same true here?
	bool const ast = bool(data);
	LOGMASKED(LOG_INTERRUPT, "(%s) %s AST\n", machine().describe_context(), ast ? "Set" : "Cleared");
	m_cpu->set_input_line(INPUT_LINE_IRQ1, ast);
}

void news_68k_base_state::irq2_w(u8 data)
{
	bool const irq2 = bool(data);
	LOGMASKED(LOG_INTERRUPT, "(%s) %s IRQ2\n", machine().describe_context(), irq2 ? "Set" : "Cleared");
	m_cpu->set_input_line(INPUT_LINE_IRQ2, irq2);
}

void news_68k_base_state::led_w(u8 data)
{
	m_led[0] = BIT(data, 0);
	m_led[1] = BIT(data, 1);
}

void news_68k_base_state::poweron_w(u8 data)
{
	LOG("(%s) Write POWERON = 0x%x\n", machine().describe_context(), data);

	if (!machine().side_effects_disabled() && !data)
	{
		machine().schedule_exit();
	}
}

void news_68k_base_state::ram_enable_w(u8 data)
{
	LOG("(%s) Enabled RAM\n", machine().describe_context());
	m_cpu->space(0).install_ram(0, m_ram->mask(), 0xc0000000, m_ram->pointer());
}

void news_68k_base_state::parallel_irq_enable_w(u8 data)
{
	LOGMASKED(LOG_PARALLEL, "(%s) %s parallel interrupt", machine().describe_context(), data ? "enabled" : "disabled");
	m_parallel_irq_enabled = data;
}

void news_68k_base_state::parallel_data_w(u8 data)
{
	LOGMASKED(LOG_PARALLEL, "Parallel data w 0x%x\n", data);
	m_parallel_data->write(data);
}

void news_68k_base_state::parallel_strobe_w(u8 data)
{
	LOG("Parallel strobe w 0x%x\n", data);
	// TODO: NEWS-OS writes 0x5 to this (laptop) - what strobe length does that correspond to?
	//       For now, this takes the HLE approach.
	m_parallel->write_strobe(1);
	m_parallel->write_strobe(0);
	m_parallel->write_strobe(1);
}

void news_68k_desktop_state::timer_w(u8 data)
{
	LOGMASKED(LOG_TIMER, "timer_w 0x%02x\n", data);

	if (data)
		m_timer->adjust(attotime::from_hz(100));
	else
		m_cpu->set_input_line(INPUT_LINE_IRQ6, CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(news_68k_desktop_state::timer)
{
	m_cpu->set_input_line(INPUT_LINE_IRQ6, ASSERT_LINE);
}

void news_68k_laptop_state::timer_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0:
			LOGMASKED(LOG_TIMER, "timer %s\n", data ? "enabled" : "disabled");
			if (data)
			{
				// TODO: conversion factor calculated assuming NEWS-OS programs timer to get 100Hz as is common on NEWS
				//       systems - need real HW info
				double constexpr TIMER_CONVERSION_FACTOR = 0.102459016;
				auto const rate = attotime::from_hz(m_timer_rate * TIMER_CONVERSION_FACTOR);
				LOGMASKED(LOG_TIMER, "Enabling timer at rate %f Hz\n", rate.as_hz());
				m_timer->adjust(rate, 0, rate);
			}
			else
			{
				m_timer->adjust(attotime::never);
				m_cpu->set_input_line(INPUT_LINE_IRQ6, CLEAR_LINE);
			}
			break;
		case 1:
			if (data)
				m_cpu->set_input_line(INPUT_LINE_IRQ6, CLEAR_LINE);
			break;
		default:
			bool const is_msb = offset == 2;
			m_timer_rate &= 0xff << (is_msb ? 0 : 8);
			m_timer_rate |= data << (is_msb ? 8 : 0);
			LOGMASKED(LOG_TIMER, "timer_w set rate = 0x%x\n", m_timer_rate);
			break;
	}
}

TIMER_CALLBACK_MEMBER(news_68k_laptop_state::timer)
{
	m_cpu->set_input_line(INPUT_LINE_IRQ6, ASSERT_LINE);
}

static void news_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM_NEWS);
	device.option_add("tape", NSCSI_TAPE_NEWS);
}

void news_68k_base_state::common(machine_config &config)
{
	M48T02(config, m_rtc);

	DMAC_0266(config, m_dma);
	m_dma->set_bus(m_cpu, 0);

	INPUT_MERGER_ANY_HIGH(config, m_irq5);
	m_irq5->output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ5);

	AM7990(config, m_net, 20_MHz_XTAL / 2);
	m_net->intr_out().set(FUNC(news_68k_base_state::irq_w<LANCE>)).invert();
	m_net->dma_in().set([this](offs_t offset) { return m_net_ram[(offset >> 1) & 0x1fff]; });
	m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[(offset >> 1) & 0x1fff]); });

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);

	/*
	 * NWS-1580:
	 * CDC WREN V HH 94221-5 (5.25" half-height SCSI-1 single-ended)
	 * 1544 cylinders, 5 heads, 52 sectors/cylinder, ~170MiB formatted
	 *
	 * Vendor   Product          Rev. Vendor-specific
	 * CDC      94221-5          5457 00018715
	 */
	NSCSI_CONNECTOR(config, "scsi:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", news_scsi_devices, nullptr);

	CENTRONICS(config, m_parallel, centronics_devices, nullptr);
	m_parallel->busy_handler().set([this](int const status) {
		bool const new_status = status;
		if (m_parallel_busy != new_status)
		{
			LOGMASKED(LOG_PARALLEL, "Parallel busy changed to %s\n", new_status ? "H" : "L");
			m_parallel_busy = new_status;
			irq_w<PRINTER>(1);
		}
	});
	m_parallel->fault_handler().set([this](const int status) {
		bool const new_status = status;
		if (m_parallel_fault != new_status)
		{
			LOGMASKED(LOG_PARALLEL, "Parallel fault changed to %s\n", new_status ? "H" : "L");
			m_parallel_fault = !new_status;
			irq_w<PRINTER>(1);
		}
	});

	OUTPUT_LATCH(config, m_parallel_data);
	m_parallel->set_output_latch(*m_parallel_data);

	NEWS_HID_HLE(config, m_hid);

	SOFTWARE_LIST(config, "software_list").set_original("sony_news").set_filter("CISC");
}

void news_68k_base_state::config_scc(machine_config &config, char const *default_device_name, int const scc_clock)
{
	SCC85C30(config, m_scc, scc_clock);
	m_scc->out_int_callback().set(
		[this](int state)
		{
			m_scc_irq_state = bool(state);
			m_irq5->in_w<2>(state);
		});

	// scc channel A
	RS232_PORT(config, m_serial[0], default_rs232_devices, default_device_name);
	m_serial[0]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));
	m_serial[0]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	m_serial[0]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	m_scc->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_scc->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));

	// scc channel B
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[1]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));
	m_serial[1]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	m_serial[1]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	m_scc->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
}

void news_68k_desktop_state::nws1580(machine_config &config)
{
	M68030(config, m_cpu, 50_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &news_68k_desktop_state::desktop_cpu_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &news_68k_desktop_state::cpu_autovector_map);

	// 16 SIMM slots for RAM arranged as two groups of 8 slots, with each bank
	// corresponding to a pair of slots in each group; first bank soldered in
	// NWS-15xx series comes with 4MB standard and uses NWA-028 4MB (no parity) RAM expansion kits
	RAM(config, m_ram);
	m_ram->set_default_size("4M");
	m_ram->set_extra_options("8M,12M,16M");
	m_ram->set_default_value(0);

	common(config);
	config_scc(config, "terminal", 3'993'600);

	INPUT_MERGER_ANY_HIGH(config, m_irq7);
	m_irq7->output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ7);

	UPD72067(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(FUNC(news_68k_desktop_state::irq_w<FDC>));
	m_fdc->drq_wr_callback().set(m_irq7, FUNC(input_merger_device::in_w<0>));
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);

	// scsi host adapter
	CXD1180(config, m_scsi, 20_MHz_XTAL / 2);
	m_scsibus->set_external_device(7, m_scsi);
	m_scsi->irq_handler().set(DEVICE_SELF, FUNC(news_68k_desktop_state::irq_w<SCSI>));
	m_scsi->irq_handler().append(m_dma, FUNC(dmac_0266_device::eop_w));
	m_scsi->drq_handler().set(m_dma, FUNC(dmac_0266_device::req_w));
	m_dma->dma_r_cb().set(m_scsi, FUNC(cxd1180_device::dma_r));
	m_dma->dma_w_cb().set(m_scsi, FUNC(cxd1180_device::dma_w));

#if DESKTOP_GRAPHICS
	m_hid->irq_out<news_hid_hle_device::KEYBOARD>().set(m_irq5, FUNC(input_merger_device::in_w<0>));
	m_hid->irq_out<news_hid_hle_device::MOUSE>().set(m_irq5, FUNC(input_merger_device::in_w<1>));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(64.0_MHz_XTAL, 1024, 0, 1024, 768, 0, 768);
	m_screen->set_screen_update(FUNC(news_68k_desktop_state::screen_update));

	// AM81C458-80JC
	BT458(config, m_ramdac, 64.0_MHz_XTAL);

	// 32 x MB81461-12 (256Kbit ZIP VRAM)
	RAM(config, m_vram);
	m_vram->set_default_size("1MiB");
	m_vram->set_default_value(0);
#endif
}

void news_68k_laptop_state::nws1250(machine_config &config)
{
	M68030(config, m_cpu, 50_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &news_68k_laptop_state::laptop_cpu_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &news_68k_laptop_state::cpu_autovector_map);

	RAM(config, m_ram);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("4M,12M"); // NWS-1230 came with 4MB standard, 1250 with 8MB
	m_ram->set_default_value(0);

	common(config);
	config_scc(config, nullptr, 4'915'200);
	m_hid->irq_out<news_hid_hle_device::KEYBOARD>().set(m_irq5, FUNC(input_merger_device::in_w<0>));
	m_hid->irq_out<news_hid_hle_device::MOUSE>().set(m_irq5, FUNC(input_merger_device::in_w<1>));

	INPUT_MERGER_ANY_HIGH(config, m_irq7);
	m_irq7->output_handler().set_inputline(m_cpu, INPUT_LINE_IRQ7);

	N82077AA(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(FUNC(news_68k_laptop_state::irq_w<FDC>));
	m_fdc->drq_wr_callback().set(m_irq7, FUNC(input_merger_device::in_w<0>));
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);

	// scsi host adapter
	CXD1185(config, m_scsi, 20_MHz_XTAL / 2); // TODO: clock rate needs confirmation on hw
	m_scsibus->set_external_device(7, m_scsi);
	m_scsi->irq_out_cb().set(DEVICE_SELF, FUNC(news_68k_laptop_state::irq_w<SCSI>));
	m_scsi->irq_out_cb().append(m_dma, FUNC(dmac_0266_device::eop_w));
	m_scsi->drq_out_cb().set(m_dma, FUNC(dmac_0266_device::req_w));
	m_scsi->port_out_cb().set(m_lcd, FUNC(news_lcd_device::lcd_dim_w)); // PRT0 is used to control the LCD dimmer
	m_dma->dma_r_cb().set(m_scsi, FUNC(cxd1185_device::dma_r));
	m_dma->dma_w_cb().set(m_scsi, FUNC(cxd1185_device::dma_w));

	// Integrated LCD panel
	NEWS_LCD(config, m_lcd);
	m_lcd->set_vram(m_vram);
}

static INPUT_PORTS_START(nws12x0)
	PORT_START("SW1")
	PORT_DIPNAME(0x07, 0x05, "Display") PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(0x07, "Console")
	PORT_DIPSETTING(0x05, "LCD")
	PORT_DIPSETTING(0x00, "Autoselect")

	PORT_DIPNAME(0x08, 0x08, "Boot Device") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x08, "SCSI")
	PORT_DIPSETTING(0x00, "Floppy")

	PORT_DIPNAME(0x10, 0x10, "Automatic Boot") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_DIPNAME(0x20, 0x20, "Diagnostic Mode") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_DIPNAME(0x40, 0x40, "No Memory Mode") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_DIPUNUSED_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END

static INPUT_PORTS_START(nws15x0)
	PORT_START("SW1")
	PORT_DIPNAME(0x07, 0x07, "Display") PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(0x07, "Console")
	PORT_DIPSETTING(0x06, "NWB-512")
	PORT_DIPSETTING(0x03, "NWB-225A")
	PORT_DIPSETTING(0x00, "Autoselect")

	PORT_DIPNAME(0x08, 0x08, "Boot Device") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x08, "SCSI")
	PORT_DIPSETTING(0x00, "Floppy")

	PORT_DIPNAME(0x10, 0x10, "Automatic Boot") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_DIPNAME(0x20, 0x20, "Diagnostic Mode") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_DIPUNUSED_DIPLOC(0xc0, 0xc0, "SW1:7,8")
INPUT_PORTS_END

ROM_START(nws1250)
	ROM_REGION32_BE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "nws1250-20", "SONY NET WORK STATION MC68030 Monitor Release 2.0") // default to match with paired graphics ROM
	ROMX_LOAD("nws1200_9006_am27c1024.bin", 0x00000, 0x20000, CRC(0b836746) SHA1(0dd7ed246c203646747eb99a72e3b91bb702796c), ROM_BIOS(0) | ROM_GROUPWORD | ROM_REVERSE)
	ROM_SYSTEM_BIOS(1, "nws1250-20a", "SONY NET WORK STATION MC68030 Monitor Release 2.0A")
	ROMX_LOAD("nws-1200_ver_2.0a_9010.ic2", 0x00000, 0x20000, CRC(87eca9d2) SHA1(235585a55bc2b3206cfec532852526a638eccad2), ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE)

	// AM27S21PC PROM
	// IDROM has system-specific data, so there is no "golden" dump.
	ROM_REGION32_BE(0x100, "idrom", 0)
	ROM_LOAD("n1250_50292_am27s21pc.ic36", 0x000, 0x100, CRC(8cf47e35) SHA1(3eef8168ffb8f7879bcbac9e8fee2115a191ae83) BAD_DUMP)

	// 2 x MB834200B (mask ROM, from system with monitor 2.0)
	// There may be a different revision of these on MB834200A ROMs
	ROM_REGION32_BE(0x100000, "krom", ROMREGION_ERASEFF)
	ROM_LOAD32_DWORD("mb834200b_u44.bin", 0x00000, 0x80000, CRC(6a50162a) SHA1(92383c3ad7aaa7b2f9c8cf781c6dcddffe7b9af8))
	ROM_LOAD32_DWORD("mb834200b_u45.bin", 0x80000, 0x80000, CRC(f2886c9b) SHA1(76363bb7ef884bcf51c50ac56963d513fe776c2e))
ROM_END

ROM_START(nws1580)
	ROM_REGION32_BE(0x10000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "nws1580", "SONY NET WORK STATION MC68030 Monitor Release 1.3")
	ROMX_LOAD("pws-1500__ver_1.3__8906.bin", 0x00000, 0x10000, CRC(76395ad9) SHA1(c2ae00218c23cef6519a4d7c74ac2c552790dfd4), ROM_BIOS(0))

	// MB7114 256x4 TTL PROM
	ROM_REGION32_BE(0x100, "idrom", 0)
	ROM_LOAD("n1580_50093.ic63", 0x000, 0x100, CRC(a7f293d6) SHA1(21deffed69e07af515ffc5511bdbf73a2a4c14fb))

	// 2 x HN62321BP (128K x 8-bit mask ROM)
	ROM_REGION32_BE(0x100000, "krom", ROMREGION_ERASEFF)
	ROM_LOAD64_BYTE("aa1.ic14", 0x00000, 0x20000, CRC(db274954) SHA1(4bc9b8a862ce9bdbf43c70f84921253876e21e58))
	ROM_LOAD64_BYTE("aa2.ic15", 0x00001, 0x20000, CRC(0d7686c7) SHA1(b0be18166b4690518e6a11ea194cc1c7a1ea6347))
ROM_END

} // anonymous namespace


//   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS                   INIT         COMPANY  FULLNAME    FLAGS
COMP(1988, nws1580, 0,      0,      nws1580, nws15x0, news_68k_desktop_state, init_common, "Sony",  "NWS-1580", MACHINE_NOT_WORKING)
COMP(1990, nws1250, 0,      0,      nws1250, nws12x0, news_68k_laptop_state,  init_common, "Sony",  "NWS-1250", MACHINE_NO_SOUND)
