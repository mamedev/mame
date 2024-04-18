// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
    ISA16 PIU10

    PIU10 000614
    -----------
    HS3PIN labeled POWER
    CD-IN 4 pin header but only 3 pins populated
    (audio) OUT 4 pin header but only 3 pins populated, markings nearby show pinout of "OUTR GND OUTL GND"
    6 pin power headers in corner

    U1, U2, U3 HD74LS138P
    U4, U5, U14 HD74LS245P
    U6 HD74LS139P
    U7 PST518
    U8, U9, U10 MX 29F1610MC-12 (U10 unpopulated) with optional "MONO" configuration beside it
    U11 ATMEL ATF1500A I5JC 0015
    U12 Micronas MAS3507D F10
    U13 Micronas DAC3350A C2
    U15 HD74LS273P
    U16 HD74LS125AP
    U18, U19 7809CT Linear Voltage Regulator
    U20 UTC 78L05 WK
    U21, U22 HA17358

    CN1 PCN96, connects to SPACE12_1
    CN3, CN4 Unpopulated 4 pin headers

    14.7456MHz XTAL near Micronas chips


    CAT702 security data:
    https://github.com/pumpitupdev/pumptools/blob/b809d8674925d3c6e6b25efa91dccc04a460a62f/src/main/sec/lockchip/lockchip-defs.h
 */

#include "emu.h"
#include "xtom3d_piu10.h"


DEFINE_DEVICE_TYPE(ISA16_PIU10, isa16_piu10, "isa16_piu10", "ISA16 PIU10 for MK-III")

isa16_piu10::isa16_piu10(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_PIU10, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_cat702(*this, "cat702")
	, m_dac3350a(*this, "dac3350a")
	, m_mas3507d(*this, "mas3507d")
	, m_flash(*this, "flash_u8")
{
}

void isa16_piu10::device_start()
{
	set_isa_device();

	save_item(NAME(m_addr));
	save_item(NAME(m_dest));
	save_item(NAME(m_flash_unlock));
	save_item(NAME(m_mp3_demand));
	save_item(NAME(m_mp3_mpeg_frame_sync));
	save_item(NAME(m_cat702_data));
}

void isa16_piu10::device_reset()
{
	m_addr = m_dest = 0;
	m_flash_unlock = false;

	m_mp3_demand = 1;
	m_mp3_mpeg_frame_sync = 1;

	m_cat702_data = 0;
}

void isa16_piu10::device_add_mconfig(machine_config &config)
{
	MACRONIX_29F1610MC_16BIT(config, m_flash);

	CAT702_PIU(config, m_cat702, 0);
	m_cat702->dataout_handler().set([this] (u16 data) { m_cat702_data = data & 1; });

	DAC3350A(config, m_dac3350a);

	MAS3507D(config, m_mas3507d);
	m_mas3507d->mpeg_frame_sync_cb().set(*this, FUNC(isa16_piu10::mas3507d_mpeg_frame_sync));
	m_mas3507d->demand_cb().set(*this, FUNC(isa16_piu10::mas3507d_demand));
	m_mas3507d->add_route(0, m_dac3350a, 1.0, 0);
	m_mas3507d->add_route(1, m_dac3350a, 1.0, 1);
}

void isa16_piu10::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_isa->install16_device(0x02d0, 0x02df, read16sm_delegate(*this, FUNC(isa16_piu10::read)), write16sm_delegate(*this, FUNC(isa16_piu10::write)));
	}
}

uint16_t isa16_piu10::read(offs_t offset)
{
	switch (offset)
	{
	case 0x5: // 2da
		if (m_dest == 0x008)
		{
			uint16_t r = 0;

			r |= (m_cat702_data & 1) << 5;

			r |= m_mp3_mpeg_frame_sync << 2; // every 256th transition the game will set the internal playback position timestamp to counter*6.2687998 to sync with the MP3 decoder chip (counter*6.2687998/240 = (1152*counter)/44100)
			r |= 1 << 1; // the MP3 data send function loops until this is 1 before sending MP3 data byte
			r |= m_mp3_demand << 0; // if not set then MP3 data send function early returns

			return r;
		}
		else if (m_dest == 0)
		{
			const uint32_t offs = m_addr;
			if (!machine().side_effects_disabled() && m_flash_unlock)
				m_addr++;
			return m_flash->read(offs);
		}
		break;
	}

	if (!machine().side_effects_disabled())
		logerror("%s: unknown read %d %03x %06x %02x\n", machine().describe_context(), m_flash_unlock, m_dest, m_addr, offset);

	return 0;
}

void isa16_piu10::write(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	// address port, bottom 16-bits are location and top 16-bits are target device
	case 0x0: // 2d0
		m_addr &= 0xfff00;
		m_addr |= data & 0xff;
		break;
	case 0x1: // 2d2
		m_addr &= 0xf00ff;
		m_addr |= (data & 0xff) << 8;
		break;

	case 0x2: // 2d4
		m_addr &= 0x0ffff;
		m_addr |= (data & 0xf) << 16;

		m_dest &= 0xff0;
		m_dest |= (data >> 4) & 0xf;
		break;
	case 0x3: // 2d6
		m_dest &= 0x00f;
		m_dest |= (data & 0xff) << 4;
		break;

	// data port
	case 0x5: // 2da
		if (m_dest == 0x010)
		{
			m_cat702->write_datain(BIT(data, 5));
			m_cat702->write_clock(BIT(data, 4));
			m_cat702->write_select(BIT(data, 3));

			m_dac3350a->i2c_sda_w(BIT(data, 1));
			m_dac3350a->i2c_scl_w(BIT(data, 0));
		}
		else if (m_dest == 0x008)
		{
			m_mas3507d->sid_w(data);
		}
		else if (m_dest == 0 && m_flash_unlock)
		{
			m_flash->write(m_addr, data);
		}
		else
		{
			logerror("%s: unknown write %d %03x %06x %02x %02x\n", machine().describe_context(), m_flash_unlock, m_dest, m_addr, offset, data);
		}
		break;

	// chip enable, 0 -> 1 transitions
	case 0x6: // 2dc
		m_flash_unlock = BIT(data, 3);
		break;

	default:
		logerror("%s: unknown write %d %03x %06x %02x %02x\n", machine().describe_context(), m_flash_unlock, m_dest, m_addr, offset, data);
		break;
	}
}

void isa16_piu10::mas3507d_mpeg_frame_sync(int state)
{
	m_mp3_mpeg_frame_sync ^= state;
}

void isa16_piu10::mas3507d_demand(int state)
{
	m_mp3_demand = state;
}
