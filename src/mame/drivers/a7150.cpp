// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Sergey Svishchev
/***************************************************************************

    Robotron A7150

    04/10/2009 Skeleton driver.

    http://www.robotrontechnik.de/index.htm?/html/computer/a7150.htm

    http://www.tiffe.de/Robotron/MMS16/
    - Confidence test is documented in A7150_Rechner...pdf, pp. 112-119
    - Internal test of KGS -- in KGS-K7070.pdf, pp. 19-23

    To do:
    - MMS16 (Multibus clone) and slot devices
    - native keyboard
    - A7100 model

****************************************************************************/

#include "emu.h"

#include "cpu/i86/i86.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/bankdev.h"

#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"

#include "bus/rs232/rs232.h"
#include "machine/isbc_215g.h"
#include "machine/keyboard.h"

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
		, m_video_ram(*this, "video_ram")
		, m_video_bankdev(*this, "video_bankdev")
		, m_palette(*this, "palette")
	{ }

	virtual void machine_reset() override;
	virtual void machine_start() override;

	DECLARE_READ8_MEMBER(a7150_kgs_r);
	DECLARE_WRITE8_MEMBER(a7150_kgs_w);

	DECLARE_WRITE_LINE_MEMBER(a7150_tmr2_w);
	DECLARE_WRITE8_MEMBER(ppi_c_w);

	DECLARE_READ8_MEMBER(kgs_host_r);
	DECLARE_WRITE8_MEMBER(kgs_host_w);
	DECLARE_WRITE_LINE_MEMBER(kgs_iml_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	void kgs_memory_remap();

	bool m_kgs_msel, m_kgs_iml;
	uint8_t m_kgs_datao, m_kgs_datai, m_kgs_ctrl;

	uint32_t screen_update_k7072(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);

	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_uart8251;
	required_device<pit8253_device> m_pit8253;
	required_device<pic8259_device> m_pic8259;

	required_device<z80_device> m_gfxcpu;
	required_device<z80ctc_device> m_ctc;
	required_shared_ptr<uint8_t> m_video_ram;
	required_device<address_map_bank_device> m_video_bankdev;
	required_device<palette_device> m_palette;
};


uint32_t a7150_state::screen_update_k7072(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y, x, b;
	int addr = 0;

	for (y = 0; y < 400; y++)
	{
		int horpos = 0;
		for (x = 0; x < 80; x++)
		{
			uint8_t code = m_video_ram[addr++];
			for (b = 0; b < 8; b++)
			{
				bitmap.pix16(y, horpos++) = ((code >> (7 - b)) & 0x01) ? 1 : 0;
			}
		}
	}

	return 0;
}

WRITE_LINE_MEMBER(a7150_state::kgs_iml_w)
{
	m_kgs_iml = !state;
	kgs_memory_remap();
}

WRITE_LINE_MEMBER(a7150_state::a7150_tmr2_w)
{
	m_uart8251->write_rxc(state);
	m_uart8251->write_txc(state);
}

WRITE8_MEMBER(a7150_state::ppi_c_w)
{
	// b0 -- INTR(B)
	// b1 -- /OBF(B)
	// m_centronics->write_ack(BIT(data, 2));
	// m_centronics->write_strobe(BIT(data, 3));
	// b4 -- serial loopback?
	// b6
	// b7
}

#define	KGS_ST_OBF	0x01
#define	KGS_ST_IBF	0x02
#define	KGS_ST_INT	0x04
#define	KGS_ST_ERR	0x80

READ8_MEMBER(a7150_state::kgs_host_r)
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

WRITE8_MEMBER(a7150_state::kgs_host_w)
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

WRITE8_MEMBER(a7150_state::kbd_put)
{
	m_kgs_datai = data;
	m_kgs_ctrl |= KGS_ST_OBF | KGS_ST_INT;
	m_pic8259->ir1_w(ASSERT_LINE);
}

READ8_MEMBER(a7150_state::a7150_kgs_r)
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

WRITE8_MEMBER(a7150_state::a7150_kgs_w)
{
	logerror("%s: KGS %d <- %02x '%c', ctrl %02x\n", machine().describe_context(), offset, data,
			 (data > 0x1f && data < 0x7f) ? data : 0x20, m_kgs_ctrl);

	switch (offset)
	{
	case 0:
		m_kgs_ctrl &= ~(KGS_ST_ERR | KGS_ST_INT);
//		m_pic8259->ir1_w(CLEAR_LINE);
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
		}
		break;
	}
}


static ADDRESS_MAP_START(a7150_mem, AS_PROGRAM, 16, a7150_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0xf7fff) AM_RAM
	AM_RANGE(0xf8000, 0xfffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(a7150_io, AS_IO, 16, a7150_state)
    ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x004a, 0x004b) AM_DEVWRITE8("isbc_215g", isbc_215g_device, write, 0x00ff) // KES board
	AM_RANGE(0x00c0, 0x00c3) AM_DEVREADWRITE8("pic8259", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x00c8, 0x00cf) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0x00ff)
	AM_RANGE(0x00d0, 0x00d7) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0x00ff)
	AM_RANGE(0x00d8, 0x00d9) AM_DEVREADWRITE8("uart8251", i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0x00da, 0x00db) AM_DEVREADWRITE8("uart8251", i8251_device, status_r, control_w, 0x00ff)
	AM_RANGE(0x0200, 0x0203) AM_READWRITE8(a7150_kgs_r, a7150_kgs_w, 0x00ff) // ABS/KGS board
	AM_RANGE(0x0300, 0x031f) AM_UNMAP // ASP board #1
	AM_RANGE(0x0320, 0x033f) AM_UNMAP // ASP board #2
ADDRESS_MAP_END

static ADDRESS_MAP_START(k7070_cpu_banked, AS_PROGRAM, 8, a7150_state)
	ADDRESS_MAP_UNMAP_HIGH
	// default map: IML=0, MSEL=0.  ROM + local RAM.
	AM_RANGE(0x00000, 0x01fff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0x02000, 0x07fff) AM_RAMBANK("kgs_ram1")
	AM_RANGE(0x08000, 0x0ffff) AM_RAMBANK("kgs_ram2")
	// IML=1, MSEL=0.   local RAM only.
	AM_RANGE(0x10000, 0x11fff) AM_RAMBANK("kgs_ram0")
	AM_RANGE(0x12000, 0x17fff) AM_RAMBANK("kgs_ram1")
	AM_RANGE(0x18000, 0x1ffff) AM_RAMBANK("kgs_ram2")
	// IML=0, MSEL=1.  ROM + local RAM.
	AM_RANGE(0x20000, 0x21fff) AM_ROM AM_REGION("user2", 0)
	AM_RANGE(0x22000, 0x27fff) AM_RAMBANK("kgs_ram1")
	// IML=1, MSEL=1.   local RAM only.
	AM_RANGE(0x30000, 0x31fff) AM_RAMBANK("kgs_ram0")
	AM_RANGE(0x32000, 0x37fff) AM_RAMBANK("kgs_ram1")
	AM_RANGE(0x38000, 0x3ffff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(k7070_cpu_mem, AS_PROGRAM, 8, a7150_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_DEVREADWRITE("video_bankdev", address_map_bank_device, read8, write8)
ADDRESS_MAP_END


static ADDRESS_MAP_START(k7070_cpu_io, AS_IO, 8, a7150_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0x0003) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0x0008, 0x000b) AM_DEVREADWRITE(Z80SIO_TAG, z80sio_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x0010, 0x0017) AM_READWRITE(kgs_host_r, kgs_host_w) // p. 11 of KGS-K7070.pdf

	AM_RANGE(0x0020, 0x0021) AM_NOP // address register
	AM_RANGE(0x0022, 0x0022) AM_NOP // function register (p. 6 of ABG-K7072.pdf)
	AM_RANGE(0x0023, 0x0023) AM_NOP // split register
	AM_RANGE(0x0030, 0x003f) AM_NOP // palette register
ADDRESS_MAP_END

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


void a7150_state::kgs_memory_remap()
{
	int bank = m_kgs_iml + m_kgs_msel + m_kgs_msel;

	logerror("%s: kgs memory: iml %d msel %d bank %d\n", machine().describe_context(), m_kgs_iml, m_kgs_msel, bank);

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
	kgs_memory_remap();
}

void a7150_state::machine_start()
{
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
static MACHINE_CONFIG_START( a7150, a7150_state )
	MCFG_CPU_ADD("maincpu", I8086, XTAL_9_832MHz/2)
	MCFG_CPU_PROGRAM_MAP(a7150_mem)
	MCFG_CPU_IO_MAP(a7150_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

	MCFG_PIC8259_ADD("pic8259", INPUTLINE("maincpu", 0), VCC, NOOP)

	// IFSP port on processor card
	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
//	MCFG_I8255_IN_PORTA_CB(DEVREAD8("cent_status_in", input_buffer_device, read))
//	MCFG_I8255_OUT_PORTB_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(a7150_state, ppi_c_w))

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_14_7456MHz/4)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir2_w))
	MCFG_PIT8253_CLK1(XTAL_14_7456MHz/4)
	MCFG_PIT8253_CLK2(XTAL_14_7456MHz/4)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(a7150_state, a7150_tmr2_w))

	MCFG_DEVICE_ADD("uart8251", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir4_w))
	MCFG_I8251_TXRDY_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir4_w))

	// IFSS port on processor card
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "loopback") // allows ACT to pass
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart8251", i8251_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart8251", i8251_device, write_cts))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart8251", i8251_device, write_dsr))

	MCFG_ISBC_215_ADD("isbc_215g", 0x4a, "maincpu")
	MCFG_ISBC_215_IRQ(DEVWRITELINE("pic8259", pic8259_device, ir5_w))

	// KGS K7070 graphics terminal controlling ABG K7072 framebuffer
	MCFG_CPU_ADD("gfxcpu", Z80, XTAL_16MHz/4)
	MCFG_CPU_PROGRAM_MAP(k7070_cpu_mem)
	MCFG_CPU_IO_MAP(k7070_cpu_io)
	MCFG_Z80_DAISY_CHAIN(k7070_daisy_chain)

	MCFG_DEVICE_ADD("video_bankdev", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(k7070_cpu_banked)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(18)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x10000)

	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_16MHz/3)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("gfxcpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE(Z80SIO_TAG, z80sio_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE(Z80SIO_TAG, z80sio_device, txca_w))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE(Z80SIO_TAG, z80sio_device, rxtxcb_w))

	MCFG_Z80SIO_ADD(Z80SIO_TAG, 4800, 0, 0, 0, 0)
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("gfxcpu", INPUT_LINE_IRQ0))
	MCFG_Z80SIO_OUT_TXDA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))
	MCFG_Z80SIO_OUT_TXDB_CB(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRB_CB(WRITELINE(a7150_state, kgs_iml_w))
//	MCFG_Z80SIO_OUT_RTSB_CB(WRITELINE(a7150_state, kgs_ifss_loopback_w))

	// V.24 port (graphics tablet)
	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio_device, ctsa_w))

	// IFSS (current loop) port (keyboard)
	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(Z80SIO_TAG, z80sio_device, rxb_w))

	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_RAW_PARAMS( XTAL_16MHz, 737,0,640, 431,0,400 )
	MCFG_SCREEN_UPDATE_DRIVER(a7150_state, screen_update_k7072)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_MONOCHROME("palette")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( a7150 )
	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "a7150.rom", 0x0000, 0x8000, CRC(57855abd) SHA1(b58f1363623d2c3ff1221e449529ecaa22573bff))

	ROM_REGION( 0x10000, "user2", ROMREGION_ERASEFF )
	// ROM from A7100
	ROM_LOAD( "KGS7070-152.bin", 0x0000, 0x2000, CRC(403f4235) SHA1(d07ccd40f8b600651d513f588bcf1ea4f15ed094))
//	ROM_LOAD( "KGS7070-153.rom", 0x0000, 0x2000, CRC(a72fe820) SHA1(4b77ab2b59ea8c3632986847ff359df26b16196b))
//	ROM_LOAD( "KGS7070-154.rom", 0x0000, 0x2000, CRC(2995ade0) SHA1(62516f2e1cb62698445f80fd823d39a1a78a7807))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY           FULLNAME       FLAGS */
COMP( 1986, a7150,  0,      0,       a7150,     a7150, driver_device,    0,     "VEB Robotron",   "A7150", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
