/**************************************************************************

  PINBALL
  Allied Leisure Cocktail Pinball
  All tables use the same base roms and some playfields even interchange
  between games.

  6504 CPU, 3x R6530 RRIOT, 5x R6520 PIA
  It is assumed that the R6530 is the same as MOS6530, and the R6520 is
  the same as MC6821.

  The schematic is too blurry to make out much detail, so used PinMAME
  for the PIA connections.

  The display units use 74164 serial units, with clock and data lines
  sufficient for all the digits of one player. Data = IC2,PB7, while
  Clock is IC2,CB2. To prevent display garbage, the new data is stored
  while IC4PBx is high, then displayed when the line goes low. IC7portB
  selects which player's display to update.

  For some reason the 'rol $46' instruction outputs the original data
  followed by the new result, so I've had to employ a horrible hack.

***************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6502/m6504.h"
#include "machine/mos6530.h"
#include "machine/6821pia.h"
#include "allied.lh"

class allied_state : public genpin_class
{
public:
	allied_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_DRIVER_INIT(allied);
	DECLARE_WRITE8_MEMBER(ic2_b_w);
	DECLARE_WRITE_LINE_MEMBER(ic2_cb2_w);
	DECLARE_WRITE8_MEMBER(ic3_b_w);
	DECLARE_WRITE8_MEMBER(ic4_b_w);
	DECLARE_WRITE_LINE_MEMBER(ic4_cb2_w);
	DECLARE_WRITE8_MEMBER(ic5_b_w);
	DECLARE_WRITE8_MEMBER(ic6_b_w);
	DECLARE_WRITE8_MEMBER(ic7_b_w);
	DECLARE_WRITE8_MEMBER(ic8_a_w);
	DECLARE_WRITE8_MEMBER(ic8_b_w);
private:
	UINT32 m_player_score[6];
	UINT8 m_display;
	UINT8 m_bit_counter;
	bool m_disp_data;
	virtual void machine_reset();
	required_device<m6504_device> m_maincpu;
};


static ADDRESS_MAP_START( allied_map, AS_PROGRAM, 8, allied_state )
	AM_RANGE(0x0000, 0x003f) AM_RAM // ic6
	AM_RANGE(0x0044, 0x0047) AM_DEVREADWRITE("ic2", pia6821_device, read, write)
	AM_RANGE(0x0048, 0x004b) AM_DEVREADWRITE("ic1", pia6821_device, read, write)
	AM_RANGE(0x0050, 0x0053) AM_DEVREADWRITE("ic7", pia6821_device, read, write)
	AM_RANGE(0x0060, 0x0063) AM_DEVREADWRITE("ic4", pia6821_device, read, write)
	AM_RANGE(0x0080, 0x008f) AM_DEVREADWRITE("ic5", mos6530_device, read, write)
	AM_RANGE(0x0840, 0x084f) AM_DEVREADWRITE("ic6", mos6530_device, read, write)
	AM_RANGE(0x00c0, 0x00c3) AM_DEVREADWRITE("ic8", pia6821_device, read, write)
	AM_RANGE(0x0100, 0x013f) AM_RAM // ic5
	AM_RANGE(0x1400, 0x1fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( allied )
INPUT_PORTS_END

WRITE8_MEMBER( allied_state::ic2_b_w )
{
	//printf("%s:IC2B:%X ",machine().describe_context(),data);
	m_disp_data = !BIT(data, 7);
}

WRITE_LINE_MEMBER( allied_state::ic2_cb2_w )
{
	if ((m_display) && (!state))
	{
		m_bit_counter++;
		if BIT(m_bit_counter, 0)
			m_player_score[m_display-1] = (m_player_score[m_display-1] << 1) | m_disp_data;
		if (m_bit_counter == 15)
			m_bit_counter = 0;
	}
}

WRITE8_MEMBER( allied_state::ic3_b_w )
{
	m_maincpu->set_input_line(M6504_IRQ_LINE, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE );
}

WRITE8_MEMBER( allied_state::ic4_b_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7446A
	UINT8 segment, i;
	//printf("%s:IC4B:%X ",machine().describe_context(),data);
	for (i = 0; i < 4; i++)
	{
		if (!BIT(data, i+4))
		{
			output_set_digit_value(i*10, patterns[0]);
			segment = (m_player_score[i] >> 0) & 15;
			output_set_digit_value(i*10+1, patterns[segment]);
			segment = (m_player_score[i] >> 4) & 15;
			output_set_digit_value(i*10+2, patterns[segment]);
			segment = (m_player_score[i] >> 8) & 15;
			output_set_digit_value(i*10+3, patterns[segment]);
			segment = (m_player_score[i] >> 12) & 15;
			output_set_digit_value(i*10+4, patterns[segment]);
			segment = (m_player_score[i] >> 16) & 15;
			output_set_digit_value(i*10+5, patterns[segment]);
		}
	}
}

WRITE_LINE_MEMBER( allied_state::ic4_cb2_w )
{
}

// cabinet solenoids
WRITE8_MEMBER( allied_state::ic5_b_w )
{
	if (!BIT(data, 2)) // chime C
		m_samples->start(1, 1);

	if (!BIT(data, 3)) // chime B
		m_samples->start(2, 2);

	if (!BIT(data, 4)) // chime A
		m_samples->start(3, 3); // tens have highest tone

	if (!BIT(data, 5)) // knocker
		m_samples->start(0, 6);

	m_maincpu->set_input_line(M6504_IRQ_LINE, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE );
}

WRITE8_MEMBER( allied_state::ic6_b_w )
{
	m_maincpu->set_input_line(M6504_IRQ_LINE, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE );
}

WRITE8_MEMBER( allied_state::ic7_b_w )
{
	//if (m_display) printf(" %X=%X",m_display,m_player_score[m_display-1]>>1);
	//printf("%s:IC7B:%X ",machine().describe_context(),data);
	m_display = data >> 4;
	if (m_display > 5)
		m_display = 0;
	m_bit_counter = 0;
}

// playfield solenoids
WRITE8_MEMBER( allied_state::ic8_a_w )
{
	if ((data & 0x07) < 0x07) // 3 bumpers
		m_samples->start(0, 0);

	if ((data & 0x60) < 0x60) // slings
		m_samples->start(0, 7);

	if (!BIT(data, 7)) // outhole
		m_samples->start(0, 5);
}

// PB0-4 = ball 1-5 LED; PB5 = shoot again lamp
WRITE8_MEMBER( allied_state::ic8_b_w )
{
	//printf("%s:IC8B:%X ",machine().describe_context(),data);
}

void allied_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(allied_state,allied)
{
}

static MACHINE_CONFIG_START( allied, allied_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6504, 3572549/4)
	MCFG_CPU_PROGRAM_MAP(allied_map)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_allied)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Devices */
	MCFG_DEVICE_ADD("ic1", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(allied_state, ic1_a_r))
	//MCFG_PIA_WRITEPA_HANDLER(WRITE8(allied_state, ic1_a_w))
	//MCFG_PIA_READPB_HANDLER(READ8(allied_state, ic1_b_r))
	//MCFG_PIA_WRITEPB_HANDLER(WRITE8(allied_state, ic1_b_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(allied_state, ic1_ca2_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(allied_state, ic1_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6504_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6504_device, irq_line))

	MCFG_DEVICE_ADD("ic2", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(allied_state, ic2_a_r))
	//MCFG_PIA_WRITEPA_HANDLER(WRITE8(allied_state, ic2_a_w))
	//MCFG_PIA_READPB_HANDLER(READ8(allied_state, ic2_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(allied_state, ic2_b_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(allied_state, ic2_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(allied_state, ic2_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6504_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6504_device, irq_line))

	MCFG_DEVICE_ADD("ic4", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(allied_state, ic4_a_r))
	//MCFG_PIA_WRITEPA_HANDLER(WRITE8(allied_state, ic4_a_w))
	//MCFG_PIA_READPB_HANDLER(READ8(allied_state, ic4_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(allied_state, ic4_b_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(allied_state, ic4_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(allied_state, ic4_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6504_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6504_device, irq_line))

	MCFG_DEVICE_ADD("ic7", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(allied_state, ic7_a_r))
	//MCFG_PIA_WRITEPA_HANDLER(WRITE8(allied_state, ic7_a_w))
	//MCFG_PIA_READPB_HANDLER(READ8(allied_state, ic7_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(allied_state, ic7_b_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(allied_state, ic7_ca2_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(allied_state, ic7_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6504_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6504_device, irq_line))

	MCFG_DEVICE_ADD("ic8", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(allied_state, ic8_a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(allied_state, ic8_a_w))
	//MCFG_PIA_READPB_HANDLER(READ8(allied_state, ic8_b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(allied_state, ic8_b_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(allied_state, ic8_ca2_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(allied_state, ic8_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6504_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6504_device, irq_line))

	MCFG_DEVICE_ADD("ic3", MOS6530, 3572549/4) // unknown where the ram and i/o is located
	MCFG_MOS6530_OUT_PB_CB(WRITE8(allied_state, ic3_b_w))

	MCFG_DEVICE_ADD("ic5", MOS6530, 3572549/4)
	//MCFG_MOS6530_IN_PA_CB(READ8(allied_state, ic5_a_r))
	//MCFG_MOS6530_OUT_PA_CB(WRITE8(allied_state, ic5_a_w))
	//MCFG_MOS6530_IN_PB_CB(READ8(allied_state, ic5_b_r))
	MCFG_MOS6530_OUT_PB_CB(WRITE8(allied_state, ic5_b_w))

	MCFG_DEVICE_ADD("ic6", MOS6530, 3572549/4)
	//MCFG_MOS6530_IN_PA_CB(READ8(allied_state, ic6_a_r))
	//MCFG_MOS6530_OUT_PA_CB(WRITE8(allied_state, ic6_a_w))
	//MCFG_MOS6530_IN_PB_CB(READ8(allied_state, ic6_b_r))
	MCFG_MOS6530_OUT_PB_CB(WRITE8(allied_state, ic6_b_w))
MACHINE_CONFIG_END


ROM_START( allied )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "r6530-009.u5", 0x1400, 0x0400, CRC(e4fb64fb) SHA1(a3d9de7cbfb42180a860e0bbbeaeba96d8bd1e20))
	ROM_LOAD( "r6530-010.u6", 0x1800, 0x0400, CRC(dca980dd) SHA1(3817d75413854d889fc1ce4fd6a51d820d1e0534))
	ROM_LOAD( "r6530-011.u3", 0x1c00, 0x0400, CRC(13f42789) SHA1(baa0f73fda08a3c5d6f1423fb329e4febb07ef97))
ROM_END

#define rom_suprpick    rom_allied
#define rom_royclark    rom_allied
#define rom_thndbolt    rom_allied
#define rom_hoedown     rom_allied
#define rom_takefive    rom_allied
#define rom_heartspd    rom_allied
#define rom_foathens    rom_allied
#define rom_disco79     rom_allied
#define rom_erosone     rom_allied
#define rom_circa33     rom_allied
#define rom_starshot    rom_allied


GAME(1977,  allied,     0,          allied, allied, allied_state, allied, ROT0, "Allied Leisure",               "Allied System",                GAME_IS_BIOS_ROOT)
GAME(1977,  suprpick,   allied,     allied, allied, allied_state,   allied, ROT0,   "Allied Leisure",               "Super Picker",                 GAME_IS_SKELETON_MECHANICAL)
GAME(1977,  royclark,   allied,     allied, allied, allied_state,   allied, ROT0,   "Fascination Int.",             "Roy Clark - The Entertainer",  GAME_IS_SKELETON_MECHANICAL)
GAME(1977,  thndbolt,   allied,     allied, allied, allied_state,   allied, ROT0,   "Allied Leisure",               "Thunderbolt",                  GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  hoedown,    allied,     allied, allied, allied_state,   allied, ROT0,   "Allied Leisure",               "Hoe Down",                     GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  takefive,   allied,     allied, allied, allied_state,   allied, ROT0,   "Allied Leisure",               "Take Five",                    GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  heartspd,   allied,     allied, allied, allied_state,   allied, ROT0,   "Allied Leisure",               "Hearts & Spades",              GAME_IS_SKELETON_MECHANICAL)
GAME(1978,  foathens,   allied,     allied, allied, allied_state,   allied, ROT0,   "Allied Leisure",               "Flame of Athens",              GAME_IS_SKELETON_MECHANICAL)
GAME(1979,  disco79,    allied,     allied, allied, allied_state,   allied, ROT0,   "Allied Leisure",               "Disco '79",                    GAME_IS_SKELETON_MECHANICAL)
GAME(1979,  erosone,    allied,     allied, allied, allied_state,   allied, ROT0,   "Fascination Int.",             "Eros One",                     GAME_IS_SKELETON_MECHANICAL)
GAME(1979,  circa33,    allied,     allied, allied, allied_state,   allied, ROT0,   "Fascination Int.",             "Circa 1933",                   GAME_IS_SKELETON_MECHANICAL)
GAME(1979,  starshot,   allied,     allied, allied, allied_state,   allied, ROT0,   "Allied Leisure",               "Star Shooter",                 GAME_IS_SKELETON_MECHANICAL)
