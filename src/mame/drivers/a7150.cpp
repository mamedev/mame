// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Sergey Svishchev
/***************************************************************************

Robotron A7150

2009-10-04 Skeleton driver.

http://www.robotrontechnik.de/index.htm?/html/computer/a7150.htm

http://www.tiffe.de/Robotron/MMS16/
- Confidence test is documented in A7150_Rechner...pdf, pp. 112-119
- Internal test of KGS -- in KGS-K7070.pdf, pp. 19-23

After about a minute, the self-test will appear.

To do:
- Machine hangs when screen should scroll
- MMS16 (Multibus clone) and slot devices
- native keyboard
- A7100 model

****************************************************************************/

#include "emu.h"

#include "cpu/i86/i86.h"
#include "machine/i8087.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/bankdev.h"
#include "machine/input_merger.h"

#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"

#include "bus/rs232/rs232.h"
#include "machine/isbc_215g.h"
#include "machine/keyboard.h"

#include "emupal.h"
#include "screen.h"


#define SCREEN_TAG          "screen"
#define Z80_TAG             "gfxcpu"
#define Z80CTC_TAG          "z80ctc"
#define Z80SIO_TAG          "z80sio"
#define RS232_A_TAG         "kgsv24"
#define RS232_B_TAG         "kgsifss"


class a7150_state : public driver_device
{
public:
	a7150_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart8251(*this, "uart8251")
		, m_pit8253(*this, "pit8253")
		, m_pic8259(*this, "pic8259")
		, m_gfxcpu(*this, "gfxcpu")
		, m_ctc(*this, Z80CTC_TAG)
		, m_rs232(*this, "rs232")
		, m_video_ram(*this, "video_ram")
		, m_video_bankdev(*this, "video_bankdev")
		, m_palette(*this, "palette")
	{ }

	void a7150(machine_config &config);

protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	uint8_t a7150_kgs_r(offs_t offset);
	void a7150_kgs_w(offs_t offset, uint8_t data);

	void a7150_tmr2_w(int state);
	void ppi_c_w(uint8_t data);

	void ifss_write_txd(int state);
	void ifss_write_dtr(int state);

	uint8_t kgs_host_r(offs_t offset);
	void kgs_host_w(offs_t offset, uint8_t data);
	void kgs_iml_w(int state);
	void ifss_loopback_w(int state);
	void kbd_put(uint8_t data);
	void kgs_memory_remap();

	bool m_kgs_msel, m_kgs_iml;
	uint8_t m_kgs_datao, m_kgs_datai, m_kgs_ctrl;
	bool m_ifss_loopback;

	uint32_t screen_update_k7072(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);

	required_device<i8086_cpu_device> m_maincpu;
	required_device<i8251_device> m_uart8251;
	required_device<pit8253_device> m_pit8253;
	required_device<pic8259_device> m_pic8259;

	required_device<z80_device> m_gfxcpu;
	required_device<z80ctc_device> m_ctc;
	required_device<rs232_port_device> m_rs232;
	required_shared_ptr<uint8_t> m_video_ram;
	required_device<address_map_bank_device> m_video_bankdev;
	required_device<palette_device> m_palette;

	void io_map(address_map &map);
	void mem_map(address_map &map);
	void k7070_cpu_banked(address_map &map);
	void k7070_cpu_io(address_map &map);
	void k7070_cpu_mem(address_map &map);
};


uint32_t a7150_state::screen_update_k7072(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int addr = 0;

	for (int y = 0; y < 400; y++)
	{
		int horpos = 0;
		for (int x = 0; x < 80; x++)
		{
			uint8_t code = m_video_ram[addr++];
			for (int b = 0; b < 8; b++)
			{
				bitmap.pix(y, horpos++) = (code >> (7 - b)) & 1;
			}
		}
	}

	return 0;
}

void a7150_state::kgs_iml_w(int state)
{
	m_kgs_iml = !state;
	kgs_memory_remap();
}

void a7150_state::a7150_tmr2_w(int state)
{
	m_uart8251->write_rxc(state);
	m_uart8251->write_txc(state);
}

void a7150_state::ifss_loopback_w(int state)
{
	m_ifss_loopback = !state;
}

void a7150_state::ifss_write_txd(int state)
{
	if (m_ifss_loopback)
		m_uart8251->write_rxd(state);
	else
		m_rs232->write_txd(state);
}

void a7150_state::ifss_write_dtr(int state)
{
	if (m_ifss_loopback)
		m_uart8251->write_dsr(state);
	else
		m_rs232->write_dtr(state);
}

void a7150_state::ppi_c_w(uint8_t data)
{
	// b0 -- INTR(B)
	// b1 -- /OBF(B)
	// m_centronics->write_ack(BIT(data, 2));
	// m_centronics->write_strobe(BIT(data, 3));
}

#define KGS_ST_OBF  0x01
#define KGS_ST_IBF  0x02
#define KGS_ST_INT  0x04
#define KGS_ST_ERR  0x80

uint8_t a7150_state::kgs_host_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_kgs_datao;
		m_kgs_ctrl &= ~(KGS_ST_ERR | KGS_ST_IBF);
		m_pic8259->ir7_w(ASSERT_LINE);
		break;

	case 2:
		data = m_kgs_ctrl;
		break;

	case 6:
		data = ioport("DSEL0")->read();
		break;

	case 7:
		data = ioport("DSEL1")->read();
		break;

	default:
		break;
	}

	if (0 && offset != 2 && offset != 5)
		logerror("%s: kgs %d == %02x '%c'\n", machine().describe_context(), offset, data,
				 (data > 0x1f && data < 0x7f) ? data : 0x20);

	return data;
}

void a7150_state::kgs_host_w(offs_t offset, uint8_t data)
{
	if (0) logerror("%s: kgs %d <- %02x '%c', ctrl %02x\n", machine().describe_context(), offset, data,
			 (data > 0x1f && data < 0x7f) ? data : 0x20, m_kgs_ctrl);

	switch (offset)
	{
	case 1:
		if (m_kgs_ctrl & KGS_ST_OBF)
		{
			m_kgs_ctrl |= KGS_ST_ERR;
		}
		else
		{
			m_kgs_datai = data;
			m_kgs_ctrl |= KGS_ST_OBF;
			m_pic8259->ir6_w(ASSERT_LINE);
		}
		break;

	case 3:
		m_kgs_ctrl |= KGS_ST_INT;
		m_pic8259->ir1_w(ASSERT_LINE);
		break;

	case 4:
		m_kgs_ctrl |= KGS_ST_ERR;
		break;

	case 5:
		m_kgs_msel = (data != 0);
		kgs_memory_remap();
		break;
	}
}

void a7150_state::kbd_put(uint8_t data)
{
	m_kgs_datai = data;
	m_kgs_ctrl |= KGS_ST_OBF | KGS_ST_INT;
	m_pic8259->ir1_w(ASSERT_LINE);
}

uint8_t a7150_state::a7150_kgs_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_kgs_ctrl;
		break;

	case 1:
		data = m_kgs_datai;
		m_kgs_ctrl &= ~KGS_ST_OBF;
		break;
	}

	if (offset)
	logerror("%s: KGS %d == %02x '%c'\n", machine().describe_context(), offset, data,
			 (data > 0x1f && data < 0x7f) ? data : 0x20);

	return data;
}

void a7150_state::a7150_kgs_w(offs_t offset, uint8_t data)
{
	logerror("%s: KGS %d <- %02x '%c', ctrl %02x\n", machine().describe_context(), offset, data,
			 (data > 0x1f && data < 0x7f) ? data : 0x20, m_kgs_ctrl);

	switch (offset)
	{
	case 0:
		m_kgs_ctrl &= ~(KGS_ST_ERR | KGS_ST_INT);
//      m_pic8259->ir1_w(CLEAR_LINE);
		break;

	case 1:
		if (m_kgs_ctrl & KGS_ST_IBF)
		{
			m_kgs_ctrl |= KGS_ST_ERR;
		}
		else
		{
			m_kgs_datao = data;
			m_kgs_ctrl |= KGS_ST_IBF;
			m_pic8259->ir7_w(CLEAR_LINE);
		}
		break;
	}
}


void a7150_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0xf7fff).ram();
	map(0xf8000, 0xfffff).rom().region("maincpu", 0);
}

void a7150_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x004a, 0x004a).w("isbc_215g", FUNC(isbc_215g_device::write)); // KES board
	map(0x00c0, 0x00c3).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x00c8, 0x00cf).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x00d0, 0x00d7).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x00d8, 0x00db).rw(m_uart8251, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x0200, 0x0203).rw(FUNC(a7150_state::a7150_kgs_r), FUNC(a7150_state::a7150_kgs_w)).umask16(0x00ff); // ABS/KGS board
	map(0x0300, 0x031f).unmaprw(); // ASP board #1
	map(0x0320, 0x033f).unmaprw(); // ASP board #2
}

void a7150_state::k7070_cpu_banked(address_map &map)
{
	map.unmap_value_high();
	// default map: IML=0, MSEL=0.  ROM + local RAM.
	map(0x00000, 0x01fff).rom().region("user2", 0);
	map(0x02000, 0x07fff).ram().share("kgs_ram1");
	map(0x08000, 0x0ffff).ram().share("kgs_ram2");
	// IML=1, MSEL=0.   local RAM only.
	map(0x10000, 0x11fff).ram().share("kgs_ram0");
	map(0x12000, 0x17fff).ram().share("kgs_ram1");
	map(0x18000, 0x1ffff).ram().share("kgs_ram2");
	// IML=0, MSEL=1.  ROM + local RAM.
	map(0x20000, 0x21fff).rom().region("user2", 0);
	map(0x22000, 0x27fff).ram().share("kgs_ram1");
	// IML=1, MSEL=1.   local RAM only.
	map(0x30000, 0x31fff).ram().share("kgs_ram0");
	map(0x32000, 0x37fff).ram().share("kgs_ram1");
	map(0x38000, 0x3ffff).ram().share("video_ram");
}

void a7150_state::k7070_cpu_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(m_video_bankdev, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
}


void a7150_state::k7070_cpu_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x0000, 0x0003).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0008, 0x000b).rw(Z80SIO_TAG, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x0010, 0x0017).rw(FUNC(a7150_state::kgs_host_r), FUNC(a7150_state::kgs_host_w)); // p. 11 of KGS-K7070.pdf

	map(0x0020, 0x0021).noprw(); // address register
	map(0x0022, 0x0022).noprw(); // function register (p. 6 of ABG-K7072.pdf)
	map(0x0023, 0x0023).noprw(); // split register
	map(0x0030, 0x003f).noprw(); // palette register
}

/* Input ports */
static INPUT_PORTS_START( a7150 )
	PORT_START("DSEL0")
	PORT_DIPNAME(0x01, 0x01, "Codepoint 0x24")
	PORT_DIPSETTING(0x00, "Currency sign" )
	PORT_DIPSETTING(0x01, "Dollar sign" )
	PORT_DIPNAME(0x02, 0x02, "Perform I/O test")
	PORT_DIPSETTING(0x00, DEF_STR(No) )
	PORT_DIPSETTING(0x02, DEF_STR(Yes) )
	PORT_DIPNAME(0x04, 0x00, "Perform VRAM test")
	PORT_DIPSETTING(0x00, DEF_STR(Yes) )
	PORT_DIPSETTING(0x04, DEF_STR(No) )

	PORT_START("DSEL1")
	PORT_DIPNAME(0x03, 0x02, "V.24 Parity")
	PORT_DIPSETTING(0x00, "No parity" )
	PORT_DIPSETTING(0x01, "Odd" )
	PORT_DIPSETTING(0x02, "No parity" )
	PORT_DIPSETTING(0x03, "Even" )
	PORT_DIPNAME(0x04, 0x04, "V.24 Character size")
	PORT_DIPSETTING(0x00, "7 bits")
	PORT_DIPSETTING(0x04, "8 bits")
	PORT_DIPNAME(0x38, 0x38, "V.24 Baud rate")
	PORT_DIPSETTING(0x38, "19200")
	PORT_DIPSETTING(0x30, "9600")
	PORT_DIPSETTING(0x28, "4800")
	PORT_DIPSETTING(0x20, "2400")
	PORT_DIPSETTING(0x18, "1200")
	PORT_DIPSETTING(0x10, "600")
	PORT_DIPSETTING(0x08, "300")
	PORT_DIPNAME(0x40, 0x40, "IFSS Parity")
	PORT_DIPSETTING(0x00, "Odd" )
	PORT_DIPSETTING(0x40, "Even" )
	PORT_DIPNAME(0x80, 0x80, "IFSS Baud rate")
	PORT_DIPSETTING(0x00, "9600")
	PORT_DIPSETTING(0x80, "Same as V.24")
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( kbd_rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_28800 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_28800 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
	DEVICE_INPUT_DEFAULTS( "FLOW_CONTROL", 0x01, 0x01 )
DEVICE_INPUT_DEFAULTS_END


void a7150_state::kgs_memory_remap()
{
	int bank = m_kgs_iml + m_kgs_msel + m_kgs_msel;

	if (0) logerror("%s: kgs memory: iml %d msel %d bank %d\n", machine().describe_context(), m_kgs_iml, m_kgs_msel, bank);

	m_video_bankdev->set_bank(bank);
}

void a7150_state::screen_eof(screen_device &screen, bool state)
{
	m_gfxcpu->set_input_line(INPUT_LINE_NMI, state);
}

void a7150_state::machine_reset()
{
	m_kgs_ctrl = 3;
	m_kgs_datao = m_kgs_datai = 0;
	m_kgs_iml = m_kgs_msel = 0;
	m_ifss_loopback = false;
	kgs_memory_remap();
}

void a7150_state::machine_start()
{
	save_item(NAME(m_kgs_msel));
	save_item(NAME(m_kgs_iml));
	save_item(NAME(m_kgs_datao));
	save_item(NAME(m_kgs_datai));
	save_item(NAME(m_kgs_ctrl));
	save_item(NAME(m_ifss_loopback));
}

static const z80_daisy_config k7070_daisy_chain[] =
{
	{ Z80SIO_TAG },
	{ Z80CTC_TAG },
	{ nullptr }
};

/*
 * K2771.30 ZRE - processor board
 * K3571    OPS - 256KB RAM board (x4)
 * K7070    KGS - graphics terminal, running firmware from A7100
 * K7072    ABG - dumb monochrome framebuffer
 * K5170    KES - media controller (compatible with iSBC 215A)
 *
 * (framebuffer and terminal should be slot devices.)
 */
void a7150_state::a7150(machine_config &config)
{
	I8086(config, m_maincpu, XTAL(9'832'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &a7150_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &a7150_state::io_map);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));
	m_maincpu->esc_opcode_handler().set("i8087", FUNC(i8087_device::insn_w));
	m_maincpu->esc_data_handler().set("i8087", FUNC(i8087_device::addr_w));

	i8087_device &i8087(I8087(config, "i8087", XTAL(9'832'000)/2));
	i8087.set_space_86(m_maincpu, AS_PROGRAM);
	i8087.irq().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	i8087.busy().set_inputline("maincpu", INPUT_LINE_TEST);

	PIC8259(config, m_pic8259, 0);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	// IFSP port on processor card
	i8255_device &ppi(I8255(config, "ppi8255"));
//  ppi.in_pa_callback().set("cent_status_in", FUNC(input_buffer_device::read));
//  ppi.out_pb_callback().set("cent_data_out", output_latch_device::write));
	ppi.out_pc_callback().set(FUNC(a7150_state::ppi_c_w));

	PIT8253(config, m_pit8253, 0);
	m_pit8253->set_clk<0>(14.7456_MHz_XTAL/4);
	m_pit8253->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir2_w));
	m_pit8253->set_clk<1>(14.7456_MHz_XTAL/4);
	m_pit8253->set_clk<2>(14.7456_MHz_XTAL/4);
	m_pit8253->out_handler<2>().set(FUNC(a7150_state::a7150_tmr2_w));

	INPUT_MERGER_ANY_HIGH(config, "uart_irq").output_handler().set(m_pic8259, FUNC(pic8259_device::ir4_w));

	I8251(config, m_uart8251, 0);
	m_uart8251->txd_handler().set(FUNC(a7150_state::ifss_write_txd));
	m_uart8251->dtr_handler().set(FUNC(a7150_state::ifss_write_dtr));
	m_uart8251->rts_handler().set(FUNC(a7150_state::ifss_loopback_w));
	m_uart8251->rxrdy_handler().set("uart_irq", FUNC(input_merger_device::in_w<0>));
	m_uart8251->txrdy_handler().set("uart_irq", FUNC(input_merger_device::in_w<1>));

	// IFSS port on processor card -- keyboard runs at 28800 8N2
	RS232_PORT(config, m_rs232, default_rs232_devices, "keyboard");
	m_rs232->rxd_handler().set(m_uart8251, FUNC(i8251_device::write_rxd));
	m_rs232->cts_handler().set(m_uart8251, FUNC(i8251_device::write_cts));
	m_rs232->dsr_handler().set(m_uart8251, FUNC(i8251_device::write_dsr));
	m_rs232->set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(kbd_rs232_defaults));

	ISBC_215G(config, "isbc_215g", 0, 0x4a, m_maincpu).irq_callback().set(m_pic8259, FUNC(pic8259_device::ir5_w));

	// KGS K7070 graphics terminal controlling ABG K7072 framebuffer
	Z80(config, m_gfxcpu, XTAL(16'000'000)/4);
	m_gfxcpu->set_addrmap(AS_PROGRAM, &a7150_state::k7070_cpu_mem);
	m_gfxcpu->set_addrmap(AS_IO, &a7150_state::k7070_cpu_io);
	m_gfxcpu->set_daisy_config(k7070_daisy_chain);

	ADDRESS_MAP_BANK(config, m_video_bankdev, 0);
	m_video_bankdev->set_map(&a7150_state::k7070_cpu_banked);
	m_video_bankdev->set_endianness(ENDIANNESS_BIG);
	m_video_bankdev->set_addr_width(18);
	m_video_bankdev->set_data_width(8);
	m_video_bankdev->set_stride(0x10000);

	Z80CTC(config, m_ctc, 16_MHz_XTAL/3);
	m_ctc->intr_callback().set_inputline(m_gfxcpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(1230750);
	m_ctc->set_clk<1>(1230750);
	m_ctc->set_clk<2>(1230750);
	m_ctc->set_clk<3>(1230750);
	m_ctc->zc_callback<0>().set(Z80SIO_TAG, FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<0>().append(Z80SIO_TAG, FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<1>().set(Z80SIO_TAG, FUNC(z80sio_device::rxtxcb_w));

	z80sio_device& sio(Z80SIO(config, Z80SIO_TAG, XTAL(16'000'000)/4));
	sio.out_int_callback().set_inputline(m_gfxcpu, INPUT_LINE_IRQ0);
	sio.out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	sio.out_txdb_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	sio.out_dtrb_callback().set(FUNC(a7150_state::kgs_iml_w));
	//sio.out_rtsb_callback().set(FUNC(a7150_state::kgs_ifss_loopback_w));

	// V.24 port (graphics tablet)
	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, "loopback"));
	rs232a.rxd_handler().set(Z80SIO_TAG, FUNC(z80sio_device::rxa_w));
	rs232a.dcd_handler().set(Z80SIO_TAG, FUNC(z80sio_device::dcda_w));
	rs232a.cts_handler().set(Z80SIO_TAG, FUNC(z80sio_device::ctsa_w));

	// IFSS (current loop) port (keyboard)
	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, "loopback"));
	rs232b.rxd_handler().set(Z80SIO_TAG, FUNC(z80sio_device::rxb_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(XTAL(16'000'000), 737,0,640, 431,0,400);
	screen.set_screen_update(FUNC(a7150_state::screen_update_k7072));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, palette_device::MONOCHROME);
}

/* ROM definition */
ROM_START( a7150 )
	ROM_REGION16_LE( 0x8000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("2.3")

	// A7100
	ROM_SYSTEM_BIOS(0, "1.1", "ACT 1.1")
	ROMX_LOAD("q259.bin", 0x4001, 0x2000, CRC(fb5b547b) SHA1(1d17fcededa91cad321a7b237a46a308142d902b), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("q260.bin", 0x0001, 0x2000, CRC(b51f8ed6) SHA1(9aa6291bf8ab49a343741717366992649e2957b3), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("q261.bin", 0x4000, 0x2000, CRC(43c08ea3) SHA1(ea697180b415b71d834968be84431a6efe9490c2), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("q262.bin", 0x0000, 0x2000, CRC(9df1c396) SHA1(a627889e1162e5b2fe95804de52bb78e41aaf7cc), ROM_BIOS(0) | ROM_SKIP(1))

	// A7150
	ROM_SYSTEM_BIOS(1, "2.1", "ACT 2.1")
	ROMX_LOAD("265.bin",  0x4001, 0x2000, CRC(a5fb5f35) SHA1(9d9501441cad0ef724dec7b5ffb52b17a678a9f8), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("266.bin",  0x0001, 0x2000, CRC(f5898eb7) SHA1(af3fd82813fbea7883dea4d7e23a9b5e5b2b844a), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("267.bin",  0x4000, 0x2000, CRC(c1873a01) SHA1(77f15cc217cd854732fbe33d395e1ea9867fedd7), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("268.bin",  0x0000, 0x2000, CRC(e3f09213) SHA1(1e2d69061f8e84697440b219181e0b870fe21835), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(2, "2.2", "ACT 2.2")
	ROMX_LOAD("269.bin",  0x4001, 0x2000, CRC(f137f94b) SHA1(7cb79f332db48cb66dae04c1ce1bdd169a6ab561), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("270.bin",  0x0001, 0x2000, CRC(1ea44a33) SHA1(f5708d1f6a9dc109979a9a91a80f2a4e4956d1eb), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("271.bin",  0x4000, 0x2000, CRC(de2222c9) SHA1(e02225c93b49f0380dfb2d996b63370141359199), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("272.bin",  0x0000, 0x2000, CRC(5001c528) SHA1(ce67c35326fbfd17f086a37ffe81b79aefaef0cb), ROM_BIOS(2) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(3, "2.3", "ACT 2.3")
	ROMX_LOAD("273.rom",  0x4001, 0x2000, CRC(67ca9b78) SHA1(bcb6221f6df28b24b602846b149ac12e93b5e356), ROM_BIOS(3) | ROM_SKIP(1))
	ROMX_LOAD("274.rom",  0x0001, 0x2000, CRC(6fa68834) SHA1(49abe48bbb5ae151f977a9c63b27336c15e8a08d), ROM_BIOS(3) | ROM_SKIP(1))
	ROMX_LOAD("275.rom",  0x4000, 0x2000, CRC(0da54426) SHA1(7492caff98b1d1a896c5964942b17beadf996b60), ROM_BIOS(3) | ROM_SKIP(1))
	ROMX_LOAD("276.rom",  0x0000, 0x2000, CRC(5924192a) SHA1(eb494d9f96a0b3ea69f4b9cb2b7add66a8c16946), ROM_BIOS(3) | ROM_SKIP(1))

	ROM_REGION( 0x2000, "user2", ROMREGION_ERASEFF )
	// ROM from A7100
	ROM_LOAD( "kgs7070-152.bin", 0x0000, 0x2000, CRC(403f4235) SHA1(d07ccd40f8b600651d513f588bcf1ea4f15ed094))
//  ROM_LOAD( "kgs7070-153.rom", 0x0000, 0x2000, CRC(a72fe820) SHA1(4b77ab2b59ea8c3632986847ff359df26b16196b))
//  ROM_LOAD( "kgs7070-154.rom", 0x0000, 0x2000, CRC(2995ade0) SHA1(62516f2e1cb62698445f80fd823d39a1a78a7807))
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY         FULLNAME  FLAGS
COMP( 1986, a7150, 0,      0,      a7150,   a7150, a7150_state, empty_init, "VEB Robotron", "A7150",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
