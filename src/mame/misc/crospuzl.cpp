// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************************************************

    Cross Puzzle

    driver by Angelo Salese, based off original crystal.cpp by ElSemi

    TODO:
    - Dies at POST with a SPU error,
      supposedly it should print a "running system." instead of "Ok" at the
      end of the POST routine.
      Update: it tries to load a "sdata.bin" file, which is nowhere to be found in the dump.
      Considering also that first $20000 block is empty and loading the flash linearly gives
      the reference memory size but then game isn't detected at all.
    - Hooking up nand_device instead of the custom implementation here
      makes the game to print having all memory available and no game
      detected, fun
    - I2C RTC interface should be correct but still doesn't work, sending
      unrecognized slave address 0x30 (device type might be wrong as well)

    Notes:
    - Game enables UART1 receive irq, if that irq is enable it just prints
      "___sysUART1_ISR<LF>___sysUART1_ISR_END<LF>"

=============================================================================

 This PCB uses ADC 'Amazon-LF' SoC, EISC CPU core - However PCBs have been
 seen with a standard VRenderZERO+ MagicEyes EISC chip

****************************************************************************/

#include "emu.h"
#include "cpu/se3208/se3208.h"
#include "machine/pcf8583.h"
#include "machine/nvram.h"
#include "machine/vrender0.h"
#include "emupal.h"
#include "speaker.h"

#include <algorithm>


namespace {

class crospuzl_state : public driver_device
{
public:
	crospuzl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_flash(*this, "flash"),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc"),
		m_rtc(*this, "rtc")
	{ }


	void crospuzl(machine_config &config);

private:

	/* memory pointers */
	required_shared_ptr<uint32_t> m_workram;
	required_region_ptr<uint8_t> m_flash;

	/* devices */
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
	required_device<pcf8583_device> m_rtc;

	uint8_t    m_FlashCmd = 0;
	uint8_t    m_FlashPrevCommand = 0;
	uint32_t   m_FlashAddr = 0;
	uint8_t    m_FlashShift = 0;

//  void Banksw_w(uint32_t data);
	uint8_t FlashCmd_r();
	void FlashCmd_w(uint8_t data);
	void FlashAddr_w(uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void crospuzl_mem(address_map &map) ATTR_COLD;

	// PIO
	uint32_t m_PIO = 0;
	uint32_t m_ddr = 0;
	uint32_t PIOlddr_r();
	void PIOlddr_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t PIOldat_r();
	void PIOldat_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t PIOedat_r();
};


uint32_t crospuzl_state::PIOedat_r()
{
	// TODO: this needs fixing in serflash_device
	// (has a laconic constant for the ready line)
	return (m_rtc->sda_r() << 19)
		| (machine().rand() & 0x04000000); // serial ready line
}

uint8_t crospuzl_state::FlashCmd_r()
{
	if ((m_FlashCmd & 0xff) == 0xff)
	{
		return 0xff;
	}
	if ((m_FlashCmd & 0xff) == 0x90)
	{
		// Service Mode has the first two bytes of the ID printed,
		// in format ****/ee81
		// ee81 has no correspondence in the JEDEC flash vendor ID list,
		// and the standard claims that the ID is 7 + 1 parity bit.
		// TODO: Retrieve ID from actual HW service mode screen.
//      const uint8_t id[5] = { 0xee, 0x81, 0x00, 0x15, 0x00 };
		const uint8_t id[5] = { 0xec, 0xf1, 0x00, 0x95, 0x40 };
		uint8_t res = id[m_FlashAddr];
		m_FlashAddr ++;
		m_FlashAddr %= 5;
		return res;
	}
	if ((m_FlashCmd & 0xff) == 0x30)
	{
		uint8_t res = m_flash[m_FlashAddr];
		m_FlashAddr++;
		return res;
	}
	return 0;
}

void crospuzl_state::FlashCmd_w(uint8_t data)
{
	m_FlashPrevCommand = m_FlashCmd;
	m_FlashCmd = data;
	m_FlashShift = 0;
	m_FlashAddr = 0;
	logerror("%08x %08x CMD\n",m_FlashPrevCommand, m_FlashCmd);
}

void crospuzl_state::FlashAddr_w(uint8_t data)
{
	m_FlashAddr |= data << (m_FlashShift*8);
	m_FlashShift ++;
	if (m_FlashShift == 4)
		logerror("%08x %02x ADDR\n",m_FlashAddr,m_FlashShift);
}

uint32_t crospuzl_state::PIOlddr_r()
{
	return m_ddr;
}

void crospuzl_state::PIOlddr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (BIT(m_ddr, 19) != BIT(data, 19))
		m_rtc->sda_w(BIT(data, 19) ? 1 : BIT(m_PIO, 19));
	if (BIT(m_ddr, 20) != BIT(data, 20))
		m_rtc->scl_w(BIT(data, 20) ? 1 : BIT(m_PIO, 20));
	COMBINE_DATA(&m_ddr);
}

uint32_t crospuzl_state::PIOldat_r()
{
	return m_PIO;
}

// PIO Latched output DATa Register
// TODO: change me
void crospuzl_state::PIOldat_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!BIT(m_ddr, 19))
		m_rtc->sda_w(BIT(data, 19));
	if (!BIT(m_ddr, 20))
		m_rtc->scl_w(BIT(data, 20));

	COMBINE_DATA(&m_PIO);
}

void crospuzl_state::crospuzl_mem(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom().nopw();

	map(0x01500000, 0x01500000).r(FUNC(crospuzl_state::FlashCmd_r));
	map(0x01500100, 0x01500100).w(FUNC(crospuzl_state::FlashCmd_w));
	map(0x01500200, 0x01500200).w(FUNC(crospuzl_state::FlashAddr_w));
	map(0x01510000, 0x01510003).portr("IN0");
	map(0x01511000, 0x01511003).portr("IN1");
	map(0x01512000, 0x01512003).portr("IN2");
	map(0x01513000, 0x01513003).portr("IN3");

	map(0x01600000, 0x01607fff).ram().share("nvram");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x0180001c, 0x0180001f).noprw();
	map(0x01802000, 0x01802003).rw(FUNC(crospuzl_state::PIOlddr_r), FUNC(crospuzl_state::PIOlddr_w));
	map(0x01802004, 0x01802007).rw(FUNC(crospuzl_state::PIOldat_r), FUNC(crospuzl_state::PIOldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(crospuzl_state::PIOedat_r));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));

//  map(0x05000000, 0x05ffffff).bankr("mainbank");
//  map(0x05000000, 0x05000003).rw(FUNC(crospuzl_state::FlashCmd_r), FUNC(crospuzl_state::FlashCmd_w));
}

void crospuzl_state::machine_start()
{
//  save_item(NAME(m_Bank));
	save_item(NAME(m_FlashCmd));
	save_item(NAME(m_PIO));
	save_item(NAME(m_ddr));
}

void crospuzl_state::machine_reset()
{
	m_FlashCmd = 0xff;
	m_FlashAddr = 0;
	m_FlashShift = 0;
	m_FlashPrevCommand = 0xff;
	m_ddr = 0xffffffff;
}

static INPUT_PORTS_START(crospuzl)
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "DSW2" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("PCB-SW1")
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void crospuzl_state::crospuzl(machine_config &config)
{
	SE3208(config, m_maincpu, 14318180 * 3); // FIXME: 72 MHz-ish
	m_maincpu->set_addrmap(AS_PROGRAM, &crospuzl_state::crospuzl_mem);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	VRENDER0_SOC(config, m_vr0soc, 14318180 * 3); // FIXME: 72 MHz-ish
	m_vr0soc->set_host_cpu_tag(m_maincpu);
	m_vr0soc->set_external_vclk(14318180 * 2); // Unknown clock, should output ~70 Hz?

//  ROM strings have references to a K9FXX08 device
//  TODO: use this device, in machine/smartmed.h (has issues with is_busy() emulation)
//  SAMSUNG_K9F1G08U0B(config, m_nand, 0); // TODO: exact flavor

	PCF8583(config, m_rtc, 32.768_kHz_XTAL);
}

ROM_START( crospuzl )
	ROM_REGION( 0x80010, "maincpu", 0 )
	ROM_LOAD("en29lv040a.u5",  0x000000, 0x80010, CRC(d50e8500) SHA1(d681cd18cd0e48854c24291d417d2d6d28fe35c1) )

	ROM_REGION( 0x8400010, "flash", ROMREGION_ERASE00 ) // NAND Flash
	ROM_LOAD("k9f1g08u0a.riser",  0x00000, 0x8400010, BAD_DUMP CRC(7f3c88c3) SHA1(db3169a7b4caab754e9d911998a2ece13c65ce5b) )
	ROM_COPY( "flash",            0x20000, 0x00000, 0x20000 )
ROM_END

} // anonymous namespace


GAME( 200?, crospuzl, 0,        crospuzl, crospuzl, crospuzl_state, empty_init,    ROT0, "<unknown>",           "Cross Puzzle", MACHINE_NOT_WORKING )
