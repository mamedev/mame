// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 Mattel Intellivision Entertainment Computer System expansion emulation

 TODO:
   - Make paged rom emulation more accurate (according to
   http://spatula-city.org/~im14u2c/intv/tech/ecs.html
   writes to $xa5y should be available for every x and every y, i.e. we shall
   have writes to every 4K chunk of the memory map, and there shall be 16 pages
   for each)
   Current emulation is instead tailored around the minimal usage necessary to
   make the main expansion and World Series Major League Baseball happy

 ***********************************************************************************************************/


#include "emu.h"
#include "ecs.h"
#include "speaker.h"


//-------------------------------------------------
//  intv_ecs_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(INTV_ROM_ECS, intv_ecs_device, "intv_ecs", "Intellivision ECS Expansion")

intv_ecs_device::intv_ecs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: intv_rom_device(mconfig, INTV_ROM_ECS, tag, owner, clock),
	m_snd(*this, "ay8914"),
	m_subslot(*this, "subslot"),
	m_voice_enabled(false),
	m_ramd0_enabled(false),
	m_ram88_enabled(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void intv_ecs_device::device_start()
{
	// if the ECS is mounted directly in the system, use device rom and alloc RAM
	if (m_rom == nullptr)
	{
		std::string region_tag;
		m_rom = memregion(region_tag.assign(tag()).append(":ecs").c_str())->base();
	}
	if (m_ram.empty())
	{
		m_ram.resize(0x800);
	}

	save_item(NAME(m_bank_base));
}

void intv_ecs_device::device_reset()
{
	memset(m_bank_base, 0, sizeof(m_bank_base));
}

void intv_ecs_device::late_subslot_setup()
{
	switch (m_subslot->get_type())
	{
		case INTV_RAM:
			m_ramd0_enabled = true;
			break;
		case INTV_GFACT:
			m_ram88_enabled = true;
			break;
		case INTV_VOICE:
			m_voice_enabled = true;
			m_subslot->late_subslot_setup();
			break;
		case INTV_ECS:
			printf("WARNING: You cannot connect serially multiple ECS units.\n");
			printf("WARNING: Emulation will likely misbehave.\n");
			break;
		case INTV_KEYCOMP:
			printf("WARNING: You cannot connect the Keyboard component to the ECS unit.\n");
			printf("WARNING: Emulation will likely misbehave.\n");
			break;
	}
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------


void intv_ecs_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono_ecs").front_center();

	AY8914(config, m_snd, XTAL(3'579'545)/2);
	m_snd->port_a_read_callback().set("ctrl_port", FUNC(intvecs_control_port_device::portA_r));
	m_snd->port_b_read_callback().set("ctrl_port", FUNC(intvecs_control_port_device::portB_r));
	m_snd->port_a_write_callback().set("ctrl_port", FUNC(intvecs_control_port_device::portA_w));
	m_snd->add_route(ALL_OUTPUTS, "mono_ecs", 0.33);

	INTVECS_CONTROL_PORT(config, "ctrl_port", intvecs_control_port_devices, "keybd");
	INTV_CART_SLOT(config, m_subslot, intv_cart, nullptr);
}


ROM_START( ecs )
	ROM_REGION( 0x20000, "ecs", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ecs_rom.20", 0x04000, 0x2000, CRC(5f9d05e5) SHA1(083a3e7405b8f8b4b8a5003ca9c31b8d824b535c))
	ROM_LOAD16_WORD_SWAP( "ecs_rom.70", 0x0e000, 0x2000, CRC(46bb1f48) SHA1(eda44a8476fdada1ae90fba0d0287611e2efa074))
	ROM_LOAD16_WORD_SWAP( "ecs_rom.e0", 0x1c000, 0x2000, CRC(c2ebcd90) SHA1(b3c14955f56c57e6f0d8fbb695771946cfcf6582))
ROM_END

const tiny_rom_entry *intv_ecs_device::device_rom_region() const
{
	return ROM_NAME( ecs );
}


/*-------------------------------------------------
 Paged ROM handling
 -------------------------------------------------*/

READ16_MEMBER(intv_ecs_device::read_rom20)
{
	if (m_bank_base[2])
		return INTV_ROM16_READ(offset + 0x2000);
	else
		return 0xffff;
}

READ16_MEMBER(intv_ecs_device::read_rom70)
{
	if (m_bank_base[7])
		return 0xffff;
	else
		return INTV_ROM16_READ(offset + 0x7000);
}

READ16_MEMBER(intv_ecs_device::read_rome0)
{
	if (m_bank_base[14])
		return INTV_ROM16_READ(offset + 0xe000);
	else    // if WSMLB is loaded, it shall go here, otherwise 0xffff
		return m_subslot->read_rome0(space, offset, mem_mask);
}

READ16_MEMBER(intv_ecs_device::read_romf0)
{
	// only WSMLB should come here with bank_base = 1
	if (m_bank_base[15])
		return m_subslot->read_romf0(space, offset + 0x1000, mem_mask);
	else
		return m_subslot->read_romf0(space, offset, mem_mask);
}


/*-------------------------------------------------
 read_audio
 -------------------------------------------------*/

READ16_MEMBER(intv_ecs_device::read_ay)
{
	if (ACCESSING_BITS_0_7)
		return m_snd->read(offset);
	else
		return 0xffff;
}

/*-------------------------------------------------
 write_audio
 -------------------------------------------------*/

WRITE16_MEMBER(intv_ecs_device::write_ay)
{
	if (ACCESSING_BITS_0_7)
		return m_snd->write(offset, data);
}


READ16_MEMBER(intv_ecs_device::read_rom80)
{
	if (m_ram88_enabled && offset >= 0x800)
		return m_subslot->read_ram(space, offset & 0x7ff, mem_mask);
	else
		return m_subslot->read_rom80(space, offset, mem_mask);
}


READ16_MEMBER(intv_ecs_device::read_romd0)
{
	if (m_ramd0_enabled && offset < 0x800)
		return m_subslot->read_ram(space, offset, mem_mask);
	else
		return m_subslot->read_romd0(space, offset, mem_mask);
}


WRITE16_MEMBER(intv_ecs_device::write_rom20)
{
	if (offset == 0xfff)
	{
		if (data == 0x2a50)
			m_bank_base[2] = 0;
		else if (data == 0x2a51)
			m_bank_base[2] = 1;
	}
}

WRITE16_MEMBER(intv_ecs_device::write_rom70)
{
	if (offset == 0xfff)
	{
		if (data == 0x7a50)
			m_bank_base[7] = 0;
		else if (data == 0x7a51)
			m_bank_base[7] = 1;
	}
}

WRITE16_MEMBER(intv_ecs_device::write_rome0)
{
	if (offset == 0xfff)
	{
		if (data == 0xea50)
			m_bank_base[14] = 0;
		else if (data == 0xea51)
			m_bank_base[14] = 1;
	}
}

WRITE16_MEMBER(intv_ecs_device::write_romf0)
{
	if (offset == 0xfff)
	{
		if (data == 0xfa50)
			m_bank_base[15] = 0;
		else if (data == 0xfa51)
			m_bank_base[15] = 1;
	}
}
