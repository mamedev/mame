// license:BSD-3-Clause
// copyright-holders: Gabriele D'Antona
/*
    Olympia BOSS
    Made in Germany around 1981

    The BOSS series was not a great success, as its members differed too much to be compatible:
    First they were 8085 based, later machines used a Z80A.

    Other distinguishing features were the capacity of the disk drives:

    BOSS A: Two 128K floppy drives
    BOSS B: Two 256K disk drives
    BOSS C: Two 600K disk drives
    BOSS D: One 600K disk drive, one 5 MB harddisk
    BOSS M: M for multipost, up to four BOSS machines linked together for up to 20MB shared harddisk space

    Olympia favoured the French Prologue operating system over CPM (cf. Olympia People PC) and supplied BAL
    as a programming language with it.

    Video is 80x28

    There are no service manuals available (or no documentation in general), so everything is guesswork.

    - Ports 0x80 and 0x81 seem to be related to the graphics chip and cursor position
    The rom outs value 0x81 to port 0x81 and then the sequence <column> <row> (?) to port 0x80

    - The machine boots up and shows "BOSS .." on the screen. Every keystroke is repeated on screen.
    If you press <return>, the machine seems to go into a boot sequence (from the HD, probably)

    The harddisk controller is based on a MSC-9056.

    Links: http://www.old-computers.com/museum/computer.asp?c=95
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/floppy.h"
#include "machine/keyboard.h"
#include "video/upd3301.h"
#include "machine/i8257.h"
#include "machine/i8255.h"
#include "machine/am9519.h"
#include "machine/upd765.h"
#include "machine/pic8259.h"
#include "machine/i8251.h"
#include "screen.h"


namespace {

#define UPD3301_TAG     "upd3301"
#define I8257_TAG       "i8257"
#define SCREEN_TAG      "screen"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class olyboss_state : public driver_device
{
public:
	olyboss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dma(*this, I8257_TAG)
		, m_crtc(*this, UPD3301_TAG)
		, m_fdc(*this, "fdc")
		, m_uic(*this, "uic")
		, m_pic(*this, "pic")
		, m_ppi(*this, "ppi")
		, m_fdd0(*this, "fdc:0")
		, m_fdd1(*this, "fdc:1")
		, m_rom(*this, "mainrom")
		, m_lowram(*this, "lowram")
		, m_char_rom(*this, UPD3301_TAG)
	{ }

public:
	void bossa85(machine_config &config);
	void bossb85(machine_config &config);
	void olybossb(machine_config &config);
	void olybossc(machine_config &config);
	void olybossd(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(toggle_tim);

private:
	u8 keyboard_read();

	UPD3301_DRAW_CHARACTER_MEMBER( olyboss_display_pixels );

	void hrq_w(int state);
	void tc_w(int state);
	void romdis_w(int state);
	u8 dma_mem_r(offs_t offset);
	void dma_mem_w(offs_t offset, u8 data);
	u8 fdcctrl_r();
	void fdcctrl_w(u8 data);
	void fdcctrl85_w(u8 data);
	u8 fdcdma_r();
	void fdcdma_w(u8 data);
	void crtcdma_w(u8 data);
	u8 rom_r(offs_t offset);
	void rom_w(offs_t offset, u8 data);
	void vchrmap_w(offs_t offset, u8 data);
	void vchrram_w(offs_t offset, u8 data);
	void vchrram85_w(offs_t offset, u8 data);
	void ppic_w(u8 data);
	void olyboss_io(address_map &map) ATTR_COLD;
	void olyboss_mem(address_map &map) ATTR_COLD;
	void olyboss85_io(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<i8257_device> m_dma;
	required_device<upd3301_device> m_crtc;
	required_device<upd765a_device> m_fdc;
	optional_device<am9519_device> m_uic;
	optional_device<pic8259_device> m_pic;
	optional_device<i8255_device> m_ppi;
	required_device<floppy_connector> m_fdd0;
	optional_device<floppy_connector> m_fdd1;
	required_memory_region m_rom;
	required_shared_ptr<u8> m_lowram;
	required_memory_region m_char_rom;

	bool m_keybhit;
	u8 m_keystroke;
	void keyboard_put(u8 data);
	void keyboard85_put(u8 data);
	u8 m_fdcctrl, m_fdctype;
	u8 m_channel, m_vchrmap, m_vchrpage;
	u16 m_vchraddr;
	u8 m_vchrram[0x800];
	bool m_romen, m_timstate;
	emu_timer *m_timer;
};

void olyboss_state::machine_reset()
{
	m_keybhit = false;
	m_romen = true;
	m_timstate = false;

	m_fdcctrl = 0;
	m_vchrmap = 0;
	m_vchrpage = 0;
	m_timer->adjust(attotime::from_hz(30), 0, attotime::from_hz(30)); // unknown timer freq, possibly com2651 BRCLK
}

TIMER_CALLBACK_MEMBER(olyboss_state::toggle_tim)
{
	m_timstate = !m_timstate;
	if(m_pic)
		m_pic->ir0_w(m_timstate);
	else
		m_uic->ireq7_w(m_timstate);
}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void olyboss_state::olyboss_mem(address_map &map)
{
	map(0x0000, 0x7ff).rw(FUNC(olyboss_state::rom_r), FUNC(olyboss_state::rom_w)).share("lowram");
	map(0x800, 0xffff).ram();
}

void olyboss_state::olyboss_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x08).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x10, 0x11).m(m_fdc, FUNC(upd765a_device::map));
	//map(0x20, 0x20) //beeper?
	map(0x30, 0x30).rw(m_uic, FUNC(am9519_device::data_r), FUNC(am9519_device::data_w));
	map(0x31, 0x31).rw(m_uic, FUNC(am9519_device::stat_r), FUNC(am9519_device::cmd_w));
	map(0x40, 0x43).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	//map(0x50, 0x53) COM2651
	map(0x60, 0x60).rw(FUNC(olyboss_state::fdcctrl_r), FUNC(olyboss_state::fdcctrl_w));
	map(0x80, 0x81).rw(m_crtc, FUNC(upd3301_device::read), FUNC(upd3301_device::write));
	map(0x82, 0x84).w(FUNC(olyboss_state::vchrmap_w));
	map(0x90, 0x9f).w(FUNC(olyboss_state::vchrram_w));
}

void olyboss_state::olyboss85_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x0, 0x8).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x10, 0x11).m(m_fdc, FUNC(upd765a_device::map));
	map(0x20, 0x21).rw(m_crtc, FUNC(upd3301_device::read), FUNC(upd3301_device::write));
	map(0x30, 0x31).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x42, 0x42).r(FUNC(olyboss_state::keyboard_read));
	map(0x42, 0x44).w(FUNC(olyboss_state::vchrram85_w));
	map(0x45, 0x45).w(FUNC(olyboss_state::fdcctrl85_w));
}

static INPUT_PORTS_START( olyboss )
	PORT_START("DSW")
INPUT_PORTS_END

u8 olyboss_state::rom_r(offs_t offset)
{
	return m_romen ?  m_rom->as_u8(offset) : m_lowram[offset];
}

void olyboss_state::rom_w(offs_t offset, u8 data)
{
	m_lowram[offset] = data;
}

void olyboss_state::vchrram85_w(offs_t offset, u8 data)
{
	switch(offset)
	{
		case 0:
			m_vchraddr = (m_vchraddr & 0x00f) | (data << 4);
			break;
		case 1:
			m_vchraddr = (m_vchraddr & 0xff0) | (data & 0xf);
			break;
		case 2:
			m_vchrram[m_vchraddr] = data;
			break;
	}
}

void olyboss_state::vchrmap_w(offs_t offset, u8 data)
{
	switch(offset)
	{
		case 0:
			m_vchrmap = data;
			break;
		case 2:
			m_vchrpage = data & 0x7f;
			break;
	}
}

void olyboss_state::vchrram_w(offs_t offset, u8 data)
{
	m_vchrram[(m_vchrpage << 4) + (offset ^ 0xf)] = data;
}

void olyboss_state::romdis_w(int state)
{
	m_romen = state ? false : true;
}

//**************************************************************************
//  VIDEO
//**************************************************************************

UPD3301_DRAW_CHARACTER_MEMBER( olyboss_state::olyboss_display_pixels )
{
	u8 data = cc & 0x7f;
	if(cc & 0x80)
		data = m_vchrram[(data << 4) | lc];
	else
		data = m_char_rom->base()[(data << 4) | lc];

	//if (lc >= 8) return;
	if (csr)
		data = 0xff;

	for (int i = 0; i < 8; i++)
	{
		int color = BIT(data, 7);
		bitmap.pix(y, (sx * 8) + i) = color ? 0xffffff : 0;
		data <<= 1;
	}
}

//**************************************************************************
//  KEYBOARD
//**************************************************************************

u8 olyboss_state::keyboard_read()
{
	//logerror ("keyboard_read offs [%d]\n",offset);
	if (m_keybhit)
	{
		m_keybhit=false;
		if(m_pic)
			m_pic->ir1_w(CLEAR_LINE);
		return m_keystroke;
	}
	return 0x00;
}

void olyboss_state::ppic_w(u8 data)
{
	m_uic->ireq4_w(BIT(data, 5) ? CLEAR_LINE : ASSERT_LINE);
	m_fdcctrl = (m_fdcctrl & ~0x10) | (BIT(data, 5) ? 0x10 : 0);
}

void olyboss_state::machine_start()
{
	m_timer = timer_alloc(FUNC(olyboss_state::toggle_tim), this);
	const char *type = m_fdd0->get_device()->shortname();
	if(!strncmp(type, "floppy_525_qd", 13))
		m_fdctype = 0xa0;
	else
		m_fdctype = 0x80;
}

void olyboss_state::keyboard_put(u8 data)
{
	if (data)
	{
		//logerror("Keyboard stroke [%2x]\n",data);
		m_keystroke=data ^ 0xff;
		m_keybhit=true;
		m_ppi->pc4_w(ASSERT_LINE);
		m_ppi->pc4_w(CLEAR_LINE);
	}
}

void olyboss_state::keyboard85_put(u8 data)
{
	if(data)
	{
		m_pic->ir1_w(ASSERT_LINE);
		m_keybhit = true;
		m_keystroke = data;
	}
}

/* 8257 Interface */

void olyboss_state::hrq_w(int state)
{
	//logerror("hrq_w\n");
	m_maincpu->set_input_line(INPUT_LINE_HALT,state);
	m_dma->hlda_w(state);
}

void olyboss_state::tc_w(int state)
{
	if((m_channel == 0) && state)
	{
		m_fdc->tc_w(1);
		m_fdc->tc_w(0);
	}
}

u8 olyboss_state::dma_mem_r(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	return program.read_byte(offset);
}

void olyboss_state::dma_mem_w(offs_t offset, u8 data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.write_byte(offset, data);
}

u8 olyboss_state::fdcdma_r()
{
	m_channel = 0;
	return m_fdc->dma_r();
}

void olyboss_state::fdcdma_w(u8 data)
{
	m_channel = 0;
	m_fdc->dma_w(data);
}

void olyboss_state::crtcdma_w(u8 data)
{
	m_channel = 2;
	m_crtc->dack_w(data);
}

u8 olyboss_state::fdcctrl_r()
{
	return m_fdcctrl | m_fdctype; // 0xc0 seems to indicate an 8" drive, 0x80 a 5.25" dd drive, 0xa0 a 5.25" qd drive
}

void olyboss_state::fdcctrl_w(u8 data)
{
	m_fdcctrl = data;
	m_romen = (m_fdcctrl & 1) ? false : true;
	m_fdd0->get_device()->mon_w(!(data & 2));
	if(m_fdd1)
		m_fdd1->get_device()->mon_w(!(data & 4));
}

void olyboss_state::fdcctrl85_w(u8 data)
{
	m_fdcctrl = data;
	m_fdd0->get_device()->mon_w(!(data & 0x40));
	if(m_fdd1)
		m_fdd1->get_device()->mon_w(!(data & 0x80));
}

static void bossa_floppies(device_slot_interface &device)
{
	device.option_add("525ssdd", FLOPPY_525_SSDD);
}

static void bossb_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

static void bosscd_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

void olyboss_state::olybossd(machine_config &config)
{
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &olyboss_state::olyboss_mem);
	m_maincpu->set_addrmap(AS_IO, &olyboss_state::olyboss_io);
	m_maincpu->set_irq_acknowledge_callback("uic", FUNC(am9519_device::iack_cb));

	/* video hardware */

	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_refresh_hz(60);
	screen.set_screen_update(UPD3301_TAG, FUNC(upd3301_device::screen_update));
	screen.set_size(80*8, 28*11);
	screen.set_visarea(0, (80*8)-1, 0, (28*11)-1);

	/* devices */

	AM9519(config, m_uic, 0);
	m_uic->out_int_callback().set_inputline("maincpu", 0);

	UPD765A(config, m_fdc, 8'000'000, true, true);
	m_fdc->intrq_wr_callback().set(m_uic, FUNC(am9519_device::ireq2_w)).invert();
	m_fdc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq0_w));
	FLOPPY_CONNECTOR(config, m_fdd0, bosscd_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	m_fdd0->enable_sound(true);

	I8257(config, m_dma, XTAL(4'000'000));
	m_dma->out_hrq_cb().set(FUNC(olyboss_state::hrq_w));
	m_dma->in_memr_cb().set(FUNC(olyboss_state::dma_mem_r));
	m_dma->out_memw_cb().set(FUNC(olyboss_state::dma_mem_w));
	m_dma->in_ior_cb<0>().set(FUNC(olyboss_state::fdcdma_r));
	m_dma->out_iow_cb<0>().set(FUNC(olyboss_state::fdcdma_w));
	m_dma->out_iow_cb<2>().set(FUNC(olyboss_state::crtcdma_w));
	m_dma->out_tc_cb().set(FUNC(olyboss_state::tc_w));

	UPD3301(config, m_crtc, XTAL(14'318'181));
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(olyboss_state::olyboss_display_pixels));
	m_crtc->set_attribute_fetch_callback(m_crtc, FUNC(upd3301_device::default_attr_fetch));
	m_crtc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq2_w));
	m_crtc->int_wr_callback().set(m_uic, FUNC(am9519_device::ireq0_w)).invert();
	m_crtc->set_screen(SCREEN_TAG);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(olyboss_state::keyboard_read));
	m_ppi->out_pc_callback().set(FUNC(olyboss_state::ppic_w));

	/* keyboard */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(olyboss_state::keyboard_put));
}

void olyboss_state::olybossb(machine_config &config)
{
	olybossd(config);
	config.device_remove("fdc:0");
	FLOPPY_CONNECTOR(config, "fdc:0", bossb_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", bossb_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

void olyboss_state::olybossc(machine_config &config)
{
	olybossd(config);
	FLOPPY_CONNECTOR(config, "fdc:1", bosscd_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

void olyboss_state::bossb85(machine_config &config)
{
	i8085a_cpu_device &maincpu(I8085A(config, m_maincpu, 4_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &olyboss_state::olyboss_mem);
	maincpu.set_addrmap(AS_IO, &olyboss_state::olyboss85_io);
	maincpu.in_inta_func().set(m_pic, FUNC(pic8259_device::acknowledge));
	maincpu.out_sod_func().set(FUNC(olyboss_state::romdis_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_refresh_hz(60);
	screen.set_screen_update(UPD3301_TAG, FUNC(upd3301_device::screen_update));
	screen.set_size(80*8, 28*11);
	screen.set_visarea(0, (80*8)-1, 0, (28*11)-1);

	/* devices */

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_maincpu, 0);

	UPD765A(config, m_fdc, 8'000'000, true, true);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, I8085_RST65_LINE);
	m_fdc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq0_w));
	FLOPPY_CONNECTOR(config, "fdc:0", bossb_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", bossb_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	I8257(config, m_dma, XTAL(4'000'000));
	m_dma->out_hrq_cb().set(FUNC(olyboss_state::hrq_w));
	m_dma->in_memr_cb().set(FUNC(olyboss_state::dma_mem_r));
	m_dma->out_memw_cb().set(FUNC(olyboss_state::dma_mem_w));
	m_dma->in_ior_cb<0>().set(FUNC(olyboss_state::fdcdma_r));
	m_dma->out_iow_cb<0>().set(FUNC(olyboss_state::fdcdma_w));
	m_dma->out_iow_cb<2>().set(FUNC(olyboss_state::crtcdma_w));
	m_dma->out_tc_cb().set(FUNC(olyboss_state::tc_w));

	UPD3301(config, m_crtc, XTAL(14'318'181));
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(olyboss_state::olyboss_display_pixels));
	m_crtc->set_attribute_fetch_callback(m_crtc, FUNC(upd3301_device::default_attr_fetch));
	m_crtc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq2_w));
	m_crtc->int_wr_callback().set_inputline("maincpu", I8085_RST75_LINE);
	m_crtc->set_screen(SCREEN_TAG);

	/* keyboard */
	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(olyboss_state::keyboard85_put));
}

void olyboss_state::bossa85(machine_config &config)
{
	bossb85(config);
	FLOPPY_CONNECTOR(config.replace(), "fdc:0", bossa_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config.replace(), "fdc:1", bossa_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************
ROM_START( bossa85 )
	ROM_REGION(0x800, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD( "boss_8085_bios.bin", 0x0000, 0x800, CRC(43030231) SHA1(a1f6546a9dc1066324e93e5eed886f2313678180) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "olympia_boss_graphics_251-461.bin", 0x0000, 0x800, CRC(56149540) SHA1(b2b893bd219308fc98a38528beb7ddae391c7609) )
ROM_END

ROM_START( bossb85 )
	ROM_REGION(0x800, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD( "boss_8085_bios.bin", 0x0000, 0x800, CRC(43030231) SHA1(a1f6546a9dc1066324e93e5eed886f2313678180) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "olympia_boss_graphics_251-461.bin", 0x0000, 0x800, CRC(56149540) SHA1(b2b893bd219308fc98a38528beb7ddae391c7609) )
ROM_END

ROM_START( olybossb )                                           // verified: BOSS B uses the same ROMs as D, so C is safe to assume as well
	ROM_REGION(0x800, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD( "olympia_boss_system_251-462.bin", 0x0000, 0x800, CRC(01b99609) SHA1(07b764c36337c12f7b40aa309b0805ceed8b22e2) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "olympia_boss_graphics_251-461.bin", 0x0000, 0x800, CRC(56149540) SHA1(b2b893bd219308fc98a38528beb7ddae391c7609) )
ROM_END

ROM_START( olybossc )
	ROM_REGION(0x800, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD( "olympia_boss_system_251-462.bin", 0x0000, 0x800, CRC(01b99609) SHA1(07b764c36337c12f7b40aa309b0805ceed8b22e2) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "olympia_boss_graphics_251-461.bin", 0x0000, 0x800, CRC(56149540) SHA1(b2b893bd219308fc98a38528beb7ddae391c7609) )
ROM_END

ROM_START( olybossd )
	ROM_REGION(0x800, "mainrom", ROMREGION_ERASEFF)
	ROM_LOAD( "olympia_boss_system_251-462.bin", 0x0000, 0x800, CRC(01b99609) SHA1(07b764c36337c12f7b40aa309b0805ceed8b22e2) )

	ROM_REGION( 0x800, UPD3301_TAG, 0)
	ROM_LOAD( "olympia_boss_graphics_251-461.bin", 0x0000, 0x800, CRC(56149540) SHA1(b2b893bd219308fc98a38528beb7ddae391c7609) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//   YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY                  FULLNAME               FLAGS
COMP(1981, bossa85,  olybossd, 0,      bossa85,  olyboss, olyboss_state, empty_init, "Olympia International", "Olympia BOSS A 8085", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP(1981, bossb85,  olybossd, 0,      bossb85,  olyboss, olyboss_state, empty_init, "Olympia International", "Olympia BOSS B 8085", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP(1981, olybossb, olybossd, 0,      olybossb, olyboss, olyboss_state, empty_init, "Olympia International", "Olympia BOSS B",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP(1981, olybossc, olybossd, 0,      olybossc, olyboss, olyboss_state, empty_init, "Olympia International", "Olympia BOSS C",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP(1981, olybossd, 0,        0,      olybossd, olyboss, olyboss_state, empty_init, "Olympia International", "Olympia BOSS D",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
