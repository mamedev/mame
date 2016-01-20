// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/i8251.h"
#include "machine/i8257.h"
#include "machine/upd765.h"
#include "video/mc6845.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/keyboard.h"

class peoplepc_state : public driver_device
{
public:
	peoplepc_state(const machine_config &mconfig, device_type type, std::string tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_pic_1(*this, "pic8259_1"),
		m_8251key(*this, "i8251_0"),
		m_8251ser(*this, "i8251_1"),
		m_fdc(*this, "upd765"),
		m_flop0(*this, "upd765:0"),
		m_flop1(*this, "upd765:1"),
		m_dmac(*this, "i8257"),
		m_gfxdecode(*this, "gfxdecode"),
		m_gvram(*this, "gvram"),
		m_cvram(*this, "cvram"),
		m_charram(4*1024)
	{ }
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<pic8259_device> m_pic_1;
	required_device<i8251_device> m_8251key;
	required_device<i8251_device> m_8251ser;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_flop0;
	required_device<floppy_connector> m_flop1;
	required_device<i8257_device> m_dmac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<UINT16> m_gvram;
	required_shared_ptr<UINT16> m_cvram;
	dynamic_buffer m_charram;

	MC6845_UPDATE_ROW(update_row);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE16_MEMBER(charram_w);
	DECLARE_WRITE_LINE_MEMBER(tty_clock_tick_w);
	DECLARE_WRITE_LINE_MEMBER(kbd_clock_tick_w);
	DECLARE_WRITE8_MEMBER(dmapg_w);
	DECLARE_WRITE_LINE_MEMBER(tc_w);
	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_FLOPPY_FORMATS( floppy_formats );
	int floppy_load(floppy_image_device *dev);
	void floppy_unload(floppy_image_device *dev);

	UINT8 m_dma0pg;
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};

static const gfx_layout peoplepc_charlayout =
{
	8, 19,                   /* 8 x 19 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0},
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8, 16*8, 17*8, 18*8 },
	8*32
};

MC6845_UPDATE_ROW(peoplepc_state::update_row)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i, j;

	for(i = 0; i < x_count; i++)
	{
		if(0)
		{
			UINT16 offset = ((ma | (ra << 1)) << 4) + i;
			UINT8 data = m_gvram[offset] >> (offset & 1 ? 8 : 0);

			for(j = 8; j >= 0; j--)
				bitmap.pix32(y, (i * 8) + j) = palette[( data & 1 << j ) ? 1 : 0];
		}
		else
		{
			UINT8 data = m_charram[(m_cvram[(ma + i) & 0x3fff] & 0x7f) * 32 + ra];
			for(j = 0; j < 8; j++)
				bitmap.pix32(y, (i * 8) + j) = palette[(data & (1 << j)) ? 1 : 0];
		}
	}
}

READ8_MEMBER(peoplepc_state::get_slave_ack)
{
	if (offset == 7)
		return m_pic_1->acknowledge();

	return 0x00;
}

WRITE16_MEMBER(peoplepc_state::charram_w)
{
	m_charram[offset] = data;
	m_gfxdecode->gfx(0)->mark_dirty(offset/16);
}

WRITE_LINE_MEMBER(peoplepc_state::tty_clock_tick_w)
{
	m_8251ser->write_txc(state);
	m_8251ser->write_rxc(state);
}

WRITE_LINE_MEMBER(peoplepc_state::kbd_clock_tick_w)
{
	m_8251key->write_txc(state);
	m_8251key->write_rxc(state);
}

WRITE8_MEMBER(peoplepc_state::dmapg_w)
{
	m_dma0pg = data;
}

WRITE_LINE_MEMBER(peoplepc_state::tc_w)
{
	m_fdc->tc_w(state);
}

WRITE_LINE_MEMBER(peoplepc_state::hrq_w)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dmac->hlda_w(state);
}

READ8_MEMBER(peoplepc_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset | (m_dma0pg << 16));
}

WRITE8_MEMBER(peoplepc_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset | (m_dma0pg << 16), data);
}

int peoplepc_state::floppy_load(floppy_image_device *dev)
{
	dev->mon_w(0);
	return IMAGE_INIT_PASS;
}

void peoplepc_state::floppy_unload(floppy_image_device *dev)
{
	dev->mon_w(1);
}

void peoplepc_state::machine_reset()
{
	m_flop0->get_device()->mon_w(!m_flop0->get_device()->exists());
	m_flop1->get_device()->mon_w(!m_flop1->get_device()->exists());
}

void peoplepc_state::machine_start()
{
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(machine().device<palette_device>("palette"), peoplepc_charlayout, &m_charram[0], 0, 1, 0));
	m_dma0pg = 0;

	// FIXME: cheat as there no docs about how or obvious ports that set to control the motor
	m_flop0->get_device()->setup_load_cb(floppy_image_device::load_cb(FUNC(peoplepc_state::floppy_load), this));
	m_flop0->get_device()->setup_unload_cb(floppy_image_device::unload_cb(FUNC(peoplepc_state::floppy_unload), this));
	m_flop1->get_device()->setup_load_cb(floppy_image_device::load_cb(FUNC(peoplepc_state::floppy_load), this));
	m_flop1->get_device()->setup_unload_cb(floppy_image_device::unload_cb(FUNC(peoplepc_state::floppy_unload), this));
}

static ADDRESS_MAP_START( peoplepc_map, AS_PROGRAM, 16, peoplepc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x7ffff) AM_RAM
	AM_RANGE(0xc0000, 0xdffff) AM_RAM AM_SHARE("gvram")
	AM_RANGE(0xe0000, 0xe3fff) AM_RAM AM_SHARE("cvram")
	AM_RANGE(0xe4000, 0xe5fff) AM_WRITE(charram_w)
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(peoplepc_io, AS_IO, 16, peoplepc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0014, 0x0017) AM_DEVREADWRITE8("pic8259_1", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x0018, 0x001b) AM_DEVREADWRITE8("pic8259_0", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x0020, 0x0031) AM_DEVREADWRITE8("i8257", i8257_device, read, write, 0x00ff)
	AM_RANGE(0x0040, 0x0047) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write, 0x00ff)
	AM_RANGE(0x0048, 0x004f) AM_DEVREADWRITE8("pit8253", pit8253_device, read, write, 0x00ff)
	AM_RANGE(0x0054, 0x0055) AM_DEVREADWRITE8("i8251_0", i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0x0056, 0x0057) AM_DEVREADWRITE8("i8251_0", i8251_device, status_r, control_w, 0x00ff)
	AM_RANGE(0x005c, 0x005d) AM_DEVREADWRITE8("i8251_1", i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0x005e, 0x005f) AM_DEVREADWRITE8("i8251_1", i8251_device, status_r, control_w, 0x00ff)
	AM_RANGE(0x0064, 0x0067) AM_DEVICE8("upd765", upd765a_device, map, 0x00ff)
	AM_RANGE(0x006c, 0x006d) AM_DEVWRITE8("h46505", mc6845_device, address_w, 0x00ff)
	AM_RANGE(0x006e, 0x006f) AM_DEVREADWRITE8("h46505", mc6845_device, register_r, register_w, 0x00ff)
	AM_RANGE(0x0070, 0x0071) AM_WRITE8(dmapg_w, 0x00ff)
ADDRESS_MAP_END

static SLOT_INTERFACE_START( peoplepc_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( peoplepc_state::floppy_formats )
	FLOPPY_IMD_FORMAT
FLOPPY_FORMATS_END

SLOT_INTERFACE_START( peoplepc_keyboard_devices )
	SLOT_INTERFACE("keyboard", SERIAL_KEYBOARD)
SLOT_INTERFACE_END

static DEVICE_INPUT_DEFAULTS_START(keyboard)
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_START( olypeopl, peoplepc_state)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, XTAL_14_7456MHz/3)
	MCFG_CPU_PROGRAM_MAP(peoplepc_map)
	MCFG_CPU_IO_MAP(peoplepc_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_0", pic8259_device, inta_cb)

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_14_7456MHz/6)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(peoplepc_state, kbd_clock_tick_w))
	MCFG_PIT8253_CLK1(XTAL_14_7456MHz/6)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(peoplepc_state, tty_clock_tick_w))
	MCFG_PIT8253_CLK2(XTAL_14_7456MHz/6)
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("pic8259_0", pic8259_device, ir0_w))

	MCFG_PIC8259_ADD("pic8259_0", INPUTLINE("maincpu", 0), VCC, READ8(peoplepc_state, get_slave_ack))
	MCFG_PIC8259_ADD("pic8259_1", DEVWRITELINE("pic8259_0", pic8259_device, ir7_w), GND, NULL)

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_22MHz,640,0,640,475,0,475)
	MCFG_SCREEN_UPDATE_DEVICE( "h46505", mc6845_device, screen_update )

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)
	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_MC6845_ADD("h46505", H46505, "screen", XTAL_22MHz/8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(peoplepc_state, update_row)

	MCFG_DEVICE_ADD("i8257", I8257, XTAL_14_7456MHz/3)
	MCFG_I8257_OUT_HRQ_CB(WRITELINE(peoplepc_state, hrq_w))
	MCFG_I8257_OUT_TC_CB(WRITELINE(peoplepc_state, tc_w))
	MCFG_I8257_IN_MEMR_CB(READ8(peoplepc_state, memory_read_byte))
	MCFG_I8257_OUT_MEMW_CB(WRITE8(peoplepc_state, memory_write_byte))
	MCFG_I8257_IN_IOR_0_CB(DEVREAD8("upd765", upd765a_device, mdma_r))
	MCFG_I8257_OUT_IOW_0_CB(DEVWRITE8("upd765", upd765a_device, mdma_w))

	MCFG_UPD765A_ADD("upd765", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE("pic8259_0", pic8259_device, ir2_w))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE("i8257", i8257_device, dreq0_w))
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", peoplepc_floppies, "525qd", peoplepc_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", peoplepc_floppies, "525qd", peoplepc_state::floppy_formats)

	MCFG_DEVICE_ADD("i8251_0", I8251, 0)
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("pic8259_1", pic8259_device, ir1_w))
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("kbd", rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD("kbd", peoplepc_keyboard_devices, "keyboard")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("i8251_0", i8251_device, write_rxd))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("keyboard", keyboard)

	MCFG_DEVICE_ADD("i8251_1", I8251, 0)
MACHINE_CONFIG_END

ROM_START( olypeopl )
	ROM_REGION(0x2000,"maincpu", 0)
	ROM_SYSTEM_BIOS(0, "hd",  "HD ROM")
	ROMX_LOAD( "u01271c0.bin", 0x00000, 0x1000, CRC(8e0ef114) SHA1(774bab0a3e29853e9f6b951cf73082063ea61e6d), ROM_SKIP(1)|ROM_BIOS(1))
	ROMX_LOAD( "u01271d0.bin", 0x00001, 0x1000, CRC(e2419bf9) SHA1(d88381f8709c91e2adba08f378e29bd0d19ee5ae), ROM_SKIP(1)|ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "2fd",  "2 FD ROM")
	ROMX_LOAD( "u01277f3.bin", 0x00000, 0x1000, CRC(428ff135) SHA1(ec11f0e43455570c40f5dc4b84f8420da5939368), ROM_SKIP(1)|ROM_BIOS(2))
	ROMX_LOAD( "u01277g3.bin", 0x00001, 0x1000, CRC(3295691c) SHA1(7d7ade62117d11656b8dd86cf0703127616d55bc), ROM_SKIP(1)|ROM_BIOS(2))
ROM_END

COMP( 198?, olypeopl,   0,    0,         olypeopl,      0, driver_device,      0,      "Olympia", "People PC", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
