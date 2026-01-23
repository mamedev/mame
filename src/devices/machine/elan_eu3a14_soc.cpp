// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a14_soc.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(ELAN_EU3A14_SOC,     elan_eu3a14_soc_device,     "elan_eu3a14_soc_device",     "ELAN EU3A14")

elan_eu3a14_soc_device::elan_eu3a14_soc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock),
	m_extbus_config("extbus", ENDIANNESS_LITTLE, 8, 24),
	m_sys(*this, "sys"),
	m_sound(*this, "eu3a05sound"),
	m_vid(*this, "commonvid"),
	m_palette(*this, "palette"),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_read_callback(*this, 0xff),
	m_write_callback(*this),
	m_default_spriteramaddr(0),
	m_default_tileramaddr(0),
	m_disable_timer(false),
	m_is_pal(false)
{
	m_extbus_config.m_addr_width = 24;
	m_extbus_config.m_logaddr_width = 24;
	program_config.m_internal_map = address_map_constructor(FUNC(elan_eu3a14_soc_device::int_map), this);
}

elan_eu3a14_soc_device::elan_eu3a14_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	elan_eu3a14_soc_device(mconfig, ELAN_EU3A14_SOC, tag, owner, clock)
{
}

// background
static const gfx_layout helper16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16 * 16 * 8
};

static const gfx_layout helper16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP16(0,4) },
	{ STEP16(0,16*4) },
	16 * 16 * 4
};

static const gfx_layout helper8x8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8 * 8 * 8
};

static GFXDECODE_START( gfx_helper )
	// TODO: how do we avoid the direct region reference here? (this is only used for debugging)
	GFXDECODE_ENTRY( ":maincpu", 0, helper16x16x8_layout,  0x0, 2  )
	GFXDECODE_ENTRY( ":maincpu", 0, helper16x16x4_layout,  0x0, 32  )
	GFXDECODE_ENTRY( ":maincpu", 0, helper8x8x8_layout,    0x0, 2  )
	GFXDECODE_ENTRY( ":maincpu", 0, gfx_8x8x4_packed_msb,  0x0, 32  )
GFXDECODE_END


void elan_eu3a14_soc_device::device_add_mconfig(machine_config &config)
{
	ELAN_EU3A14_SYS(config, m_sys, 0);
	m_sys->set_cpu(DEVICE_SELF);
	m_sys->bank_change_callback().set(FUNC(elan_eu3a14_soc_device::bank_change));

	/* video hardware */

	PALETTE(config, m_palette).set_entries(512);

	ELAN_EU3A14_VID(config, m_vid, 0);
	m_vid->set_cpu(DEVICE_SELF);
	m_vid->set_palette(m_palette);
	m_vid->set_entries(512);
	m_vid->set_tilerambase(0);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ELAN_EU3A05_SOUND(config, m_sound, 8000);
	m_sound->space_read_callback().set(FUNC(elan_eu3a14_soc_device::read_full_space));
	m_sound->add_route(ALL_OUTPUTS, "mono", 1.0);
	m_sound->sound_end_cb<0>().set(FUNC(elan_eu3a14_soc_device::sound_end0));
	m_sound->sound_end_cb<1>().set(FUNC(elan_eu3a14_soc_device::sound_end1));
	m_sound->sound_end_cb<2>().set(FUNC(elan_eu3a14_soc_device::sound_end2));
	m_sound->sound_end_cb<3>().set(FUNC(elan_eu3a14_soc_device::sound_end3));
	m_sound->sound_end_cb<4>().set(FUNC(elan_eu3a14_soc_device::sound_end4));
	m_sound->sound_end_cb<5>().set(FUNC(elan_eu3a14_soc_device::sound_end5));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_helper);
}

device_memory_interface::space_config_vector elan_eu3a14_soc_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config),
			std::make_pair(AS_OPCODES, &sprogram_config),
			std::make_pair(AS_EXTERNAL, &m_extbus_config),
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config),
			std::make_pair(AS_EXTERNAL, &m_extbus_config),
		};
}

void elan_eu3a14_soc_device::device_reset()
{
	m6502_device::device_reset();

	m_current_bank = 0x01;

	m_portdir[0] = 0x00;
	m_portdir[1] = 0x00;
	m_portdir[2] = 0x00;

	// see note in eu3a05, probably applies here too but nothing depends on it
	set_state_int(M6502_S, 0x1ff);

	if (m_is_pal)
		m_sys->set_pal();

	// pass per-game kludges to subdevices
	m_vid->set_default_spriteramaddr(m_default_spriteramaddr);
	m_vid->set_tilerambase(m_default_tileramaddr);
	if (m_disable_timer)
		m_sys->disable_timer_irq();
}

void elan_eu3a14_soc_device::device_start()
{
	m6502_device::device_start();

	m_vid->create_bitmaps(m_screen);
}

void elan_eu3a14_soc_device::int_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x3fff).ram().share("mainram"); // 200-9ff is sprites? a00 - ??? is tilemap?

	map(0x4800, 0x4bff).rw(m_vid, FUNC(elan_eu3a14vid_device::palette_r), FUNC(elan_eu3a14vid_device::palette_w));

	map(0x5000, 0x501f).m(m_sys, FUNC(elan_eu3a14sys_device::map)); // including DMA controller

	// probably GPIO like eu3a05, although it access 47/48 as unknown instead of 48/49/4a
	map(0x5040, 0x5040).w(FUNC(elan_eu3a14_soc_device::porta_dir_w));
	map(0x5041, 0x5041).rw(FUNC(elan_eu3a14_soc_device::porta_dat_r), FUNC(elan_eu3a14_soc_device::porta_dat_w));
	map(0x5042, 0x5042).w(FUNC(elan_eu3a14_soc_device::portb_dir_w));
	map(0x5043, 0x5043).rw(FUNC(elan_eu3a14_soc_device::portb_dat_r), FUNC(elan_eu3a14_soc_device::portb_dat_w));
	map(0x5044, 0x5044).w(FUNC(elan_eu3a14_soc_device::portc_dir_w));
	map(0x5045, 0x5045).rw(FUNC(elan_eu3a14_soc_device::portc_dat_r), FUNC(elan_eu3a14_soc_device::portc_dat_w));

	map(0x5046, 0x5046).nopw();
	map(0x5047, 0x5047).nopw();
	map(0x5048, 0x5048).nopw();

	// 5060 - 506e  r/w during startup on foot (adc?)

	// 0x5080 - 50bf = SOUND AREA (same as eu5a03?)
	map(0x5080, 0x50bf).m(m_sound, FUNC(elan_eu3a05_sound_device::map));

	// 0x5100 - 517f = VIDEO AREA
	map(0x5100, 0x517f).m(m_vid, FUNC(elan_eu3a14vid_device::map));

	map(0x6000, 0xdfff).rw(FUNC(elan_eu3a14_soc_device::bank_r), FUNC(elan_eu3a14_soc_device::bank_w));

	map(0xe000, 0xffff).r(FUNC(elan_eu3a14_soc_device::fixed_r));

	map(0xfffa, 0xfffb).r(m_sys, FUNC(elan_eu3a05commonsys_device::nmi_vector_r)); // custom vectors handled with NMI for now
	//map(0xfffe, 0xffff).r(m_sys, FUNC(elan_eu3a05commonsys_device::irq_vector_r));  // allow normal IRQ for brk

}
