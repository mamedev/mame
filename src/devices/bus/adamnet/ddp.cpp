// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Digital Data Pack emulation

**********************************************************************/

#include "emu.h"
#include "ddp.h"

#include "formats/adam_cas.h"



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
	map(0x0400, 0x07ff).ram();
}

static const cassette_image::Options adam_cassette_options =
{
	2,      /* channels */
	16,     /* bits per sample */
	44100   /* sample frequency */
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void adam_digital_data_pack_device::device_add_mconfig(machine_config &config)
{
	M6801(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &adam_digital_data_pack_device::adam_ddp_mem);
	m_maincpu->out_p1_cb().set(FUNC(adam_digital_data_pack_device::p1_w));
	m_maincpu->in_p2_cb().set(FUNC(adam_digital_data_pack_device::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(adam_digital_data_pack_device::p2_w));
	// Port 3 = Multiplexed Address/Data
	m_maincpu->in_p4_cb().set(FUNC(adam_digital_data_pack_device::p4_r));

	for (auto &ddp : m_ddp)
	{
		CASSETTE(config, ddp);
		ddp->set_formats(coleco_adam_cassette_formats);
		ddp->set_create_opts(&adam_cassette_options);
		ddp->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_MUTED);
		ddp->set_interface("adam_cass");
	}
}



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
		m_ddp(*this, {"cassette", "cassette2"}),
		m_wr(0), m_track(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_digital_data_pack_device::device_start()
{
	// state saving
	save_item(NAME(m_wr));
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

void adam_digital_data_pack_device::p1_w(uint8_t data)
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

	for (int n = 0; n < 2; n++)
	{
		if (m_ddp[n]->exists())
		{
			m_ddp[n]->set_speed(BIT(data, 0) ? (double) 80/1.875 : 20/1.875); // speed select
			if (!BIT(data, 3))
				m_ddp[n]->go_forward();
			if (!BIT(data, 4))
				m_ddp[n]->go_reverse();
			m_ddp[n]->change_state(BIT(data, n + 1) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR); // motor control
		}
	}

	// data write 0/1
	m_wr = BIT(data, 6, 2);
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

uint8_t adam_digital_data_pack_device::p2_r()
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
		data |= m_ddp[1]->exists() << 1; // Cassette in place 1

	// NET RXD
	data |= m_bus->rxd_r(this) << 3;

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

void adam_digital_data_pack_device::p2_w(uint8_t data)
{
	/*

	    bit     description

	    0       WRT DATA
	    1
	    2       TRACK A/B (0=B, 1=A)
	    3
	    4       NET TXD

	*/

	for (int n = 0; n < 2; n++)
	{
		if (m_ddp[n]->exists())
		{
			m_ddp[n]->set_channel(!BIT(data, 2)); // Track select
			if (!BIT(m_wr, n))
				m_ddp[n]->output(BIT(data, 0) ? 1.0 : -1.0); // write data
		}
	}

	// NET TXD
	m_bus->txd_w(this, BIT(data, 4));
}


//-------------------------------------------------
//  p4_r -
//-------------------------------------------------

uint8_t adam_digital_data_pack_device::p4_r()
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

	for (int n = 0; n < 2; n++)
	{
		if (m_ddp[n]->exists())
		{
			data |= ((m_ddp[n]->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_STOPPED) << (n + 3); // motion sense
			if (n == 0)
				data |= 1 << 5; // cassette in place (drive 0)
			data |= (m_ddp[n]->input() < 0) << 7; // read data
		}
	}

	// read data 0 (always 1)
	data |= 0x40;

	return data;
}
