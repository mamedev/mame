// license:BSD-3-Clause
// copyright-holders:Luca Elia, Angelo Salese
/****************************************************************************

    Trivia R Us (c) 2009 AGT

    driver by Angelo Salese, based off original crystal.cpp by ElSemi
	original mods on this driver by Luca Elia

    TODO:
    - touch panel, according to service mode can be generic, atouch or 3M 
	  (microtouch?). It interfaces thru UART0 port;
    - RTC (unknown type);
	- Split romset or add a slot option supporting debug terminal mode;

=============================================================================

****************************************************************************/

#include "emu.h"
#include "cpu/se3208/se3208.h"
#include "machine/nvram.h"
#include "machine/vrender0.h"
#include "machine/microtch.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

class trivrus_state : public driver_device
{
public:
	trivrus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_flash(*this, "flash"),
		m_maincpu(*this, "maincpu"),
		m_mainbank(*this, "mainbank"),
		m_vr0soc(*this, "vr0soc"),
		m_microtouch(*this, "microtouch")
	{ }


	void trivrus(machine_config &config);

private:

	/* memory pointers */
	required_shared_ptr<uint32_t> m_workram;
	required_region_ptr<uint32_t> m_flash;

	/* devices */
	required_device<se3208_device> m_maincpu;
	optional_memory_bank m_mainbank;
	required_device<vrender0soc_device> m_vr0soc;
	required_device<microtouch_device> m_microtouch;

	uint32_t    m_FlashCmd;
	uint32_t    m_Bank;
	uint32_t    m_maxbank;

	DECLARE_READ32_MEMBER(FlashCmd_r);
	DECLARE_WRITE32_MEMBER(FlashCmd_w);
	DECLARE_WRITE32_MEMBER(Banksw_w);

	IRQ_CALLBACK_MEMBER(icallback);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void trivrus_mem(address_map &map);

	// PIO
	DECLARE_READ32_MEMBER(PIOldat_r);
	uint32_t m_PIO;
	DECLARE_WRITE32_MEMBER(PIOldat_w);
	DECLARE_READ32_MEMBER(PIOedat_r);

	DECLARE_READ8_MEMBER(trivrus_input_r);
	DECLARE_WRITE8_MEMBER(trivrus_input_w);
	uint8_t m_trivrus_input;
};

IRQ_CALLBACK_MEMBER(trivrus_state::icallback)
{
	return m_vr0soc->irq_callback();
}

WRITE32_MEMBER(trivrus_state::FlashCmd_w)
{
	m_FlashCmd = data;
}

READ32_MEMBER(trivrus_state::PIOedat_r)
{
	return 0;
}

READ32_MEMBER(trivrus_state::PIOldat_r)
{
	// ...
	return m_PIO;
}

// PIO Latched output DATa Register
WRITE32_MEMBER(trivrus_state::PIOldat_w)
{
	// ...
	COMBINE_DATA(&m_PIO);
}

READ8_MEMBER(trivrus_state::trivrus_input_r)
{
	switch (m_trivrus_input)
	{
		case 1: return ioport("IN1")->read();
		case 2: return ioport("IN2")->read();
		case 3: return ioport("IN3")->read();
		case 4: return ioport("IN4")->read();
		case 5: return ioport("IN5")->read();
		case 6: return ioport("DSW")->read();
	}
	logerror("%s: unknown input %02x read\n", machine().describe_context(), m_trivrus_input);
	return 0xff;
}

WRITE8_MEMBER(trivrus_state::trivrus_input_w)
{
	m_trivrus_input = data & 0xff;
}

READ32_MEMBER(trivrus_state::FlashCmd_r)
{
	if ((m_FlashCmd & 0xff) == 0xff)
	{
		if (m_Bank < m_maxbank)
		{
			uint32_t *ptr = (uint32_t*)(m_mainbank->base());
			return ptr[0];
		}
		else
			return 0xffffffff;
	}
	if ((m_FlashCmd & 0xff) == 0x90)
	{
		if (m_Bank < m_maxbank)
			return 0x00180089;  //Intel 128MBit
		else
			return 0xffffffff;
	}
	return 0;
}


WRITE32_MEMBER(trivrus_state::Banksw_w)
{
	m_Bank = (data >> 1) & 7;
	m_mainbank->set_entry(m_Bank);
}

void trivrus_state::trivrus_mem(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom().nopw();

	map(0x01280000, 0x01280003).w(FUNC(trivrus_state::Banksw_w));

	map(0x01500000, 0x01500000).rw(FUNC(trivrus_state::trivrus_input_r), FUNC(trivrus_state::trivrus_input_w));
	// reads occurs by SELECTING the given register on successive ODD addresses then reading at 0x01500011
	// bit 0 of 1500010 looks some kind of busy flag (game tight loops if on)
	// on write:
//  0x01500010 = sec
//  0x01500012 = min
//  0x01500014 = hour
//  0x01500016 = day
//  0x01500018 = month
//  0x0150001c = year - 2000
	// all regs are in BCD format
	map(0x01600000, 0x01607fff).ram().share("nvram");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802004, 0x01802007).rw(FUNC(trivrus_state::PIOldat_r), FUNC(trivrus_state::PIOldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(trivrus_state::PIOedat_r));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));

	map(0x05000000, 0x05ffffff).bankr("mainbank");
	map(0x05000000, 0x05000003).rw(FUNC(trivrus_state::FlashCmd_r), FUNC(trivrus_state::FlashCmd_w));
//  0x06000000 accessed during POST during above check then discarded, probably a debug left-over
}

void trivrus_state::machine_start()
{
	if (m_mainbank)
	{
		m_maxbank = (m_flash) ? m_flash.bytes() / 0x1000000 : 0;
		uint8_t *dummy_region = auto_alloc_array(machine(), uint8_t, 0x1000000);
		std::fill_n(&dummy_region[0], 0x1000000, 0xff); // 0xff Filled at Unmapped area
		uint8_t *ROM = (m_flash) ? (uint8_t *)&m_flash[0] : dummy_region;
		for (int i = 0; i < 8; i++)
		{
			if ((i < m_maxbank))
				m_mainbank->configure_entry(i, ROM + i * 0x1000000);
			else
				m_mainbank->configure_entry(i, dummy_region);
		}
	}

	save_item(NAME(m_Bank));
	save_item(NAME(m_FlashCmd));
	save_item(NAME(m_PIO));
	save_item(NAME(m_trivrus_input));
}

void trivrus_state::machine_reset()
{
	m_Bank = 0;
	m_mainbank->set_entry(m_Bank);
	m_FlashCmd = 0xff;
}

static INPUT_PORTS_START( trivrus )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Left/True")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Down")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Enter/Exit")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Next")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Right/False")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) // with 1 impulse it misses often
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Sound")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Free Game Switch")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW ) PORT_NAME("Setup")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Monitor Type" )
	PORT_DIPSETTING(    0x01, "VGA" )
	PORT_DIPSETTING(    0x00, "Normal" )
	PORT_DIPNAME( 0x02, 0x02, "UART Monitor Mode" ) // communicates via UART0 port to an unknown device (presumably a terminal, unemulated)
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Touch Screen" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void trivrus_state::trivrus(machine_config &config)
{
	SE3208(config, m_maincpu, 14318180 * 3); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &trivrus_state::trivrus_mem);
	m_maincpu->set_irq_acknowledge_callback(FUNC(trivrus_state::icallback));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	VRENDER0_SOC(config, m_vr0soc, 14318180 * 3);
	m_vr0soc->set_host_cpu_tag(m_maincpu);
	m_vr0soc->set_external_vclk(28636360);
	m_vr0soc->tx_callback<0>().set(m_microtouch, FUNC(microtouch_device::rx));
	
	// TODO: 3M from service mode, most likely wrong?
	MICROTOUCH(config, m_microtouch, 9600).stx().set(m_vr0soc, FUNC(vrender0soc_device::rx_w<0>));
}

ROM_START( trivrus )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u4", 0x00000, 0x80000, CRC(2d2e9a11) SHA1(73e7b19a032eae21312ca80f8c42cc16725496a7) )

	ROM_REGION32_LE( 0x2000000, "flash", ROMREGION_ERASEFF ) // Flash
	ROM_LOAD( "u3", 0x000000, 0x1000010, CRC(ba901707) SHA1(e281ba07024cd19ef1ab72d2197014f7b1f4d30f) )
ROM_END

GAME( 2009, trivrus,  0,        trivrus,  trivrus,  trivrus_state, empty_init,    ROT0, "AGT",                 "Trivia R Us (v1.07)", 0 )
