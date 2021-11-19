// license:BSD-3-Clause
// copyright-holders:R. Belmont, smf
/***************************************************************************

  Konami GV System (aka "Baby Phoenix") - Arcade PSX Hardware
  ===========================================================
  Driver by R. Belmont & smf


***************************************************************************
Konami GV
Hardware Info by Guru
---------------------

Konami GV uses Playstation 1 based hardware.


Known Games
-----------

Game       Description                                              Mother Board   Code       Version       Date   Time

powyak96   Jikkyou Powerful Pro Yakyuu '96                          GV999          GV017   JAPAN 1.03   96.05.27  18:00
hyperath   Hyper Athlete                                            ZV610          GV021   JAPAN 1.00   96.06.09  19:00
lacrazyc   Let's Attack Crazy Cross                                 ZV610          GV027   ASIA  1.10   96.01.18  12:00
susume     Susume! Taisen Puzzle-Dama                               ZV610          GV027   JAPAN 1.20   96.03.04  12:00
btchamp    Beat the Champ                                           GV999          GV053   UAA01        ?
kdeadeye   Dead Eye                                                 GV999          GV054   UAA01        ?
weddingr   Wedding Rhapsody                                         ?              GX624   JAA          97.05.29   9:12
tmosh      Tokimeki Memorial Oshiete Your Heart                     GV999          GQ673   JAA          97.03.14  ?
tmoshs     Tokimeki Memorial Oshiete Your Heart Seal Version        GV999          GE755   JAA          97.08.06  11:52
tmoshspa   Tokimeki Memorial Oshiete Your Heart Seal Version Plus   GV999          GE756   JAA          97.08.24  12:20
tmoshsp    Tokimeki Memorial Oshiete Your Heart Seal Version Plus   GV999          GE756   JAB          97.09.27   9:10
nagano98   Winter Olypmics in Nagano 98                             GV999          GX720   EAA01 1.03   98.01.08  10:45
naganoj    Hyper Olympic in Nagano                                  GV999          GX720   JAA01 1.02   98.01.07  01:10
simpbowl   Simpsons Bowling                                         GV999          GQ829   UAA          ?

Notes:

The Tokimeki Memorial Oshiete Your Heart games use an extra PCB plugged in on top for controlling the printer and the sensors.
Additionally, there is a small PCB for connecting to a sensor... PCB number GE755-PWB(S)

Simpsons Bowling uses an extra PCB plugged in on top containing flash ROMs and circuitry to control the trackball.

Some of the other games may also have extra PCBs.



PCB Layouts
-----------

ZV610 PWB301331
|---------------------------------------|
|   000180       056602      LM324   CN8|
|CN2                                    |
|                                       |
|      999A01.7E                     CN6|
|                         CXD2922BQ     |
|      10E                KM416V256BLT-7|
|                                       |
|J     12E                              |
|A CXD2923AR     058239                 |
|M                                      |
|M                     CXD8530BQ        |
|A   D482445LGW-A70            93CF96-2 |
|               CXD8514Q               S|
|    D482445LGW-A70                    C|
|                      67.7376MHz      S|
|         53.693175MHz                 I|
|                                 32MHz |
|    93C46   KM48V514BJ-6  KM48V514BJ-6 |
|            KM48V514BJ-6  KM48V514BJ-6 |
|    CN5      CN3                001231 |
|---------------------------------------|

GV999 PWB301949A
|---------------------------------------|
|                056602      LM324   CN8|
|CN2                                    |
|TEST_SW                                |
|      999A01.7E                     CN6|
|MC44200         CN4      CXD2925Q      |
|      9E                 TC51V4260BJ-80|
|                                       |
|J     12E                              |
|A               058239                 |
|M  53.693175MHz                        |
|M                     CXD8530CQ        |
|A                             93CF96-2 |
|      CXD8561Q                        S|
|              KM4132G271Q-12          C|
|                      67.7376MHz      S|
|         53.693175MHz                 I|
|                                 32MHz |
|    93C46   KM48V514BJ-6  KM48V514BJ-6 |
|            KM48V514BJ-6  KM48V514BJ-6 |
|    CN5      CN3                001231 |
|---------------------------------------|

Notes:

      - These two PCBs are functionally equivalent and can be exchanged between games and work fine (see CD-swapping note below).

      - Simpsons Bowling and Dead Eye use a GV999 with a daughtercard containing flash ROMs and CPLDs:
        PWB402610
        Xilinx XC3020A
        Xilinx 1718DPC
        74F244N (2 of these)
        LVT245SS (2 of theses)
        On Simpsons Bowling, this also has one µPD4701AC and an empty space for a second.

      - 000180 is used for driving the RGB output. It's a very thin piece of very brittle ceramic
        containing a circuit, a LM1203 chip, some smt transistors/caps/resistors etc (let's just say
        placing this thing on the edge of the PCB wasn't a good design choice!)
        On GV999, it has been replaced by three transistors and a MC44200.

      - 056602 seems to be some sort of A/D converter (another ceramic thing containing caps/resistors/transistors and a chip)

      - CXD2922 and CXD2925 are SPU's.

      - The BIOS on ZV610 and GV999 is identical. It's a 4M mask ROM, compatible with 27C040.

      - The CD contains one MODE 1 data track and several Redbook audio tracks which are streamed to the speaker via CN8.

      - The ZV and GV PCB's are virtually identical aside from some minor component shuffling and the RGB output mechanism.
        However note that the GPU revision is different between the two boards and so are some of the other Sony IC's.

      - CN8 used to connect redbook audio output from CD drive to PCB.

      - CN6 used to connect power to CD drive.

      - CN2 used for extra speaker connection for stereo output.

      - CN3, CN5 used for connecting 3rd and 4th player controls.

      - CN4 is present only on GV999 and is used to connect extra PCBs with additional functionality.
        For example:
        Simpsons Bowling additional flash ROM & trackball control PCB.
        Tokimeki Memorial Oshiete Your Heart printer and sensor control PCB.

      - 001231, 058239 are PALCE16V8H PALs.

      - 10E, 12E are unpopulated positions for 16M TSOP56 FLASHROMs (10E is 9E on GV999).

      - If the CD is swapped to another GV game, the game will boot but will stop with an error '25C MBAD' (the EEPROM is 25C)
        So the games can not be swapped by simply exchanging CDs because the EEPROM will not re-init itself if the CDROM is swapped.
        This appears to be some form of mild protection to stop operators swapping CD's.
        However it is possible to swap games to another PCB by exchanging the CD _AND_ the EEPROM from another PCB which belongs
        to that same game. It won't work with a blank EEPROM or a different games' EEPROM.


Tokimeki Memorial Oshiete Your Heart control PCB Layout
-------------------------------------------------------

GQ673 PWB404691A

    |------------------------|
    |CN11                    |
    |             003673     |----------|
    |CN10   74QST3383    LVT245  CN3    |
    |                               CN4 |
    |CN9  2SC2320        LS245          |
    |     2SC2320        LS273          |
    |CN8  2SC2320        HCT04          |
    |     2SC2320  MB3516A              |
    |CN7           14.31818MHz          |
    |                                   |
    |CN6                         2SC2320|
|---|             uPC324                |---|
|CN5    2SC2320  ADC0838         CXA1585Q   |
|                PC817     VR   3.579545MHz |
|-------------------------------------------|

Notes: (all main parts shown)

       This PCB is plugged into the Tokimeki Memorial Oshiete Your Heart main board into CN4
       It provides additional functionality for the printer and sensor(s) and possibly other things.

       CN10 & CN11 - TCS7927-54 4-pin mini DIN connectors
               CN9 - 6 pin connector
               CN8 - 5 pin connector
               CN7 - 7 pin connector
               CN6 - 3 pin connector
               CN5 - 2 pin connector
               CN3 - 4 pin power connector. Joins to CN6 on mainboard via a Y-splitter cable. The other end of the
                     Y cable is approximately 300mm long and joins to the GE755-PWB(S) PCB.
               CN4 - 6 pin connector used to power the CDROM drive
            003673 - PAL16V8D
         74QST3383 - Quality Semiconductor Inc. High Speed CMOS Bus Exchange Switch
           2SC2320 - NPN Transistor equivalent to 2SC945
           MB3516A - Fujitsu MB3516A RGB Encoder
            uPC324 - NEC uPC324 Low power quad operational amplifier
           ADC0838 - Analog Devices ADC0838 8-Bit Serial I/O A/D Converter with Multiplexer Options
             PC817 - Sharp PC817 Optocoupler
                VR - 500 Ohm potentiometer
          CXA1585Q - Sony CXA1585Q RGB Decoder

Tokimeki Memorial Oshiete Your Heart Sensor PCB
-----------------------------------------------

GQ673 PWB404691A
|----------------|
| LS14  E756S1   |
|         uPC817 |
| CN1  CN2  CN3  |
|----------------|
Notes:
      E756S1 - PAL16V8H
      uPC817 - Sharp PC817 Optocoupler
         CN1 - 4 pin power connector joining to GQ673 PCB CN3 and CN6 on mainboard via a Y-splitter cable.
     CN1/CN2 - 2 pin connector

***************************************************************************/


#include "emu.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd.h"
#include "cpu/psx/psx.h"
#include "machine/am53cf96.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "machine/mb89371.h"
#include "machine/upd4701.h"
#include "machine/ram.h"
#include "sound/cdda.h"
#include "sound/spu.h"
#include "video/psx.h"
#include "screen.h"
#include "speaker.h"
#include "cdrom.h"


namespace {

class konamigv_state : public driver_device
{
public:
	konamigv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_am53cf96(*this, "am53cf96")
		, m_btc_trackball(*this, "upd%u", 1)
		, m_maincpu(*this, "maincpu")
	{
	}

	void tmosh(machine_config &config);
	void kdeadeye(machine_config &config);
	void btchamp(machine_config &config);
	void konamigv(machine_config &config);

protected:
	void konamigv_map(address_map &map);

	virtual void machine_start() override;

	void btc_trackball_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t tokimeki_serial_r();
	void tokimeki_serial_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void scsi_dma_read( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size );
	void scsi_dma_write( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size );

	void btchamp_map(address_map &map);
	void kdeadeye_map(address_map &map);
	void tmosh_map(address_map &map);

	static void cdrom_config(device_t *device);

	required_device<am53cf96_device> m_am53cf96;
	optional_device_array<upd4701_device, 2> m_btc_trackball;

	uint8_t m_sector_buffer[ 4096 ];
	required_device<cpu_device> m_maincpu;
};

class simpbowl_state : public konamigv_state
{
public:
	simpbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: konamigv_state(mconfig, type, tag)
		, m_flash8(*this, "flash%u", 0)
	{
	}

	void simpbowl(machine_config &config);

private:
	virtual void machine_start() override;

	uint16_t flash_r(offs_t offset);
	void flash_w(offs_t offset, uint16_t data);

	void simpbowl_map(address_map &map);

	required_device_array<fujitsu_29f016a_device, 4> m_flash8;

	uint32_t m_flash_address;
};

void konamigv_state::konamigv_map(address_map &map)
{
	map(0x1f000000, 0x1f00001f).rw(m_am53cf96, FUNC(am53cf96_device::read), FUNC(am53cf96_device::write)).umask32(0x00ff00ff);
	map(0x1f100000, 0x1f100003).portr("P1");
	map(0x1f100004, 0x1f100007).portr("P2");
	map(0x1f100008, 0x1f10000b).portr("P3_P4");
	map(0x1f180000, 0x1f180003).portw("EEPROMOUT");
	map(0x1f680000, 0x1f68001f).rw("mb89371", FUNC(mb89371_device::read), FUNC(mb89371_device::write)).umask32(0x00ff00ff);
	map(0x1f780000, 0x1f780003).nopw(); // watchdog?
}

void simpbowl_state::simpbowl_map(address_map &map)
{
	konamigv_map(map);

	map(0x1f680080, 0x1f68008f).rw(FUNC(simpbowl_state::flash_r), FUNC(simpbowl_state::flash_w));
	map(0x1f6800c0, 0x1f6800c7).r("upd", FUNC(upd4701_device::read_xy)).umask32(0xff00ff00);
	map(0x1f6800c9, 0x1f6800c9).r("upd", FUNC(upd4701_device::reset_xy_r));
}

void konamigv_state::btchamp_map(address_map &map)
{
	konamigv_map(map);

	map(0x1f380000, 0x1f3fffff).rw("flash", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x1f680080, 0x1f680087).r(m_btc_trackball[0], FUNC(upd4701_device::read_xy)).umask32(0xff00ff00);
	map(0x1f680080, 0x1f680087).r(m_btc_trackball[1], FUNC(upd4701_device::read_xy)).umask32(0x00ff00ff);
	map(0x1f680088, 0x1f680089).w(FUNC(konamigv_state::btc_trackball_w));
	map(0x1f6800e0, 0x1f6800e3).nopw();
}

void konamigv_state::kdeadeye_map(address_map &map)
{
	konamigv_map(map);

	map(0x1f380000, 0x1f3fffff).rw("flash", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x1f680080, 0x1f680083).portr("GUNX1");
	map(0x1f680090, 0x1f680093).portr("GUNY1");
	map(0x1f6800a0, 0x1f6800a3).portr("GUNX2");
	map(0x1f6800b0, 0x1f6800b3).portr("GUNY2");
	map(0x1f6800c0, 0x1f6800c3).portr("BUTTONS");
	map(0x1f6800e0, 0x1f6800e3).nopw();
}

void konamigv_state::tmosh_map(address_map &map)
{
	konamigv_map(map);

	map(0x1f680080, 0x1f680081).r(FUNC(konamigv_state::tokimeki_serial_r));
	map(0x1f680090, 0x1f680091).w(FUNC(konamigv_state::tokimeki_serial_w));
}

// SCSI

void konamigv_state::scsi_dma_read( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size )
{
	uint8_t *sector_buffer = m_sector_buffer;
	int i;
	int n_this;

	while( n_size > 0 )
	{
		if( n_size > sizeof( m_sector_buffer ) / 4 )
		{
			n_this = sizeof( m_sector_buffer ) / 4;
		}
		else
		{
			n_this = n_size;
		}
		if( n_this < 2048 / 4 )
		{
			// non-READ commands
			m_am53cf96->dma_read_data( n_this * 4, sector_buffer );
		}
		else
		{
			// assume normal 2048 byte data for now
			m_am53cf96->dma_read_data( 2048, sector_buffer );
			n_this = 2048 / 4;
		}
		n_size -= n_this;

		i = 0;
		while( n_this > 0 )
		{
			p_n_psxram[ n_address / 4 ] =
				( sector_buffer[ i + 0 ] << 0 ) |
				( sector_buffer[ i + 1 ] << 8 ) |
				( sector_buffer[ i + 2 ] << 16 ) |
				( sector_buffer[ i + 3 ] << 24 );
			n_address += 4;
			i += 4;
			n_this--;
		}
	}
}

void konamigv_state::scsi_dma_write( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size )
{
	uint8_t *sector_buffer = m_sector_buffer;
	int i;
	int n_this;

	while( n_size > 0 )
	{
		if( n_size > sizeof( m_sector_buffer ) / 4 )
		{
			n_this = sizeof( m_sector_buffer ) / 4;
		}
		else
		{
			n_this = n_size;
		}
		n_size -= n_this;

		i = 0;
		while( n_this > 0 )
		{
			sector_buffer[ i + 0 ] = ( p_n_psxram[ n_address / 4 ] >> 0 ) & 0xff;
			sector_buffer[ i + 1 ] = ( p_n_psxram[ n_address / 4 ] >> 8 ) & 0xff;
			sector_buffer[ i + 2 ] = ( p_n_psxram[ n_address / 4 ] >> 16 ) & 0xff;
			sector_buffer[ i + 3 ] = ( p_n_psxram[ n_address / 4 ] >> 24 ) & 0xff;
			n_address += 4;
			i += 4;
			n_this--;
		}

		m_am53cf96->dma_write_data( i, sector_buffer );
	}
}

void konamigv_state::machine_start()
{
	save_item(NAME(m_sector_buffer));
}

void simpbowl_state::machine_start()
{
	konamigv_state::machine_start();
	save_item(NAME(m_flash_address));
}

void konamigv_state::cdrom_config(device_t *device)
{
	device->subdevice<cdda_device>("cdda")->add_route(0, "^^lspeaker", 1.0);
	device->subdevice<cdda_device>("cdda")->add_route(1, "^^rspeaker", 1.0);
	device = device->subdevice("cdda");
}

void konamigv_state::konamigv(machine_config &config)
{
	// basic machine hardware
	CXD8530BQ(config, m_maincpu, XTAL(67'737'600));
	m_maincpu->set_addrmap(AS_PROGRAM, &konamigv_state::konamigv_map);
	m_maincpu->subdevice<psxdma_device>("dma")->install_read_handler(5, psxdma_device::read_delegate(&konamigv_state::scsi_dma_read, this));
	m_maincpu->subdevice<psxdma_device>("dma")->install_write_handler(5, psxdma_device::write_delegate(&konamigv_state::scsi_dma_write, this));
	m_maincpu->subdevice<ram_device>("ram")->set_default_size("2M");

	MB89371(config, "mb89371", 0);
	EEPROM_93C46_16BIT(config, "eeprom");

	scsi_port_device &scsi(SCSI_PORT(config, "scsi", 0));
	scsi.set_slot_device(1, "cdrom", SCSICD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_4));
	scsi.slot(1).set_option_machine_config("cdrom", cdrom_config);

	AM53CF96(config, m_am53cf96, 0);
	m_am53cf96->set_scsi_port("scsi");
	m_am53cf96->irq_handler().set("maincpu:irq", FUNC(psxirq_device::intin10));

	// video hardware
	CXD8514Q(config, "gpu", XTAL(53'693'175), 0x100000, subdevice<psxcpu_device>("maincpu")).set_screen("screen");

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	spu_device &spu(SPU(config, "spu", XTAL(67'737'600)/2, subdevice<psxcpu_device>("maincpu")));
	spu.add_route(0, "lspeaker", 0.75);
	spu.add_route(1, "rspeaker", 0.75);
}


static INPUT_PORTS_START( konamigv )
	PORT_START("P1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00001000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER( "eeprom", eeprom_serial_93cxx_device, do_read )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_P4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("EEPROMOUT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
INPUT_PORTS_END

// Simpsons Bowling

uint16_t simpbowl_state::flash_r(offs_t offset)
{
	if (offset == 4)   // set odd address
	{
		m_flash_address |= 1;
	}

	if (offset == 0)
	{
		int chip = (m_flash_address >= 0x200000) ? 2 : 0;

		int ret = ( m_flash8[chip]->read(m_flash_address & 0x1fffff) & 0xff ) |
			( m_flash8[chip+1]->read(m_flash_address & 0x1fffff) << 8 );

		m_flash_address++;

		return ret;
	}

	return 0;
}

void simpbowl_state::flash_w(offs_t offset, uint16_t data)
{
	int chip;

	switch (offset)
	{
		case 0:
			chip = (m_flash_address >= 0x200000) ? 2 : 0;
			m_flash8[chip]->write(m_flash_address & 0x1fffff, data&0xff);
			m_flash8[chip+1]->write(m_flash_address & 0x1fffff, (data>>8)&0xff);
			break;

		case 1:
			m_flash_address = 0;
			m_flash_address |= (data<<1);
			break;

		case 2:
			m_flash_address &= 0xff00ff;
			m_flash_address |= (data<<8);
			break;

		case 3:
			m_flash_address &= 0x00ffff;
			m_flash_address |= (data<<15);
			break;
	}
}

void simpbowl_state::simpbowl(machine_config &config)
{
	konamigv(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &simpbowl_state::simpbowl_map);

	FUJITSU_29F016A(config, "flash0");
	FUJITSU_29F016A(config, "flash1");
	FUJITSU_29F016A(config, "flash2");
	FUJITSU_29F016A(config, "flash3");

	upd4701_device &upd(UPD4701A(config, "upd"));
	upd.set_portx_tag("TRACK0_X");
	upd.set_porty_tag("TRACK0_Y");
}

static INPUT_PORTS_START( simpbowl )
	PORT_INCLUDE( konamigv )

	PORT_START("TRACK0_X")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACK0_Y")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

INPUT_PORTS_END

// Beat the Champ

void konamigv_state::btc_trackball_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
//  osd_printf_debug( "w %08x %08x %08x %08x\n", m_maincpu->pc(), offset, data, mem_mask );

	for (int i = 0; i < 2; i++)
	{
		m_btc_trackball[i]->cs_w(BIT(data, 1));
		m_btc_trackball[i]->resetx_w(!BIT(data, 0));
		m_btc_trackball[i]->resety_w(!BIT(data, 0));
	}
}

void konamigv_state::btchamp(machine_config &config)
{
	konamigv(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &konamigv_state::btchamp_map);

	SHARP_LH28F400(config, "flash");

	UPD4701A(config, m_btc_trackball[0]);
	m_btc_trackball[0]->set_portx_tag("TRACK0_X");
	m_btc_trackball[0]->set_porty_tag("TRACK0_Y");

	UPD4701A(config, m_btc_trackball[1]);
	m_btc_trackball[1]->set_portx_tag("TRACK1_X");
	m_btc_trackball[1]->set_porty_tag("TRACK1_Y");
}

static INPUT_PORTS_START( btchamp )
	PORT_INCLUDE( konamigv )

	PORT_START("TRACK0_X")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

	PORT_START("TRACK0_Y")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(1)

	PORT_START("TRACK1_X")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)

	PORT_START("TRACK1_Y")
	PORT_BIT( 0xfff, 0x000, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(63) PORT_PLAYER(2)
INPUT_PORTS_END

// Tokimeki Memorial games - have a mouse and printer and who knows what else

uint16_t konamigv_state::tokimeki_serial_r()
{
	// bits checked: 0x80 and 0x20 for periodic status (800b6968 and 800b69e0 in tmoshs)
	// 0x08 for reading the serial device (8005e624)

	return 0xffff;
}

void konamigv_state::tokimeki_serial_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	    serial EEPROM-like device here: when mem_mask == 0x000000ff only,

	    0x40 = chip enable
	    0x20 = clock
	    0x10 = data

	    tmoshs sends 6 bits: 110100 then reads 8 bits.
	    readback is bit 3 (0x08) of serial_r
	    This happens starting around 8005e580.
	*/

}

void konamigv_state::tmosh(machine_config &config)
{
	konamigv(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &konamigv_state::tmosh_map);
}

/*
Dead Eye

CD:
    P/N 002715
    054
    UA
    A01
*/

void konamigv_state::kdeadeye(machine_config &config)
{
	konamigv(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &konamigv_state::kdeadeye_map);

	SHARP_LH28F400(config, "flash");
}

static INPUT_PORTS_START( kdeadeye )
	PORT_INCLUDE( konamigv )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0000007f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P2")
	PORT_BIT( 0x0000007f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P3_P4")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("GUNX1")
	PORT_BIT( 0xffff, 0x0100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x004c, 0x01bb ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 1 )

	PORT_START("GUNY1")
	PORT_BIT( 0xffff, 0x0077, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 1 )

	PORT_START("GUNX2")
	PORT_BIT( 0xffff, 0x0100, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x004c, 0x01bb ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 2 )

	PORT_START("GUNY2")
	PORT_BIT( 0xffff, 0x0077, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY( 100 ) PORT_KEYDELTA( 5 ) PORT_PLAYER( 2 )

	PORT_START("BUTTONS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER( 2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

// Wedding Rhapsody

static INPUT_PORTS_START( weddingr )
	PORT_INCLUDE( konamigv )

	// Control Type must match selection in service mode (no sense line to detect control panel type)
	// Buttons 2 and 3 are shown in service mode, but not used by the game
	// Button 1-3 inputs are read in service mode even when 4 Buttons is selected, but they could confuse users

	PORT_START("CFG")
	PORT_CONFNAME( 0x01, 0x01, "Control Type" )
	PORT_CONFSETTING(    0x01, "4 Buttons" )
	PORT_CONFSETTING(    0x00, "Joystick and Button" )

	PORT_MODIFY("P1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Answer 3/Zoom In")   PORT_CONDITION("CFG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Answer 4/Zoom Out")  PORT_CONDITION("CFG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Answer 1/Pan Left")  PORT_CONDITION("CFG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Answer 2/Pan Right") PORT_CONDITION("CFG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x00000070, IP_ACTIVE_LOW, IPT_UNKNOWN )                                                   PORT_CONDITION("CFG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(1)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(1)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 )                 PORT_PLAYER(1)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 )                 PORT_PLAYER(1)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 )                 PORT_PLAYER(1)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)

	PORT_MODIFY("P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Answer 3/Zoom In")   PORT_CONDITION("CFG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Answer 4/Zoom Out")  PORT_CONDITION("CFG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Answer 1/Pan Left")  PORT_CONDITION("CFG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Answer 2/Pan Right") PORT_CONDITION("CFG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x00000070, IP_ACTIVE_LOW, IPT_UNKNOWN )                                                   PORT_CONDITION("CFG", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_8WAY PORT_PLAYER(2)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_8WAY PORT_PLAYER(2)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 )                 PORT_PLAYER(2)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 )                 PORT_PLAYER(2)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 )                 PORT_PLAYER(2)                    PORT_CONDITION("CFG", 0x01, EQUALS, 0x00)

	PORT_MODIFY("P3_P4")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

#define GV_BIOS \
	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) \
	ROM_LOAD( "999a01.7e",   0x0000000, 0x080000, CRC(ad498d2d) SHA1(02a82a2fe1fba0404517c3602324bfa64e23e478) )

ROM_START( konamigv )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", ROMREGION_ERASE00 ) // default EEPROM
ROM_END

ROM_START( lacrazyc )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "lacrazyc.25c",   0x000000, 0x000080, CRC(e20e5730) SHA1(066b49236c658a4ef2930f7bacc4b2354dd7f240) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "gv027-a1", 0, BAD_DUMP SHA1(840d0d4876cf1b814c9d8db975aa6c92e1fe4039) )
ROM_END

ROM_START( susume )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "susume.25c",   0x000000, 0x000080, CRC(52f17df7) SHA1(b8ad7787b0692713439d7d9bebfa0c801c806006) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "gv027j1", 0, BAD_DUMP SHA1(e7e6749ac65de7771eb8fed7d5eefaec3f902255) )
ROM_END

ROM_START( hyperath )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "hyperath.25c", 0x000000, 0x000080, CRC(20a8c435) SHA1(a0f203a999757fba68b391c525ac4b9684a57ba9) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "gv021-j1", 0, SHA1(579442444025b18da658cd6455c51459fbc3de0e) )
ROM_END

ROM_START( powyak96 )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "powyak96.25c", 0x000000, 0x000080, CRC(405a7fc9) SHA1(e2d978f49748ba3c4a425188abcd3d272ec23907) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "powyak96", 0, BAD_DUMP SHA1(ebd0ea18ff9ce300ea1e30d66a739a96acfb0621) )
ROM_END

ROM_START( weddingr )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "weddingr.25c", 0x000000, 0x000080, CRC(b90509a0) SHA1(41510a0ceded81dcb26a70eba97636d38d3742c3) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "weddingr", 0, BAD_DUMP SHA1(4e7122b191747ab7220fe4ce1b4483d62ab579af) )
ROM_END

ROM_START( simpbowl )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "simpbowl.25c", 0x000000, 0x000080, CRC(2c61050c) SHA1(16ae7f81cbe841c429c5c7326cf83e87db1782bf) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "829uaa02", 0, SHA1(2ec4cc608d5582e478ee047b60ccee67b52f060c) )
ROM_END

ROM_START( btchamp )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "btchmp.25c", 0x000000, 0x000080, CRC(6d02ea54) SHA1(d3babf481fd89db3aec17f589d0d3d999a2aa6e1) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "btchamp", 0, BAD_DUMP SHA1(c9c858e9034826e1a12c3c003dd068a49a3577e1) )
ROM_END

ROM_START( kdeadeye )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "kdeadeye.25c", 0x000000, 0x000080, CRC(3935d2df) SHA1(cbb855c475269077803c380dbc3621e522efe51e) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "kdeadeye", 0, BAD_DUMP SHA1(3c737c51717925be724dcb93d30769649029b8ce) )
ROM_END

ROM_START( nagano98 )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "nagano98.25c",  0x000000, 0x000080, CRC(b64b7451) SHA1(a77a37e0cc580934d1e7e05d523bae0acd2c1480) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "nagano98", 0, BAD_DUMP SHA1(1be7bd4531f249ff2233dd40a206c8d60054a8c6) )
ROM_END

ROM_START( naganoj )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "720ja.25c",  0x000000, 0x000080, CRC(34c473ba) SHA1(768225b04a293bdbc114a092d14dee28d52044e9) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "720jaa01", 0, SHA1(437160996551ef4dfca43899d1d14beca62eb4c9) )
ROM_END

ROM_START( tmosh )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "tmosh.25c", 0x000000, 0x000080, NO_DUMP )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "673jaa01", 0, SHA1(eaa76073749f9db48c1bee3dff9bea955683c8a8) )
ROM_END

ROM_START( tmoshs )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "tmoshs.25c", 0x000000, 0x000080, CRC(e57b833f) SHA1(f18a0974a6be69dc179706643aab837ff61c2738) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "755jaa01", 0, SHA1(fc742a0b763ba38350ba7eb5d775948632aafd9d) )
ROM_END

ROM_START( tmoshsp )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "tmoshsp.25c", 0x000000, 0x000080, CRC(af4cdd87) SHA1(97041e287e4c80066043967450779b81b62b2b8e) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "756jab01", 0, SHA1(b2c59b9801debccbbd986728152f314535c67e53) )
ROM_END

ROM_START( tmoshspa )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "tmoshsp.25c", 0x000000, 0x000080, CRC(af4cdd87) SHA1(97041e287e4c80066043967450779b81b62b2b8e) )

	DISK_REGION( "scsi:" SCSI_PORT_DEVICE1 ":cdrom" )
	DISK_IMAGE_READONLY( "756jaa01", 0, BAD_DUMP SHA1(5e6d349ad1a22c0dbb1ec26aa05febc830254339) ) // The CD was damaged
ROM_END

} // Anonymous namespace


// BIOS placeholder
GAME( 1995, konamigv, 0,        konamigv, konamigv, konamigv_state, empty_init, ROT0, "Konami", "Baby Phoenix/GV System", MACHINE_IS_BIOS_ROOT )

GAME( 1996, powyak96, konamigv, konamigv, konamigv, konamigv_state, empty_init, ROT0, "Konami", "Jikkyou Powerful Pro Yakyuu '96 (GV017 Japan 1.03)", MACHINE_IMPERFECT_SOUND )
GAME( 1996, hyperath, konamigv, konamigv, konamigv, konamigv_state, empty_init, ROT0, "Konami", "Hyper Athlete (GV021 Japan 1.00)", MACHINE_IMPERFECT_SOUND )
GAME( 1996, lacrazyc, konamigv, konamigv, konamigv, konamigv_state, empty_init, ROT0, "Konami", "Let's Attack Crazy Cross (GV027 Asia 1.10)", MACHINE_IMPERFECT_SOUND )
GAME( 1996, susume,   lacrazyc, konamigv, konamigv, konamigv_state, empty_init, ROT0, "Konami", "Susume! Taisen Puzzle-Dama (GV027 Japan 1.20)", MACHINE_IMPERFECT_SOUND )
GAME( 1996, btchamp,  konamigv, btchamp,  btchamp,  konamigv_state, empty_init, ROT0, "Konami", "Beat the Champ (GV053 UAA01)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
GAME( 1996, kdeadeye, konamigv, kdeadeye, kdeadeye, konamigv_state, empty_init, ROT0, "Konami", "Dead Eye (GV054 UAA01)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
GAME( 1997, weddingr, konamigv, konamigv, weddingr, konamigv_state, empty_init, ROT0, "Konami", "Wedding Rhapsody (GX624 JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1997, tmosh,    konamigv, tmosh,    konamigv, konamigv_state, empty_init, ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart (GQ673 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1997, tmoshs,   konamigv, tmosh,    konamigv, konamigv_state, empty_init, ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart Seal Version (GE755 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1997, tmoshsp,  konamigv, tmosh,    konamigv, konamigv_state, empty_init, ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart Seal Version Plus (GE756 JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1997, tmoshspa, tmoshsp,  tmosh,    konamigv, konamigv_state, empty_init, ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart Seal Version Plus (GE756 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1998, nagano98, konamigv, konamigv, konamigv, konamigv_state, empty_init, ROT0, "Konami", "Nagano Winter Olympics '98 (GX720 EAA)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 1998, naganoj,  nagano98, konamigv, konamigv, konamigv_state, empty_init, ROT0, "Konami", "Hyper Olympic in Nagano (GX720 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 2000, simpbowl, konamigv, simpbowl, simpbowl, simpbowl_state, empty_init, ROT0, "Konami", "Simpsons Bowling (GQ829 UAA)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
