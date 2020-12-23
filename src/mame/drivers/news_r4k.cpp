// license:BSD-3-Clause
// copyright-holders:Brice Onken, based on Patrick Mackinlay's NEWS 68k and r3k emulators
// thanks-to:Patrick Mackinlay

/*
 * Sony NEWS R4000/4400-based workstations.
 *
 * Sources/More Information:
 *   - https://github.com/robohack/ucb-csrg-bsd/blob/master/sys/news3400/
 *   - http://ozuma.o.oo7.jp/nws5000x.htm
 *   - https://katsu.watanabe.name/doc/sonynews/
 *   - https://web.archive.org/web/20170202100940/www3.videa.or.jp/NEWS/
 *   - https://github.com/NetBSD/src/tree/trunk/sys/arch/newsmips
 *   - https://github.com/briceonk/news-os
 *
 *  Command used to build: make ARCHOPTS=-U_FORTIFY_SOURCE TOOLS=1 -j 13 SOURCES=src/mame/drivers/news_r4k.cpp REGENIE=1
 */

#include "emu.h"

// Devices
#include "cpu/mips/mips3.h"
#include "machine/ram.h"
#include "machine/timekpr.h"
#include "machine/z80scc.h"
#include "machine/am79c90.h"
#include "machine/upd765.h"
#include "machine/dmac_0448.h"
#include "machine/news_hid.h"
#include "machine/cxd1185.h"
#include "screen.h"
#include "sound/spkrdev.h"
#include "speaker.h"
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"
#include "formats/pc_dsk.h"

// MAME infra imports
#include "debugger.h"

#define VERBOSE 1
#include "logmacro.h"

class news_r4k_state : public driver_device
{
public:
	news_r4k_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag),
		  m_cpu(*this, "cpu"),
		  m_ram(*this, "ram"),
		  m_rtc(*this, "rtc"),
		  m_escc(*this, "escc"),
		  m_serial(*this, "serial%u", 0U) { }

public:
	// NWS-5000X
	void nws5000x(machine_config &config);
	void init_nws5000x();

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	void cpu_map(address_map &map);

	// machine config
	void machine_common(machine_config &config);

	// machine init
	void init_common();

	// Bitmap update (not implemented yet)
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	// Interrupts (not implemented yet)
	void inten_w(offs_t offset, u16 data, u16 mem_mask);
	u16 inten_r() { return m_inten; }
	u16 intst_r() { return m_intst; }
	void intclr_w(offs_t offset, u16 data, u16 mem_mask);

	// See news5000 section of https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/include/adrsmap.h
	enum irq_number : unsigned
	{
		EXT3 = 0,
		EXT1 = 1,
		SLOT3 = 2,
		SLOT1 = 3,
		DMA = 4,
		LANCE = 5,
		SCC = 6,
		BEEP = 7,
		CBSY = 8,
		CFLT = 9,
		MOUSE = 10,
		KBD = 11,
		TIMER = 12,
		BERR = 13,
		ABORT = 14,
		PERR = 15,
	}; // TODO: Needs to be scrubbed
	template <irq_number Number>
	void irq_w(int state);
	void int_check();

	// Platform hardware emulation methods
	u32 bus_error();
	void itimer_w(u8 data);
	void itimer(void *ptr, s32 param);
	u8 debug_r() { return m_debug; }
	void debug_w(u8 data);

	// DECLARE_FLOPPY_FORMATS(floppy_formats);

	// Devices
	// MIPS R4400 CPU
	required_device<r4400be_device> m_cpu;

	// Main memory
	required_device<ram_device> m_ram;

	// ST Micro M48T02 Timekeeper NVRAM + RTC
	required_device<m48t02_device> m_rtc;

	// Zilog ESCC serial controller
	required_device<z80scc_device> m_escc;
	required_device_array<rs232_port_device, 2> m_serial;

	// 5000X has a DMAC3 DMA controller
	// required_device<dmac_0448_device> m_dma;

	// 5000X has a SONIC ethernet controller
	// required_device<am7990_device> m_net;
	// std::unique_ptr<u16[]> m_net_ram;

	// TBD: Floppy controller type
	// required_device<upd72067_device> m_fdc;

	// NEWS keyboard and mouse
	// required_device<news_hid_hle_device> m_hid;

	// 5000X has an HP SPIFI3 SCSI controller
	// required_device<cxd1185_device> m_scsi;
	// required_device<nscsi_bus_device> m_scsibus;

	// 5000X has a DSC-39 "xb" video card
	// required_shared_ptr<u32> m_vram;

	// TBD: LEDs
	// output_finder<4> m_led;

	// Hardware timers
	// emu_timer *m_itimer;
	emu_timer *m_freerun_timer;
	u32 freerun_timer_val;
	TIMER_CALLBACK_MEMBER(freerun_clock);

	// Interrupts and other platform state
	u16 m_inten;
	u16 m_intst;
	u8 m_debug;

	// LED control
	uint8_t led_state_r(offs_t offset);
	void led_state_w(offs_t offset, uint8_t data);
	bool m_int_state[4];

	// APBus control (will be split into a device eventually)
	uint8_t apbus_cmd_r(offs_t offset);
	void apbus_cmd_w(offs_t offset, uint8_t data);

	// Temporary workarounds
	uint8_t hack1(offs_t offset);
	uint8_t hack2(offs_t offset);
	uint8_t hack3(offs_t offset);
	uint8_t freerun_r(offs_t offset);

	// Constants
	const uint32_t XTAL_75_MHz = 75000000;
	const uint32_t ICACHE_SIZE = 16384;
	const uint32_t DCACHE_SIZE = 16384;
	const uint32_t NVRAM_SIZE = 0x7f8;
	const char* MAIN_MEMORY_DEFAULT = "64M";
};

//FLOPPY_FORMATS_MEMBER(news_r4k_state::floppy_formats)
//FLOPPY_PC_FORMAT
//FLOPPY_FORMATS_END

void news_r4k_state::machine_common(machine_config &config)
{
	// CPU setup
	// Board has a 75MHz crystal, multiplier (if any) TBD
	R4400BE(config, m_cpu, XTAL_75_MHz);
	m_cpu->set_addrmap(AS_PROGRAM, &news_r4k_state::cpu_map);
	m_cpu->set_icache_size(ICACHE_SIZE);
	m_cpu->set_dcache_size(DCACHE_SIZE);

	// Main memory
	RAM(config, m_ram);
	m_ram->set_default_size(MAIN_MEMORY_DEFAULT);

	// Timekeeper IC
	M48T02(config, m_rtc);

	// General ESCC setup
	// 9.8304MHz per NetBSD source
	SCC85230(config, m_escc, 9.8304_MHz_XTAL);
	// m_escc->out_int_callback().set(FUNC(news_r4k_state::irq_w<SCC>));

	// ESCC channel A (mapped to serial port 1)
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[1]->cts_handler().set(m_escc, FUNC(z80scc_device::ctsa_w));
	m_serial[1]->dcd_handler().set(m_escc, FUNC(z80scc_device::dcda_w));
	m_serial[1]->rxd_handler().set(m_escc, FUNC(z80scc_device::rxa_w));
	m_escc->out_rtsa_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_escc->out_txda_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_escc->out_dtra_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));

	// ESCC channel B (mapped to serial port 0)
	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	m_serial[0]->cts_handler().set(m_escc, FUNC(z80scc_device::ctsb_w));
	m_serial[0]->dcd_handler().set(m_escc, FUNC(z80scc_device::dcdb_w));
	m_serial[0]->rxd_handler().set(m_escc, FUNC(z80scc_device::rxb_w));
	m_escc->out_rtsb_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_escc->out_txdb_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_escc->out_dtrb_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
}

void news_r4k_state::nws5000x(machine_config &config)
{
	machine_common(config);
}

/*
 * cpu_map
 * 
 * Assign the address map for the CPU
 * Reference: https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/include/adrsmap.h
 */
void news_r4k_state::cpu_map(address_map &map)
{
	map.unmap_value_high();

	// NEWS firmware
	map(0x1fc00000, 0x1fc3ffff).rom().region("mrom", 0); // Monitor ROM
	map(0x1f3c0000, 0x1f3c03ff).rom().region("idrom", 0); // IDROM

	// Front panel DIP switches - TODO: mirror length
	map(0x1f3d0000, 0x1f3d0007).lr64(NAME([this](offs_t offset) {
		ioport_value dipsw = this->ioport("FRONT_PANEL")->read();
		dipsw |= 0xff00; // Matches physical platform
		return ((unsigned long) dipsw << 32) | dipsw;
	}));
	
	// Hardware timers
	// map(0x1f800000, 0x1f800000); // TIMER0
	map(0x1f840000, 0x1f840003).r(FUNC(news_r4k_state::freerun_r)); // FREERUN

	// Timekeeper NVRAM
	map(0x1f880000, 0x1f8817f7).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write)).umask32(0x000000ff);
	// Timekeeper RTC
	map(0x1f881fe0, 0x1f881fff).lrw8(
		NAME([this](offs_t offset) {
			return m_rtc->read(NVRAM_SIZE + offset);
		}),
		NAME([this](offs_t offset, uint8_t data) {
			return m_rtc->write(NVRAM_SIZE + offset, data);
		})
	).umask32(0x000000ff);

	// Interrupt clear ports
	// map(0x1f4e0000, 0x1f4e0003).ram(); // INTCLR0
	// map(0x1f4e0004, 0x1f4e0007).ram(); // INTCLR1
	// map(0x1f4e0008, 0x1f4e000b).ram(); // INTCLR2
	// map(0x1f4e000c, 0x1f4e000f).ram(); // INTCLR3
	// map(0x1f4e0010, 0x1f4e0013).ram(); // INTCLR4
	// map(0x1f4e0014, 0x1f4e0017).ram(); // INTCLR5

	// Interrupt enable ports
	// map(0x1fa00000, 0x1fa00003).ram(); // INTEN0
	// map(0x1fa00004, 0x1fa00007).ram(); // INTEN1
	// map(0x1fa00008, 0x1fa0000b).ram(); // INTEN2
	// map(0x1fa0000c, 0x1fa0000f).ram(); // INTEN3
	// map(0x1fa00010, 0x1fa00013).ram(); // INTEN4
	// map(0x1fa00014, 0x1fa00017).ram(); // INTEN5

	// Interrupt status ports
	// map(0x1fa00020, 0x1fa00023).ram(); // INTST0
	// map(0x1fa00024, 0x1fa00027).ram(); // INTST1
	// map(0x1fa00028, 0x1fa0002b).ram(); // INTST2
	// map(0x1fa0002c, 0x1fa0002f).ram(); // INTST3
	// map(0x1fa00030, 0x1fa00033).ram(); // INTST4
	// map(0x1fa00034, 0x1fa00037).ram(); // INTST5

	// LEDs
	map(0x1f3f0000, 0x1f3f0014).rw(FUNC(news_r4k_state::led_state_r), FUNC(news_r4k_state::led_state_w));

	// APBus region
	// map(0x1f520004, 0x1f520007); // WBFLUSH
	map(0x1f520000, 0x1f520013).rw(FUNC(news_r4k_state::apbus_cmd_r), FUNC(news_r4k_state::apbus_cmd_w));
	//map(0x14c00004, 0x14c00007).ram(); // some kind of AP-bus register? Fully booted 5000X yields: 14c00004: 00007316
	// map(0x14c0000c, 0x14c0000c); // APBUS_INTMSK /* interrupt mask */
	// map(0x14c00014, 0x14c00014); // APBUS_INTST /* interrupt status */
	// map(0x14c0001c, 0x14c0001c); // APBUS_BER_A /* Bus error address */
	// map(0x14c00034, 0x14c00034); // APBUS_CTRL /* configuration control */
	// map(0x1400005c, 0x1400005c); // APBUS_DER_A /* DMA error address */
	// map(0x14c0006c, 0x14c0006c); // APBUS_DER_S /* DMA error slot */
	// map(0x14c00084, 0x14c00084); // APBUS_DMA /* unmapped DMA coherency */
	// map(0x14c20000, 0x14c40000); // APBUS_DMAMAP /* DMA mapping RAM */

	// Serial port
	map(0x1e950000, 0x1e95000f).rw(m_escc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask64(0x000000ff000000ff); // SCCPORT0A

	// TODO: ESCCF? ESCC FIFO?
	// TODO: map(0x1e900000, 0x1e900000);

	// Sonic network controller (https://git.qemu.org/?p=qemu.git;a=blob;f=hw/net/dp8393x.c;h=674b04b3547cdf312620a13c2f183e0ecfab24fb;hb=HEAD)
	// map(0x1e600000, 0x1e600000); // TODO: this (see https://github.com/NetBSD/src/blob/fc1bde7fb56cf2ceb6c98f29a7547fbd92d9ca25/sys/arch/newsmips/apbus/if_sn_ap.c, https://github.com/NetBSD/src/blob/64b8a48e1288eb3902ed73113d157af50b2ec596/sys/arch/newsmips/apbus/if_snreg.h)

	// DMAC3 DMA Controller 0
	// map(0x1e200000, 0x1e20000f); // End addr meeds confirmation

	// DMAC3 DMA Controller 1
	// map(0x1e300000, 0x1e30000f); // End addr meeds confirmation

	// xb (Sony DSC-39 video card)
	// map(0x14900000, 0x14900000);

	// sb (Likely Fujitsu MB86431 audio chip)
	// map(0x1ed00000, 0x1ed00000);

	// spifi controller 1 (scsi bus 0)
	// map(0x1e280000, 0x1e280000);

	// spifi controller 2 (scsi bus 1)
	// map(0x1e380000, 0x1e380000);

	// ms (mouse)
	// map(0x1f900014, 0x1f900014);

	// lp (printer port??)
	// map(0x1ed30000, 0x1ed30000);

	// kb (keyboard)
	// map(0x1f900000, 0x1f900000);

	// fd (floppy disk?)
	// map(0x1ed20000, 0x1ed20000);


	// Everything below this comment is just for debug
	map(0x1e980000, 0x1e9fffff).ram(); // is this mirrored?
	//map(0x1f3f0000, 0x1f3f0017);
	map(0x1fe00000, 0x1fffffff).ram(); // determine mirror of this RAM - it is smaller than this size
	//map(0x1f840000, 0x1f84ffff).ram(); // what is this
	map(0x1f3e0000, 0x1f3efff0).r(FUNC(news_r4k_state::hack1)); // ditto ;__;
	map(0x1f4c0000, 0x1f4c0007).ram();							// Register for something that is accessed very early in mrom flow (0xbfc0040C)

	map(0x14400004, 0x14400007).r(FUNC(news_r4k_state::hack2));
	map(0x14900004, 0x14900007).r(FUNC(news_r4k_state::hack3));
	//map(0x14400008, 0x1440004f).ram(); // not sure what this is, register?
	//map(0x1e280074, 0x1e280077); // Fully booted: 1e280074: 00000001
	//map(0x1e380074, 0x1e380077); // Fully booted: 1e380074: 00000001
	// map(0x1ed6020c, 0x1ed6020f).lr8(NAME([this](offs_t o) {
	// 	return 0x80;
	// }));

	// Unknown regions that mrom accesses
	//map(0x1f4c0000, 0x1f4c0003).ram();
	//map(0x1f4c0004, 0x1f4c0007).ram(); // 1F4C0000, 1F4C0004 - writes, APBus init?
	//map(0x1f520008, 0x1f52000B).ram();  // waiting for AP-Bus to come up?
	//map(0x1f52000C, 0x1f52000F).ram(); // 1F520008, 1F52000C - reads, APBus region??? Physically close to APBus WBFlush instruction
}

void news_r4k_state::machine_start()
{
	/*
	m_led.resolve();
	*/

	// m_net_ram = std::make_unique<u16[]>(65536);
	// save_pointer(NAME(m_net_ram), 65536);

	/*

	save_item(NAME(m_inten));
	save_item(NAME(m_intst));
	save_item(NAME(m_debug));
	save_item(NAME(m_int_state));
	save_item(NAME(m_lcd_enable));
	save_item(NAME(m_lcd_dim));

	m_itimer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(news_r4k_state::itimer), this));
data
	for (bool &int_state : m_int_state)
		int_state = false;
	m_lcd_enable = false;
	m_lcd_dim = false;
    */

	// Allocate freerunning clock
	m_freerun_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(news_r4k_state::freerun_clock), this));
}

TIMER_CALLBACK_MEMBER(news_r4k_state::freerun_clock) { freerun_timer_val++; }

void news_r4k_state::machine_reset()
{
	// TODO: what is the actual frequency of the freerunning clock?
	// Too fast causes the mrom to overwhelm the ESCC and gets off into the weeds
	freerun_timer_val = 0;
	m_freerun_timer->adjust(attotime::zero, 0, attotime::from_usec(1000));
}

void news_r4k_state::init_common()
{

	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());

	/*
	// HACK: hardwire the rate until fdc is better understood
	m_fdc->set_rate(500000);

	// HACK: signal floppy density?
	m_scsi->port_w(0x02);
    */
}

void news_r4k_state::init_nws5000x()
{
	init_common();
}

uint8_t news_r4k_state::led_state_r(offs_t offset) { return 0; }
void news_r4k_state::led_state_w(offs_t offset, uint8_t data)
{
	// map(0x1f3f0000, 0x1f3f0000); // LED_POWER
	// map(0x1f3f0004, 0x1f3f0004); // LED_DISK
	// map(0x1f3f0008, 0x1f3f0008); // LED_FLOPPY
	// map(0x1f3f000c, 0x1f3f000c); // LED_SEC
	// map(0x1f3f0010, 0x1f3f0010); // LED_NET
	// map(0x1f3f0014, 0x1f3f0014); // LED_CD
	// LOG("Setting LED at offset 0x%x to 0x%x\n", offset, data);
}

uint8_t news_r4k_state::apbus_cmd_r(offs_t offset)
{
	// ugly hack, like everything else about this driver right now
	// these values came from my NWS-5000X after it booted to the monitor
	// so this is pretending to be fully initialized. Needless to say, that
	// *might* cause problems with the monitor
	uint8_t value = 0x0;
	if (offset == 7)
	{
		value = 0x1;
	}
	else if (offset == 11)
	{
		value = 0x1;
	}
	else if (offset == 15)
	{
		value = 0xc8;
	}
	else if (offset == 19)
	{
		value = 0x32;
	}
	LOG("APBus read triggered at offset 0x%x, returing 0x%x\n", offset, value);
	return value;
}

void news_r4k_state::apbus_cmd_w(offs_t offset, uint8_t data)
{
	LOG("AP-Bus command called, offset 0x%x, set to 0x%x\n", offset, data);
}

uint8_t news_r4k_state::hack1(offs_t offset)
{
	if (offset % 4 == 0 || offset % 4 == 1)
	{
		return 0x0;
	}
	else if (offset % 4 == 2)
	{
		return 0x6f;
	}
	else if (offset % 4 == 3)
	{
		return 0xe0;
	}
	else
	{
		LOG("uh oh!\n");
		return 0x0;
	}
}

uint8_t news_r4k_state::hack2(offs_t offset)
{
	if (offset < 1)
	{
		return 0x0;
	}
	else if (offset == 1)
	{
		return 0x3;
	}
	else if (offset == 2)
	{
		return 0xff;
	}
	else if (offset == 3)
	{
		return 0x17;
	}
	else
	{
		return 0x0;
	}
}

uint8_t news_r4k_state::hack3(offs_t offset)
{
	if (offset < 1)
	{
		return 0x0;
	}
	else if (offset == 1)
	{
		return 0x0;
	}
	else if (offset == 2)
	{
		return 0x03;
	}
	else if (offset == 3)
	{
		return 0x28;
	}
	else
	{
		return 0x0;
	}
}

uint8_t news_r4k_state::freerun_r(offs_t offset)
{
	return freerun_timer_val & 0x000F << (4 * offset);
}

u32 news_r4k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	/*
	if (!m_lcd_enable)
		return 0;

	rgb_t const black = rgb_t::black();
	rgb_t const white = m_lcd_dim ? rgb_t(191, 191, 191) : rgb_t::white();

	u32 const *pixel_pointer = m_vram;

	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
	{
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 32)
		{
			u32 const pixel_data = *pixel_pointer++;

			for (unsigned i = 0; i < 32; i++)
				bitmap.pix(y, x + i) = BIT(pixel_data, 31 - i) ? black : white;
		}
	}
*/
	return 0;
}

void news_r4k_state::inten_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*
	COMBINE_DATA(&m_inten);

	int_check();
    */
}

template <news_r4k_state::irq_number Number>
void news_r4k_state::irq_w(int state)
{
	/*
	LOG("irq number %d state %d\n",  Number, state);

	if (state)
		m_intst |= 1U << Number;
	else
		m_intst &= ~(1U << Number);

	int_check();
    */
}

void news_r4k_state::intclr_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*
	m_intst &= ~(data & mem_mask);

	int_check();
    */
}

void news_r4k_state::int_check()
{
	/*
	// TODO: assume 44422222 11100000
	static int const int_line[] = { INPUT_LINE_IRQ0, INPUT_LINE_IRQ1, INPUT_LINE_IRQ2, INPUT_LINE_IRQ4 };
	static u16 const int_mask[] = { 0x001f, 0x00e0, 0x1f00, 0xe000 };

	for (unsigned i = 0; i < ARRAY_LENGTH(m_int_state); i++)
	{
		bool const int_state = m_intst & m_inten & int_mask[i];

		if (m_int_state[i] != int_state)
		{
			m_int_state[i] = int_state;
			m_cpu->set_input_line(int_line[i], int_state);
		}
	}
    */
}

u32 news_r4k_state::bus_error()
{
	/*
	if (!machine().side_effects_disabled())
		irq_w<BERR>(ASSERT_LINE);
	*/
	return 0;
}

void news_r4k_state::itimer_w(u8 data)
{
	/*
	LOG("itimer_w 0x%02x (%s)\n", data, machine().describe_context());

	// TODO: assume 0xff stops the timer
	u8 const ticks = data + 1;

	m_itimer->adjust(attotime::from_ticks(ticks, 800), 0, attotime::from_ticks(ticks, 800));
    */
}

void news_r4k_state::itimer(void *ptr, s32 param)
{
	/*
	irq_w<TIMER>(ASSERT_LINE);
    */
}

void news_r4k_state::debug_w(u8 data)
{
	/*
	 * The low four bits of this register control the diagnostic LEDs labelled 1-4
	 * with bit 0 correspondig to LED #1, and a 0 value enabling the LED. A non-
	 * exhaustive list of diagnostic codes produced by the PROM follows:
	 *
	 *  4321  Stage
	 *  ...x  EPROM checksum
	 *  ..x.  NVRAM test (byte)
	 *  ..xx  NVRAM test (word)
	 *  .x..  NVRAM test (dword)
	 *  .x.x  read dip-switch SW2
	 *  .xx.  write test 0x1fe70000-1fe9ffff?
	 *  .xxx  address decode
	 *  x...  NVRAM test (dword)
	 *  x..x  RAM sizing
	 *  x.x.  inventory/boot
	 *
	 */
	/*
	LOG("debug_w 0x%02x (%s)\n", data, machine().describe_context());

	for (unsigned i = 0; i < 4; i++)
		if (BIT(data, i + 4))
			m_led[i] = BIT(data, i);

	m_debug = data;
    */
}

/*static void news_scsi_devices(device_slot_interface &device)
{
    //device.option_add("harddisk", NSCSI_HARDDISK);
    //device.option_add("cdrom", NSCSI_CDROM);
}*/

/*
 * NWS-5000X DIP switches
 *
 * 1. Console - switch between serial and bitmap console. Bitmap is not implemented yet.
 * 2. Bitmap Disable - Enable or disable the internal video card
 * 3. Abort/Resume Enable - Unknown
 * 4. Clear NVRAM - Upon boot, clear NVRAM contents and restore default values if set
 * 5. Auto Boot - Upon boot, automatically attempt to boot from the disk specified by the bootdev NVRAM variable
 * 6. Run Diagnostic Test - Attempt to run diagnostic test after ROM monitor has booted
 * 7. External APSlot Probe Disable - If set, do not attempt to probe the expansion APBus slots
 * 8. No Memory Mode - If set, do not use the main memory (limits system to 128KiB)
 */
static INPUT_PORTS_START(nws5000)
	PORT_START("FRONT_PANEL")
	PORT_DIPNAME(0x01, 0x00, "Console") PORT_DIPLOCATION("FRONT_PANEL:1")
	PORT_DIPSETTING(0x00, "Serial Terminal")
	PORT_DIPSETTING(0x01, "Bitmap")
	PORT_DIPNAME(0x02, 0x00, "Bitmap Disable") PORT_DIPLOCATION("FRONT_PANEL:2")
	PORT_DIPSETTING(0x00, "Enable built-in bitmap")
	PORT_DIPSETTING(0x02, "Disable inner bitmap")
	PORT_DIPNAME(0x04, 0x00, "Abort/Resume Enable") PORT_DIPLOCATION("FRONT_PANEL:3")
	PORT_DIPSETTING(0x00, "Disable Abort/Resume")
	PORT_DIPSETTING(0x04, "Enable Abort/Resume")
	PORT_DIPNAME(0x08, 0x00, "Clear NVRAM") PORT_DIPLOCATION("FRONT_PANEL:4")
	PORT_DIPSETTING(0x00, "Do not clear")
	PORT_DIPSETTING(0x08, "Clear NVRAM")
	PORT_DIPNAME(0x10, 0x00, "Auto Boot") PORT_DIPLOCATION("FRONT_PANEL:5")
	PORT_DIPSETTING(0x00, "Auto Boot Disable")
	PORT_DIPSETTING(0x10, "Auto Boot Enable")
	PORT_DIPNAME(0x20, 0x00, "Run Diagnostic Test") PORT_DIPLOCATION("FRONT_PANEL:6")
	PORT_DIPSETTING(0x00, "No Diagnostic Test")
	PORT_DIPSETTING(0x20, "Run Diagnostic Test")
	PORT_DIPNAME(0x40, 0x00, "External APSlot Probe Disable") PORT_DIPLOCATION("FRONT_PANEL:7")
	PORT_DIPSETTING(0x00, "Enable External APSlot Probe")
	PORT_DIPSETTING(0x40, "Disable External APSlot Probe")
	PORT_DIPNAME(0x80, 0x00, "No Memory Mode") PORT_DIPLOCATION("FRONT_PANEL:8")
	PORT_DIPSETTING(0x00, "Main Memory Enabled")
	PORT_DIPSETTING(0x80, "Main Memory Disabled");
INPUT_PORTS_END

ROM_START(nws5000x)
ROM_REGION64_BE(0x40000, "mrom", 0)
ROM_SYSTEM_BIOS(0, "nws5000x", "APbus System Monitor Release 3.201")
ROMX_LOAD("mpu-33__ver3.201__1994_sony.rom", 0x00000, 0x40000, CRC(8a6ca2b7) SHA1(72d52e24a554c56938d69f7d279b2e65e284fd59), ROM_BIOS(0))

ROM_REGION64_BE(0x400, "idrom", 0)
ROM_LOAD("idrom.rom", 0x000, 0x400, CRC(89edfebe) SHA1(3f69ebfaf35610570693edf76aa94c10b30de627) BAD_DUMP)
ROM_END

//   YEAR  NAME      PARENT COMPAT MACHINE   INPUT    CLASS           INIT           COMPANY FULLNAME                      FLAGS
COMP(1994, nws5000x, 0,     0,     nws5000x, nws5000, news_r4k_state, init_nws5000x, "Sony", "NET WORK STATION NWS-5000X", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
