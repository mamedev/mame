// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
/*
 * SGI Personal IRIS family driver
 *
 *   Year  Model  Board  Type  CPU    Clock    I/D Cache    Code Name
 *   1988  4D/20  IP6    IP6   R2000  12.5MHz  16KiB/8KiB   Eclipse
 *   1989  4D/25  IP10   IP6   R3000  20MHz    64KiB/32KiB  Eclipse
 *   1991  4D/30  IP14   IP12  R3000  30MHz    64KiB/64KiB  Magnum
 *   1991  4D/35  IP12   IP12  R3000  36MHz    64KiB/64KiB  Magnum
 *
 * Sources:
 *   - http://bitsavers.org/pdf/sgi/personal_iris/VME-Eclipse_CPU_VIP10_Specification.pdf
 *   - http://www.bitsavers.org/pdf/sgi/personal_iris/SGI_IP-6_Schematic.pdf
 *   - http://www.futuretech.blinkenlights.nl/pitechrep.html
 *   - https://hardware.majix.org/computers/sgi.pi/index.shtml
 *   - http://archive.irix.cc/sgistuff/hardware/systems/personal.html
 *   - http://archive.irix.cc/developerforum95/Silicon_Graphics_Developer_Forum_95_The_CD_Volume_2/documents/hw_handbook_html/handbook.html
 *   - https://github.com/NetBSD/src/tree/trunk/sys/arch/sgimips/
 *
 * TODO:
 *   - audio, printer
 *   - devicify ioc1 and ctl1
 *   - 4d30/35 skeleton only
 *
 * Status:
 *   - parity and cache diagnostics fail
 *   - irix 4.0.5 working
 *   - ide test failures
 *     - lca2 (unimplemented fpga program/readback)
 *     - nvram4 (security mode?)
 *     - fpu (cvt.?.? invalid operation exceptions)
 */

#include "emu.h"

// cpu and memory
#include "cpu/mips/mips1.h"
#include "cpu/dsp56000/dsp56000.h"
#include "machine/ram.h"
#include "machine/eepromser.h"

// other devices
#include "machine/wd33c9x.h"
#include "machine/am79c90.h"
#include "machine/mc68681.h"
#include "machine/pit8253.h"
#include "machine/dp8573.h"
#include "machine/z80scc.h"
#include "machine/edlc.h"
#include "machine/input_merger.h"

// buses and connectors
#include "machine/nscsi_bus.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/cd.h"
#include "bus/rs232/rs232.h"
#include "bus/sgikbd/sgikbd.h"
#include "bus/rs232/hlemouse.h"

// video and audio
#include "sgi_gr1.h"

#include "4dpi.lh"

#define LOG_GENERAL (1U << 0)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

#include "softlist.h"

class pi4d2x_state : public driver_device
{
public:
	pi4d2x_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_eeprom(*this, "eeprom")
		, m_rtc(*this, "rtc")
		, m_pit(*this, "pit")
		, m_scsi(*this, "scsi:0:wd33c93")
		, m_enet(*this, "enet")
		, m_duart(*this, "duart%u", 0U)
		, m_serial(*this, "serial%u", 1U)
		, m_gfx(*this, "gfx")
		, m_softlist(*this, "softlist")
		, m_leds(*this, "led%u", 0U)
		, m_vme_isr(0)
	{
	}

	void pi4d20(machine_config &config);
	void pi4d25(machine_config &config);

	void initialize();

private:
	required_device<mips1_device_base> m_cpu;
	required_device<ram_device> m_ram;
	required_device<eeprom_serial_93c56_16bit_device> m_eeprom;

	required_device<dp8573_device> m_rtc;
	required_device<pit8254_device> m_pit;
	required_device<wd33c9x_base_device> m_scsi;
	required_device<am7990_device> m_enet;
	required_device_array<scn2681_device, 2> m_duart;
	required_device_array<rs232_port_device, 2> m_serial;
	required_device<sgi_gr1_device> m_gfx;
	required_device<software_list_device> m_softlist;

	enum leds : unsigned
	{
		LED_HBT = 0, // heartbeat (1Hz)
		LED_CPU = 1, // cpu activity
		LED_GFX = 2, // graphics
		LED_FPU = 3, // fpu present
		LED_CON = 4, // console
	};
	output_finder<5> m_leds;

	void common(machine_config &config);
	void map(address_map &map);

	template <unsigned N> void lio_interrupt(int state) { lio_interrupt(N, state); }
	void lio_interrupt(unsigned number, int state);
	void scsi_drq(int state);

	u8 sysid_r();

	u32 buserror_r(offs_t offset)
	{
		m_cpu->berr_w(1);
		return 0;
	}
	void buserror_w(offs_t offset, u32 data, u32 mem_mask)
	{
		m_cpu->set_input_line(INPUT_LINE_IRQ5, 1);
	}

	enum sysid_mask : u8
	{
		SYSID_SERDATA = 0x01, // serial memory data output state
		SYSID_FPPRES  = 0x02, // floating point processor present (active low)
		SYSID_SERCLK  = 0x04, // serial memory clock
		SYSID_VMEFBT  = 0x04, // vme fast bus timeout
		SYSID_GDMAERR = 0x08, // error in graphics dma
		SYSID_GDMAEN  = 0x10, // graphics dma busy
		SYSID_GDMARDY = 0x20, // asserted at end of graphics dma
		SYSID_GDMARST = 0x40, // asserted in reset of graphics dma
		SYSID_VMERMW  = 0x80, // asserted in vme read-modify-write
	};

	enum lio_int_number : unsigned
	{
		LIO_D0     = 0, // duart 0 interrupt
		LIO_D1     = 1, // duart 1 interrupt
		LIO_VR     = 2, // vertical retrace interrupt
		LIO_CENTR  = 3, // parallel port interrupt
		LIO_SCSI   = 4, // scsi interrupt
		LIO_ENET   = 5, // ethernet interrupt
		LIO_GE     = 6, // ge interrupt
		LIO_FIFO   = 7, // fifo full interrupt
		LIO_AC     = 8, // vme ac fail interrupt
		LIO_VRSTAT = 9, // vert retrace status: no interrupt
	};

	enum memcfg_mask : u8
	{
		MEMCFG_MEMSIZE  = 0x0f, // (n+1)/16 memory populated
		MEMCFG_4MRAM    = 0x10, // 4M DRAMs
		MEMCFG_TIMERDIS = 0x20, // disable timer (active low)
		MEMCFG_FMEM     = 0x40, // reduce cas pulse on reads
		MEMCFG_REFDIS   = 0x80, // disable memory refresh (active low)
	};

	enum cpuctrl_mask : u16
	{
		CPUCTRL_SERDATA = 0x0100, // serial memory data out
		CPUCTRL_SIN     = 0x0200, // system init (reset)
		CPUCTRL_RPAR    = 0x0400, // enable parity checking
		CPUCTRL_SLA     = 0x0800, // enable slave accesses
		CPUCTRL_ARB     = 0x1000, // enable vme arbiter
		CPUCTRL_BAD     = 0x2000, // write bad parity
		CPUCTRL_DOG     = 0x4000, // enable watchdog timer
		CPUCTRL_FPER    = 0x8000, // fast peripheral cycle
	};

	enum cpuauxctl_mask : u8
	{
		CPUAUXCTRL_LED = 0x0f, // diagnostic leds (active low)
		CPUAUXCTRL_PE  = 0x10, // console led (active low)
		CPUAUXCTRL_CS  = 0x20, // eeprom chip select
		CPUAUXCTRL_CLK = 0x40, // serial clock
		CPUAUXCTRL_GR  = 0x80, // graphics reset (active low)
	};

	u8 m_memcfg = 0;
	u8 m_sysid = 0;
	u8 m_vme_isr = 0;
	u8 m_vme_imr = 0;
	u16 m_cpuctrl = 0;
	u8 m_cpuauxctl = 0;
	u32 m_erradr = 0;
	u32 m_refadr = 0;
	attotime m_refresh_timer;

	enum parerr_mask : u8
	{
		PARERR_GDMA  = 0x01,
		PARERR_DMA   = 0x02,
		PARERR_CPU   = 0x04,
		PARERR_VME   = 0x08,
		PARERR_BYTE  = 0xf0,
	};
	u8 m_parerr = 0;

	u16 m_lio_isr = 0;
	u8 m_lio_imr = 0;
	bool m_lio_int = false;
	int m_lio_fifo = 0;

	u16 m_dmalo = 0;
	u8 m_mapindex = 0;
	std::unique_ptr<u16 []> m_dmahi;
	offs_t m_dmaaddr = 0;

	u32 m_gdma_dabr = 0;   // descriptor array base
	u32 m_gdma_bufadr = 0; // buffer address
	u16 m_gdma_burst = 0;  // burst/delay
	u16 m_gdma_buflen = 0; // buffer length
};

class pi4d3x_state : public driver_device
{
public:
	pi4d3x_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_eprom(*this, "boot")
		, m_eeprom(*this, "eeprom")
		, m_rtc(*this, "rtc")
		, m_pit(*this, "pit")
		, m_scsi(*this, "scsi:0:wd33c93a")
		, m_enet(*this, "enet")
		, m_duart(*this, "duart%u", 0U)
		, m_serial(*this, "serial%u", 1U)
		, m_dsp(*this, "dsp")
	{
		std::fill(std::begin(m_lio_isr), std::end(m_lio_isr), 0 );
	}

	void pi4d30(machine_config &config);
	void pi4d35(machine_config &config);

	void initialize();

private:
	required_device<mips1_device_base> m_cpu;
	required_device<ram_device> m_ram;
	required_region_ptr<u16> m_eprom;
	required_device<eeprom_serial_93c56_16bit_device> m_eeprom;

	required_device<dp8573_device> m_rtc;
	required_device<pit8254_device> m_pit;
	required_device<wd33c9x_base_device> m_scsi;
	required_device<seeq8003_device> m_enet;
	required_device_array<z80scc_device, 3> m_duart;
	required_device_array<rs232_port_device, 4> m_serial;
	required_device<dsp56001_device> m_dsp;

	void common(machine_config &config);
	void map(address_map &map);

	template <unsigned LIO, unsigned N> void lio_interrupt(int state) { lio_interrupt(LIO, N, state); }
	void lio_interrupt(unsigned const lio, unsigned const number, int state);

	u8 m_lio_isr[2];
	u8 m_lio_imr[2];
	bool m_lio_int[2];

	//u8 m_sysid = 1; // FPPRES
	u8 m_cpuauxctl = 0;
};

void pi4d2x_state::map(address_map &map)
{
	// silence local memory
	map(0x00000000, 0x0fffffff).noprw();

	// vme address space produces bus errors by default
	map(0x10000000, 0x1effffff).rw(FUNC(pi4d2x_state::buserror_r), FUNC(pi4d2x_state::buserror_w));

	// TODO: 1 32-bit 6U VME slot
	//map(0x10000000, 0x1bffffff); // vme a32 modifier 0x09 non-privileged
	//map(0x1c000000, 0x1cffffff); // vme a24 modifier 0x3d privileged
	//map(0x1d000000, 0x1d0fffff); // vme a16 modifier 0x2d privileged
	//map(0x1d100000, 0x1d1fffff); // vme a16 modifier 0x29 non-privileged
	//map(0x1df00000, 0x1dffffff).umask32(0x0000ff00); // VME_IACK: vme interrupt acknowledge
	//map(0x1e000000, 0x1effffff); // vme a24 modifier 0x39 non-privileged

	//map(0x1f000000, 0x1fbfffff); // local I/O (duarts, timers, etc.)
	map(0x1f000000, 0x1f007fff).m(m_gfx, FUNC(sgi_gr1_device::map)).mirror(0x8000);

	map(0x1f800000, 0x1f800003).lrw8(NAME([this]() { return m_memcfg; }), NAME([this](u8 data) { m_memcfg = data; })).umask32(0xff000000);
	map(0x1f800000, 0x1f800003).r(FUNC(pi4d2x_state::sysid_r)).umask32(0x00ff0000);

	map(0x1f840000, 0x1f840003).lrw8(NAME([this]() { return m_vme_isr; }), NAME([this](u8 data) { m_vme_isr = data; })).umask32(0x000000ff);
	map(0x1f840008, 0x1f84000b).lrw8(NAME([this]() { return m_vme_imr; }), NAME([this](u8 data) { m_vme_imr = data; })).umask32(0x000000ff);

	map(0x1f880000, 0x1f880003).lrw16(
		NAME([this]() { return m_cpuctrl; }),
		[this](u16 data)
		{
			m_eeprom->di_write(BIT(data, 8));

			// reset system
			if (BIT(data, 9))
				machine().schedule_soft_reset();

			//BIT(data, 10); // enable parity checking
			//BIT(data, 11); // enable slave accesses
			//BIT(data, 12); // enable vme arbiter
			//BIT(data, 13); // write bad parity
			//BIT(data, 14); // enable watchdog timer
			//BIT(data, 15); // fast peripheral cycle

			m_cpuctrl = data;
		}, "cpuctrl_w").umask32(0x0000ffff);
	//map(0x1f8c0000, 0x1f8c0003); // lca readback trigger (b)
	map(0x1f8e0000, 0x1f8e0003).lrw8(
		NAME([this]() { return m_cpuauxctl; }),
		[this](u8 data)
		{
			// cpu leds
			m_leds[LED_HBT] = BIT(data, 0);
			m_leds[LED_CPU] = BIT(data, 1);
			m_leds[LED_GFX] = BIT(data, 2);
			m_leds[LED_FPU] = BIT(data, 3);

			// console led
			m_leds[LED_CON] = BIT(data, 4);

			// serial eeprom chip select and clock out
			m_eeprom->cs_write(BIT(data, 5));
			m_eeprom->clk_write(BIT(data, 6));

			m_gfx->reset_w(BIT(data, 7));

			m_cpuauxctl = data;
		}, "cpuauxctl_w").umask32(0xff000000);

	map(0x1f900000, 0x1f900003).lrw16(
		NAME([this]() { return m_dmalo; }),
		[this](u16 data)
		{
			m_dmalo = data;
			m_mapindex = 0;

			m_dmaaddr = (u32(m_dmahi[m_mapindex]) << 12) | (m_dmalo & 0x0ffc);
		}, "dmalo_w").umask32(0x0000ffff);

	map(0x1f910000, 0x1f910003).lrw8(
		NAME([this]() { return m_mapindex; }),
		NAME([this](u8 data) { m_mapindex = data; })).umask32(0x000000ff);

	/*
	 * DMA address mapping table is a pair of CY7C128-35PC 2048x8 SRAMs which
	 * read/write to data bus D27-12. A10 is tied high, giving 1024 entries.
	 */
	map(0x1f920000, 0x1f920fff).lrw16(
		NAME([this](offs_t offset) { return m_dmahi[offset]; }),
		NAME([this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_dmahi[offset]); })).umask32(0x0000ffff);

	// emulation can ignore dma flush
	map(0x1f940000, 0x1f940003).nopw();

	map(0x1f950000, 0x1f9501ff).rw(m_enet, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w)).umask32(0xffff0000);
	map(0x1f960000, 0x1f960003).lr8([this]() { m_enet->reset_w(1); return 0; }, "etherrdy").umask32(0xff000000);
	map(0x1f960004, 0x1f960007).lr8([this]() { m_enet->reset_w(0); return 0; }, "etherrst").umask32(0xff000000);
	//map(0x1f960008, 0x1f96000b).rw().umask32(0xff000000); // etherwait: wait state control

	map(0x1f980000, 0x1f980003).lr16(NAME([this]() { return m_lio_isr; })).umask32(0x0000ffff);
	map(0x1f980008, 0x1f98000b).lrw8(
		NAME([this]() { return m_lio_imr; }),
		[this](u8 data)
		{
			m_lio_imr = data;

			// fifo interrupt status follows line state if not enabled
			if (!BIT(m_lio_imr, LIO_FIFO))
			{
				if (m_lio_fifo)
					m_lio_isr |= (1U << LIO_FIFO);
				else
					m_lio_isr &= ~(1U << LIO_FIFO);
			}

			// update interrupt line
			bool const lio_int = ~m_lio_isr & m_lio_imr;
			if (m_lio_int ^ lio_int)
			{
				m_lio_int = lio_int;
				m_cpu->set_input_line(INPUT_LINE_IRQ1, m_lio_int);
			}
		}, "lio_imr_w").umask32(0x000000ff);

	// 1 0 a7 a6 a5 a4 a3 a2 a1 a0 1 0  lance dmahi
	// 0 0 a7 a6 a5 a4 a3 a2 a1 a0 1 0  scsi dmahi
	// 0 1 a7 a6 a5 a4 a3 a2 a1 a0 1 0  printer/audio dmahi

	// TODO: printer/audio
	//map(0x1f970000, 0x1f970003).r().umask32(0x00ff0000); // pbstat - printer byte status
	//map(0x1f9c0000, 0x1f9c0003).rw().umask32(0x0000ffff); // prdmact - dma byte count
	//map(0x1f9c0004, 0x1f9c0007).w().umask32(0x00ff0000); // aogndac - audio output gain
	//map(0x1f9c0104, 0x1f9c0107).w().umask32(0x00ff0000);
	//map(0x1f9c0204, 0x1f9c0207).r().umask32(0x00ff0000); // prdmast - dma status
	//map(0x1f9c0304, 0x1f9c0307).r().umask32(0x00ff0000); // a/dreg - a/d i/o
	//map(0x1f9d0000, 0x1f9d0003).w().umask32(?); // pchrld - reload registers
	//map(0x1f9d0004, 0x1f9d0007).rw().umask32(0x0000ffff); // prdmalo - dma low addr reg
	//map(0x1f9e0000, 0x1f9e0003).rw().umask32(0x000000ff); // mapindex - printer map index (5-bit)
	//map(0x1f9e0004, 0x1f9e0007).w().umask32(0xff000000); // dmastop
	//map(0x1f9e0008, 0x1f9e000b).w().umask32(0x000000ff); // prswack - soft ack
	//map(0x1f9e000c, 0x1f9e000f).w().umask32(0xff000000); // dmastart
	//map(0x1f9f0000, 0x1f9f0003).r().umask32(0xff000000); // prdy - turn off reset
	//map(0x1f9f0004, 0x1f9f0007).r().umask32(0xff000000); // prst - turn on reset
	//map(0x1f9f0008, 0x1f9f000b).rw().umask32(0x0000ffff); // prdmacn - dma control
	//map(0x1f9f000c, 0x1f9f000f).rw().umask32(0xffffffff); // prdmadr - dma data reg

	// HACK: pass diagnostic iom3: ioc multiplexer registers test
	map(0x1f9c0000, 0x1f9c0003).ram().umask32(0x0000ffff);
	map(0x1f9d0004, 0x1f9d0007).ram().umask32(0x0000ffff);
	map(0x1f9e0000, 0x1f9e0003).ram().umask32(0x000000ff);
	map(0x1f9f000c, 0x1f9f000f).ram().umask32(0xffffffff);

	map(0x1fa00000, 0x1fa00003).lr8([this]() { m_cpu->set_input_line(INPUT_LINE_IRQ4, 0); return 0; }, "timer1_ack").umask32(0xff000000);
	map(0x1fa20000, 0x1fa20003).lr8([this]() { m_cpu->set_input_line(INPUT_LINE_IRQ2, 0); return 0; }, "timer0_ack").umask32(0xff000000);

	map(0x1fa40000, 0x1fa40003).lr32([this]() { m_cpu->set_input_line(INPUT_LINE_IRQ5, 0); m_cpu->berr_w(0); return m_erradr; }, "erradr");

	map(0x1fa40004, 0x1fa40007).lrw32(
		[this]()
		{
			if (m_memcfg & MEMCFG_TIMERDIS)
			{
				// refresh cycle is generated every 64μs
				u64 const refreshes = (machine().time() - m_refresh_timer).as_ticks(15.625_kHz_XTAL);

				// each refresh cycle generates 4 sequential accesses
				// TODO: should the other factor be 1024 for 1M DRAM?
				return u32(m_refadr + refreshes * 4096 * 4);
			}
			else
				return m_refadr;
		}, "refadr_r",
		[this](u32 data)
		{
			m_refadr = data;
			m_refresh_timer = machine().time();
		}, "refadr_w");

	map(0x1fa40008, 0x1fa4000b).lrw32(NAME([this]() { return m_gdma_dabr; }), NAME([this](u32 data) { m_gdma_dabr = data; }));
	map(0x1fa4000c, 0x1fa4000f).lrw32(NAME([this]() { return m_gdma_bufadr; }), NAME([this](u32 data) { m_gdma_bufadr = data; }));
	map(0x1fa40010, 0x1fa40013).lrw16(NAME([this]() { return m_gdma_burst; }), NAME([this](u16 data) { m_gdma_burst = data; })).umask32(0xffff0000);
	map(0x1fa40010, 0x1fa40013).lrw16(NAME([this]() { return m_gdma_buflen; }), NAME([this](u16 data) { m_gdma_buflen = data; })).umask32(0x0000ffff);

	map(0x1fa60000, 0x1fa60003).lrw8([this]() { m_sysid |= SYSID_VMERMW; return 0; }, "vmermw_r", [this](u8 data) { m_sysid |= SYSID_VMERMW; }, "vmermw_w").umask32(0xff000000);
	//map(0x1fa60004, 0x1fa60007).rw("actpup").umask32(0xff000000); // turn on active bus pullup
	map(0x1fa60018, 0x1fa6001b).lrw8([this]() { m_sysid |= SYSID_VMEFBT; return 0; }, "vmefbon_r", [this](u8 data) { m_sysid |= SYSID_VMEFBT; }, "vmefbon_w").umask32(0xff000000);
	map(0x1fa6001c, 0x1fa6001f).lrw8([this]() { m_sysid &= ~SYSID_VMEFBT; return 0; }, "vmefbof_r", [this](u8 data) { m_sysid &= ~SYSID_VMEFBT; }, "vmefbof_w").umask32(0xff000000);
	map(0x1fa60020, 0x1fa60023).nopr(); // reload gfx dma burst/delay reg (FIXME: silenced)
	//map(0x1fa60024, 0x1fa60027).rw("enraso").umask32(0xff000000); // enable ctl ras decoder

	map(0x1fa80000, 0x1fa80003).lr8([this]() { m_scsi->reset_w(0); return 0; }, "scsirdy").umask32(0xff000000);
	map(0x1fa80004, 0x1fa80007).lr8([this]() { m_scsi->reset_w(1); return 0; }, "scsirst").umask32(0xff000000);
	map(0x1fa80008, 0x1fa8000b).lr8([]() { return 0; }, "scsibstat").umask32(0x00ff0000);

	// TODO: IOC2 configuration register, bus error on IOC1
	//map(0x1fa80008, 0x1fa8000b).rw(FUNC(pi4d2x_state::buserror_r), FUNC(pi4d2x_state::buserror_w));

	map(0x1faa0000, 0x1faa0003).lrw8([this](offs_t offset) { m_parerr &= ~(PARERR_BYTE | (1 << offset)); return 0; }, "clrerr_r", [this](offs_t offset) { m_parerr &= ~(PARERR_BYTE | (1 << offset)); }, "clrerr_w");
	map(0x1faa0004, 0x1faa0007).lr8(NAME([this]() { return m_parerr; })).umask32(0x00ff0000);

	map(0x1fac0000, 0x1fac0003).lrw8([this]() { lio_interrupt<LIO_VR>(1); return 0; }, "vrrst_r", [this](u8 data) { lio_interrupt<LIO_VR>(1); }, "vrrst_w").umask32(0xff000000);

	map(0x1fb00000, 0x1fb00003).rw(m_scsi, FUNC(wd33c93_device::indir_addr_r), FUNC(wd33c93_device::indir_addr_w)).umask32(0x00ff0000);
	map(0x1fb00100, 0x1fb00103).rw(m_scsi, FUNC(wd33c93_device::indir_reg_r), FUNC(wd33c93_device::indir_reg_w)).umask32(0x00ff0000);

	map(0x1fb40000, 0x1fb4000f).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask32(0xff000000);

	map(0x1fb80000, 0x1fb800ff).lrw8(
		NAME([this](offs_t offset) { return m_duart[BIT(offset, 0)]->read(offset >> 2); }),
		NAME([this](offs_t offset, u8 data) { m_duart[BIT(offset, 0)]->write(offset >> 2, data); })).umask32(0xff000000);

	map(0x1fbc0000, 0x1fbc007f).rw(m_rtc, FUNC(dp8573_device::read), FUNC(dp8573_device::write)).umask32(0xff000000);

	map(0x1fc00000, 0x1fc3ffff).rom().region("boot", 0);

	// unused memory address space produces bus errors
	map(0x40000000, 0xffffffff).rw(FUNC(pi4d2x_state::buserror_r), FUNC(pi4d2x_state::buserror_w));
}

void pi4d3x_state::map(address_map &map)
{

	//map(0x1fa00000, 0x1fa00003).noprw().umask32(0xffff0000); // CPUCTRL
	// RSTCONFIG
	//map(0x1fa00008, 0x1fa0000b).lr8([this](){ return m_sysid; }, "sysid").umask32(0xff000000);
	// MEMCFG0
	// MEMCFG1
	// REFTIM
	// PARERR
	//map(0x1fa10210, 0x1fa10213).nopw(); // CLERERR

	map(0x1fb80000, 0x1fb800bf).ram(); // FIXME: stub out ethernet, scsi and parallel registers

	map(0x1fb80100, 0x1fb8011f).m(m_enet, FUNC(seeq8003_device::map)).umask32(0x000000ff);

	map(0x1fb80120, 0x1fb80123).rw(m_scsi, FUNC(wd33c93_device::indir_addr_r), FUNC(wd33c93_device::indir_addr_w)).umask32(0x0000ff00);
	map(0x1fb80124, 0x1fb80127).rw(m_scsi, FUNC(wd33c93_device::indir_reg_r), FUNC(wd33c93_device::indir_reg_w)).umask32(0x0000ff00);

	map(0x1fb80180, 0x1fb801bb).ram(); // FIXME: stub out pbus registers
	map(0x1fb801bc, 0x1fb801bf).lrw8(
		[this]()
		{
			u8 data = m_cpuauxctl & ~0x10;

			// serial eeprom data in
			if (m_eeprom->do_read())
				data |= 0x10;

			return data;
		}, "cpu_aux_ctrl_r",
		[this](u8 data)
		{
			// console LED / protection register enable (PRE)
			//m_leds[LED_CON] = BIT(data, 0);

			// serial eeprom chip select, clock and data out
			m_eeprom->cs_write(BIT(data, 1));
			m_eeprom->clk_write(BIT(data, 2));
			m_eeprom->di_write(BIT(data, 3));

			m_cpuauxctl = data;
		}, "cpu_aux_ctrl_w").umask32(0x000000ff);

	map(0x1fb801c0, 0x1fb801c3).lr8(NAME([this]() { return m_lio_isr[0]; })).umask32(0x000000ff);
	map(0x1fb801c4, 0x1fb801c7).lrw8(
		NAME([this]() { return m_lio_imr[0]; }),
		[this](u8 data)
		{
			m_lio_imr[0] = data;

			// update interrupt line
			bool const lio_int = m_lio_isr[0] & m_lio_imr[0];
			if (m_lio_int[0] ^ lio_int)
			{
				m_lio_int[0] = lio_int;
				m_cpu->set_input_line(INPUT_LINE_IRQ1, lio_int);
			}
		}, "lio0_imr_w").umask32(0x000000ff);

	map(0x1fb801c8, 0x1fb801cb).lr8(NAME([this]() { return m_lio_isr[1]; })).umask32(0x000000ff);
	map(0x1fb801cc, 0x1fb801cf).lrw8(
		NAME([this]() { return m_lio_imr[1]; }),
		[this](u8 data)
		{
			m_lio_imr[1] = data;

			// update interrupt line
			bool const lio_int = m_lio_isr[1] & m_lio_imr[1];
			if (m_lio_int[1] ^ lio_int)
			{
				m_lio_int[1] = lio_int;
				m_cpu->set_input_line(INPUT_LINE_IRQ2, lio_int);
			}
		}, "lio1_imr_w").umask32(0x000000ff);

	map(0x1fb801d0, 0x1fb801df).ram(); // FIXME: stub out int2 registers
	map(0x1fb801e0, 0x1fb801e3).lw8(
		[this](u8 data)
		{
			// timer 0 acknowledge
			if (BIT(data, 0))
				m_cpu->set_input_line(INPUT_LINE_IRQ3, 0);

			// timer 1 acknowledge
			if (BIT(data, 1))
				m_cpu->set_input_line(INPUT_LINE_IRQ4, 0);
		}, "timer_ack").umask32(0x000000ff);
	map(0x1fb801f0, 0x1fb801ff).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask32(0x000000ff);

	map(0x1fb80d00, 0x1fb80d0f).rw(m_duart[0], FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0x000000ff);
	map(0x1fb80d10, 0x1fb80d1f).rw(m_duart[1], FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0x000000ff);
	map(0x1fb80d20, 0x1fb80d2f).rw(m_duart[2], FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0x000000ff);

	map(0x1fb80e00, 0x1fb80e7f).rw(m_rtc, FUNC(dp8573_device::read), FUNC(dp8573_device::write)).umask32(0x000000ff);

	map(0x1fbe0000, 0x1fbfffff).ram().share("dsp_sram"); // 3xTC55328J-25 32KiB CMOS static RAM

	map(0x1fbd0000, 0x1fbd0003).nopr(); // FIXME: board revision register

	map(0x1fc00000, 0x1fc7ffff).lr16([this](offs_t offset) { return m_eprom[offset]; }, "boot");
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM_SGI).machine_config(
		[](device_t *device)
		{
			downcast<nscsi_cdrom_device &>(*device).set_block_size(512);
		});
	device.option_add("harddisk", NSCSI_HARDDISK);
}

void pi4d2x_state::pi4d20(machine_config &config)
{
	R3000(config, m_cpu, 25_MHz_XTAL / 2, 16384, 8192);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010);

	common(config);
}

void pi4d2x_state::pi4d25(machine_config &config)
{
	R3000(config, m_cpu, 20_MHz_XTAL, 32768, 65536);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010);

	common(config);
}

void pi4d3x_state::pi4d30(machine_config &config)
{
	R3000A(config, m_cpu, 30_MHz_XTAL, 65536, 65536);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010A);

	common(config);
}

void pi4d3x_state::pi4d35(machine_config &config)
{
	R3000A(config, m_cpu, 36_MHz_XTAL, 65536, 65536);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010A);

	common(config);
}

void pi4d2x_state::common(machine_config &config)
{
	m_cpu->set_endianness(ENDIANNESS_BIG);
	m_cpu->set_addrmap(AS_PROGRAM, &pi4d2x_state::map);
	m_cpu->in_brcond<0>().set([]() { return 1; }); // writeback complete

	// 16 SIMM slots with 1, 2? or 4MB SIMMs installed in sets of 4
	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("4M,8M,12M,32M,48M,64M");
	m_ram->set_default_value(0);

	EEPROM_93C56_16BIT(config, m_eeprom);

	DP8573(config, m_rtc); // DP8572AN

	PIT8254(config, m_pit);
	m_pit->set_clk<2>(3.6864_MHz_XTAL);
	m_pit->out_handler<0>().set([this](int state) { if (state) m_cpu->set_input_line(INPUT_LINE_IRQ2, 1); });
	m_pit->out_handler<1>().set([this](int state) { if (state) m_cpu->set_input_line(INPUT_LINE_IRQ4, 1); });
	m_pit->out_handler<2>().set(m_pit, FUNC(pit8254_device::write_clk0));
	m_pit->out_handler<2>().append(m_pit, FUNC(pit8254_device::write_clk1));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0").option_set("wd33c93", WD33C93).machine_config(
		[this](device_t *device)
		{
			wd33c9x_base_device &wd33c93(downcast<wd33c9x_base_device &>(*device));

			wd33c93.set_clock(10000000);
			wd33c93.irq_cb().set(*this, FUNC(pi4d2x_state::lio_interrupt<LIO_SCSI>)).invert();
			wd33c93.drq_cb().set(*this, FUNC(pi4d2x_state::scsi_drq));
		});
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, "cdrom", false);
	NSCSI_CONNECTOR(config, "scsi:7", scsi_devices, nullptr, false);

	AM7990(config, m_enet);
	m_enet->intr_out().set(FUNC(pi4d2x_state::lio_interrupt<LIO_ENET>));
	m_enet->dma_in().set(
		[this](offs_t offset)
		{
			unsigned const page = 0x200 + ((offset >> 12) & 0xff);
			u32 const address = (u32(m_dmahi[page]) << 12) | (offset & 0xfff);

			return m_cpu->space(0).read_word(address);
		});
	m_enet->dma_out().set(
		[this](offs_t offset, u16 data, u16 mem_mask)
		{
			unsigned const page = 0x200 + ((offset >> 12) & 0xff);
			u32 const address = (u32(m_dmahi[page]) << 12) | (offset & 0xfff);

			m_cpu->space(0).write_word(address, data, mem_mask);
		});

	// duart 0 (keyboard/mouse)
	SCN2681(config, m_duart[0], 3.6864_MHz_XTAL); // SCN2681AC1N24
	sgi_keyboard_port_device &keyboard_port(SGIKBD_PORT(config, "keyboard_port", default_sgi_keyboard_devices, "hlekbd"));
	rs232_port_device &mouse_port(RS232_PORT(config, "mouse_port",
		[](device_slot_interface &device)
		{
			device.option_add("mouse", SGI_HLE_SERIAL_MOUSE);
		},
		"mouse"));

	// duart 0 outputs
	m_duart[0]->irq_cb().set(FUNC(pi4d2x_state::lio_interrupt<LIO_D0>)).invert();
	m_duart[0]->a_tx_cb().set(keyboard_port, FUNC(sgi_keyboard_port_device::write_txd));
	m_duart[0]->b_tx_cb().set(mouse_port, FUNC(rs232_port_device::write_txd));

	// duart 0 inputs
	keyboard_port.rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_a_w));
	mouse_port.rxd_handler().set(m_duart[0], FUNC(scn2681_device::rx_b_w));

	// duart 1 (serial ports)
	SCN2681(config, m_duart[1], 3.6864_MHz_XTAL); // SCN2681AC1N40
	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);

	// duart 1 outputs
	m_duart[1]->irq_cb().set(FUNC(pi4d2x_state::lio_interrupt<LIO_D1>)).invert();
	m_duart[1]->a_tx_cb().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_duart[1]->b_tx_cb().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_duart[1]->outport_cb().set(
		[this](u8 data)
		{
			m_serial[0]->write_rts(BIT(data, 0));
			m_serial[1]->write_rts(BIT(data, 1));
			m_duart[1]->ip5_w(BIT(data, 3));
			m_duart[1]->ip6_w(BIT(data, 3));
			m_serial[0]->write_dtr(BIT(data, 4));
			m_serial[1]->write_dtr(BIT(data, 5));
		});

	// duart 1 inputs
	m_serial[0]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_a_w));
	m_serial[0]->cts_handler().set(m_duart[1], FUNC(scn2681_device::ip0_w));
	m_serial[0]->dcd_handler().set(m_duart[1], FUNC(scn2681_device::ip3_w));

	m_serial[1]->rxd_handler().set(m_duart[1], FUNC(scn2681_device::rx_b_w));
	m_serial[1]->cts_handler().set(m_duart[1], FUNC(scn2681_device::ip1_w));
	m_serial[1]->dcd_handler().set(m_duart[1], FUNC(scn2681_device::ip2_w));

	// graphics
	SGI_GR1(config, m_gfx);
	m_gfx->out_vblank().set(
		[this](int state)
		{
			if (state)
			{
				m_lio_isr &= ~(1U << LIO_VRSTAT);
				lio_interrupt<LIO_VR>(0);
			}
			else
				m_lio_isr |= (1U << LIO_VRSTAT);
		});
	m_gfx->out_int().set(*this, FUNC(pi4d2x_state::lio_interrupt<LIO_GE>)).invert();
	m_gfx->out_int_fifo().set(*this, FUNC(pi4d2x_state::lio_interrupt<LIO_FIFO>)).invert();

	// TODO: vme slot, cpu interrupt 0

	SOFTWARE_LIST(config, m_softlist).set_original("sgi_mips");

	config.set_default_layout(layout_4dpi);
}

void pi4d3x_state::common(machine_config &config)
{
	m_cpu->set_endianness(ENDIANNESS_BIG);
	m_cpu->set_addrmap(AS_PROGRAM, &pi4d3x_state::map);
	m_cpu->in_brcond<0>().set([]() { return 1; }); // writeback complete

	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("8M,32M,64M,128M");
	m_ram->set_default_value(0);

	EEPROM_93C56_16BIT(config, m_eeprom);

	DP8573(config, m_rtc); // DP8572AV

	// 1: local0
	// 2: local1
	// 3: 8254.0
	// 4: 8254.1
	// int 7, 11 -> mappable?
	// level < 8 -> local0
	// level < 16 -> local1
	// level < 24 -> map0
	// level < 32 -> map1

	// FIXME: part of INT2 asic
	PIT8254(config, m_pit);
	m_pit->set_clk<2>(1_MHz_XTAL);
	m_pit->out_handler<0>().set([this](int state) { if (state) m_cpu->set_input_line(INPUT_LINE_IRQ3, 1); });
	m_pit->out_handler<1>().set([this](int state) { if (state) m_cpu->set_input_line(INPUT_LINE_IRQ4, 1); });
	m_pit->out_handler<2>().set(m_pit, FUNC(pit8254_device::write_clk0));
	m_pit->out_handler<2>().append(m_pit, FUNC(pit8254_device::write_clk1));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0").option_set("wd33c93a", WD33C93A).machine_config(
		[this](device_t *device)
		{
			wd33c9x_base_device &wd33c93(downcast<wd33c9x_base_device &>(*device));

			wd33c93.set_clock(10'000'000);
			wd33c93.irq_cb().set(*this, &pi4d3x_state::lio_interrupt<0, 2>, "lio0.2");
			//wd33c93.drq_cb().set(*this, FUNC(pi4d2x_state::scsi_drq));
		});
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7", scsi_devices, nullptr, false);

	SEEQ8003(config, m_enet, 0);
	m_enet->out_int_cb().set(&pi4d3x_state::lio_interrupt<0, 3>, "lio0.3");

	input_merger_device &duart_int(INPUT_MERGER_ANY_HIGH(config, "duart_int"));
	duart_int.output_handler().set(&pi4d3x_state::lio_interrupt<0, 5>, "lio0.5");

	// duart 0: keyboard/mouse
	SCC85C30(config, m_duart[0], 10_MHz_XTAL); // Z8513010VSC ESCC
	m_duart[0]->out_int_callback().set(duart_int, FUNC(input_merger_device::in_w<0>));

	// keyboard
	sgi_keyboard_port_device &keyboard_port(SGIKBD_PORT(config, "keyboard_port", default_sgi_keyboard_devices, "hlekbd"));
	m_duart[0]->out_txdb_callback().set(keyboard_port, FUNC(sgi_keyboard_port_device::write_txd));
	keyboard_port.rxd_handler().set(m_duart[0], FUNC(z80scc_device::rxb_w));

	// mouse
	rs232_port_device &mouse_port(RS232_PORT(config, "mouse_port",
		[](device_slot_interface &device)
		{
			device.option_add("mouse", SGI_HLE_SERIAL_MOUSE);
		},
		"mouse"));
	m_duart[0]->out_txda_callback().set(mouse_port, FUNC(rs232_port_device::write_txd));
	mouse_port.rxd_handler().set(m_duart[0], FUNC(z80scc_device::rxa_w));

	// duart 1: serial ports
	SCC85C30(config, m_duart[1], 10_MHz_XTAL); // Z8513010VSC ESCC
	m_duart[1]->configure_channels(3'686'400, 0, 3'686'400, 0);
	m_duart[1]->out_int_callback().set(duart_int, FUNC(input_merger_device::in_w<1>));

	// serial port 1
	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);
	m_duart[1]->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
	m_duart[1]->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_duart[1]->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_serial[0]->cts_handler().set(m_duart[1], FUNC(z80scc_device::ctsa_w));
	m_serial[0]->dcd_handler().set(m_duart[1], FUNC(z80scc_device::dcda_w));
	m_serial[0]->rxd_handler().set(m_duart[1], FUNC(z80scc_device::rxa_w));

	// serial port 2
	RS232_PORT(config, m_serial[1], default_rs232_devices, "terminal");
	m_duart[1]->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));
	m_duart[1]->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_duart[1]->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_serial[1]->cts_handler().set(m_duart[1], FUNC(z80scc_device::ctsb_w));
	m_serial[1]->dcd_handler().set(m_duart[1], FUNC(z80scc_device::dcdb_w));
	m_serial[1]->rxd_handler().set(m_duart[1], FUNC(z80scc_device::rxb_w));

	// duart 2: "Apple" RS-422 serial ports
	SCC85C30(config, m_duart[2], 10_MHz_XTAL); // Z8513010VSC ESCC
	m_duart[2]->out_int_callback().set(duart_int, FUNC(input_merger_device::in_w<2>));

	// serial port 3
	// FIXME: HSKO/HSKI/GPI
	RS232_PORT(config, m_serial[2], default_rs232_devices, nullptr);
	m_duart[2]->out_txda_callback().set(m_serial[2], FUNC(rs232_port_device::write_txd));
	m_serial[2]->rxd_handler().set(m_duart[2], FUNC(z80scc_device::rxa_w));

	// serial port 4
	// FIXME: HSKO/HSKI/GPI
	RS232_PORT(config, m_serial[3], default_rs232_devices, nullptr);
	m_duart[2]->out_txdb_callback().set(m_serial[3], FUNC(rs232_port_device::write_txd));
	m_serial[3]->rxd_handler().set(m_duart[2], FUNC(z80scc_device::rxb_w));

	DSP56001(config, m_dsp, 20_MHz_XTAL);
}

void pi4d2x_state::initialize()
{
	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());

	m_memcfg = 0;
	m_sysid = 0;

	m_lio_isr = 0x3ff;
	m_lio_imr = 0;
	m_lio_int = false;

	m_refresh_timer = machine().time();

	m_dmahi = make_unique_clear<u16 []>(2048);

	m_leds.resolve();
}

void pi4d3x_state::initialize()
{
	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());
}

void pi4d2x_state::lio_interrupt(unsigned number, int state)
{
	u16 const mask = 1 << number;

	if (number == LIO_FIFO)
	{
		m_lio_fifo = state;

		// special handling for enabled fifo interrupt
		if ((m_lio_imr & mask) && !(m_lio_isr & mask))
			return;
	}

	// record interrupt state
	if (state)
		m_lio_isr |= mask;
	else
		m_lio_isr &= ~mask;

	// update interrupt line
	bool const lio_int = ~m_lio_isr & m_lio_imr;
	if (m_lio_int ^ lio_int)
	{
		m_lio_int = lio_int;
		m_cpu->set_input_line(INPUT_LINE_IRQ1, m_lio_int);
	}
}

void pi4d3x_state::lio_interrupt(unsigned const lio, unsigned const number, int state)
{
	u8 const mask = 1 << number;

	// record interrupt state
	if (state)
		m_lio_isr[lio] |= mask;
	else
		m_lio_isr[lio] &= ~mask;

	// update interrupt line
	bool const lio_int = m_lio_isr[lio] & m_lio_imr[lio];
	if (m_lio_int[lio] ^ lio_int)
	{
		m_lio_int[lio] = lio_int;

		m_cpu->set_input_line(lio ? INPUT_LINE_IRQ2 : INPUT_LINE_IRQ1, lio_int);
	}
}

void pi4d2x_state::scsi_drq(int state)
{
	if (state)
	{
		if (m_dmalo & 0x8000)
			m_cpu->space(0).write_byte(m_dmaaddr++, m_scsi->dma_r());
		else
			m_scsi->dma_w(m_cpu->space(0).read_byte(m_dmaaddr++));

		if (!(m_dmaaddr & 0xfff))
			m_dmaaddr = u32(m_dmahi[++m_mapindex]) << 12;
	}
}

u8 pi4d2x_state::sysid_r()
{
	u8 data = m_sysid;

	if (m_eeprom->do_read())
		data |= SYSID_SERDATA;

	return data;
}

ROM_START(4d20)
	ROM_REGION32_BE(0x40000, "boot", 0)
	// 3.2e firmware has been found in both 4D/20 and 4D/25 hardware
	ROM_SYSTEM_BIOS(0, "3.2e", "Version 3.2 Rev E, Fri Jul 14 14:37:38 PDT 1989 SGI")
	ROMX_LOAD("070_8000_007_boot_0.h1c5", 0x000000, 0x010000, CRC(e448b865) SHA1(f0276b76360ea0b3250dbdaa7a1e57ea8f6144d6), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_007_boot_1.h1d2", 0x000001, 0x010000, CRC(59fda717) SHA1(ef3ccb1f8a815e7b13c79deeea0d73006deed09f), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_007_boot_2.h1d9", 0x000002, 0x010000, CRC(569146ad) SHA1(5442a13ed93afdaa55c1951b97e335cf60dde834), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_007_boot_3.h1e6", 0x000003, 0x010000, CRC(682977c3) SHA1(d9bcf7cdc5caef4221929fe26eccf34253fa7f29), ROM_BIOS(0) | ROM_SKIP(3))

	ROM_SYSTEM_BIOS(1, "3.1c", "Version 4D1-3.1 Rev C, Tue Jan 10 15:11:42 PST 1989 SGI")
	ROMX_LOAD("070_8000_005_boot_0.h1c5", 0x000000, 0x010000, CRC(c7a182de) SHA1(56038f54b5a3254960ad7c8232f1a7cf058b9ead), ROM_BIOS(1) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_005_boot_1.h1d2", 0x000001, 0x010000, CRC(4b1395f5) SHA1(926f3172b79ebaf7040ff04b0cfdc3d48d03293c), ROM_BIOS(1) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_005_boot_2.h1d9", 0x000002, 0x010000, CRC(e0a55120) SHA1(0b675489ea94bf85a5a0e5f0ebf0c0b7ff5fc389), ROM_BIOS(1) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_005_boot_3.h1e6", 0x000003, 0x010000, CRC(11536526) SHA1(5149f453347ae566e9fee4447615dff88c7f6a37), ROM_BIOS(1) | ROM_SKIP(3))

	ROM_SYSTEM_BIOS(2, "3.1a", "Version 4D1-3.1 Rev A, Tue Aug  9 17:50:39 PDT 1988 SGI")
	ROMX_LOAD("070_8000_003_boot_0.h1c5", 0x000000, 0x010000, CRC(32cdcdc5) SHA1(1568ae3877193d3c93bdcccd755736b6cd82cbf3), ROM_BIOS(2) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_003_boot_1.h1d2", 0x000001, 0x010000, CRC(4b3a02ee) SHA1(248e5cea8d3218686a044e5b013a1213441a5332), ROM_BIOS(2) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_003_boot_2.h1d9", 0x000002, 0x010000, CRC(3d80d19c) SHA1(42fb95f4d76ede073c5d9bbc2f144b4f2fbf407f), ROM_BIOS(2) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_003_boot_3.h1e6", 0x000003, 0x010000, CRC(48b9322c) SHA1(600554a0ccc4bab4881a96a9886eea40cd04c8e4), ROM_BIOS(2) | ROM_SKIP(3))
ROM_END

ROM_START(4d25)
	ROM_REGION32_BE(0x40000, "boot", 0)
	ROM_SYSTEM_BIOS(0, "3.2e", "Version 3.2 Rev E, Fri Jul 14 14:37:38 PDT 1989 SGI")
	ROMX_LOAD("070_8000_007_boot_0.h1c5", 0x000000, 0x010000, CRC(e448b865) SHA1(f0276b76360ea0b3250dbdaa7a1e57ea8f6144d6), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_007_boot_1.h1d2", 0x000001, 0x010000, CRC(59fda717) SHA1(ef3ccb1f8a815e7b13c79deeea0d73006deed09f), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_007_boot_2.h1d9", 0x000002, 0x010000, CRC(569146ad) SHA1(5442a13ed93afdaa55c1951b97e335cf60dde834), ROM_BIOS(0) | ROM_SKIP(3))
	ROMX_LOAD("070_8000_007_boot_3.h1e6", 0x000003, 0x010000, CRC(682977c3) SHA1(d9bcf7cdc5caef4221929fe26eccf34253fa7f29), ROM_BIOS(0) | ROM_SKIP(3))
ROM_END

ROM_START(4d30)
	ROM_REGION16_BE(0x80000, "boot", 0)
	ROM_SYSTEM_BIOS(0, "4.0.1c", "SGI Version 4.0.1 Rev C GR1/GR2/LG1,  Feb 14, 1992")
	ROMX_LOAD("ip14prom.bin", 0x000000, 0x080000, NO_DUMP, ROM_BIOS(0))
ROM_END

ROM_START(4d35)
	ROM_REGION16_BE(0x80000, "boot", 0) // TC574096D-120 (262,144x16-bit EEPROM)

	ROM_SYSTEM_BIOS(0, "4.0.1d", "SGI Version 4.0.1 Rev D LG1/GR2,  Mar 24, 1992")
	ROMX_LOAD("ip12prom.002be.u61", 0x000000, 0x040000, CRC(d35f105c) SHA1(3d08dfb961d7512bd8ed41cb6e01e89d14134f09), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "4.0.1c", "SGI Version 4.0.1 Rev C GR1/GR2/LG1,  Feb 14, 1992")
	ROMX_LOAD("ip12prom.070-8086-002.u61", 0x000000, 0x080000, CRC(543cfc3f) SHA1(a7331876f11bff40c960f822923503eca17191c5), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "4.0a", "SGI Version 4.0 Rev A IP12,  Aug 22, 1991")
	ROMX_LOAD("ip12prom.070-8045-002.u61", 0x000000, 0x040000, CRC(fe999bae) SHA1(eb054c365a6e018be3b9ae44169c0ffc6447c6f0), ROM_BIOS(2))
ROM_END

//   YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY                 FULLNAME                FLAGS
COMP(1988, 4d20, 0,      0,      pi4d20,  0,     pi4d2x_state, initialize, "Silicon Graphics Inc", "Personal IRIS 4D/20",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP(1989, 4d25, 0,      0,      pi4d25,  0,     pi4d2x_state, initialize, "Silicon Graphics Inc", "Personal IRIS 4D/25",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP(1991, 4d30, 0,      0,      pi4d30,  0,     pi4d3x_state, initialize, "Silicon Graphics Inc", "Personal IRIS 4D/30",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP(1991, 4d35, 0,      0,      pi4d35,  0,     pi4d3x_state, initialize, "Silicon Graphics Inc", "Personal IRIS 4D/35",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
