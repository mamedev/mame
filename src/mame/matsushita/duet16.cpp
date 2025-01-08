// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/floppy.h"
#include "machine/i8087.h"
#include "machine/i8251.h"
#include "machine/input_merger.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/upd765.h"
#include "machine/msm58321.h"
#include "machine/6840ptm.h"
#include "machine/z80sio.h"
#include "machine/pit8253.h"
#include "machine/am9517a.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/keyboard.h"


namespace {

class duet16_state : public driver_device
{
public:
	duet16_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic(*this, "pic"),
		m_fdc(*this, "fdc"),
		m_dmac(*this, "dmac"),
		m_fd(*this, "fdc:%u", 0),
		m_pal(*this, "palette"),
		m_chrpal(*this, "chrpal"),
		m_rtc(*this, "rtc"),
		m_tmint(*this, "tmint"),
		m_screen(*this, "screen"),
		m_chrrom(*this, "char"),
		m_cvram(*this, "cvram"),
		m_gvram(*this, "gvram")
	{ }

	void duet16(machine_config &config);
protected:
	void machine_reset() override ATTR_COLD;
private:
	u8 pic_r(offs_t offset);
	void pic_w(offs_t offset, u8 data);
	u8 dma_mem_r(offs_t offset);
	void dma_mem_w(offs_t offset, u8 data);
	u8 dmapg_r();
	void dmapg_w(u8 data);
	void fdcctrl_w(u8 data);
	void dispctrl_w(u8 data);
	void pal_w(offs_t offset, u8 data);
	void hrq_w(int state);
	u8 rtc_r();
	void rtc_w(u8 data);
	u8 rtc_stat_r();
	void rtc_addr_w(u8 data);
	u16 sysstat_r();
	void rtc_d0_w(int state);
	void rtc_d1_w(int state);
	void rtc_d2_w(int state);
	void rtc_d3_w(int state);
	void rtc_busy_w(int state);
	void rtc_irq_reset();
	MC6845_UPDATE_ROW(crtc_update_row);
	void duet16_io(address_map &map) ATTR_COLD;
	void duet16_mem(address_map &map) ATTR_COLD;
	required_device<i8086_cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<upd765a_device> m_fdc;
	required_device<am9517a_device> m_dmac;
	required_device_array<floppy_connector, 2> m_fd;
	required_device<palette_device> m_pal;
	required_device<palette_device> m_chrpal;
	required_device<msm58321_device> m_rtc;
	required_device<input_merger_device> m_tmint;
	required_device<screen_device> m_screen;
	required_memory_region m_chrrom;
	required_shared_ptr<u16> m_cvram;
	required_shared_ptr<u16> m_gvram;
	u8 m_dmapg, m_dispctrl;
	u8 m_rtc_d;
	bool m_rtc_busy, m_rtc_irq;
};

void duet16_state::machine_reset()
{
	m_rtc->cs1_w(ASSERT_LINE);
	rtc_irq_reset();
}

u8 duet16_state::pic_r(offs_t offset)
{
	return m_pic->read(offset ^ 1);
}

void duet16_state::pic_w(offs_t offset, u8 data)
{
	m_pic->write(offset ^ 1, data);
}

void duet16_state::fdcctrl_w(u8 data)
{
	floppy_image_device *f = m_fd[BIT(data, 2) ? 1 : 0]->get_device();
	m_fdc->set_floppy(f);

	m_fd[0]->get_device()->mon_w(!BIT(data, 0));
	m_fd[1]->get_device()->mon_w(!BIT(data, 0));
	m_fdc->reset_w(!BIT(data, 1));

	// TODO: bit 3 = LSPD
}

u8 duet16_state::dma_mem_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte((m_dmapg << 16) | offset);
}

void duet16_state::dma_mem_w(offs_t offset, u8 data)
{
	m_maincpu->space(AS_PROGRAM).write_byte((m_dmapg << 16) | offset, data);
}

u8 duet16_state::dmapg_r()
{
	return m_dmapg;
}

void duet16_state::dmapg_w(u8 data)
{
	m_dmapg = data & 0xf;
}

void duet16_state::hrq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dmac->hack_w(state);
}

u16 duet16_state::sysstat_r()
{
	return 0xb484;
}

void duet16_state::duet16_mem(address_map &map)
{
	map(0x00000, 0x8ffff).ram();
	map(0xa8000, 0xbffff).ram().share("gvram");
	map(0xc0000, 0xc0fff).ram().share("cvram");
	map(0xf8000, 0xf801f).rw(FUNC(duet16_state::dmapg_r), FUNC(duet16_state::dmapg_w)).umask16(0x00ff);
	map(0xf8000, 0xf801f).rw("dmac", FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0xff00);
	map(0xf8020, 0xf8023).rw(FUNC(duet16_state::pic_r), FUNC(duet16_state::pic_w)).umask16(0x00ff);
	map(0xf8040, 0xf804f).rw("itm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
	map(0xf8060, 0xf8067).rw("bgpit", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0xf8080, 0xf8087).rw("sio", FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask16(0x00ff);
	map(0xf80a0, 0xf80a3).rw("kbusart", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0xf80c0, 0xf80c0).rw("crtc", FUNC(hd6845s_device::status_r), FUNC(hd6845s_device::address_w));
	map(0xf80c2, 0xf80c2).rw("crtc", FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0xf80e0, 0xf80e3).rw("i8741", FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w)).umask16(0x00ff);
	map(0xf8100, 0xf8103).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff);
	map(0xf8120, 0xf8120).rw(FUNC(duet16_state::rtc_r), FUNC(duet16_state::rtc_w));
	map(0xf8160, 0xf819f).w(FUNC(duet16_state::pal_w));
	map(0xf8200, 0xf8201).r(FUNC(duet16_state::sysstat_r));
	map(0xf8220, 0xf8220).w(FUNC(duet16_state::fdcctrl_w));
	map(0xf8260, 0xf8260).w(FUNC(duet16_state::rtc_addr_w));
	map(0xf8280, 0xf8280).r(FUNC(duet16_state::rtc_stat_r));
	map(0xf8280, 0xf8280).w(FUNC(duet16_state::dispctrl_w));
	map(0xfe000, 0xfffff).rom().region("rom", 0);
}

void duet16_state::duet16_io(address_map &map)
{
}

void duet16_state::pal_w(offs_t offset, u8 data)
{
	int entry = (BIT(offset, 0) ? 2 : 0) | (BIT(offset, 5) ? 0 : 4);
	m_pal->set_pen_color(entry, pal1bit(BIT(data, 1)), pal1bit(BIT(data, 2)), pal1bit(BIT(data, 0)));
	m_pal->set_pen_color(entry + 1, pal1bit(BIT(data, 5)), pal1bit(BIT(data, 6)), pal1bit(BIT(data, 4)));
}

void duet16_state::dispctrl_w(u8 data)
{
	m_dispctrl = data;
}

MC6845_UPDATE_ROW(duet16_state::crtc_update_row)
{
	if(!de)
		return;
	u8 const *const gvram = (u8 *)&m_gvram[0];
	for(int i = 0; i < x_count; i++)
	{
		u16 coffset = (ma + i) & 0x07ff;
		u16 goffset = (((ma * 16) + (ra * 80) + i) & 0x7fff) ^ 1;
		u8 g2 = gvram[goffset];
		u8 g1 = gvram[goffset + 0x08000];
		u8 g0 = gvram[goffset + 0x10000];
		u8 attr = m_cvram[coffset] >> 8;
		u8 chr = m_cvram[coffset & ~BIT(attr, 6)] & 0xff;
		u8 data = m_chrrom->base()[(chr * 16) + ra + (BIT(m_dispctrl, 3) * 0x1000)];
		if(BIT(attr, 6))
		{
			if(!(i & 1))
				data = bitswap<8>(data, 7, 7, 6, 6, 5, 5, 4, 4);
			else
				data = bitswap<8>(data, 3, 3, 2, 2, 1, 1, 0, 0);
		}
		if(BIT(attr, 4) && (m_screen->frame_number() & 32)) // ~1.7 Hz
			data = 0;
		if(BIT(attr, 3))
			data ^= 0xff;
		if(BIT(attr, 5) && (ra > 12)) // underline start?
		{
			attr |= 7;
			data = 0xff;
		}
		if(((i & ~BIT(attr, 6)) == cursor_x) && (m_screen->frame_number() & 16)) // ~3.4 Hz
			data ^= 0xff;

		rgb_t fg = m_chrpal->pen_color(attr & 7);

		for(int xi = 0; xi < 8; xi++)
		{
			rgb_t color;
			if((data & (0x80 >> xi)) && BIT(m_dispctrl, 1))
				color = fg;
			else if(BIT(m_dispctrl, 0))
				color = m_pal->pen_color((BIT(g2, 7 - xi) << 2) | (BIT(g1, 7 - xi) << 1) | BIT(g0, 7 - xi));
			else
				color = 0;
			bitmap.pix(y, (i * 8) + xi) = color;
		}
	}
}

void duet16_state::rtc_d0_w(int state)
{
	m_rtc_d = (m_rtc_d & ~1) | (state ? 1 : 0);
}

void duet16_state::rtc_d1_w(int state)
{
	m_rtc_d = (m_rtc_d & ~2) | (state ? 2 : 0);
}

void duet16_state::rtc_d2_w(int state)
{
	m_rtc_d = (m_rtc_d & ~4) | (state ? 4 : 0);
}

void duet16_state::rtc_d3_w(int state)
{
	m_rtc_d = (m_rtc_d & ~8) | (state ? 8 : 0);
}

void duet16_state::rtc_busy_w(int state)
{
	if (state && !m_rtc_busy && !m_rtc_irq)
	{
		m_rtc_irq = true;
		m_tmint->in_w<1>(1);
	}
	m_rtc_busy = state;
}

void duet16_state::rtc_irq_reset()
{
	m_rtc_irq = false;
	m_tmint->in_w<1>(0);
}

u8 duet16_state::rtc_r()
{
	u8 ret;
	m_rtc->cs2_w(ASSERT_LINE);
	m_rtc->read_w(ASSERT_LINE);
	ret = m_rtc_d;
	m_rtc->read_w(CLEAR_LINE);
	m_rtc->cs2_w(CLEAR_LINE);
	return ret;
}

void duet16_state::rtc_w(u8 data)
{
	m_rtc->d0_w(BIT(data, 0));
	m_rtc->d1_w(BIT(data, 1));
	m_rtc->d2_w(BIT(data, 2));
	m_rtc->d3_w(BIT(data, 3));
	m_rtc->cs2_w(ASSERT_LINE);
	m_rtc->write_w(ASSERT_LINE);
	m_rtc->write_w(CLEAR_LINE);
	m_rtc->cs2_w(CLEAR_LINE);
}

u8 duet16_state::rtc_stat_r()
{
	u8 status = m_rtc_irq ? 0x80 : 0;
	if (!machine().side_effects_disabled())
		rtc_irq_reset();
	return status;
}

void duet16_state::rtc_addr_w(u8 data)
{
	m_rtc->d0_w(BIT(data, 0));
	m_rtc->d1_w(BIT(data, 1));
	m_rtc->d2_w(BIT(data, 2));
	m_rtc->d3_w(BIT(data, 3));
	m_rtc->cs2_w(ASSERT_LINE);
	m_rtc->address_write_w(ASSERT_LINE);
	m_rtc->address_write_w(CLEAR_LINE);
	m_rtc->cs2_w(CLEAR_LINE);
}

static const gfx_layout duet16_charlayout =
{
	8, 16,                   /* 8 x 16 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ STEP8(0,1) },
	/* y offsets */
	{ STEP16(0,8) },
	8*16                 /* every char takes 8 bytes */
};

static GFXDECODE_START(gfx_duet16)
	GFXDECODE_ENTRY( "char", 0x0000, duet16_charlayout, 0, 1 )
GFXDECODE_END


static void duet16_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void duet16_keyboard_devices(device_slot_interface &device)
{
	device.option_add("keyboard", SERIAL_KEYBOARD);
}

static DEVICE_INPUT_DEFAULTS_START(keyboard)
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void duet16_state::duet16(machine_config &config)
{
	I8086(config, m_maincpu, 24_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &duet16_state::duet16_mem);
	m_maincpu->set_addrmap(AS_IO, &duet16_state::duet16_io);
	m_maincpu->set_irq_acknowledge_callback("pic", FUNC(pic8259_device::inta_cb));
	m_maincpu->esc_opcode_handler().set("i8087", FUNC(i8087_device::insn_w));
	m_maincpu->esc_data_handler().set("i8087", FUNC(i8087_device::addr_w));

	i8087_device &i8087(I8087(config, "i8087", 24_MHz_XTAL / 3));
	i8087.set_space_86(m_maincpu, AS_PROGRAM);
	i8087.irq().set(m_pic, FUNC(pic8259_device::ir2_w)); // INT87
	i8087.busy().set_inputline(m_maincpu, INPUT_LINE_TEST);

	I8741A(config, "i8741", 20_MHz_XTAL / 4);

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	AM9517A(config, m_dmac, 20_MHz_XTAL / 4);
	m_dmac->out_hreq_callback().set(FUNC(duet16_state::hrq_w));
	m_dmac->in_memr_callback().set(FUNC(duet16_state::dma_mem_r));
	m_dmac->out_memw_callback().set(FUNC(duet16_state::dma_mem_w));
	m_dmac->in_ior_callback<0>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_dmac->out_iow_callback<0>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_dmac->out_eop_callback().set(m_fdc, FUNC(upd765a_device::tc_line_w));

	pit8253_device &bgpit(PIT8253(config, "bgpit", 0));
	bgpit.set_clk<0>(8_MHz_XTAL / 13);
	bgpit.set_clk<1>(8_MHz_XTAL / 13);
	bgpit.set_clk<2>(8_MHz_XTAL / 13);
	bgpit.out_handler<0>().set("sio", FUNC(upd7201_device::txca_w)); // TODO: selected through LS153
	bgpit.out_handler<0>().append("sio", FUNC(upd7201_device::rxca_w));
	bgpit.out_handler<1>().set("sio", FUNC(upd7201_device::txcb_w));
	bgpit.out_handler<1>().append("sio", FUNC(upd7201_device::rxcb_w));
	bgpit.out_handler<2>().set("kbusart", FUNC(i8251_device::write_txc));
	bgpit.out_handler<2>().append("kbusart", FUNC(i8251_device::write_rxc));

	ptm6840_device &itm(PTM6840(config, "itm", 8_MHz_XTAL / 8));
	itm.set_external_clocks(0.0, 0.0, (8_MHz_XTAL / 8).dvalue()); // C3 = 1MHz
	itm.o3_callback().set("itm", FUNC(ptm6840_device::set_c1)); // C1 = C2 = O3
	itm.o3_callback().append("itm", FUNC(ptm6840_device::set_c2));
	itm.irq_callback().set(m_tmint, FUNC(input_merger_device::in_w<0>));

	upd7201_device& sio(UPD7201(config, "sio", 8_MHz_XTAL / 2));
	sio.out_int_callback().set("pic", FUNC(pic8259_device::ir1_w)); // INT5

	i8251_device &kbusart(I8251(config, "kbusart", 8_MHz_XTAL / 4));
	kbusart.txd_handler().set("kbd", FUNC(rs232_port_device::write_txd));
	kbusart.rts_handler().set("kbusart", FUNC(i8251_device::write_cts));
	kbusart.rxrdy_handler().set("kbint", FUNC(input_merger_device::in_w<0>));
	kbusart.txrdy_handler().set("kbint", FUNC(input_merger_device::in_w<1>));

	rs232_port_device &kbd(RS232_PORT(config, "kbd", duet16_keyboard_devices, "keyboard"));
	kbd.rxd_handler().set("kbusart", FUNC(i8251_device::write_rxd));
	kbd.set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));

	INPUT_MERGER_ANY_HIGH(config, "kbint").output_handler().set(m_pic, FUNC(pic8259_device::ir5_w)); // INT2

	INPUT_MERGER_ANY_HIGH(config, m_tmint).output_handler().set(m_pic, FUNC(pic8259_device::ir0_w)); // INT6

	UPD765A(config, m_fdc, 8_MHz_XTAL, true, false);
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq0_w));
	m_fdc->intrq_wr_callback().set(m_pic, FUNC(pic8259_device::ir3_w)); // INT4
	FLOPPY_CONNECTOR(config, "fdc:0", duet16_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats, true);
	FLOPPY_CONNECTOR(config, "fdc:1", duet16_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats, true);

	hd6845s_device &crtc(HD6845S(config, "crtc", 2000000)); // "46505S" on schematics
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(duet16_state::crtc_update_row));

	PALETTE(config, m_pal).set_entries(8);
	PALETTE(config, m_chrpal, palette_device::BRG_3BIT);

	GFXDECODE(config, "gfxdecode", m_chrpal, gfx_duet16);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(640, 480);
	m_screen->set_visarea_full();
	m_screen->set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	MSM58321(config, m_rtc, 32768_Hz_XTAL);
	m_rtc->d0_handler().set(FUNC(duet16_state::rtc_d0_w));
	m_rtc->d1_handler().set(FUNC(duet16_state::rtc_d1_w));
	m_rtc->d2_handler().set(FUNC(duet16_state::rtc_d2_w));
	m_rtc->d3_handler().set(FUNC(duet16_state::rtc_d3_w));
	m_rtc->busy_handler().set(FUNC(duet16_state::rtc_busy_w));
	m_rtc->set_year0(1980);
	m_rtc->set_default_24h(true);
}

ROM_START(duet16)
	ROM_REGION16_LE(0x2000, "rom", 0)
	ROM_LOAD16_BYTE("duet16_h516a_3.bin", 0x0001, 0x1000, CRC(936706aa) SHA1(412ff9c7bf4443d2ed29a8d792fc3c849c9393cc))
	ROM_LOAD16_BYTE("duet16_h517a_z.bin", 0x0000, 0x1000, CRC(1633cce8) SHA1(5145d04a48921cacfed17a94873e8988772fc8d4))

	ROM_REGION(0x2000, "char", 0)
	ROM_LOAD("duet16_char_j500a_4.bin", 0x0000, 0x2000, CRC(edf860f8) SHA1(0dcc584db701d21b7c3304cd2296562ebda6fb4c))

	ROM_REGION(0x400, "i8741", 0)
	ROM_LOAD("duet16_key_8741ak001b_z.bin", 0x000, 0x400, CRC(d23ee68d) SHA1(3b6a86fe2a304823c5385cd673f9580a35199dac))
ROM_END

} // anonymous namespace


COMP( 1983, duet16, 0, 0, duet16, 0, duet16_state, empty_init, "Panafacom (Panasonic/Fujitsu)", "Duet-16", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
