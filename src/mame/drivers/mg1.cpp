// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Whitechapel Computer Works MG-1 (Milliard Gargantubrain)
 *
 * Sources:
 *   - http://www.cpu-ns32k.net/Whitechapel.html
 *
 * TODO:
 *   - skeleton only
 */

#include "emu.h"

// cpus and memory
#include "cpu/ns32000/ns32000.h"
#include "cpu/m6800/m6801.h"
#include "machine/ram.h"
#include "machine/nvram.h"

// various hardware
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ns32202.h"
#include "machine/am79c90.h"
//#include "machine/am9516.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"
//#include "machine/d7261.h"
#include "machine/mm58174.h"

// buses and connectors
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"
#include "formats/applix_dsk.h"
#include "formats/pc_dsk.h"

// video
#include "screen.h"
#include "video/mc6845.h"

#include "mg1.lh"

#define VERBOSE 0
#include "logmacro.h"

class mg1_state : public driver_device
{
public:
	mg1_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_fpu(*this, "fpu")
		, m_mmu(*this, "mmu")
		, m_icu(*this, "icu")
		, m_ram(*this, "ram")
		, m_sram(*this, "sram")
		, m_iop(*this, "iop")
		, m_iop_ram(*this, "iop_ram")
		, m_iop_sram(*this, "iop_sram")
		, m_iop_ctc(*this, "iop_ctc")
		, m_usart(*this, "usart")
		, m_serial(*this, "serial")
		, m_rtc(*this, "rtc")
		, m_fdc(*this, "fdc")
		, m_fdd(*this, "fdc:0:35dd")
		, m_net(*this, "net")
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen")
		, m_vmram(*this, "vmram")
		, m_led_err(*this, "led_err")
		, m_led_fdd(*this, "led_fdd")
	{
	}

	// machine config
	void mg1(machine_config &config);

	void init_common();

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map);
	void iop_map(address_map &map);

private:
	MC6845_UPDATE_ROW(update_row);

	static void floppy_formats(format_registration &fr);

	// devices
	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_sram;

	required_device<m6800_cpu_device> m_iop;
	required_shared_ptr<u8> m_iop_ram;
	required_device<nvram_device> m_iop_sram;

	required_device<pit8253_device> m_iop_ctc;

	required_device<i8251_device> m_usart;
	required_device<rs232_port_device> m_serial;
	required_device<mm58174_device> m_rtc;
	required_device<wd1770_device> m_fdc;
	required_device<floppy_image_device> m_fdd;
	required_device<am7990_device> m_net;

	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	required_shared_ptr<u16> m_vmram;

	output_finder<> m_led_err;
	output_finder<> m_led_fdd;

	u8 m_sem[6] = { 0xc0, 0x80, 0xc0, 0xc0, 0xc0, 0xc0 };
};


void mg1_state::machine_start()
{
}


void mg1_state::machine_reset()
{
	m_fdc->set_floppy(m_fdd);
}


void mg1_state::init_common()
{
	m_led_err.resolve();
	m_led_fdd.resolve();
}


template <unsigned ST> void mg1_state::cpu_map(address_map &map)
{
	// rom page
	map(0x000000, 0x003fff).mirror(0xef4000).rom().region("prom", 0);
	map(0x008000, 0x008fff).mirror(0xef3000).ram().share("sram");  // 2xTC5516AP 2048x8 SRAM
	map(0x00c000, 0x00cfff).mirror(0xef3000).ram().share("vmram"); // 2xD4016C-3 2048x8 SRAM

	// i/o page
	//map(0x308000, 0x3081ff).mirror(0xcf6000); // wcw reserved
	map(0x308200, 0x3083ff).mirror(0xcf6000).umask16(0x00ff).lw8(
		[this](u8 data)
		{
			if (BIT(data, 2))
			{
				// DRAM-ON
				m_cpu->space(0).unmap_readwrite(0x000000, 0xbfffff);
				m_cpu->space(0).install_ram(0, m_ram->mask(), 0x7fffff ^ m_ram->mask(), m_ram->pointer());
			}
		}, "dma_reg_w");
	//map(0x308400, 0x3085ff).mirror(0xcf6000); // wcw reserved
	map(0x30862e, 0x30863b).mirror(0xcf6000).umask16(0x00ff).lrw8(
		[this](offs_t offset)
		{
			u8 const data = m_sem[offset];
			if (!BIT(data, 7))
				m_sem[offset] |= 0x80;

			return data;
		}, "cpu_sem_r",
		[this](offs_t offset, u8 data)
		{
			m_sem[offset] &= ~0x80;
		}, "cpu_sem_w");
	map(0x308700, 0x3087ff).mirror(0xcf6000).umask16(0x00ff).lrw8(
		[this](offs_t offset)
		{
			return m_iop_ram[offset];
		}, "iop_ram_r",
		[this](offs_t offset, u8 data)
		{
			if (offset == 18 + m_iop_ram[3])
				logerror("iop command %d\n", data);

			m_iop_ram[offset] = data;
		}, "iop_ram_w");
	//map(0x308800, 0x3089ff).mirror(0xcf6000); // ctc
	//map(0x308a00, 0x308bff).mirror(0xcf6000); // raster-op function ctl
	//map(0x308c00, 0x308dff).mirror(0xcf6000); // raster-op
	//map(0x308e00, 0x308fff).mirror(0xcf6000); // general/raster-op dma

	map(0x309000, 0x309003).mirror(0xcf6000).umask16(0x00ff).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x309200, 0x3093ff).mirror(0xcf6000).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	map(0x309400, 0x3095ff).mirror(0xcf6000).umask16(0x00ff).rw(m_rtc, FUNC(mm58174_device::read), FUNC(mm58174_device::write));

	map(0x309600, 0x3097ff).mirror(0xcf6000).lr8(  // hdc
		[](offs_t offset)
		{
			return 0xff;
		}, "hdc_r");

	map(0x309800, 0x309807).mirror(0xcf6000).umask16(0x00ff).rw(m_fdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write));

	map(0x309a00, 0x309bff).mirror(0xcf6000).umask16(0x00ff).lw8(
		[this](u8 data)
		{
			m_fdd->ss_w(!BIT(data, 0));
			m_fdc->dden_w(BIT(data, 1));
			m_led_fdd = !BIT(data, 2);
			m_led_err = BIT(data, 3);
			//BIT(data, 4); // upd-head-select3
		}, "fdc_reg_w");

	//map(0x309c00, 0x309dff).mirror(0xcf6000); // dma interrupt acknowledge
	map(0x309e00, 0x309e3f).mirror(0xcf6000).umask16(0x00ff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>));
}


void mg1_state::iop_map(address_map &map)
{
	map(0x0000, 0x001f).m(m_iop, FUNC(m6801_cpu_device::m6801_io));
	map(0x0017, 0x001c).lrw8(
		[this](offs_t offset)
		{
			u8 const data = m_sem[offset];
			if (!BIT(data, 7))
				m_sem[offset] |= 0xc0;

			return data;
		}, "iop_sem_r",
		[this](offs_t offset, u8 data)
		{
			m_sem[offset] &= ~0x80;
		}, "iop_sem_w");

	map(0x0080, 0x00ff).ram().share("iop_ram");

	// i/o area
	//map(0x2000, 0x201f).mirror(0x1e00).lw8([this](u8 data) { logerror("mouse x counter 0x%02x\n", data); }, "mouse_x"); // mouse x counter
	map(0x2020, 0x2020).mirror(0x1e00).lw8([this](offs_t offset, u8 data) { logerror("host reset %x,0x%02x\n", offset, data); }, "host_reset").select(0x0100);
	//map(0x2040, 0x205f).mirror(0x1e00).lw8([this](u8 data) { logerror("cursor 0x%02x\n", data); }, "cursor"); // cursor pixel offset & cursor number
	map(0x2060, 0x207f).mirror(0x1e00).lw8([this](u8 data) { m_icu->ir_w<10>(0); }, "iopint");
	map(0x2080, 0x2080).mirror(0x1e00).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x2081, 0x2081).mirror(0x1e00).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	//map(0x20a0, 0x20bf).mirror(0x1e00).lw8([this](u8 data) { logerror("mouse buttons 0x%02x\n", data); }, "mouse_b"); // mouse buttons
	//map(0x20c0, 0x20df).mirror(0x1e00).lw8([this](u8 data) { logerror("mouse y counter 0x%02x\n", data); }, "mouse_y"); // mouse y counter
	map(0x20e0, 0x20ff).mirror(0x1e00).rw(m_iop_ctc, FUNC(pit8253_device::read), FUNC(pit8253_device::write));

	map(0x4000, 0x47ff).mirror(0x3800).ram().share("iop_sram"); // D4016C-3 2048x8 SRAM
	map(0x8000, 0xbfff).mirror(0x4000).rom().region("iop_prom", 0);
}


static INPUT_PORTS_START(mg1)
INPUT_PORTS_END


MC6845_UPDATE_ROW(mg1_state::update_row)
{
	// 16 columns x 50 rows of characters, each 64x16 pixels
	// 10 bits look up video mapping ram -> va6-21 (16 bits)
	// 6 bits give address of 64-bit character in page

	for (unsigned column = 0; column < x_count; column++)
	{
		u16 const vma = ((ma & 0x0ff0) << 4) | ((ra & 0x0f) << 4) | column;
		u16 const va = (u32(m_vmram[vma >> 6]) << 6) | (vma & 0x3f);

		for (unsigned byte = 0; byte < 8; byte++)
		{
			u8 const data = m_ram->read((va << 3) | BYTE8_XOR_LE(byte));
			unsigned const x = column * 64 + byte * 8;

			bitmap.pix(y, x + 0) = BIT(data, 0) ? rgb_t::black() : rgb_t::white();
			bitmap.pix(y, x + 1) = BIT(data, 1) ? rgb_t::black() : rgb_t::white();
			bitmap.pix(y, x + 2) = BIT(data, 2) ? rgb_t::black() : rgb_t::white();
			bitmap.pix(y, x + 3) = BIT(data, 3) ? rgb_t::black() : rgb_t::white();
			bitmap.pix(y, x + 4) = BIT(data, 4) ? rgb_t::black() : rgb_t::white();
			bitmap.pix(y, x + 5) = BIT(data, 5) ? rgb_t::black() : rgb_t::white();
			bitmap.pix(y, x + 6) = BIT(data, 6) ? rgb_t::black() : rgb_t::white();
			bitmap.pix(y, x + 7) = BIT(data, 7) ? rgb_t::black() : rgb_t::white();
		}
	}
}


void mg1_state::mg1(machine_config &config)
{
	NS32016(config, m_cpu, 16_MHz_XTAL / 2);
	m_cpu->set_addrmap(0, &mg1_state::cpu_map<0>);
	m_cpu->set_addrmap(6, &mg1_state::cpu_map<6>);
	m_cpu->set_fpu(m_fpu);
	m_cpu->set_mmu(m_mmu);

	NS32081(config, m_fpu, 16_MHz_XTAL / 2);

	NS32082(config, m_mmu, 16_MHz_XTAL / 2);

	NS32202(config, m_icu, 5_MHz_XTAL);
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	/*
	 *  2  busint2
	 *  3  winint
	 *  4  dma2int
	 *  5  busint3

	 *  7  busint4

	 *  9  busint5
	 * 10  iopint

	 * 12  busint6
	 * 13  (not used)
	 * 14  (not used)
	 * 15  (not used)
	 */

	RAM(config, m_ram).set_default_size("2M").set_extra_options("4M,6M,8M").set_default_value(0);

	NVRAM(config, m_sram); // 2xTC5516AP 2048x8 SRAM

	M6801(config, m_iop, 8_MHz_XTAL / 8); // TODO: MC68121 (mode 2)
	m_iop->set_addrmap(0, &mg1_state::iop_map);

	NVRAM(config, m_iop_sram); // 1xD4016C-3 2048x8 SRAM

	PIT8253(config, m_iop_ctc);

	I8251(config, m_usart, m_iop->clock());
	m_icu->out_cout().set(m_usart, FUNC(i8251_device::rx_clock_w));
	m_icu->out_cout().append(m_usart, FUNC(i8251_device::tx_clock_w));
	m_usart->rxrdy_handler().set(m_icu, FUNC(ns32202_device::ir_w<1>));
	m_usart->txrdy_handler().set(m_icu, FUNC(ns32202_device::ir_w<11>));

	RS232_PORT(config, m_serial, default_rs232_devices, nullptr);
	m_serial->rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	m_serial->dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));
	m_serial->dcd_handler().set(m_usart, FUNC(i8251_device::write_cts));
	m_usart->txd_handler().set(m_serial, FUNC(rs232_port_device::write_txd));
	m_usart->dtr_handler().set(m_serial, FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set(m_serial, FUNC(rs232_port_device::write_rts));
	// TODO: tset/rset

	MM58174(config, m_rtc, 32.768_kHz_XTAL);

	HD6845S(config, m_crtc, 60_MHz_XTAL / 64);
	m_crtc->set_show_border_area(false);
	m_crtc->set_hpixels_per_column(64);
	m_crtc->set_update_row_callback(FUNC(mg1_state::update_row));

	// black & white crt, 56Hz refresh, 46.877kHz line, line sync 1.066uS, frame sync 341.32uS
	// crtc sees it as 16 col x 50 rows, with 64x16 character cells
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(60_MHz_XTAL, 20*64, 0, 16*64, 51*16 + 4, 0, 50*16);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	AM7990(config, m_net);
	m_net->intr_out().set(m_icu, FUNC(ns32202_device::ir_w<6>));
	m_net->dma_in().set([this](offs_t offset) { return m_cpu->space(0).read_word(offset); });
	m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { m_cpu->space(0).write_word(offset, data, mem_mask); });

	WD1770(config, m_fdc, 8_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(m_icu, FUNC(ns32202_device::ir_w<8>));
	//m_fdc->drq_wr_callback().set(m_dma, FUNC(dmac_0448_device::drq<1>));

	FLOPPY_CONNECTOR(config, "fdc:0", "35dd", FLOPPY_35_DD, true, floppy_formats).enable_sound(true);

	//SOFTWARE_LIST(config, "flop_list").set_original("mg1_flop");
	//SOFTWARE_LIST(config, "hdd_list").set_original("mg1_hdd");

	config.set_default_layout(layout_mg1);
}


void mg1_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_APPLIX_FORMAT);
}


ROM_START(mg1)
	ROM_REGION16_LE(0x4000, "prom", 0)
	ROM_SYSTEM_BIOS(0, "260", "v2.60")
	ROMX_LOAD("sys_260_even.u291", 0x0000, 0x2000, CRC(24b45b73) SHA1(04d86587b104aa122ac395aa39eb92a1f4d68def), ROM_BIOS(0) | ROM_SKIP(1) )
	ROMX_LOAD("sys_260_odd.u292",  0x0001, 0x2000, CRC(a46ebbf8) SHA1(a2ab9fa3a9576d63d8d49730bfcd58a0f508b30f), ROM_BIOS(0) | ROM_SKIP(1) )
	ROM_SYSTEM_BIOS(1, "251", "v2.51")
	ROMX_LOAD("sys_251.bin", 0x0000, 0x4000, CRC(aa6c7ccd) SHA1(ef52f0a014c209414f669b7a4d200e9bb9a09fea), ROM_BIOS(1))

	ROM_REGION(0x4000, "iop_prom", 0)
	ROM_LOAD("iop_30.u285", 0x0000, 0x2000, CRC(733cd089) SHA1(31ffdd85b4ae2ac35dcde292a0d42860baaba88d))
	ROM_RELOAD(             0x2000, 0x2000)
ROM_END


/*   YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT         COMPANY  FULLNAME    FLAGS */
COMP(1984, mg1,  0,      0,      mg1,     mg1,   mg1_state, init_common, "Whitechapel Computer Works",  "MG-1", MACHINE_IS_SKELETON)
