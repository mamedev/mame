// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
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
#include "includes/amiga.h"
#include "cpu/m68000/m68000.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/amigafdc.h"
#include "speaker.h"


class upscope_state : public amiga_state
{
public:
	upscope_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag)
		, m_prev_cia1_porta(0xff)
		, m_parallel_data(0xff)
		, m_ppi(*this, "ppi")
	{ }

	void upscope(machine_config &config);

	void init_upscope();

private:
	uint8_t m_nvram[0x100];
	uint8_t m_prev_cia1_porta;
	uint8_t m_parallel_data;
	uint8_t m_nvram_address_latch;
	uint8_t m_nvram_data_latch;

	uint8_t upscope_cia_0_portb_r();
	void upscope_cia_0_portb_w(uint8_t data);
	uint8_t upscope_cia_1_porta_r();
	void upscope_cia_1_porta_w(uint8_t data);

	void lamps_w(uint8_t data);
	void coin_counter_w(uint8_t data);


	void a500_mem(address_map &map);
	void main_map(address_map &map);
	void overlay_512kb_map(address_map &map);

	virtual void machine_reset() override;

	required_device<i8255_device> m_ppi;
};


/*************************************
 *
 *  Debugging
 *
 *************************************/

#define LOG_IO          0



/*************************************
 *
 *  Reset state
 *
 *************************************/

void upscope_state::machine_reset()
{
	// reset base machine
	amiga_state::machine_reset();

	m_prev_cia1_porta = 0xff;
}


void upscope_state::upscope_cia_0_portb_w(uint8_t data)
{
	m_parallel_data = data;
}

uint8_t upscope_state::upscope_cia_0_portb_r()
{
	return m_nvram_data_latch;
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

uint8_t upscope_state::upscope_cia_1_porta_r()
{
	return 0xf8 | (m_prev_cia1_porta & 0x07);
}

void upscope_state::upscope_cia_1_porta_w(uint8_t data)
{
	/* on a low transition of POUT, we latch stuff for the NVRAM */
	if ((m_prev_cia1_porta & 2) && !(data & 2))
	{
		/* if SEL == 1 && BUSY == 0, we latch an address */
		if ((data & 5) == 4)
		{
			if (LOG_IO) logerror("Latch address: %02X\n", m_parallel_data);
			m_nvram_address_latch = m_parallel_data;
		}

		/* if SEL == 1 && BUSY == 1, we write data to internal registers */
		else if ((data & 5) == 5)
		{
			m_ppi->write(m_nvram_address_latch & 0x03, m_parallel_data);
		}

		/* if SEL == 0 && BUSY == 1, we write data to NVRAM */
		else if ((data & 5) == 1)
		{
			if (LOG_IO) logerror("NVRAM data write @ %02X = %02X\n", m_nvram_address_latch, m_parallel_data);
			m_nvram[m_nvram_address_latch] = m_parallel_data;
		}

		/* if SEL == 0 && BUSY == 0, who knows? */
		else
		{
			logerror("Unexpected: POUT low with SEL == 0/BUSY == 0\n");
		}
	}

	/* on a low transition of BUSY, we latch stuff for reading */
	else if ((m_prev_cia1_porta & 1) && !(data & 1))
	{
		/* if SEL == 1, we read internal data registers */
		if (data & 4)
		{
			if (LOG_IO) logerror("Internal register (%d) read\n", m_nvram_address_latch);
			m_nvram_data_latch = m_ppi->read(m_nvram_address_latch & 0x03);
		}

		/* if SEL == 0, we read NVRAM */
		else
		{
			m_nvram_data_latch = m_nvram[m_nvram_address_latch];
			if (LOG_IO) logerror("NVRAM data read @ %02X = %02X\n", m_nvram_address_latch, m_nvram_data_latch);
		}
	}

	/* remember the previous value */
	m_prev_cia1_porta = data;
}

void upscope_state::lamps_w(uint8_t data)
{
	// 7-------  bubble light
	// -6------  sight
	// --5-----  torpedo 4
	// ---4----  torpedo 3
	// ----3---  torpedo 2
	// -----2--  torpedo 1
	// ------1-  enemy left
	// -------0  enemy right
}

void upscope_state::coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
}


/*************************************
 *
 *  Memory map
 *
 *************************************/

void upscope_state::overlay_512kb_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x07ffff).mirror(0x180000).ram().share("chip_ram");
	map(0x200000, 0x27ffff).rom().region("kickstart", 0);
}

void upscope_state::a500_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).m(m_overlay, FUNC(address_map_bank_device::amap16));
	map(0xa00000, 0xbfffff).rw(FUNC(upscope_state::cia_r), FUNC(upscope_state::cia_w));
	map(0xc00000, 0xd7ffff).m(m_chipset, FUNC(address_map_bank_device::amap16));
	map(0xd80000, 0xddffff).noprw();
	map(0xde0000, 0xdeffff).m(m_chipset, FUNC(address_map_bank_device::amap16));
	map(0xdf0000, 0xdfffff).m(m_chipset, FUNC(address_map_bank_device::amap16));
	map(0xe00000, 0xe7ffff).nopw().r(FUNC(upscope_state::rom_mirror_r));
	map(0xe80000, 0xefffff).noprw(); // autoconfig space (installed by devices)
	map(0xf80000, 0xffffff).rom().region("kickstart", 0);
}

void upscope_state::main_map(address_map &map)
{
	a500_mem(map);
	map(0xf00000, 0xf7ffff).rom().region("user2", 0);
}



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

void upscope_state::upscope(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, amiga_state::CLK_7M_NTSC);
	m_maincpu->set_addrmap(AS_PROGRAM, &upscope_state::main_map);

	ADDRESS_MAP_BANK(config, m_overlay).set_map(&upscope_state::overlay_512kb_map).set_options(ENDIANNESS_BIG, 16, 22, 0x200000);
	ADDRESS_MAP_BANK(config, m_chipset).set_map(&upscope_state::ocs_map).set_options(ENDIANNESS_BIG, 16, 9, 0x200);

	AMIGA_COPPER(config, m_copper, amiga_state::CLK_7M_NTSC);
	m_copper->set_host_cpu_tag(m_maincpu);
	m_copper->mem_read_cb().set(FUNC(amiga_state::chip_ram_r));
	m_copper->set_ecs_mode(false);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	ntsc_video(config);

	PALETTE(config, m_palette, FUNC(upscope_state::amiga_palette), 4096);

	MCFG_VIDEO_START_OVERRIDE(upscope_state,amiga)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	PAULA_8364(config, m_paula, amiga_state::CLK_C1_NTSC);
	m_paula->add_route(0, "rspeaker", 0.50);
	m_paula->add_route(1, "lspeaker", 0.50);
	m_paula->add_route(2, "lspeaker", 0.50);
	m_paula->add_route(3, "rspeaker", 0.50);
	m_paula->mem_read_cb().set(FUNC(amiga_state::chip_ram_r));
	m_paula->int_cb().set(FUNC(amiga_state::paula_int_w));

	/* cia */
	MOS8520(config, m_cia_0, amiga_state::CLK_E_NTSC);
	m_cia_0->irq_wr_callback().set(FUNC(amiga_state::cia_0_irq));
	m_cia_0->pa_wr_callback().set(FUNC(amiga_state::cia_0_port_a_write));
	m_cia_0->pb_rd_callback().set(FUNC(upscope_state::upscope_cia_0_portb_r));
	m_cia_0->pb_wr_callback().set(FUNC(upscope_state::upscope_cia_0_portb_w));

	MOS8520(config, m_cia_1, amiga_state::CLK_E_NTSC);
	m_cia_1->irq_wr_callback().set(FUNC(amiga_state::cia_1_irq));
	m_cia_1->pa_rd_callback().set(FUNC(upscope_state::upscope_cia_1_porta_r));
	m_cia_1->pa_wr_callback().set(FUNC(upscope_state::upscope_cia_1_porta_w));

	/* fdc */
	AMIGA_FDC(config, m_fdc, amiga_state::CLK_7M_NTSC);
	m_fdc->index_callback().set("cia_1", FUNC(mos8520_device::flag_w));
	m_fdc->read_dma_callback().set(FUNC(amiga_state::chip_ram_r));
	m_fdc->write_dma_callback().set(FUNC(amiga_state::chip_ram_w));
	m_fdc->dskblk_callback().set(FUNC(amiga_state::fdc_dskblk_w));
	m_fdc->dsksyn_callback().set(FUNC(amiga_state::fdc_dsksyn_w));

	// i/o extension
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set_ioport("IO0");
	m_ppi->out_pb_callback().set(FUNC(upscope_state::lamps_w));
	m_ppi->out_pc_callback().set(FUNC(upscope_state::coin_counter_w));
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( upscope )
	ROM_REGION16_BE(0x80000, "kickstart", 0)
	ROM_LOAD16_WORD("315093-01.u2", 0x00000, 0x40000, CRC(a6ce1636) SHA1(11f9e62cf299f72184835b7b2a70a16333fc0d88))
	ROM_COPY("kickstart", 0x000000, 0x40000, 0x40000)

	ROM_REGION16_BE(0x080000, "user2", 0)
	ROM_LOAD16_BYTE( "upscope.u5",   0x000001, 0x008000, CRC(c109912e) SHA1(dcac9522e3c4818b2a02212b9173540fcf4bd463) )
	ROM_LOAD16_BYTE( "upscope.u13",  0x000000, 0x008000, CRC(9c8b071a) SHA1(69f9f8c17630ed568975e65dadc03213677a12dd) )
	ROM_LOAD16_BYTE( "upscope.u6",   0x010001, 0x008000, CRC(962f371e) SHA1(5682c62f34df2cc70f6125cf14203087670571db) )
	ROM_LOAD16_BYTE( "upscope.u14",  0x010000, 0x008000, CRC(1231bfc1) SHA1(f99adfabb01c1a15130f82f6a09d5458109a28bb) )

	ROM_LOAD16_BYTE( "upscope.u1",   0x040001, 0x008000, CRC(7a8de1fb) SHA1(30b87f07e0e0f66699402dffaeb0ca00c554f23e) )
	ROM_LOAD16_BYTE( "upscope.u9",   0x040000, 0x008000, CRC(5d16521e) SHA1(93e0a1644bd8adbb6f9fca6d4a252c11812c6ada) )
	ROM_LOAD16_BYTE( "upscope.u2",   0x050001, 0x008000, CRC(2089ef6b) SHA1(a12d87c8b368ffbadb556aca2e43e50348d34839) )
	ROM_LOAD16_BYTE( "upscope.u10",  0x050000, 0x008000, CRC(fbab44f5) SHA1(cd49f1f79e2181b3a9c40aebfba9d7c314dc909b) )

	ROM_LOAD16_BYTE( "upscope.u3",   0x060001, 0x008000, CRC(9b325528) SHA1(5bde1a42b62dd810843349ee9edf76e1c7521653) )
	ROM_LOAD16_BYTE( "upscope.u11",  0x060000, 0x008000, CRC(40e54449) SHA1(7d6ed97b87d74d80776cb682c78cd3b4a68633f4) )
	ROM_LOAD16_BYTE( "upscope.u4",   0x070001, 0x008000, CRC(6585ef1d) SHA1(b95e5e424266a50d4b63501278eb5d618fde5ba2) )
	ROM_LOAD16_BYTE( "upscope.u12",  0x070000, 0x008000, CRC(a909e388) SHA1(62acc30ab97d6a46a6d0782bb4ceb58061332724) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

void upscope_state::init_upscope()
{
	m_agnus_id = AGNUS_HR_NTSC;
	m_denise_id = DENISE;

	// allocate nvram
	subdevice<nvram_device>("nvram")->set_base(m_nvram, sizeof(m_nvram));
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1986, upscope, 0, upscope, upscope, upscope_state, init_upscope, ORIENTATION_FLIP_X, "Grand Products", "Up Scope", MACHINE_IMPERFECT_SOUND )
