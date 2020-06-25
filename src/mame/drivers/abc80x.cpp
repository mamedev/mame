// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

Luxor ABC 800C/800M

Main PCB Layout
---------------

|-----------------------|-----------------------------------|
|   4116    4116        |                                   |
|   4116    4116        |                                   |
|   4116    4116        |                                   |
|   4116    4116        |           Z80         Z80CTC      |
|   4116    4116        |CN1                                |
|   4116    4116        |                                   |
|   4116    4116        |                       Z80DART     |
|                       |                       CN6         |
|   ROM3    ROM7        |                                   |
|                       |                       Z80SIO2     |
|   ROM2    ROM6        |-----------------------------------|
|                                                           |
|   ROM1    ROM5                                            |
|                                                       CN2 |
|   ROM0    ROM4                                            |
|                       CN7                                 |
|-------------------------------------------|           CN3 |
|                                           |               |
|                                           |               |
|                                           |           CN4 |
|                                           |               |
|               VIDEO BOARD                 |               |
|                                           |           CN5 |
|                                           |               |
|                                           |               |
|                                           |               |
|-------------------------------------------|---------------|

Notes:
    Relevant IC's shown.

    Z80     - ? Z80A CPU (labeled "Z80A (800) KASS.")
    Z80CTC  - Sharp LH0082A Z80A-CTC
    Z80DART - SGS Z8470AB1 Z80A-DART
    Z8OSIO2 - SGS Z8442AB1 Z80A-SIO/2
    4116    - Toshiba TMS4116-20NL 1Kx8 RAM
    ROM0    - NEC D2732D 4Kx8 EPROM "ABC M-12"
    ROM1    - NEC D2732D 4Kx8 EPROM "ABC 1-12"
    ROM2    - NEC D2732D 4Kx8 EPROM "ABC 2-12"
    ROM3    - NEC D2732D 4Kx8 EPROM "ABC 3-12"
    ROM4    - NEC D2732D 4Kx8 EPROM "ABC 4-12"
    ROM5    - NEC D2732D 4Kx8 EPROM "ABC 5-12"
    ROM6    - Hitachi HN462732G 4Kx8 EPROM "ABC 6-52"
    ROM7    - NEC D2732D 4Kx8 EPROM "ABC 7-22"
    CN1     - ABC bus connector
    CN2     - tape connector
    CN3     - RS-232 channel B connector
    CN4     - RS-232 channel A connector
    CN5     - video connector
    CN6     - keyboard connector
    CN7     - video board connector


Video PCB Layout (Color)
------------------------

55 10789-01

|-------------------------------------------|
|                                           |
|                                           |
|               2114                        |
|                                           |
|               2114        TROM            |
|                                           |
|       12MHz               TIC             |
|                                           |
|                                           |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    2114    - GTE Microcircuits 2114-2CB 1Kx4 RAM
    TIC     - Philips SAA5020 Teletext Timing Chain
    TROM    - Philips SAA5052 Teletext Character Generator w/Swedish Character Set


Video PCB Layout (Monochrome)
-----------------------------

55 10790-02

|-------------------------------------------|
|                                           |
|               2114                        |
|               2114                        |
|                                      12MHz|
|               2114                        |
|               2114                        |
|                                           |
|               CRTC            ROM0        |
|                                           |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    2114    - Toshiba TMM314AP-1 1Kx4 RAM
    CRTC    - Hitachi HD46505SP CRTC
    ROM0    - NEC D2716D 2Kx8 EPROM "VUM SE"


Video PCB Layout (High Resolution)
----------------------------------

55 10791-01

|-------------------------------------------|
|                                           |
|   4116        PROM1                       |
|   4116                                    |
|   4116                                    |
|   4116                                    |
|   4116                                    |
|   4116                                    |
|   4116                                    |
|   4116                PROM0               |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    4116    - Motorola MCM4116AC25 1Kx8 RAM
    PROM0   - Philips 82S123 32x8 Bipolar PROM "HRU I"
    PROM1   - Philips 82S131 512x4 Bipolar PROM "HRU II"

*/

/*

    TODO:

    - abc806 30K banking
    - cassette
    - abc800 video card bus

*/

#include "emu.h"
#include "includes/abc80x.h"

#define LOG 0

#define ABCBUS_TAG "bus"


//**************************************************************************
//  SOUND
//**************************************************************************

//-------------------------------------------------
//  DISCRETE_SOUND( abc800 )
//-------------------------------------------------

static DISCRETE_SOUND_START( abc800_discrete )
	DISCRETE_INPUT_LOGIC(NODE_01)
	DISCRETE_LOGIC_JKFLIPFLOP(NODE_02, 1,1, NODE_01, 1, 1) // 74LS393 @ 7C (input _CP0, output Q0)
	DISCRETE_OUTPUT(NODE_02, 5000)
DISCRETE_SOUND_END


//-------------------------------------------------
//  pling_r - speaker read
//-------------------------------------------------

uint8_t abc800_state::pling_r()
{
	if (!machine().side_effects_disabled())
	{
		m_discrete->write(NODE_01, 0);
		m_discrete->write(NODE_01, 1);
	}

	return 0xff;
}



//**************************************************************************
//  MEMORY BANKING
//**************************************************************************

uint8_t abc800_state::read(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset < 0x4000 && (!m_keydtr || m_fetch_charram))
	{
		data = m_video_ram[offset];
	}
	else if (offset < 0x7800)
	{
		data = m_rom->base()[offset];
	}
	else if (offset < 0x8000 && m_fetch_charram)
	{
		data = m_rom->base()[offset];
	}
	else if (offset < 0x8000)
	{
		data = m_char_ram[offset & (m_char_ram_size - 1)];
	}
	else
	{
		data = m_ram->pointer()[offset & m_ram->mask()];
	}

	return data;
}

void abc800_state::write(offs_t offset, uint8_t data)
{
	if (offset < 0x4000 && (!m_keydtr || m_fetch_charram))
	{
		m_video_ram[offset] = data;
	}
	else if (offset >= 0x7800 && offset < 0x8000)
	{
		m_char_ram[offset & (m_char_ram_size - 1)] = data;
	}
	else if (offset >= 0x8000)
	{
		m_ram->pointer()[offset & m_ram->mask()] = data;
	}
}

uint8_t abc802_state::read(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset < 0x8000)
	{
		if (!m_lrs)
		{
			data = m_ram->pointer()[offset];
		}
		else
		{
			if (offset < 0x7800)
			{
				data = m_rom->base()[offset];
			}
			else
			{
				if (m_fetch_charram)
				{
					data = m_rom->base()[offset];
				}
				else
				{
					data = m_char_ram[offset & (m_char_ram_size - 1)];
				}
			}
		}
	}
	else
	{
		data = m_ram->pointer()[offset];
	}

	return data;
}

void abc802_state::write(offs_t offset, uint8_t data)
{
	if (offset < 0x8000)
	{
		if (!m_lrs)
		{
			m_ram->pointer()[offset] = data;
		}
		else if (offset >= 0x7800)
		{
			m_char_ram[offset & (m_char_ram_size - 1)] = data;
		}
	}
	else
	{
		m_ram->pointer()[offset] = data;
	}
}

void abc806_state::read_pal_p4(offs_t offset, bool m1l, bool xml, offs_t &m, bool &romd, bool &ramd, bool &hre, bool &vr)
{
	uint8_t map = m_map[offset >> 12] ^ 0xff;
	bool enl = BIT(map, 7);

	/*
	uint16_t input = 1 << 14 | m_keydtr << 12 | xml << 9 | enl << 8 | m_eme << 7 | m1l << 6 | BIT(offset, 11) << 5 | BIT(offset, 12) << 4 | BIT(offset, 13) << 3 | BIT(offset, 14) << 2 | BIT(offset, 15) << 1 | 1;
	int palout = m_pal->read(input);

	romd = BIT(palout, 0);
	ramd = BIT(palout, 7);
	hre = BIT(palout, 4);
	bool mux = BIT(palout, 6);
	*/

	romd = offset >= 0x8000;
	ramd = offset < 0x8000;
	hre = 0;
	vr = (offset & 0xf800) != 0x7800;
	bool mux = 0;

	if (!m_keydtr && offset < 0x8000)
	{
		romd = 1;
		hre = 1;
		vr = 1;
		mux = 0;
	}

	if (m_eme && !enl)
	{
		romd = 1;
		ramd = 1;
		hre = 1;
		vr = 1;
		mux = 1;
	}

	if (!m1l)
	{
		vr = 1;
	}
/*
    if (!m1l && (offset < 0x7800)
    {
        TODO 0..30k read from videoram if fetch opcode from 7800-7fff
        romd = 1;
        hre = 1;
        mux = 0;
    }
*/
	size_t videoram_mask = m_ram->size() - 0x8001;

	m = (mux ? ((map & 0x7f) << 12 | (offset & 0xfff)) : ((m_hrs & 0xf0) << 11 | (offset & 0x7fff))) & videoram_mask;
}

uint8_t abc806_state::read(offs_t offset)
{
	uint8_t data = 0xff;

	offs_t m = 0;
	bool m1l = 1, xml = 1, romd = 0, ramd = 0, hre = 0, vr = 1;
	read_pal_p4(offset, m1l, xml, m, romd, ramd, hre, vr);

	if (!romd)
	{
		data = m_rom->base()[offset & 0x7fff];
	}

	if (!ramd)
	{
		data = m_ram->pointer()[offset & 0x7fff];
	}

	if (hre)
	{
		data = m_video_ram[m];
	}

	if (!vr)
	{
		data = charram_r(offset & 0x7ff);
	}

	return data;
}

uint8_t abc806_state::m1_r(offs_t offset)
{
	uint8_t data = 0xff;

	offs_t m = 0;
	bool m1l = 0, xml = 1, romd = 0, ramd = 0, hre = 0, vr = 1;
	read_pal_p4(offset, m1l, xml, m, romd, ramd, hre, vr);

	if (!romd)
	{
		data = m_rom->base()[offset & 0x7fff];
	}

	if (!ramd)
	{
		data = m_ram->pointer()[offset & 0x7fff];
	}

	if (hre)
	{
		data = m_video_ram[m];
	}

	if (!vr)
	{
		data = charram_r(offset & 0x7ff);
	}

	return data;
}

void abc806_state::write(offs_t offset, uint8_t data)
{
	offs_t m = 0;
	bool m1l = 1, xml = 1, romd = 0, ramd = 0, hre = 0, vr = 1;
	read_pal_p4(offset, m1l, xml, m, romd, ramd, hre, vr);

	if (!ramd)
	{
		m_ram->pointer()[offset & 0x7fff] = data;
	}

	if (hre)
	{
		m_video_ram[m] = data;
	}

	if (!vr)
	{
		charram_w(offset & 0x7ff, data);
	}
}


//-------------------------------------------------
//  m1_r - opcode read
//-------------------------------------------------

uint8_t abc800_state::m1_r(offs_t offset)
{
	if (offset >= 0x7800 && offset < 0x8000)
	{
		m_fetch_charram = true;

		return m_rom->base()[offset];
	}

	m_fetch_charram = false;

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}


//-------------------------------------------------
//  mai_r - memory bank map read
//-------------------------------------------------

uint8_t abc806_state::mai_r(offs_t offset)
{
	int bank = offset >> 12;

	return m_map[bank];
}


//-------------------------------------------------
//  mao_w - memory bank map write
//-------------------------------------------------

void abc806_state::mao_w(offs_t offset, uint8_t data)
{
	/*

	    bit     description

	    0       physical block address bit 0
	    1       physical block address bit 1
	    2       physical block address bit 2
	    3       physical block address bit 3
	    4       physical block address bit 4
	    5
	    6
	    7       allocate block

	*/

	int bank = offset >> 12;

	if (LOG) logerror("MAO %04x %02x %02x\n",offset,bank,data);

	m_map[bank] = data;
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( abc800_m1 )
//-------------------------------------------------

void abc800_state::abc800_m1(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(abc800_state::m1_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( abc800_mem )
//-------------------------------------------------

void abc800_state::abc800_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(abc800_state::read), FUNC(abc800_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( abc800_io )
//-------------------------------------------------

void abc800_state::abc800_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).mirror(0xff18).rw(ABCBUS_TAG, FUNC(abcbus_slot_device::inp_r), FUNC(abcbus_slot_device::out_w));
	map(0x01, 0x01).mirror(0xff18).rw(ABCBUS_TAG, FUNC(abcbus_slot_device::stat_r), FUNC(abcbus_slot_device::cs_w));
	map(0x02, 0x02).mirror(0xff18).w(ABCBUS_TAG, FUNC(abcbus_slot_device::c1_w));
	map(0x03, 0x03).mirror(0xff18).w(ABCBUS_TAG, FUNC(abcbus_slot_device::c2_w));
	map(0x04, 0x04).mirror(0xff18).w(ABCBUS_TAG, FUNC(abcbus_slot_device::c3_w));
	map(0x05, 0x05).mirror(0xff18).w(ABCBUS_TAG, FUNC(abcbus_slot_device::c4_w));
	map(0x05, 0x05).mirror(0xff18).r(FUNC(abc800_state::pling_r));
	map(0x07, 0x07).mirror(0xff18).r(ABCBUS_TAG, FUNC(abcbus_slot_device::rst_r));
	map(0x20, 0x23).mirror(0xff0c).rw(m_dart, FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x40, 0x43).mirror(0xff1c).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x60, 0x63).mirror(0xff1c).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( abc800c_io )
//-------------------------------------------------

void abc800_state::abc800c_io(address_map &map)
{
	abc800_io(map);

	map(0x06, 0x06).mirror(0xff18).w(FUNC(abc800_state::hrs_w));
	map(0x07, 0x07).mirror(0xff18).w(FUNC(abc800_state::hrc_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( abc800m_io )
//-------------------------------------------------

void abc800_state::abc800m_io(address_map &map)
{
	abc800c_io(map);

	map(0x31, 0x31).mirror(0xff06).r(MC6845_TAG, FUNC(mc6845_device::register_r));
	map(0x38, 0x38).mirror(0xff06).w(MC6845_TAG, FUNC(mc6845_device::address_w));
	map(0x39, 0x39).mirror(0xff06).w(MC6845_TAG, FUNC(mc6845_device::register_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( abc802_mem )
//-------------------------------------------------

void abc802_state::abc802_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(abc802_state::read), FUNC(abc802_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( abc802_io )
//-------------------------------------------------

void abc802_state::abc802_io(address_map &map)
{
	abc800_io(map);

	map(0x31, 0x31).mirror(0xff06).r(MC6845_TAG, FUNC(mc6845_device::register_r));
	map(0x38, 0x38).mirror(0xff06).w(MC6845_TAG, FUNC(mc6845_device::address_w));
	map(0x39, 0x39).mirror(0xff06).w(MC6845_TAG, FUNC(mc6845_device::register_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( abc806_mem )
//-------------------------------------------------

void abc806_state::abc806_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(abc806_state::read), FUNC(abc806_state::write));
}


//-------------------------------------------------
//  ADDRESS_MAP( abc806_io )
//-------------------------------------------------

void abc806_state::abc806_io(address_map &map)
{
	abc800m_io(map);

	map(0x06, 0x06).mirror(0xff18).w(FUNC(abc806_state::hrs_w));
	map(0x07, 0x07).mirror(0xff18).w(FUNC(abc806_state::hrc_w));
	map(0x34, 0x34).select(0xff00).rw(FUNC(abc806_state::mai_r), FUNC(abc806_state::mao_w));
	map(0x35, 0x35).mirror(0xff00).rw(FUNC(abc806_state::ami_r), FUNC(abc806_state::amo_w));
	map(0x36, 0x36).mirror(0xff00).rw(FUNC(abc806_state::sti_r), FUNC(abc806_state::sto_w));
	map(0x37, 0x37).select(0xff00).rw(FUNC(abc806_state::cli_r), FUNC(abc806_state::sso_w));
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( abc800 )
//-------------------------------------------------

INPUT_PORTS_START( abc800 )
	PORT_START("SB")
	PORT_DIPNAME( 0xff, 0xaa, "Serial Communications" ) PORT_DIPLOCATION("SB:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0xaa, "Asynchronous, Single Speed" )
	PORT_DIPSETTING(    0x2e, "Asynchronous, Split Speed" )
	PORT_DIPSETTING(    0x50, "Synchronous" )
	PORT_DIPSETTING(    0x8b, "ABC NET" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( abc802 )
//-------------------------------------------------

static INPUT_PORTS_START( abc802 )
	PORT_INCLUDE(abc800)

	PORT_START("CONFIG")
	PORT_DIPNAME( 0x01, 0x00, "Clear Screen Time Out" ) PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("S2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Characters Per Line" ) PORT_DIPLOCATION("S3:1")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x04, "80" )
	PORT_DIPNAME( 0x08, 0x08, "Frame Frequency" )
	PORT_DIPSETTING(    0x00, "60 Hz" )
	PORT_DIPSETTING(    0x08, "50 Hz" )
INPUT_PORTS_END


//-------------------------------------------------
//  INPUT_PORTS( abc806 )
//-------------------------------------------------

static INPUT_PORTS_START( abc806 )
	PORT_INCLUDE(abc800)
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  cassette
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER( abc800_state::cassette_input_tick )
{
	if (m_cassette == nullptr) return;

	int dfd_in = m_cassette->input() > 0;

	if (m_dfd_in && !dfd_in)
	{
		m_sio->rxb_w(!(m_tape_ctr == 15));
	}

	if (!dfd_in && (m_tape_ctr == 15))
	{
		m_tape_ctr = 4;
	}

	m_dfd_in = dfd_in;
}

void abc800_state::cassette_output_tick(int state)
{
	if (m_cassette == nullptr) return;

	if (m_ctc_z0 && !state)
	{
		m_sio_txcb = !m_sio_txcb;
		m_sio->txcb_w(!m_sio_txcb);

		if (m_sio_txdb || m_sio_txcb)
		{
			m_dfd_out = !m_dfd_out;
			m_cassette->output((m_dfd_out & m_sio_rtsb) ? +1.0 : -1.0);
		}

		if (m_tape_ctr < 15)
		{
			m_tape_ctr++;
		}

		m_sio->rxcb_w(m_tape_ctr == 15);
	}

	m_ctc_z0 = state;
}


//-------------------------------------------------
//  Z80CTC
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER( abc800_state::ctc_tick )
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);

	m_ctc->trg1(1);
	m_ctc->trg1(0);

	m_ctc->trg2(1);
	m_ctc->trg2(0);
}

WRITE_LINE_MEMBER( abc800_state::ctc_z0_w )
{
	if (BIT(m_sb, 2))
	{
		m_sio->txca_w(state);
		m_ctc->trg3(state);
	}

	cassette_output_tick(state);
}

WRITE_LINE_MEMBER( abc800_state::ctc_z1_w )
{
	if (BIT(m_sb, 3))
	{
		m_sio->rxca_w(state);
	}

	if (BIT(m_sb, 4))
	{
		m_sio->txca_w(state);
		m_ctc->trg3(state);
	}
}


//-------------------------------------------------
//  Z80SIO
//-------------------------------------------------

WRITE_LINE_MEMBER( abc800_state::sio_txdb_w )
{
	m_sio_txdb = state;
}

WRITE_LINE_MEMBER( abc800_state::sio_dtrb_w )
{
	if (m_cassette == nullptr) return;

	if (state)
	{
		m_cassette->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
	}
	else
	{
		m_cassette->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
	}
}

WRITE_LINE_MEMBER( abc800_state::sio_rtsb_w )
{
	if (m_cassette == nullptr) return;

	m_sio_rtsb = state;

	if (!m_sio_rtsb)
	{
		m_cassette->output(-1.0);
	}
}


//-------------------------------------------------
//  Z80DART abc800
//-------------------------------------------------

WRITE_LINE_MEMBER( abc800_state::keydtr_w )
{
	if (LOG) logerror("%s KEYDTR %u\n",machine().describe_context(),state);

	m_keydtr = state;
}


//-------------------------------------------------
//  Z80DART abc802
//-------------------------------------------------

WRITE_LINE_MEMBER( abc802_state::lrs_w )
{
	if (LOG) logerror("%s LRS %u\n",machine().describe_context(),state);

	m_lrs = state;
}

WRITE_LINE_MEMBER( abc802_state::mux80_40_w )
{
	if (LOG) logerror("%s 80/40 MUX %u\n",machine().describe_context(),state);

	m_80_40_mux = state;
}


//-------------------------------------------------
//  z80_daisy_config abc800_daisy_chain
//-------------------------------------------------

static const z80_daisy_config abc800_daisy_chain[] =
{
	{ Z80CTC_TAG },
	{ Z80SIO_TAG },
	{ Z80DART_TAG },
	{ nullptr }
};



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  abc800
//-------------------------------------------------

void abc800_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_fetch_charram));
	save_item(NAME(m_sb));
	save_item(NAME(m_ctc_z0));
	save_item(NAME(m_sio_txcb));
	save_item(NAME(m_sio_txdb));
	save_item(NAME(m_sio_rtsb));
	save_item(NAME(m_dfd_out));
	save_item(NAME(m_dfd_in));
	save_item(NAME(m_tape_ctr));
}

void abc800_state::machine_reset()
{
	m_sb = m_io_sb->read();

	m_dart->ria_w(1);

	// 50/60 Hz
	m_dart->ctsb_w(0); // 0 = 50Hz, 1 = 60Hz

	m_dfd_in = 0;
}


//-------------------------------------------------
//  abc802
//-------------------------------------------------

void abc802_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_sb));
	save_item(NAME(m_ctc_z0));
	save_item(NAME(m_sio_txcb));
	save_item(NAME(m_sio_txdb));
	save_item(NAME(m_sio_rtsb));
	save_item(NAME(m_dfd_out));
	save_item(NAME(m_dfd_in));
	save_item(NAME(m_tape_ctr));
	save_item(NAME(m_lrs));
}

void abc802_state::machine_reset()
{
	uint8_t config = m_config->read();
	m_sb = m_io_sb->read();

	// memory banking
	m_lrs = 1;

	// clear screen time out (S1)
	m_sio->dcdb_w(BIT(config, 0));

	// unknown (S2)
	m_sio->ctsb_w(BIT(config, 1));

	// 40/80 char (S3)
	m_dart->ria_w(BIT(config, 2)); // 0 = 40, 1 = 80

	// 50/60 Hz
	m_dart->ctsb_w(BIT(config, 3)); // 0 = 50Hz, 1 = 60Hz

	m_dfd_in = 0;
}


//-------------------------------------------------
//  abc806
//-------------------------------------------------

void abc806_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_fetch_charram));
	save_item(NAME(m_sb));
	save_item(NAME(m_ctc_z0));
	save_item(NAME(m_sio_txcb));
	save_item(NAME(m_sio_txdb));
	save_item(NAME(m_sio_rtsb));
	save_item(NAME(m_dfd_out));
	save_item(NAME(m_dfd_in));
	save_item(NAME(m_tape_ctr));
	save_item(NAME(m_keydtr));
	save_item(NAME(m_eme));
	save_item(NAME(m_map));
}

void abc806_state::machine_reset()
{
	m_sb = m_io_sb->read();

	m_dart->ria_w(1);

	// 50/60 Hz
	m_dart->ctsb_w(0); // 0 = 50Hz, 1 = 60Hz

	m_dfd_in = 0;

	m_hrs = 0;

	// clear STO lines
	for (int i = 0; i < 8; i++) {
		sto_w(i);
	}
}


//-------------------------------------------------
//  bac quickload
//-------------------------------------------------

QUICKLOAD_LOAD_MEMBER(abc800_state::quickload_cb)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	std::vector<uint8_t> data;
	data.resize(quickload_size);
	image.fread(&data[0], quickload_size);

	uint8_t prstat = data[2];
	uint16_t prgsz = (data[5] << 8) | data[4];
	uint16_t varsz = (data[7] << 8) | data[6];
	uint16_t varad = (data[9] << 8) | data[8];
	uint16_t comsz = (data[13] << 8) | data[12];
	uint16_t comcs = (data[14] << 8) | data[15];
	uint16_t comtop = 0x8000 + comsz;
	uint16_t vartb = comtop;

	uint16_t heap = 0x8000 + comsz + varad;
	uint16_t bofa = 0xf169 - prgsz - 8;
	uint16_t eofa = bofa;

	for (int i = 0; i < prgsz; i++) {
		space.write_byte(eofa++, data[i]);
	}
	eofa--;

	for (int i = prgsz; i < quickload_size; i++) {
		space.write_byte(heap++, data[i]);
	}
	heap = 0x8000 + comsz + varsz;

	space.write_byte(0xff06, bofa & 0xff);
	space.write_byte(0xff07, bofa >> 8);

	space.write_byte(0xff08, eofa & 0xff);
	space.write_byte(0xff09, eofa >> 8);

	space.write_byte(0xff0a, heap & 0xff);
	space.write_byte(0xff0b, heap >> 8);

	space.write_byte(0xff26, prstat);

	space.write_byte(0xff2c, vartb & 0xff);
	space.write_byte(0xff2d, vartb >> 8);

	space.write_byte(0xff30, comtop & 0xff);
	space.write_byte(0xff31, comtop >> 8);

	space.write_byte(0xff32, comcs & 0xff);
	space.write_byte(0xff33, comcs >> 8);

	return image_init_result::PASS;
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( common )
//-------------------------------------------------

void abc800_state::common(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, ABC800_X01/2/2);
	m_maincpu->set_daisy_config(abc800_daisy_chain);
	m_maincpu->set_addrmap(AS_OPCODES, &abc800_state::abc800_m1);

	// peripheral hardware
	Z80CTC(config, m_ctc, ABC800_X01/2/2);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(abc800_state::ctc_z0_w));
	m_ctc->zc_callback<1>().set(FUNC(abc800_state::ctc_z1_w));
	m_ctc->zc_callback<2>().set(m_dart, FUNC(z80dart_device::rxca_w));
	m_ctc->zc_callback<2>().append(m_dart, FUNC(z80dart_device::txca_w));
	TIMER(config, TIMER_CTC_TAG).configure_periodic(FUNC(abc800_state::ctc_tick), attotime::from_hz(ABC800_X01/2/2/2));

	Z80SIO(config, m_sio, ABC800_X01/2/2);
	m_sio->out_txda_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(FUNC(abc800_state::sio_txdb_w));
	m_sio->out_dtrb_callback().set(FUNC(abc800_state::sio_dtrb_w));
	m_sio->out_rtsb_callback().set(FUNC(abc800_state::sio_rtsb_w));
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80DART(config, m_dart, ABC800_X01/2/2);
	m_dart->out_txda_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_dart->out_dtra_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_dart->out_rtsa_callback().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));
	m_dart->out_txdb_callback().set(ABC_KEYBOARD_PORT_TAG, FUNC(abc_keyboard_port_device::txd_w));
	m_dart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	DISCRETE(config, m_discrete, abc800_discrete).add_route(ALL_OUTPUTS, "mono", 0.80);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	TIMER(config, TIMER_CASSETTE_TAG).configure_periodic(FUNC(abc800_state::cassette_input_tick), attotime::from_hz(44100));

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_dart, FUNC(z80dart_device::rxa_w));
	rs232a.dcd_handler().set(m_dart, FUNC(z80dart_device::dcda_w));
	rs232a.cts_handler().set(m_dart, FUNC(z80dart_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	rs232b.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));
	rs232b.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));

	abc_keyboard_port_device &kb(ABC_KEYBOARD_PORT(config, ABC_KEYBOARD_PORT_TAG, abc_keyboard_devices, nullptr));
	kb.out_rx_handler().set(m_dart, FUNC(z80dart_device::rxb_w));
	kb.out_trxc_handler().set(m_dart, FUNC(z80dart_device::rxtxcb_w));
	kb.out_keydown_handler().set(m_dart, FUNC(z80dart_device::dcdb_w));

	ABCBUS_SLOT(config, ABCBUS_TAG, ABC800_X01/2/2, abcbus_cards, nullptr);

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("abc800");
	SOFTWARE_LIST(config, "hdd_list").set_original("abc800_hdd");

	// quickload
	QUICKLOAD(config, "quickload", "bac", attotime::from_seconds(2)).set_load_callback(FUNC(abc800_state::quickload_cb));
}


//-------------------------------------------------
//  machine_config( abc800c )
//-------------------------------------------------

void abc800c_state::abc800c(machine_config &config)
{
	common(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &abc800_state::abc800_mem);
	m_maincpu->set_addrmap(AS_IO, &abc800c_state::abc800c_io);

	// video hardware
	abc800c_video(config);

	// peripheral hardware
	m_dart->out_dtrb_callback().set(FUNC(abc800_state::keydtr_w));

	abc_keyboard_port_device &kb(*subdevice<abc_keyboard_port_device>(ABC_KEYBOARD_PORT_TAG));
	kb.set_default_option("abc800");
	kb.set_fixed(true);

	subdevice<abcbus_slot_device>(ABCBUS_TAG)->set_default_option("abc830");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("32K");
}


//-------------------------------------------------
//  machine_config( abc800m )
//-------------------------------------------------

void abc800m_state::abc800m(machine_config &config)
{
	common(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &abc800_state::abc800_mem);
	m_maincpu->set_addrmap(AS_IO, &abc800m_state::abc800m_io);

	// video hardware
	abc800m_video(config);

	// peripheral hardware
	m_dart->out_dtrb_callback().set(FUNC(abc800_state::keydtr_w));

	abc_keyboard_port_device &kb(*subdevice<abc_keyboard_port_device>(ABC_KEYBOARD_PORT_TAG));
	kb.set_default_option("abc800");
	kb.set_fixed(true);

	subdevice<abcbus_slot_device>(ABCBUS_TAG)->set_default_option("abc830");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("32K");
}


//-------------------------------------------------
//  machine_config( abc802 )
//-------------------------------------------------

void abc802_state::abc802(machine_config &config)
{
	common(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &abc802_state::abc802_mem);
	m_maincpu->set_addrmap(AS_IO, &abc802_state::abc802_io);

	// video hardware
	abc802_video(config);

	// peripheral hardware
	m_dart->out_dtrb_callback().set(FUNC(abc802_state::lrs_w));
	m_dart->out_rtsb_callback().set(FUNC(abc802_state::mux80_40_w));

	subdevice<abc_keyboard_port_device>(ABC_KEYBOARD_PORT_TAG)->set_default_option("abc55");

	subdevice<abcbus_slot_device>(ABCBUS_TAG)->set_default_option("abc834");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K");
}


//-------------------------------------------------
//  machine_config( abc806 )
//-------------------------------------------------

void abc806_state::abc806(machine_config &config)
{
	common(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &abc806_state::abc806_mem);
	m_maincpu->set_addrmap(AS_IO, &abc806_state::abc806_io);

	// video hardware
	abc806_video(config);

	// peripheral hardware
	m_dart->out_dtrb_callback().set(FUNC(abc800_state::keydtr_w));

	E0516(config, E0516_TAG, ABC806_X02);

	subdevice<abc_keyboard_port_device>(ABC_KEYBOARD_PORT_TAG)->set_default_option("abc77");

	subdevice<abcbus_slot_device>(ABCBUS_TAG)->set_default_option("abc832");

	// internal ram
	RAM(config, RAM_TAG).set_default_size("160K").set_extra_options("544K");

	// software list
	SOFTWARE_LIST(config, "flop_list2").set_original("abc806");
}



//**************************************************************************
//  ROMS
//**************************************************************************

/*

    ABC800 DOS ROMs

    Label       Drive Type
    ----------------------------
    ABC 6-1X    ABC830,DD82,DD84
    800 8"      DD88
    ABC 6-2X    ABC832
    ABC 6-3X    ABC838
    ABC 6-5X    UFD dos
    ABC 6-52    UFD dos with HD
    UFD 6.XX    Winchester


    Floppy Controllers

    Art N/O
    --------
    55 10761-01     "old" controller
    55 10828-01     "old" controller
    55 20900-0x
    55 21046-11     Luxor Conkort   25 pin D-sub connector
    55 21046-21     Luxor Conkort   34 pin FDD connector
    55 21046-41     Luxor Conkort   both of the above

*/

//-------------------------------------------------
//  ROM( abc800c )
//-------------------------------------------------

ROM_START( abc800c )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	ROM_LOAD( "abc c-1.1m", 0x0000, 0x1000, CRC(37009d29) SHA1(a1acd817ff8c2ed93935a163c5cf3d83d8bd6fb5) )
	ROM_LOAD( "abc 1-1.1l", 0x1000, 0x1000, CRC(ff15a28d) SHA1(bb4523ab1d190bc787f1923567b86795f66c26e2) )
	ROM_LOAD( "abc 2-1.1k", 0x2000, 0x1000, CRC(6ff2f528) SHA1(32d1639769c4a20b7a45c44f4c6993f77397f7a1) )
	ROM_LOAD( "abc 3-1.1j", 0x3000, 0x1000, CRC(9a43c47a) SHA1(964eb28e3d9779d1b13e0e938495133bbdb3aed3) )
	ROM_LOAD( "abc 4-1.2m", 0x4000, 0x1000, CRC(ba18db9c) SHA1(0c4543766fe9b10bee333f3f832f6f2209e449f8) )
	ROM_LOAD( "abc 5-1.2l", 0x5000, 0x1000, CRC(abbeb026) SHA1(44ebe417e3fa8d7878c23b56782aac8b443f1bfc) )
	ROM_LOAD( "abc 6-1.2k", 0x6000, 0x1000, CRC(4bd5e808) SHA1(5ca0a60571de6cfa3d6d166e0cde3c78560569f3) ) // 1981-01-12
	ROM_LOAD( "abc 7-22.2j", 0x7000, 0x1000, CRC(774511ab) SHA1(5171e43213a402c2d96dee33453c8306ac1aafc8) )

	ROM_REGION( 0x20, "hru", 0 )
	ROM_LOAD( "hru i.4g", 0x00, 0x20, CRC(d970a972) SHA1(c47fdd61fccc68368d42f03a01c7af90ab1fe1ab) )

	ROM_REGION( 0x200, "hru2", 0 )
	ROM_LOAD( "hru ii.3a", 0x000, 0x200, CRC(8e9d7cdc) SHA1(4ad16dc0992e31cdb2e644c7be81d334a56f7ad6) )
ROM_END


//-------------------------------------------------
//  ROM( abc800m )
//-------------------------------------------------

ROM_START( abc800m )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("ufd20")
	ROM_LOAD( "abc m-12.1m", 0x0000, 0x1000, CRC(f85b274c) SHA1(7d0f5639a528d8d8130a22fe688d3218c77839dc) )
	ROM_LOAD( "abc 1-12.1l", 0x1000, 0x1000, CRC(1e99fbdc) SHA1(ec6210686dd9d03a5ed8c4a4e30e25834aeef71d) )
	ROM_LOAD( "abc 2-12.1k", 0x2000, 0x1000, CRC(ac196ba2) SHA1(64fcc0f03fbc78e4c8056e1fa22aee12b3084ef5) )
	ROM_LOAD( "abc 3-12.1j", 0x3000, 0x1000, CRC(3ea2b5ee) SHA1(5a51ac4a34443e14112a6bae16c92b5eb636603f) )
	ROM_LOAD( "abc 4-12.2m", 0x4000, 0x1000, CRC(695cb626) SHA1(9603ce2a7b2d7b1cbeb525f5493de7e5c1e5a803) )
	ROM_LOAD( "abc 5-12.2l", 0x5000, 0x1000, CRC(b4b02358) SHA1(95338efa3b64b2a602a03bffc79f9df297e9534a) )
	ROM_SYSTEM_BIOS( 0, "13", "ABC-DOS (1982-07-19)" )
	ROMX_LOAD( "abc 6-13.2k", 0x6000, 0x1000, CRC(6fa71fb6) SHA1(b037dfb3de7b65d244c6357cd146376d4237dab6), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "ufd19", "UFD-DOS v.19 (1983-05-31)" )
	ROMX_LOAD( "abc 6-11 ufd.2k", 0x6000, 0x1000, CRC(2a45be80) SHA1(bf08a18a74e8bdaee2656a3c8246c0122398b58f), ROM_BIOS(1) ) // is this "ABC 6-5" or "ABC 6-51" instead?
	ROM_SYSTEM_BIOS( 2, "ufd20", "UFD-DOS v.20 (1984-03-02)" )
	ROMX_LOAD( "abc 6-52.2k", 0x6000, 0x1000, CRC(c311b57a) SHA1(4bd2a541314e53955a0d53ef2f9822a202daa485), ROM_BIOS(2) )
	ROM_LOAD_OPTIONAL( "abc 7-21.2j", 0x7000, 0x1000, CRC(fd137866) SHA1(3ac914d90db1503f61397c0ea26914eb38725044) )
	ROM_LOAD( "abc 7-22.2j", 0x7000, 0x1000, CRC(774511ab) SHA1(5171e43213a402c2d96dee33453c8306ac1aafc8) )

	ROM_REGION( 0x800, MC6845_TAG, 0 )
	ROM_LOAD( "vum se.7c", 0x000, 0x800, CRC(f9152163) SHA1(997313781ddcbbb7121dbf9eb5f2c6b4551fc799) )

	ROM_REGION( 0x20, "hru", 0 )
	ROM_LOAD( "hru i.4g", 0x00, 0x20, CRC(d970a972) SHA1(c47fdd61fccc68368d42f03a01c7af90ab1fe1ab) )

	ROM_REGION( 0x200, "hru2", 0 )
	ROM_LOAD( "hru ii.3a", 0x000, 0x200, CRC(8e9d7cdc) SHA1(4ad16dc0992e31cdb2e644c7be81d334a56f7ad6) )
ROM_END


//-------------------------------------------------
//  ROM( abc802 )
//-------------------------------------------------

ROM_START( abc802 )
	ROM_REGION( 0x8000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("ufd20")
	ROM_LOAD( "abc 02-11.9f",  0x0000, 0x2000, CRC(b86537b2) SHA1(4b7731ef801f9a03de0b5acd955f1e4a1828355d) )
	ROM_LOAD( "abc 12-11.11f", 0x2000, 0x2000, CRC(3561c671) SHA1(f12a7c0fe5670ffed53c794d96eb8959c4d9f828) )
	ROM_LOAD( "abc 22-11.12f", 0x4000, 0x2000, CRC(8dcb1cc7) SHA1(535cfd66c84c0370fd022d6edf702d3d1ad1b113) )
	ROM_SYSTEM_BIOS( 0, "abc", "ABC-DOS (1983-05-31)" )
	ROMX_LOAD( "abc 32-12.14f", 0x6000, 0x2000, CRC(23cd0f43) SHA1(639daec4565dcdb4de408b808d0c6cd97baa35d2), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "ufd19", "UFD-DOS v.19 (1984-03-02)" )
	ROMX_LOAD( "abc 32-21.14f", 0x6000, 0x2000, CRC(57050b98) SHA1(b977e54d1426346a97c98febd8a193c3e8259574), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "ufd20", "UFD-DOS v.20 (1984-04-03)" )
	ROMX_LOAD( "abc 32-31.14f", 0x6000, 0x2000, CRC(fc8be7a8) SHA1(a1d4cb45cf5ae21e636dddfa70c99bfd2050ad60), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "mica620", "MICA DOS v.20 (1984-03-02)" )
	ROMX_LOAD( "mica820.14f",   0x6000, 0x2000, CRC(edf998af) SHA1(daae7e1ff6ef3e0ddb83e932f324c56f4a98f79b), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "luxnet01", "LUXNET 01" )
	ROMX_LOAD( "322n01.14f",   0x6000, 0x2000, CRC(0911bc92) SHA1(bf58b3be40ce07638eb265aa2dd97c5562a0c41b), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "luxnet02", "LUXNET 02" )
	ROMX_LOAD( "322n02.14f",   0x6000, 0x2000, CRC(2384baec) SHA1(8ae0371242c201913b2d33a75f670d2bccf29582), ROM_BIOS(5) )

	ROM_REGION( 0x1000, MC6845_TAG, 0 )
	ROM_LOAD( "abc t02-1.3g", 0x0000, 0x1000, CRC(4d54eed8) SHA1(04cb5fc5f3d7ba9b9a5ae0ec94241d1fe83647f7) ) // 64 90191-01

	ROM_REGION( 0x400, "plds", 0 )
	/*
	    1   CLK
	    2   CUR
	    3   FC
	    4   IHS
	    5   LL
	    6   ATE
	    7   ATD
	    8   AT0
	    9   AT1
	    10  GND
	    11  GND
	    12  >O1
	    13  >O0
	    14
	    15
	    16  >RI
	    17  >RG
	    18  INV
	    19  >D
	    20  Vcc
	*/
	ROM_LOAD( "abc p2-1.2g", 0x000, 0x400, NO_DUMP ) // PAL16R4
ROM_END


//-------------------------------------------------
//  ROM( abc806 )
//-------------------------------------------------

ROM_START( abc806 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("ufd20")
	ROM_LOAD( "abc 06-11.1m",  0x0000, 0x1000, CRC(27083191) SHA1(9b45592273a5673e4952c6fe7965fc9398c49827) ) // BASIC PROM ABC 06-11 "64 90231-02"
	ROM_LOAD( "abc 16-11.1l",  0x1000, 0x1000, CRC(eb0a08fd) SHA1(f0b82089c5c8191fbc6a3ee2c78ce730c7dd5145) ) // BASIC PROM ABC 16-11 "64 90232-02"
	ROM_LOAD( "abc 26-11.1k",  0x2000, 0x1000, CRC(97a95c59) SHA1(013bc0a2661f4630c39b340965872bf607c7bd45) ) // BASIC PROM ABC 26-11 "64 90233-02"
	ROM_LOAD( "abc 36-11.1j",  0x3000, 0x1000, CRC(b50e418e) SHA1(991a59ed7796bdcfed310012b2bec50f0b8df01c) ) // BASIC PROM ABC 36-11 "64 90234-02"
	ROM_LOAD( "abc 46-11.2m",  0x4000, 0x1000, CRC(17a87c7d) SHA1(49a7c33623642b49dea3d7397af5a8b9dde8185b) ) // BASIC PROM ABC 46-11 "64 90235-02"
	ROM_LOAD( "abc 56-11.2l",  0x5000, 0x1000, CRC(b4b02358) SHA1(95338efa3b64b2a602a03bffc79f9df297e9534a) ) // BASIC PROM ABC 56-11 "64 90236-02"
	ROM_SYSTEM_BIOS( 0, "ufd19", "UFD-DOS v.19 (1984-03-02)" )
	ROMX_LOAD( "abc 66-21.2k", 0x6000, 0x1000, CRC(c311b57a) SHA1(4bd2a541314e53955a0d53ef2f9822a202daa485), ROM_BIOS(0) ) // DOS PROM ABC 66-21 "64 90314-01"
	ROM_SYSTEM_BIOS( 1, "ufd20", "UFD-DOS v.20 (1984-04-03)" )
	ROMX_LOAD( "abc 66-31.2k", 0x6000, 0x1000, CRC(a2e38260) SHA1(0dad83088222cb076648e23f50fec2fddc968883), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "mica20", "MICA DOS v.20 (1984-04-03)" )
	ROMX_LOAD( "mica2006.2k",  0x6000, 0x1000, CRC(58bc2aa8) SHA1(0604bd2396f7d15fcf3d65888b4b673f554037c0), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "catnet", "CAT-NET" )
	ROMX_LOAD( "cmd8_5.2k",    0x6000, 0x1000, CRC(25430ef7) SHA1(03a36874c23c215a19b0be14ad2f6b3b5fb2c839), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "luxnet", "LUXNET" )
	ROMX_LOAD( "ln806.2k",    0x6000, 0x1000, CRC(034b5991) SHA1(ba7f8653f4e516687a4399abef450e361f2bfd20), ROM_BIOS(4) )
	ROM_LOAD_OPTIONAL( "abc 76-11.2j",  0x7000, 0x1000, CRC(3eb5f6a1) SHA1(02d4e38009c71b84952eb3b8432ad32a98a7fe16) ) // Options-PROM ABC 76-11 "64 90238-02"
	ROM_LOAD( "abc 76-xx.2j",  0x7000, 0x1000, CRC(b364cc49) SHA1(9a2c373778856a31902cdbd2ae3362c200a38e24) ) // Enhanced Options-PROM

	ROM_REGION( 0x1000, MC6845_TAG, 0 )
	ROM_LOAD( "abc t6-11.7c", 0x0000, 0x1000, CRC(b17c51c5) SHA1(e466e80ec989fbd522c89a67d274b8f0bed1ff72) ) // 64 90243-01

	ROM_REGION( 0x200, "rad", 0 )
	ROM_LOAD( "60 90241-01.9b", 0x000, 0x200, CRC(ad549ebb) SHA1(4fe228ce3b84ed6f0ffd5431f2f33b94c3e5268b) ) // "RAD" 7621/7643 (82S131/82S137), character line address

	ROM_REGION( 0x20, "hru", 0 )
	ROM_LOAD( "60 90128-01.6e", 0x00, 0x20, CRC(d970a972) SHA1(c47fdd61fccc68368d42f03a01c7af90ab1fe1ab) ) // "HRU I" 7603 (82S123), HR horizontal timing and video memory access

	ROM_REGION( 0x200, "hru2", 0 )
	ROM_LOAD( "60 90127-01.12g", 0x000, 0x200, CRC(8e9d7cdc) SHA1(4ad16dc0992e31cdb2e644c7be81d334a56f7ad6) ) // "HRU II" 7621 (82S131), ABC800C HR compatibility mode palette

	ROM_REGION( 0x200, "v50", 0 )
	ROM_LOAD( "60 90242-01.7e", 0x000, 0x200, CRC(788a56d8) SHA1(d81e55cdddc36f5d41bf0a33104c75fac590b764) ) // "V50" 7621 (82S131), HR vertical timing 50Hz

	//ROM_REGION( 0x200, "v60", 0 )
	//ROM_LOAD( "60 90319-01.7e", 0x000, 0x200, NO_DUMP ) // "V60" 7621 (82S131), HR vertical timing 60Hz

	//ROM_REGION( 0x400, "att_hand", 0 )
	/*
	    1   E6P (RAD A8)
	    2   THP (chargen A12)
	    3   CCLK
	    4   B0 (TX ATT 6)
	    5   B1 (TX ATT 7)
	    6   CONDP (40/80)
	    7   B2 (TX ATT 0)
	    8   B3 (TX ATT 1)
	    9   ULP (RAD A5)
	    10  FLP (RAD A6)
	    11  F2P (RTF)
	    12  GND
	    13  F3P (GTF)
	    14  F4P (BTF)
	    15  B5P (RTB)
	    16  B4 (TX ATT 2)
	    17  B5 (TX ATT 3)
	    18  B6 (TX ATT 4)
	    19  B7 (TX ATT 5)
	    20  LP (*DEN+3)
	    21  B6P (GTB)
	    22  B7P (BTB)
	    23  E5P (RAD A7)
	    24  Vcc
	*/
	//ROM_LOAD( "60 90225-01.11c", 0x000, 0x400, NO_DUMP ) // "VIDEO ATTRIBUTE" 40033A (?)

	ROM_REGION( 0x104, "abc_p3", 0 )
	/*
	    1   12MHz
	    2   DOT
	    3   RTF
	    4   GTF
	    5   BTF
	    6   RTB
	    7   GTB
	    8   BTB
	    9   SFG
	    10  GND
	    11  GND
	    12  RFG
	    13  GFG
	    14  >YL
	    15  >BL
	    16  >GL
	    17  >RL
	    18  BFG
	    19  >FGE
	    20  Vcc
	*/
	ROM_LOAD( "60 90239-01.1b",  0x000, 0x104, CRC(f3d0ba00) SHA1(bcc0ee26ecac0028aef6bf5cb308133b509bb360) ) // "ABC P3-11" PAL16R4, color encoder

	ROM_REGION( 0x104, "abc_p4", 0 )
	/*
	    1   I3
	    2   A15
	    3   A14
	    4   B13
	    5   B12
	    6   B11
	    7   M1L
	    8   EME
	    9   ENL
	    10  GND
	    11  XML
	    12  >ROMD
	    13  HRAL
	    14  HRBL
	    15  KDL
	    16  >HRE
	    17  RKDL
	    18  >MUX
	    19  >RAMD
	    20  Vcc
	*/
	ROM_LOAD( "60 90240-01.2d",  0x000, 0x104, CRC(3cc5518d) SHA1(343cf951d01c9d361b695bb4e80eaadf0820b6bc) ) // "ABC P4-11" PAL16L8, memory mapper
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT   CLASS          INIT        COMPANY             FULLNAME        FLAGS
COMP( 1981, abc800c, 0,       0,      abc800c, abc800, abc800c_state, empty_init, "Luxor Datorer AB", "ABC 800 C/HR", MACHINE_SUPPORTS_SAVE )
COMP( 1981, abc800m, abc800c, 0,      abc800m, abc800, abc800m_state, empty_init, "Luxor Datorer AB", "ABC 800 M/HR", MACHINE_SUPPORTS_SAVE )
COMP( 1983, abc802,  0,       0,      abc802,  abc802, abc802_state,  empty_init, "Luxor Datorer AB", "ABC 802",      MACHINE_SUPPORTS_SAVE )
COMP( 1983, abc806,  0,       0,      abc806,  abc806, abc806_state,  empty_init, "Luxor Datorer AB", "ABC 806",      MACHINE_SUPPORTS_SAVE )
