// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/mcs48/mcs48.h"
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
#include "screen.h"

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
		m_chrrom(*this, "char"),
		m_vram(*this, "vram")
	{ }

	void duet16(machine_config &config);

private:
	DECLARE_READ8_MEMBER(pic_r);
	DECLARE_WRITE8_MEMBER(pic_w);
	DECLARE_READ8_MEMBER(dma_mem_r);
	DECLARE_WRITE8_MEMBER(dma_mem_w);
	DECLARE_READ8_MEMBER(dmapg_r);
	DECLARE_WRITE8_MEMBER(dmapg_w);
	DECLARE_WRITE8_MEMBER(fdcctrl_w);
	DECLARE_WRITE8_MEMBER(pal_w);
	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	void duet16_io(address_map &map);
	void duet16_mem(address_map &map);
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic;
	required_device<upd765a_device> m_fdc;
	required_device<am9517a_device> m_dmac;
	required_device_array<floppy_connector, 2> m_fd;
	required_device<palette_device> m_pal;
	required_memory_region m_chrrom;
	required_shared_ptr<u16> m_vram;
	u8 m_dmapg;
};

READ8_MEMBER(duet16_state::pic_r)
{
	return m_pic->read(space, offset ^ 1, mem_mask);
}

WRITE8_MEMBER(duet16_state::pic_w)
{
	m_pic->write(space, offset ^ 1, data);
}

WRITE8_MEMBER(duet16_state::fdcctrl_w)
{
	floppy_image_device *f = m_fd[BIT(data, 2) ? 1 : 0]->get_device();
	m_fdc->set_floppy(f);

	m_fd[0]->get_device()->mon_w(!BIT(data, 0));
	m_fd[1]->get_device()->mon_w(!BIT(data, 0));
	if(!BIT(data, 1))
		m_fdc->soft_reset();

	// TODO: bit 3 = LSPD
}

READ8_MEMBER(duet16_state::dma_mem_r)
{
	return m_maincpu->space(AS_PROGRAM).read_byte((m_dmapg << 16) | offset);
}

WRITE8_MEMBER(duet16_state::dma_mem_w)
{
	m_maincpu->space(AS_PROGRAM).write_byte((m_dmapg << 16) | offset, data);
}

READ8_MEMBER(duet16_state::dmapg_r)
{
	return m_dmapg;
}

WRITE8_MEMBER(duet16_state::dmapg_w)
{
	m_dmapg = data & 0xf;
}

WRITE_LINE_MEMBER(duet16_state::hrq_w)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dmac->hack_w(state);
}

ADDRESS_MAP_START(duet16_state::duet16_mem)
	AM_RANGE(0x00000, 0x9ffff) AM_RAM
	AM_RANGE(0xb8000, 0xbffff) AM_RAM
	AM_RANGE(0xc0000, 0xc0fff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xf8000, 0xf801f) AM_READWRITE8(dmapg_r, dmapg_w, 0x00ff)
	AM_RANGE(0xf8000, 0xf801f) AM_DEVREADWRITE8("dmac", am9517a_device, read, write, 0xff00)
	AM_RANGE(0xf8020, 0xf8023) AM_READWRITE8(pic_r, pic_w, 0x00ff)
	AM_RANGE(0xf8040, 0xf804f) AM_DEVREADWRITE8("itm", ptm6840_device, read, write, 0x00ff)
	AM_RANGE(0xf8060, 0xf8067) AM_DEVREADWRITE8("bgpit", pit8253_device, read, write, 0x00ff)
	AM_RANGE(0xf8080, 0xf8087) AM_DEVREADWRITE8("sio", upd7201_new_device, ba_cd_r, ba_cd_w, 0x00ff)
	AM_RANGE(0xf80a0, 0xf80a1) AM_DEVREADWRITE8("kbusart", i8251_device, data_r, data_w, 0x00ff)
	AM_RANGE(0xf80a2, 0xf80a3) AM_DEVREADWRITE8("kbusart", i8251_device, status_r, control_w, 0x00ff)
	AM_RANGE(0xf80c0, 0xf80c1) AM_DEVREADWRITE8("crtc", h46505_device, status_r, address_w, 0x00ff)
	AM_RANGE(0xf80c2, 0xf80c3) AM_DEVREADWRITE8("crtc", h46505_device, register_r, register_w, 0x00ff)
	AM_RANGE(0xf8100, 0xf8103) AM_DEVICE8("fdc", upd765a_device, map, 0x00ff)
	AM_RANGE(0xf8160, 0xf818f) AM_WRITE8(pal_w, 0xffff)
	AM_RANGE(0xf8220, 0xf8221) AM_WRITE8(fdcctrl_w, 0x00ff)
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("rom", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START(duet16_state::duet16_io)
ADDRESS_MAP_END

WRITE8_MEMBER(duet16_state::pal_w)
{
	int entry = (BIT(offset, 0) ? 2 : 0) | (BIT(offset, 5) ? 0 : 4);
	m_pal->set_pen_color(entry, pal1bit(BIT(data, 1)), pal1bit(BIT(data, 2)), pal1bit(BIT(data, 0)));
	m_pal->set_pen_color(entry + 1, pal1bit(BIT(data, 5)), pal1bit(BIT(data, 6)), pal1bit(BIT(data, 4)));
}

MC6845_UPDATE_ROW(duet16_state::crtc_update_row)
{
	const rgb_t bg = 0;
	u32 *p = &bitmap.pix32(y);

	for(int i = 0; i < x_count; i++)
	{
		u16 offset = (ma + i) & 0x1fff;
		u8 chr = m_vram[offset] & 0xff;
		u8 attr = m_vram[offset] >> 8;
		u8 data = m_chrrom->base()[(chr * 16) + ra];
		rgb_t fg = m_pal->pen_color(attr & 7);

		*p = (data & 0x80) ? fg : bg; p++;
		*p = (data & 0x40) ? fg : bg; p++;
		*p = (data & 0x20) ? fg : bg; p++;
		*p = (data & 0x10) ? fg : bg; p++;
		*p = (data & 0x08) ? fg : bg; p++;
		*p = (data & 0x04) ? fg : bg; p++;
		*p = (data & 0x02) ? fg : bg; p++;
		*p = (data & 0x01) ? fg : bg; p++;
	}
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

static GFXDECODE_START(duet16)
	GFXDECODE_ENTRY( "char", 0x0000, duet16_charlayout, 0, 1 )
GFXDECODE_END


static SLOT_INTERFACE_START( duet16_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

MACHINE_CONFIG_START(duet16_state::duet16)
	MCFG_CPU_ADD("maincpu", I8086, 24_MHz_XTAL / 3)
	MCFG_CPU_PROGRAM_MAP(duet16_mem)
	MCFG_CPU_IO_MAP(duet16_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic", pic8259_device, inta_cb)

	MCFG_CPU_ADD("i8741", I8741, 20_MHz_XTAL / 4)
	MCFG_DEVICE_DISABLE()

	MCFG_DEVICE_ADD("pic", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE("maincpu", 0))

	MCFG_DEVICE_ADD("dmac", AM9517A, 20_MHz_XTAL / 4)
	MCFG_AM9517A_OUT_HREQ_CB(WRITELINE(duet16_state, hrq_w))
	MCFG_AM9517A_IN_MEMR_CB(READ8(duet16_state, dma_mem_r))
	MCFG_AM9517A_OUT_MEMW_CB(WRITE8(duet16_state, dma_mem_w))
	MCFG_AM9517A_IN_IOR_0_CB(DEVREAD8("fdc", upd765a_device, mdma_r))
	MCFG_AM9517A_OUT_IOW_0_CB(DEVWRITE8("fdc", upd765a_device, mdma_w))
	MCFG_AM9517A_OUT_EOP_CB(DEVWRITELINE("fdc", upd765a_device, tc_line_w))

	MCFG_DEVICE_ADD("bgpit", PIT8253, 0)
	MCFG_PIT8253_CLK0(8_MHz_XTAL / 13)
	MCFG_PIT8253_CLK1(8_MHz_XTAL / 13)
	MCFG_PIT8253_CLK2(8_MHz_XTAL / 13)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("sio", upd7201_new_device, txca_w)) // TODO: selected through LS153
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio", upd7201_new_device, rxca_w))
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("sio", upd7201_new_device, txcb_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio", upd7201_new_device, rxcb_w))
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("kbusart", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("kbusart", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("itm", PTM6840, 0)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0.0, 0.0, (8_MHz_XTAL / 8).dvalue()) // C3 = 1MHz
	MCFG_PTM6840_O3_CB(DEVWRITELINE("itm", ptm6840_device, set_c1)) // C1 = C2 = O3
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("itm", ptm6840_device, set_c2))
	MCFG_PTM6840_IRQ_CB(DEVWRITELINE("pic", pic8259_device, ir0_w)) // INT6

	MCFG_DEVICE_ADD("sio", UPD7201_NEW, 8_MHz_XTAL / 2)
	MCFG_Z80SIO_OUT_INT_CB(DEVWRITELINE("pic", pic8259_device, ir1_w)) // INT5

	MCFG_DEVICE_ADD("kbusart", I8251, 8_MHz_XTAL / 4)
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("kbusart", i8251_device, write_cts))
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE("kbint", input_merger_device, in_w<0>))
	MCFG_I8251_TXRDY_HANDLER(DEVWRITELINE("kbint", input_merger_device, in_w<1>))

	MCFG_INPUT_MERGER_ANY_HIGH("kbint")
	MCFG_INPUT_MERGER_OUTPUT_HANDLER(DEVWRITELINE("pic", pic8259_device, ir5_w)) // INT2

	MCFG_UPD765A_ADD("fdc", true, false)
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE("dmac", am9517a_device, dreq0_w))
	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE("pic", pic8259_device, ir3_w)) // INT4
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", duet16_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_SLOT_FIXED(true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", duet16_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_SLOT_FIXED(true)

	MCFG_DEVICE_ADD("crtc", H46505, 2000000)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(duet16_state, crtc_update_row)

	MCFG_PALETTE_ADD("palette", 8)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", duet16)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", h46505_device, screen_update)

	MCFG_DEVICE_ADD("rtc", MSM58321, 32768_Hz_XTAL)
MACHINE_CONFIG_END

ROM_START(duet16)
	ROM_REGION(0x2000, "rom", 0)
	ROM_LOAD16_BYTE("duet16_h516a_3.bin", 0x0001, 0x1000, CRC(936706aa) SHA1(412ff9c7bf4443d2ed29a8d792fc3c849c9393cc))
	ROM_LOAD16_BYTE("duet16_h517a_z.bin", 0x0000, 0x1000, CRC(1633cce8) SHA1(5145d04a48921cacfed17a94873e8988772fc8d4))

	ROM_REGION(0x2000, "char", 0)
	ROM_LOAD("duet16_char_j500a_4.bin", 0x0000, 0x2000, CRC(edf860f8) SHA1(0dcc584db701d21b7c3304cd2296562ebda6fb4c))

	ROM_REGION(0x400, "i8741", 0)
	ROM_LOAD("duet16_key_8741ak001b_z.bin", 0x000, 0x400, CRC(d23ee68d) SHA1(3b6a86fe2a304823c5385cd673f9580a35199dac))
ROM_END

COMP(1983, duet16, 0, 0, duet16, 0, duet16_state, 0, "Panafacom (Panasonic/Fujitsu)", "Duet-16", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
