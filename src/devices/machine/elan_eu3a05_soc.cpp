// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a05_soc.h"

DEFINE_DEVICE_TYPE(ELAN_EU3A05_SOC,     elan_eu3a05_soc_device,     "elan_eu3a05_soc_device",     "ELAN EU3A05")
DEFINE_DEVICE_TYPE(ELAN_EU3A13_SOC,     elan_eu3a13_soc_device,     "elan_eu3a13_soc_device",     "ELAN EU3A13")

elan_eu3a05_soc_device::elan_eu3a05_soc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock),
	m_extbus_config("extbus", ENDIANNESS_LITTLE, 8, 24),
	m_sys(*this, "sys"),
	m_gpio(*this, "gpio"),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_sound(*this, "eu3a05sound"),
	m_vid(*this, "vid"),
	m_palette(*this, "palette"),
	m_read_callback(*this, 0xff),
	m_write_callback(*this),
	m_is_pal(false),
	m_use_alt_timer(false)
{
	m_extbus_config.m_addr_width = 24;
	m_extbus_config.m_logaddr_width = 24;
	program_config.m_internal_map = address_map_constructor(FUNC(elan_eu3a05_soc_device::int_map), this);

	m_fixed_bank_address = 0x3f8000;
}

elan_eu3a05_soc_device::elan_eu3a05_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	elan_eu3a05_soc_device(mconfig, ELAN_EU3A05_SOC, tag, owner, clock)
{
}

elan_eu3a13_soc_device::elan_eu3a13_soc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	elan_eu3a05_soc_device(mconfig, ELAN_EU3A13_SOC, tag, owner, clock)
{
	m_fixed_bank_address = 0;
}


static const gfx_layout helper_4bpp_8_layout =
{
	8,1,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ STEP8(0,4) },
	{ 0 },
	8 * 4
};

static const gfx_layout helper_8bpp_8_layout =
{
	8,1,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,8) },
	{ 0 },
	8 * 8
};


// these are fake just to make looking at the texture pages easier
static const uint32_t texlayout_xoffset_8bpp[256] = { STEP256(0,8) };
static const uint32_t texlayout_yoffset_8bpp[256] = { STEP256(0,256*8) };
static const gfx_layout texture_helper_8bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*8,
	texlayout_xoffset_8bpp,
	texlayout_yoffset_8bpp
};

static const uint32_t texlayout_xoffset_4bpp[256] = { STEP256(0,4) };
static const uint32_t texlayout_yoffset_4bpp[256] = { STEP256(0,256*4) };
static const gfx_layout texture_helper_4bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*4,
	texlayout_xoffset_4bpp,
	texlayout_yoffset_4bpp
};

static GFXDECODE_START( gfx_elan_eu3a05_fake )
	// TODO: how do we avoid the direct region reference here? (this is only used for debugging)
	GFXDECODE_ENTRY( ":maincpu", 0, helper_4bpp_8_layout,  0x0, 1  )
	GFXDECODE_ENTRY( ":maincpu", 0, texture_helper_4bpp_layout,  0x0, 1  )
	GFXDECODE_ENTRY( ":maincpu", 0, helper_8bpp_8_layout,  0x0, 1  )
	GFXDECODE_ENTRY( ":maincpu", 0, texture_helper_8bpp_layout,  0x0, 1  )
GFXDECODE_END


void elan_eu3a05_soc_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette).set_entries(256);

	ELAN_EU3A05_GPIO(config, m_gpio, 0);
	m_gpio->read_callback<0>().set(FUNC(elan_eu3a05_soc_device::port_r<0>));
	m_gpio->write_callback<0>().set(FUNC(elan_eu3a05_soc_device::port_w<0>));
	m_gpio->read_callback<1>().set(FUNC(elan_eu3a05_soc_device::port_r<1>));
	m_gpio->write_callback<1>().set(FUNC(elan_eu3a05_soc_device::port_w<1>));
	m_gpio->read_callback<2>().set(FUNC(elan_eu3a05_soc_device::port_r<2>));
	m_gpio->write_callback<2>().set(FUNC(elan_eu3a05_soc_device::port_w<2>));

	ELAN_EU3A05_SYS(config, m_sys, 0);
	m_sys->set_cpu(DEVICE_SELF);
	m_sys->bank_change_callback().set(FUNC(elan_eu3a05_soc_device::bank_change));

	ELAN_EU3A05_VID(config, m_vid, 0);
	m_vid->set_cpu(DEVICE_SELF);
	m_vid->set_palette(m_palette);
	m_vid->set_entries(256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ELAN_EU3A05_SOUND(config, m_sound, 8000);
	m_sound->space_read_callback().set(FUNC(elan_eu3a05_soc_device::read_full_space));
	m_sound->add_route(ALL_OUTPUTS, "mono", 1.0);
	/* just causes select sound to loop in Tetris for now!
	m_sound->sound_end_cb<0>().set(FUNC(elan_eu3a05_state::sound_end0));
	m_sound->sound_end_cb<1>().set(FUNC(elan_eu3a05_state::sound_end1));
	m_sound->sound_end_cb<2>().set(FUNC(elan_eu3a05_state::sound_end2));
	m_sound->sound_end_cb<3>().set(FUNC(elan_eu3a05_state::sound_end3));
	m_sound->sound_end_cb<4>().set(FUNC(elan_eu3a05_state::sound_end4));
	m_sound->sound_end_cb<5>().set(FUNC(elan_eu3a05_state::sound_end5));
	*/

	GFXDECODE(config, "gfxdecode", m_palette, gfx_elan_eu3a05_fake);
}

void elan_eu3a13_soc_device::device_add_mconfig(machine_config &config)
{
	elan_eu3a05_soc_device::device_add_mconfig(config);

	ELAN_EU3A13_VID(config.replace(), m_vid, 0);
	m_vid->set_cpu(DEVICE_SELF);
	m_vid->set_palette(m_palette);
	m_vid->set_entries(256);

	ELAN_EU3A13_SYS(config.replace(), m_sys, 0);
	m_sys->set_cpu(DEVICE_SELF);
	m_sys->bank_change_callback().set(FUNC(elan_eu3a13_soc_device::bank_change));
}

device_memory_interface::space_config_vector elan_eu3a05_soc_device::memory_space_config() const
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

void elan_eu3a05_soc_device::device_reset()
{
	m6502_device::device_reset();

	/* the 6502 core sets the default stack value to 0x01bd
	   and Tetris does not initialize it to anything else

	   Tetris stores the playfield data at 0x100 - 0x1c7 and
	   has a clear routine that will erase that range and
	   trash the stack

	   It seems likely this 6502 sets it to 0x1ff by default
	   at least.

	   According to
	   http://mametesters.org/view.php?id=6486
	   this isn't right for known 6502 types either
	*/
	set_state_int(M6502_S, 0x1ff);

	if (m_is_pal)
		m_sys->set_pal();

	if (m_use_alt_timer)
		m_sys->set_alt_timer();
}

void elan_eu3a05_soc_device::device_start()
{
	m6502_device::device_start();
}

void elan_eu3a05_soc_device::int_map(address_map &map)
{
	map(0x0000, 0x3fff).ram();

	// can the addresses move around?
	map(0x4800, 0x49ff).rw(m_vid, FUNC(elan_eu3a05commonvid_device::palette_r), FUNC(elan_eu3a05commonvid_device::palette_w));

	map(0x5000, 0x501f).m(m_sys, FUNC(elan_eu3a05sys_device::map)); // including DMA controller
	map(0x5020, 0x503f).m(m_vid, FUNC(elan_eu3a05vid_device::map));

	// 504x GPIO area?
	map(0x5040, 0x5046).rw(m_gpio, FUNC(elan_eu3a05gpio_device::gpio_r), FUNC(elan_eu3a05gpio_device::gpio_w));
	// 5047
	map(0x5048, 0x504a).w(m_gpio, FUNC(elan_eu3a05gpio_device::gpio_unk_w));

	// 506x unknown
	map(0x5060, 0x506d).ram(); // read/written by tetris (ADC?)

	// 508x sound
	map(0x5080, 0x50bf).m(m_sound, FUNC(elan_eu3a05_sound_device::map));

	//map(0x5000, 0x50ff).ram();
	map(0x6000, 0xdfff).rw(FUNC(elan_eu3a05_soc_device::bank_r), FUNC(elan_eu3a05_soc_device::bank_w));

	map(0xe000, 0xffff).r(FUNC(elan_eu3a05_soc_device::fixed_r));
	// not sure how these work, might be a modified 6502 core instead.
	map(0xfffa, 0xfffb).r(m_sys, FUNC(elan_eu3a05commonsys_device::nmi_vector_r)); // custom vectors handled with NMI for now
	//map(0xfffe, 0xffff).r(m_sys, FUNC(elan_eu3a05commonsys_device::irq_vector_r));  // allow normal IRQ for brk
}
