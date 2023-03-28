// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Omron Luna 88K and 88K² systems.
 *
 * Sources:
 *  - Tetsuya Isaki's nono Luna emulator (http://www.pastel-flower.jp/~isaki/nono/)
 *  - OpenBSD source code
 *
 * TODO:
 *  - scsi controller
 *  - xp i/o controller
 *  - lance memory
 *  - crt controller
 *  - slotify graphics
 *  - expansion slots
 *  - abort/power switches
 *  - multi-cpu configurations
 *
 * WIP:
 *  - scsi probe/boot hangs
 */

/*
 * Omron 3W4SX-9100/DT9581 UNIX WORKSTATION
 *
 * PWB7187 (MAIN)
 * --
 * MB89352A                 SCSI
 * D7201AC                  serial (keyboard/mouse + port A)
 * HD647180X0FS6            I/O processor (serial port -00 and -01, printer)
 * TF116NF
 * TF175NF
 * HM62256ALFP-10T * 5      32,768x8 static RAM
 * MB89352FP                SCSI
 * MDS74ACE36X646
 * TF177HF
 * MC88915FN70              clock driver
 * DS1000M-25               delay line
 * HN27C1024HCC-85 * 2      PROM
 * DS1397 RTC/NVRAM         MC146818 RTC + 4Kx8 NVRAM
 * M5M82C55A * 2            PPI
 * OSC1 12.288MHz           IOP clock
 * OSC2 33.333MHz           CPU clock
 * OSC3 50MHz
 * OSC4 200Hz
 * OSC5 8MHz
 * KSS EXO3 3C 19.660M      PC-98 bus?
 * Ports: keyboard, SCSI, RS232C-A, -01, -00, printer
 *
 * PWB7188 (CPU+MMU)
 * --
 * MC88100RC33
 * MC88200RC33 * 2
 *
 * PWB87132 (W-LAN)
 * --
 * AM7990JC/80
 * DS1220Y-200 NVRAM
 * AM7992BDC
 * OSC1 40MHz
 * Another AM7990 and AM7992B are unpopulated
 *
 * PWB7131 BM08HR (video)
 * --
 * Bt458LPJ124 RAMDAC
 * HD6445CP4
 * Bt438KPJ
 * OSC1 108.9920MHz
 * HM53462ZP-10 * 8*8
 */

#include "emu.h"

#include "luna_kbd.h"

#include "cpu/m88000/m88000.h"
#include "cpu/z180/hd647180x.h"

#include "machine/am79c90.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/mb89352.h"
#include "machine/mc146818.h"
#include "machine/mc88200.h"
#include "machine/ram.h"
#include "machine/z80sio.h"
#include "video/bt45x.h"
#include "video/hd44780.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"

#include "emupal.h"
#include "screen.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class luna_88k_state : public driver_device
{
public:
	luna_88k_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_cmmu(*this, "cmmu%u", 0U)
		, m_ram(*this, "ram")
		, m_iop(*this, "iop")
		, m_rtc(*this, "rtc")
		, m_sio(*this, "sio")
		, m_pio(*this, "pio%u", 0U)
		, m_serial(*this, "serial%u", 0U)
		, m_spc(*this, "spc%u", 0U)
		, m_net(*this, "net%u", 0U)
		, m_ramdac(*this, "ramdac")
		, m_vram(*this, "vram", 0x20'0000, ENDIANNESS_BIG)
		, m_lcdc(*this, "lcdc")
		, m_eprom(*this, "eprom")
		, m_iop_ram(*this, "iop_ram")
		, m_boot(*this, "boot")
		, m_sw(*this, "SW%u", 1U)
		, m_irq_state(false)
		, m_irq_active{}
	{
	}

	void luna88k2(machine_config &config);
	void init();

	DECLARE_INPUT_CHANGED_MEMBER(abort) { irq(0, 7, newval); }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void cpu_map(address_map &map);
	void iop_map_mem(address_map &map);
	void iop_map_pio(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	void plane_common_w(offs_t offset, u32 data, u32 mem_mask);
	void logic_common_w(offs_t offset, u32 data);

	template <unsigned Plane> u32 plane_r(offs_t offset, u32 data);
	template <unsigned Plane> void plane_w(offs_t offset, u32 data, u32 mem_mask);
	template <unsigned Plane> void logic_w(offs_t offset, u32 data);

	u32 irq_ctl_r(offs_t offset);
	void irq_ctl_w(offs_t offset, u32 data);

	void irq(unsigned cpu, unsigned interrupt, int state);
	template <unsigned CPU, unsigned Interrupt> void irq(int state) { irq(CPU, Interrupt, state); }

	void irq_check();

	required_device<mc88100_device> m_cpu;
	required_device_array<mc88200_device, 2> m_cmmu;
	required_device<ram_device> m_ram;
	required_device<hd647180x_device> m_iop;
	required_device<mc146818_device> m_rtc;
	required_device<upd7201_device> m_sio;
	required_device_array<i8255_device, 2> m_pio;
	required_device_array<rs232_port_device, 2> m_serial;
	required_device_array<mb89352_device, 2> m_spc;
	required_device_array<am7990_device, 2> m_net;

	required_device<bt458_device> m_ramdac;
	memory_share_creator<u32> m_vram;
	required_device<ks0066_f00_device> m_lcdc;

	required_region_ptr<u32> m_eprom;
	required_shared_ptr<u8> m_iop_ram;
	memory_view m_boot;

	required_ioport_array<2> m_sw;

	// video state
	u8 m_plane_active;
	u8 m_plane_func[8];
	u32 m_plane_mask[8];

	// interrupt state
	bool m_irq_state;
	u8 m_irq_active[4];
	u8 m_irq_mask[4];
};

void luna_88k_state::init()
{
	// HACK: bypass abort switch test failure
	m_eprom[0x1fa58 >> 2] = 0xf4406000;

	// HACK: bypass xp int2/int5 test failure
	m_eprom[0x1fa8c >> 2] = 0xf4406000;

	// HACK: bypass power switch test failure
	m_eprom[0x1e48c >> 2] = 0xf5806000;

	// HACK: bypass PC-I/F test failure
	m_eprom[0x1e4c0 >> 2] = 0xf7206000;
}

void luna_88k_state::machine_start()
{
}

void luna_88k_state::machine_reset()
{
	m_boot.select(0);

	// TODO: disabled until firmware is dumped
	m_iop->suspend(SUSPEND_REASON_RESET, false);

	m_plane_active = 0;
	for (u8 &f : m_plane_func)
		f = 5;
	for (u32 &m : m_plane_mask)
		m = 0xffff'ffffU;

	for (u8 &i : m_irq_mask)
		i = 0;

	irq_check();
}

void luna_88k_state::cpu_map(address_map &map)
{
	map(0x0000'0000, 0x03ff'ffff).view(m_boot);
	m_boot[0](0x0000'0000, 0x0003'ffff).rom().region("eprom", 0);
	m_boot[1](0x0000'0000, 0x00ff'ffff).ram();

	map(0x0400'0000, 0x3fff'ffff).lrw32(
		[this]() { m_cmmu[1]->bus_error_w(1); return 0; }, "bus_error",
		[this](u32 data) { m_cmmu[1]->bus_error_w(1); }, "bus_error");

	map(0x4100'0000, 0x4103'ffff).rom().region("eprom", 0);
	map(0x4100'0000, 0x4103'ffff).lw32([this](offs_t offset, u32 data) { m_boot.select(1); }, "boot");

	map(0x4300'0000, 0x4300'03ff).rom().region("fuserom", 0);
	map(0x4500'0000, 0x4500'0001).rw(m_rtc, FUNC(ds1397_device::read), FUNC(ds1397_device::write));
	map(0x4700'0000, 0x4700'003f).rw(m_rtc, FUNC(ds1397_device::xram_r), FUNC(ds1397_device::xram_w));
	map(0x4900'0000, 0x4900'000f).rw(m_pio[0], FUNC(i8255_device::read), FUNC(i8255_device::write)).umask32(0xff000000);
	map(0x4d00'0000, 0x4d00'000f).rw(m_pio[1], FUNC(i8255_device::read), FUNC(i8255_device::write)).umask32(0xff000000);
	map(0x5100'0000, 0x5100'000f).rw(m_sio, FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask32(0xff000000);
	// 0x6100'0000 tas register
	map(0x6300'0000, 0x6300'0003).nopr(); // power switch?
	map(0x6300'0000, 0x6300'000f).lw32([this](offs_t offset, u32 data) { irq(offset, 6, 0); }, "sysclk_clr");
	map(0x6500'0000, 0x6500'000f).rw(FUNC(luna_88k_state::irq_ctl_r), FUNC(luna_88k_state::irq_ctl_w));
	map(0x6900'0000, 0x6900'000f).lrw32(
		[this](offs_t offset) { irq(offset, 1, 0); return 0xffffffff; }, "softint_clr",
		[this](offs_t offset, u32 data) { irq(offset, 1, 1); }, "softint_set");
	map(0x6b00'0000, 0x6b00'000f).lr32([this](offs_t offset) { irq(offset, 1, 0); return 0xffffffff; }, "softint_clr");
	// 0x6d00'0000 reset cpu 0-3, all

	// 3 port ram
	map(0x7100'0000, 0x7100'ffff).lrw8(
		[this](offs_t offset) { return m_iop_ram[offset]; }, "iop_ram_r",
		[this](offs_t offset, u8 data) { m_iop_ram[offset] = data; }, "iop_ram_w");
	map(0x7101'0000, 0x7101'ffff).ram().share("net");

	// 0x8100'0000 ext board A
	// 0x8300'0000 ext board B
	// 0x9000'0000 pc-98 ext board
	// 0x9100'0000 pc-9801 irq 4
	// 0xa100'0000 mask rom 0x400000

	//map(0xb100'0000, 0xb100'ffff); // rfcnt (pad,vert_loc,pad,horiz_loc)
	map(0xb104'0000, 0xb104'ffff).lw8([this](u8 data) { LOG("plane_active 0x%02x\n", data); m_plane_active = data; }, "plane_active");
	map(0xb108'0000, 0xb10b'ffff).w(FUNC(luna_88k_state::plane_common_w));
	map(0xb10c'0000, 0xb10f'ffff).rw(FUNC(luna_88k_state::plane_r<0>), FUNC(luna_88k_state::plane_w<0>));
	map(0xb110'0000, 0xb113'ffff).rw(FUNC(luna_88k_state::plane_r<1>), FUNC(luna_88k_state::plane_w<1>));
	map(0xb114'0000, 0xb117'ffff).rw(FUNC(luna_88k_state::plane_r<2>), FUNC(luna_88k_state::plane_w<2>));
	map(0xb118'0000, 0xb11b'ffff).rw(FUNC(luna_88k_state::plane_r<3>), FUNC(luna_88k_state::plane_w<3>));
	map(0xb11c'0000, 0xb11f'ffff).rw(FUNC(luna_88k_state::plane_r<4>), FUNC(luna_88k_state::plane_w<4>));
	map(0xb120'0000, 0xb123'ffff).rw(FUNC(luna_88k_state::plane_r<5>), FUNC(luna_88k_state::plane_w<5>));
	map(0xb124'0000, 0xb127'ffff).rw(FUNC(luna_88k_state::plane_r<6>), FUNC(luna_88k_state::plane_w<6>));
	map(0xb128'0000, 0xb12b'ffff).rw(FUNC(luna_88k_state::plane_r<7>), FUNC(luna_88k_state::plane_w<7>));
	map(0xb12c'0000, 0xb12c'ffff).w(FUNC(luna_88k_state::logic_common_w));

	map(0xb130'0000, 0xb130'ffff).w(FUNC(luna_88k_state::logic_w<0>));
	map(0xb134'0000, 0xb134'ffff).w(FUNC(luna_88k_state::logic_w<1>));
	map(0xb138'0000, 0xb138'ffff).w(FUNC(luna_88k_state::logic_w<2>));
	map(0xb13c'0000, 0xb13c'ffff).w(FUNC(luna_88k_state::logic_w<3>));
	map(0xb140'0000, 0xb140'ffff).w(FUNC(luna_88k_state::logic_w<4>));
	map(0xb144'0000, 0xb144'ffff).w(FUNC(luna_88k_state::logic_w<5>));
	map(0xb148'0000, 0xb148'ffff).w(FUNC(luna_88k_state::logic_w<6>));
	map(0xb14c'0000, 0xb14c'ffff).w(FUNC(luna_88k_state::logic_w<7>));

	map(0xc100'0000, 0xc100'000f).m(m_ramdac, FUNC(bt458_device::map)).umask32(0xff000000).mirror(0x0010'0000);

	// 0xd000'0000 board check register?
	// 0xd100'0000 crtc-ii
	// 0xd180'0000 bitmap board identify rom

	map(0xe100'0000, 0xe100'003f).rw(m_spc[0], FUNC(mb89352_device::mb89352_r), FUNC(mb89352_device::mb89352_w)).umask32(0xff000000);
	map(0xe100'0040, 0xe100'007f).rw(m_spc[1], FUNC(mb89352_device::mb89352_r), FUNC(mb89352_device::mb89352_w)).umask32(0xff000000);
	map(0xf100'0000, 0xf100'0007).rw(m_net[0], FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w)).umask32(0xffff0000);
	map(0xf100'0010, 0xf100'0017).rw(m_net[1], FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w)).umask32(0xffff0000);
}

void luna_88k_state::iop_map_mem(address_map &map)
{
	map(0x0'0000, 0x0'ffff).ram().share("iop_ram");
}

void luna_88k_state::iop_map_pio(address_map &map)
{
	map(0x0049, 0x0049).nopr();
}

void keyboard_devices(device_slot_interface &device)
{
	device.option_add("keyboard", LUNA_KEYBOARD);
}

void luna_88k_state::luna88k2(machine_config &config)
{
	MC88100(config, m_cpu, 33.333_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &luna_88k_state::cpu_map);

	MC88200(config, m_cmmu[0], 33.333_MHz_XTAL, 0x07); // cpu0 cmmu i0
	m_cmmu[0]->set_mbus(m_cpu, AS_PROGRAM);
	m_cpu->set_cmmu_i(m_cmmu[0]);
	MC88200(config, m_cmmu[1], 33.333_MHz_XTAL, 0x06); // cpu0 cmmu d0
	m_cmmu[1]->set_mbus(m_cpu, AS_PROGRAM);
	m_cpu->set_cmmu_d(m_cmmu[1]);

	// 6 SIMMs for RAM arranged as three groups of 2?
	RAM(config, m_ram);
	m_ram->set_default_size("16M");

	clock_device &sys_clk(CLOCK(config, "sys_clk", 200 / 2));
	sys_clk.signal_handler().set([this](int state) { if (state) irq(0, 6, 1); });

	DS1397(config, m_rtc, 32'768);

	HD647180X(config, m_iop, 12'288'000);
	m_iop->set_addrmap(AS_PROGRAM, &luna_88k_state::iop_map_mem);
	m_iop->set_addrmap(AS_IO, &luna_88k_state::iop_map_pio);

	UPD7201(config, m_sio, 19'660'800); // ?
	m_sio->out_int_callback().set(&luna_88k_state::irq<0, 5>, "irq0,5");

	// RS-232C-A
	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);
	m_sio->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_serial[0]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxa_w));
	m_serial[0]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsa_w));

	// keyboard/mouse
	RS232_PORT(config, m_serial[1], keyboard_devices, "keyboard");
	m_sio->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_serial[1]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxb_w));
	m_serial[1]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsb_w));

	clock_device &sio_clk(CLOCK(config, "sio_clk", 19.660'800_MHz_XTAL / 128));
	sio_clk.signal_handler().append(m_sio, FUNC(upd7201_device::rxca_w));
	sio_clk.signal_handler().append(m_sio, FUNC(upd7201_device::txca_w));
	sio_clk.signal_handler().append(m_sio, FUNC(upd7201_device::rxcb_w));
	sio_clk.signal_handler().append(m_sio, FUNC(upd7201_device::txcb_w));

	I8255A(config, m_pio[0], 8'000'000); // M5M82C55AFP-2
	/*
	 * pio0
	 *   port a: dipsw1 (r/o)
	 *   port b: dipsw2 (r/o)
	 *   port c: host interrupt control (r/w)
	 *     bit  function
	 *      0   xp_int1_req (intr b)
	 *      1   unused (ibf b)
	 *      2   xp_int1_ena (inte b)
	 *      3   xp_int5_req (intr a)
	 *      4   xp_int5_ena (inte a)
	 *      5   unused (ibf a)
	 *      6   parity (pc6 output to enable parity error)
	 *      7   xp_reset (pc7 output to reset hd647180 xp)
	 */
	m_pio[0]->in_pa_callback().set([this]() { return m_sw[0]->read(); });
	m_pio[0]->in_pb_callback().set([this]() { return m_sw[1]->read(); });
	m_pio[0]->out_pc_callback().set(
		[this](u8 data)
		{
			LOG("iop reset %d\n", BIT(data, 7));
#if 0
			if (BIT(data, 7))
				m_iop->suspend(SUSPEND_REASON_RESET, false);
			else
				m_iop->resume(SUSPEND_REASON_RESET);
#endif
		});
	m_pio[0]->out_pc_callback().append(m_pio[0], FUNC(i8255_device::pc2_w)).bit(1);
	m_pio[0]->out_pc_callback().append(m_pio[0], FUNC(i8255_device::pc4_w)).bit(5);

	I8255A(config, m_pio[1], 8'000'000); // M5M82C55AFP-2
	/*
	 * pio1
	 *   port a: lcd data (r/w)
	 *   port b: interrupt i/o processor
	 *   port c: interrupt status, powerdown, lcd control
	 *     bit  function
	 *      0
	 *      1   xp intreq input?
	 *      2
	 *      3
	 *      4   power off?
	 *      5   lcd rw
	 *      6   lcd rs
	 *      7   lcd e
	 */
	m_pio[1]->in_pa_callback().set(m_lcdc, FUNC(ks0066_f00_device::db_r));
	m_pio[1]->out_pa_callback().set(m_lcdc, FUNC(ks0066_f00_device::db_w));
	m_pio[1]->out_pc_callback().append(m_lcdc, FUNC(ks0066_f00_device::rw_w)).bit(5);
	m_pio[1]->out_pc_callback().append(m_lcdc, FUNC(ks0066_f00_device::rs_w)).bit(6);
	m_pio[1]->out_pc_callback().append(m_lcdc, FUNC(ks0066_f00_device::e_w)).bit(7);

	scsi_port_device &scsi0(SCSI_PORT(config, "scsi0"));
	scsi0.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_6));

	SCSI_PORT(config, "scsi1");

	input_merger_any_high_device &spc_irq(INPUT_MERGER_ANY_HIGH(config, "spc_irq"));
	spc_irq.output_handler().set(&luna_88k_state::irq<0, 3>, "irq0,3");

	MB89352A(config, m_spc[0], 0);
	m_spc[0]->irq_cb().set(spc_irq, FUNC(input_merger_any_high_device::in_w<0>));
	m_spc[0]->set_scsi_port("scsi0");

	MB89352A(config, m_spc[1], 0);
	m_spc[0]->irq_cb().set(spc_irq, FUNC(input_merger_any_high_device::in_w<1>));
	m_spc[1]->set_scsi_port("scsi1");

	input_merger_any_high_device &net_irq(INPUT_MERGER_ANY_HIGH(config, "net_irq"));
	net_irq.output_handler().set(&luna_88k_state::irq<0, 4>, "irq0,4");

	AM7990(config, m_net[0], 40_MHz_XTAL / 4);
	m_net[0]->intr_out().set(net_irq, FUNC(input_merger_any_high_device::in_w<0>));

	AM7990(config, m_net[1], 40_MHz_XTAL / 4);
	m_net[1]->intr_out().set(net_irq, FUNC(input_merger_any_high_device::in_w<1>));

	// TODO: crt timing control by HD6445CP4
	screen_device &crt(SCREEN(config, "crt", SCREEN_TYPE_RASTER));
	crt.set_raw(108'992'000, 2048, 0, 2048, 1024, 0, 1024);
	crt.set_screen_update(FUNC(luna_88k_state::screen_update));

	BT458(config, m_ramdac, 108'992'000);

	KS0066_F00(config, m_lcdc, 250'000);
	m_lcdc->set_function_set_at_any_time(true);
	m_lcdc->set_lcd_size(2, 16);

	palette_device &palette(PALETTE(config, "palette", palette_device::MONOCHROME));

	screen_device &lcd(SCREEN(config, "lcd", SCREEN_TYPE_LCD));
	lcd.set_raw(192'000, 40 * 6, 0, 16 * 6, 2 * 8, 0, 2 * 8);
	lcd.set_screen_update(m_lcdc, FUNC(ks0066_f00_device::screen_update));
	lcd.set_palette(palette);
}

u32 luna_88k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
	{
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 32)
		{
			unsigned const offset = (y * 64) + (x >> 5);

			u32 const plane0 = m_vram[0 * 0x1'0000 + offset];
			u32 const plane1 = m_vram[1 * 0x1'0000 + offset];
			u32 const plane2 = m_vram[2 * 0x1'0000 + offset];
			u32 const plane3 = m_vram[3 * 0x1'0000 + offset];
			u32 const plane4 = m_vram[4 * 0x1'0000 + offset];
			u32 const plane5 = m_vram[5 * 0x1'0000 + offset];
			u32 const plane6 = m_vram[6 * 0x1'0000 + offset];
			u32 const plane7 = m_vram[7 * 0x1'0000 + offset];

			for (unsigned p = 0; p < 32; p++)
			{
				u8 const index
					= BIT(plane0, 31 - p) << 0
					| BIT(plane1, 31 - p) << 1
					| BIT(plane2, 31 - p) << 2
					| BIT(plane3, 31 - p) << 3
					| BIT(plane4, 31 - p) << 4
					| BIT(plane5, 31 - p) << 5
					| BIT(plane6, 31 - p) << 6
					| BIT(plane7, 31 - p) << 7;

				bitmap.pix(y, x + p) = m_ramdac->pen_color(index);
			}
		}
	}

	return 0;
}

void luna_88k_state::plane_common_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (BIT(m_plane_active, 0)) plane_w<0>(offset, data, mem_mask);
	if (BIT(m_plane_active, 1)) plane_w<1>(offset, data, mem_mask);
	if (BIT(m_plane_active, 2)) plane_w<2>(offset, data, mem_mask);
	if (BIT(m_plane_active, 3)) plane_w<3>(offset, data, mem_mask);
	if (BIT(m_plane_active, 4)) plane_w<4>(offset, data, mem_mask);
	if (BIT(m_plane_active, 5)) plane_w<5>(offset, data, mem_mask);
	if (BIT(m_plane_active, 6)) plane_w<6>(offset, data, mem_mask);
	if (BIT(m_plane_active, 7)) plane_w<7>(offset, data, mem_mask);
}

void luna_88k_state::logic_common_w(offs_t offset, u32 data)
{
	LOG("logic_common func 0x%x mask 0x%08x\n", offset & 0xf, data);

	if (BIT(m_plane_active, 0)) logic_w<0>(offset, data);
	if (BIT(m_plane_active, 1)) logic_w<1>(offset, data);
	if (BIT(m_plane_active, 2)) logic_w<2>(offset, data);
	if (BIT(m_plane_active, 3)) logic_w<3>(offset, data);
	if (BIT(m_plane_active, 4)) logic_w<4>(offset, data);
	if (BIT(m_plane_active, 5)) logic_w<5>(offset, data);
	if (BIT(m_plane_active, 6)) logic_w<6>(offset, data);
	if (BIT(m_plane_active, 7)) logic_w<7>(offset, data);
}

template <unsigned Plane> u32 luna_88k_state::plane_r(offs_t offset, u32 data)
{
	return m_vram[Plane * 0x1'0000 + offset];
}

template <unsigned Plane> void luna_88k_state::plane_w(offs_t offset, u32 data, u32 mem_mask)
{
	u32 const memory = m_vram[Plane * 0x1'0000 + offset];

	// logic operations and mask per HM53462 datasheet
	switch (m_plane_func[Plane])
	{
	case 0x0: data = 0; break;
	case 0x1: data &= memory; break;
	case 0x2: data = ~data & memory; break;
	case 0x3: break; // X4 -> X1
	case 0x4: data &= ~memory; break;
	case 0x5: break;
	case 0x6: data ^= memory; break;
	case 0x7: data |= memory; break;
	case 0x8: data = ~data & ~memory; break;
	case 0x9: data = (data & memory) | (~data & ~memory); break;
	case 0xa: data = ~data; break;
	case 0xb: data = ~data | memory; break;
	case 0xc: data = ~memory; break;
	case 0xd: data |= ~memory; break;
	case 0xe: data = ~data | ~memory; break;
	case 0xf: data = 0xffffffffU; break;
	}

	mem_mask &= m_plane_mask[Plane];

	COMBINE_DATA(&m_vram[Plane * 0x1'0000 + offset]);
}

template <unsigned Plane> void luna_88k_state::logic_w(offs_t offset, u32 data)
{
	if (VERBOSE & LOG_GENERAL)
	{
		static char const *const func[] =
		{
			"0", "AND1", "AND2", "X4->X1", "AND3", "THROUGH", "EOR", "OR1",
			"NOR", "ENOR", "INV1", "OR2", "INV2", "OR3", "NAND", "1"
		};

		LOG("logic plane %u %s func 0x%1x mask 0x%08x\n", Plane, func[offset & 0xf], offset & 0xf, data);
	}

	m_plane_func[Plane] = offset & 0xf;
	m_plane_mask[Plane] = data;
}

u32 luna_88k_state::irq_ctl_r(offs_t offset)
{
	u32 data = u32(m_irq_mask[offset]) << 18;

	u8 const active = m_irq_active[offset] & (0x80 | m_irq_mask[offset] << 1);
	if (active)
	{
		unsigned const level = 31 - count_leading_zeros_32(active);

		data |= (level << 29);
	}

	return data;
}

void luna_88k_state::irq_ctl_w(offs_t offset, u32 data)
{
	m_irq_mask[offset] = BIT(data, 26, 6);

	irq_check();
}

void luna_88k_state::irq(unsigned cpu, unsigned interrupt, int state)
{
	if (state)
		m_irq_active[cpu] |= (1U << interrupt);
	else
		m_irq_active[cpu] &= ~(1U << interrupt);

	irq_check();
}

void luna_88k_state::irq_check()
{
	bool irq_state = m_irq_active[0] & (0x80 | m_irq_mask[0] << 1);

	if (irq_state != m_irq_state)
	{
		m_irq_state = irq_state;
		m_cpu->set_input_line(INPUT_LINE_IRQ0, m_irq_state);
	}
}

static INPUT_PORTS_START(luna88k)
	PORT_START("SW1")
	PORT_DIPNAME(0x80, 0x00, "Start") PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(0x00, "Autoboot")
	PORT_DIPSETTING(0x80, "Monitor")
	PORT_DIPNAME(0x40, 0x40, "Console") PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(0x00, "Serial")
	PORT_DIPSETTING(0x40, "Graphics")
	PORT_DIPNAME(0x20, 0x00, "SW1#3") PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x00, "SW1#4") PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x00, "SW1#5") PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "SW1#6") PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "SW1#7") PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x01, 0x01, "Mode") PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(0x00, "Diagnostic")
	PORT_DIPSETTING(0x01, "Normal")

	// user-defined switches
	PORT_START("SW2")
	PORT_DIPNAME(0x80, 0x00, "SW2#1") PORT_DIPLOCATION("SW2:!1")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x80, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x00, "SW2#2") PORT_DIPLOCATION("SW2:!2")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x40, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x00, "SW2#3") PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x00, "SW2#4") PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x00, "SW2#5") PORT_DIPLOCATION("SW2:!5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "SW2#6") PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x04, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x00, "SW2#7") PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x02, DEF_STR(On))
	PORT_DIPNAME(0x01, 0x00, "SW2#8") PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))

	PORT_START("ABORT")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Abort") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, luna_88k_state, abort, 0)
INPUT_PORTS_END

ROM_START(luna88k2)
	ROM_REGION32_BE(0x40000, "eprom", 0)
	ROM_LOAD32_WORD_SWAP("7187__high__1.37.bin", 0x00000, 0x20000, CRC(a7515231) SHA1(86b3e42a8df6fa33cf68f372f4053b240c8cc4e2)) // HN27C1024HCC-85
	ROM_LOAD32_WORD_SWAP("7187__low__1.37.bin",  0x00002, 0x20000, CRC(8e65ea4a) SHA1(288300c71c0e92f114cb84fa293a4839d2e181a6)) // HN27C1024HCC-85

	// FIXME: this holds an actual machine serial number
	ROM_REGION32_BE(0x400, "fuserom", ROMREGION_ERASEFF)
	ROM_LOAD32_BYTE("fuserom.bin", 0x000, 0x100, CRC(780e4617) SHA1(f939cad237b3b4317cf6093e9b56661cdf60455e))

	ROM_REGION(0x4000, "iop", 0)
	ROM_LOAD("hd647180x.ic13", 0x0000, 0x4000, NO_DUMP) // HD647180X0FS6
ROM_END

} // anonymous namespace

/*   YEAR   NAME      PARENT  COMPAT  MACHINE   INPUT    CLASS           INIT  COMPANY  FULLNAME       FLAGS */
COMP(1992?, luna88k2, 0,      0,      luna88k2, luna88k, luna_88k_state, init, "Omron", u8"Luna 88K²", MACHINE_IS_SKELETON)
