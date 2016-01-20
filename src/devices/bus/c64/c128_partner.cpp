// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Timeworks PARTNER 128 cartridge emulation

**********************************************************************/

/*

    PCB Layout
    ----------

    |---------------|
    |LS74  SW     * |
    |LS09      LS273|
    |LS139   RAM    |
    |LS133          |
    |     LS240     |
    |LS33    ROM    |
    |LS09           |
     |||||||||||||||

    ROM     - Toshiba TMM24128AP 16Kx8 EPROM (blank label)
    RAM     - Sony CXK5864PN-15L 8Kx8 SRAM
    SW      - push button switch
    *       - solder point for joystick port dongle

*/

#include "c128_partner.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C128_PARTNER = &device_creator<partner128_t>;


//-------------------------------------------------
//  INPUT_PORTS( c128_partner )
//-------------------------------------------------

WRITE_LINE_MEMBER( partner128_t::nmi_w )
{
	if (state)
	{
		m_ls74_q1 = 1;
	}
}

static INPUT_PORTS_START( c128_partner )
	PORT_START("NMI")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Menu") PORT_CODE(KEYCODE_END) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, partner128_t, nmi_w)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor partner128_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( c128_partner );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  partner128_t - constructor
//-------------------------------------------------

partner128_t::partner128_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C128_PARTNER, "PARTNER 128", tag, owner, clock, "c128_partner", __FILE__),
	device_c64_expansion_card_interface(mconfig, *this),
	//device_vcs_control_port_interface(mconfig, *this),
	m_ram(*this, "ram"), t_joyb2(nullptr),
	m_ram_a12_a7(0),
	m_ls74_cd(0),
	m_ls74_q1(0),
	m_ls74_q2(0),
	m_joyb2(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void partner128_t::device_start()
{
	// allocate memory
	m_ram.allocate(0x2000);

	// simulate the 16.7ms pulse from CIA1 PB2 that would arrive thru the joystick port dongle
	t_joyb2 = timer_alloc();
	t_joyb2->adjust(attotime::from_msec(16), 0, attotime::from_msec(16));

	// state saving
	save_item(NAME(m_ram_a12_a7));
	save_item(NAME(m_ls74_cd));
	save_item(NAME(m_ls74_q1));
	save_item(NAME(m_ls74_q2));
	save_item(NAME(m_joyb2));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void partner128_t::device_reset()
{
	m_ram_a12_a7 = 0;

	m_ls74_cd = 0;
	m_ls74_q1 = 0;
	m_ls74_q2 = 0;

	nmi_w(CLEAR_LINE);
}


//-------------------------------------------------
//  device_timer -
//-------------------------------------------------

void partner128_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (m_ls74_cd)
	{
		m_ls74_q2 = m_ls74_q1;

		nmi_w(m_ls74_q2 ? ASSERT_LINE : CLEAR_LINE);
	}
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

UINT8 partner128_t::c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!roml)
	{
		data = m_roml[offset & 0x3fff];
	}

	if (!io1)
	{
		if (BIT(offset, 7))
		{
			data = m_roml[offset & 0x3fff];

			if (m_ls74_cd)
			{
				m_ls74_q1 = 0;
			}
		}
		else
		{
			data = m_ram[(m_ram_a12_a7 << 7) | (offset & 0x7f)];
		}
	}

	if (m_ls74_q2 && ((offset & 0xfffa) == 0xfffa))
	{
		// override the 8502 NMI/IRQ vectors with 0xdede
		data = 0xde;
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void partner128_t::c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!io1)
	{
		if (BIT(offset, 7))
		{
			/*

			    bit     description

			    0       RAM A7
			    1       RAM A8
			    2       RAM A9
			    3       RAM A10
			    4       RAM A11
			    5       RAM A12
			    6       LS74 1Cd,2Cd
			    7       N/C

			*/

			m_ram_a12_a7 = data & 0x3f;

			m_ls74_cd = BIT(data, 6);

			if (!m_ls74_cd)
			{
				m_ls74_q1 = 0;
				m_ls74_q2 = 0;

				nmi_w(CLEAR_LINE);
			}
		}
		else
		{
			m_ram[(m_ram_a12_a7 << 7) | (offset & 0x7f)] = data;
		}
	}

	if (sphi2 && ((offset & 0xfff0) == 0xd600))
	{
		m_ram[(m_ram_a12_a7 << 7) | (offset & 0x7f)] = data;
	}
}


//-------------------------------------------------
//  vcs_joy_w - joystick write
//-------------------------------------------------

void partner128_t::vcs_joy_w(UINT8 data)
{
	int joyb2 = BIT(data, 2);

	if (!m_joyb2 && joyb2 && m_ls74_cd)
	{
		m_ls74_q2 = m_ls74_q1;

		nmi_w(m_ls74_q2 ? ASSERT_LINE : CLEAR_LINE);
	}

	m_joyb2 = joyb2;
}
