#include "emu.h"
#include "cpu/z80/z80.h"
#include "peyper.lh"

class peyper_state : public driver_device
{
public:
	peyper_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

    UINT8 irq_state;

    UINT8 display_block;
    UINT8 display[16];
};


static READ8_HANDLER(sw_r)
{
    return 0xff;
}

static WRITE8_HANDLER(col_w)
{
    peyper_state *state = space->machine().driver_data<peyper_state>();

    if (data==0x90) state->display_block = 0;
}

static const UINT8 hex_to_7seg[16] =
    {0x3F, 0x06, 0x5B, 0x4F,
     0x66, 0x6D, 0x7D, 0x07,
     0x7F, 0x6F, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00 };

/* seems to only work correctly for 'solarwap', 'poleposn' and 'sonstwar' (look at how high-scores are displayed for example) - or shall layout be changed ? */
static WRITE8_HANDLER(disp_w)
{
    peyper_state *state = space->machine().driver_data<peyper_state>();
    state->display[state->display_block] = data;

    UINT8 a = data & 0x0f;
    UINT8 b = data >> 4;
    UINT8 hex_a = hex_to_7seg[a];
    UINT8 hex_b = hex_to_7seg[b];
/*
0 -> XA0 DPL25,DPL27
1 -> XA1 DPL26,DPL28
2 -> DPL23,DPL5
3 -> DPL22,DPL4
4 -> DPL21,DPL3
5 -> DPL20,DPL2
6 -> DPL19,DPL1
7 -> DPL30,DPL33
*/
    switch(state->display_block) {
        case 0 :
                output_set_indexed_value("dpl_",25,hex_a);
                output_set_indexed_value("dpl_",27,hex_b);
                break;
        case 1 :
                output_set_indexed_value("dpl_",26,hex_a);
                output_set_indexed_value("dpl_",28,hex_b);
                break;
        case 2 :
                output_set_indexed_value("dpl_",23,hex_a);
                output_set_indexed_value("dpl_",5,hex_b);
                break;
        case 3 :
                output_set_indexed_value("dpl_",22,hex_a);
                output_set_indexed_value("dpl_",4,hex_b);
                break;
        case 4 :
                output_set_indexed_value("dpl_",21,hex_a);
                output_set_indexed_value("dpl_",3,hex_b);
                break;
        case 5 :
                output_set_indexed_value("dpl_",20,hex_a);
                output_set_indexed_value("dpl_",2,hex_b);
                break;
        case 6 :
                output_set_indexed_value("dpl_",19,hex_a);
                output_set_indexed_value("dpl_",1,hex_b);
                break;
        case 7 :
                output_set_indexed_value("dpl_",30,hex_a);
                output_set_indexed_value("dpl_",33,hex_b);
                break;
/*
8 ->  XB0
9 ->  XB1
10 -> DPL11,DPL17
11 -> DPL10,DPL16
12 -> DPL09,DPL15
13 -> DPL08,DPL14
14 -> DPL07,DPL13
15 -> DPL31,DPL32
*/
        case 8 :
                /*
                if (BIT(a,3)) logerror("TILT\n");
                if (BIT(a,2)) logerror("ONC\n");
                if (BIT(a,1)) logerror("GAME OVER\n");
                if (BIT(a,0)) logerror("BALL IN PLAY\n");
                */
                output_set_indexed_value("led_",1,BIT(b,0)); // PLAYER 1
                output_set_indexed_value("led_",2,BIT(b,1)); // PLAYER 2
                output_set_indexed_value("led_",3,BIT(b,2)); // PLAYER 3
                output_set_indexed_value("led_",4,BIT(b,3)); // PLAYER 4
                break;
        case 9 :
                if (!BIT(b,0)) output_set_indexed_value("dpl_",6,hex_to_7seg[0]);
                if (!BIT(b,1)) output_set_indexed_value("dpl_",12,hex_to_7seg[0]);
                if (!BIT(b,2)) output_set_indexed_value("dpl_",24,hex_to_7seg[0]);
                if (!BIT(b,3)) output_set_indexed_value("dpl_",18,hex_to_7seg[0]);
                output_set_indexed_value("dpl_",29,hex_a);
                break;
        case 10 :
                output_set_indexed_value("dpl_",11,hex_a);
                output_set_indexed_value("dpl_",17,hex_b);
                break;
        case 11 :
                output_set_indexed_value("dpl_",10,hex_a);
                output_set_indexed_value("dpl_",16,hex_b);
                break;
        case 12 :
                output_set_indexed_value("dpl_",9,hex_a);
                output_set_indexed_value("dpl_",15,hex_b);
                break;
        case 13 :
                output_set_indexed_value("dpl_",8,hex_a);
                output_set_indexed_value("dpl_",14,hex_b);
                break;
        case 14 :
                output_set_indexed_value("dpl_",7,hex_a);
                output_set_indexed_value("dpl_",13,hex_b);
                break;
        case 15 :
                output_set_indexed_value("dpl_",31,hex_a);
                output_set_indexed_value("dpl_",32,hex_b);
                break;
    }

    state->display_block++;
    state->display_block&=0x0f;
}

static WRITE8_HANDLER(lamp_w)
{
    //logerror("lamp_w %02x\n",data);
    //logerror("[%d]= %02x\n",4+offset/4,data);
}

static WRITE8_HANDLER(lamp7_w)
{
    //logerror("[7]= %02x\n",data);
}

static WRITE8_HANDLER(sol_w)
{
    //logerror("sol_w %02x\n",data);
}


static CUSTOM_INPUT( wolfman_replay_hs_r )
{
	int bit_mask = (FPTR)param;

	switch (bit_mask)
	{
		case 0x03:
			return ((input_port_read(field.machine(), "REPLAY") & bit_mask) >> 0);
		case 0x40:
			return ((input_port_read(field.machine(), "REPLAY") & bit_mask) >> 6);
		default:
			logerror("wolfman_replay_hs_r : invalid %02X bit_mask\n",bit_mask);
			return 0;
	}
}


static ADDRESS_MAP_START( peyper_map, AS_PROGRAM, 8 )
//  AM_RANGE(0x0000, 0xffff) AM_NOP
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x5FFF) AM_ROM
	AM_RANGE(0x6000, 0x67FF) AM_RAM //AM_BASE_GENERIC(nvram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( peyper_io, AS_IO, 8 )
//  AM_RANGE(0x0000, 0xffff) AM_NOP
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(sw_r,disp_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(col_w)
//  AM_RANGE(0x04, 0x04) AM_DEVWRITE("ay8910_0", ay8910_address_w)
//  AM_RANGE(0x06, 0x06) AM_DEVWRITE("ay8910_0", ay8910_data_w)
//  AM_RANGE(0x08, 0x08) AM_DEVWRITE("ay8910_1", ay8910_address_w)
//  AM_RANGE(0x0a, 0x0a) AM_DEVWRITE("ay8910_1", ay8910_data_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(sol_w)
	AM_RANGE(0x10, 0x18) AM_WRITE(lamp_w)
	AM_RANGE(0x20, 0x20) AM_READ_PORT("DSW0")
	AM_RANGE(0x24, 0x24) AM_READ_PORT("DSW1")
	AM_RANGE(0x28, 0x28) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x2c, 0x2c) AM_WRITE(lamp7_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( pbsonic_generic )
	/* SYSTEM : port 0x28 (cpl'ed) */
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             // N.C.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             // N.C.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )            // Reset
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )               // Tilt
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )             // Start game - Might also display next screen in "Test Mode"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )              // Small coin (25 pesetas)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )              // Medium coin (50 pesetas but 100 pesetas in 'sonstwar') - never mentionned in the manuals
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )              // Big coin (100 pesetas)

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_START("DSW0")
	PORT_DIPUNKNOWN( 0x80, IP_ACTIVE_LOW )                  // game specific settings
	PORT_DIPNAME( 0x40, 0x00, "Match Feature" )             // Loteria
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      // Reclamo
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda
	PORT_DIPSETTING(    0x18, "A 2/1 B 1/1 C 1/3" )
	PORT_DIPSETTING(    0x00, "A 1/1 B 1/2 C 1/5" )
	PORT_DIPSETTING(    0x08, "A 1/1 B 1/3 C 1/6" )
	PORT_DIPSETTING(    0x10, "A 1/2 B 1/4 C 1/8" )
	PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x03, 0x00, "Replay at / High-score" )    // Puntos/Premios - high-score not mentionned in some manuals - values are game specific
	PORT_DIPSETTING(    0x00, "0k 0k and 0k / 0k" )
	PORT_DIPSETTING(    0x01, "0k 0k and 0k / 0k" )
	PORT_DIPSETTING(    0x02, "0k 0k and 0k / 0k" )
	PORT_DIPSETTING(    0x03, "0k 0k and 0k / 0k" )
INPUT_PORTS_END


/* verified from Z80 code - NO manual found ! */
static INPUT_PORTS_START( odin_dlx )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x6014..0x601b - 0x11 when pressed (code at 0x0190) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x601c..0x6023 - 0x11 when pressed (code at 0x019d) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x6024..0x602b - 0x11 when pressed (code at 0x01af) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x602c..0x6033 - 0x11 when pressed (code at 0x01bf) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x6034..0x603b - 0x11 when pressed (code at 0x01cf) */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
//  PORT_DIPNAME( 0x40, 0x00, "Match Feature" )             // Loteria - code at 0x0aa5 - stored at 0x609e (0x00 YES / 0x01 NO) - then code at 0x0986
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      // Reclamo - code at 0x0ab2 - stored at 0x609d (0x00 ON / 0x01 OFF)
//  PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x0a84 - tables at 0x0b2f (4 * 3) - credits BCD stored at 0x6103
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x0a77 - stored at 0x6099
	PORT_DIPNAME( 0x03, 0x00, "Replay at / High-score" )    // Puntos/Premios - code at 0x0a34 and 0x0a57 - tables at 0x0aff (4 * 9) and 0x0b23 (4 * 3)
	PORT_DIPSETTING(    0x00, "800k 1200k and 8000k / 1400k" )
	PORT_DIPSETTING(    0x01, "900k 1300k and 8000k / 1500k" )
	PORT_DIPSETTING(    0x02, "1000k 1400k and 8000k / 1600k" )
	PORT_DIPSETTING(    0x03, "1100k 1500k and 8000k / 1700k" )

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Speed-up ?" )                // from Visual Pinball - code at 0x0abe - stored at 0x6009 (0x02 YES / 0x04 NO)
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )                        // mostly affects music (samples ?)
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x00, "Clear RAM on Reset" )        // Borrador RAM - code at 0x0ace - range 0x60ff..0x6121 - 0x611f = 0x55 and 0x6120 = 0xaa
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0f, 0x08, "Test Mode" )                 // Visualizacion - code at 0x1542 (similar code in 'solarwap')
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x03, "Displays Replays and Extra Balls" )    // (0x6116++) on player 1, (0x6119++) on player 3 - I can't tell ATM which is which :(
	PORT_DIPSETTING(    0x05, "Displays Coins" )                      // Coin A (0x6106++) on player 1, Coin B (0x6109++) on player 3 and coin C (0x610c++) on player 2
	PORT_DIPSETTING(    0x06, "Displays Hours and Games" )            // hours since boot (0x6110++) on player 1, games started on player 3 (0x6113++)
INPUT_PORTS_END

/* verified from Z80 code - manual found */
static INPUT_PORTS_START( solarwap )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x6065..0x606c - 0x11 when pressed (code at 0x0193) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x606d..0x6074 - 0x11 when pressed (code at 0x01a0) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x6075..0x605c - 0x11 when pressed (code at 0x01b0) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x607d..0x6084 - 0x11 when pressed (code at 0x01c0) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x6085..0x608c - 0x11 when pressed (code at 0x01d0) */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x00, "DSW0-1 Unknown" )            // code at 0x1818 - stored at 0x6097 - manual says "unused"
	PORT_DIPSETTING(    0x80, "00" )
	PORT_DIPSETTING(    0x00, "FF" )
//  PORT_DIPNAME( 0x40, 0x00, "Match Feature" )             // Loteria - code at 0x0a27 - stored at 0x60fc (0x00 YES / 0x01 NO) - then code at 0x0937
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      // Reclamo - code at 0x0a32 - stored at 0x60fb (0x00 ON / 0x01 OFF)
//  PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x0a06 - tables at 0x0ac2 (4 * 3) - credits BCD stored at 0x6162
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x09f9 - stored at 0x60f7
	PORT_DIPNAME( 0x03, 0x00, "Replay at / High-score" )    // Puntos/Premios - code at 0x09b6 and 0x09d9 - tables at 0x0a92 (4 * 9) and 0x0ab6 (4 * 3)
	PORT_DIPSETTING(    0x01, "1900k and 2500k / 3500k" )
	PORT_DIPSETTING(    0x00, "2200k and 2800k / 3500k" )
	PORT_DIPSETTING(    0x02, "2500k and 3100k / 4000k" )
	PORT_DIPSETTING(    0x03, "2800k and 3400k / 4500k" )

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Extra Ball Light" )          // code at 0x0a3c - stored at 0x60ca (0x10 Flashing / 0x20 Always Lit)
	PORT_DIPSETTING(    0x00, "Flashing" )
	PORT_DIPSETTING(    0x80, "Always Lit" )
	PORT_DIPNAME( 0x40, 0x40, "Medium Left Lane" )          // code at 0x0a49 - stored at 0x60fd (when "Lit Value")
	PORT_DIPSETTING(    0x40, "Lit Value" )
	PORT_DIPSETTING(    0x00, "100k when 100k Lit" )
	PORT_DIPNAME( 0x20, 0x00, "Clear RAM on Reset" )        // Borrador RAM - code at 0x0a61 - range 0x615e..0x6180 - 0x617e = 0x55 and 0x617f = 0xaa
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "Background Music" )          // code at 0x0a52 - stored at 0x6010 (0x00 ON / 0xff No Hit)
	PORT_DIPSETTING(    0x10, "When hitting nothing" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f, 0x08, "Test Mode" )                 // Visualizacion - code at 0x0eef
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x03, "Displays Replays and Extra Balls" )    // (0x6175++) on player 1, (0x6178++) on player 3 - I can't tell ATM which is which :(
	PORT_DIPSETTING(    0x05, "Displays Coins" )                      // Coin A (0x6165++) on player 1, Coin B (0x6168++) on player 3 and coin C (0x616b++) on player 2
	PORT_DIPSETTING(    0x06, "Displays Hours and Games" )            // hours since boot (0x616f++) on player 1, games started on player 3 (0x6172++)
INPUT_PORTS_END

/* verified from Z80 code - manual found */
static INPUT_PORTS_START( poleposn )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x604b..0x6052 - 0x11 when pressed (code at 0x019c) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x6053..0x605a - 0x11 when pressed (code at 0x01a9) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x605b..0x6062 - 0x11 when pressed (code at 0x01b9) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x6063..0x606a - 0x11 when pressed (code at 0x01c9) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x606b..0x6072 - 0x11 when pressed (code at 0x01d9) */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
//  PORT_DIPNAME( 0x40, 0x00, "Match Feature" )             // Loteria - code at 0x092f - stored at 0x60f5 (0x00 YES / 0x01 NO) - then code at 0x0859
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      // Reclamo - code at 0x093a - stored at 0x60f4 (0x00 ON / 0x01 OFF)
//  PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x0915 - tables at 0x09ed (4 * 3) - credits BCD stored at 0x618c
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x0901 - stored at 0x60f0
	PORT_DIPNAME( 0x03, 0x00, "Replay at / High-score" )    // Puntos/Premios - code at 0x08b4 and 0x08e1 - tables at 0x09bd (4 * 9) and 0x09e1 (4 * 3)
	PORT_DIPSETTING(    0x00, "1500k 2200k and 8000k / 3000k" )
	PORT_DIPSETTING(    0x01, "1700k 2400k and 8000k / 3200k" )
	PORT_DIPSETTING(    0x02, "2000k 2600k and 8000k / 3500k" )
	PORT_DIPSETTING(    0x03, "2400k 3000k and 8000k / 3900k" )

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Test Mode" )                 // Visualizacion - code at 0x0a4c
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x00, "Clear RAM on Reset" )        // Borrador RAM - code at 0x098c - range 0x6188..0x61aa - 0x61a8 = 0x55 and 0x61a9 = 0xaa
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x03, 0x00, "Double Playfield" )          // Puntos/xBonus - code at 0x0944 - stored +1 at 0x60f9
	PORT_DIPSETTING(    0x00, "When Bonus x3" )
	PORT_DIPSETTING(    0x01, "When Bonus x3 and x6" )
	PORT_DIPSETTING(    0x02, "When Bonus x3, x6 and x9" )
	PORT_DIPSETTING(    0x03, "When Bonus x3, x6, x9 and x12" )
INPUT_PORTS_END

/* verified from Z80 code - manual found but with incorrect "Coinage" and "Replay at / High-score" settings (another revision floating around ?) */
static INPUT_PORTS_START( sonstwar )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x6013..0x601a - 0x11 when pressed (code at 0x0187) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x601b..0x6022 - 0x11 when pressed (code at 0x0194) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x6023..0x602a - 0x11 when pressed (code at 0x01a4) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x602b..0x6032 - 0x11 when pressed (code at 0x01b4) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x6033..0x603a - 0x11 when pressed (code at 0x01c4) */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
//  PORT_DIPNAME( 0x40, 0x00, "Match Feature" )             // Loteria - code at 0x08c3 - stored at 0x6096 (0x00 YES / 0x01 NO) - then code at 0x07cb
//  PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )      // Reclamo - code at 0x09cf - stored at 0x6094 (0x00 ON / 0x01 OFF)
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x08a2 - tables at 0x0961 (4 * 3) - credits BCD stored at 0x617f
	PORT_DIPSETTING(    0x18, "A 3/1 B 1/2 C 1/2" )                   // manual says "A 2/1 C 1/3"
	PORT_DIPSETTING(    0x08, "A 2/1 B 1/3 C 1/3" )                   // manual says "A 1/1 C 1/6"
	PORT_DIPSETTING(    0x10, "A 3/2 B 1/4 C 1/4" )                   // manual says "A 1/2 C 1/8"
	PORT_DIPSETTING(    0x00, "A 1/1 B 1/5 C 1/5" )                   // manual says "A 1/1 C 1/5"
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x0901 - stored at 0x60f0
	PORT_DIPNAME( 0x03, 0x00, "Replay at / High-score" )    // Puntos/Premios - code at 0x0867 and 0x096d - tables at 0x0931 (4 * 9) and 0x0955 (4 * 3)
	PORT_DIPSETTING(    0x00, "1400k 2200k and 9000k / 3300k" )       // manual says "1700k and 2500k / 3600k"
	PORT_DIPSETTING(    0x01, "1700k 2500k and 9000k / 3600k" )       // manual says "2000k and 2800k / 3900k"
	PORT_DIPSETTING(    0x02, "2000k 2800k and 9000k / 3900k" )       // manual says "2300k and 3100k / 4200k"
	PORT_DIPSETTING(    0x03, "2300k 3100k and 9000k / 4200k" )       // manual says "2600k and 3400k / 4500k"

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Clear RAM on Reset" )        // Borrador RAM - code at 0x083c - range 0x6176..0x6194 - 0x6194 = 0x5a and 0x6195 = 0xa5
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x60, 0x00, "Special" )                   // code at 0x08f0 - stored +1 at 0x6098
	PORT_DIPSETTING(    0x00, "1 Attack" )
	PORT_DIPSETTING(    0x01, "2 Attacks" )
	PORT_DIPSETTING(    0x02, "3 Attacks" )
	PORT_DIPSETTING(    0x03, "4 Attacks" )
	PORT_DIPNAME( 0x18, 0x00, "Extra Ball" )                // code at 0x08da - stored +1 at 0x6097
	PORT_DIPSETTING(    0x00, "1 Star" )
	PORT_DIPSETTING(    0x01, "2 Stars" )
	PORT_DIPSETTING(    0x02, "3 Stars" )
	PORT_DIPSETTING(    0x03, "4 Stars" )
	PORT_DIPNAME( 0x04, 0x00, "Test Mode" )                 // Visualizacion - code at 0x0e31
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
INPUT_PORTS_END

/* verified from Z80 code - manual found */
static INPUT_PORTS_START( wolfman )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x602a..0x6031 - 0x11 when pressed (code at 0x0173) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x6032..0x6039 - 0x11 when pressed (code at 0x0180) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x603a..0x6041 - 0x11 when pressed (code at 0x0190) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x6042..0x6049 - 0x11 when pressed (code at 0x01a0) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x604a..0x6051 - 0x11 when pressed (code at 0x01b0) */

	/* COIN2 doesn't give any credits and isn't even tested due to code at 0x01dd : only contents of 0x602f and 0x6031 are tested */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, "Match" )                     // Loteria - code at 0x0a8a - stored at 0x60c2 (0x00 Score / 0x01 Replay) - then code at 0x0e41
	PORT_DIPSETTING(    0x40, "Awards Replay" )                       // Partida
	PORT_DIPSETTING(    0x00, "Doubles Score" )                       // Dobla Tanteo
	PORT_DIPNAME( 0x20, 0x00, "DSW0-3 Unknown" )            // Bola Reclamo - code at 0x0a94 - stored at 0x60c1 (0x00 ON / 0x01 OFF) - then code at 0x072e - sound related ?
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x20, "01" )
//  PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x0a69 - tables at 0x0b30 (4 * 3) - credits BCD stored at 0x6151
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x0a5c - stored at 0x60bd
	PORT_BIT( 0x03, 0x00, IPT_SPECIAL) PORT_CUSTOM(wolfman_replay_hs_r, (void *)0x03)

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPNAME( 0x80, 0x00, "Adjust Replay" )             // Premios por Puntuacion - code at 0x0aa3 - stored at 0x60c4 and 0x60cc (0x00 NO / 0x05 YES)
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Yes ) )
	PORT_BIT( 0x40, 0x00, IPT_SPECIAL) PORT_CUSTOM(wolfman_replay_hs_r, (void *)0x40)
	PORT_DIPNAME( 0x20, 0x00, "Clear RAM on Reset" )        // Borrador RAM - code at 0x0ace - range 0x6141..0x616f - 0x616d = 0x5a and 0x616e = 0xa5
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0f, 0x08, "Test Mode" )                 // Visualizacion - code at 0x0f35
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x03, "Displays Replays and Extra Balls" )    // (0x6164++) on player 1, (0x6167++) on player 3 - I can't tell ATM which is which :(
	PORT_DIPSETTING(    0x05, "Displays Coins" )                      // Coin A (0x6154++) on player 1, Coin B (0x6157++) on player 3 and coin C (0x615a++) on player 2
	PORT_DIPSETTING(    0x06, "Displays Hours and Games" )            // hours since boot (0x615e++) on player 1, games started on player 3 (0x6161++)

	/* Fake port to handle settings via multiple input ports */
	PORT_START("REPLAY")
	PORT_DIPNAME( 0x43, 0x40, "Replay at / High-score" )    // Puntos/Premios - code at 0x09f4 and 0x0a25 - tables at 0x0af4 (4 * 9) and 0x0b24 (4 * 3) or 0x0b18 (1 * 9) and 0x0b21 (1 * 3)
	PORT_DIPSETTING(    0x40, "800k and 1200k and 8000k / 1410k" )    // manual says "800k and 1200k / 1400k"
	PORT_DIPSETTING(    0x41, "1000k and 1400k and 8000k / 1610k" )   // manual says "1000k and 1400k / 1600k"
	PORT_DIPSETTING(    0x42, "1200k and 1600k and 8000k / 1810k" )   // manual says "1200k and 1600k / 1800k"
	PORT_DIPSETTING(    0x43, "1400k and 1800k and 8000k / 2010k" )   // manual says "1400k and 1800k / 2000k"
	PORT_DIPSETTING(    0x00, "2400k and 2800k and 6800k / 3610k" )   // not mentionned in the manual where it is "conenecte" to DSW1-3 ("Clear RAM on Reset")
INPUT_PORTS_END

/* verified from Z80 code - NO manual found ! */
static INPUT_PORTS_START( odisea )
	PORT_INCLUDE( pbsonic_generic )

	/* SYSTEM : port 0x28 (cpl'ed) -> 0x00 in 0x6030..0x6037 - 0x11 when pressed (code at 0x0153) */

	/* SW0 : port 0x00 (after 0x40 written to port 0x01) -> 0x00 in 0x6038..0x603f - 0x11 when pressed (code at 0x0160) */
	/* SW1 : port 0x00 (after 0x41 written to port 0x01) -> 0x00 in 0x6040..0x6047 - 0x11 when pressed (code at 0x0170) */
	/* SW2 : port 0x00 (after 0x42 written to port 0x01) -> 0x00 in 0x6048..0x604f - 0x11 when pressed (code at 0x0180) */
	/* SW3 : port 0x00 (after 0x43 written to port 0x01) -> 0x00 in 0x6050..0x6057 - 0x11 when pressed (code at 0x0190) */

	/* DSW0 : port 0x20 - DSW0-1 is bit 7 ... DSW0-8 is bit 0 */
	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x80, 0x00, "DSW0-1 Unknown" )            // code at 0x0a99 - stored at 0x60c5
	PORT_DIPSETTING(    0x00, "02" )
	PORT_DIPSETTING(    0x80, "01" )
	PORT_DIPNAME( 0x40, 0x40, "Match" )                     // code at 0x0a83 - stored at 0x60c1 (0x00 Score / 0x01 Replay) - then code at 0x0de7
	PORT_DIPSETTING(    0x40, "Awards Replay" )
	PORT_DIPSETTING(    0x00, "Doubles Score" )
	PORT_DIPNAME( 0x20, 0x00, "DSW0-3 Unknown" )            // Bola Reclamo - code at 0x0a8f - stored at 0x60bf - then code at 0x06d8 (similar code in 'wolfman') - sound related ?
	PORT_DIPSETTING(    0x00, "00" )
	PORT_DIPSETTING(    0x20, "01" )
//  PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coinage ) )          // Partidas/Moneda - code at 0x0a62 - tables at 0x0b17 (4 * 3) - credits BCD stored at 0x6173
//  PORT_DIPNAME( 0x04, 0x00, "Balls" )                     // Bolas/Partida - code at 0x0a55 - stored at 0x60bb
	/* Replay at 2400k 2800k and 3200k / High-score 3210k - code at 0x09f3 and 0x0b23 - tables at 0x0aff (1 * 9) and 0x0b08 (1 * 3) */
	PORT_DIPNAME( 0x02, 0x00, "DSW0-7 Unknown" )            // code at 0x0342 - stored at 0x60a7
	PORT_DIPSETTING(    0x00, "14" )
	PORT_DIPSETTING(    0x02, "28" )
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )

	/* DSW1 : port 0x24 - DSW1-1 is bit 7 ... DSW1-8 is bit 0 */
	PORT_START("DSW1")
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x28, 0x28, "Test Mode" )                 // code at 0x0a0b (RAM) - range 0x6161..0x618d - 0x618c = 0x5a and 0x618d = 0xa5 - code at 0x0fc8 (Test)
	PORT_DIPSETTING(    0x28, DEF_STR( Off ) )                        // does NOT clear RAM on reset
	PORT_DIPSETTING(    0x20, "Displays Coins" )                      // Coin A (0x6176++) on player 3 and coin C (0x617c++) on player 1 - also clears RAM on reset
	PORT_DIPSETTING(    0x08, "Displays Games, Replays & EB" )        // games started (0x617f++) on player 2, replays (0x6182++) on player 3 and EB (0x6185++) on player 4 - also clears RAM on reset
//  PORT_DIPSETTING(    0x00, "INVALID" )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
INPUT_PORTS_END


static MACHINE_RESET( peyper )
{
    peyper_state *state = machine.driver_data<peyper_state>();
    state->irq_state = 0;
}

static MACHINE_CONFIG_START( peyper, peyper_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 2500000)
	MCFG_CPU_PROGRAM_MAP(peyper_map)
	MCFG_CPU_IO_MAP(peyper_io)
    MCFG_CPU_PERIODIC_INT(irq0_line_hold, 1250 * 2)

	MCFG_MACHINE_RESET( peyper )

    /* video hardware */
    MCFG_DEFAULT_LAYOUT(layout_peyper)
MACHINE_CONFIG_END


static DRIVER_INIT( peyper )
{
}


/*-------------------------------------------------------------------
/ Night Fever (1979)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Odin De Luxe (1985)
/-------------------------------------------------------------------*/
ROM_START(odin_dlx)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1a.bin", 0x0000, 0x2000, CRC(4fca9bfc) SHA1(05dce75919375d01a306aef385bcaac042243695))
	ROM_LOAD("2a.bin", 0x2000, 0x2000, CRC(46744695) SHA1(fdbd8a93b3e4a9697e77e7d381759829b86fe28b))
ROM_END

/*-------------------------------------------------------------------
/ Solar Wars (1986)
/-------------------------------------------------------------------*/
ROM_START(solarwap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("solarw1c.bin", 0x0000, 0x2000, CRC(aa6bf0cd) SHA1(7332a4b1679841283d846f3e4f1792cb8e9529bf))
	ROM_LOAD("solarw2.bin",  0x2000, 0x2000, CRC(95e2cbb1) SHA1(f9ab3222ca0b9e0796030a7a618847a4e8f77957))
ROM_END

/*-------------------------------------------------------------------
/ Gamatron (1986)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Pole Position (1987)
/-------------------------------------------------------------------*/
ROM_START(poleposn)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("1.bin", 0x0000, 0x2000, CRC(fdd37f6d) SHA1(863fef32ab9b5f3aca51788b6be9373a01fa0698))
	ROM_LOAD("2.bin", 0x2000, 0x2000, CRC(967cb72b) SHA1(adef17018e2caf65b64bbfef72fe159b9704c409))
	ROM_LOAD("3.bin", 0x4000, 0x2000, CRC(461fe9ca) SHA1(01bf35550e2c55995f167293746f355cfd484af1))
ROM_END

/*-------------------------------------------------------------------
/ Star Wars (1987)
/-------------------------------------------------------------------*/
ROM_START(sonstwar)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("sw1.bin", 0x0000, 0x2000, CRC(a2555d92) SHA1(5c82be85bf097e94953d11c0d902763420d64de4))
	ROM_LOAD("sw2.bin", 0x2000, 0x2000, CRC(c2ae34a7) SHA1(0f59242e3aec5da7111e670c4d7cf830d0030597))
	ROM_LOAD("sw3.bin", 0x4000, 0x2000, CRC(aee516d9) SHA1(b50e54d4d5db59e3fb71fb000f9bc5e34ff7de9c))
ROM_END

/*-------------------------------------------------------------------
/ Hang-On (1988)
/-------------------------------------------------------------------*/


/*-------------------------------------------------------------------
/ Odisea Paris-Dakar (1987)
/-------------------------------------------------------------------*/
ROM_START(odisea)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("odiseaa.bin", 0x0000, 0x2000, CRC(29a40242) SHA1(321e8665df424b75112589fc630a438dc6f2f459))
	ROM_LOAD("odiseab.bin", 0x2000, 0x2000, CRC(8bdf7c17) SHA1(7202b4770646fce5b2ba9e3b8ca097a993123b14))
	ROM_LOAD("odiseac.bin", 0x4000, 0x2000, CRC(832dee5e) SHA1(9b87ffd768ab2610f2352adcf22c4a7880de47ab))
ROM_END

/*-------------------------------------------------------------------
/ Wolf Man (1987)
/-------------------------------------------------------------------*/
ROM_START(wolfman)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("memoriaa.bin", 0x0000, 0x2000, CRC(1fec83fe) SHA1(5dc887d0fa00129ae31451c03bfe442f87dd2f54))
	ROM_LOAD("memoriab.bin", 0x2000, 0x2000, CRC(62a1e3ec) SHA1(dc472c7c9d223820f8f1031c92e36890c1fcba7d))
	ROM_LOAD("memoriac.bin", 0x4000, 0x2000, CRC(468f16f0) SHA1(66ce0464d82331cfc0ac1f6fbd871066e4e57262))
ROM_END


GAME( 1985, odin_dlx, 0, peyper,   odin_dlx, peyper,   ROT0, "Sonic",  "Odin De Luxe",          GAME_IS_SKELETON_MECHANICAL)
GAME( 1986, solarwap, 0, peyper,   solarwap, peyper,   ROT0, "Sonic",  "Solar Wars (Sonic)",    GAME_IS_SKELETON_MECHANICAL)
GAME( 1987, poleposn, 0, peyper,   poleposn, peyper,   ROT0, "Sonic",  "Pole Position (Sonic)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1987, sonstwar, 0, peyper,   sonstwar, peyper,   ROT0, "Sonic",  "Star Wars (Sonic)",     GAME_IS_SKELETON_MECHANICAL)

GAME( 1987, wolfman,  0, peyper,   wolfman,  peyper,   ROT0, "Peyper", "Wolf Man",              GAME_IS_SKELETON_MECHANICAL)
GAME( 1987, odisea,   0, peyper,   odisea,   peyper,   ROT0, "Peyper", "Odisea Paris-Dakar",    GAME_IS_SKELETON_MECHANICAL)
