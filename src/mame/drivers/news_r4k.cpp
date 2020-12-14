// license:BSD-3-Clause
// copyright-holders:Brice Onken, based on Patrick Mackinlay's NEWS 68k and r3k emulators
// thanks-to:Patrick Mackinlay

/*
 * Sony NEWS R4000/4400-based workstations.
 *
 * Sources:
 *   - https://github.com/robohack/ucb-csrg-bsd/blob/master/sys/news3400/
 *   - http://ozuma.o.oo7.jp/nws5000x.htm
 *   - https://katsu.watanabe.name/doc/sonynews/
 *   - https://web.archive.org/web/20170202100940/www3.videa.or.jp/NEWS/
 *   - https://github.com/briceonk/news-os
 *
 *  Command used to build: make ARCHOPTS=-U_FORTIFY_SOURCE TOOLS=1 -j 13 SOURCES=src/mame/drivers/news_r4k.cpp REGENIE=1
 */

#include "emu.h"

// Devices
#include "cpu/mips/mips3.h" // TODO: change to mips3.h
#include "machine/ram.h"
#include "machine/timekpr.h" // 5000X has an M48T02-150PC1
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
		  //m_dma(*this, "dma"),
		  //m_rtc(*this, "rtc"),
		  m_scc(*this, "scc"),
		  //m_net(*this, "net"),
		  //m_fdc(*this, "fdc"),
		  //m_lcd(*this, "lcd"),
		  //m_hid(*this, "hid"),
		  //m_scsi(*this, "scsi:7:cxd1185"),
		  m_serial(*this, "serial%u", 0U)
	//m_scsibus(*this, "scsi"),
	//m_vram(*this, "vram"),
	//m_led(*this, "led%u", 0U)
	{
	}

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	void cpu_map(address_map &map);

	// machine config
	void common(machine_config &config);

public:
	// NWS-5000X
	void nws5000x(machine_config &config);
	void init_nws5000x();

protected:
	void init_common();
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

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

	u32 bus_error();
	void itimer_w(u8 data);
	void itimer(void *ptr, s32 param);
	u8 debug_r() { return m_debug; }
	void debug_w(u8 data);

	// DECLARE_FLOPPY_FORMATS(floppy_formats);

	// devices
	// TODO: Scrub these - NWS-5000X has some differences to the NWS-3xxx series beyond just the AP-Bus
	required_device<r4400be_32_device> m_cpu;
	required_device<ram_device> m_ram;
	//required_device<dmac_0448_device> m_dma;
	//required_device<m48t02_device> m_rtc;
	required_device<z80scc_device> m_scc;
	//required_device<am7990_device> m_net;
	// required_device<upd72067_device> m_fdc;

	//required_device<screen_device> m_lcd;
	//required_device<news_hid_hle_device> m_hid;
	//required_device<cxd1185_device> m_scsi;

	required_device_array<rs232_port_device, 2> m_serial;
	//required_device<nscsi_bus_device> m_scsibus;

	//required_shared_ptr<u32> m_vram;
	//output_finder<4> m_led;

	std::unique_ptr<u16[]> m_net_ram;

	// emu_timer *m_itimer;
	emu_timer *m_freerun_timer;
	u32 freerun_timer_val;

	u16 m_inten;
	u16 m_intst;
	u8 m_debug;

	uint8_t log_mem_access_r(offs_t offset);
	void log_mem_access_w(offs_t offset, uint8_t data);

	uint8_t debug_mem_r(offs_t offset);
	void debug_mem_w(offs_t offset, uint8_t data);

	uint8_t led_state_r(offs_t offset);
	void led_state_w(offs_t offset, uint8_t data);
	bool m_int_state[4];

	uint8_t apbus_cmd_r(offs_t offset);
	void apbus_cmd_w(offs_t offset, uint8_t data);

	//void freerun_clock(int context);

	TIMER_CALLBACK_MEMBER(freerun_clock);

	uint8_t hack1(offs_t offset);
	uint8_t hack2(offs_t offset);
	uint8_t hack3(offs_t offset);
	uint8_t freerun_r(offs_t offset);
	//uint8_t hack2_apbus_read(offs_t offset);
};

//FLOPPY_FORMATS_MEMBER(news_r4k_state::floppy_formats)
//FLOPPY_PC_FORMAT
//FLOPPY_FORMATS_END

void news_r4k_state::machine_start()
{
	/*
	m_led.resolve();
	*/

	m_net_ram = std::make_unique<u16[]>(65536);
	save_pointer(NAME(m_net_ram), 65536);

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
	m_freerun_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(news_r4k_state::freerun_clock), this));
}

TIMER_CALLBACK_MEMBER(news_r4k_state::freerun_clock)
{
	LOG("Freerun timer tick\n");
	freerun_timer_val++;
}

void news_r4k_state::machine_reset()
{
	freerun_timer_val = 0;
	m_freerun_timer->adjust(attotime::zero, 0, attotime::from_usec(1)); // TODO: what is the actual frequency of the freerunning clock?
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

uint8_t news_r4k_state::log_mem_access_r(offs_t offset)
{
	LOG("Read from 0x%x\n", offset);
	return 0;
}

void news_r4k_state::log_mem_access_w(offs_t offset, uint8_t data)
{
	LOG("Write 0x%x to 0x%x\n", data, offset);
}

uint8_t news_r4k_state::debug_mem_r(offs_t offset)
{
	return 0;
}

void news_r4k_state::debug_mem_w(offs_t offset, uint8_t data)
{
}

uint8_t news_r4k_state::led_state_r(offs_t offset)
{
	return 0; // who cares
}
void news_r4k_state::led_state_w(offs_t offset, uint8_t data)
{
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

/*
 * cpu_map
 * 
 * Assign the address map for the CPU
 * Reference: https://github.com/NetBSD/src/blob/trunk/sys/arch/newsmips/include/adrsmap.h
 * 
 */
void news_r4k_state::cpu_map(address_map &map)
{
	map.unmap_value_high();

	// Monitor ROM (NEWS firmware)
	map(0x1fc00000, 0x1fc3ffff).rom().region("mrom", 0);

	map(0x1f3d0000, 0x1f3d0003).ram();					  // DIP_SWITCH
	map(0x1f3c0000, 0x1f3c03ff).rom().region("idrom", 0); // IDROM
	// map(0x1f800000, 0x1f800000); // TIMER0
	map(0x1f840000, 0x1f840003).r(FUNC(news_r4k_state::freerun_r)); // FREERUN

	// ST TIMEKEEPER RAM+RTC
	// 2Kb SRAM (0x800 bytes)
	// RTC ports are remapped to 1fe0
	map(0x1f880000, 0x1f8817ff).ram(); // AP-bus + monitor NVRAM, mapped in an interesting way
	map(0x1f881fe0, 0x1f881fff);	   // RTC registers (TODO)

	// // Interrupt clear ports; // INTCLR0
	// map(0x1f4e0004, 0x1f4e0004); // INTCLR1
	// map(0x1f4e0008, 0x1f4e0008); // INTCLR2
	// map(0x1f4e000c, 0x1f4e000c); // INTCLR3
	// map(0x1f4e0010, 0x1f4e0010); // INTCLR4
	// map(0x1f4e0014, 0x1f4e0014); // INTCLR5

	// // Interrupt enable ports
	// map(0x1fa00000, 0x1fa00000); // INTEN0
	// map(0x1fa00004, 0x1fa00004); // INTEN1
	// map(0x1fa00008, 0x1fa00008); // INTEN2
	// map(0x1fa0000c, 0x1fa0000c); // INTEN3
	// map(0x1fa00010, 0x1fa00010); // INTEN4
	// map(0x1fa00014, 0x1fa00014); // INTEN5

	// // Interrupt status ports
	// map(0x1fa00020, 0x1fa00020); // INTST0
	// map(0x1fa00024, 0x1fa00024); // INTST1
	// map(0x1fa00028, 0x1fa00028); // INTST2
	// map(0x1fa0002c, 0x1fa0002c); // INTST3
	// map(0x1fa00030, 0x1fa00030); // INTST4
	// map(0x1fa00034, 0x1fa00034); // INTST5

	// LEDs
	// map(0x1f3f0000, 0x1f3f0000); // LED_POWER
	// map(0x1f3f0004, 0x1f3f0004); // LED_DISK
	// map(0x1f3f0008, 0x1f3f0008); // LED_FLOPPY
	// map(0x1f3f000c, 0x1f3f000c); // LED_SEC
	// map(0x1f3f0010, 0x1f3f0010); // LED_NET
	// map(0x1f3f0014, 0x1f3f0014); // LED_CD
	map(0x1f3f0000, 0x1f3f0014).rw(FUNC(news_r4k_state::led_state_r), FUNC(news_r4k_state::led_state_w));

	// APBus region
	// map(0x1f520004, 0x1f520007); // WBFLUSH
	map(0x1f520000, 0x1f520013).rw(FUNC(news_r4k_state::apbus_cmd_r), FUNC(news_r4k_state::apbus_cmd_w));
	// map(0x14c0000c, 0x14c0000c); // APBUS_INTMSK /* interrupt mask */
	// map(0x14c00014, 0x14c00014); // APBUS_INTST /* interrupt status */
	// map(0x14c0001c, 0x14c0001c); // APBUS_BER_A /* Bus error address */
	// map(0x14c00034, 0x14c00034); // APBUS_CTRL /* configuration control */
	// map(0x1400005c, 0x1400005c); // APBUS_DER_A /* DMA error address */
	// map(0x14c0006c, 0x14c0006c); // APBUS_DER_S /* DMA error slot */
	// map(0x14c00084, 0x14c00084); // APBUS_DMA /* unmapped DMA coherency */
	// map(0x14c20000, 0x14c40000); // APBUS_DMAMAP /* DMA mapping RAM */

	// Serial port (TODO: other serial ports)
	//map(0x1e950000, 0x1e950003).rw(FUNC(news_r4k_state::log_mem_access_r), FUNC(news_r4k_state::log_mem_access_w)); // SCCPORT0A
	map(0x1e950000, 0x1e950003).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));

	// TESTING - needed for mrom to boot - work RAM? or some unknown devices??
	map(0x1e980000, 0x1e9fffff).ram(); // is this mirrored?
	//map(0x1f3f0000, 0x1f3f0017);
	map(0x1fe00000, 0x1fffffff).ram(); // determine mirror of this RAM - it is smaller than this size
	//map(0x1f840000, 0x1f84ffff).ram(); // what is this
	map(0x1f3e0000, 0x1f3efff0).r(FUNC(news_r4k_state::hack1)); // ditto ;__;
	map(0x1f4c0000, 0x1f4c0007).ram();							// Register for something that is accessed very early in mrom flow (0xbfc0040C)

	map(0x14400004, 0x14400007).r(FUNC(news_r4k_state::hack2));
	map(0x14900004, 0x14900007).r(FUNC(news_r4k_state::hack3));
	map(0x14400008, 0x1440004f).ram(); // not sure what this is, register?

	// Unknown regions that mrom accesses
	//map(0x1f4c0000, 0x1f4c0003).ram();
	//map(0x1f4c0004, 0x1f4c0007).ram(); // 1F4C0000, 1F4C0004 - writes, APBus init?
	//map(0x1f520008, 0x1f52000B).ram();  // waiting for AP-Bus to come up?
	//map(0x1f52000C, 0x1f52000F).ram(); // 1F520008, 1F52000C - reads, APBus region??? Physically close to APBus WBFlush instruction

	// TODO: ESCCF?
	// TODO: map(0x1e900000, 0x1e900000);

	// Sonic network controller (https://git.qemu.org/?p=qemu.git;a=blob;f=hw/net/dp8393x.c;h=674b04b3547cdf312620a13c2f183e0ecfab24fb;hb=HEAD)
	// map(0x1e600000, 0x1e600000); // TODO: this (see https://github.com/NetBSD/src/blob/fc1bde7fb56cf2ceb6c98f29a7547fbd92d9ca25/sys/arch/newsmips/apbus/if_sn_ap.c, https://github.com/NetBSD/src/blob/64b8a48e1288eb3902ed73113d157af50b2ec596/sys/arch/newsmips/apbus/if_snreg.h)

	// DMA Controller 0
	//map(0x1e200000, 0x1e20000f); // End addr meeds confirmation
	// DMA Controller 1
	// map(0x1e300000, 0x1e30000f); // End addr meeds confirmation

	// xb (Sony DSC-39 video card)
	// map(0x14900000, 0x14900000);

	// sb (???)
	// map(0x1ed00000, 0x1ed00000);

	// spifi controller 1 (????, related to DMAC DMA)
	// map(0x1e280000, 0x1e280000);

	// spifi controller 2 (????, related ot DMAC DMA)
	//map(0x1e380000, 0x1e380000);

	// ms (mouse)
	//map(0x1f900014, 0x1f900014);

	// lp (printer port??)
	//map(0x1ed30000, 0x1ed30000);

	// kb (keyboard)
	//map(0x1f900000, 0x1f900000);

	// fd (???)
	//map(0x1ed20000, 0x1ed20000);

	/*

	map(0x10000000, 0x101fffff).rom().region("krom", 0);
	map(0x10200000, 0x1021ffff).ram().share("vram").mirror(0xa0000000);

	map(0x18000000, 0x18ffffff).r(FUNC(news_r4k_state::bus_error));

	map(0x1fc00000, 0x1fc1ffff).rom().region("eprom", 0);
	//map(0x1fc40004, 0x1fc40007).w().umask32(0xff); ??
	// 1fc40007 // power/reboot/PARK?
	map(0x1fc80000, 0x1fc80001).rw(FUNC(news_r4k_state::inten_r), FUNC(news_r4k_state::inten_w));
	map(0x1fc80002, 0x1fc80003).r(FUNC(news_r4k_state::intst_r));
	map(0x1fc80004, 0x1fc80005).w(FUNC(news_r4k_state::intclr_w));
	map(0x1fc80006, 0x1fc80006).w(FUNC(news_r4k_state::itimer_w));
	// 1fcc0000 // cstrobe?
	// 1fcc0002 // sccstatus0?
	map(0x1fcc0003, 0x1fcc0003).rw(FUNC(news_r4k_state::debug_r), FUNC(news_r4k_state::debug_w));
	// 1fcc0007 // sccvect?

	map(0x1fd00000, 0x1fd00007).m(m_hid, FUNC(news_hid_hle_device::map));
	map(0x1fd40000, 0x1fd40003).noprw(); // FIXME: ignore buzzer for now

	map(0x1fe00100, 0x1fe0010f).m(m_scsi, FUNC(cxd1185_device::map));
	map(0x1fe00200, 0x1fe00203).m(m_fdc, FUNC(upd72067_device::map));
	map(0x1fe00300, 0x1fe00300).lr8([]() { return 0xff; }, "sound_r"); // HACK: disable sound
	//map(0x1fe00300, 0x1fe00307); // sound
	map(0x1fe40000, 0x1fe40003).portr("SW2");
	//map(0x1fe70000, 0x1fe9ffff).ram(); // ??
	map(0x1fe80000, 0x1fe800ff).rom().region("idrom", 0).mirror(0x0003ff00);
	map(0x1fec0000, 0x1fec0003).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));

	map(0x1ff40000, 0x1ff407ff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write));
	map(0x1ff60000, 0x1ff6001b).lw8([this](offs_t offset, u8 data) { LOG("crtc offset %x 0x%02x\n", offset, data); }, "lfbm_crtc_w"); // TODO: HD64646FS
	map(0x1ff80000, 0x1ff80003).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	map(0x1ffc0000, 0x1ffc3fff).lrw16(
		[this](offs_t offset) { return m_net_ram[offset]; }, "net_ram_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset]); }, "net_ram_w");
*/
}

static INPUT_PORTS_START(nws5000)
	/*
	PORT_START("SW2")
	// TODO: other combinations of switches 1-3 may be valid
	PORT_DIPNAME(0x07000000, 0x02000000, "Display") PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(0x00000000, "Console")
	PORT_DIPSETTING(0x02000000, "LCD")
	PORT_DIPNAME(0x08000000, 0x00000000, "Boot Device") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x00000000, "Disk")
	PORT_DIPSETTING(0x08000000, "Network")
	PORT_DIPNAME(0x10000000, 0x00000000, "Automatic Boot") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x00000000, DEF_STR(Off))
	PORT_DIPSETTING(0x10000000, DEF_STR(On))
	PORT_DIPNAME(0x20000000, 0x00000000, "Diagnostic Mode") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(0x00000000, DEF_STR(Off))
	PORT_DIPSETTING(0x20000000, DEF_STR(On))
	// TODO: not completely clear what this switch does
	PORT_DIPNAME(0x40000000, 0x00000000, "RAM") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(0x00000000, "Enabled")
	PORT_DIPSETTING(0x40000000, "Disabled")
	PORT_DIPNAME(0x80000000, 0x00000000, "Console Baud") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x00000000, "9600")
	PORT_DIPSETTING(0x80000000, "1200")

	PORT_START("SW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
    */
	INPUT_PORTS_END

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

void news_r4k_state::common(machine_config &config)
{
	R4400BE32(config, m_cpu, 25_MHz_XTAL);					  // TODO: Main crystal frequency
	m_cpu->set_addrmap(AS_PROGRAM, &news_r4k_state::cpu_map); // TODO: Split PROGRAM/IO?
	//	void set_icache_size(size_t icache_size) { c_icache_size = icache_size; }
	//void set_dcache_size(size_t dcache_size) { c_dcache_size = dcache_size; }
	m_cpu->set_icache_size(16384);
	m_cpu->set_dcache_size(16384);

	RAM(config, m_ram);
	m_ram->set_default_size("64M");

	/*
	R3000A(config, m_cpu, 20_MHz_XTAL, 32768, 32768);
	m_cpu->set_addrmap(AS_PROGRAM, &news_r4k_state::cpu_map);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010Av4);

	// 3 banks of 4x30-pin SIMMs with parity, first bank is soldered
	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	// TODO: confirm each bank supports 4x1M or 4x4M
	m_ram->set_extra_options("4M,8M,12M,20M,24M,32M,36M,48M");
	*/

	//DMAC_0448(config, m_dma, 0);
	//m_dma->set_bus(m_cpu, 0);
	//m_dma->out_int_cb().set(FUNC(news_r4k_state::irq_w<DMA>));
	//m_dma->dma_r_cb<1>().set(m_fdc, FUNC(upd72067_device::dma_r));
	//m_dma->dma_w_cb<1>().set(m_fdc, FUNC(upd72067_device::dma_w));
	// TODO: channel 2 audio
	// TODO: channel 3 video
	/*

	M48T02(config, m_rtc);
	*/

	SCC85C30(config, m_scc, 4.9152_MHz_XTAL);
	m_scc->out_int_callback().set(FUNC(news_r4k_state::irq_w<SCC>));

	// scc channel A
	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
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

	/*
	AM7990(config, m_net);
	m_net->intr_out().set(FUNC(news_r4k_state::irq_w<LANCE>)).invert();
	m_net->dma_in().set([this](offs_t offset) { return m_net_ram[offset >> 1]; });
	m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset >> 1]); });

	UPD72067(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(m_dma, FUNC(dmac_0448_device::irq<1>));
	m_fdc->drq_wr_callback().set(m_dma, FUNC(dmac_0448_device::drq<1>));
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_formats).enable_sound(false);

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);
	// inquiry content for hard disk is "HITACHI DK312C          CS01"
	NSCSI_CONNECTOR(config, "scsi:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", news_scsi_devices, nullptr);

	// scsi host adapter
	NSCSI_CONNECTOR(config, "scsi:7").option_set("cxd1185", CXD1185).clock(16_MHz_XTAL).machine_config(
		[this](device_t *device)
		{
			cxd1185_device &adapter = downcast<cxd1185_device &>(*device);

			adapter.irq_out_cb().set(m_dma, FUNC(dmac_0448_device::irq<0>));
			adapter.drq_out_cb().set(m_dma, FUNC(dmac_0448_device::drq<0>));
			adapter.port_out_cb().set(
				[this](u8 data)
				{
					LOG("floppy %s\n", BIT(data, 0) ? "mount" : "eject");
				});

			subdevice<dmac_0448_device>(":dma")->dma_r_cb<0>().set(adapter, FUNC(cxd1185_device::dma_r));
			subdevice<dmac_0448_device>(":dma")->dma_w_cb<0>().set(adapter, FUNC(cxd1185_device::dma_w));
		});

	SCREEN(config, m_lcd, SCREEN_TYPE_LCD);
	m_lcd->set_raw(52416000, 1120, 0, 1120, 780, 0, 780);
	m_lcd->set_screen_update(FUNC(news_r4k_state::screen_update));

	NEWS_HID_HLE(config, m_hid);
	m_hid->irq_out<news_hid_hle_device::KEYBOARD>().set(FUNC(news_r4k_state::irq_w<KBD>));
	m_hid->irq_out<news_hid_hle_device::MOUSE>().set(FUNC(news_r4k_state::irq_w<MOUSE>));
    */
}

void news_r4k_state::nws5000x(machine_config &config)
{
	common(config);
}

ROM_START(nws5000x)
ROM_REGION32_BE(0x40000, "mrom", 0)
ROM_SYSTEM_BIOS(0, "nws5000x", "APbus System Monitor Release 3.201")
// TODO: change file extension??
ROMX_LOAD("mpu-33__ver3.201__1994_sony.ic64", 0x00000, 0x40000, CRC(8a6ca2b7) SHA1(72d52e24a554c56938d69f7d279b2e65e284fd59), ROM_BIOS(0))

// 2 x MB834200A-20 (4Mb mask ROM)
//ROM_REGION32_BE(0x200000, "krom", ROMREGION_ERASEFF)
//ROM_LOAD64_WORD("051_aa.ic109", 0x00000, 0x80000, CRC(1411cbcb) SHA1(793394cd3919034f85bfb015d6d3c504f83b6626))
//ROM_LOAD64_WORD("052_aa.ic110", 0x00004, 0x80000, CRC(df0f39da) SHA1(076881da022a3fe6731de0ead217285293c25dc7))

ROM_REGION32_BE(0x400, "idrom", 0)
ROM_LOAD("idrom.rom", 0x000, 0x400, CRC(89edfebe) SHA1(3f69ebfaf35610570693edf76aa94c10b30de627) BAD_DUMP)
ROM_END

//   YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT    CLASS           INIT           COMPANY  FULLNAME                      FLAGS
COMP(1994, nws5000x, 0, 0, nws5000x, nws5000, news_r4k_state, init_nws5000x, "Sony", "NET WORK STATION NWS-5000X", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
