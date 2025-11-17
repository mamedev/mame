// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
/**************************************************************************************************

 'RISC PC' hardware

 See See Find Out [Icarus 1999]
 Pang Pang Car [Icarus 1999]
 Tetris Fighters [Sego Entertainment 2001]

 driver by
  Tomasz Slanina

TODO:
 - Correct IOMD type (CL-PS7500 -> ARM7500FE?)
 - 24c01 eeprom (IOLINES)
 - timing
 - unknown reads/writes
 - improve sound hook up
 - ppcar: uses the VIDC internal DAC for SFX and the QS1000 for music.
   It's configured to use the undumped internal ROM.

===================================================================================================

See See Find Out
Icarus, 1999

PCB Layout
----------

ARIAMEDIA M/B Rev 1.0
|--------------------------------------|
|          B.U29          C.U12        |
|      A.U28     *     *     D.U13     |
|                               1008S-1|
|                    |------|          |
|    KM416C1204      |QS1000|   E.U14  |
|J       KM416C1204  |------| |--------|
|A                     24MHz  |DU2  DU3|
|M     |---------|            |        |
|M     |CL-PS7500|  |------|  |DU5  DU6|
|A     |         |  |QL2003|  |--------|
|      |         |  |      | 24C01A.U36|
|      |         |  |------|           |
|      |---------|          14.31818MHz|
|            54MHz  |------|           |
|                   |PRIME |           |
|                   |------|   DIPSW(8)|
|--------------------------------------|

Notes:

      Chips:
         QS1000: QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
           PRIME: LGS Prime 3C 9930R, clock input of 14.31818MHz (QFP100)
         QL2003: QuickLogic QL2003-XPL84C 0003BA FPGA (PLCC84)
      CL-PS7500: Cirrus Logic CL-PS7500FE-56QC-A 84903-951BD ARM 9843J
                 clock input of 54.000MHz, ARM710C; ARM7-core CPU (QFP240)
     KM416C1204: Samsung KM416C1204CJ-5 2M x8 DRAM (SOJ42)
              *: Unpopulated DIP32 socket

      ROMs:
           24C010A: Amtel 24C01A (128bytes x8Bit Serial EEPROM, DIP8)
        A, B, C, D: TMS27C040 512K x8 EPROM (DIP32)
                 E: ST M27C512 64K x8 EPROM (DIP28)
           1008S-1: HWASS 1008S-1 Wavetable Audio Samples chip, 1M x8 MaskROM (SOP32)
         DU2, DU3,: Samsung KM29W32000AT 32MBit NAND Flash 3.3V Serial EEPROM (TSOP44)
         DU5, DU6   These ROMs are mounted on a small plug-in daughterboard. There are additional
                    mounting pads for 4 more of these ROMs but they're not populated.


===================================================================================================

Pang Pang Car
Icarus, 1999

This game runs on hardware that is similar to that used on 'See See Find Out'
The game is a rip-off of RallyX

PCB Layout
----------

|--------------------------------------|
|    KM416C1204                        |
|    KM416C1204    U24  U25  U26*  U27*|
|  DA1311A                             |
|  DA1311A        |---------|54MHz     |
|  4558           |CL-PS7500| LED      |
|J                |         | |--------|
|A                |         | |DU2  DU3|
|M                |         | |        |
|M EL2386       @ |---------| |DU5  DU6|
|A       &                    |--------|
|                                      |
|  7660             |------|           |
|                   |PRIME |14.31818MHz|
|                   |------|           |
| NASN9289  XILINX                     |
| QS1000    XCS10    MAX232    DIPSW(8)|
|  24MHz     #   7705                  |
|--------------------|DB9|-------------|
                     |---|
Notes:
      Chips:
         QS1000: QDSP QS1000 AdMOS 9638R, Wavetable Audio chip, clock input of 24.000MHz (QFP100)
          PRIME: LGS Prime 3C 9849R, clock input of 14.31818MHz (QFP100)
   XILINX XCS10: Xilinx Spartan XCS10 FPGA (QFP144)
      CL-PS7500: Cirrus Logic CL-PS7500FE-56QC-A 84877-951BD ARM 9843J
                 clock input of 54.000MHz, ARM710C; ARM7-core CPU (QFP240)
     KM416C1204: Samsung KM416C1204CJ-5 2M x8 DRAM (SOJ42)
           7705: Reset/Watchdog IC (SOIC8)
           7660: DC-DC Voltage Convertor (SOIC8)
         EL2386: Elantec Semiconductor 250MHz Triple Current Feedback Op Amp with Disable (SOIC16)
              *: Unpopulated DIP32 sockets
              &: Unpopulated location for QFP100 IC
              #: Unpopulated location for SOJ42 RAM
              @: Unpopulated location for OSC1

      ROMs:
          U24, U25: AMD 29F040B 512k x8 FlashROM (DIP32)
          NASN9289: Re-badged SOP32 ROM. Should be compatible with existing QS100x Wavetable Audio Sample ROMs,
                    Dumped as 1M x8 SOP32 MaskROM
         DU2, DU3,: Samsung KM29N32000TS 32MBit NAND Flash 3.3V Serial EEPROM (TSOP44)
         DU5, DU6   These ROMs are mounted on a small plug-in daughterboard. There are additional
                    mounting pads for 4 more of these ROMs but they're not populated.

**************************************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "machine/acorn_vidc.h"
#include "machine/arm_iomd.h"
#include "machine/i2cmem.h"
#include "sound/qs1000.h"
#include "diserial.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class ssfindo_state : public driver_device, public device_serial_interface
{
public:
	ssfindo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, device_serial_interface(mconfig, *this)
		, m_maincpu(*this, "maincpu")
		, m_vidc(*this, "vidc")
		, m_iomd(*this, "iomd")
		, m_i2cmem(*this, "i2cmem")
		, m_flashrom(*this, "flash")
		, m_qs1000(*this, "qs1000")
		, m_in(*this, "IN%u", 0U)
		, m_dsw(*this, "DSW")
		{ }

	void ssfindo(machine_config &config);
	void ppcar(machine_config &config);

	void init_ssfindo();
	void init_ppcar();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<arm_vidc20_device> m_vidc;
	required_device<arm7500fe_iomd_device> m_iomd;
	optional_device<i2cmem_device> m_i2cmem;
	required_region_ptr<uint16_t> m_flashrom;
	required_device<qs1000_device> m_qs1000;
	required_ioport_array<2> m_in;
	required_ioport m_dsw;

	void init_common();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	typedef void (ssfindo_state::*speedup_func)();
	speedup_func m_speedup{};

	uint32_t m_flashType = 0;

	bool m_i2cmem_clock = false;

	virtual void tra_callback() override;
	void sound_w(uint8_t data);

private:

	// ssfindo and ppcar
	uint32_t m_flashAdr = 0;
	uint32_t m_flashOffset = 0;
	uint32_t m_adrLatch = 0;
	uint32_t m_flashN = 0;

	// ssfindo and ppcar
	uint32_t io_r();
	void io_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// ssfindo
	void debug_w(uint32_t data);
	uint32_t ff4_r();
	uint32_t SIMPLEIO_r();

	// ppcar
	uint32_t randomized_r();

	void ssfindo_speedups();
	void ppcar_speedups();

	void ppcar_map(address_map &map) ATTR_COLD;
	void ssfindo_map(address_map &map) ATTR_COLD;

	uint8_t iolines_r();
	void iolines_w(uint8_t data);
	bool m_flash_bank_select = false;

	bool m_txd = true;
};

class tetfight_state : public ssfindo_state
{
public:
	tetfight_state(const machine_config &mconfig, device_type type, const char *tag)
		: ssfindo_state(mconfig, type, tag)
		{}

	void init_tetfight();
	void tetfight(machine_config &config);

protected:

private:
	bool m_i2cdata_hack = false;

	void tetfight_map(address_map &map) ATTR_COLD;

	int iocr_od0_r();
	int iocr_od1_r();
	void iocr_od0_w(int state);
	void iocr_od1_w(int state);
	uint32_t tetfight_unk_r();
};

// TODO: eeprom  24c01 & 24c02
// TODO: untangle, kill hacks
uint8_t ssfindo_state::iolines_r()
{
	if(m_flashType == 1)
		return 0;
	else
		return machine().rand();
}

void ssfindo_state::iolines_w(uint8_t data)
{
	if(data&0xc0)
		m_adrLatch=0;

	if(m_maincpu->pc() == 0xbac0 && m_flashType == 1)
		m_flashN=data&1;

	m_flash_bank_select = BIT(data, 0);
}

// inverted compared to riscpc.cpp
// TODO: simplify hookup over i2cmem having AND 1 on state for a writeline (?)
int tetfight_state::iocr_od1_r()
{
	// TODO: completely get rid of this speedup fn or move anywhere else
	//if (m_speedup) (this->*m_speedup)();
	return (m_i2cmem->read_sda() ? 1 : 0); //eeprom read
}

int tetfight_state::iocr_od0_r()
{
	return (m_i2cmem_clock == true ? 1 : 0); //eeprom read
}

void tetfight_state::iocr_od1_w(int state)
{
	// TODO: i2c cares about the order of this!?
	// tetfight reaches PC=0x106c if initialization has success
	// rpc700 doesn't like above and go into supervisor mode even with a valid NVRAM
	// both wants clock first then data, not sure where the issue actually is
	m_i2cdata_hack = state == true;
	//m_i2cmem->write_sda(state == true ? 1 : 0);
}

void tetfight_state::iocr_od0_w(int state)
{
	m_i2cmem_clock = state;
	m_i2cmem->write_scl(state == true ? 1 : 0);
	m_i2cmem->write_sda(m_i2cdata_hack == true ? 1 : 0);
}

void ssfindo_state::ssfindo_speedups()
{
	if (m_maincpu->pc()==0x2d6c8) // ssfindo
		m_maincpu->spin_until_time(attotime::from_usec(20));
	else if (m_maincpu->pc()==0x2d6bc) // ssfindo
		m_maincpu->spin_until_time(attotime::from_usec(20));
}

void ssfindo_state::ppcar_speedups()
{
	if (m_maincpu->pc()==0x000bc8) // ppcar
		m_maincpu->spin_until_time(attotime::from_usec(20));
	else if (m_maincpu->pc()==0x000bbc) // ppcar
		m_maincpu->spin_until_time(attotime::from_usec(20));
}

uint32_t ssfindo_state::io_r()
{
	int adr=m_flashAdr*0x200+(m_flashOffset);

	switch(m_flashType)
	{
		case 0:
			//bit 0 of IOLINES  = flash select ( 5/6 or 3/2 )
			if (m_flash_bank_select)
				adr+=0x400000;
			break;

		case 1:
			adr+=0x400000*m_flashN;
			break;
	}

	if(adr<0x400000*2)
	{
		m_flashOffset++;
		return m_flashrom[adr];
	}
	return 0;
}

void ssfindo_state::io_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t temp = 0;
	COMBINE_DATA(&temp);

#if 0
	logerror("[io_w] = %x @%x [latch=%x]\n",data,m_maincpu->pc(),m_adrLatch);
#endif

	if(m_adrLatch==1)
		m_flashAdr=(temp>>16)&0xff;
	if(m_adrLatch==2)
	{
		m_flashAdr|=(temp>>16)&0xff00;
		m_flashOffset=0;
	}
	m_adrLatch=(m_adrLatch+1)%3;
}

void ssfindo_state::debug_w(uint32_t data)
{
#if 0
	osd_printf_debug("%c",data&0xff); //debug texts - malloc (ie "64 KBytes allocated, elapsed : 378 KBytes, free : 2231 KBytes")
#endif
}

uint32_t ssfindo_state::ff4_r()
{
	return machine().rand()&0x20;
}

uint32_t ssfindo_state::SIMPLEIO_r()
{
	return machine().rand()&1;
}

uint32_t ssfindo_state::randomized_r()
{
	return machine().rand();
}

void ssfindo_state::sound_w(uint8_t data)
{
	transmit_register_setup(data);
	m_maincpu->spin_until_time(attotime::from_usec(2000)); // give time to the QS1000 CPU to react. TODO: sync things correctly
}


void ssfindo_state::ssfindo_map(address_map &map)
{
	map(0x00000000, 0x000fffff).rom();
	map(0x03012e60, 0x03012e67).noprw();
	map(0x03012fe0, 0x03012fe3).w(FUNC(ssfindo_state::debug_w));
	map(0x03012ff0, 0x03012ff3).noprw();
	map(0x03012ff4, 0x03012ff7).nopw().r(FUNC(ssfindo_state::ff4_r)); //status flag ?
	map(0x03012ff8, 0x03012fff).noprw();
	map(0x03200000, 0x032001ff).m(m_iomd, FUNC(arm7500fe_iomd_device::map));
	map(0x03240000, 0x03240003).portr(m_in[0]).nopw();
	map(0x03241000, 0x03241003).portr(m_in[1]).nopw();
	map(0x03242000, 0x03242003).r(FUNC(ssfindo_state::io_r)).w(FUNC(ssfindo_state::io_w));
	map(0x03243000, 0x03243003).portr(m_dsw).nopw();
	map(0x03245002, 0x03245002).w(FUNC(ssfindo_state::sound_w));
	map(0x0324f000, 0x0324f003).r(FUNC(ssfindo_state::SIMPLEIO_r));
	map(0x03400000, 0x037fffff).w(m_vidc, FUNC(arm_vidc20_device::write));
	map(0x10000000, 0x11ffffff).ram();
}

void ssfindo_state::ppcar_map(address_map &map)
{
	map(0x00000000, 0x000fffff).rom();
	//map(0x03012be0, 0x03012be0).w(FUNC(ssfindo_state::sound_w)); // once the internal ROM is dumped
	map(0x03012bf4, 0x03012bf7).r(FUNC(ssfindo_state::randomized_r)).nopw();
	map(0x03012de0, 0x03012de3).portr(m_dsw);
	map(0x03012e60, 0x03012e67).nopw();
	map(0x03012ff8, 0x03012ffb).portr(m_in[0]).nopw();
	map(0x03200000, 0x032001ff).m(m_iomd, FUNC(arm7500fe_iomd_device::map));
	map(0x032c0000, 0x032c0003).portr(m_in[1]).nopw();
	map(0x03340000, 0x03340007).nopw();
	map(0x03341000, 0x0334101f).nopw();
	map(0x033c0000, 0x033c0003).r(FUNC(ssfindo_state::io_r)).w(FUNC(ssfindo_state::io_w));
	map(0x03400000, 0x037fffff).w(m_vidc, FUNC(arm_vidc20_device::write));
	map(0x08000000, 0x08ffffff).ram();
	map(0x10000000, 0x10ffffff).ram();
}

uint32_t tetfight_state::tetfight_unk_r()
{
	//sound status ?
	return machine().rand();
}

void tetfight_state::tetfight_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom();
	map(0x03200000, 0x032001ff).m(m_iomd, FUNC(arm7500fe_iomd_device::map));
	map(0x03240000, 0x03240003).portr(m_in[0]);
	map(0x03240004, 0x03240007).portr(m_in[1]);
	map(0x03240008, 0x0324000b).portr(m_dsw);
	map(0x03240020, 0x03240023).r(FUNC(tetfight_state::tetfight_unk_r));
	map(0x03240020, 0x03240020).w(FUNC(tetfight_state::sound_w));
	map(0x03400000, 0x037fffff).w(m_vidc, FUNC(arm_vidc20_device::write));
	map(0x10000000, 0x14ffffff).ram();
}

void ssfindo_state::machine_start()
{
	// TODO: convert to exact flash device
	save_item(NAME(m_flashAdr));
	save_item(NAME(m_flashOffset));
	save_item(NAME(m_adrLatch));
	save_item(NAME(m_flashN));
	save_item(NAME(m_i2cmem_clock));
	save_item(NAME(m_flash_bank_select));

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(31250);
}

void ssfindo_state::machine_reset()
{
	// ...
	receive_register_reset();
	transmit_register_reset();

	m_txd = true;
}

void ssfindo_state::tra_callback()
{
	m_txd = transmit_register_get_data_bit();
}

static INPUT_PORTS_START( ssfindo )
	// TODO: incomplete structs
	PORT_START("IN0")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )               // IPT_START2 ??
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1    ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1  )

	PORT_START("IN1")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1    ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW 2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW 3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW 4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW 5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW 6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ppcar )
	PORT_START("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1   )

	PORT_START("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT   ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1   )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, "Credits to Start" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPNAME( 0x0c, 0x00, "Missiles" )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "2" ) // same
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tetfight )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1    ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON2    ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON3    ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1    ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2    ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON3    ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Test Mode" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW 1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW 2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Initialize" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x010, "DSW 4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x040, "DSW 6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Number of rounds" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x00, "1" )
INPUT_PORTS_END


void ssfindo_state::ssfindo(machine_config &config)
{
	ARM7(config, m_maincpu, 54_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ssfindo_state::ssfindo_map);

	I2C_24C01(config, m_i2cmem);

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	ARM_VIDC20(config, m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_iomd, FUNC(arm_iomd_device::vblank_irq));
	m_vidc->sound_drq().set(m_iomd, FUNC(arm_iomd_device::sound_drq));

	ARM7500FE_IOMD(config, m_iomd, 54_MHz_XTAL);
	m_iomd->set_host_cpu_tag(m_maincpu);
	m_iomd->set_vidc_tag(m_vidc);
	m_iomd->iolines_read().set(FUNC(ssfindo_state::iolines_r));
	m_iomd->iolines_write().set(FUNC(ssfindo_state::iolines_w));

	SPEAKER(config, "speaker", 2).front();

	qs1000_device &qs1000(QS1000(config, "qs1000", 24_MHz_XTAL));
	qs1000.set_external_rom(true);
	// qs1000.p1_out().set(FUNC()); // TODO: writes something here
	qs1000.p3_in().set([this]() { return u8(0xfeU | m_txd); });
	qs1000.add_route(0, "speaker", 0.25, 0);
	qs1000.add_route(1, "speaker", 0.25, 1);
}

void ssfindo_state::ppcar(machine_config &config)
{
	ssfindo(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &ssfindo_state::ppcar_map);

	subdevice<qs1000_device>("qs1000")->set_external_rom(false); // ppcar has no external ROM
	subdevice<i8052_device>("qs1000:cpu")->set_disable(); // internal ROM hasn't been dumped yet

	config.device_remove("i2cmem");
}

void tetfight_state::tetfight(machine_config &config)
{
	ssfindo(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &tetfight_state::tetfight_map);

	I2C_24C02(config.replace(), m_i2cmem);

	m_iomd->iocr_read_od<0>().set(FUNC(tetfight_state::iocr_od0_r));
	m_iomd->iocr_read_od<1>().set(FUNC(tetfight_state::iocr_od1_r));
	m_iomd->iocr_write_od<0>().set(FUNC(tetfight_state::iocr_od0_w));
	m_iomd->iocr_write_od<1>().set(FUNC(tetfight_state::iocr_od1_w));
}

ROM_START( ssfindo )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD16_BYTE( "a.u28",   0x000000, 0x80000, CRC(c93edbd3) SHA1(9c703cfef49b59ccd5d68bab9bd59344bd18d67e) )
	ROM_LOAD16_BYTE( "b.u29",   0x000001, 0x80000, CRC(39ecb9e4) SHA1(9ebd3962d8014b97c68c364729248ed22f9298a4) )

	ROM_REGION16_LE(0x1000000, "flash", 0 ) /* flash roms */
	ROM_LOAD16_BYTE( "du5",     0x000000, 0x400000, CRC(b32bd453) SHA1(6d5694bfcc67102256f857932b83b38f62ca2010) )
	ROM_LOAD16_BYTE( "du6",     0x000001, 0x400000, CRC(00559591) SHA1(543aefddc02f6a521d3bd5e6e3d8e42127ff9baa) )

	ROM_LOAD16_BYTE( "du3",     0x800000, 0x400000, CRC(d1e8afb2) SHA1(598dfcbba14435a1d0571dcefe0ec62fec657fca) )
	ROM_LOAD16_BYTE( "du2",     0x800001, 0x400000, CRC(56998515) SHA1(9b71a44f56a545ff0c1170775c839d21bd01f545) )

	ROM_REGION(0x80, "i2cmem", 0 ) /* eeprom */
	ROM_LOAD( "24c01a.u36",     0x00, 0x80, CRC(b4f4849b) SHA1(f8f17dc94b2a305048693cfb78d14be57310ce56) )

	ROM_REGION(0x10000, "qs1000:cpu", 0 ) /* qdsp code */
	ROM_LOAD( "e.u14", 0x00000, 0x10000, CRC(49976f7b) SHA1(eba5b97b81736f3c184ae0c19f1b10c5ae250d51) )

	ROM_REGION(0x1000000, "qs1000", 0 ) /* HWASS 1008S-1  qdsp samples */
	ROM_LOAD( "c.u12",       0x000000, 0x080000, CRC(d24b5e56) SHA1(d89983cf4b0a6e0e4137f3799bdbcfd72c7bebe4) )
	ROM_LOAD( "d.u11",       0x080000, 0x080000, CRC(c0fdd82a) SHA1(a633045e0f5c144b4e24e04fb9446522fdb222f4) )
	ROM_LOAD( "1008s-1.u16", 0x100000, 0x100000, CRC(9aef9545) SHA1(f23ef72c3e3667923768dfdd0c5b4951b23dcbcf) )
ROM_END

ROM_START( ppcar )
	ROM_REGION(0x100000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD16_BYTE( "fk0.u24", 0x000000, 0x80000, CRC(1940a483) SHA1(9456361fd25bf037b53bd2d04764a33b299d96dd) )
	ROM_LOAD16_BYTE( "fk1.u25", 0x000001, 0x80000, CRC(75ad8679) SHA1(392288e56350e3cc49aaca82edf26f2a9e346f21) )

	ROM_REGION16_LE(0x1000000, "flash", 0 ) /* flash roms */
	ROM_LOAD16_BYTE( "du5",     0x000000, 0x400000, CRC(d4b7374a) SHA1(54c93a4235f495ba3794aea511b19db821a8acb1) )
	ROM_LOAD16_BYTE( "du6",     0x000001, 0x400000, CRC(e95a3a62) SHA1(2b1c889d208a749e3d7e4c75588c9c1f979e88d9) )

	ROM_LOAD16_BYTE( "du3",     0x800000, 0x400000, CRC(73882474) SHA1(191b64e662542b5322160c99af8e00079420d473) )
	ROM_LOAD16_BYTE( "du2",     0x800001, 0x400000, CRC(9250124a) SHA1(650f4b89c92fe4fb63fc89d4e08c4c4c611bebbc) )

	ROM_REGION(0x100000, "qs1000", 0 ) /* HWASS 1008S-1  qdsp samples */
	ROM_LOAD( "nasn9289.u9",    0x000000, 0x100000, CRC(9aef9545) SHA1(f23ef72c3e3667923768dfdd0c5b4951b23dcbcf) )
ROM_END

ROM_START( tetfight )
	ROM_REGION(0x200000, "maincpu", 0 ) /* ARM 32 bit code */
	ROM_LOAD( "u42",        0x000000, 0x200000, CRC(9101c4d2) SHA1(39da953de734e687ebbf976c821bf1017830f36c) )

	ROM_REGION16_LE(0x1000000, "flash", ROMREGION_ERASEFF ) /* flash roms */
	/* nothing? */

	ROM_REGION(0x100, "i2cmem", 0 ) /* 24c02 eeprom */
	ROM_LOAD( "u1",     0x00, 0x100, CRC(dd207b40) SHA1(6689d9dfa980bdfbd4e4e6cef7973e22ebbfe22e) )

	ROM_REGION(0x10000, "qs1000:cpu", 0 ) /* qdsp code */
	ROM_LOAD( "u12",        0x000000, 0x10000, CRC(49976f7b) SHA1(eba5b97b81736f3c184ae0c19f1b10c5ae250d51) ) // 27c512 = e.u14 on ssfindo

	ROM_REGION(0x1000000, "qs1000", 0 ) /* samples - same internal structure as qdsp samples  */
	ROM_LOAD( "u11",         0x000000, 0x80000, CRC(073050f6) SHA1(07f362f3ba468bde2341a99e6b26931d11459a92) ) // 27c040
	ROM_LOAD( "u15",         0x080000, 0x80000, CRC(477f8089) SHA1(8084facb254d60da7983d628d5945d27b9494e65) ) // 27c040
	ROM_LOAD( "1008s-1.u16", 0x100000, 0x100000, CRC(9aef9545) SHA1(f23ef72c3e3667923768dfdd0c5b4951b23dcbcf) ) // probably the same as the other games, but wasn't dumped for this one
ROM_END

void ssfindo_state::init_common()
{
	m_speedup = nullptr;
	//m_PS7500timer0 = timer_alloc(FUNC(ssfindo_state::PS7500_Timer0_callback), this);
	//m_PS7500timer1 = timer_alloc(FUNC(ssfindo_state::PS7500_Timer1_callback), this);

	//save_item(NAME(m_PS7500_IO));
	//save_item(NAME(m_PS7500_FIFO));
}

void ssfindo_state::init_ssfindo()
{
	init_common();
	m_flashType = 0;
	m_speedup = &ssfindo_state::ssfindo_speedups;
}

void ssfindo_state::init_ppcar()
{
	init_ssfindo();
	m_flashType = 1;
	m_speedup = &ssfindo_state::ppcar_speedups;
}

void tetfight_state::init_tetfight()
{
	init_common();
	m_flashType = 0;
}

} // Anonymous namespace


GAME( 1999, ssfindo, 0,        ssfindo,  ssfindo,  ssfindo_state,  init_ssfindo,  ROT0, "Icarus", "See See Find Out", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, ppcar,   0,        ppcar,    ppcar,    ssfindo_state,  init_ppcar,    ROT0, "Icarus", "Pang Pang Car",    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2001, tetfight,0,        tetfight, tetfight, tetfight_state, init_tetfight, ROT0, "Sego",   "Tetris Fighters",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // severe frame hiccups
