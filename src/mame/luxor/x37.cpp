// license:BSD-3-Clause
// copyright-holders:Curt Coder

/*

    Luxor X37 prototype

    (Luxor DS90-10 + ABC 1600 video)

*/

/*

	TODO

	- trap 26 on boot (IRQ2 autovector)
	- SASI
	- bus errors

*/

#include "emu.h"
#include "bus/abckb/abckb.h"
#include "bus/nscsi/devices.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68010.h"
#include "formats/abc1600_dsk.h"
#include "imagedev/floppy.h"
#include "machine/e0516.h"
#include "machine/hd63450.h"
#include "machine/input_merger.h"
#include "machine/nmc9306.h"
#include "machine/ns32081.h"
#include "machine/watchdog.h"
#include "machine/wd_fdc.h"
#include "machine/z80scc.h"
#include "machine/z8536.h"
#include "abc1600_v.h"
#include "softlist_dev.h"
#include "x37_sasi.h"


namespace {

//#define VERBOSE 0
#include "logmacro.h"

#define MC68010_TAG  "14m"
#define NS32081_TAG  "06o"
#define MC68450_TAG  "11m"
#define Z8536A_TAG   "06l"
#define NMC9306_TAG  "05k"
#define E050_16_TAG  "03j"
#define Z8530A_0_TAG "16m"
#define Z8530A_1_TAG "16h"
#define Z8530A_2_TAG "16o"
#define FD1797_TAG   "15g"

class x37_state : public driver_device
{
public:
	x37_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cpu(*this, MC68010_TAG),
		m_fpu(*this, NS32081_TAG),
		m_dmac(*this, MC68450_TAG),
		m_cio(*this, Z8536A_TAG),
		m_nvram(*this, NMC9306_TAG),
		m_rtc(*this, E050_16_TAG),
		m_scc(*this, {Z8530A_0_TAG, Z8530A_1_TAG, Z8530A_2_TAG}),
		m_fdc(*this, FD1797_TAG),
		m_floppy(*this, FD1797_TAG":%u", 0U),
		m_sasi(*this, "sasi"),
		m_watchdog(*this, "watchdog"),
		m_ram(*this, "ram", 0x400000, ENDIANNESS_LITTLE),
		m_segment_ram(*this, "segment_ram", 0x1000, ENDIANNESS_LITTLE),
		m_page_ram(*this, "page_ram", 0x1000*2, ENDIANNESS_LITTLE),
		m_boot_rom(*this, MC68010_TAG)
	{ }

	void x37(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68000_base_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<hd63450_device> m_dmac;
	required_device<z8536_device> m_cio;
	required_device<nmc9306_device> m_nvram;
	required_device<e0516_device> m_rtc;
	required_device_array<scc8530_device, 3> m_scc;
	required_device<fd1797_device> m_fdc;
	required_device_array<floppy_connector, 3> m_floppy;
	required_device<luxor_x37_sasi_device> m_sasi;
	required_device<watchdog_timer_device> m_watchdog;
	memory_share_creator<u8> m_ram;
	memory_share_creator<u8> m_segment_ram;
	memory_share_creator<u16> m_page_ram;
	required_memory_region m_boot_rom;

	static void floppy_formats(format_registration &fr);

	void program_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;

	int get_task(offs_t offset);
	offs_t get_ma(offs_t offset, bool &at0, bool &at1);
	uint16_t mapper_r(offs_t offset);
	void mapper_w(offs_t offset, uint16_t data);
	uint16_t ram_r(offs_t offset, uint16_t mem_mask);
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint16_t edc_status_r(offs_t offset);

	uint8_t cio_pa_r();
	void cio_pb_w(uint8_t data);
	uint8_t cio_pc_r();
	void cio_pc_w(uint8_t data);
	void mint_w(int state) { m_mint = state; };
	void sasi_int_w(int state) { m_sasi_int = state; }

	void xdck_w(offs_t offset, uint16_t data);

	u8 m_cb = 0xff;
	bool m_mint = 1;
	bool m_sasi_int = 1;
};

void x37_state::program_map(address_map &map)
{
	// map(0x000000, 0x3fffff) SYSTEM RAM
	// map(0x400000, 0x7fffff) 16-BIT EXPANSION (I/O, MEMORY, DMA ETC)
	// map(0x800000, 0xbfffff) SYSTEM CONTROL (MAPPER,DMA,CIO,EDC)
	// map(0xc00000, 0xffffff) 8-BIT I/O DEVICES (PROT. BY MAPPER)
	// map(0xc00000, 0xc0ffff) SERIAL COMM. CHANNELS
	// map(0xc10000, 0xc1ffff) MASS MEMORY GROUP
	// map(0xc20000, 0xc2ffff) 4680 BUS

	map(0x000000, 0x3fffff).rw(FUNC(x37_state::ram_r), FUNC(x37_state::ram_w));
	map(0x400000, 0x47ffff).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::vram_map)).umask16(0xffff);
	map(0x480100, 0x480101).mirror(0xfe).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::crtc_map)).umask16(0xffff);
	map(0x480800, 0x480807).mirror(0xf8).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::iowr0_map)).umask16(0xffff);
	map(0x480900, 0x480907).mirror(0xf8).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::iowr1_map)).umask16(0xffff);
	map(0x480a00, 0x480a07).mirror(0xf8).m(ABC1600_MOVER_TAG, FUNC(abc1600_mover_device::iowr2_map)).umask16(0xffff);
	for (offs_t base = 0x800000; base < 0xc00000; base += 0x200)
	{
		offs_t const mapper_base = (base - 0x800000) >> 1;
		map(base, base + 0xff).lrw16(
			NAME(([this, mapper_base](offs_t offset) -> u16 { return mapper_r(mapper_base + offset); })),
			NAME(([this, mapper_base](offs_t offset, u16 data) { mapper_w(mapper_base + offset, data); })));
	}
	map(0x800100, 0x80017f).rw(m_cio, FUNC(z8536_device::read), FUNC(z8536_device::write)).umask16(0x00ff);
	map(0x810100, 0x810101).r(FUNC(x37_state::edc_status_r));
	//map(0x820100, 0x82010f).rw(m_fpu, FUNC(ns32081_device::slow_read), FUNC(ns32081_device::slow_write));
	map(0x830100, 0x8301ff).rw(m_dmac, FUNC(hd63450_device::read), FUNC(hd63450_device::write));
	map(0xfc0000, 0xfc0007).rw(m_scc[0], FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask16(0x00ff);
	map(0xfc0010, 0xfc0017).rw(m_scc[1], FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask16(0x00ff);
	map(0xfc0020, 0xfc0027).rw(m_scc[2], FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask16(0x00ff);
	map(0xfd5000, 0xfd5001).rw(m_sasi, FUNC(luxor_x37_sasi_device::tre_r), FUNC(luxor_x37_sasi_device::tre_w));
	map(0xfd5080, 0xfd509f).rw(m_sasi, FUNC(luxor_x37_sasi_device::stat_r), FUNC(luxor_x37_sasi_device::ctrl_w));
	map(0xfdb040, 0xfdb041).rw(m_fdc, FUNC(fd1797_device::status_r), FUNC(fd1797_device::cmd_w)).umask16(0x00ff);
	map(0xfdb042, 0xfdb043).rw(m_fdc, FUNC(fd1797_device::track_r), FUNC(fd1797_device::track_w)).umask16(0x00ff);
	map(0xfdb044, 0xfdb045).rw(m_fdc, FUNC(fd1797_device::sector_r), FUNC(fd1797_device::sector_w)).umask16(0x00ff);
	map(0xfdb046, 0xfdb047).rw(m_fdc, FUNC(fd1797_device::data_r), FUNC(fd1797_device::data_w)).umask16(0x00ff);
	map(0xfdb080, 0xfdb081).w(FUNC(x37_state::xdck_w));

	// tst.w 0xfffffc ??
}

void x37_state::cpu_space_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_cpu, FUNC(m68010_device::autovectors_map));
	map(0xfffff7, 0xfffff7).lr8(NAME([this]() -> u8 { return m_cio->intack_r(); }));
	// IACK4 SCC
}

static INPUT_PORTS_START( x37 )
	// keyboard inputs defined in devices/bus/abckb/abc99.cpp
INPUT_PORTS_END

offs_t x37_state::get_ma(offs_t offset, bool &at0, bool &at1)
{
	offs_t const sega = ((offset & 0x1fc000) >> 10) | get_task(offset);
	u8 const segd = m_segment_ram[sega];
	offs_t const pga = ((offset & 0x3c00) >> 2) | (segd & 0x7f);
	u16 const pgd = m_page_ram[pga];
	at0 = BIT(pgd, 14);
	at1 = BIT(pgd, 15);

	offs_t const logical = offset << 1;
	offs_t ma = ((pgd & 0xfff) << 11) | (logical & 0x7ff);

	// TPT
	int const fc = m_cpu->get_fc();
	if (BIT(fc, 2) && ((m_cb & 0xc0) == 0xc0)) {
		if (!(logical & 0xc00000) || ((logical & 0xc00100) == 0xc00000)) {
			ma = (logical & 0x380000) | (ma & 0x47ffff);
			at1 = BIT(logical, 22);
		}
	}
	return ma;
}

uint16_t x37_state::ram_r(offs_t offset, uint16_t mem_mask)
{
	u16 data = 0;

	if (BIT(m_cb, 7) && (offset < 0x8000)) {
		offs_t boot_offset = (offset << 1) & 0x7fff;
		if (ACCESSING_BITS_0_7)
			data |= m_boot_rom->base()[boot_offset & ~1];
		if (ACCESSING_BITS_8_15)
			data |= m_boot_rom->base()[boot_offset | 1] << 8;
	} else {
		bool at0, at1;
		offs_t const ma = get_ma(offset, at0, at1);

		if (ma < 0x400000) {
			if (ACCESSING_BITS_0_7)
				data |= m_ram[ma & ~1];
			if (ACCESSING_BITS_8_15)
				data |= m_ram[ma | 1] << 8;
		} else {
			data = m_cpu->space(AS_PROGRAM).read_word(ma);
		}
	}

	return data;
}

void x37_state::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	bool at0, at1;
	offs_t const ma = get_ma(offset, at0, at1);

	if (ma < 0x400000) {
		if (ACCESSING_BITS_0_7)
			m_ram[ma & ~1] = data;
		if (ACCESSING_BITS_8_15)
			m_ram[ma | 1] = data >> 8;
	} else {
		m_cpu->space(AS_PROGRAM).write_word(ma, data, mem_mask);
	}
}

int x37_state::get_task(offs_t offset)
{
	int const fc = m_cpu->get_fc();

	if (!BIT(fc, 2) || (BIT(fc, 2) && BIT(offset, 22)))
		return (m_cb & 0x0f) ^ 0xf;
	else
		return 0;
}

uint16_t x37_state::mapper_r(offs_t offset)
{
	offs_t const logical = offset | 0x400000;
	offs_t const sega = ((offset & 0x1fc000) >> 10) | get_task(logical);
	u8 const segd = m_segment_ram[sega];

	if (BIT(offset, 6)) {
		offs_t const pga = ((offset & 0x3c00) >> 2) | (segd & 0x7f);
		return m_page_ram[pga];
	} else {
		return (BIT(segd, 7) << 15) | (segd & 0x7f);
	}
}

void x37_state::mapper_w(offs_t offset, uint16_t data)
{
	offs_t const logical = offset | 0x400000;
	offs_t const sega = ((offset & 0x1fc000) >> 10) | get_task(logical);

	if (BIT(offset, 6)) {
		u8 const segd = m_segment_ram[sega];
		offs_t const pga = ((offset & 0x3c00) >> 2) | (segd & 0x7f);
		m_page_ram[pga] = data;

		LOG("%s: %06x PAGE RAM %03x:%04x (SEG %03x:%04x)\n", machine().describe_context(), offset, pga, data, sega, (BIT(segd, 7) << 15) | (segd & 0x7f));
	} else {
		u8 const segd = (BIT(data, 15) << 7) | (data & 0x7f);
		m_segment_ram[sega] = segd;

		LOG("%s: %06x SEGMENT RAM %03x:%04x\n", machine().describe_context(), offset, sega, segd);
	}
}

uint16_t x37_state::edc_status_r(offs_t offset)
{
	/*

		bit		description

		0		MA16
		1		MA17
		2		MA18
		3		MA19
		4		MA20
		5		MA21
		6		0
		7		0
		8		ECC C0
		9		ECC C1
		10		ECC C2
		11		ECC C3
		12		ECC C4
		13		ECC C5
		14		ECC SEF
		15		ECC DEF

	*/

	if (!machine().side_effects_disabled())
	{
		m_watchdog->watchdog_reset();
	}

	return 0;
}

uint8_t x37_state::cio_pa_r()
{
	/*

		bit		description

		0		*MINT
		1		*XIRQ1
		2		*XIRQ2
		3		*XIRQ3
		4	 	*XIRQ4
		5		*XIRQ5
		6		*XIRQ6
		7		*SASI INT

	*/

	u8 data = 0x7e;

	data |= m_mint;
	data |= m_sasi_int << 7;

	return data;
}

void x37_state::cio_pb_w(uint8_t data)
{
	/*

		bit		description

		0		TASKNR
		1		TASKNR
		2		TASKNR
		3		TASKNR
		4		MAN INPUT, PERMIT OUTPUT
		5		ENABLE IRQ1, DISABLE MAN INPUT
		6
		7		BOOT, GREEN LED

	*/

	m_cb = data;
}

uint8_t x37_state::cio_pc_r()
{
	/*

	    bit     description

	    PC0     1
	    PC1     DATA IN
	    PC2     1
	    PC3     1

	*/

	uint8_t data = 0x0d;

	// data in
	data |= (m_rtc->dio_r() || m_nvram->do_r()) << 1;

	return data;
}

void x37_state::cio_pc_w(uint8_t data)
{
	/*

	    bit     description

	    PC0     CLOCK
	    PC1     DATA OUT
	    PC2     RTC CS
	    PC3     NVRAM CS

	*/

	int clock = BIT(data, 0);
	int data_out = BIT(data, 1);
	int rtc_cs = BIT(data, 2);
	int nvram_cs = BIT(data, 3);

	m_rtc->cs_w(rtc_cs);
	m_rtc->dio_w(data_out);
	m_rtc->clk_w(clock);

	m_nvram->cs_w(nvram_cs);
	m_nvram->di_w(data_out);
	m_nvram->sk_w(clock);
}

void x37_state::xdck_w(offs_t offset, uint16_t data)
{
	/*

		bit		description

		0		FPMR
		1	    FPDD
		2	    FPHLT
		3       MINI
		4       N/C
		5	    N/C
		6       PRE1
		7       PRE2
		8		SEL0
		9       SEL1
		10	    SEL2
		11      MOTOR
		12      LO1 (anded with FPWG)
		13      LO1
		14      N/C
		15      N/C

	*/

	m_fdc->mr_w(BIT(data, 0));
	m_fdc->dden_w(BIT(data, 1));
	m_fdc->hlt_w(BIT(data, 2));

	floppy_image_device *floppy = nullptr;

	for (int n = 0; n < 3; n++)
		if (BIT(data, n + 8))
			floppy = m_floppy[n]->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy) floppy->mon_w(!BIT(data, 11));
}

static void x37_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void x37_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ABC1600_FORMAT);
}

void x37_state::machine_start()
{
	for (auto & s : m_segment_ram)
		s = 0xff;

	for (auto & s : m_page_ram)
		s = 0xffff;

	save_item(NAME(m_cb));
	save_item(NAME(m_mint));
	save_item(NAME(m_sasi_int));
}

void x37_state::machine_reset()
{
	m_cb = 0xff;
}

void x37_state::x37(machine_config &config)
{
	// basic machine hardware
	M68010(config, m_cpu, XTAL(20'000'000)/2);
	m_cpu->set_addrmap(AS_PROGRAM, &x37_state::program_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &x37_state::cpu_space_map);

	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_msec(41943));

	NS32081(config, m_fpu, XTAL(20'000'000)/2);

	HD63450(config, m_dmac, XTAL(20'000'000)/2, m_cpu, AS_PROGRAM);

	Z8536(config, m_cio, XTAL(20'000'000)/5);
	m_cio->irq_wr_cb().set_inputline(m_cpu, M68K_IRQ_3);
	m_cio->pa_rd_cb().set(FUNC(x37_state::cio_pa_r));
	m_cio->pb_wr_cb().set(FUNC(x37_state::cio_pb_w));
	m_cio->pc_rd_cb().set(FUNC(x37_state::cio_pc_r));
	m_cio->pc_wr_cb().set(FUNC(x37_state::cio_pc_w));

	NMC9306(config, m_nvram, 0);

	E0516(config, m_rtc, XTAL(32'768));
	m_rtc->outsel_rd_cb().set_constant(0);

	INPUT_MERGER_ANY_HIGH(config, "irq4").output_handler().set_inputline(m_cpu, M68K_IRQ_4);
	INPUT_MERGER_ANY_HIGH(config, "req3").output_handler().set(m_dmac, FUNC(hd63450_device::drq3_w));

	SCC8530(config, m_scc[0], XTAL(20'000'000)/5);
	m_scc[0]->out_int_callback().set("irq4", FUNC(input_merger_device::in_w<0>));
	m_scc[0]->out_wreqa_callback().set("req3", FUNC(input_merger_device::in_w<0>));
	m_scc[0]->out_wreqb_callback().set("req3", FUNC(input_merger_device::in_w<1>));
	m_scc[0]->out_txdb_callback().set("kb", FUNC(abc_keyboard_port_device::txd_w));
	m_scc[0]->out_txda_callback().set("tty01", FUNC(rs232_port_device::write_txd));
	m_scc[0]->out_dtra_callback().set("tty01", FUNC(rs232_port_device::write_dtr));
	m_scc[0]->out_rtsa_callback().set("tty01", FUNC(rs232_port_device::write_rts));

	abc_keyboard_port_device &kb(ABC_KEYBOARD_PORT(config, "kb", abc_keyboard_devices, "abc99"));
	kb.out_rx_handler().set(m_scc[0], FUNC(z80scc_device::rxb_w));
	kb.out_trxc_handler().set(m_scc[0], FUNC(z80scc_device::rxtxcb_w));
	kb.out_keydown_handler().set(m_scc[0], FUNC(z80scc_device::dcdb_w));

	rs232_port_device &tty01(RS232_PORT(config, "tty01", default_rs232_devices, nullptr));
	tty01.rxd_handler().set(m_scc[0], FUNC(z80scc_device::rxa_w));
	tty01.dcd_handler().set(m_scc[0], FUNC(z80scc_device::dcda_w));
	tty01.cts_handler().set(m_scc[0], FUNC(z80scc_device::ctsa_w));

	SCC8530(config, m_scc[1], XTAL(20'000'000)/5);
	m_scc[1]->out_int_callback().set("irq4", FUNC(input_merger_device::in_w<1>));
	m_scc[1]->out_wreqa_callback().set("req3", FUNC(input_merger_device::in_w<2>));
	m_scc[1]->out_wreqb_callback().set("req3", FUNC(input_merger_device::in_w<3>));
	m_scc[1]->out_txdb_callback().set("tty02", FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtrb_callback().set("tty02", FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsb_callback().set("tty02", FUNC(rs232_port_device::write_rts));
	m_scc[1]->out_txda_callback().set("tty03", FUNC(rs232_port_device::write_txd));
	m_scc[1]->out_dtra_callback().set("tty03", FUNC(rs232_port_device::write_dtr));
	m_scc[1]->out_rtsa_callback().set("tty03", FUNC(rs232_port_device::write_rts));

	rs232_port_device &tty02(RS232_PORT(config, "tty02", default_rs232_devices, nullptr));
	tty02.rxd_handler().set(m_scc[1], FUNC(z80scc_device::rxb_w));
	tty02.dcd_handler().set(m_scc[1], FUNC(z80scc_device::dcdb_w));
	tty02.cts_handler().set(m_scc[1], FUNC(z80scc_device::ctsb_w));

	rs232_port_device &tty03(RS232_PORT(config, "tty03", default_rs232_devices, nullptr));
	tty03.rxd_handler().set(m_scc[1], FUNC(z80scc_device::rxa_w));
	tty03.dcd_handler().set(m_scc[1], FUNC(z80scc_device::dcda_w));
	tty03.cts_handler().set(m_scc[1], FUNC(z80scc_device::ctsa_w));

	SCC8530(config, m_scc[2], XTAL(20'000'000)/5);
	m_scc[2]->out_int_callback().set("irq4", FUNC(input_merger_device::in_w<2>));
	m_scc[2]->out_wreqa_callback().set("req3", FUNC(input_merger_device::in_w<4>));
	m_scc[2]->out_wreqb_callback().set("req3", FUNC(input_merger_device::in_w<5>));
	m_scc[2]->out_txdb_callback().set("tty04", FUNC(rs232_port_device::write_txd));
	m_scc[2]->out_dtrb_callback().set("tty04", FUNC(rs232_port_device::write_dtr));
	m_scc[2]->out_rtsb_callback().set("tty04", FUNC(rs232_port_device::write_rts));
	m_scc[2]->out_txda_callback().set("tty05", FUNC(rs232_port_device::write_txd));
	m_scc[2]->out_dtra_callback().set("tty05", FUNC(rs232_port_device::write_dtr));
	m_scc[2]->out_rtsa_callback().set("tty05", FUNC(rs232_port_device::write_rts));

	rs232_port_device &tty04(RS232_PORT(config, "tty04", default_rs232_devices, nullptr));
	tty04.rxd_handler().set(m_scc[2], FUNC(z80scc_device::rxb_w));
	tty04.dcd_handler().set(m_scc[2], FUNC(z80scc_device::dcdb_w));
	tty04.cts_handler().set(m_scc[2], FUNC(z80scc_device::ctsb_w));

	rs232_port_device &tty05(RS232_PORT(config, "tty05", default_rs232_devices, nullptr));
	tty05.rxd_handler().set(m_scc[2], FUNC(z80scc_device::rxa_w));
	tty05.dcd_handler().set(m_scc[2], FUNC(z80scc_device::dcda_w));
	tty05.cts_handler().set(m_scc[2], FUNC(z80scc_device::ctsa_w));

	FD1797(config, m_fdc, XTAL(16'000'000)/16);
	m_fdc->intrq_wr_callback().set_inputline(m_cpu, M68K_IRQ_2);
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(hd63450_device::drq2_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], x37_floppies, nullptr, x37_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], x37_floppies, nullptr, x37_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], x37_floppies, "525qd", x37_state::floppy_formats).enable_sound(true);

	LUXOR_X37_SASI(config, m_sasi, 0);
	m_sasi->int_callback().set(m_cio, FUNC(z8536_device::pa7_w));
	m_sasi->int_callback().append(FUNC(x37_state::sasi_int_w));
	m_sasi->req0_callback().set(m_dmac, FUNC(hd63450_device::drq0_w));

	// video hardware
	abc1600_mover_device &mover(ABC1600_MOVER(config, ABC1600_MOVER_TAG, XTAL(64'000'000)));
	mover.amm_callback().set(m_cio, FUNC(z8536_device::pa0_w));
	mover.amm_callback().append(FUNC(x37_state::mint_w));

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("x37_flop");
	SOFTWARE_LIST(config, "hdd_list").set_original("x37_hdd");
}

ROM_START( x37 )
	ROM_REGION( 0x8000, MC68010_TAG, 0 )
	ROM_LOAD( "x37.07o", 0x0000, 0x8000, CRC(d505e7e7) SHA1(a3ad839e47b1f71c394e5ce28bce199e5e4810d2) )

	//ROM_REGION( 0x20, NMC9306_TAG, 0 )
	//ROM_LOAD( "nmc9306.05k", 0x00, 0x20, CRC(233e90a6) SHA1(f7e35dc0f2be88a191a9c1ce037e35b91a7cf1c4) )

	ROM_REGION( 0xa28, "plds", 0 )
	//ROM_LOAD( "pat8000", 0x000, 0x104, NO_DUMP ) // Strobe decoder for X35 video adapter
	ROM_LOAD( "pat8003.12l", 0x000, 0x104, CRC(7c7b6dd1) SHA1(ab98fe70d589273b6a0437a818d9ae4bf9319ad5) ) // SCC decoder and clock multiplexor control
	ROM_LOAD( "pat8031.05h", 0x104, 0x104, CRC(2836e65b) SHA1(305feb8dff7d6762f2ab50d25316ad43140456eb) ) // DS60 MAPPER CONTROL
	ROM_LOAD( "pat8032.07h", 0x208, 0x104, CRC(356118d2) SHA1(e8e1dc6accdb8f0de481b91aa844f4b95f967826) ) // DS60 MAIN FUNCTION ENCODER
	ROM_LOAD( "pat8033.06h", 0x30c, 0x104, CRC(5f61f902) SHA1(b151621af0d9e851437ef4e3a02ecb78a6e102dd) ) // DS60 RAM REFRESH AND MAPPER WRITE CONTROL
	ROM_LOAD( "pat8034.12g", 0x410, 0x104, CRC(1105f161) SHA1(1923c0c954d3c812197d40f51bf3f53a158f87db) ) // DS60 CIO, SCC, FDC AND BOOTPROM CONTROLLER
	ROM_LOAD( "pat8035.05g", 0x514, 0x104, CRC(f25be5d9) SHA1(ed51b5cedea34c81b8cbdefd994e13aabd44a036) ) // DS60 RAM DATA PATH CONTROL
	ROM_LOAD( "pat8036.07g", 0x618, 0x104, CRC(350ff68e) SHA1(2d239bf324209adc7677eeb76b22c476ae0e6523) ) // DS60 RAM CONTROL SEQUENCER
	ROM_LOAD( "pat8037.10k", 0x71c, 0x104, CRC(a0e818b3) SHA1(a0d49ba0f09e235b28037539044e133f777fa4c7) ) // DS60 SASI INTERFACE CONTROL
	ROM_LOAD( "pat8038.04n", 0x820, 0x104, CRC(46ff5ce3) SHA1(c4a9025162b623bfcb74ac52f39de25bd53e448b) ) // DS60 PARITY GENERATION/DETECTION CONTROL
	ROM_LOAD( "pat8039.12h", 0x924, 0x104, CRC(d3f6974f) SHA1(98dc1bac1c822fe7af0edd683acfc2e5c51f0451) ) // DS60 NS32081 FLOATING POINT PROCESSOR INTERFACE
ROM_END

} // anonymous namespace


COMP( 1985, x37, 0,      0,      x37, x37, x37_state, empty_init, "Luxor", "X37 (prototype)", MACHINE_NOT_WORKING )
