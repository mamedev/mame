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


//-------------------------------------------------
//  intv_ecs_device - constructor
//-------------------------------------------------

const device_type INTV_ROM_ECS = &device_creator<intv_ecs_device>;

intv_ecs_device::intv_ecs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: intv_rom_device(mconfig, INTV_ROM_ECS, "Intellivision ECS Expansion", tag, owner, clock, "intv_ecs", __FILE__),
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
//  MACHINE_CONFIG_FRAGMENT( sub_slot )
//-------------------------------------------------


static MACHINE_CONFIG_FRAGMENT( sub_slot )
	MCFG_SPEAKER_STANDARD_MONO("mono_ecs")

	MCFG_SOUND_ADD("ay8914", AY8914, XTAL_3_579545MHz/2)
	MCFG_AY8910_PORT_A_READ_CB(DEVREAD8("ctrl_port", intvecs_control_port_device, portA_r))
	MCFG_AY8910_PORT_B_READ_CB(DEVREAD8("ctrl_port", intvecs_control_port_device, portB_r))
	MCFG_AY8910_PORT_A_WRITE_CB(DEVWRITE8("ctrl_port", intvecs_control_port_device, portA_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono_ecs", 0.33)

	MCFG_INTVECS_CONTROL_PORT_ADD("ctrl_port", intvecs_control_port_devices, "keybd")
	MCFG_INTV_CARTRIDGE_ADD("subslot", intv_cart, nullptr)
MACHINE_CONFIG_END


machine_config_constructor intv_ecs_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sub_slot );
}


ROM_START( ecs )
	ROM_REGION( 0x20000, "ecs", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ecs_rom.20", 0x04000, 0x2000, CRC(5f9d05e5) SHA1(083a3e7405b8f8b4b8a5003ca9c31b8d824b535c))
	ROM_LOAD16_WORD_SWAP( "ecs_rom.70", 0x0e000, 0x2000, CRC(46bb1f48) SHA1(eda44a8476fdada1ae90fba0d0287611e2efa074))
	ROM_LOAD16_WORD_SWAP( "ecs_rom.e0", 0x1c000, 0x2000, CRC(c2ebcd90) SHA1(b3c14955f56c57e6f0d8fbb695771946cfcf6582))
ROM_END

const rom_entry *intv_ecs_device::device_rom_region() const
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
		return m_snd->read(space, offset, mem_mask);
	else
		return 0xffff;
}

/*-------------------------------------------------
 write_audio
 -------------------------------------------------*/

WRITE16_MEMBER(intv_ecs_device::write_ay)
{
	if (ACCESSING_BITS_0_7)
		return m_snd->write(space, offset, data, mem_mask);
}
