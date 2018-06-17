// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Digital Data Pack emulation

**********************************************************************/

#include "emu.h"
#include "ddp.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6801_TAG       "m6801"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADAM_DDP, adam_digital_data_pack_device, "adam_ddp", "Adam Digital Data Pack")


//-------------------------------------------------
//  ROM( adam_ddp )
//-------------------------------------------------

ROM_START( adam_ddp )
	ROM_REGION( 0x800, M6801_TAG, 0 )
	ROM_LOAD( "tape rev a 8865.u24", 0x000, 0x800, CRC(6b9ea1cf) SHA1(b970f11e8f443fa130fba02ad1f60da51bf89673) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *adam_digital_data_pack_device::device_rom_region() const
{
	return ROM_NAME( adam_ddp );
}


//-------------------------------------------------
//  ADDRESS_MAP( adam_ddp_mem )
//-------------------------------------------------

void adam_digital_data_pack_device::adam_ddp_mem(address_map &map)
{
	map(0x0000, 0x001f).rw(M6801_TAG, FUNC(m6801_cpu_device::m6801_io_r), FUNC(m6801_cpu_device::m6801_io_w));
	map(0x0080, 0x00ff).ram();
	map(0x0400, 0x07ff).ram();
	map(0xf800, 0xffff).rom().region(M6801_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( adam_ddp_io )
//-------------------------------------------------

void adam_digital_data_pack_device::adam_ddp_io(address_map &map)
{
	map(M6801_PORT1, M6801_PORT1).w(FUNC(adam_digital_data_pack_device::p1_w));
	map(M6801_PORT2, M6801_PORT2).rw(FUNC(adam_digital_data_pack_device::p2_r), FUNC(adam_digital_data_pack_device::p2_w));
	map(M6801_PORT3, M6801_PORT3).noprw(); // Multiplexed Address/Data
	map(M6801_PORT4, M6801_PORT4).r(FUNC(adam_digital_data_pack_device::p4_r)).nopw();
}

static const struct CassetteOptions adam_cassette_options =
{
	2,      /* channels */
	16,     /* bits per sample */
	44100   /* sample frequency */
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(adam_digital_data_pack_device::device_add_mconfig)
	MCFG_DEVICE_ADD(M6801_TAG, M6801, XTAL(4'000'000))
	MCFG_DEVICE_PROGRAM_MAP(adam_ddp_mem)
	MCFG_DEVICE_IO_MAP(adam_ddp_io)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(coleco_adam_cassette_formats)
	MCFG_CASSETTE_CREATE_OPTS(&adam_cassette_options)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)
	MCFG_CASSETTE_INTERFACE("adam_cass")

	MCFG_CASSETTE_ADD("cassette2")
	MCFG_CASSETTE_FORMATS(coleco_adam_cassette_formats)
	MCFG_CASSETTE_CREATE_OPTS(&adam_cassette_options)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED)
	MCFG_CASSETTE_INTERFACE("adam_cass")
MACHINE_CONFIG_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adam_digital_data_pack_device - constructor
//-------------------------------------------------

adam_digital_data_pack_device::adam_digital_data_pack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ADAM_DDP, tag, owner, clock),
		device_adamnet_card_interface(mconfig, *this),
		m_maincpu(*this, M6801_TAG),
		m_ddp0(*this, "cassette"),
		m_ddp1(*this, "cassette2"), m_wr0(0), m_wr1(0), m_track(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_digital_data_pack_device::device_start()
{
	// state saving
	save_item(NAME(m_wr0));
	save_item(NAME(m_wr1));
	save_item(NAME(m_track));
}


//-------------------------------------------------
//  adamnet_reset_w -
//-------------------------------------------------

void adam_digital_data_pack_device::adamnet_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_digital_data_pack_device::p1_w )
{
	/*

	    bit     description

	    0       SPD SEL (0=20 ips, 1=80ips)
	    1       STOP0
	    2       STOP1
	    3       _GO FWD
	    4       _GO REV
	    5       BRAKE
	    6       _WR0
	    7       _WR1

	*/

	if (m_ddp0->exists())
	{
		m_ddp0->set_speed(BIT(data, 0) ? (double) 80/1.875 : 20/1.875); // speed select
		if(!(data & 0x08)) m_ddp0->go_forward();
		if(!(data & 0x10)) m_ddp0->go_reverse();
		m_ddp0->change_state(BIT(data, 1) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR); // motor control
	}

	if (m_ddp1->exists())
	{
		m_ddp1->set_speed(BIT(data, 0) ? (double) 80/1.875 : 20/1.875); // speed select
		if(!(data & 0x08)) m_ddp1->go_forward();
		if(!(data & 0x10)) m_ddp1->go_reverse();
		m_ddp1->change_state(BIT(data, 2) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR); // motor control
	}

	// data write 0
	m_wr0 = BIT(data, 6);

	// data write 1
	m_wr1 = BIT(data, 7);
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

READ8_MEMBER( adam_digital_data_pack_device::p2_r )
{
	/*

	    bit     description

	    0       mode bit 0
	    1       mode bit 1 / CIP1
	    2       mode bit 2
	    3       NET RXD
	    4

	*/

	uint8_t data = 0;

	if (m_bus->reset_r())
		data |= M6801_MODE_6;
	else
		data |= m_ddp1->exists() << 1; // Cassette in place 1

	// NET RXD
	data |= m_bus->rxd_r(this) << 3;

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

WRITE8_MEMBER( adam_digital_data_pack_device::p2_w )
{
	/*

	    bit     description

	    0       WRT DATA
	    1
	    2       TRACK A/B (0=B, 1=A)
	    3
	    4       NET TXD

	*/

	if (m_ddp0->exists())
	{
		m_ddp0->set_channel(!BIT(data, 2)); // Track select
		if (!m_wr0) m_ddp0->output(BIT(data, 0) ? 1.0 : -1.0); // write data
	}

	if (m_ddp1->exists())
	{
		m_ddp1->set_channel(!BIT(data, 2));
		if (!m_wr1) m_ddp1->output(BIT(data, 0) ? 1.0 : -1.0);
	}

	// NET TXD
	m_bus->txd_w(this, BIT(data, 4));
}


//-------------------------------------------------
//  p4_r -
//-------------------------------------------------

READ8_MEMBER( adam_digital_data_pack_device::p4_r )
{
	/*

	    bit     description

	    0       A8
	    1       A9
	    2       A10 (2114 _S)
	    3       MSENSE 0
	    4       MSENSE 1
	    5       CIP0
	    6       RD DATA 0 (always 1)
	    7       RD DATA 1 (data from drives ORed together)

	*/

	uint8_t data = 0;

	// drive 0
	if (m_ddp0->exists())
	{
		data |= ((m_ddp0->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED) << 3; // motion sense
		data |= 1 << 5; // cassette in place
		data |= (m_ddp0->input() < 0) << 7; // read data
	}

	// drive 1
	if (m_ddp1->exists())
	{
		data |= ((m_ddp1->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED) << 4; // motion sense
		data |= (m_ddp1->input() < 0) << 7; // read data
	}

	// read data 0 (always 1)
	data |= 0x40;

	return data;
}
