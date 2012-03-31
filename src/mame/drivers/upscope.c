/**********************************************************************************

    Up Scope

    Driver by Mariusz Wojcieszek

***********************************************************************************

    Up Scope
    Grand Products 1986

    rom board       +--------------+
                                      amiga I/O board
    u4  u3  u2  u1
            u6  u5      Amiga 500      7404 7400
                                       74374 6116  8255  TIP120 TIP120
    u12 u11 u10 u9                         battery       TIP120 TIP120 TIP120 TIP120
            u14 u13                                      TIP120 TIP120 TIP120 TIP120



                    +---------------+

**********************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/amiga.h"
#include "machine/6526cia.h"
#include "machine/nvram.h"
#include "machine/amigafdc.h"


class upscope_state : public amiga_state
{
public:
	upscope_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag) { }

	UINT8	m_nvram[0x100];
	UINT8 m_prev_cia1_porta;
	UINT8 m_parallel_data;
	UINT8 m_nvram_address_latch;
	UINT8 m_nvram_data_latch;
};


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define LOG_IO			0



/*************************************
 *
 *  Reset state
 *
 *************************************/

static void upscope_reset(running_machine &machine)
{
	upscope_state *state = machine.driver_data<upscope_state>();
	state->m_prev_cia1_porta = 0xff;
}



/*************************************
 *
 *  CIA-A port A access:
 *
 *  PA7 = game port 1, pin 6 (fire)
 *  PA6 = game port 0, pin 6 (fire)
 *  PA5 = /RDY (disk ready)
 *  PA4 = /TK0 (disk track 00)
 *  PA3 = /WPRO (disk write protect)
 *  PA2 = /CHNG (disk change)
 *  PA1 = /LED (LED, 0=bright / audio filter control)
 *  PA0 = OVL (ROM/RAM overlay bit)
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( upscope_cia_0_porta_w )
{
	/* switch banks as appropriate */
	memory_set_bank(device->machine(), "bank1", data & 1);

	/* swap the write handlers between ROM and bank 1 based on the bit */
	if ((data & 1) == 0)
		/* overlay disabled, map RAM on 0x000000 */
		device->machine().device("maincpu")->memory().space(AS_PROGRAM)->install_write_bank(0x000000, 0x07ffff, "bank1");

	else
		/* overlay enabled, map Amiga system ROM on 0x000000 */
		device->machine().device("maincpu")->memory().space(AS_PROGRAM)->unmap_write(0x000000, 0x07ffff);
}



/*************************************
 *
 *  CIA-A port B access:
 *
 *  PB7 = parallel data 7
 *  PB6 = parallel data 6
 *  PB5 = parallel data 5
 *  PB4 = parallel data 4
 *  PB3 = parallel data 3
 *  PB2 = parallel data 2
 *  PB1 = parallel data 1
 *  PB0 = parallel data 0
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( upscope_cia_0_portb_w )
{
	upscope_state *state = device->machine().driver_data<upscope_state>();
	state->m_parallel_data = data;
}

static READ8_DEVICE_HANDLER( upscope_cia_0_portb_r )
{
	upscope_state *state = device->machine().driver_data<upscope_state>();
	return state->m_nvram_data_latch;
}



/*************************************
 *
 *  CIA-B port A access:
 *
 *  PA7 = com line /DTR
 *  PA6 = com line /RTS
 *  PA5 = com line /carrier detect
 *  PA4 = com line /CTS
 *  PA3 = com line /DSR
 *  PA2 = SEL (Centronics parallel control)
 *  PA1 = POUT (Centronics parallel control)
 *  PA0 = BUSY (Centronics parallel control)
 *
 *************************************/

static READ8_DEVICE_HANDLER( upscope_cia_1_porta_r )
{
	upscope_state *state = device->machine().driver_data<upscope_state>();
	return 0xf8 | (state->m_prev_cia1_porta & 0x07);
}

static WRITE8_DEVICE_HANDLER( upscope_cia_1_porta_w )
{
	upscope_state *state = device->machine().driver_data<upscope_state>();
	/* on a low transition of POUT, we latch stuff for the NVRAM */
	if ((state->m_prev_cia1_porta & 2) && !(data & 2))
	{
		/* if SEL == 1 && BUSY == 0, we latch an address */
		if ((data & 5) == 4)
		{
			if (LOG_IO) logerror("Latch address: %02X\n", state->m_parallel_data);
			state->m_nvram_address_latch = state->m_parallel_data;
		}

		/* if SEL == 1 && BUSY == 1, we write data to internal registers */
		else if ((data & 5) == 5)
		{
			switch (state->m_nvram_address_latch)
			{
				case 0x01:
					/* lamps:
                        01 = Enemy Right
                        02 = Enemy Left
                        04 = Torpedo 1
                        08 = Torpedo 2
                        10 = Torpedo 3
                        20 = Torpedo 4
                        40 = Sight
                        80 = Bubble Light
                    */
					break;

				case 0x02:
					/* coin counter */
					coin_counter_w(device->machine(), 0, data & 1);
					break;

				case 0x03:
					/* Written $98 at startup and nothing afterwards */
					break;

				default:
					logerror("Internal register (%d) = %02X\n", state->m_nvram_address_latch, state->m_parallel_data);
					break;
			}
		}

		/* if SEL == 0 && BUSY == 1, we write data to NVRAM */
		else if ((data & 5) == 1)
		{
			if (LOG_IO) logerror("NVRAM data write @ %02X = %02X\n", state->m_nvram_address_latch, state->m_parallel_data);
			state->m_nvram[state->m_nvram_address_latch] = state->m_parallel_data;
		}

		/* if SEL == 0 && BUSY == 0, who knows? */
		else
		{
			logerror("Unexpected: POUT low with SEL == 0/BUSY == 0\n");
		}
	}

	/* on a low transition of BUSY, we latch stuff for reading */
	else if ((state->m_prev_cia1_porta & 1) && !(data & 1))
	{
		/* if SEL == 1, we read internal data registers */
		if (data & 4)
		{
			if (LOG_IO) logerror("Internal register (%d) read\n", state->m_nvram_address_latch);
			state->m_nvram_data_latch = (state->m_nvram_address_latch == 0) ? input_port_read(device->machine(), "IO0") : 0xff;
		}

		/* if SEL == 0, we read NVRAM */
		else
		{
			state->m_nvram_data_latch = state->m_nvram[state->m_nvram_address_latch];
			if (LOG_IO) logerror("NVRAM data read @ %02X = %02X\n", state->m_nvram_address_latch, state->m_nvram_data_latch);
		}
	}

	/* remember the previous value */
	state->m_prev_cia1_porta = data;
}



/*************************************
 *
 *  Memory map
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, upscope_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x07ffff) AM_RAMBANK("bank1") AM_BASE_SIZE_MEMBER(upscope_state, m_chip_ram, m_chip_ram_size)
	AM_RANGE(0xbfd000, 0xbfefff) AM_READWRITE(amiga_cia_r, amiga_cia_w)
	AM_RANGE(0xc00000, 0xdfffff) AM_READWRITE(amiga_custom_r, amiga_custom_w)  AM_BASE_MEMBER(upscope_state, m_custom_regs)
	AM_RANGE(0xe80000, 0xe8ffff) AM_READWRITE(amiga_autoconfig_r, amiga_autoconfig_w)
	AM_RANGE(0xfc0000, 0xffffff) AM_ROM AM_REGION("user1", 0)			/* System ROM */

	AM_RANGE(0xf00000, 0xf7ffff) AM_ROM AM_REGION("user2", 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( upscope )
	PORT_START("POT1DAT")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_SENSITIVITY(25) PORT_MINMAX(0x02,0xfe) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START("IO0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static const mos6526_interface cia_0_intf =
{
	0,														/* tod_clock */
	DEVCB_LINE(amiga_cia_0_irq),										/* irq_func */
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(upscope_cia_0_porta_w),					/* port A */
	DEVCB_HANDLER(upscope_cia_0_portb_r),
	DEVCB_HANDLER(upscope_cia_0_portb_w)	/* port B */
};

static const mos6526_interface cia_1_intf =
{
	0,														/* tod_clock */
	DEVCB_LINE(amiga_cia_1_irq),										/* irq_func */
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(upscope_cia_1_porta_r),
	DEVCB_HANDLER(upscope_cia_1_porta_w),	/* port A */
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_CONFIG_START( upscope, upscope_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, AMIGA_68000_NTSC_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_MACHINE_RESET(amiga)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.997)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512*2, 262)
	MCFG_SCREEN_VISIBLE_AREA((129-8)*2, (449+8-1)*2, 44-8, 244+8-1)
	MCFG_SCREEN_UPDATE_STATIC(amiga)

	MCFG_PALETTE_LENGTH(4096)
	MCFG_PALETTE_INIT(amiga)

	MCFG_VIDEO_START(amiga)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("amiga", AMIGA, 3579545)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(2, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(3, "rspeaker", 0.50)

	/* cia */
	MCFG_MOS8520_ADD("cia_0", AMIGA_68000_NTSC_CLOCK / 10, cia_0_intf)
	MCFG_MOS8520_ADD("cia_1", AMIGA_68000_NTSC_CLOCK / 10, cia_1_intf)

	/* fdc */
	MCFG_AMIGA_FDC_ADD("fdc", AMIGA_68000_NTSC_CLOCK)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( upscope )
	ROM_REGION(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP( "kick12.rom", 0x000000, 0x40000, CRC(a6ce1636) SHA1(11f9e62cf299f72184835b7b2a70a16333fc0d88) )
	ROM_COPY( "user1", 0x000000, 0x040000, 0x040000 )

	ROM_REGION(0x080000, "user2", 0)
	ROM_LOAD16_BYTE( "upscope.u5",   0x000000, 0x008000, CRC(c109912e) SHA1(dcac9522e3c4818b2a02212b9173540fcf4bd463) )
	ROM_LOAD16_BYTE( "upscope.u13",  0x000001, 0x008000, CRC(9c8b071a) SHA1(69f9f8c17630ed568975e65dadc03213677a12dd) )
	ROM_LOAD16_BYTE( "upscope.u6",   0x010000, 0x008000, CRC(962f371e) SHA1(5682c62f34df2cc70f6125cf14203087670571db) )
	ROM_LOAD16_BYTE( "upscope.u14",  0x010001, 0x008000, CRC(1231bfc1) SHA1(f99adfabb01c1a15130f82f6a09d5458109a28bb) )

	ROM_LOAD16_BYTE( "upscope.u1",   0x040000, 0x008000, CRC(7a8de1fb) SHA1(30b87f07e0e0f66699402dffaeb0ca00c554f23e) )
	ROM_LOAD16_BYTE( "upscope.u9",   0x040001, 0x008000, CRC(5d16521e) SHA1(93e0a1644bd8adbb6f9fca6d4a252c11812c6ada) )
	ROM_LOAD16_BYTE( "upscope.u2",   0x050000, 0x008000, CRC(2089ef6b) SHA1(a12d87c8b368ffbadb556aca2e43e50348d34839) )
	ROM_LOAD16_BYTE( "upscope.u10",  0x050001, 0x008000, CRC(fbab44f5) SHA1(cd49f1f79e2181b3a9c40aebfba9d7c314dc909b) )

	ROM_LOAD16_BYTE( "upscope.u3",   0x060000, 0x008000, CRC(9b325528) SHA1(5bde1a42b62dd810843349ee9edf76e1c7521653) )
	ROM_LOAD16_BYTE( "upscope.u11",  0x060001, 0x008000, CRC(40e54449) SHA1(7d6ed97b87d74d80776cb682c78cd3b4a68633f4) )
	ROM_LOAD16_BYTE( "upscope.u4",   0x070000, 0x008000, CRC(6585ef1d) SHA1(b95e5e424266a50d4b63501278eb5d618fde5ba2) )
	ROM_LOAD16_BYTE( "upscope.u12",  0x070001, 0x008000, CRC(a909e388) SHA1(62acc30ab97d6a46a6d0782bb4ceb58061332724) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

static DRIVER_INIT( upscope )
{
	upscope_state *state = machine.driver_data<upscope_state>();
	static const amiga_machine_interface upscope_intf =
	{
		ANGUS_CHIP_RAM_MASK,
		NULL, NULL, NULL,
		NULL,
		NULL, upscope_reset,
		NULL,
		0
	};
	amiga_machine_config(machine, &upscope_intf);

	/* allocate NVRAM */
	machine.device<nvram_device>("nvram")->set_base(state->m_nvram, sizeof(state->m_nvram));

	/* set up memory */
	memory_configure_bank(machine, "bank1", 0, 1, state->m_chip_ram, 0);
	memory_configure_bank(machine, "bank1", 1, 1, machine.region("user1")->base(), 0);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1986, upscope, 0, upscope, upscope, upscope, ORIENTATION_FLIP_X, "Grand Products", "Up Scope", GAME_IMPERFECT_SOUND )
