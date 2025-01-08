// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Whitechapel Computer Works MG-1 (Milliard Gargantubrain)
 *
 * Sources:
 *   - http://www.cpu-ns32k.net/Whitechapel.html
 *
 * TODO:
 *   - mouse
 */
/*
 * WIP
 *  - boots 42nix from hard disk image, but can't operate GUI without mouse
 */

#include "emu.h"

#include "kbd.h"

#include "bus/rs232/rs232.h"
#include "cpu/ns32000/ns32000.h"
#include "cpu/m6800/m6801.h"
#include "machine/am79c90.h"
#include "machine/am9516.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/input_merger.h"
#include "machine/mm58174.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"
#include "machine/ns32202.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/upd7261.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"

#include "imagedev/floppy.h"
#include "formats/applix_dsk.h"

#include "screen.h"
#include "speaker.h"

#include "mg1.lh"

#define VERBOSE 0
#include "logmacro.h"

namespace {

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
		, m_iop_sram(*this, "iop_sram")
		, m_iop_ctc(*this, "iop_ctc")
		, m_dma(*this, "dma%u", 0U)
		, m_usart(*this, "usart")
		, m_serial(*this, "serial")
		, m_rtc(*this, "rtc")
		, m_fdc(*this, "fdc")
		, m_fdd(*this, "fdc:0:35dd")
		, m_net(*this, "net")
		, m_hdc(*this, "hdc")
		, m_ctc(*this, "ctc")
		, m_crtc(*this, "crtc")
		, m_screen(*this, "screen")
		, m_vmram(*this, "vmram")
		, m_kbd(*this, "kbd")
		, m_buzzer(*this, "buzzer")
		, m_buzzen(*this, "buzzer_enable")
		, m_mouse_buttons(*this, "mouse_buttons")
		, m_led_err(*this, "led_err")
		, m_led_fdd(*this, "led_fdd")
	{
	}

	// machine config
	void mg1(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	template <unsigned ST> void cpu_map(address_map &map) ATTR_COLD;
	void iop_map(address_map &map) ATTR_COLD;
	void dma_map(address_map &map) ATTR_COLD;

private:
	MC6845_UPDATE_ROW(update_row);

	// devices
	required_device<ns32016_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<ns32202_device> m_icu;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_sram;

	required_device<mc68121_device> m_iop;
	required_device<nvram_device> m_iop_sram;

	required_device<pit8253_device> m_iop_ctc;

	required_device_array<am9516_device, 2> m_dma;
	required_device<i8251_device> m_usart;
	required_device<rs232_port_device> m_serial;
	required_device<mm58174_device> m_rtc;
	required_device<wd1770_device> m_fdc;
	required_device<floppy_image_device> m_fdd;
	required_device<am7990_device> m_net;

	required_device<upd7261_device> m_hdc;
	required_device<pit8253_device> m_ctc;

	required_device<mc6845_device> m_crtc;
	required_device<screen_device> m_screen;
	required_shared_ptr<u16> m_vmram;

	required_device<mg1_kbd_device> m_kbd;

	required_device<speaker_sound_device> m_buzzer;
	required_device<input_merger_all_high_device> m_buzzen;

	required_ioport m_mouse_buttons;

	output_finder<> m_led_err;
	output_finder<> m_led_fdd;

	u8 m_sem[6] = { 0xc0, 0x80, 0xc0, 0xc0, 0xc0, 0xc0 };
	u8 m_iop_p2;
};

void mg1_state::machine_start()
{
	m_led_err.resolve();
	m_led_fdd.resolve();
}

void mg1_state::machine_reset()
{
	m_fdc->set_floppy(m_fdd);
	m_iop_p2 = 0;
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
				m_cpu->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());
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
			return m_iop->dpram_r(offset);
		}, "iop_ram_r",
		[this](offs_t offset, u8 data)
		{
			if (offset == 18 + m_iop->dpram_r(3))
				logerror("iop command %d\n", data);

			m_iop->dpram_w(offset, data);
		}, "iop_ram_w");
	//map(0x308800, 0x3089ff).mirror(0xcf6000); // ctc
	//map(0x308a00, 0x308bff).mirror(0xcf6000); // raster-op function ctl
	//map(0x308c00, 0x308dff).mirror(0xcf6000); // raster-op

	map(0x308e00, 0x308e01).mirror(0xcf6000).rw(m_dma[0], FUNC(am9516_device::data_r), FUNC(am9516_device::data_w));
	map(0x308e02, 0x308e03).mirror(0xcf6000).rw(m_dma[0], FUNC(am9516_device::addr_r), FUNC(am9516_device::addr_w));
	map(0x308e04, 0x308e05).mirror(0xcf6000).rw(m_dma[1], FUNC(am9516_device::data_r), FUNC(am9516_device::data_w));
	map(0x308e06, 0x308e07).mirror(0xcf6000).rw(m_dma[1], FUNC(am9516_device::addr_r), FUNC(am9516_device::addr_w));

	map(0x309000, 0x309003).mirror(0xcf6000).umask16(0x00ff).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x309200, 0x3093ff).mirror(0xcf6000).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	map(0x309400, 0x3095ff).mirror(0xcf6000).umask16(0x00ff).rw(m_rtc, FUNC(mm58174_device::read), FUNC(mm58174_device::write));

	map(0x309600, 0x309603).mirror(0xcf6000).m(m_hdc, FUNC(upd7261_device::map)).umask16(0x00ff);

	map(0x309800, 0x309807).mirror(0xcf6000).umask16(0x00ff).rw(m_fdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write));

	map(0x309a00, 0x309bff).mirror(0xcf6000).umask16(0x00ff).lw8(
		[this](u8 data)
		{
			m_fdd->ss_w(BIT(data, 0));
			m_fdc->dden_w(BIT(data, 1));
			m_led_fdd = !BIT(data, 2);
			m_led_err = BIT(data, 3);
			m_hdc->head_w(BIT(data, 4) ? 0x08 : 0x00);
		}, "fdc_reg_w");

	//map(0x309c00, 0x309dff).mirror(0xcf6000); // dma interrupt acknowledge
	map(0x309e00, 0x309e3f).mirror(0xcf6000).umask16(0x00ff).m(m_icu, FUNC(ns32202_device::map<BIT(ST, 1)>));
}

void mg1_state::iop_map(address_map &map)
{
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

	// i/o area
	map(0x2000, 0x201f).mirror(0x1e00).lw8([this](u8 data) { m_buzzen->in_w<1>(BIT(data, 0)); }, "mouse_x"); // mouse x counter/buzzer enable
	map(0x2020, 0x2020).mirror(0x1e00).lw8([this](offs_t offset, u8 data) { logerror("host reset %x,0x%02x\n", offset, data); }, "host_reset").select(0x0100);
	//map(0x2040, 0x205f).mirror(0x1e00).lw8([this](u8 data) { logerror("cursor 0x%02x\n", data); }, "cursor"); // cursor pixel offset & cursor number
	map(0x2060, 0x207f).mirror(0x1e00).lw8([this](u8 data) { m_icu->ir_w<10>(0); }, "iopint");
	map(0x2080, 0x2080).mirror(0x1e00).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x2081, 0x2081).mirror(0x1e00).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x20a0, 0x20bf).mirror(0x1e00).lr8([this]() { return m_mouse_buttons->read(); }, "mouse_b"); // mouse buttons
	//map(0x20c0, 0x20df).mirror(0x1e00).lw8([this](u8 data) { logerror("mouse y counter 0x%02x\n", data); }, "mouse_y"); // mouse y counter
	map(0x20e0, 0x20ff).mirror(0x1e00).rw(m_iop_ctc, FUNC(pit8253_device::read), FUNC(pit8253_device::write));

	map(0x4000, 0x47ff).mirror(0x3800).ram().share("iop_sram"); // D4016C-3 2048x8 SRAM
	map(0x8000, 0xbfff).mirror(0x4000).rom().region("iop_prom", 0);
}

void mg1_state::dma_map(address_map &map)
{
	// system memory buffers swap DMA byte lanes
	map(0x000000, 0xffffff).lrw16(
		[this](offs_t offset, u16 mem_mask) { return swapendian_int16(m_cpu->space(0).read_word(offset << 1, swapendian_int16(mem_mask))); }, "dma_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { m_cpu->space(0).write_word(offset << 1, swapendian_int16(data), swapendian_int16(mem_mask)); }, "dma_w");
}

static INPUT_PORTS_START(mg1)
	PORT_START("mouse_buttons")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Mouse Left Button")      PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Mouse Middle Button")    PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Mouse Right Button")     PORT_CODE(MOUSECODE_BUTTON2)
INPUT_PORTS_END

MC6845_UPDATE_ROW(mg1_state::update_row)
{
	// 16 columns x 50 rows of characters, each 64x16 pixels
	// 10 bits look up video mapping ram -> va6-21 (16 bits)
	// 6 bits give address of 64-bit character in page

	for (unsigned column = 0; column < x_count; column++)
	{
		u16 const vma = ((ma & 0x0ff0) << 4) | ((ra & 0x0f) << 4) | column;
		u32 const va = (u32(m_vmram[0x400 + (vma >> 6)]) << 6) | (vma & 0x3f);

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

static void floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();

	// TODO: unrelated to applix, but same logical format
	fr.add(FLOPPY_APPLIX_FORMAT);
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
	m_icu->out_int().set_inputline(m_cpu, INPUT_LINE_IRQ0).invert();

	/*
	 *  0              edge    lo
	 *  1              level   hi
	 *  2  busint2     level   lo
	 *  3  winint      level   hi
	 *  4  dma2int     edge    lo
	 *  5  busint3     level   lo
	 *  6              level   lo
	 *  7  busint4     level   lo
	 *  8  fdcintr     level   hi
	 *  9  busint5     edge    lo
	 * 10  iopint      edge    lo
	 * 11              edge    hi
	 * 12  busint6     edge    lo
	 * 13  (not used)  edge    lo
	 * 14  (not used)  edge    lo
	 * 15  (not used)  edge    lo
	 *
	 * tpl  0x090a
	 * eltg 0x01ee
	 *
	 *
	 */

	RAM(config, m_ram).set_default_size("2M").set_extra_options("4M,6M,8M").set_default_value(0);

	NVRAM(config, m_sram); // 2xTC5516AP 2048x8 SRAM

	MC68121(config, m_iop, 8_MHz_XTAL / 8); // MC68121 (mode 2)
	m_iop->set_addrmap(0, &mg1_state::iop_map);
	m_iop->in_p2_cb().set([this]() { return m_iop_p2; });

	NVRAM(config, m_iop_sram); // 1xD4016C-3 2048x8 SRAM

	PIT8253(config, m_iop_ctc);
	m_iop_ctc->set_clk<0>(8_MHz_XTAL / 8);
	m_iop_ctc->out_handler<0>().set(m_buzzen, FUNC(input_merger_all_high_device::in_w<0>));

	INPUT_MERGER_ALL_HIGH(config, m_buzzen);
	m_buzzen->output_handler().set(m_buzzer, FUNC(speaker_sound_device::level_w));

	// channel 1 & 2 cursor position
	m_iop_ctc->set_clk<1>(60_MHz_XTAL / 64);
	m_iop_ctc->set_clk<2>(60_MHz_XTAL / 64);
	// out2 -> cout2 (triggered every line)
	// out1 -> cout1 (goes low when cursor required on this line)

	AM9516(config, m_dma[0], 8_MHz_XTAL / 2); // graphics (not used)

	AM9516(config, m_dma[1], 8_MHz_XTAL / 2); // general, ch1 hard disk, ch2 -> J3 or floppy
	m_dma[1]->out_int().set(m_icu, FUNC(ns32202_device::ir_w<4>));
	m_dma[1]->set_addrmap(am9516_device::SYSTEM_MEM, &mg1_state::dma_map);

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
	m_crtc->out_vsync_callback().set(m_iop_ctc, FUNC(pit8253_device::write_gate1));
	m_crtc->out_hsync_callback().set(m_iop_ctc, FUNC(pit8253_device::write_gate2));

	// black & white crt, 56Hz refresh, 46.877kHz line, line sync 1.066uS, frame sync 341.32uS
	// crtc sees it as 16 col x 50 rows, with 64x16 character cells
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(60_MHz_XTAL, 20*64, 1*64, 17*64, 51*16 + 4, 0, 50*16);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	AM7990(config, m_net);
	m_net->intr_out().set(m_icu, FUNC(ns32202_device::ir_w<6>));
	m_net->dma_in().set([this](offs_t offset) { return m_cpu->space(0).read_word(offset); });
	m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask) { m_cpu->space(0).write_word(offset, data, mem_mask); });

	UPD7261(config, m_hdc, 10_MHz_XTAL);
	m_hdc->out_dreq().set(m_dma[1], FUNC(am9516_device::dreq_w<0>)).invert();
	m_hdc->out_int().set(m_icu, FUNC(ns32202_device::ir_w<3>));

	HARDDISK(config, "hdc:0");
	HARDDISK(config, "hdc:1");

	WD1770(config, m_fdc, 8_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(m_icu, FUNC(ns32202_device::ir_w<8>));
	m_fdc->drq_wr_callback().set(m_dma[1], FUNC(am9516_device::dreq_w<1>)).invert();

	// format is 80 tracks, 5 sectors/track, 1024 bytes/sector
	FLOPPY_CONNECTOR(config, "fdc:0", "35dd", FLOPPY_35_DD, true, floppy_formats).enable_sound(true);

	PIT8253(config, m_ctc);

	MG1_KBD(config, m_kbd);
	m_kbd->out_data().set(
		[this](int state)
		{
			if (state)
				m_iop_p2 |= 0x08;
			else
				m_iop_p2 &= ~0x08;
		});

	/*
	 * Documentation indicates the serial clock for the IOP should be driven by the keyboard, however the available
	 * keyboard firmware does not produce a compatible clock output. A hand-drawn sketch indicates the system front
	 * panel allows the serial clock to be generated by a 2.4576MHz crystal and a 4060 divider. The default strapping
	 * selects a 256 divisor, giving a 9600Hz clock which after the 8x divider in the IOP gives a 1200 baud data rate.
	 */
	clock_device &kbd_clk(CLOCK(config, "kbd_clock", 2.4576_MHz_XTAL / 256));
	kbd_clk.signal_handler().set([this](int state) { if (state) m_iop->clock_serial(); });

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_buzzer);
	m_buzzer->add_route(ALL_OUTPUTS, "mono", 0.50);

	//SOFTWARE_LIST(config, "flop_list").set_original("mg1_flop");
	//SOFTWARE_LIST(config, "hdd_list").set_original("mg1_hdd");

	config.set_default_layout(layout_mg1);
}

ROM_START(mg1)
	ROM_REGION16_LE(0x4000, "prom", 0)
	ROM_SYSTEM_BIOS(0, "260", "v2.60")
	ROMX_LOAD("sys_260_even.u291", 0x0000, 0x2000, CRC(24b45b73) SHA1(04d86587b104aa122ac395aa39eb92a1f4d68def), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("sys_260_odd.u292",  0x0001, 0x2000, CRC(a46ebbf8) SHA1(a2ab9fa3a9576d63d8d49730bfcd58a0f508b30f), ROM_BIOS(0) | ROM_SKIP(1))

	// floppy support seems to work better with this firmware version
	ROM_SYSTEM_BIOS(1, "251", "v2.51")
	ROMX_LOAD("even_2.51__sys__13.1_u291.u291", 0x0000, 0x2000, CRC(677cab3c) SHA1(d0197b45ddb1ddd8cd125727312b06dcae0f984a), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("odd_2.51__sys__13.1_u292.u292",  0x0001, 0x2000, CRC(b0134a98) SHA1(a81bd4987030b09799bad0c3bc758ea8aed8cd2f), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_REGION(0x4000, "iop_prom", 0)
	ROM_LOAD("3.0__iop__24.7.87.u285", 0x0000, 0x2000, CRC(733cd089) SHA1(31ffdd85b4ae2ac35dcde292a0d42860baaba88d))
	ROM_RELOAD(                        0x2000, 0x2000)
ROM_END

} // anonymous namespace

/*   YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                       FULLNAME  FLAGS */
COMP(1984, mg1,  0,      0,      mg1,     mg1,   mg1_state, empty_init, "Whitechapel Computer Works", "MG-1",   MACHINE_NOT_WORKING)
