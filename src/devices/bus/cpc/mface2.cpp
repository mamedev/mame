// license:BSD-3-Clause
// copyright-holders:Kevin Thacker, Barry Rodewald
/*
 * mface2.c  --  Romantic Robot Multiface II expansion device for the Amstrad CPC/CPC+
 *
 *  Created on: 31/07/2011
 */

#include "emu.h"
#include "mface2.h"
#include "includes/amstrad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CPC_MFACE2 = &device_creator<cpc_multiface2_device>;

// device machine config
static MACHINE_CONFIG_FRAGMENT( cpc_mface2 )
	// pass-through
	MCFG_DEVICE_ADD("exp", CPC_EXPANSION_SLOT, 0)
	MCFG_DEVICE_SLOT_INTERFACE(cpc_exp_cards, nullptr, false)
	MCFG_CPC_EXPANSION_SLOT_OUT_IRQ_CB(DEVWRITELINE("^", cpc_expansion_slot_device, irq_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_NMI_CB(DEVWRITELINE("^", cpc_expansion_slot_device, nmi_w))
	MCFG_CPC_EXPANSION_SLOT_OUT_ROMDIS_CB(DEVWRITELINE("^", cpc_expansion_slot_device, romdis_w))  // ROMDIS
MACHINE_CONFIG_END

DIRECT_UPDATE_MEMBER( cpc_multiface2_device::amstrad_default )
{
	return address;
}

/* used to setup computer if a snapshot was specified */
DIRECT_UPDATE_MEMBER( cpc_multiface2_device::amstrad_multiface_directoverride )
{
		int pc;

		pc = machine().device("maincpu")->safe_pc();
		/* there are two places where CALL &0065 can be found
		in the multiface rom. At this address there is a RET.

		To disable the multiface from being detected, the multiface
		stop button must be pressed, then the program that was stopped
		must be returned to. When this is done, the multiface cannot
		be detected and the out operations to page the multiface
		ram/rom into the address space will not work! */

		/* I assume that the hardware in the multiface detects
		the PC set to 0x065 and uses this to enable/disable the multiface
		*/

		/* I also use this to allow the stop button to be pressed again */
		if (pc==0x0164)
		{
			/* first call? */
			m_multiface_flags |= MULTIFACE_VISIBLE;
		}
		else if (pc==0x0c98)
		{
			/* second call */

			/* no longer visible */
			m_multiface_flags &= ~(MULTIFACE_VISIBLE|MULTIFACE_STOP_BUTTON_PRESSED);

			m_romdis=0;

			/* clear op base override */
			machine().device("maincpu")->memory().space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(cpc_multiface2_device::amstrad_default),this));
		}

		return pc;
}

int cpc_multiface2_device::multiface_hardware_enabled()
{
	if (m_multiface_ram!=nullptr)
	{
		if ((ioport("multiface")->read() & 0x01)!=0)
		{
			return 1;
		}
	}

	return 0;
}

/* multiface traps calls to 0x0065 when it is active.
This address has a RET and so executes no code.

It is believed that it is used to make multiface invisible to programs */

/*#define MULTIFACE_0065_TOGGLE                   0x0008*/


void cpc_multiface2_device::multiface_rethink_memory()
{
	unsigned char *multiface_rom;

	/* multiface hardware enabled? */
	if (!multiface_hardware_enabled())
		return;

	multiface_rom = memregion("multiface")->base();

	if ((m_multiface_flags & MULTIFACE_RAM_ROM_ENABLED)!=0 && m_romdis != 0)
	{
		/* set bank addressess */
		machine().root_device().membank("bank1")->set_base(multiface_rom);
		machine().root_device().membank("bank2")->set_base(m_multiface_ram.get());
		machine().root_device().membank("bank9")->set_base(multiface_rom);
		machine().root_device().membank("bank10")->set_base(m_multiface_ram.get());
	}
}

machine_config_constructor cpc_multiface2_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cpc_mface2 );
}

void cpc_multiface2_device::check_button_state()
{
	if(!multiface_hardware_enabled())
		return;
	// TODO: reset button
	if (ioport("multiface")->read() & 0x02)
	{
		multiface_stop();
	}
}

/* simulate the stop button has been pressed */
void cpc_multiface2_device::multiface_stop()
{
	/* multiface hardware enabled? */
	if (!multiface_hardware_enabled())
		return;

	/* if stop button not already pressed, do press action */
	/* pressing stop button while multiface is running has no effect */
	if ((m_multiface_flags & MULTIFACE_STOP_BUTTON_PRESSED)==0)
	{
		/* initialise 0065 toggle */
				/*state->m_multiface_flags &= ~MULTIFACE_0065_TOGGLE;*/

		m_multiface_flags |= MULTIFACE_RAM_ROM_ENABLED;

		/* stop button has been pressed, furthur pressess will not issue a NMI */
		m_multiface_flags |= MULTIFACE_STOP_BUTTON_PRESSED;

		m_romdis = 1;

		/* page rom into memory */
		multiface_rethink_memory();

		/* pulse the nmi line */
		m_slot->nmi_w(1);
		m_slot->nmi_w(0);

		/* initialise 0065 override to monitor calls to 0065 */
		machine().device("maincpu")->memory().space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(cpc_multiface2_device::amstrad_multiface_directoverride),this));
	}
}

/* any io writes are passed through here */
int cpc_multiface2_device::multiface_io_write(UINT16 offset, UINT8 data)
{
	int ret = 0;

	/* multiface hardware enabled? */
	if (!multiface_hardware_enabled())
		return 0;

		/* visible? */
	if (m_multiface_flags & MULTIFACE_VISIBLE)
	{
		if (offset==0x0fee8)
		{
			m_multiface_flags |= MULTIFACE_RAM_ROM_ENABLED;
			ret = 1;
		}

		if (offset==0x0feea)
		{
			m_multiface_flags &= ~MULTIFACE_RAM_ROM_ENABLED;
			ret = 1;
		}
	}

	/* update multiface ram with data */
	/* these are decoded fully! */
	switch ((offset>>8) & 0x0ff)
	{
			/* gate array */
			case 0x07f:
			{
					switch (data & 0x0c0)
					{
							/* pen index */
							case 0x00:
							{
								m_multiface_ram[0x01fcf] = data;
							}
							break;
								/* pen colour */
							case 0x040:
							{
								int pen_index;
								pen_index = m_multiface_ram[0x01fcf] & 0x0f;
								if (m_multiface_ram[0x01fcf] & 0x010)
								{
										m_multiface_ram[0x01fdf + pen_index] = data;
								}
							else
								{
									m_multiface_ram[0x01f90 + pen_index] = data & 0x01f;
								}
							}
							break;
								/* rom/mode selection */
							case 0x080:
							{
									m_multiface_ram[0x01fef] = data;
								}
							break;
								/* ram configuration */
							case 0x0c0:
							{
									m_multiface_ram[0x01fff] = data;
							}
							break;
							default:
						break;
				}
			}
			break;

			/* crtc register index */
			case 0x0bc:
			{
					m_multiface_ram[0x01cff] = data;
			}
			break;
			/* crtc register write */
			case 0x0bd:
			{
					int reg_index;
					reg_index = m_multiface_ram[0x01cff] & 0x0f;
					m_multiface_ram[0x01db0 + reg_index] = data;
			}
			break;


			/* 8255 ppi control */
			case 0x0f7:
			{
				m_multiface_ram[0x017ff] = data;
			}
			break;
			/* rom select */
			case 0x0df:
			{
				m_multiface_ram[0x01aac] = data;
			}
			break;
			default:
				break;
	}
	return ret;
}

static INPUT_PORTS_START(cpc_mface2)
	PORT_START("multiface")
	PORT_CONFNAME(0x01, 0x00, "Multiface Two" )
	PORT_CONFSETTING(0x00, DEF_STR( Off) )
	PORT_CONFSETTING(0x01, DEF_STR( On) )
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Multiface Two's Stop Button") PORT_CODE(KEYCODE_F6)
	//  PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Multiface Two's Reset Button") PORT_CODE(KEYCODE_F3)  Not implemented
INPUT_PORTS_END



//-------------------------------------------------
//  Device ROM definition
//-------------------------------------------------

// Second known revision (1988)
ROM_START( cpc_mface2 )
	ROM_REGION( 0x2000, "multiface", 0 )
	ROM_LOAD("multface.rom", 0x0000, 0x2000, CRC(f36086de) SHA1(1431ec628d38f000715545dd2186b684c5fe5a6f))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *cpc_multiface2_device::device_rom_region() const
{
	return ROM_NAME( cpc_mface2 );
}

ioport_constructor cpc_multiface2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cpc_mface2 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

cpc_multiface2_device::cpc_multiface2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CPC_MFACE2, "Multiface II", tag, owner, clock, "cpc_mf2", __FILE__),
	device_cpc_expansion_card_interface(mconfig, *this), m_slot(nullptr), m_multiface_ram(nullptr), m_multiface_flags(0), m_romdis(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cpc_multiface2_device::device_start()
{
	m_slot = dynamic_cast<cpc_expansion_slot_device *>(owner());

	/* after a reset the multiface is visible */
	m_multiface_flags = MULTIFACE_VISIBLE;

	/* allocate ram */
	m_multiface_ram = std::make_unique<UINT8[]>(8192);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cpc_multiface2_device::device_reset()
{
	/* stop button not pressed and ram/rom disabled */
	m_multiface_flags &= ~(MULTIFACE_STOP_BUTTON_PRESSED |
					MULTIFACE_RAM_ROM_ENABLED);
	/* as on the real hardware the multiface is visible after a reset! */
	m_multiface_flags |= MULTIFACE_VISIBLE;
}
