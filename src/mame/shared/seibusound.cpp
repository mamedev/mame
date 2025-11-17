// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, R. Belmont
/***************************************************************************

    Seibu Sound System v1.02, designed 1986 by Seibu Kaihatsu

    The Seibu sound system comprises of a Z80A, a YM3812, a YM3931*, and
    an Oki MSM6295.  As well as sound the Z80 can controls coins and pass
    data to the main cpu.  There are a few little quirks that make it
    worthwhile emulating in a separate file:

    * The YM3812 generates interrupt RST10, by asserting the interrupt line,
    and placing 0xd7 on the data bus.

    * The main cpu generates interrupt RST18, by asserting the interrupt line,
    and placing 0xdf on the data bus.

    A problem can occur if both the YM3812 and the main cpu try to assert
    the interrupt line at the same time.  The effect in the old Mame
    emulation would be for sound to stop playing - this is because a RST18
    cancelled out a RST10, and if a single RST10 is dropped sound stops
    as the YM3812 timer is not reset.  The problem occurs because even
    if both interrupts happen at the same time, there can only be one value
    on the data bus.  Obviously the real hardware must have some circuit
    to prevent this.  It is emulated by user timers to control the z80
    interrupt vector.

    * The YM3931 is the main/sub cpu interface, similar to Konami's K054986A
      or Taito's TC0140SYT.  It also provides the Z80 memory map and
      interrupt control.  It's not a Yamaha chip :-)

    Emulation by Bryan McPhail, mish@tendril.co.uk
    ADPCM by R. Belmont and Jarek Burczynski

***************************************************************************/

#include "emu.h"
#include "seibusound.h"

#include "sound/okiadpcm.h"
#include "sound/okim6295.h"


/***************************************************************************
    Seibu Sound System
***************************************************************************/

/*
    Games using encrypted sound cpu:

    Air Raid         1987   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Cabal            1988   "Michel/Seibu    sound 11/04/88"
    Dead Angle       1988?  "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Dynamite Duke    1989   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Toki             1989   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Raiden (alt)     1990   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."

    raiden and the decrypted raidena are not identical, there are vast sections of different data.
    However, there are a few different bytes in the middle of identical data, suggesting a possible
    error in the decryption scheme: they all require an additional XOR with 0x20 and are located at
    similar addresses.
    00002422: 03 23
    000024A1: 00 20
    000024A2: 09 29
    00002822: 48 68
    000028A1: 06 26
    00002A21: 17 37
    00002A22: 00 20
    00002AA1: 12 32
    00002C21: 02 22
    00002CA1: 02 22
    00002CA2: 17 37
*/

DEFINE_DEVICE_TYPE(SEIBU_SOUND, seibu_sound_device, "seibu_sound", "Seibu Sound System")

seibu_sound_device::seibu_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEIBU_SOUND, tag, owner, clock)
	, m_int_cb(*this)
	, m_coin_io_cb(*this, 0)
	, m_ym_read_cb(*this, 0)
	, m_ym_write_cb(*this)
	, m_sound_rom(*this, finder_base::DUMMY_TAG)
	, m_rom_bank(*this, finder_base::DUMMY_TAG)
	, m_main2sub{0}
	, m_sub2main{0}
	, m_main2sub_pending(false)
	, m_sub2main_pending(false)
	, m_rst10_irq(false)
	, m_rst18_irq(false)
	, m_rst10_service(false)
	, m_rst18_service(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void seibu_sound_device::device_start()
{
	if (m_sound_rom.found() && m_rom_bank.found())
	{
		if (m_sound_rom.length() > 0x10000)
		{
			m_rom_bank->configure_entries(0, (m_sound_rom.length() - 0x10000) / 0x8000, &m_sound_rom[0x10000], 0x8000);

			/* Denjin Makai definitely needs this at start-up, it never writes to the bankswitch */
			m_rom_bank->set_entry(0);
		}
		else
			m_rom_bank->set_base(&m_sound_rom[0x8000]);
	}

	m_main2sub[0] = m_main2sub[1] = 0;
	m_sub2main[0] = m_sub2main[1] = 0;

	save_item(NAME(m_main2sub));
	save_item(NAME(m_sub2main));
	save_item(NAME(m_main2sub_pending));
	save_item(NAME(m_sub2main_pending));
	save_item(NAME(m_rst10_irq));
	save_item(NAME(m_rst18_irq));
	save_item(NAME(m_rst10_service));
	save_item(NAME(m_rst18_service));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void seibu_sound_device::device_reset()
{
	update_irq_lines(VECTOR_INIT);
}

void seibu_sound_device::update_irq_lines(s32 param)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(seibu_sound_device::update_irq_synced), this), param);
}

TIMER_CALLBACK_MEMBER(seibu_sound_device::update_irq_synced)
{
	switch (param)
	{
	case VECTOR_INIT:
		m_rst10_irq = m_rst18_irq = false;
		m_rst10_service = m_rst18_service = false;
		break;

	case RST10_ASSERT:
		m_rst10_irq = true;
		break;

	case RST10_CLEAR:
		m_rst10_irq = false;
		break;

	case RST10_ACKNOWLEDGE:
		m_rst10_service = true;
		break;

	case RST10_EOI:
		m_rst10_service = false;
		break;

	case RST18_ASSERT:
		m_rst18_irq = true;
		break;

	case RST18_ACKNOWLEDGE:
		m_rst18_service = true;
		m_rst18_irq = false;
		break;

	case RST18_EOI:
		m_rst18_service = false;
		break;
	}

	m_int_cb((m_rst10_irq && !m_rst10_service) || (m_rst18_irq && !m_rst18_service) ? ASSERT_LINE : CLEAR_LINE);
}

IRQ_CALLBACK_MEMBER(seibu_sound_device::im0_vector_cb)
{
	if (m_rst18_irq && !m_rst18_service)
	{
		if (!machine().side_effects_disabled())
			update_irq_lines(RST18_ACKNOWLEDGE);
		return 0xdf;
	}
	else if (m_rst10_irq && !m_rst10_service)
	{
		if (!machine().side_effects_disabled())
			update_irq_lines(RST10_ACKNOWLEDGE);
		return 0xd7;
	}
	else
	{
		if (!machine().side_effects_disabled())
			logerror("Spurious interrupt taken\n");
		return 0x00;
	}
}


void seibu_sound_device::irq_clear_w(u8)
{
	/* Denjin Makai and SD Gundam doesn't like this, it's tied to the rst18 ack ONLY so it could be related to it. */
	//update_irq_lines(VECTOR_INIT);

	update_irq_lines(RST18_EOI);
}

void seibu_sound_device::rst10_ack_w(u8)
{
	update_irq_lines(RST10_EOI);
}

void seibu_sound_device::rst18_ack_w(u8)
{
	update_irq_lines(RST18_EOI);
}

void seibu_sound_device::fm_irqhandler(int state)
{
	update_irq_lines(state ? RST10_ASSERT : RST10_CLEAR);
}

u8 seibu_sound_device::ym_r(offs_t offset)
{
	return m_ym_read_cb(offset);
}

void seibu_sound_device::ym_w(offs_t offset, u8 data)
{
	m_ym_write_cb(offset, data);
}

void seibu_sound_device::bank_w(u8 data)
{
	if (m_rom_bank.found())
		m_rom_bank->set_entry(BIT(data, 0));
}

u8 seibu_sound_device::coin_r(offs_t offset)
{
	return m_coin_io_cb(offset);
}

void seibu_sound_device::coin_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}

u8 seibu_sound_device::soundlatch_r(offs_t offset)
{
	return m_main2sub[offset];
}

u8 seibu_sound_device::main_data_pending_r()
{
	return m_sub2main_pending ? 1 : 0;
}

void seibu_sound_device::main_data_w(offs_t offset, u8 data)
{
	m_sub2main[offset] = data;
}

void seibu_sound_device::pending_w(u8)
{
	/* just a guess */
	m_main2sub_pending = false;
	m_sub2main_pending = true;
}

u8 seibu_sound_device::main_r(offs_t offset)
{
	//logerror("%s: seibu_main_r(%x)\n",machine().describe_context(),offset);
	switch (offset)
	{
		case 2:
		case 3:
			return m_sub2main[offset - 2];
		case 5:
			return m_main2sub_pending ? 1 : 0;
		default:
			//logerror("%s: seibu_main_r(%x)\n",machine().describe_context(),offset);
			return 0xff;
	}
}

void seibu_sound_device::main_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0:
		case 1:
			m_main2sub[offset] = data;
			break;
		case 4:
			update_irq_lines(RST18_ASSERT);
			break;
		case 2: //Sengoku Mahjong writes here
		case 6:
			/* just a guess */
			m_sub2main_pending = false;
			m_main2sub_pending = true;
			break;
		default:
			//logerror("%s: seibu_main_w(%x,%02x)\n",machine().describe_context(),offset,data);
			break;
	}
}

// used only by NMK16 bootlegs
void seibu_sound_device::main_mustb_w(offs_t, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_main2sub[0] = data & 0xff;
	if (ACCESSING_BITS_8_15)
		m_main2sub[1] = data >> 8;

//  logerror("seibu_main_mustb_w: %x -> %x %x\n", data, main2sub[0], main2sub[1]);

	update_irq_lines(RST18_ASSERT);
}

/***************************************************************************/

void seibu_sound_common::seibu_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x27ff).ram();
	map(0x4000, 0x4000).w("seibu_sound", FUNC(seibu_sound_device::pending_w));
	map(0x4001, 0x4001).w("seibu_sound", FUNC(seibu_sound_device::irq_clear_w));
	map(0x4002, 0x4002).w("seibu_sound", FUNC(seibu_sound_device::rst10_ack_w));
	map(0x4003, 0x4003).w("seibu_sound", FUNC(seibu_sound_device::rst18_ack_w));
	map(0x4007, 0x4007).w("seibu_sound", FUNC(seibu_sound_device::bank_w));
	map(0x4008, 0x4009).rw("seibu_sound", FUNC(seibu_sound_device::ym_r), FUNC(seibu_sound_device::ym_w));
	map(0x4010, 0x4011).r("seibu_sound", FUNC(seibu_sound_device::soundlatch_r));
	map(0x4012, 0x4012).r("seibu_sound", FUNC(seibu_sound_device::main_data_pending_r));
	map(0x4013, 0x4013).r("seibu_sound", FUNC(seibu_sound_device::coin_r));
	map(0x4018, 0x4019).w("seibu_sound", FUNC(seibu_sound_device::main_data_w));
	map(0x401b, 0x401b).w("seibu_sound", FUNC(seibu_sound_device::coin_w));
	map(0x6000, 0x6000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x8000, 0xffff).bankr("seibu_bank");
}

/***************************************************************************/

/***************************************************************************
    Seibu ADPCM device
    (MSM5205 with interface to sample ROM provided by YM3931)
***************************************************************************/

DEFINE_DEVICE_TYPE(SEIBU_ADPCM, seibu_adpcm_device, "seibu_adpcm", "Seibu ADPCM interface")

seibu_adpcm_device::seibu_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEIBU_ADPCM, tag, owner, clock)
	, m_msm(*this, finder_base::DUMMY_TAG)
	, m_current(0)
	, m_end(0)
	, m_nibble(0)
	, m_playing(false)
	, m_base(*this, DEVICE_SELF)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void seibu_adpcm_device::device_start()
{
	save_item(NAME(m_current));
	save_item(NAME(m_end));
	save_item(NAME(m_nibble));
	save_item(NAME(m_playing));
}

void seibu_adpcm_device::device_reset()
{
	m_playing = false;
	m_msm->reset_w(1);
}

// "decrypt" is a bit flowery here, as it's probably just line-swapping to
// simplify PCB layout/routing rather than intentional protection, but it
// still fits, especially since the Z80s for all these games are truly encrypted.

void seibu_adpcm_device::decrypt()
{
	for (int i = 0; i < m_base.length(); i++)
	{
		m_base[i] = bitswap<8>(m_base[i], 7, 5, 3, 1, 6, 4, 2, 0);
	}
}

void seibu_adpcm_device::adr_w(offs_t offset, u8 data)
{
	if (offset)
	{
		m_end = data << 8;
	}
	else
	{
		m_current = data << 8;
		m_nibble = 4;
	}
}

void seibu_adpcm_device::ctl_w(u8 data)
{
	// sequence is 00 02 01 each time.
	switch (data)
	{
		case 0:
			m_msm->reset_w(1);
			m_playing = false;
			break;
		case 2:
			break;
		case 1:
			m_msm->reset_w(0);
			m_playing = true;
			break;
	}
}

void seibu_adpcm_device::msm_int(int state)
{
	if (!state || !m_playing)
		return;

	u8 const val = (m_base[m_current] >> m_nibble) & 15;
	m_msm->data_w(val);

	m_nibble ^= 4;
	if (m_nibble == 4)
	{
		m_current++;
		if (m_current >= m_end)
		{
			m_msm->reset_w(1);
			m_playing = false;
		}
	}
}
