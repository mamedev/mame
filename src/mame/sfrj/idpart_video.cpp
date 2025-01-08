// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic

#include "emu.h"
#include "idpart_video.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8257.h"
#include "machine/pit8253.h"
#include "machine/z80sio.h"
#include "video/i8275.h"

#include "emupal.h"
#include "screen.h"

namespace {

class idpart_video_device : public device_t, public device_rs232_port_interface
{
public:
	idpart_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, IDPART_VIDEO, tag, owner, clock)
		, device_rs232_port_interface(mconfig, *this)
		, m_maincpu(*this, "maincpu")
		, m_mpsc(*this, "mpsc")
		, m_serial(*this, "serial")
		, m_dma(*this, "dma")
		, m_pit(*this, "pit")
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_chargen(*this, "chargen")
		, m_dreq0(0)
		, m_dack0(0)
	{
	}

	virtual void input_txd(int state) override { m_mpsc->rxa_w(state); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);

	uint8_t memory_read_byte(offs_t offset) { return m_program.read_byte(offset); }
	void memory_write_byte(offs_t offset, uint8_t data) { m_program.write_byte(offset, data); }
	u8 fast_zero_r(offs_t offset) { return 0; }
	void dreq0_w(int state) { m_dreq0 = state; }
	int dreq0_r() { return m_dreq0; }
	void dack0_w(int state) { m_dack0 = state; }
	void tc_w(int state) { if (!m_dack0) m_maincpu->set_input_line(I8085_RST75_LINE, state);}

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8274_device> m_mpsc;
	required_device<rs232_port_device> m_serial;
	required_device<i8257_device> m_dma;
	required_device<pit8254_device> m_pit;
	required_device<i8275_device> m_crtc;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_chargen;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	int m_dreq0;
	int m_dack0;
};


/* Memory maps */
void idpart_video_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).mirror(0x4000).rom().region("maincpu", 0);
	map(0x8000, 0x8bff).mirror(0x6000).ram();
}

void idpart_video_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x01).mirror(0x0e).rw(m_crtc, FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0x10, 0x13).mirror(0x0c).rw(m_mpsc, FUNC(i8274_device::cd_ba_r), FUNC(i8274_device::cd_ba_w));
	map(0x20, 0x2f).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x30, 0x33).mirror(0x0c).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
}

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	128,                    /* 128 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_idpart_video )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

I8275_DRAW_CHARACTER_MEMBER(idpart_video_device::display_pixels)
{
	using namespace i8275_attributes;
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u8 pixels = m_chargen[(linecount & 7) + (charcode << 3)];
	if (BIT(attrcode, VSP))
		pixels = 0;

	if (BIT(attrcode, LTEN))
		pixels = 0xff;

	if (BIT(attrcode, RVV))
		pixels ^= 0xff;
	bool hlgt = BIT(attrcode, HLGT);
	for (u8 i = 0; i < 7; i++)
		bitmap.pix(y, x + i) = palette[((pixels >> (7-i)) & 1) ? (hlgt ? 2 : 1) : 0];
}

static DEVICE_INPUT_DEFAULTS_START(keyboard)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_300)
DEVICE_INPUT_DEFAULTS_END

void idpart_video_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(6'144'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &idpart_video_device::mem_map);
	m_maincpu->set_addrmap(AS_IO, &idpart_video_device::io_map);
	m_maincpu->in_sid_func().set(*this, FUNC(idpart_video_device::dreq0_r));

	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);
	GFXDECODE(config, "gfxdecode", "palette", gfx_idpart_video);

	RS232_PORT(config, m_serial, default_rs232_devices, nullptr);
	m_serial->rxd_handler().set(m_mpsc, FUNC(z80sio_device::rxb_w));
	m_serial->set_default_option("keyboard");
	m_serial->set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));

	I8274(config, m_mpsc, XTAL(6'144'000) / 2);
	m_mpsc->out_txdrqa_callback().set(m_dma, FUNC(i8257_device::dreq2_w));
	m_mpsc->out_rxdrqa_callback().set(m_dma, FUNC(i8257_device::dreq3_w));
	m_mpsc->out_txda_callback().set(*this, FUNC(idpart_video_device::output_rxd));
	m_mpsc->out_txdb_callback().set(m_serial, FUNC(rs232_port_device::write_txd));
	m_mpsc->out_int_callback().set_inputline(m_maincpu, I8085_RST65_LINE);

	I8257(config, m_dma, XTAL(6'144'000) / 2);
	m_dma->out_hrq_cb().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_dma->out_hrq_cb().append(m_dma, FUNC(i8257_device::hlda_w));
	m_dma->in_memr_cb().set(FUNC(idpart_video_device::memory_read_byte));
	m_dma->out_memw_cb().set(FUNC(idpart_video_device::memory_write_byte));
	m_dma->out_tc_cb().set(*this, FUNC(idpart_video_device::tc_w));
	m_dma->out_iow_cb<0>().set(m_crtc, FUNC(i8275_device::dack_w));
	m_dma->out_dack_cb<0>().set(*this, FUNC(idpart_video_device::dack0_w));
	m_dma->in_ior_cb<1>().set(*this, FUNC(idpart_video_device::fast_zero_r));
	m_dma->in_ior_cb<2>().set(m_mpsc, FUNC(i8274_device::da_r));
	m_dma->out_iow_cb<3>().set(m_mpsc, FUNC(i8274_device::da_w));

	// PIT
	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(XTAL(6'144'000) / 2);
	m_pit->set_clk<1>(XTAL(6'144'000) / 2);
	m_pit->set_clk<2>(XTAL(6'144'000) / 2);
	m_pit->out_handler<0>().set(m_mpsc, FUNC(i8274_device::txca_w));
	m_pit->out_handler<0>().append(m_mpsc, FUNC(i8274_device::rxca_w));
	m_pit->out_handler<1>().set(m_mpsc, FUNC(i8274_device::txcb_w));
	m_pit->out_handler<1>().append(m_mpsc, FUNC(i8274_device::rxcb_w));

	/* video hardware */
	I8275(config, m_crtc, XTAL(22'680'000) / 2 / 7);
	m_crtc->set_character_width(7);
	m_crtc->set_display_callback(FUNC(idpart_video_device::display_pixels));
	m_crtc->set_screen("screen");
	m_crtc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq0_w));
	m_crtc->drq_wr_callback().append(*this, FUNC(idpart_video_device::dreq0_w));
	m_crtc->irq_wr_callback().set_inputline(m_maincpu, I8085_RST55_LINE);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(22'680'000) / 2, 728, 0, 560, 310, 0, 270);
	screen.set_screen_update(m_crtc, FUNC(i8275_device::screen_update));
}

INPUT_PORTS_START(idpart_video)
INPUT_PORTS_END

ioport_constructor idpart_video_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(idpart_video);
}

ROM_START(idpart_video)
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "video.e34",  0x0000, 0x0800, CRC(67e2c351) SHA1(1ca86b1b35d9a213db0c3f291dc839fc3621269c))
	ROM_REGION( 0x0800, "chargen",0 )
	ROM_LOAD( "video.e4",   0x0000, 0x0800, CRC(5fe52e0d) SHA1(c6067e3fdda84e673aaa74ae6561b535157a8281))
ROM_END

const tiny_rom_entry *idpart_video_device::device_rom_region() const
{
	return ROM_NAME( idpart_video );
}

void idpart_video_device::device_start()
{
	m_maincpu->space(AS_PROGRAM).specific(m_program);
	save_item(NAME(m_dack0));
	save_item(NAME(m_dreq0));
}

void idpart_video_device::device_reset()
{
	m_dma->dreq1_w(1);

	output_rxd(1);

	output_dcd(0);
	output_dsr(0);
	output_cts(0);
	m_dack0 = 0;
	m_dreq0 = 0;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(IDPART_VIDEO, device_rs232_port_interface, idpart_video_device, "idpart_video", "Iskra Delta Partner Video (Text) Board")
