// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_ep3a19a_soc.h"

DEFINE_DEVICE_TYPE(ELAN_EP3A19A_SOC,     elan_ep3a19a_cpu_device,     "elan_ep3a19a_cpu_device",     "ELAN EP3A19A (NTSC)")

elan_ep3a19a_cpu_device::elan_ep3a19a_cpu_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock) :
	m6502_device(mconfig, type, tag, owner, clock),
	m_extbus_config("extbus", ENDIANNESS_LITTLE, 8, 24),
	m_sys(*this, "sys"),
	m_gpio(*this, "gpio"),
	m_screen(*this, ":screen"),
	m_ram(*this, "ram"),
	m_sound(*this, "eu3a05sound"),
	m_vid(*this, "vid"),
	m_palette(*this, "palette")
{
	m_extbus_config.m_addr_width = 24;
	m_extbus_config.m_logaddr_width = 24;
	program_config.m_internal_map = address_map_constructor(FUNC(elan_ep3a19a_cpu_device::int_map), this);
}

elan_ep3a19a_cpu_device::elan_ep3a19a_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	elan_ep3a19a_cpu_device(mconfig, ELAN_EP3A19A_SOC, tag, owner, clock)
{
}

void elan_ep3a19a_cpu_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette).set_entries(256);

	ELAN_EU3A05_GPIO(config, m_gpio, 0);
	m_gpio->read_callback<0>().set_ioport(":IN0");
	m_gpio->read_callback<1>().set_ioport(":IN1");
	m_gpio->read_callback<2>().set_ioport(":IN2");

	ELAN_EP3A19A_SYS(config, m_sys, 0);
	m_sys->set_cpu(DEVICE_SELF);
	m_sys->bank_change_callback().set(FUNC(elan_ep3a19a_cpu_device::bank_change));

	ELAN_EP3A19A_VID(config, m_vid, 0);
	m_vid->set_cpu(DEVICE_SELF);
	m_vid->set_palette(m_palette);
	m_vid->set_entries(256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ELAN_EU3A05_SOUND(config, m_sound, 8000);
	m_sound->space_read_callback().set(FUNC(elan_ep3a19a_cpu_device::read_full_space));
	m_sound->add_route(ALL_OUTPUTS, "mono", 1.0);

	/*
	m_sound->sound_end_cb<0>().set(FUNC(elan_ep3a19a_cpu_device::sound_end0));
	m_sound->sound_end_cb<1>().set(FUNC(elan_ep3a19a_cpu_device::sound_end1));
	m_sound->sound_end_cb<2>().set(FUNC(elan_ep3a19a_cpu_device::sound_end2));
	m_sound->sound_end_cb<3>().set(FUNC(elan_ep3a19a_cpu_device::sound_end3));
	m_sound->sound_end_cb<4>().set(FUNC(elan_ep3a19a_cpu_device::sound_end4));
	m_sound->sound_end_cb<5>().set(FUNC(elan_ep3a19a_cpu_device::sound_end5));
	*/
}

device_memory_interface::space_config_vector elan_ep3a19a_cpu_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config),
			std::make_pair(AS_OPCODES, &sprogram_config),
			std::make_pair(5, &m_extbus_config),
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config),
			std::make_pair(5, &m_extbus_config),
		};
}

void elan_ep3a19a_cpu_device::device_reset()
{
	m6502_device::device_reset();

	// see note in eu3a05, probably applies here too but nothing depends on it
	set_state_int(M6502_S, 0x1ff);
}

void elan_ep3a19a_cpu_device::device_start()
{
	m6502_device::device_start();
}

void elan_ep3a19a_cpu_device::int_map(address_map &map)
{
	// can the addresses move around?
	map(0x0000, 0x3fff).ram().share("ram");
	map(0x4800, 0x49ff).rw(m_vid, FUNC(elan_eu3a05commonvid_device::palette_r), FUNC(elan_eu3a05commonvid_device::palette_w));

	map(0x5000, 0x5014).m(m_sys, FUNC(elan_ep3a19asys_device::map)); // including DMA controller
	map(0x5020, 0x503f).m(m_vid, FUNC(elan_eu3a05vid_device::map));

	// 504x GPIO area?
	map(0x5040, 0x5046).rw(m_gpio, FUNC(elan_eu3a05gpio_device::gpio_r), FUNC(elan_eu3a05gpio_device::gpio_w));
	// 5047
	//map(0x5048, 0x504a).w(m_gpio, FUNC(elan_eu3a05gpio_device::gpio_unk_w));

	// 506x unknown
	//map(0x5060, 0x506d).ram(); // read/written by tetris (ADC?)

	// 508x sound
	map(0x5080, 0x50bf).m(m_sound, FUNC(elan_eu3a05_sound_device::map));

	//map(0x5000, 0x50ff).ram();
	map(0x6000, 0xdfff).rw(FUNC(elan_ep3a19a_cpu_device::bank_r), FUNC(elan_ep3a19a_cpu_device::bank_w));

	map(0xe000, 0xffff).r(FUNC(elan_ep3a19a_cpu_device::fixed_r));
	// not sure how these work, might be a modified 6502 core instead.
	//map(0xfffa, 0xfffb).r(m_sys, FUNC(elan_eu3a05commonsys_device::nmi_vector_r)); // custom vectors handled with NMI for now
	map(0xfffa, 0xfffb).r(FUNC(elan_ep3a19a_cpu_device::nmi_vector_r)); // custom vectors handled with NMI for now

	//map(0xfffe, 0xffff).r(m_sys, FUNC(elan_eu3a05commonsys_device::irq_vector_r));  // allow normal IRQ for brk
}
