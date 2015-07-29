// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*************************************************************************

    barata.c

    "Dona Barata"

    Brazilian "whack-a-mole"-style game themed after stepping on cockroaches.
    The name "Dona Barata" means "Lady Cockroach" in brazilian portuguese.

    Manufactured by Matic: http://maticplay.com.br/
    This driver still only emulates an early prototype of the game.
    Propper dumps of the actual released game is still lacking.
    Photos on the web make us believe that there are at least 2 official
    releases of this game.

    http://www.maticplay.com.br/equipamentos.php?equipamento=dona-barata
    http://www.valedosduendes.com.br/site/wp-content/uploads/2012/02/barata_1.jpg

    Driver by Felipe Sanches <juca@members.fsf.org>

**************************************************************************

    TO-DO:

    * at the moment, the portbits for the rows are still a guess
    * as we don't have access to actual PCBs, the CPU clock frequency is a guess
        (but maybe it can be infered by analysing the 1ms delay routine used)
    * we don't have sound samples or background music dumps
        (i.e. we lack dumps of all of the sound memory)
    * we don't have ROM dumps of the official releases of the game
    * it would be nice to add photographic artwork to improve the layout

**************************************************************************/

#define CPU_CLOCK       (XTAL_6MHz)         /* main cpu clock */

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/dac.h"
#include "barata.lh"
#include "rendlay.h"

class barata_state : public driver_device
{
public:
	barata_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac") { }
	DECLARE_WRITE8_MEMBER(fpga_w);
	DECLARE_WRITE8_MEMBER(port0_w);
	DECLARE_WRITE8_MEMBER(port2_w);
	DECLARE_READ8_MEMBER(port2_r);
	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_dac;
private:
	unsigned char row_selection;
};

/************************
*      Input Ports      *
************************/

static INPUT_PORTS_START( barata )
	PORT_START("PORT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

/* these portbits for the cockroach button rows are still a guess */
	PORT_START("PLAYER1_ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1_0") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1_1") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1_2") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1_3") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PLAYER1_ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1_4") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1_5") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1_6") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P1_7") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PLAYER2_ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2_0") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2_1") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2_2") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2_3") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PLAYER2_ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2_4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2_5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2_6") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("P2_7") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* BCD to Seven Segment Decoder */
static UINT8 dec_7seg(int data)
{
	UINT8 segment;
	switch (data)
	{
		case 0: segment = 0x3f; break;
		case 1: segment = 0x06; break;
		case 2: segment = 0x5b; break;
		case 3: segment = 0x4f; break;
		case 4: segment = 0x66; break;
		case 5: segment = 0x6d; break;
		case 6: segment = 0x7d; break;
		case 7: segment = 0x07; break;
		case 8: segment = 0x7f; break;
		case 9: segment = 0x6f; break;
		default: segment = 0x79;
	}

	return segment;
}

#define FPGA_PLAY_BGM               0
#define FPGA_STOP_BGM               1
#define FPGA_PLAY_SAMPLE            2
#define FPGA_LAMP                   3
#define FPGA_COUNTER                4
#define FPGA_WAITING_FOR_NEW_CMD    5

const char* mode_strings[] = {
"Play background music",
"Stop background music",
"Play sound sample",
"Set lamp states",
"Set counter values"
};

static void fpga_send(unsigned char cmd){
	static unsigned char byte = 0;
	static unsigned char mode = FPGA_WAITING_FOR_NEW_CMD;
	static unsigned char lamp_data = 0;

	logerror("FPGA CMD: %d\n", cmd);

	if (mode == FPGA_WAITING_FOR_NEW_CMD){
		if (cmd < FPGA_WAITING_FOR_NEW_CMD){
			mode = cmd;
			byte=1;
			logerror("SET FPGA MODE: %s\n", mode_strings[mode]);

			if (mode == FPGA_PLAY_BGM){
				logerror("PLAY_BGM.\n");
				mode = FPGA_WAITING_FOR_NEW_CMD;
			}

			if (mode == FPGA_STOP_BGM){
				logerror("STOP_BGM.\n");
				mode = FPGA_WAITING_FOR_NEW_CMD;
			}
		}
		return;
	}

	bool state, erase_all;
	char lamp_index;
	if (mode == FPGA_LAMP){
		switch (byte){
			case 1:
				lamp_data = cmd;
				break;
			case 2:
				lamp_data = (lamp_data << 3) | cmd;
				state = BIT(lamp_data,5);
				erase_all = BIT(lamp_data,4);
				lamp_index = lamp_data & 0x0F;

				if (erase_all){
//                  logerror("LED: ERASE ALL\n");
					for (int i=0; i<16; i++){
						output_set_led_value(i, 1);
					}
				} else {
					output_set_led_value(lamp_index, state ? 0 : 1);
				}
			default:
				mode = FPGA_WAITING_FOR_NEW_CMD;
				break;
		}
		byte++;
		return;
	}

	static unsigned char counter_bank = 0;
	static unsigned char counter_data = 0;
	static bool counter_state = false;
	if (mode == FPGA_COUNTER){
		//logerror("FPGA_COUNTER byte:%d cmd:%d\n", byte, cmd);
		switch (byte){
			case 1:
				counter_bank = BIT(cmd,2);
				counter_state = BIT(cmd,1);
				counter_data = (cmd & 1);
				break;
			case 2:
				counter_data = (counter_data << 3) | cmd;
				break;
			case 3:
				counter_data = (counter_data << 3) | cmd;

				if (counter_state){
					output_set_digit_value(2*counter_bank, 0);
					output_set_digit_value(2*counter_bank+1, 0);
				} else {
					output_set_digit_value(2*counter_bank, dec_7seg(counter_data/10));
					output_set_digit_value(2*counter_bank+1, dec_7seg(counter_data%10));
				}
			default:
				mode = FPGA_WAITING_FOR_NEW_CMD;
				break;
		}
		byte++;
		return;
	}

	static unsigned char sample_index = 0;
	if (mode == FPGA_PLAY_SAMPLE){
		switch (byte){
			case 1:
				sample_index = cmd;
				break;
			case 2:
				sample_index = (sample_index << 3) | cmd;
				logerror("PLAY_SAMPLE #%d.\n", sample_index);
			default:
				mode = FPGA_WAITING_FOR_NEW_CMD;
				break;
		}
		byte++;
		return;
	}
}

WRITE8_MEMBER(barata_state::fpga_w)
{
	static unsigned char old_data = 0;
	if (!BIT(old_data, 5) && BIT(data, 5)){
		//process the command sent to the FPGA
		fpga_send((data >> 2) & 7);
	}
	old_data = data;
}

WRITE8_MEMBER(barata_state::port0_w)
{
	row_selection = data;
}

WRITE8_MEMBER(barata_state::port2_w)
{
	/* why does it write to PORT2 ? */
}

READ8_MEMBER(barata_state::port2_r)
{
	if (!BIT(row_selection, 0)) return ioport("PLAYER1_ROW1")->read();
	if (!BIT(row_selection, 1)) return ioport("PLAYER1_ROW2")->read();
	if (!BIT(row_selection, 2)) return ioport("PLAYER2_ROW1")->read();
	if (!BIT(row_selection, 3)) return ioport("PLAYER2_ROW2")->read();
	return 0;
}

/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( i8051_io_port, AS_IO, 8, barata_state )
	AM_RANGE(MCS51_PORT_P0,   MCS51_PORT_P0  ) AM_WRITE(port0_w)
	AM_RANGE(MCS51_PORT_P1,   MCS51_PORT_P1  ) AM_READ_PORT("PORT1")
	AM_RANGE(MCS51_PORT_P2,   MCS51_PORT_P2  ) AM_READWRITE(port2_r, port2_w)
	AM_RANGE(MCS51_PORT_P3,   MCS51_PORT_P3  ) AM_WRITE(fpga_w)
ADDRESS_MAP_END

/************************
*    Machine Drivers    *
************************/

static MACHINE_CONFIG_START( barata, barata_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8051, CPU_CLOCK)
	MCFG_CPU_IO_MAP(i8051_io_port)

	MCFG_DEFAULT_LAYOUT( layout_barata )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.55)

	/* TODO: add sound samples */
MACHINE_CONFIG_END

/*************************
*        Rom Load        *
*************************/

ROM_START( barata )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "barata.bin",      0x0000, 0x06a8, CRC(a5b68617) SHA1(4c7cd7c494d20236732c8d1f2b2904bfe99f5252) )
ROM_END

/*************************
*      Game Drivers      *
*************************/
GAME( 2002, barata,     0,        barata,   barata,    driver_device, 0,        ROT0,  "Eletro Matic Equipamentos Eletromec??nicos", "Dona Barata (early prototype)", MACHINE_IMPERFECT_GRAPHICS )
