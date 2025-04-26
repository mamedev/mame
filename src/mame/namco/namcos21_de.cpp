// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Naibo, David Haywood
/**

see http://www.tvspels-nostalgi.com/driverseye.htm for details about setup

2008/06/11, by Naibo(translated to English by Mameplus team):
Driver's Eyes works,
    -the communication work between CPU and 3D DSP should be limited to the master M68000,
    if the address mapping is done in the shared memory, master CPU would be disturbed by the slave one.

    -The left, center and right screens have separate programs and boards, each would work independently.
    About projection angles of left and right screen, the angle is correct on "DRIVER'S EYES" title screen, however in the tracks of demo mode it doesn't seem correct.
    (probably wants angle sent by main board?)

    -On demo screen, should fog effects be turned off?

    NOTES:

    Driver's Eyes
        not yet working

    TODO:

    Driver's Eyes
        add communications for Left and Right screen (linked C139 or something else?)

*/

#include "emu.h"
#include "namcoio_gearbox.h"
#include "screen.h"
#include "emupal.h"
#include "speaker.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6805/m6805.h"
#include "cpu/m6809/m6809.h"
#include "cpu/tms32025/tms32025.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "namco_c139.h"
#include "namco_c148.h"
#include "namco68.h"
#include "namcos21_dsp.h"
#include "namco_c355spr.h"
#include "namcos21_3d.h"
#include "sound/c140.h"
#include "sound/ymopm.h"
#include "emupal.h"


#define ENABLE_LOGGING      0

DECLARE_DEVICE_TYPE(NAMCO_DE_PCB, namco_de_pcbstack_device)


class namco_de_pcbstack_device : public device_t
{
public:
	// construction/destruction
	namco_de_pcbstack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void configure_c148_standard(machine_config &config);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:

	INTERRUPT_GEN_MEMBER( irq0_line_hold );
	INTERRUPT_GEN_MEMBER( irq1_line_hold );

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_slave;
	required_device<namcoc68_device> m_c68;
	required_device<namco_c139_device> m_sci;
	required_device<namco_c148_device> m_master_intc;
	required_device<namco_c148_device> m_slave_intc;
	required_device<c140_device> m_c140;
	required_device<namco_c355spr_device> m_c355spr;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_memory_bank m_audiobank;
	required_region_ptr<u16> m_c140_region;
	required_shared_ptr<uint8_t> m_dpram;
	required_device<namcos21_3d_device> m_namcos21_3d;
	required_device<namcos21_dsp_device> m_namcos21_dsp;

	uint16_t m_video_enable;

	uint16_t video_enable_r();
	void video_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t dpram_word_r(offs_t offset);
	void dpram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t dpram_byte_r(offs_t offset);
	void dpram_byte_w(offs_t offset, uint8_t data);

	void eeprom_w(offs_t offset, uint8_t data);
	uint8_t eeprom_r(offs_t offset);

	void sound_bankselect_w(uint8_t data);

	void sound_reset_w(uint8_t data);
	void system_reset_w(uint8_t data);
	void reset_all_subcpus(int state);

	std::unique_ptr<uint8_t[]> m_eeprom;

	TIMER_DEVICE_CALLBACK_MEMBER(screen_scanline);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void configure_c68_namcos21(machine_config &config);

	void driveyes_common_map(address_map &map) ATTR_COLD;
	void driveyes_master_map(address_map &map) ATTR_COLD;
	void driveyes_slave_map(address_map &map) ATTR_COLD;

	void sound_map(address_map &map) ATTR_COLD;
	void c140_map(address_map &map) ATTR_COLD;
};


DEFINE_DEVICE_TYPE(NAMCO_DE_PCB, namco_de_pcbstack_device, "namco_de_pcb", "Namco Driver's Eyes PCB stack")


namco_de_pcbstack_device::namco_de_pcbstack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NAMCO_DE_PCB, tag, owner, clock),
	m_maincpu(*this, "maincpu"),
	m_audiocpu(*this, "audiocpu"),
	m_slave(*this, "slave"),
	m_c68(*this, "c68mcu"),
	m_sci(*this, "sci"),
	m_master_intc(*this, "master_intc"),
	m_slave_intc(*this, "slave_intc"),
	m_c140(*this, "c140"),
	m_c355spr(*this, "c355spr"),
	m_palette(*this, "palette"),
	m_screen(*this, "screen"),
	m_audiobank(*this, "audiobank"),
	m_c140_region(*this, "c140"),
	m_dpram(*this, "dpram"),
	m_namcos21_3d(*this, "namcos21_3d"),
	m_namcos21_dsp(*this, "namcos21dsp")
{
}

INTERRUPT_GEN_MEMBER( namco_de_pcbstack_device::irq0_line_hold )   { device.execute().set_input_line(0, HOLD_LINE); }
INTERRUPT_GEN_MEMBER( namco_de_pcbstack_device::irq1_line_hold )   { device.execute().set_input_line(1, HOLD_LINE); }


void namco_de_pcbstack_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_maincpu, 49.152_MHz_XTAL / 4); /* Master */
	m_maincpu->set_addrmap(AS_PROGRAM, &namco_de_pcbstack_device::driveyes_master_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(namco_de_pcbstack_device::screen_scanline), "screen", 0, 1);

	M68000(config, m_slave, 49.152_MHz_XTAL / 4); /* Slave */
	m_slave->set_addrmap(AS_PROGRAM, &namco_de_pcbstack_device::driveyes_slave_map);

	MC6809E(config, m_audiocpu, 3072000); /* Sound */
	m_audiocpu->set_addrmap(AS_PROGRAM, &namco_de_pcbstack_device::sound_map);
	m_audiocpu->set_periodic_int(FUNC(namco_de_pcbstack_device::irq0_line_hold), attotime::from_hz(2*60));

	configure_c68_namcos21(config);

	NAMCOS21_DSP(config, m_namcos21_dsp, 0);
	m_namcos21_dsp->set_renderer_tag("namcos21_3d");

	config.set_maximum_quantum(attotime::from_hz(6000)); /* 100 CPU slices per frame */

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	configure_c148_standard(config);
	NAMCO_C139(config, m_sci, 0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: basic parameters to get 60.606060 Hz, x2 is for interlace
	m_screen->set_raw(49.152_MHz_XTAL / 4 * 2, 768, 0, 496, 264*2, 0, 480);
	m_screen->set_screen_update(FUNC(namco_de_pcbstack_device::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBRG_888, 0x10000/2);

	NAMCOS21_3D(config, m_namcos21_3d, 0);
	m_namcos21_3d->set_fixed_palbase(0x3f00);
	m_namcos21_3d->set_zz_shift_mult(10, 0x100);
	m_namcos21_3d->set_depth_reverse(false);
	m_namcos21_3d->set_framebuffer_size(496,480);

	NAMCO_C355SPR(config, m_c355spr, 0);
	m_c355spr->set_screen(m_screen);
	m_c355spr->set_palette(m_palette);
	m_c355spr->set_scroll_offsets(0x26, 0x19);
	m_c355spr->set_tile_callback(namco_c355spr_device::c355_obj_code2tile_delegate());
	m_c355spr->set_palxor(0xf); // reverse mapping
	m_c355spr->set_color_base(0x1000);
	m_c355spr->set_external_prifill(true);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	C140(config, m_c140, 49.152_MHz_XTAL / 2304);
	m_c140->set_addrmap(0, &namco_de_pcbstack_device::c140_map);
	m_c140->int1_callback().set_inputline(m_audiocpu, M6809_FIRQ_LINE);
	m_c140->add_route(0, "lspeaker", 0.50);
	m_c140->add_route(1, "rspeaker", 0.50);

	YM2151(config, "ymsnd", 3.579545_MHz_XTAL).add_route(0, "lspeaker", 0.30).add_route(1, "rspeaker", 0.30);
}


uint32_t namco_de_pcbstack_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//uint8_t *videoram = m_gpu_videoram.get();
	int pivot = 3;
	int pri;
	bitmap.fill(0xff, cliprect );
	screen.priority().fill(0, cliprect);
	m_c355spr->build_sprite_list_and_render_sprites(cliprect); // TODO : buffered?

	m_c355spr->draw(screen, bitmap, cliprect, 2 );
	m_c355spr->draw(screen, bitmap, cliprect, 14 );   //driver's eyes

	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0x7fc0, 0x7ffe);

	m_c355spr->draw(screen, bitmap, cliprect, 0 );
	m_c355spr->draw(screen, bitmap, cliprect, 1 );

	m_namcos21_3d->copy_visible_poly_framebuffer(bitmap, cliprect, 0, 0x7fbf);

	for (pri = pivot; pri < 8; pri++)
	{
		m_c355spr->draw(screen, bitmap, cliprect, pri);
	}

	m_c355spr->draw(screen, bitmap, cliprect, 15 );   //driver's eyes

	return 0;

}

uint16_t namco_de_pcbstack_device::video_enable_r()
{
	return m_video_enable;
}

void namco_de_pcbstack_device::video_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA( &m_video_enable ); /* 0x40 = enable */
	if( m_video_enable!=0 && m_video_enable!=0x40 )
	{
		logerror( "unexpected video_enable_w=0x%x\n", m_video_enable );
	}
}

/***********************************************************/

/* dual port ram memory handlers */

uint16_t namco_de_pcbstack_device::dpram_word_r(offs_t offset)
{
	return m_dpram[offset];
}

void namco_de_pcbstack_device::dpram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if( ACCESSING_BITS_0_7 )
	{
		m_dpram[offset] = data&0xff;
	}
}

uint8_t namco_de_pcbstack_device::dpram_byte_r(offs_t offset)
{
	return m_dpram[offset];
}

void namco_de_pcbstack_device::dpram_byte_w(offs_t offset, uint8_t data)
{
	m_dpram[offset] = data;
}

/*************************************************************/
/* SOUND 6809 CPU Memory declarations                        */
/*************************************************************/

void namco_de_pcbstack_device::sound_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr("audiobank"); /* banked */
	map(0x3000, 0x3003).nopw(); /* ? */
	map(0x4000, 0x4001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x5000, 0x51ff).mirror(0x0e00).rw(m_c140, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w));
	map(0x6000, 0x61ff).mirror(0x0e00).rw(m_c140, FUNC(c140_device::c140_r), FUNC(c140_device::c140_w)); // mirrored
	map(0x7000, 0x77ff).mirror(0x0800).rw(FUNC(namco_de_pcbstack_device::dpram_byte_r), FUNC(namco_de_pcbstack_device::dpram_byte_w)).share("dpram");
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).nopw(); /* amplifier enable on 1st write */
	map(0xc000, 0xffff).nopw(); /* avoid debug log noise; games write frequently to 0xe000 */
	map(0xc000, 0xc001).w(FUNC(namco_de_pcbstack_device::sound_bankselect_w));
	map(0xd001, 0xd001).nopw(); /* watchdog */
	map(0xd000, 0xffff).rom().region("audiocpu", 0x01000);
}

void namco_de_pcbstack_device::c140_map(address_map &map)
{
	map.global_mask(0x7fffff);
	// TODO: LSB not used? verify from schematics/real hardware
	map(0x000000, 0x7fffff).lr16([this](offs_t offset) { return m_c140_region[((offset & 0x300000) >> 1) | (offset & 0x7ffff)]; }, "c140_rom_r");
}

/*************************************************************/
/* I/O HD63705 MCU Memory declarations                       */
/*************************************************************/

void namco_de_pcbstack_device::configure_c68_namcos21(machine_config &config)
{
	NAMCOC68(config, m_c68, 8000000);
	m_c68->in_pb_callback().set_ioport("MCUB");
	m_c68->in_pc_callback().set_ioport("MCUC");
	m_c68->in_ph_callback().set_ioport("MCUH");
	m_c68->in_pdsw_callback().set_ioport("DSW");
	m_c68->di0_in_cb().set_ioport("MCUDI0");
	m_c68->di1_in_cb().set_ioport("MCUDI1");
	m_c68->di2_in_cb().set_ioport("MCUDI2");
	m_c68->di3_in_cb().set_ioport("MCUDI3");
	m_c68->an0_in_cb().set_ioport("AN0");
	m_c68->an1_in_cb().set_ioport("AN1");
	m_c68->an2_in_cb().set_ioport("AN2");
	m_c68->an3_in_cb().set_ioport("AN3");
	m_c68->an4_in_cb().set_ioport("AN4");
	m_c68->an5_in_cb().set_ioport("AN5");
	m_c68->an6_in_cb().set_ioport("AN6");
	m_c68->an7_in_cb().set_ioport("AN7");
	m_c68->dp_in_callback().set(FUNC(namco_de_pcbstack_device::dpram_byte_r));
	m_c68->dp_out_callback().set(FUNC(namco_de_pcbstack_device::dpram_byte_w));
}

/*************************************************************/
/* Driver's Eyes Memory declarations overrides               */
/*************************************************************/


void namco_de_pcbstack_device::driveyes_common_map(address_map &map)
{
	map(0x700000, 0x71ffff).rw(m_c355spr, FUNC(namco_c355spr_device::spriteram_r), FUNC(namco_c355spr_device::spriteram_w));
	map(0x720000, 0x720007).rw(m_c355spr, FUNC(namco_c355spr_device::position_r), FUNC(namco_c355spr_device::position_w));
	map(0x740000, 0x74ffff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x750000, 0x75ffff).ram().w(m_palette, FUNC(palette_device::write16_ext)).share("palette_ext");
	map(0x760000, 0x760001).rw(FUNC(namco_de_pcbstack_device::video_enable_r), FUNC(namco_de_pcbstack_device::video_enable_w));
	map(0x800000, 0x8fffff).rom().region("data", 0);
	map(0x900000, 0x90ffff).ram().share("sharedram");
	map(0xa00000, 0xa00fff).rw(FUNC(namco_de_pcbstack_device::dpram_word_r), FUNC(namco_de_pcbstack_device::dpram_word_w));
	map(0xb00000, 0xb03fff).rw(m_sci, FUNC(namco_c139_device::ram_r), FUNC(namco_c139_device::ram_w));
	map(0xb80000, 0xb8000f).m(m_sci, FUNC(namco_c139_device::regs_map));
}

void namco_de_pcbstack_device::driveyes_master_map(address_map &map)
{
	driveyes_common_map(map);
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram(); /* private work RAM */
	map(0x180000, 0x183fff).rw(FUNC(namco_de_pcbstack_device::eeprom_r), FUNC(namco_de_pcbstack_device::eeprom_w)).umask16(0x00ff);
	map(0x1c0000, 0x1fffff).m(m_master_intc, FUNC(namco_c148_device::map));

	// DSP related
	map(0x250000, 0x25ffff).ram().share("namcos21dsp:winrun_polydata");
	map(0x280000, 0x281fff).w(m_namcos21_dsp, FUNC(namcos21_dsp_device::winrun_dspbios_w));
	map(0x380000, 0x38000f).rw(m_namcos21_dsp, FUNC(namcos21_dsp_device::winrun_dspcomram_control_r), FUNC(namcos21_dsp_device::winrun_dspcomram_control_w));
	map(0x3c0000, 0x3c1fff).rw(m_namcos21_dsp, FUNC(namcos21_dsp_device::winrun_68k_dspcomram_r), FUNC(namcos21_dsp_device::winrun_68k_dspcomram_w));
	map(0x400000, 0x400001).w(m_namcos21_dsp, FUNC(namcos21_dsp_device::pointram_control_w));
	map(0x440000, 0x440001).rw(m_namcos21_dsp, FUNC(namcos21_dsp_device::pointram_data_r), FUNC(namcos21_dsp_device::pointram_data_w));
}

void namco_de_pcbstack_device::driveyes_slave_map(address_map &map)
{
	driveyes_common_map(map);
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10ffff).ram(); /* private work RAM */
	map(0x1c0000, 0x1fffff).m(m_slave_intc, FUNC(namco_c148_device::map));
}

void namco_de_pcbstack_device::sound_bankselect_w(uint8_t data)
{
	m_audiobank->set_entry(data>>4);
}

void namco_de_pcbstack_device::sound_reset_w(uint8_t data)
{
	if (data & 0x01)
	{
		/* Resume execution */
		m_audiocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_maincpu->yield();
	}
	else
	{
		/* Suspend execution */
		m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
}

void namco_de_pcbstack_device::system_reset_w(uint8_t data)
{
	reset_all_subcpus(data & 1 ? CLEAR_LINE : ASSERT_LINE);

	if (data & 0x01)
		m_maincpu->yield();
}

void namco_de_pcbstack_device::reset_all_subcpus(int state)
{
	m_slave->set_input_line(INPUT_LINE_RESET, state);
	m_c68->ext_reset(state);
}

void namco_de_pcbstack_device::eeprom_w(offs_t offset, uint8_t data)
{
	m_eeprom[offset] = data;
}

uint8_t namco_de_pcbstack_device::eeprom_r(offs_t offset)
{
	return m_eeprom[offset];
}


TIMER_DEVICE_CALLBACK_MEMBER(namco_de_pcbstack_device::screen_scanline)
{
	int scanline = param;
//  int cur_posirq = get_posirq_scanline()*2;

	if(scanline == 240*2)
	{
		m_master_intc->vblank_irq_trigger();
		m_slave_intc->vblank_irq_trigger();
		m_c68->ext_interrupt(ASSERT_LINE);
	}
}

void namco_de_pcbstack_device::configure_c148_standard(machine_config &config)
{
	NAMCO_C148(config, m_master_intc, 0, m_maincpu, true);
	m_master_intc->link_c148_device(m_slave_intc);
	m_master_intc->out_ext1_callback().set(FUNC(namco_de_pcbstack_device::sound_reset_w));
	m_master_intc->out_ext2_callback().set(FUNC(namco_de_pcbstack_device::system_reset_w));

	NAMCO_C148(config, m_slave_intc, 0, m_slave, false);
	m_slave_intc->link_c148_device(m_master_intc);
}

void namco_de_pcbstack_device::device_start()
{
	m_eeprom = std::make_unique<uint8_t[]>(0x2000);
	subdevice<nvram_device>("nvram")->set_base(m_eeprom.get(), 0x2000);

	uint32_t max = memregion("audiocpu")->bytes() / 0x4000;
	for (int i = 0; i < 0x10; i++)
		m_audiobank->configure_entry(i, memregion("audiocpu")->base() + (i % max) * 0x4000);

	save_item(NAME(m_video_enable));
}

void namco_de_pcbstack_device::device_reset()
{
	/* Initialise the bank select in the sound CPU */
	m_audiobank->set_entry(0); /* Page in bank 0 */

	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE );

	/* Place CPU2 & CPU3 into the reset condition */
	reset_all_subcpus(ASSERT_LINE);
}


class namcos21_de_state : public driver_device
{
public:
	namcos21_de_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_pcb(*this, "pcb_%u", 0U),
		m_io_gearbox(*this, "gearbox")
	{ }

	void driveyes(machine_config &config);

private:
	required_device_array<namco_de_pcbstack_device, 3> m_pcb;
	required_device<namcoio_gearbox_device> m_io_gearbox;
};

// driveyes only
void namcos21_de_state::driveyes(machine_config &config)
{
	NAMCO_DE_PCB(config, m_pcb[0], 0);
	NAMCO_DE_PCB(config, m_pcb[1], 0);
	NAMCO_DE_PCB(config, m_pcb[2], 0);

	NAMCOIO_GEARBOX(config, m_io_gearbox, 0);
}

// stacks with the DSWs set to left or right screen will show 'receive error' because they want comms from the main screen

static INPUT_PORTS_START( driveyes )
	PORT_START("pcb_1:MCUC")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Button") PORT_CODE(KEYCODE_0) PORT_TOGGLE // alt test mode switch
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("pcb_1:MCUB")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("gearbox", FUNC(namcoio_gearbox_device::clutch_r))
	PORT_BIT( 0x37, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) /* ? */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) /* ? */

	PORT_START("pcb_1:MCUH")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Red Button")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Green Button")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("pcb_1:DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "DSW2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Screen (DON'T CHANGE)")
	PORT_DIPSETTING(    0x0c, "Center (correct)" )
	PORT_DIPSETTING(    0x08, "Left (invalid)" )
	PORT_DIPSETTING(    0x04, "Right (invalid)" )
	PORT_DIPSETTING(    0x00, "Right (invalid) (duplicate)" )
	PORT_DIPNAME( 0x10, 0x10, "DSW5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "PCM ROM")
	PORT_DIPSETTING(    0x20, "2M" )
	PORT_DIPSETTING(    0x00, "4M" )
	PORT_DIPNAME( 0x40, 0x40, "DSW7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Screen Stop")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("pcb_1:AN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_1:AN1")
	PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(10) PORT_NAME("Gas Pedal")
	PORT_START("pcb_1:AN2")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(10) PORT_NAME("Steering Wheel")
	PORT_START("pcb_1:AN3")
	PORT_BIT( 0xff, 0x80, IPT_PEDAL2 ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(15) PORT_KEYDELTA(10) PORT_NAME("Brake Pedal")
	PORT_START("pcb_1:AN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_1:AN5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_1:AN6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_1:AN7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("pcb_1:MCUDI0")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_DEVICE_MEMBER("gearbox", FUNC(namcoio_gearbox_device::in_r))
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("pcb_1:MCUDI1")     /* 63B05Z0 - $3001 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_1:MCUDI2")     /* 63B05Z0 - $3002 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_1:MCUDI3")     /* 63B05Z0 - $3003 */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	// the side view screens do still read inputs, but it's more likely that the main screen should be transfering them
	// somehow rather than the controls being directly split
	PORT_START("pcb_0:DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "DSW2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x08, "Screen (DON'T CHANGE)")
	PORT_DIPSETTING(    0x0c, "Center (invalid)" )
	PORT_DIPSETTING(    0x08, "Left (correct)" )
	PORT_DIPSETTING(    0x04, "Right (invalid)" )
	PORT_DIPSETTING(    0x00, "Right (invalid) (duplicate)" )
	PORT_DIPNAME( 0x10, 0x10, "DSW5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "PCM ROM")
	PORT_DIPSETTING(    0x20, "2M" )
	PORT_DIPSETTING(    0x00, "4M" )
	PORT_DIPNAME( 0x40, 0x40, "DSW7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Screen Stop")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("pcb_0:MCUC")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:MCUB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:MCUH")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:AN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:AN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:AN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:AN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:AN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:AN5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:AN6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:AN7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:MCUDI0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:MCUDI1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:MCUDI2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_0:MCUDI3")

	PORT_START("pcb_2:DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "DSW2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x04, "Screen (DON'T CHANGE)")
	PORT_DIPSETTING(    0x0c, "Center (invalid)" )
	PORT_DIPSETTING(    0x08, "Left (invalid)" )
	PORT_DIPSETTING(    0x04, "Right (correct)" )
	PORT_DIPSETTING(    0x00, "Right (invalid) (duplicate)" )
	PORT_DIPNAME( 0x10, 0x10, "DSW5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "PCM ROM")
	PORT_DIPSETTING(    0x20, "2M" )
	PORT_DIPSETTING(    0x00, "4M" )
	PORT_DIPNAME( 0x40, 0x40, "DSW7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Screen Stop")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("pcb_2:MCUC")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:MCUB")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:MCUH")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:AN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:AN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:AN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:AN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:AN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:AN5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:AN6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:AN7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:MCUDI0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:MCUDI1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:MCUDI2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("pcb_2:MCUDI3")
INPUT_PORTS_END

/*
    Note, only the main screen PCB stack has voice roms populated
    the sound program also differs on the side screen sets

    pcb_0 = left
    pcb_1 = center
    pcb_2 = right
*/

ROM_START( driveyes )
	// pcb_1 - center
	ROM_REGION( 0x40000, "pcb_1:maincpu", 0 ) /* C68C - 68k code */
	ROM_LOAD16_BYTE( "de2-mp-ub.3j", 0x000000, 0x20000, CRC(f9c86fb5) SHA1(b48d16e8f26e7a2cfecb30285b517c42e5585ac7) )
	ROM_LOAD16_BYTE( "de2-mp-lb.1j", 0x000001, 0x20000, CRC(11d8587a) SHA1(ecb1e8fe2ba56b6f6a71a5552d5663b597165786) )

	ROM_REGION( 0x40000, "pcb_1:slave", 0 ) /* C68 - 68k code */
	ROM_LOAD16_BYTE( "de1-sp-ub.6c", 0x000000, 0x20000, CRC(231b144f) SHA1(42518614cb083455dc5fec71e699403907ca784b) )
	ROM_LOAD16_BYTE( "de1-sp-lb.4c", 0x000001, 0x20000, CRC(50cb9f59) SHA1(aec7fa080854f0297d9e90e3aaeb0f332fd579bd) )

	ROM_REGION( 0x20000, "pcb_1:audiocpu", 0 ) /* Sound */
	ROM_LOAD( "de1-snd0.8j",  0x000000, 0x020000, CRC(5474f203) SHA1(e0ae2f6978deb0c934d9311a334a6e36bb402aee) ) /* correct for center view */

	ROM_REGION16_BE( 0x400000, "pcb_1:c140", ROMREGION_ERASE00 ) /* sound samples - populated for center view only */
	ROM_LOAD16_BYTE("de1-voi0.12b", 0x080000, 0x40000, CRC(fc44adbd) SHA1(4268bb1f025e47a94212351d1c1cfd0e5029221f) )
	ROM_LOAD16_BYTE("de1-voi1.12c", 0x180000, 0x40000, CRC(a71dc55a) SHA1(5e746184db9144ab4e3a97b20195b92b0f56c8cc) )
	ROM_LOAD16_BYTE("de1-voi2.12d", 0x280000, 0x40000, CRC(4d32879a) SHA1(eae65f4b98cee9efe4e5dad7298c3717cfb1e6bf) )
	ROM_LOAD16_BYTE("de1-voi3.12e", 0x380000, 0x40000, CRC(e4832d18) SHA1(0460c79d3942aab89a765b0bd8bbddaf19a6d682) )

	ROM_REGION( 0x8000, "pcb_1:c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x200000, "pcb_1:c355spr", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "de1-obj0.5s", 0x000000, 0x40000, CRC(7438bd53) SHA1(7619c4b56d5c466e845eb45e6157dcaf2a03ad94) )
	ROM_LOAD32_BYTE( "de1-obj1.5x", 0x000001, 0x40000, CRC(45f2334e) SHA1(95f277a4e43d6662ae44d6b69a57f65c72978319) )
	ROM_LOAD32_BYTE( "de1-obj2.3s", 0x000002, 0x40000, CRC(8f1a542c) SHA1(2cb59713607d8929815a9b28bf2a384b6a6c9db8) )
	ROM_LOAD32_BYTE( "de1-obj3.3x", 0x000003, 0x40000, CRC(fc94544c) SHA1(6297445c64784ee253716f6438d98e5fcd4e7520) )
	ROM_LOAD32_BYTE( "de1-obj4.4s", 0x100000, 0x40000, CRC(335f0ea4) SHA1(9ec065d99ad0874b262b372334179a7e7612558e) )
	ROM_LOAD32_BYTE( "de1-obj5.4x", 0x100001, 0x40000, CRC(9e22999c) SHA1(02624186c359b5e2c96cd3f0e2cb1598ea36dff7) )
	ROM_LOAD32_BYTE( "de1-obj6.2s", 0x100002, 0x40000, CRC(346df4d5) SHA1(edbadb9db93b7f5a3b064c7f6acb77001cdacce2) )
	ROM_LOAD32_BYTE( "de1-obj7.2x", 0x100003, 0x40000, CRC(9ce325d7) SHA1(de4d788bec14842507ed405244974b4fd4f07515) )

	ROM_REGION16_BE( 0x100000, "pcb_1:data", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "de1-data-u.3a",  0x00000, 0x80000, CRC(fe65d2ab) SHA1(dbe962dda7efa60357fa3a684a265aaad49df5b5) )
	ROM_LOAD16_BYTE( "de1-data-l.1a",  0x00001, 0x80000, CRC(9bb37aca) SHA1(7f5dffc95cadcf12f53ff7944920afc25ed3cf68) )

	ROM_REGION16_BE( 0xc0000, "pcb_1:namcos21dsp:point16", 0 ) /* 3d objects */
	ROM_LOAD16_BYTE( "de1-pt0-ub.8j", 0x00000, 0x20000, CRC(3b6b746d) SHA1(40c992ef4cf5187b30aba42c5fe7ce0f8f02bee0) )
	ROM_LOAD16_BYTE( "de1-pt0-lb.8d", 0x00001, 0x20000, CRC(9c5c477e) SHA1(c8ae8a663227d636d35bd5f432d23f05d6695942) )
	ROM_LOAD16_BYTE( "de1-pt1-u.8l",  0x40000, 0x20000, CRC(23bc72a1) SHA1(083e2955ae2f88d1ad461517b47054d64375b46e) )
	ROM_LOAD16_BYTE( "de1-pt1-l.8e",  0x40001, 0x20000, CRC(a05ee081) SHA1(1be4c61ad716abb809856e04d4bb450943706a55) )
	ROM_LOAD16_BYTE( "de1-pt2-u.5n",  0x80000, 0x20000, CRC(10e83d81) SHA1(446fedc3b1e258a39fb9467e5327c9f9a9f1ac3f) )
	ROM_LOAD16_BYTE( "de1-pt2-l.7n",  0x80001, 0x20000, CRC(3339a976) SHA1(c9eb9c04f7b3f2a85e5ab64ffb2fe4fcfb6c494b) )

	ROM_REGION( 0x2000, "pcb_1:nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "nvram", 0x0000, 0x2000, CRC(fa6623e9) SHA1(8c313f136724eb6c829261b223a2ac1fc08d00c2) )

	// pcb_0 - left
	ROM_REGION( 0x40000, "pcb_0:maincpu", 0 ) /* C68C - 68k code */
	ROM_LOAD16_BYTE( "de2-mp-ub.3j", 0x000000, 0x20000, CRC(f9c86fb5) SHA1(b48d16e8f26e7a2cfecb30285b517c42e5585ac7) )
	ROM_LOAD16_BYTE( "de2-mp-lb.1j", 0x000001, 0x20000, CRC(11d8587a) SHA1(ecb1e8fe2ba56b6f6a71a5552d5663b597165786) )

	ROM_REGION( 0x40000, "pcb_0:slave", 0 ) /* C68 - 68k code */
	ROM_LOAD16_BYTE( "de1-sp-ub.6c", 0x000000, 0x20000, CRC(231b144f) SHA1(42518614cb083455dc5fec71e699403907ca784b) )
	ROM_LOAD16_BYTE( "de1-sp-lb.4c", 0x000001, 0x20000, CRC(50cb9f59) SHA1(aec7fa080854f0297d9e90e3aaeb0f332fd579bd) )

	ROM_REGION( 0x20000, "pcb_0:audiocpu", 0 ) /* Sound */
	ROM_LOAD( "de1-snd0r.8j", 0x000000, 0x020000, CRC(7bbeda42) SHA1(fe840cc9069758928492bbeec79acded18daafd9) ) // correct for left & right views

	ROM_REGION16_BE( 0x400000, "pcb_0:c140", ROMREGION_ERASE00 ) /* sound samples */
	/* unpopulated for left / right views */

	ROM_REGION( 0x8000, "pcb_0:c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x200000, "pcb_0:c355spr", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "de1-obj0.5s", 0x000000, 0x40000, CRC(7438bd53) SHA1(7619c4b56d5c466e845eb45e6157dcaf2a03ad94) )
	ROM_LOAD32_BYTE( "de1-obj1.5x", 0x000001, 0x40000, CRC(45f2334e) SHA1(95f277a4e43d6662ae44d6b69a57f65c72978319) )
	ROM_LOAD32_BYTE( "de1-obj2.3s", 0x000002, 0x40000, CRC(8f1a542c) SHA1(2cb59713607d8929815a9b28bf2a384b6a6c9db8) )
	ROM_LOAD32_BYTE( "de1-obj3.3x", 0x000003, 0x40000, CRC(fc94544c) SHA1(6297445c64784ee253716f6438d98e5fcd4e7520) )
	ROM_LOAD32_BYTE( "de1-obj4.4s", 0x100000, 0x40000, CRC(335f0ea4) SHA1(9ec065d99ad0874b262b372334179a7e7612558e) )
	ROM_LOAD32_BYTE( "de1-obj5.4x", 0x100001, 0x40000, CRC(9e22999c) SHA1(02624186c359b5e2c96cd3f0e2cb1598ea36dff7) )
	ROM_LOAD32_BYTE( "de1-obj6.2s", 0x100002, 0x40000, CRC(346df4d5) SHA1(edbadb9db93b7f5a3b064c7f6acb77001cdacce2) )
	ROM_LOAD32_BYTE( "de1-obj7.2x", 0x100003, 0x40000, CRC(9ce325d7) SHA1(de4d788bec14842507ed405244974b4fd4f07515) )

	ROM_REGION16_BE( 0x100000, "pcb_0:data", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "de1-data-u.3a",  0x00000, 0x80000, CRC(fe65d2ab) SHA1(dbe962dda7efa60357fa3a684a265aaad49df5b5) )
	ROM_LOAD16_BYTE( "de1-data-l.1a",  0x00001, 0x80000, CRC(9bb37aca) SHA1(7f5dffc95cadcf12f53ff7944920afc25ed3cf68) )

	ROM_REGION16_BE( 0xc0000, "pcb_0:namcos21dsp:point16", 0 ) /* 3d objects */
	ROM_LOAD16_BYTE( "de1-pt0-ub.8j", 0x00000, 0x20000, CRC(3b6b746d) SHA1(40c992ef4cf5187b30aba42c5fe7ce0f8f02bee0) )
	ROM_LOAD16_BYTE( "de1-pt0-lb.8d", 0x00001, 0x20000, CRC(9c5c477e) SHA1(c8ae8a663227d636d35bd5f432d23f05d6695942) )
	ROM_LOAD16_BYTE( "de1-pt1-u.8l",  0x40000, 0x20000, CRC(23bc72a1) SHA1(083e2955ae2f88d1ad461517b47054d64375b46e) )
	ROM_LOAD16_BYTE( "de1-pt1-l.8e",  0x40001, 0x20000, CRC(a05ee081) SHA1(1be4c61ad716abb809856e04d4bb450943706a55) )
	ROM_LOAD16_BYTE( "de1-pt2-u.5n",  0x80000, 0x20000, CRC(10e83d81) SHA1(446fedc3b1e258a39fb9467e5327c9f9a9f1ac3f) )
	ROM_LOAD16_BYTE( "de1-pt2-l.7n",  0x80001, 0x20000, CRC(3339a976) SHA1(c9eb9c04f7b3f2a85e5ab64ffb2fe4fcfb6c494b) )

	ROM_REGION( 0x2000, "pcb_0:nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "nvram", 0x0000, 0x2000, CRC(fa6623e9) SHA1(8c313f136724eb6c829261b223a2ac1fc08d00c2) )

	// pcb_2 - right
	ROM_REGION( 0x40000, "pcb_2:maincpu", 0 ) /* C68C - 68k code */
	ROM_LOAD16_BYTE( "de2-mp-ub.3j", 0x000000, 0x20000, CRC(f9c86fb5) SHA1(b48d16e8f26e7a2cfecb30285b517c42e5585ac7) )
	ROM_LOAD16_BYTE( "de2-mp-lb.1j", 0x000001, 0x20000, CRC(11d8587a) SHA1(ecb1e8fe2ba56b6f6a71a5552d5663b597165786) )

	ROM_REGION( 0x40000, "pcb_2:slave", 0 ) /* C68 - 68k code */
	ROM_LOAD16_BYTE( "de1-sp-ub.6c", 0x000000, 0x20000, CRC(231b144f) SHA1(42518614cb083455dc5fec71e699403907ca784b) )
	ROM_LOAD16_BYTE( "de1-sp-lb.4c", 0x000001, 0x20000, CRC(50cb9f59) SHA1(aec7fa080854f0297d9e90e3aaeb0f332fd579bd) )

	ROM_REGION( 0x20000, "pcb_2:audiocpu", 0 ) /* Sound */
	ROM_LOAD( "de1-snd0r.8j", 0x000000, 0x020000, CRC(7bbeda42) SHA1(fe840cc9069758928492bbeec79acded18daafd9) ) // correct for left & right views

	ROM_REGION16_BE( 0x400000, "pcb_2:c140", ROMREGION_ERASE00 ) /* sound samples */
	/* unpopulated for left / right views */

	ROM_REGION( 0x8000, "pcb_2:c68mcu:external", ROMREGION_ERASE00 ) /* C68 (M37450) I/O MCU program */
	/* external ROM not populated, unclear how it would map */

	ROM_REGION( 0x200000, "pcb_2:c355spr", 0 ) /* sprites */
	ROM_LOAD32_BYTE( "de1-obj0.5s", 0x000000, 0x40000, CRC(7438bd53) SHA1(7619c4b56d5c466e845eb45e6157dcaf2a03ad94) )
	ROM_LOAD32_BYTE( "de1-obj1.5x", 0x000001, 0x40000, CRC(45f2334e) SHA1(95f277a4e43d6662ae44d6b69a57f65c72978319) )
	ROM_LOAD32_BYTE( "de1-obj2.3s", 0x000002, 0x40000, CRC(8f1a542c) SHA1(2cb59713607d8929815a9b28bf2a384b6a6c9db8) )
	ROM_LOAD32_BYTE( "de1-obj3.3x", 0x000003, 0x40000, CRC(fc94544c) SHA1(6297445c64784ee253716f6438d98e5fcd4e7520) )
	ROM_LOAD32_BYTE( "de1-obj4.4s", 0x100000, 0x40000, CRC(335f0ea4) SHA1(9ec065d99ad0874b262b372334179a7e7612558e) )
	ROM_LOAD32_BYTE( "de1-obj5.4x", 0x100001, 0x40000, CRC(9e22999c) SHA1(02624186c359b5e2c96cd3f0e2cb1598ea36dff7) )
	ROM_LOAD32_BYTE( "de1-obj6.2s", 0x100002, 0x40000, CRC(346df4d5) SHA1(edbadb9db93b7f5a3b064c7f6acb77001cdacce2) )
	ROM_LOAD32_BYTE( "de1-obj7.2x", 0x100003, 0x40000, CRC(9ce325d7) SHA1(de4d788bec14842507ed405244974b4fd4f07515) )

	ROM_REGION16_BE( 0x100000, "pcb_2:data", 0 ) /* 68k */
	ROM_LOAD16_BYTE( "de1-data-u.3a",  0x00000, 0x80000, CRC(fe65d2ab) SHA1(dbe962dda7efa60357fa3a684a265aaad49df5b5) )
	ROM_LOAD16_BYTE( "de1-data-l.1a",  0x00001, 0x80000, CRC(9bb37aca) SHA1(7f5dffc95cadcf12f53ff7944920afc25ed3cf68) )

	ROM_REGION16_BE( 0xc0000, "pcb_2:namcos21dsp:point16", 0 ) /* 3d objects */
	ROM_LOAD16_BYTE( "de1-pt0-ub.8j", 0x00000, 0x20000, CRC(3b6b746d) SHA1(40c992ef4cf5187b30aba42c5fe7ce0f8f02bee0) )
	ROM_LOAD16_BYTE( "de1-pt0-lb.8d", 0x00001, 0x20000, CRC(9c5c477e) SHA1(c8ae8a663227d636d35bd5f432d23f05d6695942) )
	ROM_LOAD16_BYTE( "de1-pt1-u.8l",  0x40000, 0x20000, CRC(23bc72a1) SHA1(083e2955ae2f88d1ad461517b47054d64375b46e) )
	ROM_LOAD16_BYTE( "de1-pt1-l.8e",  0x40001, 0x20000, CRC(a05ee081) SHA1(1be4c61ad716abb809856e04d4bb450943706a55) )
	ROM_LOAD16_BYTE( "de1-pt2-u.5n",  0x80000, 0x20000, CRC(10e83d81) SHA1(446fedc3b1e258a39fb9467e5327c9f9a9f1ac3f) )
	ROM_LOAD16_BYTE( "de1-pt2-l.7n",  0x80001, 0x20000, CRC(3339a976) SHA1(c9eb9c04f7b3f2a85e5ab64ffb2fe4fcfb6c494b) )

	ROM_REGION( 0x2000, "pcb_2:nvram", 0 ) /* default settings, including calibration */
	ROM_LOAD( "nvram", 0x0000, 0x2000, CRC(fa6623e9) SHA1(8c313f136724eb6c829261b223a2ac1fc08d00c2) )
ROM_END


/*    YEAR  NAME       PARENT    MACHINE   INPUT       CLASS           INIT           MONITOR  COMPANY  FULLNAME                                 FLAGS */

// 3 PCB stacks in a single cage (3x 4 PCBs) linked for 3 screen panorama, boards look similar to original Namco System 21 (not 21B) including TMS320C25 DSP, but use C68 I/O MCU and sprite chip instead of "68000 'GPU'" ?
GAME( 1992, driveyes,  0,        driveyes, driveyes,   namcos21_de_state, empty_init,    ROT0,    "Namco", "Driver's Eyes (Japan) (1992/01/10, Main Ver 2.1, Sub Ver 1.1)",                 MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN)

