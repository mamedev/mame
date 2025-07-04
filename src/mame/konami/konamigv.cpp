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
simpbowl   The Simpsons Bowling                                     GV999          GQ829   UAA          ?

Notes:

The Tokimeki Memorial Oshiete Your Heart games use an extra PCB plugged in on top for controlling the printer and the sensors.
Additionally, there is a small PCB for connecting to a sensor... PCB number GE755-PWB(S)

The Simpsons Bowling uses an extra PCB plugged in on top containing flash ROMs and circuitry to control the trackball.

Some of the other games may also have extra PCBs.

Have seen two different CD-ROM drives:
- Sony CDU-76S (found in Hyper Athlete)
- Toshiba XM-5401B (found in Tokimeki Memorial Oshiete Your Heart)



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
|A   D482445LGW-A70            53CF96-2 |
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
|A                             53CF96-2 |
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

      - The Simpsons Bowling and Dead Eye use a GV999 with a daughtercard containing flash ROMs and CPLDs:
        PWB402610
        Xilinx XC3020A
        Xilinx 1718DPC
        74F244N (2 of these)
        LVT245SS (2 of theses)
        On The Simpsons Bowling, this also has one µPD4701AC and an empty space for a second.

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
        The Simpsons Bowling additional flash ROM & trackball control PCB.
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

       CN10 & CN11 - TCS7927-54 4-pin mini DIN connectors, S-Video in and out for printer
               CN9 - 6 pin connector
               CN8 - 5 pin connector
               CN7 - 7 pin connector, printer communication
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


Notes:
The Tokimeki Memorial Oshiete Your Heart printer appears to be a model based on the Sony UP-1200 which is
a color video printer that takes S-Video as input.
https://www.ykuns-mechanical-club.com/tokimemo.html
https://www2.biglobe.ne.jp/~tell/keihin/tokimeki/tyheart/tyheart.html

***************************************************************************/


#include "emu.h"

#include "bus/nscsi/cd.h"
#include "cpu/psx/psx.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "machine/mb89371.h"
#include "machine/ncr53c90.h"
#include "machine/upd4701.h"
#include "machine/ram.h"
#include "sound/cdda.h"
#include "sound/spu.h"
#include "video/psx.h"

#include "screen.h"
#include "speaker.h"

#include "cdrom.h"
#include "endianness.h"


namespace {

class konamigv_state : public driver_device
{
public:
	konamigv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
		, m_ncr53cf96(*this, "scsi:7:ncr53cf96")
		, m_btc_trackball(*this, "upd%u", 1)
		, m_maincpu(*this, "maincpu")
	{
	}

	void kdeadeye(machine_config &config);
	void btchamp(machine_config &config);
	void konamigv(machine_config &config);

protected:
	void konamigv_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void btc_trackball_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void scsi_dma_read(uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size);
	void scsi_dma_write(uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size);
	void scsi_drq(int state);

	void btchamp_map(address_map &map) ATTR_COLD;
	void kdeadeye_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(scsi_dma_transfer);

	required_device<screen_device> m_screen;

	required_device<ncr53cf96_device> m_ncr53cf96;
	optional_device_array<upd4701_device, 2> m_btc_trackball;

	required_device<cpu_device> m_maincpu;

	uint32_t *m_dma_data_ptr;
	uint32_t m_dma_offset;
	int32_t m_dma_size;
	bool m_dma_is_write;
	bool m_dma_requested;

	emu_timer *m_dma_timer;
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
	virtual void machine_start() override ATTR_COLD;

	uint16_t flash_r(offs_t offset);
	void flash_w(offs_t offset, uint16_t data);

	void simpbowl_map(address_map &map) ATTR_COLD;

	required_device_array<fujitsu_29f016a_device, 4> m_flash8;

	uint32_t m_flash_address = 0;
};

class tokimeki_state : public konamigv_state
{
public:
	tokimeki_state(const machine_config &mconfig, device_type type, const char *tag)
		: konamigv_state(mconfig, type, tag)
		, m_heartbeat(*this, "HEARTBEAT")
		, m_gsr(*this, "GSR")
		, m_printer_meta(*this, "PRINTER_META")
		, m_device_val_start_state(0)
		, m_printer_is_manual_layout(false)
	{
	}

	void tmosh(machine_config &config);

	void tmoshs_init();
	void tmoshsp_init();

	void heartbeat_pulse_w(int state);
	uint16_t tokimeki_serial_r();
	void tokimeki_serial_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	ioport_value tokimeki_device_check_r();
	void tokimeki_device_check_w(int state);

private:
	enum
	{
		// This should actually be A6 sized paper (105mm x 148mm)
		PRINTER_PAGE_WIDTH = 800,
		PRINTER_PAGE_HEIGHT = 600,
	};

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t printer_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void tmosh_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(heartbeat_timer_tick);
	TIMER_CALLBACK_MEMBER(printing_status_timeout);
	TIMER_CALLBACK_MEMBER(printer_scan_video_frame);

	required_ioport m_heartbeat;
	required_ioport m_gsr;
	required_ioport m_printer_meta;

	uint16_t m_device_val_start_state;
	uint16_t m_device_val;
	uint8_t m_serial_val;
	uint8_t m_serial_len;
	uint8_t m_serial_clk;
	uint8_t m_serial_sensor_id;
	uint16_t m_serial_sensor_data;
	uint8_t m_heartbeat_signal;

	uint8_t m_printer_bit;
	uint8_t m_printer_data[6];
	uint8_t m_printer_curbit;
	uint8_t m_printer_curbyte;
	attotime m_printer_pulse_starttime;
	uint8_t m_printer_is_printing;
	uint32_t m_printer_current_image;
	bool m_printer_is_manual_layout;
	bool m_printer_page_is_dirty;
	uint8_t m_printer_video_last_vblank_state;

	bitmap_rgb32 m_page_bitmap;

	emu_timer *m_heartbeat_timer;
	emu_timer *m_printer_printing_status_timeout;
	emu_timer *m_printer_video_frame_timer;
};

void konamigv_state::konamigv_map(address_map &map)
{
	map(0x1f000000, 0x1f00001f).m(m_ncr53cf96, FUNC(ncr53cf96_device::map)).umask16(0x00ff);
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

void tokimeki_state::tmosh_map(address_map &map)
{
	konamigv_map(map);

	map(0x1f680080, 0x1f680081).r(FUNC(tokimeki_state::tokimeki_serial_r));
	map(0x1f680090, 0x1f680091).w(FUNC(tokimeki_state::tokimeki_serial_w));
}

// SCSI

void konamigv_state::scsi_dma_read(uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size)
{
	m_dma_data_ptr = p_n_psxram;
	m_dma_offset = n_address;
	m_dma_size = n_size * 4;
	m_dma_is_write = false;
	m_dma_timer->adjust(attotime::zero);
}

void konamigv_state::scsi_dma_write(uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size)
{
	m_dma_data_ptr = p_n_psxram;
	m_dma_offset = n_address;
	m_dma_size = n_size * 4;
	m_dma_is_write = true;
	m_dma_timer->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(konamigv_state::scsi_dma_transfer)
{
	// TODO: Figure out proper DMA timings
	while (m_dma_requested && m_dma_data_ptr != nullptr && m_dma_size > 0)
	{
		if (m_dma_is_write)
			m_ncr53cf96->dma_w(util::little_endian_cast<const uint8_t>(m_dma_data_ptr)[m_dma_offset]);
		else
			util::little_endian_cast<uint8_t>(m_dma_data_ptr)[m_dma_offset] = m_ncr53cf96->dma_r();

		m_dma_offset++;
		m_dma_size--;
	}
}

void konamigv_state::scsi_drq(int state)
{
	if (!m_dma_requested && state)
		m_dma_timer->adjust(attotime::zero);

	m_dma_requested = state;
}

void konamigv_state::machine_start()
{
	m_dma_timer = timer_alloc(FUNC(konamigv_state::scsi_dma_transfer), this);
}

void konamigv_state::machine_reset()
{
	m_dma_timer->adjust(attotime::never);
	m_dma_data_ptr = nullptr;
	m_dma_offset = 0;
	m_dma_size = 0;
	m_dma_requested = m_dma_is_write = false;
}

void simpbowl_state::machine_start()
{
	konamigv_state::machine_start();
	save_item(NAME(m_flash_address));
}

void tokimeki_state::machine_start()
{
	konamigv_state::machine_start();
	save_item(NAME(m_device_val));
	save_item(NAME(m_serial_val));
	save_item(NAME(m_serial_len));
	save_item(NAME(m_serial_clk));
	save_item(NAME(m_serial_sensor_id));
	save_item(NAME(m_serial_sensor_data));
	save_item(NAME(m_heartbeat_signal));

	save_item(NAME(m_printer_bit));
	save_item(NAME(m_printer_curbit));
	save_item(NAME(m_printer_curbyte));
	save_item(NAME(m_printer_is_printing));
	save_item(NAME(m_printer_current_image));
	save_item(NAME(m_printer_data));
	save_item(NAME(m_printer_page_is_dirty));
	save_item(NAME(m_printer_video_last_vblank_state));
	save_item(NAME(m_page_bitmap));

	m_heartbeat_timer = timer_alloc(FUNC(tokimeki_state::heartbeat_timer_tick), this);
	m_heartbeat_timer->adjust(attotime::zero);

	m_printer_printing_status_timeout = timer_alloc(FUNC(tokimeki_state::printing_status_timeout), this);
	m_printer_video_frame_timer = timer_alloc(FUNC(tokimeki_state::printer_scan_video_frame), this);

	m_page_bitmap.allocate(PRINTER_PAGE_WIDTH, PRINTER_PAGE_HEIGHT);
}

void tokimeki_state::machine_reset()
{
	konamigv_state::machine_reset();
	m_device_val = m_device_val_start_state;
	m_serial_val = 0;
	m_serial_len = 0;
	m_serial_clk = 0;
	m_serial_sensor_id = 0;
	m_serial_sensor_data = 0;
	m_heartbeat_signal = 1;

	m_printer_bit = 0;
	m_printer_curbit = 0;
	m_printer_curbyte = 0;
	m_printer_is_printing = 0;
	m_printer_current_image = 0;
	m_printer_video_last_vblank_state = 0;

	std::fill(std::begin(m_printer_data), std::end(m_printer_data), 0);

	m_page_bitmap.fill(0xffffffff);
	m_printer_page_is_dirty = true;
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

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:4").option_set("cdrom", NSCSI_XM5401).machine_config(
			[](device_t *device)
			{
				device->subdevice<cdda_device>("cdda")->add_route(0, "^^speaker", 1.0, 0);
				device->subdevice<cdda_device>("cdda")->add_route(1, "^^speaker", 1.0, 1);
			});
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr53cf96", NCR53CF96).clock(32_MHz_XTAL/2).machine_config(
			[this](device_t *device)
			{
				ncr53cf96_device &adapter = downcast<ncr53cf96_device &>(*device);
				adapter.irq_handler_cb().set(":maincpu:irq", FUNC(psxirq_device::intin10));
				adapter.drq_handler_cb().set(*this, FUNC(konamigv_state::scsi_drq));
			});

	// video hardware
	CXD8514Q(config, "gpu", XTAL(53'693'175), 0x100000, subdevice<psxcpu_device>("maincpu")).set_screen("screen");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	spu_device &spu(SPU(config, "spu", XTAL(67'737'600)/2, subdevice<psxcpu_device>("maincpu")));
	spu.add_route(1, "speaker", 0.75, 0); // to verify the channels, btchamp's "game sound test" in the sound test menu speaks the words left, right, center
	spu.add_route(0, "speaker", 0.75, 1);
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
	PORT_BIT( 0x00002000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
INPUT_PORTS_END

// The Simpsons Bowling

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

// Tokimeki Memorial games - has a heart rate sensor, GSR (galvanic skin response) sensor, and printer

TIMER_CALLBACK_MEMBER(tokimeki_state::heartbeat_timer_tick)
{
	const auto heartrate = m_heartbeat->read() & 0xff;

	if (heartrate > 0)
	{
		m_heartbeat_timer->adjust(attotime::from_msec(60'000 / (heartrate + 49)));
		m_heartbeat_signal = 0;
	}
	else
	{
		// hand not on sensor, wait 100 ms for the heartbeat value to change again
		m_heartbeat_timer->adjust(attotime::from_msec(100));
	}
}

TIMER_CALLBACK_MEMBER(tokimeki_state::printing_status_timeout)
{
	m_printer_is_printing = 0;
}

TIMER_CALLBACK_MEMBER(tokimeki_state::printer_scan_video_frame)
{
	// only accept video frame when it's the start of the next vblank area
	const int vblank = m_screen->vblank();
	const bool is_next_vblank = vblank != m_printer_video_last_vblank_state && vblank != 0;

	m_printer_video_last_vblank_state = vblank;

	if (!is_next_vblank)
	{
		m_printer_video_frame_timer->adjust(attotime::from_nsec(500));
		return;
	}

	if (m_printer_is_manual_layout)
	{
		// 4x4 image layout
		// The game tells the printer to take a video still of each individual
		// image that'll be printed on the sticker sheet instead of one only
		// one picture for the entire sheet like the non-plus version.

		// HACK: crop out some unwanted padding and garbage from the input image.
		// The left side of image gets cropped off due to PSX gpu rendering issues.
		// The actual image itself uses 4x4 pixels except for a few pixels
		// around the edge of the image, so those few pixels are also cropped
		// to allow for clean scaling without chunky pixels.
		const int32_t crop_left = 1;
		const int32_t crop_top = 40;
		const int32_t crop_right = 19;
		const int32_t crop_bottom = 16;

		bitmap_rgb32 cropped(
			m_screen->curbitmap().as_rgb32(),
			{
				crop_left,
				m_screen->cliprect().max_x - crop_right,
				crop_top,
				m_screen->cliprect().max_y - crop_bottom,
			}
		);

		// scale the individual screenshot down to roughly the right size
		const int32_t inner_pad_x = 6, inner_pad_y = 5;
		const int32_t width_margin = 163, height_margin = 161; // full size of padding on both sides
		const int32_t dest_w = (PRINTER_PAGE_WIDTH - width_margin - (inner_pad_x * 3)) / 4;
		const int32_t dest_h = (PRINTER_PAGE_HEIGHT - height_margin - (inner_pad_y * 3)) / 4;
		const int32_t scale_w = (cropped.width() << 16) / dest_w;
		const int32_t scale_h = (cropped.height() << 16) / dest_h;

		bitmap_rgb32 scaled(dest_w, dest_h);

		copyrozbitmap(
			scaled,
			scaled.cliprect(),
			cropped,
			0, 0,
			scale_w, 0, 0, scale_h,
			false
		);

		// render the cropped and scaled image to the printer page
		const int x = (m_printer_current_image % 4) * (scaled.width() + inner_pad_x);
		const int y = (m_printer_current_image / 4) * (scaled.height() + inner_pad_y);
		copybitmap(m_page_bitmap, scaled, 0, 0, width_margin / 2 + x, height_margin / 2 + y, m_page_bitmap.cliprect());

		m_printer_current_image++;
	}
	else
	{
		// center entire screen onto printer page
		m_page_bitmap.fill(0xffffffff);

		const int x = (PRINTER_PAGE_WIDTH - m_screen->width()) / 2;
		const int y = (PRINTER_PAGE_HEIGHT - m_screen->height()) / 2;
		copybitmap(m_page_bitmap, m_screen->curbitmap().as_rgb32(), 0, 0, x, y, m_page_bitmap.cliprect());
	}
}

void tokimeki_state::heartbeat_pulse_w(int state)
{
	if (state)
		m_heartbeat_signal = 0;
}

uint16_t tokimeki_state::tokimeki_serial_r()
{
	uint16_t r = m_heartbeat_signal << 2;
	m_heartbeat_signal = 1;

	if (m_serial_sensor_id != 0)
		r |= BIT(m_serial_sensor_data, 8) << 3;

	const auto printer_meta = m_printer_meta->read();
	if (!BIT(printer_meta, 0))
		r |= 0x20; // no paper loaded

	if (!BIT(printer_meta, 1))
		r |= 0x80; // printer is malfunctioning

	r |= m_printer_is_printing << 6;

	return r;
}

void tokimeki_state::tokimeki_serial_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	    0x20 = serial clock
	    0x10 = serial data
	    0x02 = sensors dest

	    0x80 = printer s-video in display flag? only shown during printer config menus
	    0x01 = printer data. the duration of the value being held 1 is used to determine the command or value
	*/

	const int printer_bit = BIT(data, 0);
	if (printer_bit && !m_printer_bit)
	{
		m_printer_pulse_starttime = machine().time();
	}
	else if (!printer_bit && m_printer_bit)
	{
		/*
		duration of the transition from 1->0->1 is used to figure out what is being sent/
		the durations are clocked against the PS1's clock via GetRCnt(0xf2000002)
		internally the values compared are 2400, 4800, and 9600 cycles
		9600 = start of new data transfer
		4800 = data bit 1
		2400 = data bit 0
		*/
		const auto curtime = machine().time();
		const auto ticks = (curtime - m_printer_pulse_starttime).as_ticks(m_maincpu->clock() / 16);

		if (ticks >= 9000)
		{
			// start of data transfer
			m_printer_curbit = 0;
		}
		else
		{
			// data bit transfer
			const int bit = ticks >= 4000 && ticks <= 5600;

			if (m_printer_curbit == 0)
				m_printer_data[m_printer_curbyte] = 0;

			m_printer_data[m_printer_curbyte] |= bit << m_printer_curbit;
			m_printer_curbit++;

			// game is programmed to always sends two bytes. first byte is 7 bits, second byte is hardcoded to be 0xf9
			if ((m_printer_curbyte & 1) == 0 && m_printer_curbit == 7)
			{
				m_printer_curbyte++;
				m_printer_curbit = 0;
			}
			else if ((m_printer_curbyte & 1) == 1 && m_printer_curbit == 8)
			{
				if (m_printer_data[m_printer_curbyte] != 0xf9)
					logerror("tokimeki printer 2nd byte was expected to be f9, found %02x", m_printer_data[m_printer_curbyte]);

				m_printer_curbyte++;
				m_printer_curbit = 0;
			}
		}

		if (m_printer_curbyte >= 6)
		{
			/*
			All of the commands seem to correspond to physical buttons available on the machine.
			The manual configuration option in the printer test menu appears to let you control
			the printer's OSD via the cabinet controls and you can see it sending 64/65/66/67
			depending on which direction you press on the joystick.

			0x0b = sent before 0x63 -> 0x62 sequence when holding button to exit manual configuration menu. stop?
			0x10 = memory in
			0x11 = print
			0x17 = source/memory?
			0x62 = menu
			0x63 = exec
			0x64 = up
			0x65 = down
			0x66 = left
			0x67 = right
			*/

			// The same sequence of 2 bytes will be sent 3 times every time it is sent, so only accept the value if it was repeated 3 times
			if (m_printer_data[0] != m_printer_data[2] || m_printer_data[0] != m_printer_data[4] || m_printer_data[1] != m_printer_data[3] || m_printer_data[1] != m_printer_data[5])
			{
				logerror("printer command not accepted, found different bytes [%02x %02x %02x] [%02x %02x %02x]", m_printer_data[0], m_printer_data[2], m_printer_data[4], m_printer_data[1], m_printer_data[3], m_printer_data[5]);
			}
			else if (m_printer_data[1] != 0xf9)
			{
				logerror("printer command not accepted, found second byte that wasn't 0xf9 [%02x %02x %02x] [%02x %02x %02x]", m_printer_data[0], m_printer_data[2], m_printer_data[4], m_printer_data[1], m_printer_data[3], m_printer_data[5]);
			}
			else if (m_printer_data[0] == 0x10)
			{
				// memory in
				m_printer_video_last_vblank_state = m_screen->vblank();
				m_printer_video_frame_timer->adjust(m_screen->time_until_vblank_start());
			}
			else if (m_printer_data[0] == 0x11)
			{
				// print
				// tmoshs expects the busy status bit to be set for a little bit after this command or it errors out
				m_printer_is_printing = 1;
				m_printer_page_is_dirty = true;
				m_printer_printing_status_timeout->adjust(attotime::from_msec(1000));
			}
			else if (m_printer_data[0] == 0x17)
			{
				// source/memory?
				m_page_bitmap.fill(0xffffffff);
				m_printer_current_image = 0;
			}
			else
			{
				logerror("tokimeki printer found unknown command %02x\n", m_printer_data[0]);
			}

			m_printer_curbyte = 0;
			m_printer_curbit = 0;
		}
	}

	m_printer_bit = printer_bit;

	const int serial_clk = BIT(data, 5);
	if (BIT(data, 1))
	{
		m_serial_sensor_data = 0;
		m_serial_sensor_id = 0;
		m_serial_val = 0;
		m_serial_len = 0;
	}
	else if (!m_serial_clk && serial_clk)
	{
		if (m_serial_len < 5)
		{
			// Sends 5 bits of data for the sensor ID
			m_serial_val |= BIT(data, 4) << (4 - m_serial_len);

			if (m_serial_len == 4)
			{
				m_serial_sensor_id = m_serial_val;

				switch (m_serial_sensor_id)
				{
					case 0x1a: // GSR sensor value
						m_serial_sensor_data = m_gsr->read();
						break;
					case 0x18:
					case 0x19:
					default:
						m_serial_sensor_data = 0;
						break;
				}
			}
		}
		else if (m_serial_len >= 6)
		{
			// Shifts data between reads of tokimeki_serial_r
			m_serial_sensor_data <<= 1;
		}

		m_serial_len++;
	}

	m_serial_clk = serial_clk;
}

void tokimeki_state::tmosh(machine_config &config)
{
	konamigv(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &tokimeki_state::tmosh_map);

	auto &screen(SCREEN(config, "printer", SCREEN_TYPE_RASTER));
	screen.set_size(PRINTER_PAGE_WIDTH, PRINTER_PAGE_HEIGHT);
	screen.set_visarea_full();
	screen.set_refresh_hz(10); // infrequently updated, only displays printed page
	screen.set_screen_update(FUNC(tokimeki_state::printer_screen_update));
}

uint32_t tokimeki_state::printer_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (!m_printer_page_is_dirty)
		return UPDATE_HAS_NOT_CHANGED;

	copybitmap(bitmap, m_page_bitmap, 0, 0, 0, 0, cliprect);
	m_printer_page_is_dirty = false;

	return 0;
}

void tokimeki_state::tmoshs_init()
{
	m_device_val_start_state = 0xf073;
}

void tokimeki_state::tmoshsp_init()
{
	m_device_val_start_state = 0xf0ba;
	m_printer_is_manual_layout = true;
}

ioport_value tokimeki_state::tokimeki_device_check_r()
{
	return BIT(m_device_val, 15);
}

void tokimeki_state::tokimeki_device_check_w(int state)
{
	// The check is accepted when it reads whatever is specified in m_device_val_start_state
	if (state)
		m_device_val = (m_device_val << 1) | BIT(m_device_val, 15);
}

static INPUT_PORTS_START( tmosh )
	PORT_INCLUDE( konamigv )

	PORT_MODIFY("P1")
	PORT_BIT( 0xffffc3e0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0xfffffbff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(tokimeki_state::tokimeki_device_check_r))

	PORT_MODIFY("P3_P4")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("EEPROMOUT")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(tokimeki_state::tokimeki_device_check_w))

	// valid range for heart rate is 50-150, anything outside of that range makes the game show ? in the heart rate meter area
	// Setting the value here to 0 will act as if the player's hand is off the sensor, and anything after that acts as 50-100
	// Default heartbeat is configured for 80 beats per minute
	PORT_START("HEARTBEAT")
	PORT_BIT( 0x0ff, 31,             IPT_PADDLE_V ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_MINMAX(0, 101) PORT_NAME("Heart Rate") PORT_CONDITION("CONTROLS", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x0ff,  0,             IPT_CUSTOM ) PORT_CONDITION("CONTROLS", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x100, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_WRITE_LINE_MEMBER(FUNC(tokimeki_state::heartbeat_pulse_w)) PORT_NAME("Heartbeat Pulse") PORT_CONDITION("CONTROLS", 0x01, EQUALS, 0x00)

	// value read during calibration is treated as zero
	// scale in operator menu goes from 0x00-0xff but only 0x00-0x80 is actually usable in-game
	PORT_START("GSR")
	PORT_BIT( 0x0ff, 0x00, IPT_POSITIONAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(2) PORT_CENTERDELTA(0) PORT_MINMAX(0x00, 0x80) PORT_NAME("GSR Sensor")

	PORT_START("PRINTER_META")
	PORT_CONFNAME( 0x01, 0x01, "Printer Paper Empty" )
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x02, "Printer Malfunction" )
	PORT_CONFSETTING(    0x02, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )

	PORT_START("CONTROLS")
	PORT_CONFNAME( 0x01, 0x01, "Heartbeat Input" )
	PORT_CONFSETTING(    0x00, "Direct" )
	PORT_CONFSETTING(    0x01, "Simulated" )
INPUT_PORTS_END

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

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "gv027-a1", 0, BAD_DUMP SHA1(840d0d4876cf1b814c9d8db975aa6c92e1fe4039) )
ROM_END

ROM_START( susume )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "susume.25c",   0x000000, 0x000080, CRC(52f17df7) SHA1(b8ad7787b0692713439d7d9bebfa0c801c806006) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "gv027j1", 0, BAD_DUMP SHA1(e7e6749ac65de7771eb8fed7d5eefaec3f902255) )
ROM_END

ROM_START( hyperath )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "hyperath.25c", 0x000000, 0x000080, CRC(20a8c435) SHA1(a0f203a999757fba68b391c525ac4b9684a57ba9) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "gv021-j1", 0, SHA1(579442444025b18da658cd6455c51459fbc3de0e) )
ROM_END

ROM_START( powyak96 )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "powyak96.25c", 0x000000, 0x000080, CRC(405a7fc9) SHA1(e2d978f49748ba3c4a425188abcd3d272ec23907) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "powyak96", 0, BAD_DUMP SHA1(ebd0ea18ff9ce300ea1e30d66a739a96acfb0621) )
ROM_END

ROM_START( weddingr )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "weddingr.25c", 0x000000, 0x000080, CRC(b90509a0) SHA1(41510a0ceded81dcb26a70eba97636d38d3742c3) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "weddingr", 0, BAD_DUMP SHA1(4e7122b191747ab7220fe4ce1b4483d62ab579af) )
ROM_END

ROM_START( simpbowl )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "simpbowl.25c", 0x000000, 0x000080, CRC(2c61050c) SHA1(16ae7f81cbe841c429c5c7326cf83e87db1782bf) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "829uaa02", 0, SHA1(2ec4cc608d5582e478ee047b60ccee67b52f060c) )
ROM_END

ROM_START( btchamp )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "btchmp.25c", 0x000000, 0x000080, CRC(6d02ea54) SHA1(d3babf481fd89db3aec17f589d0d3d999a2aa6e1) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "btchamp", 0, BAD_DUMP SHA1(c9c858e9034826e1a12c3c003dd068a49a3577e1) )
ROM_END

ROM_START( kdeadeye )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "kdeadeye.25c", 0x000000, 0x000080, CRC(3935d2df) SHA1(cbb855c475269077803c380dbc3621e522efe51e) )

	// constructed from six reads of the same disc using two drives, 80 sectors have Q subcode CRC errors
	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "054uaa01", 0, SHA1(a05079e4e5024ca66b7f6b81de74695d86c62dd8) )
ROM_END

ROM_START( nagano98 )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "nagano98.25c",  0x000000, 0x000080, CRC(b64b7451) SHA1(a77a37e0cc580934d1e7e05d523bae0acd2c1480) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "nagano98", 0, BAD_DUMP SHA1(1be7bd4531f249ff2233dd40a206c8d60054a8c6) )
ROM_END

ROM_START( naganoj )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "720ja.25c",  0x000000, 0x000080, CRC(34c473ba) SHA1(768225b04a293bdbc114a092d14dee28d52044e9) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "720jaa01", 0, SHA1(437160996551ef4dfca43899d1d14beca62eb4c9) )
ROM_END

ROM_START( tmosh )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "tmosh.25c", 0x000000, 0x000080, BAD_DUMP CRC(2f6a27fc) SHA1(4ead9313f07e9bf7aa0272dba59db6b21510e00b) ) // hand crafted

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "673jaa01", 0, SHA1(eaa76073749f9db48c1bee3dff9bea955683c8a8) )
ROM_END

ROM_START( tmoshs )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "tmoshs.25c", 0x000000, 0x000080, CRC(e57b833f) SHA1(f18a0974a6be69dc179706643aab837ff61c2738) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "755jaa01", 0, SHA1(fc742a0b763ba38350ba7eb5d775948632aafd9d) )
ROM_END

ROM_START( tmoshsp )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "tmoshsp.25c", 0x000000, 0x000080, CRC(af4cdd87) SHA1(97041e287e4c80066043967450779b81b62b2b8e) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "756jab01", 0, SHA1(b2c59b9801debccbbd986728152f314535c67e53) )
ROM_END

ROM_START( tmoshspa )
	GV_BIOS

	ROM_REGION16_BE( 0x0000080, "eeprom", 0 ) // default EEPROM
	ROM_LOAD( "tmoshsp.25c", 0x000000, 0x000080, CRC(af4cdd87) SHA1(97041e287e4c80066043967450779b81b62b2b8e) )

	DISK_REGION( "scsi:4:cdrom" )
	DISK_IMAGE_READONLY( "756jaa01", 0, BAD_DUMP SHA1(5e6d349ad1a22c0dbb1ec26aa05febc830254339) ) // The CD was damaged
ROM_END

} // Anonymous namespace


// BIOS placeholder
GAME( 1995, konamigv, 0,        konamigv, konamigv, konamigv_state, empty_init, ROT0, "Konami", "Baby Phoenix/GV System", MACHINE_IS_BIOS_ROOT )

GAME( 1996, powyak96, konamigv, konamigv, konamigv, konamigv_state, empty_init,   ROT0, "Konami", "Jikkyou Powerful Pro Yakyuu '96 (GV017 Japan 1.03)", MACHINE_IMPERFECT_SOUND )
GAME( 1996, hyperath, konamigv, konamigv, konamigv, konamigv_state, empty_init,   ROT0, "Konami", "Hyper Athlete (GV021 Japan 1.00)", MACHINE_IMPERFECT_SOUND )
GAME( 1996, lacrazyc, konamigv, konamigv, konamigv, konamigv_state, empty_init,   ROT0, "Konami", "Let's Attack Crazy Cross (GV027 Asia 1.10)", MACHINE_IMPERFECT_SOUND )
GAME( 1996, susume,   lacrazyc, konamigv, konamigv, konamigv_state, empty_init,   ROT0, "Konami", "Susume! Taisen Puzzle-Dama (GV027 Japan 1.20)", MACHINE_IMPERFECT_SOUND )
GAME( 1996, btchamp,  konamigv, btchamp,  btchamp,  konamigv_state, empty_init,   ROT0, "Konami", "Beat the Champ (GV053 UAA01)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
GAME( 1996, kdeadeye, konamigv, kdeadeye, kdeadeye, konamigv_state, empty_init,   ROT0, "Konami", "Dead Eye (GV054 UAA01)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
GAME( 1997, weddingr, konamigv, konamigv, weddingr, konamigv_state, empty_init,   ROT0, "Konami", "Wedding Rhapsody (GX624 JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 1997, tmosh,    konamigv, tmosh,    tmosh,    tokimeki_state, empty_init,   ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart (GQ673 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS ) // graphics: picture to be printed is cut off on the left
GAME( 1997, tmoshs,   konamigv, tmosh,    tmosh,    tokimeki_state, tmoshs_init,  ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart Seal Version (GE755 JAA)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, tmoshsp,  konamigv, tmosh,    tmosh,    tokimeki_state, tmoshsp_init, ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart Seal Version Plus (GE756 JAB)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, tmoshspa, tmoshsp,  tmosh,    tmosh,    tokimeki_state, tmoshsp_init, ROT0, "Konami", "Tokimeki Memorial Oshiete Your Heart Seal Version Plus (GE756 JAA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, nagano98, konamigv, konamigv, konamigv, konamigv_state, empty_init,   ROT0, "Konami", "Nagano Winter Olympics '98 (GX720 EAA)", MACHINE_IMPERFECT_SOUND)
GAME( 1998, naganoj,  nagano98, konamigv, konamigv, konamigv_state, empty_init,   ROT0, "Konami", "Hyper Olympic in Nagano (GX720 JAA)", MACHINE_IMPERFECT_SOUND)
GAME( 2000, simpbowl, konamigv, simpbowl, simpbowl, simpbowl_state, empty_init,   ROT0, "Konami", "The Simpsons Bowling (GQ829 UAA)", MACHINE_IMPERFECT_SOUND)
