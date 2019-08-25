// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************************************************

    P's Attack (c) 2004 Uniana

    driver by Angelo Salese, based off original crystal.cpp by ElSemi

    TODO:
    - Compact Flash hookup;
    - Enables wavetable IRQ;

=============================================================================

P's Attack (c) 2004 Uniana Co., Ltd

+----------54321---654321--654321---------------------------+
|VOL       TICKET  GUN_1P  GUN_2P                 +---------|
|                                                 |         |
+-+                                               |  256MB  |
  |       CC-DAC                                  | Compact |
+-+                                  EMUL*        |  Flash  |
|                                                 |         |
|5          +---+                                 +---------|
|6          |   |                                           |
|P          | R |   25.1750MHz              +--------------+|
|I          | A |                           |     42Pin*   ||
|N          | M |                           +--------------+|
|           |   |                           +--------------+|
|C          +---+       +------------+      |     SYS      ||
|O                      |            |      +--------------+|
|N          +---+       |            |                      |
|N          |   |       |VRenderZERO+|                      |
|E SERVICE  | R |       | MagicEyes  |  +-------+    62256* |
|C          | A |       |            |  |  RAM  |           |
|T TEST     | M |       |            |  +-------+    62256* |
|O          |   |       +------------+                      |
|R RESET    +---+                                           |
|                                   14.31818MHz             |
+-+                                                         |
  |                                EEPROM                   |
+-+                GAL                                 DSW  |
|                                                           |
|  VGA                           PIC               BAT3.6V* |
+-----------------------------------------------------------+

* denotes unpopulated device

RAM are Samsung K4S641632H-TC75
VGA is a standard PC 15 pin VGA connection
DSW is 2 switch dipswitch (switches 3-8 are unpopulated)
PIC is a Microchip PIC16C711-041/P (silkscreened on the PCB as COSTOM)
SYS is a ST M27C160 EPROM (silkscreened on the PCB as SYSTEM_ROM_32M)
GAL is a GAL16V8B (not dumped)
EMUL is an unpopulated 8 pin connector
EEPROM is a 93C86 16K 5.0v Serial EEPROM (2048x8-bit or 1024x16-bit)
CC-DAC is a TDA1311A Stereo Continuous Calibration DAC


 P's Attack non JAMMA standard 56pin Edge Connector Pinout:

                          56pin Edge Connector
          Solder Side            |             Parts Side
------------------------------------------------------------------
             GND             | A | 1 |             GND
             GND             | B | 2 |             GND
             +5              | C | 3 |             +5
             +5              | D | 4 |             +5
       Player 1 Start Lamp   | E | 5 |         Coin Lamp
             +12             | F | 6 |             +12
------------ KEY ------------| G | 7 |------------ KEY -----------
       Player 2 Start Lamp   | H | 8 |        Coin Counter
        L Speaker (-)        | J | 9 |        L Speaker (+)
        R Speaker (-)        | K | 10|        R Speaker (+)
     Video Vertical Sync     | L | 11|
        Video Green          | M | 12|        Video Red
        Video Sync           | N | 13|        Video Blue
        Service Switch       | P | 14|        Video GND
    Video Horizontal Sync    | R | 15|        Test Switch
                             | S | 16|        Coin Switch
       Start Player 2        | T | 17|        Start Player 1
                             | U | 18|
                             | V | 19|
                             | W | 20|
                             | X | 21|
                             | Y | 22|
                             | a | 23|
                             | b | 24|
                             | d | 25|
                             | e | 26|
             GND             | f | 27|             GND
             GND             | g | 28|             GND


TICKET is a 5 pin connector:

  1| LED
  2| GND
  3| OUT
  4| IN
  5| +12v

GUN_xP are 6 pin gun connectors (pins 3-6 match the UNICO sytle guns):

 GUN-1P: Left (Blue) Gun Connector Pinout

  1| GND
  2| Solenoid
  3| Sensor
  4| +5V
  5| Switch (Trigger)
  6| GND

 GUN-2P: Right (Pink) Gun Connector Pinout

  1| GND
  2| Solenoid
  3| Sensor
  4| +5V
  5| Switch (Trigger)
  6| GND

****************************************************************************/

#include "emu.h"
#include "cpu/se3208/se3208.h"
#include "machine/nvram.h"
#include "machine/eepromser.h"
#include "machine/vrender0.h"
#include "sound/vrender0.h"
#include "video/vrender0.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

#define IDLE_LOOP_SPEEDUP

class psattack_state : public driver_device
{
public:
	psattack_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_textureram(*this, "textureram"),
		m_frameram(*this, "frameram"),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc"),
		m_vr0vid(*this, "vr0vid"),
		m_vr0snd(*this, "vr0snd"),
		m_screen(*this, "screen")
	{ }


	void init_psattack();
	void psattack(machine_config &config);

private:

	/* memory pointers */
	required_shared_ptr<uint32_t> m_workram;
	required_shared_ptr<uint32_t> m_textureram;
	required_shared_ptr<uint32_t> m_frameram;

	/* devices */
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
	required_device<vr0video_device> m_vr0vid;
	required_device<vr0sound_device> m_vr0snd;
	required_device<screen_device> m_screen;

#ifdef IDLE_LOOP_SPEEDUP
	uint8_t     m_FlipCntRead;
	DECLARE_WRITE_LINE_MEMBER(idle_skip_resume_w);
	DECLARE_WRITE_LINE_MEMBER(idle_skip_speedup_w);
#endif

	IRQ_CALLBACK_MEMBER(icallback);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	void psattack_mem(address_map &map);
};

uint32_t psattack_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_vr0soc->crt_is_blanked()) // Blank Screen
	{
		bitmap.fill(0, cliprect);
		return 0;
	}

	m_vr0vid->screen_update(screen, bitmap, cliprect);
	return 0;
}

WRITE_LINE_MEMBER(psattack_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		if (m_vr0soc->crt_active_vblank_irq() == true)
			m_vr0soc->IntReq(24);      //VRender0 VBlank

		m_vr0vid->execute_drawing();
	}
}

IRQ_CALLBACK_MEMBER(psattack_state::icallback)
{
	return m_vr0soc->irq_callback();
}

#ifdef IDLE_LOOP_SPEEDUP
WRITE_LINE_MEMBER(psattack_state::idle_skip_resume_w)
{
	m_FlipCntRead = 0;
	m_maincpu->resume(SUSPEND_REASON_SPIN);
}

WRITE_LINE_MEMBER(psattack_state::idle_skip_speedup_w)
{
	m_FlipCntRead++;
	if (m_FlipCntRead >= 16 && m_vr0soc->irq_pending() == false && state == ASSERT_LINE)
		m_maincpu->suspend(SUSPEND_REASON_SPIN, 1);
}
#endif


void psattack_state::psattack_mem(address_map &map)
{
	map(0x00000000, 0x001fffff).rom().nopw();

	//   0x1400c00, 0x1400c01 read cfcard memory (auto increment?)
	//   0x1402800, 0x1402807 read/write regs?
	//   0x1802410, 0x1802413 peripheral chip select for above
	map(0x01500000, 0x01500003).portr("IN0");
	map(0x01500004, 0x01500007).portr("IN1");
	map(0x01500008, 0x0150000b).portr("IN2");
	//0x0150000c is prolly eeprom

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x0300ffff).m(m_vr0vid, FUNC(vr0video_device::regs_map));
	map(0x03800000, 0x03ffffff).ram().share("textureram");
	map(0x04000000, 0x047fffff).ram().share("frameram");
	map(0x04800000, 0x04800fff).rw(m_vr0snd, FUNC(vr0sound_device::vr0_snd_read), FUNC(vr0sound_device::vr0_snd_write));
}

static INPUT_PORTS_START( psattack )
	PORT_START("IN0")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void psattack_state::machine_start()
{
	m_vr0vid->set_areas(reinterpret_cast<uint8_t*>(m_textureram.target()), reinterpret_cast<uint16_t*>(m_frameram.target()));
	m_vr0snd->set_areas(m_textureram, m_frameram);

#ifdef IDLE_LOOP_SPEEDUP
	save_item(NAME(m_FlipCntRead));
#endif
}

void psattack_state::machine_reset()
{
#ifdef IDLE_LOOP_SPEEDUP
	m_FlipCntRead = 0;
#endif
}

void psattack_state::psattack(machine_config &config)
{
	SE3208(config, m_maincpu, 14318180 * 3); // TODO : different between each PCBs
	m_maincpu->set_addrmap(AS_PROGRAM, &psattack_state::psattack_mem);
	m_maincpu->set_irq_acknowledge_callback(FUNC(psattack_state::icallback));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// evolution soccer defaults
	m_screen->set_raw((XTAL(14'318'180)*2)/4, 455, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(psattack_state::screen_update));
	m_screen->screen_vblank().set(FUNC(psattack_state::screen_vblank));
	m_screen->set_palette("palette");

	VRENDER0_SOC(config, m_vr0soc, 0);
	m_vr0soc->set_host_cpu_tag(m_maincpu);
	m_vr0soc->set_host_screen_tag(m_screen);

	VIDEO_VRENDER0(config, m_vr0vid, 14318180, m_maincpu);
	#ifdef IDLE_LOOP_SPEEDUP
	m_vr0soc->idleskip_cb().set(FUNC(psattack_state::idle_skip_resume_w));
	m_vr0vid->idleskip_cb().set(FUNC(psattack_state::idle_skip_speedup_w));
	#endif

	PALETTE(config, "palette", palette_device::RGB_565);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SOUND_VRENDER0(config, m_vr0snd, 0);
	m_vr0snd->add_route(0, "lspeaker", 1.0);
	m_vr0snd->add_route(1, "rspeaker", 1.0);
}

ROM_START( psattack )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD("5.sys",  0x000000, 0x200000, CRC(f09878e4) SHA1(25b8dbac47d3911615c8874746e420ece13e7181) )

	ROM_REGION( 0x4010, "pic16c711", 0 )
	ROM_LOAD("16c711.pic",  0x0000, 0x137b, CRC(617d8292) SHA1(d32d6054ce9db2e31efaf41015afcc78ed32f6aa) ) // raw dump
	ROM_LOAD("16c711.bin",  0x0000, 0x4010, CRC(b316693f) SHA1(eba1f75043bd415268eedfdb95c475e73c14ff86) ) // converted to binary

	DISK_REGION( "cfcard" )
	DISK_IMAGE_READONLY( "psattack", 0, SHA1(e99cd0dafc33ec13bf56061f81dc7c0a181594ee) )
ROM_END

void psattack_state::init_psattack()
{
}

GAME( 2004, psattack, 0,        psattack, psattack,  psattack_state, init_psattack, ROT0, "Uniana",              "P's Attack", MACHINE_IS_SKELETON ) // has a CF card instead of flash roms

