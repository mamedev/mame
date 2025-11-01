/*
 *
 * Convergent AWS series
 *
 * Predecessor to the NGen series.  8088 or 8086 based workstations.
 *
 */

#include "emu.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "machine/i8257.h"
#include "video/i8275.h"
#include "machine/z80sio.h"
#include "cpu/i86/i86.h"
#include "screen.h"


namespace {

class aws_state : public driver_device
{
public:
	aws_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dmac(*this, "dmac"),
		m_pic(*this, "pic"),
		m_pit(*this, "pit"),
		m_fdc(*this, "fdc"),
		m_crtc(*this, "crtc"),
		m_serial1(*this, "serial1"),
		//m_serial2(*this, "serial2")
		m_fontrom(*this, "fontrom")
		{}

	void aws(machine_config &config);

protected:
	void aws_mem(address_map &map);
	void aws_io(address_map &map);

private:
	required_device<i8086_cpu_device> m_maincpu;
	required_device<i8257_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<i8272a_device> m_fdc;
	required_device<i8275_device> m_crtc;
	required_device<upd7201_device> m_serial1;  // Cluster networking and Keyboard
//  required_device<upd7201_device> m_serial2;  // RS-232C ports

	required_region_ptr<u8> m_fontrom;

	void pit_out0_w(int state);
	void pit_out1_w(int state);
	void pit_out2_w(int state);

	void hrq_w(int state);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	void ram_w(offs_t offset, uint8_t data);
	uint8_t ram_r(offs_t offset);
};

void aws_state::pit_out0_w(int state)
{
}

void aws_state::pit_out1_w(int state)
{
}

void aws_state::pit_out2_w(int state)
{
	m_serial1->dcdb_w(state);
}

I8275_DRAW_CHARACTER_MEMBER(aws_state::display_pixels)
{
//  using namespace i8275_attributes;
	for(int i=0;i<9;i++)
	{
		if(i>=7)
			bitmap.pix(y, x + i) = rgb_t::black();
		else
			bitmap.pix(y, x + i) = BIT(m_fontrom[((linecount & 0x0f) << 7) | (charcode & 0x7f)],i) ? rgb_t::white() : rgb_t::black();
	}
}

void aws_state::hrq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT,state);
	m_dmac->hlda_w(state);
}

void aws_state::ram_w(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

uint8_t aws_state::ram_r(offs_t offset)
{
	uint8_t ret = m_maincpu->space(AS_PROGRAM).read_byte(offset);
	return ret;
}

static INPUT_PORTS_START( aws )
INPUT_PORTS_END

void aws_state::aws_mem(address_map &map)
{
	map(0x00000, 0x7ffff).ram();
	map(0xff000, 0xfffff).rom().region("bios", 0);
}

void aws_state::aws_io(address_map &map)
{
	map(0x0000, 0x000f).rw(m_dmac, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x0020, 0x0023).rw(m_crtc, FUNC(i8275_device::read), FUNC(i8275_device::write)).umask16(0x00ff);
	map(0x0040, 0x0047).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x0060, 0x0067).rw(m_serial1, FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::cd_ba_w)).umask16(0x00ff);
	// 0x80-0x8f - i8272/uPD765 floppy or 8X320 HDC
	map(0x0080, 0x008f).m(m_fdc, FUNC(i8272a_device::map));
	map(0x00a0, 0x00a1).rw(m_pic,FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	// 0xa4      - Extended communication status/controller
	// 0xa8-0xab - uPD7201 serial (RS-232)
	// 0xac-0xaf - 8253 timer (RS-232)
	// 0xb0      - Printer status/data
	// 0xb4      - Extended DMA address (disk)
	// 0xe0      - Parity - low 8 bits of address
	// 0xe4      - Parity - mid 8 bits of address
	// 0xe8      - Parity - high 4 bits of address, bit 4 high is DMA is the cause
	// 0xf0      - Enable parity
	// 0xf4      - Disable parity, clear error
}

void aws_state::aws(machine_config &config)
{
	// basic machine hardware
	I8086(config, m_maincpu, 24_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &aws_state::aws_mem);
	m_maincpu->set_addrmap(AS_IO, &aws_state::aws_io);

	I8257(config, m_dmac, 24_MHz_XTAL / 8);
	m_dmac->out_hrq_cb().set(FUNC(aws_state::hrq_w));
	m_dmac->in_memr_cb().set(FUNC(aws_state::ram_r));
	m_dmac->out_memw_cb().set(FUNC(aws_state::ram_w));
	m_dmac->out_iow_cb<2>().set(m_crtc,FUNC(i8275_device::dack_w));

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(19.6608_MHz_XTAL / 16);  // 1.23 MHz
	m_pit->out_handler<0>().set(FUNC(aws_state::pit_out0_w));  // Tone generator
	m_pit->set_clk<1>(19.6608_MHz_XTAL / 16);  // 1.23 MHz
	m_pit->out_handler<1>().set(FUNC(aws_state::pit_out1_w));  // Cluster communications clock
	m_pit->set_clk<2>(76000);  // 76kHz
	m_pit->out_handler<2>().set(FUNC(aws_state::pit_out2_w));  // Timer interrupt

	UPD7201(config, m_serial1, 24_MHz_XTAL / 8);
	m_serial1->out_int_callback().set(m_pic,FUNC(pic8259_device::ir1_w));

	// video
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_size(720, 320);
	screen.set_visarea(0, 719, 0, 319);
	screen.set_refresh_hz(60);
	screen.set_screen_update(m_crtc, FUNC(i8275_device::screen_update));

	I8275(config, m_crtc, 17.82_MHz_XTAL / 9);  // divisor unsure?
	m_crtc->set_character_width(9);
	// TODO: display callback
	m_crtc->set_display_callback(FUNC(aws_state::display_pixels));
	m_crtc->drq_wr_callback().set(m_dmac,FUNC(i8257_device::dreq2_w));
	m_crtc->set_screen("screen");

	// FDC board
	I8272A(config, m_fdc, 0);
	PIC8259(config, m_pic, 0);

}

ROM_START( aws220 )
	ROM_REGION16_LE( 0x1000, "bios", 0)
	ROM_LOAD( "72-00077_l.bin",  0x000000, 0x001000, CRC(87ca4912) SHA1(c4f7ecda8d007bb212166cfa3cdf494da3966ca9) )  // bootstrap ROM v6.0

	ROM_REGION( 0x1000, "fontrom", 0 )
	ROM_LOAD( "72-00098_r.bin",  0x000000, 0x001000, CRC(e74c12a7) SHA1(0cde785aad1bdad5b44c41ba5b05bfb0eb0b1092) )

ROM_END

}  // anonymous namesapce

COMP( 1982, aws220,    0,    0, aws,    aws, aws_state,    empty_init, "Convergent Technologies",  "AWS-220", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
