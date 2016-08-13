// license:BSD-3-Clause
// copyright-holders:Carl

#include "emu.h"
#include "cpu/i86/i286.h"
#include "machine/at.h"
#include "bus/isa/isa_cards.h"

class vis_state : public driver_device
{
public:
	vis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb(*this, "mb")
		{ }
	required_device<cpu_device> m_maincpu;
	required_device<at_mb_device> m_mb;

	DECLARE_READ8_MEMBER(sysctl_r);
	DECLARE_WRITE8_MEMBER(sysctl_w);
	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_WRITE8_MEMBER(unk_w);
	DECLARE_READ8_MEMBER(unk2_r);
	DECLARE_READ8_MEMBER(unk3_r);
	DECLARE_READ8_MEMBER(cdrom_r);
	DECLARE_WRITE8_MEMBER(cdrom_w);
protected:
	void machine_reset() override;
private:
	UINT8 m_sysctl;
	UINT8 m_unkidx;
	UINT8 m_unk[16];
	UINT8 m_cdcmd, m_cdstat;
};

void vis_state::machine_reset()
{
	m_cdcmd = 0;
	m_cdstat = 0;
	m_sysctl = 0;
}

READ8_MEMBER(vis_state::unk_r)
{
	if(offset)
		return m_unk[m_unkidx];
	return 0;
}

WRITE8_MEMBER(vis_state::unk_w)
{
	if(offset)
		m_unk[m_unkidx] = data;
	else
		m_unkidx = data & 0xf;
}

READ8_MEMBER(vis_state::unk2_r)
{
	return 0x40;
}

READ8_MEMBER(vis_state::unk3_r)
{
	return 0x00;
}

// probably a mitsumi isa non-atapi cdrom controller, mcd.c in older linux versions
READ8_MEMBER(vis_state::cdrom_r)
{
	if(m_cdcmd)
	{
		if(offset)
		{
			int ret = m_cdstat;
			m_cdstat = 0;
			return ret;
		}
		else
		{
			m_cdcmd = 0;
			return 0x80;
		}
	}
	if(offset)
		return 0;
	else
		return 0x20;
}

WRITE8_MEMBER(vis_state::cdrom_w)
{
	if(!offset)
	{
		m_cdcmd = data;
		m_cdstat = 4;
	}
}

READ8_MEMBER(vis_state::sysctl_r)
{
	return m_sysctl;
}

WRITE8_MEMBER(vis_state::sysctl_w)
{
	if(BIT(data, 0) && !BIT(m_sysctl, 0))
		m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	//m_maincpu->set_input_line(INPUT_LINE_A20, BIT(data, 1) ? CLEAR_LINE : ASSERT_LINE);
	m_sysctl = data;
}

static ADDRESS_MAP_START( at16_map, AS_PROGRAM, 16, vis_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x09ffff) AM_RAM
	AM_RANGE(0x0d8000, 0x0fffff) AM_ROM AM_REGION("bios", 0xd8000)
	AM_RANGE(0x100000, 0x15ffff) AM_RAM
	AM_RANGE(0xff0000, 0xffffff) AM_ROM AM_REGION("bios", 0xf0000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( at16_io, AS_IO, 16, vis_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0026, 0x0027) AM_READWRITE8(unk_r, unk_w, 0xffff)
	AM_RANGE(0x006a, 0x006b) AM_READ8(unk2_r, 0x00ff)
	AM_RANGE(0x0092, 0x0093) AM_READWRITE8(sysctl_r, sysctl_w, 0x00ff)
	AM_RANGE(0x0000, 0x00ff) AM_DEVICE("mb", at_mb_device, map)
	AM_RANGE(0x0310, 0x0311) AM_READWRITE8(cdrom_r, cdrom_w, 0xffff)
	AM_RANGE(0x031a, 0x031b) AM_READ8(unk3_r, 0x00ff)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( vis, vis_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, XTAL_12MHz )
	MCFG_CPU_PROGRAM_MAP(at16_map)
	MCFG_CPU_IO_MAP(at16_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259_master", pic8259_device, inta_cb)
	MCFG_80286_SHUTDOWN(DEVWRITELINE("mb", at_mb_device, shutdown))

	MCFG_DEVICE_ADD("mb", AT_MB, 0)

	MCFG_ISA16_SLOT_ADD("mb:isabus","vga", pc_isa16_cards, "clgd542x", true)
MACHINE_CONFIG_END

ROM_START(vis)
	ROM_REGION(0x100000,"bios", 0)
	ROM_LOAD( "p513bk0b.bin", 0x00000, 0x80000, CRC(364e3f74) SHA1(04260ef1e65e482c9c49d25ace40e22487d6aab9))
	ROM_LOAD( "p513bk1b.bin", 0x80000, 0x80000, CRC(e18239c4) SHA1(a0262109e10a07a11eca43371be9978fff060bc5))
ROM_END

COMP ( 1992, vis,  0, 0, vis, 0, driver_device, 0, "Tandy/Memorex", "Video Information System MD-2500", MACHINE_NOT_WORKING )

