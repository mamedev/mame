// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 A7800 XBoarD & XM expansions emulation

 The XBoarD should be socketed in the A7800 pcb in place of the Maria chip.
 It adds to the system additional 128K of RAM and an onboard pokey.
 The XM seems to work the same as XBoarD, but it also features HighScore savings
 (using the same ROM as Atari HighScore cart)


 Currently, we emulate both of these as a passthru cart, even if not 100% accurate for the XBoarD


 Memory map:

 POKEY1            $0450    $045F     16 bytes
 POKEY2*           $0460    $046F     16 bytes
 XCTRL             $0470    $047F     1 byte
 RAM               $4000    $7FFF     16384 bytes

 XCTRL Bit Description

 +-------------------------------+
 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 +-------------------------------+
   |   |   |   |   |   |   |   |
   |   |   |   |   |   |   |   +-- Bank select bit 0 \
   |   |   |   |   |   |   +------ Bank select bit 1  | Totally 128 KByte in 16 KByte banks
   |   |   |   |   |   +---------- Bank select bit 2 /
   |   |   |   |   +-------------- Enable memory bit (1 = Memory enabled, 0 after power on)
   |   |   |   +------------------ Enable POKEY bit** (1 = POKEY enabled, 0 after power on)
   |   |   |
   NA  NA  NA = Not Available or Not Used

 * = Can be mounted piggy back on the first POKEY. Description how to do this will come when i have tried it out.
 ** This bit controls both POKEY chip select signals.

 TODO:
  - verify what happens when 2 POKEYs are present

***********************************************************************************************************/


#include "emu.h"
#include "xboard.h"
#include "a78_carts.h"
#include "speaker.h"


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(A78_XBOARD, a78_xboard_device, "a78_xboard", "Atari 7800 XBoarD expansion")
DEFINE_DEVICE_TYPE(A78_XM,     a78_xm_device,     "a78_xm",     "Atari 7800 XM expansion module")


a78_xboard_device::a78_xboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: a78_rom_device(mconfig, type, tag, owner, clock)
	, m_xbslot(*this, "xb_slot")
	, m_pokey(*this, "xb_pokey")
	, m_reg(0), m_ram_bank(0)
{
}


a78_xboard_device::a78_xboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_xboard_device(mconfig, A78_XBOARD, tag, owner, clock)
{
}


a78_xm_device::a78_xm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a78_xboard_device(mconfig, A78_XM, tag, owner, clock)
	, m_ym(*this, "xm_ym2151"), m_ym_enabled(0)
{
}


void a78_xboard_device::device_start()
{
	save_item(NAME(m_reg));
	save_item(NAME(m_ram_bank));
}

void a78_xboard_device::device_reset()
{
	m_reg = 0;
	m_ram_bank = 0;
}

void a78_xm_device::device_start()
{
	save_item(NAME(m_reg));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_ym_enabled));
}

void a78_xm_device::device_reset()
{
	m_reg = 0;
	m_ram_bank = 0;
	m_ym_enabled = 0;
}


void a78_xboard_device::device_add_mconfig(machine_config &config)
{
	A78_CART_SLOT(config, m_xbslot, a7800_cart, nullptr);

	SPEAKER(config, "xb_speaker").front_center();

	POKEY(config, m_pokey, XTAL(14'318'181)/8);
	m_pokey->add_route(ALL_OUTPUTS, "xb_speaker", 1.00);
}

void a78_xm_device::device_add_mconfig(machine_config &config)
{
	A78_CART_SLOT(config, m_xbslot, a7800_cart, nullptr);

	SPEAKER(config, "xb_speaker").front_center();

	POKEY(config, m_pokey, XTAL(14'318'181)/8);
	m_pokey->add_route(ALL_OUTPUTS, "xb_speaker", 1.00);

	YM2151(config, m_ym, XTAL(14'318'181)/4).add_route(ALL_OUTPUTS, "xb_speaker", 1.00);
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 XBoarD: passthru + 128K RAM + POKEY

 -------------------------------------------------*/

READ8_MEMBER(a78_xboard_device::read_40xx)
{
	if (BIT(m_reg, 3) && offset < 0x4000)
		return m_ram[offset + (m_ram_bank * 0x4000)];
	else
		return m_xbslot->read_40xx(space, offset);
}

WRITE8_MEMBER(a78_xboard_device::write_40xx)
{
	if (BIT(m_reg, 3) && offset < 0x4000)
		m_ram[offset + (m_ram_bank * 0x4000)] = data;
	else
		m_xbslot->write_40xx(space, offset, data);
}

READ8_MEMBER(a78_xboard_device::read_04xx)
{
	if (BIT(m_reg, 4) && offset >= 0x50 && offset < 0x60)
		return m_pokey->read(offset & 0x0f);
	else if (BIT(m_reg, 4) && offset >= 0x60 && offset < 0x70)
		return m_xbslot->read_04xx(space, offset - 0x10);   // access second POKEY
	else
		return 0xff;
}

WRITE8_MEMBER(a78_xboard_device::write_04xx)
{
	if (BIT(m_reg, 4) && offset >= 0x50 && offset < 0x60)
		m_pokey->write(offset & 0x0f, data);
	else if (BIT(m_reg, 4) && offset >= 0x60 && offset < 0x70)
		m_xbslot->write_04xx(space, offset - 0x10, data);   // access second POKEY
	else if (offset >= 0x70 && offset < 0x80)
	{
		m_reg = data;
		m_ram_bank = m_reg & 7;
	}
}


/*-------------------------------------------------

 XM: Same as above but also featuring High Score savings

 -------------------------------------------------*/

READ8_MEMBER(a78_xm_device::read_10xx)
{
	return m_nvram[offset];
}

WRITE8_MEMBER(a78_xm_device::write_10xx)
{
	m_nvram[offset] = data;
}

READ8_MEMBER(a78_xm_device::read_30xx)
{
	return m_rom[offset];
}

READ8_MEMBER(a78_xm_device::read_04xx)
{
	if (BIT(m_reg, 4) && offset >= 0x50 && offset < 0x60)
		return m_pokey->read(offset & 0x0f);
	else if (m_ym_enabled && offset >= 0x60 && offset <= 0x61)
		return m_ym->read(offset & 1);
	else if (BIT(m_reg, 4) && offset >= 0x60 && offset < 0x70)
		return m_xbslot->read_04xx(space, offset - 0x10);   // access second POKEY
	else
		return 0xff;
}

WRITE8_MEMBER(a78_xm_device::write_04xx)
{
	if (BIT(m_reg, 4) && offset >= 0x50 && offset < 0x60)
		m_pokey->write(offset & 0x0f, data);
	else if (m_ym_enabled && offset >= 0x60 && offset <= 0x61)
		m_ym->write(offset & 1, data);
	else if (BIT(m_reg, 4) && offset >= 0x60 && offset < 0x70)
		m_xbslot->write_04xx(space, offset - 0x10, data);   // access second POKEY
	else if (offset >= 0x70 && offset < 0x80)
	{
		//printf("regs 0x%X\n", data);
		if (data == 0x84)
			m_ym_enabled = 1;
		m_reg = data;
		m_ram_bank = m_reg & 7;
	}
}
