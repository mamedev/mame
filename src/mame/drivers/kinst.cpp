// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Killer Instinct hardware

    driver by Aaron Giles and Bryan McPhail

    Games supported:
        * Killer Instinct
        * Killer Instinct 2

    Known bugs:
        * the SRAM test fails in diagnostics; this is due to the fact that
          the test relies on executing out of the cache while it tromps
          over (and eventually restores) the instructions it is executing;
          this will likely never be fixed

****************************************************************************

Guru-Readme for Killer Instinct 1 / Killer Instinct 2
Rare/Nintendo, 1994/1995

This is a fighting game using a hard drive to hold the graphics + code,
running on hardware made by Midway Manufacturing.
The hardware is using similar PCB technology to other Midway games
of the era such as San Francisco Rush, Mortal Kombat 3, Cruis'n Exotica,
War Gods etc. The majority of the IC's on the PCB are surface mounted.

PCB Layout
----------

KILLER INSTINCT V4.0
5770-14397-03
(C)1994 Nintendo/Rare
(sticker - MIDWAY GAMES 44464 I457034 A-20333)
(sticker from another PCB - MIDWAY GAMES 44447 635095)
|---------------------------------------------------------------|
|J1 J2       LED1 LED2     GAL               U10  U11  U12  U13 |
|TDA7240 TL084  AD1851  10MHz                                   |
|                      |--------|  6116                         |
|                      |ANALOG  |  6116                         |
|                      |DEVICES |  6116                         |
|                      |ADSP2105|            U33  U34  U35  U36 |
|                      |--------|                               |
|J3                                                             |
|-|             71256 71256 71256 71256                         |
  |             71256 71256 71256 71256      MT4C4001  MT4C4001 |
|-|             71256 71256 71256 71256      MT4C4001  MT4C4001 |
| ULN2064B      71256 71256 71256 71256      MT4C4001  MT4C4001 |
|                                            MT4C4001  MT4C4001 |
|J      |----|                               MT4C4001  MT4C4001 |
|A      |EPM |                               MT4C4001  MT4C4001 |
|M      |7096|        |------|               MT4C4001  MT4C4001 |
|M      |----|        |R4600 | 49FCT805      MT4C4001  MT4C4001 |
|A           |----|   |      |                                  |
|            |EPM |   |------|  50MHz   |----|   |----|     JP30|
|-|          |7032|             MAX705  |EPM |   |EPM |         |
  |          |x---|             JP32    |7128|   |7128|   U98   |
|-|DSW1  DSW2                 J6        |x---|   |y---|         |
|       J7       J8           J9    S3                          |
|---------------------|---------------|-------------------------|
                      |     IDE44     |
                      |               | <---- Conversion board to convert KI1 into KI2
                      |    EPM7032y   |       PCB Number: 5772-14668-01
                      |               |       Used only with KI 2 software on KI 1 PCB
                      |     IDE44     |
                      |-|-----------|-|
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                        |||||||||||||
                      |-|||||||||||||-|
                      | |-----------| |
                      |               |
                      |   Seagate     |
                      |   ST9150AG    |
                      |      or       | <---- Can be replaced with a IDE44>CF adapter or IDE44>SD adapter
                      |   ST9420AG    |       if the factory boot ROM is replaced with the "ANY-IDE" boot ROM.
                      |               |       Note there are different "ANY-IDE" boot ROMs for KI and KI 2.
                      | 2.5" H/Drive  |
                      |               |
                      |               |
                      |---------------|
Notes:
      R4600    - Integrated Device Technology IDT79R4600-100MS R4600 CPU running at 100MHz, with heatsink (QFP208)
      GAL      - Lattice GAL20V8 labelled 'KI-U1 A-19802 (C)1994 Nintendo/Rare' (DIP24)
      MT4C4001 - Micron Technology 1M x4-bit Fast Page Mode DRAM (SOJ28)
      71256    - Integrated Device Technology IDT71256SA20Y 32k x8-bit SRAM (SOJ28). Alternative: Mosel MS62256A-20RC 32k x8-bit SRAM (SOJ28)
      6116     - Integrated Device Technology IDT6116SA25SO 2k x8-bit SRAM (SOP24)
      ADSP2105 - Analog Devices ADSP-2105KP-40. Clock input 10.000MHz (PLCC68)
      49FCT805 - Integrated Device Technology IDT49FCT805 Non-Inverting Buffer/Clock Driver with Two Independent Output Banks and Tri-State Control (SOIC20)
      TDA7240  - ST Microelectronics TDA7240A 20W Mono Bridge Audio Amplifier IC (DIL4+3). Note the cabinet has two speakers but the generated sound is mono.
      TL084    - Texas Instruments TL084 General Purpose J-FET Quad Operational Amplifier (DIP14)
      AD1851   - Analog Devices AD1851 Monolithic PCM Audio DAC (DIP16)
      ULN2064B - ST Microelectronics ULN2064B 50V 1.5A Quad Darlington Switch (DIP16)
      MAX705   - Maxim MAX705 Microprocessor Supervisory Circuit / Reset Chip (DIP8)
      J1       - 2 pin position for connector labelled 'LINE OUT' but not populated
      J2       - 2 pin position for connector labelled 'AUX IN' but not populated
      J3       - 10 pin connector for extra controls; coin interlock (pin 6), bill in (pin 5), volume minus (pin 2), volume plus (pin 3)
      J6       - 44 pin connector for 2.5" IDE hard drive
      J7       - 15 pin connector marked 'PLAYER 3' but used for player 1 'low' buttons (pins 7, 8, 9) and player 2 'low' buttons (pins 11, 12, 13)
                 Note the 3 'high' buttons for player 1 and player 2 are on the JAMMA connector. JAMMA pins 25 & 26 top and bottom have no connection.
      J8       - 8 pin connector for coin 3 (pin 2) and coin 4 (pin 6). Probably used for Bill Acceptors etc.
      J9       - Position for 40 pin IDE connector for 3.5" IDE hard drive, but not populated. Note the connector can be added and used with a
                 3.5" IDE HDD but the power for the HDD must be provided separately.
      JP30     - 3 pin jumper to configure Boot ROM. Set to 1-2. Settings are 1-2 = 4MBit (27C4001/27C040) or 2-3 = 8MBit (27C080)
      JP32     - 2 pin jumper to disable Watch Dog (Hard-wired on the PCB shorted 1-2)
      S3       - Reset push-button switch
      LED1     - H/Drive Activity LED
      LED2     - Sound Activity LED
      H/Drive  - 2.5" IDE Hard Drive with 44-pin connector. Note the model is checked by the program and the game will not run unless it finds
                 the correct model. There is a modified Boot ROM available that allows it to work with any model HDD or a cheap Chinese $2 CF>IDE44
                 adapter. A cheap Chinese $5 SD>IDE44 adapter can also be used.
                   - For KI1, Seagate Marathon 2.5" IDE hard drive, model ST9150AG (131MB formatted capacity)
                     C/H/S = 419/13/47 = 131076608 bytes, labelled 'L1 KILLER INSTINCT DISK (C)1984 NINTENDO/RARE'
                   - For KI2, Seagate Marathon 2.5" IDE hard drive, model ST9420AG (420.8MB formatted capacity)
                     C/H/S = 988/16/52 = 420872192 bytes, labelled 'L2.1 KILLER INSTINCT 2 DISK (C)1985 NINTENDO/RARE'
      EPM7096  - Altera EPM7096LC68-10 labelled 'KI-U92 A-19488 (C)1994 NINTENDO/RARE' (PLCC68)
      EPM7128x - Altera MAX EPM7128ELC84-10 CPLD labelled 'KI-U103 A-19486 (C)1994 NINTENDO/RARE' (PLCC84)
      EPM7128y - Altera MAX EPM7128ELC84-10 CPLD labelled 'KI-U104 A-19487 (C)1994 NINTENDO/RARE' (PLCC84)
      EPM7032x - Altera EPM7032LC44-15T CPLD
                   - For KI labelled 'KI-U96 A-19489 (C)1994 NINTENDO/RARE' (PLCC44)
                   - For KI2 labelled 'K12-U96 A-20351 (C)1996 NINTENDO/RARE' (PLCC44). Note if the PCB has this chip it is a dedicated KI2 PCB.
                     If the software is replaced with KI 1 (boot ROM and HDD) the board will boot but immediately jumps to test mode and then freezes.
                     Also, the top title "Killer Instinct" flashes rapidly. If the sticker on U96 is missing (so can't be identified as a KI 2) and
                     someone has tried to make it run KI 1 and it has the above symptoms, it is likely a dedicated KI 2 PCB.
      EPM7032y - Altera EPM7032LC44-15T CPLD labelled 'K12-U1 A-20383 (C)1996 NINTENDO/RARE' (PLCC44)
                 This IC is mounted on a small PCB (5772-14668-01) and connected between the IDE44 interface on the main board and the IDE44 Hard Drive.
                 It is used only with the KI2 conversion kit. This is mainly a protection device to stop game swaps on a KI 1 PCB. However if the
                 sub board is removed and all the EPROMs and HDD are changed, the main board will run Killer Instinct, providing U96 is the correct
                 chip for KI. If U96 is the type for KI2, then the main board will only run KI2 and can't be converted to KI.
      HSync    - 15.3846kHz
      VSync    - 58.9634Hz

      ROMs
      ----

      Killer Instinct:
      U10 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U10 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U11 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U11 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U12 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U12 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U13 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U13 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U33 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U33 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U34 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U34 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U35 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U35 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U36 - ST 27C4001 EPROM labelled 'L1 KILLER INSTINCT U36 MUSIC/SPCH (C) 1994 Nintendo/Rare' (DIP32)
      U98 - ST 27C4001 EPROM labelled 'L1.5D KILLER INSTINCT U98 ROM 1 (C) 1994 Nintendo/Rare' (DIP32)
            Note earlier revisions exist, this is the last revision.

      Killer Instinct 2:
      U10 - ST 27C4001 EPROM labelled 'L1.0 KILLER INSTINCT 2 U10 MUSIC/SPCH (C) 1995 Nintendo/Rare' (DIP32)
      U11 - ST 27C4001 EPROM labelled 'L1.0 KILLER INSTINCT 2 U11 MUSIC/SPCH (C) 1995 Nintendo/Rare' (DIP32)
      U12 - ST 27C4001 EPROM labelled 'L1.0 KILLER INSTINCT 2 U12 MUSIC/SPCH (C) 1995 Nintendo/Rare' (DIP32)
      U13 - ST 27C4001 EPROM labelled 'L1.0 KILLER INSTINCT 2 U13 MUSIC/SPCH (C) 1995 Nintendo/Rare' (DIP32)
      U33 - ST 27C4001 EPROM labelled 'L1.0 KILLER INSTINCT 2 U33 MUSIC/SPCH (C) 1995 Nintendo/Rare' (DIP32)
      U34 - ST 27C4001 EPROM labelled 'L1.0 KILLER INSTINCT 2 U34 MUSIC/SPCH (C) 1995 Nintendo/Rare' (DIP32)
      U35 - ST 27C4001 EPROM labelled 'L1.0 KILLER INSTINCT 2 U35 MUSIC/SPCH (C) 1995 Nintendo/Rare' (DIP32)
      U36 - ST 27C4001 EPROM labelled 'L1.0 KILLER INSTINCT 2 U36 MUSIC/SPCH (C) 1995 Nintendo/Rare' (DIP32)
      U98 - ST 27C4001 EPROM labelled 'L1.4 KILLER INSTINCT 2 U98 (C) 1995 Nintendo/Rare' (DIP32)
            Note earlier revisions exist, this is the last revision.

      BOOT ROMs for IDE HDD to Compact Flash conversion
      -------------------------------------------------
      Killer Instinct:
      U98 - ST 27C4001 or 27C040 EPROM. File name = KI_L15DI.U98, CRC32 = 230F55FB

      Killer Instinct 2 Dedicated PCB (U96 labelled 'K12-U96 A-20351')
      U98 - ST 27C4001 or 27C040 EPROM. File name = KI2_L14P.U98.BIN, CRC32 = D80C937A

      Killer Instinct 2 on Killer Instinct 1 PCB (U96 labelled 'KI-U96 A-19489') with KI 2 protection PCB 5772-14668-01
      U98 - ST 27C4001 or 27C040 EPROM. File name = KI2_D14P.U98, CRC32 = D716D428

***************************************************************************/

#include "emu.h"
#include "audio/dcs.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/idehd.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/mips/mips3.h"
#include "emupal.h"
#include "screen.h"


class kinst_state : public driver_device
{
public:
	kinst_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_rambase(*this, "rambase"),
		m_rambase2(*this, "rambase2"),
		m_control(*this, "control"),
		m_rombase(*this, "rombase"),
		m_maincpu(*this, "maincpu"),
		m_ata(*this, "ata"),
		m_dcs(*this, "dcs"),
		m_palette(*this, "palette")
	{
	}

	void init_kinst();
	void init_kinst2();

	void kinst(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_shared_ptr<uint32_t> m_rambase;
	required_shared_ptr<uint32_t> m_rambase2;
	required_shared_ptr<uint32_t> m_control;
	required_shared_ptr<uint32_t> m_rombase;
	required_device<mips3_device> m_maincpu;
	required_device<ata_interface_device> m_ata;
	required_device<dcs_audio_2k_device> m_dcs;
	required_device<palette_device> m_palette;

	uint32_t *m_video_base;
	const uint8_t *m_control_map;
	emu_timer *m_irq0_stop_timer;

	enum
	{
		TIMER_IRQ0_STOP
	};

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(irq0_start);
	DECLARE_READ32_MEMBER(control_r);
	DECLARE_WRITE32_MEMBER(control_w);
	DECLARE_READ32_MEMBER(ide_r);
	DECLARE_WRITE32_MEMBER(ide_w);
	DECLARE_READ32_MEMBER(ide_extra_r);
	DECLARE_WRITE32_MEMBER(ide_extra_w);

	void main_map(address_map &map);
};



/* constants */
#define MASTER_CLOCK    XTAL(50'000'000)



/*************************************
 *
 *  Machine start
 *
 *************************************/

void kinst_state::machine_start()
{
	/* set the fastest DRC options */
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS);

	/* configure fast RAM regions */
	m_maincpu->add_fastram(0x08000000, 0x087fffff, false, m_rambase2);
	m_maincpu->add_fastram(0x00000000, 0x0007ffff, false, m_rambase);
	m_maincpu->add_fastram(0x1fc00000, 0x1fc7ffff, true,  m_rombase);

	m_irq0_stop_timer = timer_alloc(TIMER_IRQ0_STOP);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

void kinst_state::machine_reset()
{
	ide_hdd_device *hdd = m_ata->subdevice<ata_slot_device>("0")->subdevice<ide_hdd_device>("hdd");
	uint16_t *identify_device = hdd->identify_device_buffer();

	if (strncmp(machine().system().name, "kinst2", 6) != 0)
	{
		/* kinst: tweak the model number so we pass the check */
		identify_device[27] = ('S' << 8) | 'T';
		identify_device[28] = ('9' << 8) | '1';
		identify_device[29] = ('5' << 8) | '0';
		identify_device[30] = ('A' << 8) | 'G';
		identify_device[31] = (' ' << 8) | ' ';
	}
	else
	{
		/* kinst2: tweak the model number so we pass the check */
		identify_device[10] = ('0' << 8) | '0';
		identify_device[11] = ('S' << 8) | 'T';
		identify_device[12] = ('9' << 8) | '1';
		identify_device[13] = ('5' << 8) | '0';
		identify_device[14] = ('A' << 8) | 'G';
	}

	/* set a safe base location for video */
	m_video_base = &m_rambase[0x30000/4];
}



/*************************************
 *
 *  Video refresh
 *
 *************************************/

uint32_t kinst_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *pen = m_palette->pens();

	/* loop over rows and copy to the destination */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t *src = &m_video_base[640/4 * y];
		uint32_t *dest = &bitmap.pix32(y, cliprect.min_x);

		/* loop over columns */
		for (int x = cliprect.min_x; x < cliprect.max_x; x += 2)
		{
			uint32_t data = *src++;

			/* store two pixels */
			*dest++ = pen[(data >>  0) & 0x7fff];
			*dest++ = pen[(data >> 16) & 0x7fff];
		}
	}
	return 0;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

void kinst_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_IRQ0_STOP:
		m_maincpu->set_input_line(0, CLEAR_LINE);
		break;
	default:
		throw emu_fatalerror("Unknown id in kinst_state::device_timer");
	}
}


INTERRUPT_GEN_MEMBER(kinst_state::irq0_start)
{
	device.execute().set_input_line(0, ASSERT_LINE);
	m_irq0_stop_timer->adjust(attotime::from_usec(50));
}


/*************************************
 *
 *  IDE controller access
 *
 *************************************/

READ32_MEMBER(kinst_state::ide_r)
{
	return m_ata->read_cs0(offset / 2, mem_mask);
}


WRITE32_MEMBER(kinst_state::ide_w)
{
	m_ata->write_cs0(offset / 2, data, mem_mask);
}


READ32_MEMBER(kinst_state::ide_extra_r)
{
	return m_ata->read_cs1(6, 0xff);
}


WRITE32_MEMBER(kinst_state::ide_extra_w)
{
	m_ata->write_cs1(6, data, 0xff);
}



/*************************************
 *
 *  Control handling
 *
 *************************************/

READ32_MEMBER(kinst_state::control_r)
{
	uint32_t result;
	static const char *const portnames[] = { "P1", "P2", "VOLUME", "UNUSED", "DSW" };

	/* apply shuffling */
	offset = m_control_map[offset / 2];
	result = m_control[offset];

	switch (offset)
	{
		case 2:     /* $90 -- sound return */
			result = ioport(portnames[offset])->read();
			result &= ~0x0002;
			if (m_dcs->control_r() & 0x800)
				result |= 0x0002;
			break;

		case 0:     /* $80 */
		case 1:     /* $88 */
		case 3:     /* $98 */
			result = ioport(portnames[offset])->read();
			break;

		case 4:     /* $a0 */
			result = ioport(portnames[offset])->read();
			if (m_maincpu->pc() == 0x802d428)
				m_maincpu->spin_until_interrupt();
			break;
	}

	return result;
}


WRITE32_MEMBER(kinst_state::control_w)
{
	/* apply shuffling */
	offset = m_control_map[offset / 2];
	uint32_t olddata = m_control[offset];
	COMBINE_DATA(&m_control[offset]);

	switch (offset)
	{
		case 0:     /* $80 - VRAM buffer control */
			if (data & 4)
				m_video_base = &m_rambase[0x58000/4];
			else
				m_video_base = &m_rambase[0x30000/4];
			break;

		case 1:     /* $88 - sound reset */
			m_dcs->reset_w(~data & 0x01);
			break;

		case 2:     /* $90 - sound control */
			if (!(olddata & 0x02) && (m_control[offset] & 0x02))
				m_dcs->data_w(m_control[3]);
			break;

		case 3:     /* $98 - sound data */
			break;
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void kinst_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0007ffff).ram().share("rambase");
	map(0x08000000, 0x087fffff).ram().share("rambase2");
	map(0x10000080, 0x100000ff).rw(FUNC(kinst_state::control_r), FUNC(kinst_state::control_w)).share("control");
	map(0x10000100, 0x1000013f).rw(FUNC(kinst_state::ide_r), FUNC(kinst_state::ide_w));
	map(0x10000170, 0x10000173).rw(FUNC(kinst_state::ide_extra_r), FUNC(kinst_state::ide_extra_w));
	map(0x1fc00000, 0x1fc7ffff).rom().region("user1", 0).share("rombase");
}




/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( kinst )
	PORT_START("P1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 High Attack - Quick")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 High Attack - Medium")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 High Attack - Fierce")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Low Attack - Quick")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Low Attack - Medium")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Low Attack - Fierce")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x00001000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_CUSTOM )  /* door */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 High Attack - Quick")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 High Attack - Medium")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 High Attack - Fierce")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Low Attack - Quick")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Low Attack - Medium")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Low Attack - Fierce")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BILL1 )    /* bill */
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_CUSTOM )  /* coin door */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("VOLUME")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_CUSTOM )  /* sound status */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0000fff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )   /* verify */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00000003, 0x00000003, "Blood Level" )
	PORT_DIPSETTING(          0x00000003, DEF_STR( High ))
	PORT_DIPSETTING(          0x00000002, DEF_STR( Medium ))
	PORT_DIPSETTING(          0x00000001, DEF_STR( Low ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( None ))
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Demo_Sounds ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000004, DEF_STR( On ))
	PORT_DIPNAME( 0x00000008, 0x00000008, "Finishing Moves" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000008, DEF_STR( On ))
	PORT_DIPNAME( 0x00000010, 0x00000010, "Display Warning" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000010, DEF_STR( On ))
	PORT_DIPNAME( 0x00000020, 0x00000020, "Blood" )
	PORT_DIPSETTING(          0x00000020, "Red" )
	PORT_DIPSETTING(          0x00000000, "White" )
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unused ))
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unused ))
	PORT_DIPSETTING(          0x00000080, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_DIPNAME( 0x00000100, 0x00000100, "Coinage Source" )
	PORT_DIPSETTING(          0x00000100, "Dipswitch" )
	PORT_DIPSETTING(          0x00000000, "Disk" )
	PORT_DIPNAME( 0x00003e00, 0x00003e00, DEF_STR( Coinage ))
	PORT_DIPSETTING(          0x00003e00, "USA-1" )
	PORT_DIPSETTING(          0x00003c00, "USA-2" )
	PORT_DIPSETTING(          0x00003a00, "USA-3" )
	PORT_DIPSETTING(          0x00003800, "USA-4" )
	PORT_DIPSETTING(          0x00003400, "USA-9" )
	PORT_DIPSETTING(          0x00003200, "USA-10" )
	PORT_DIPSETTING(          0x00003600, "USA-ECA" )
	PORT_DIPSETTING(          0x00003000, "USA-Free Play" )
	PORT_DIPSETTING(          0x00002e00, "German-1" )
	PORT_DIPSETTING(          0x00002c00, "German-2" )
	PORT_DIPSETTING(          0x00002a00, "German-3" )
	PORT_DIPSETTING(          0x00002800, "German-4" )
	PORT_DIPSETTING(          0x00002600, "German-ECA" )
	PORT_DIPSETTING(          0x00002000, "German-Free Play" )
	PORT_DIPSETTING(          0x00001e00, "French-1" )
	PORT_DIPSETTING(          0x00001c00, "French-2" )
	PORT_DIPSETTING(          0x00001a00, "French-3" )
	PORT_DIPSETTING(          0x00001800, "French-4" )
	PORT_DIPSETTING(          0x00001600, "French-ECA" )
	PORT_DIPSETTING(          0x00001000, "French-Free Play" )
	PORT_DIPNAME( 0x00004000, 0x00004000, "Coin Counters" )
	PORT_DIPSETTING(          0x00004000, "1" )
	PORT_DIPSETTING(          0x00000000, "2" )
	PORT_DIPNAME( 0x00008000, 0x00008000, "Test Switch" )
	PORT_DIPSETTING(          0x00008000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( kinst2 )
	PORT_START("P1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 High Attack - Quick")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 High Attack - Medium")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 High Attack - Fierce")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Low Attack - Quick")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Low Attack - Medium")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Low Attack - Fierce")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x00001000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_CUSTOM )  /* door */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 High Attack - Quick")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 High Attack - Medium")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 High Attack - Fierce")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Low Attack - Quick")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Low Attack - Medium")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Low Attack - Fierce")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BILL1 )    /* bill */
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_CUSTOM )  /* coin door */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("VOLUME")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_CUSTOM )  /* sound status */
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0000fff0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNUSED )   /* verify */
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00000003, 0x00000003, "Blood Level" )
	PORT_DIPSETTING(          0x00000003, DEF_STR( High ))
	PORT_DIPSETTING(          0x00000002, DEF_STR( Medium ))
	PORT_DIPSETTING(          0x00000001, DEF_STR( Low ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( None ))
	PORT_DIPNAME( 0x00000004, 0x00000004, DEF_STR( Demo_Sounds ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000004, DEF_STR( On ))
	PORT_DIPNAME( 0x00000008, 0x00000008, "Finishing Moves" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000008, DEF_STR( On ))
	PORT_DIPNAME( 0x00000010, 0x00000010, "Display Warning" )
	PORT_DIPSETTING(          0x00000000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000010, DEF_STR( On ))
	PORT_DIPNAME( 0x00000020, 0x00000020, DEF_STR( Unused ))
	PORT_DIPSETTING(          0x00000020, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_DIPNAME( 0x00000040, 0x00000040, DEF_STR( Unused ))
	PORT_DIPSETTING(          0x00000040, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_DIPNAME( 0x00000080, 0x00000080, DEF_STR( Unused ))
	PORT_DIPSETTING(          0x00000080, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_DIPNAME( 0x00000100, 0x00000100, "Coinage Source" )
	PORT_DIPSETTING(          0x00000100, "Dipswitch" )
	PORT_DIPSETTING(          0x00000000, "Disk" )
	PORT_DIPNAME( 0x00003e00, 0x00003e00, DEF_STR( Coinage ))
	PORT_DIPSETTING(          0x00003e00, "USA-1" )
	PORT_DIPSETTING(          0x00003c00, "USA-2" )
	PORT_DIPSETTING(          0x00003a00, "USA-3" )
	PORT_DIPSETTING(          0x00003800, "USA-4" )
	PORT_DIPSETTING(          0x00003400, "USA-9" )
	PORT_DIPSETTING(          0x00003200, "USA-10" )
	PORT_DIPSETTING(          0x00003600, "USA-ECA" )
	PORT_DIPSETTING(          0x00003000, "USA-Free Play" )
	PORT_DIPSETTING(          0x00002e00, "German-1" )
	PORT_DIPSETTING(          0x00002c00, "German-2" )
	PORT_DIPSETTING(          0x00002a00, "German-3" )
	PORT_DIPSETTING(          0x00002800, "German-4" )
	PORT_DIPSETTING(          0x00002600, "German-ECA" )
	PORT_DIPSETTING(          0x00002000, "German-Free Play" )
	PORT_DIPSETTING(          0x00001e00, "French-1" )
	PORT_DIPSETTING(          0x00001c00, "French-2" )
	PORT_DIPSETTING(          0x00001a00, "French-3" )
	PORT_DIPSETTING(          0x00001800, "French-4" )
	PORT_DIPSETTING(          0x00001600, "French-ECA" )
	PORT_DIPSETTING(          0x00001000, "French-Free Play" )
	PORT_DIPNAME( 0x00004000, 0x00004000, "Coin Counters" )
	PORT_DIPSETTING(          0x00004000, "1" )
	PORT_DIPSETTING(          0x00000000, "2" )
	PORT_DIPNAME( 0x00008000, 0x00008000, "Test Switch" )
	PORT_DIPSETTING(          0x00008000, DEF_STR( Off ))
	PORT_DIPSETTING(          0x00000000, DEF_STR( On ))
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END




/*************************************
 *
 *  Machine driver
 *
 *************************************/

void kinst_state::kinst(machine_config &config)
{
	/* basic machine hardware */
	R4600LE(config, m_maincpu, MASTER_CLOCK*2);
	m_maincpu->set_icache_size(16384);
	m_maincpu->set_dcache_size(16384);
	m_maincpu->set_addrmap(AS_PROGRAM, &kinst_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(kinst_state::irq0_start));

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, true);
	m_ata->irq_handler().set_inputline("maincpu", 1);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(320, 240);
	screen.set_visarea(0, 319, 0, 239);
	screen.set_screen_update(FUNC(kinst_state::screen_update));

	PALETTE(config, m_palette, palette_device::BGR_555);

	/* sound hardware */
	DCS_AUDIO_2K(config, m_dcs, 0);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( kinst )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_DEFAULT_BIOS("v1.5d")
	ROM_SYSTEM_BIOS(0, "v1.5d", "Killer Instinct (v1.5d)")
	ROMX_LOAD( "ki-l15d.u98", 0x00000, 0x80000, CRC(7b65ca3d) SHA1(607394d4ba1713f38c2cb5159303cace9cde991e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "v1.4", "Killer Instinct (v1.4)")
	ROMX_LOAD( "ki-l14.u98", 0x00000, 0x80000, CRC(afedb75f) SHA1(07254f20707377f7195e64675eb6458e663c1a9a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "v1.3", "Killer Instinct (v1.3)")
	ROMX_LOAD( "ki-l13.u98", 0x00000, 0x80000, CRC(65f7ea31) SHA1(7f21620a512549db6821a0b4fa53681a767b7974), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "proto-v4.7", "Killer Instinct (proto v4.7)")
	ROMX_LOAD( "ki-p47.u98", 0x00000, 0x80000, CRC(05e67bcb) SHA1(501e69b3026394f69229a6e9866c1037502b86bb), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "v1.5d-anyide", "Killer Instinct (v1.5d AnyIDE)") // unofficial version, allows use of alternate hard drives or CF cards
	ROMX_LOAD( "ki_l15di.u98", 0x00000, 0x80000, CRC(230f55fb) SHA1(f5f12311aae922d12f98d72ac8fdd77b7b084af2), ROM_BIOS(4) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "u10-l1", 0x000000, 0x80000, CRC(b6cc155f) SHA1(810d455df8f385d76143e9d7d048f2b555ff8bf0) )
	ROM_LOAD16_BYTE( "u11-l1", 0x200000, 0x80000, CRC(0b5e05df) SHA1(0595909cb667c38ac7c8c7bd0646b28899e27777) )
	ROM_LOAD16_BYTE( "u12-l1", 0x400000, 0x80000, CRC(d05ce6ad) SHA1(7a8ee405c118fd176b66353fa7bfab888cc63cd2) )
	ROM_LOAD16_BYTE( "u13-l1", 0x600000, 0x80000, CRC(7d0954ea) SHA1(ea4d1f153eb284f1bcfc5295fbce316bba6083f4) )
	ROM_LOAD16_BYTE( "u33-l1", 0x800000, 0x80000, CRC(8bbe4f0c) SHA1(b22e365bc8d58a80eaac226be14b4bb8d9a04844) )
	ROM_LOAD16_BYTE( "u34-l1", 0xa00000, 0x80000, CRC(b2e73603) SHA1(ee439f5162a2b3379d3f802328017bb3c68547d2) )
	ROM_LOAD16_BYTE( "u35-l1", 0xc00000, 0x80000, CRC(0aaef4fc) SHA1(48c4c954ac9db648f28ad64f9845e19ec432eec3) )
	ROM_LOAD16_BYTE( "u36-l1", 0xe00000, 0x80000, CRC(0577bb60) SHA1(cc78070cc41701e9a91fde5cfbdc7e1e83354854) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst", 0, SHA1(81d833236e994528d1482979261401b198d1ca53) )
ROM_END


ROM_START( kinst2 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_DEFAULT_BIOS("v1.4")
	ROM_SYSTEM_BIOS(0, "v1.4", "Killer Instinct 2 (v1.4)")
	ROMX_LOAD( "ki2-l14.u98", 0x00000, 0x80000, CRC(27d0285e) SHA1(aa7a2a9d72a47dd0ea2ee7b2776b79288060b179), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "v1.3", "Killer Instinct 2 (v1.3)")
	ROMX_LOAD( "ki2-l13.u98", 0x00000, 0x80000, CRC(25ebde3b) SHA1(771d150fb4de0a2ceb279954b9545458e93e2405), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "v1.1", "Killer Instinct 2 (v1.1)")
	ROMX_LOAD( "ki2-l11.u98", 0x00000, 0x80000, CRC(0cb8de1e) SHA1(fe447f4b1d29b524f57c5ba1890652ef6afff88a), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "v1.0", "Killer Instinct 2 (v1.0)")
	ROMX_LOAD( "ki2-l10.u98", 0x00000, 0x80000, CRC(b17b4b3d) SHA1(756629cd1b51ae50f2b9818765dd3d277c3019b3), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS(4, "v1.4-anyide", "Killer Instinct 2 (v1.4 AnyIDE)")
	ROMX_LOAD( "ki2_l14p.u98", 0x00000, 0x80000, CRC(d80c937a) SHA1(85a009638f2eada4c63240fc30a9e7be59afab7f), ROM_BIOS(4) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "ki2_l1.u10", 0x000000, 0x80000, CRC(fdf6ed51) SHA1(acfc9460cd5df01403b7f00b2f68c2a8734ad6d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u11", 0x200000, 0x80000, CRC(f9e70024) SHA1(fe7fc78f1c60b15f2bbdc4c455f55cdf30f48ed4) )
	ROM_LOAD16_BYTE( "ki2_l1.u12", 0x400000, 0x80000, CRC(2994c199) SHA1(9997a83432cb720f65b40a8af46f31a5d0d16d8e) )
	ROM_LOAD16_BYTE( "ki2_l1.u13", 0x600000, 0x80000, CRC(3fe6327b) SHA1(7ff164fc2f079d039921594be92208973d43aa03) )
	ROM_LOAD16_BYTE( "ki2_l1.u33", 0x800000, 0x80000, CRC(6f4dcdcf) SHA1(0ab6dbfb76e9fa2db072e287864ad1f9d514dd9b) )
	ROM_LOAD16_BYTE( "ki2_l1.u34", 0xa00000, 0x80000, CRC(5db48206) SHA1(48456a7b6592c40bc9c664dcd2ee2cfd91942811) )
	ROM_LOAD16_BYTE( "ki2_l1.u35", 0xc00000, 0x80000, CRC(7245ce69) SHA1(24a3ff009c8a7f5a0bfcb198b8dcb5df365770d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u36", 0xe00000, 0x80000, CRC(8920acbb) SHA1(0fca72c40067034939b984b4bf32972a5a6c26af) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst2", 0, SHA1(e7c9291b4648eae0012ea0cc230731ed4987d1d5) )
ROM_END


ROM_START( kinst2uk )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* 512k for R4600 code */
	ROM_DEFAULT_BIOS("v1.4k")
	ROM_SYSTEM_BIOS(0, "v1.4k", "Killer Instinct 2 (v1.4k, upgrade kit)")
	ROMX_LOAD( "ki2-l14k.u98", 0x00000, 0x80000, CRC(9cbd00a8) SHA1(926dce4bb9016331ea40d3c337a9ace896f07493), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "v1.3k", "Killer Instinct 2 (v1.3k, upgrade kit)")
	ROMX_LOAD( "ki2-l13k.u98", 0x00000, 0x80000, CRC(3b4f16fc) SHA1(c28416f94453fd1f73ba01025276a04610569d12), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "v1.4-anyide", "Killer Instinct 2 (v1.4k, upgrade kit AnyIDE)")
	ROMX_LOAD( "ki2_d14p.u98", 0x00000, 0x80000, CRC(d716d428) SHA1(1a3b000fdc35b3824a0c8142ba9b496490894543), ROM_BIOS(2) )

	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  /* sound data */
	ROM_LOAD16_BYTE( "ki2_l1.u10", 0x000000, 0x80000, CRC(fdf6ed51) SHA1(acfc9460cd5df01403b7f00b2f68c2a8734ad6d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u11", 0x200000, 0x80000, CRC(f9e70024) SHA1(fe7fc78f1c60b15f2bbdc4c455f55cdf30f48ed4) )
	ROM_LOAD16_BYTE( "ki2_l1.u12", 0x400000, 0x80000, CRC(2994c199) SHA1(9997a83432cb720f65b40a8af46f31a5d0d16d8e) )
	ROM_LOAD16_BYTE( "ki2_l1.u13", 0x600000, 0x80000, CRC(3fe6327b) SHA1(7ff164fc2f079d039921594be92208973d43aa03) )
	ROM_LOAD16_BYTE( "ki2_l1.u33", 0x800000, 0x80000, CRC(6f4dcdcf) SHA1(0ab6dbfb76e9fa2db072e287864ad1f9d514dd9b) )
	ROM_LOAD16_BYTE( "ki2_l1.u34", 0xa00000, 0x80000, CRC(5db48206) SHA1(48456a7b6592c40bc9c664dcd2ee2cfd91942811) )
	ROM_LOAD16_BYTE( "ki2_l1.u35", 0xc00000, 0x80000, CRC(7245ce69) SHA1(24a3ff009c8a7f5a0bfcb198b8dcb5df365770d3) )
	ROM_LOAD16_BYTE( "ki2_l1.u36", 0xe00000, 0x80000, CRC(8920acbb) SHA1(0fca72c40067034939b984b4bf32972a5a6c26af) )

	DISK_REGION( "ata:0:hdd:image" )
	DISK_IMAGE( "kinst2", 0, SHA1(e7c9291b4648eae0012ea0cc230731ed4987d1d5) )
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void kinst_state::init_kinst()
{
	static const uint8_t kinst_control_map[8] = { 0,1,2,3,4,5,6,7 };

	/* set up the control register mapping */
	m_control_map = kinst_control_map;
}


void kinst_state::init_kinst2()
{
	static const uint8_t kinst2_control_map[8] = { 2,4,1,0,3,5,6,7 };

	// read: $80 on ki2 = $90 on ki
	// read: $88 on ki2 = $a0 on ki
	// write: $80 on ki2 = $90 on ki
	// write: $90 on ki2 = $88 on ki
	// write: $98 on ki2 = $80 on ki
	// write: $a0 on ki2 = $98 on ki

	/* set up the control register mapping */
	m_control_map = kinst2_control_map;
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

 // versions selectable by changing bioses

GAME( 1994, kinst,    0,      kinst, kinst,  kinst_state, init_kinst,  ROT0, "Rare", "Killer Instinct", MACHINE_SUPPORTS_SAVE )
GAME( 1995, kinst2,   0,      kinst, kinst2, kinst_state, init_kinst2, ROT0, "Rare", "Killer Instinct 2", MACHINE_SUPPORTS_SAVE )
GAME( 1995, kinst2uk, kinst2, kinst, kinst2, kinst_state, init_kinst2, ROT0, "Rare", "Killer Instinct 2 (Upgrade kit)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
