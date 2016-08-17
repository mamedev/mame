// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    Saitek/Scisys Kasparov Stratos Chess Computer

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"

class stratos_state : public driver_device
{
public:
	stratos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			maincpu(*this, "maincpu"),
			nvram(*this, "nvram"),
			bank_8000(*this, "bank_8000"),
			bank_4000(*this, "bank_4000"),
			nvram_bank(*this, "nvram_bank")
	{ }

	required_device<m65c02_device> maincpu;
	required_device<nvram_device> nvram;
	required_memory_bank bank_8000;
	required_memory_bank bank_4000;
	required_memory_bank nvram_bank;

	std::unique_ptr<UINT8[]> nvram_data;
	UINT8 control, led_latch_control;
	UINT32 individual_leds;
	UINT8 latch_AH_red, latch_AH_green, latch_18_red, latch_18_green;

	DECLARE_DRIVER_INIT(stratos);
	DECLARE_WRITE8_MEMBER(p2000_w);
	DECLARE_READ8_MEMBER(p2200_r);
	DECLARE_WRITE8_MEMBER(p2200_w);
	DECLARE_WRITE8_MEMBER(p2400_w);
	DECLARE_READ8_MEMBER(control_r);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(lcd_r);
	DECLARE_WRITE8_MEMBER(lcd_w);

	TIMER_DEVICE_CALLBACK_MEMBER(irq_timer);

	void show_leds();
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	virtual void machine_reset() override;
};

DRIVER_INIT_MEMBER( stratos_state, stratos )
{
	nvram_data = std::make_unique<UINT8[]>(0x2000);
	nvram->set_base(nvram_data.get(), 0x2000);

	bank_8000 ->configure_entries(0, 4, memregion("roms_8000")->base(), 0x8000);
	bank_4000 ->configure_entries(0, 4, memregion("roms_4000")->base(), 0x4000);
	nvram_bank->configure_entries(0, 2, nvram_data.get(),               0x1000);
}

void stratos_state::machine_reset()
{
	control = 0x00;
	led_latch_control = 0x00;
	individual_leds = 0x00000;
	latch_AH_red = 0;
	latch_AH_green = 0;
	latch_18_red = 0;
	latch_18_green = 0;
	bank_8000 ->set_entry(0);
	bank_4000 ->set_entry(0);
	nvram_bank->set_entry(0);
}

void stratos_state::show_leds()
{
	static const char *led_pos[18] = {
		nullptr, nullptr, "gPawn", "gKnight", "gBishop", "gRook", "gQueen", "gKing", nullptr, nullptr, "rPawn", "rKnight", "rBishop", "rRook", "rQueen", "rKing", nullptr, nullptr
	};
	char str_red[64];
	char str_green[64];

	char *pr = str_red;
	char *pg = str_green;

	*pr = *pg = 0;

	for(int i=0; i != 18; i++)
		if(individual_leds & (1 << i)) {
			const char *pos = led_pos[i];
			if(!pos)
				pr += sprintf(pr, " <%d>", i);
			else if(pos[0] == 'r')
				pr += sprintf(pr, " %s", pos+1);
			else
				pg += sprintf(pg, " %s", pos+1);
		}

	// Obviously slightly incorrect
	if(!(led_latch_control & 8)) {
		pr += sprintf(pr, " %c%c", 'A' + latch_AH_red, '1' + latch_18_red);
		pg += sprintf(pg, " %c%c", 'A' + latch_AH_green, '1' + latch_18_green);
	}

	logerror("leds R:%s -- G:%s (%s)\n", str_red, str_green, machine().describe_context());
}

TIMER_DEVICE_CALLBACK_MEMBER(stratos_state::irq_timer)
{
	maincpu->set_input_line(M65C02_IRQ_LINE, HOLD_LINE);
}

UINT32 stratos_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static bool nmi=false;

	if(machine().input().code_pressed(KEYCODE_W)) {
		if(!nmi) {
			maincpu->set_input_line(M65C02_NMI_LINE, PULSE_LINE);
			nmi = true;
		}
	} else
		nmi = false;


	return 0;
}

WRITE8_MEMBER(stratos_state::p2000_w)
{
	led_latch_control = data;

	if(!(data & 0x10))
		latch_18_red = data & 7;
	if(!(data & 0x20))
		latch_18_green = data & 7;
	if(!(data & 0x40))
		latch_AH_red = data & 7;
	if(!(data & 0x80))
		latch_AH_green = data & 7;

	show_leds();
}

READ8_MEMBER(stratos_state::p2200_r)
{
	logerror("p2200_r (%s)\n", machine().describe_context());
	return machine().rand();
}

WRITE8_MEMBER(stratos_state::p2200_w)
{
	logerror("p2200_w %02x -> %02x (%s)\n", data, data^0xff, machine().describe_context());
}

WRITE8_MEMBER(stratos_state::p2400_w)
{
	if(control & 0x20) {
		individual_leds = individual_leds & 0x100ff;
		individual_leds |= (data ^ 0xff) << 8;
		if(!(control & 0x04))
			individual_leds |= 0x20000;
	} else {
		individual_leds = individual_leds & 0x2ff00;
		individual_leds |= data ^ 0xff;
		if(!(control & 0x04))
			individual_leds |= 0x10000;
	}
}

READ8_MEMBER(stratos_state::control_r)
{
	static int xx = 0;
	xx = !xx;
	// [d659/d630]
	// d64e:
	//   2000 = f9
	//   2000 = f7
	//    8fb = (2600) & 20

	// d625: test device?
	//   8fb=00 : (00, 80) (05, 80) (06, 80)
	//   8fb=20 : (03, 20) (06, 20) (04, 20)
	//  { 2000=f9, 2000=f0 | first, test (2600) & second
	// -> 3-bit mask
	//  d518: 0c 0a 10 0e 14 18 0b 08 -> 1b, timing on loop at e788
	//  d520: 02cc 035b 0219 0266 01ad 0166 0300 0432 -> 8d7/8


	// table at f70e (index on d545 somehow):
	// 0c 0d 0e 0f 10 11 01 02

	// table at d545:
	// 00 4800 6800
	// 01 4802 6800
	// 02 4800 6802
	// 03 4801 6800
	// 04 4800 6801
	// 05 4801 6801
	// 06 4c00 6800
	// 07 4800 6c00
	// 08 4800 6c00
	// 09 4c00 6800
	// 0a 4c00 6c00
	// 0b 4c00 6c00
	// 0c 4880 6800
	// 0d 4840 6800
	// 0e 4820 6800
	// 0f 4810 6800
	// 10 4808 6800
	// 11 4804 6800
	// 12 4800 6880
	// 13 4800 6840
	// 14 4800 6820
	// 15 4800 6810
	// 16 4800 6808
	// 17 4800 6804
	// 18 4880 6880
	// 19 4840 6840
	// 1a 4820 6820
	// 1b 4810 6810
	// 1c 4808 6808
	// 1d 4804 6804

	// Power up led test table
	// 1208 Ki Green
	// 1308 Qu
	// 1408 Ro
	// 1508 Bi
	// 1608 Kn
	// 1708 Pa
	// 0727 8
	// 0026 7
	// 0025 6
	// 0024 5
	// 0023 4
	// 0022 3
	// 0021 2
	// 0020 1
	// 0080 A
	// 0081 B
	// 0082 C
	// 0083 D
	// 0084 E
	// 0085 F
	// 0086 G
	// 0087 H
	// 0008 -
	// 0c08 Ki Red
	// 0d08 Qu
	// 0e08 Ro
	// 0f08 Bi
	// 1008 Kn
	// 1108 Pa
	// 0108
	// 0208
	// 0308
	// 0408
	// 0617 8
	// 0016 7
	// 0015 6
	// 0014 5
	// 0013 4
	// 0012 3
	// 0011 2
	// 0010 1
	// 0040 A
	// 0041 B
	// 0042 C
	// 0043 D
	// 0044 E
	// 0045 F
	// 0046 G
	// 0047 H
	// 0009
	// 00c2

	// (20) = difficulty level

	logerror("control_r (%s)\n", machine().describe_context());
	return xx ? 0x20 : 0x00;
}

WRITE8_MEMBER(stratos_state::control_w)
{
	logerror("control_w %02x bank %d (%s)\n", data, data & 3, machine().describe_context());

	control = data;
	bank_8000->set_entry(data & 3);
	bank_4000->set_entry(data & 3);
	nvram_bank->set_entry((data >> 1) & 1);
}


READ8_MEMBER(stratos_state::lcd_r)
{
	return 0x00;
}

WRITE8_MEMBER(stratos_state::lcd_w)
{
	// 08 0b - 00?
	// 04 06 - 05
	// 02 0d - 07
	// 01 00 - 05

	static UINT8 vals[18];
	static int idx = 0;
	if(data == 0)
		idx = 0;
	if(idx/2 >= 18)
		return;
	if(idx & 1)
		vals[idx/2] = (vals[idx/2] & 0xf0) | (data & 0xf);
	else
		vals[idx/2] = (data & 0xf) << 4;

	idx++;
	if(idx == 18*2) {
		logerror("lcd");
		for(auto & val : vals)
			logerror(" %02x", val);
		logerror("\n");
	}
}

static ADDRESS_MAP_START( stratos_mem, AS_PROGRAM, 8, stratos_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_WRITE(p2000_w)
	AM_RANGE(0x2200, 0x2200) AM_READWRITE(p2200_r, p2200_w)
	AM_RANGE(0x2400, 0x2400) AM_WRITE(p2400_w)
	AM_RANGE(0x2600, 0x2600) AM_READWRITE(control_r, control_w)
	AM_RANGE(0x2800, 0x37ff) AM_RAMBANK("nvram_bank")
	AM_RANGE(0x3800, 0x3800) AM_READWRITE(lcd_r, lcd_w)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank_4000")
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank_8000")
ADDRESS_MAP_END

static INPUT_PORTS_START( stratos )
INPUT_PORTS_END

static MACHINE_CONFIG_START( stratos, stratos_state )
	MCFG_CPU_ADD("maincpu", M65C02, 5670000)
	MCFG_CPU_PROGRAM_MAP(stratos_mem)

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(240, 64)
	MCFG_SCREEN_VISIBLE_AREA(0, 239, 0, 63)
	MCFG_SCREEN_UPDATE_DRIVER(stratos_state, screen_update)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", stratos_state, irq_timer, attotime::from_hz(1000))

	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

ROM_START( stratos )
	ROM_REGION(0x20000, "roms_8000", 0)
	ROM_LOAD("w1_728m_u3.u3",  0x0000, 0x8000, CRC(b58a7256) SHA1(75b3a3a65f4ca8d52aa5b17a06319bff59d9014f))
	ROM_LOAD("bw1_918n_u4.u4", 0x8000, 0x8000, CRC(cb0de631) SHA1(f78d40213be21775966cbc832d64acd9b73de632))
	ROM_FILL(0x10000, 0x10000, 0xff)

	ROM_REGION(0x10000, "roms_4000", 0)
	ROM_FILL(0x00000, 0x10000, 0xff)
ROM_END

/*     YEAR  NAME      PARENT   COMPAT  MACHINE    INPUT     CLASS          INIT     COMPANY    FULLNAME                           FLAGS */
CONS(  1986, stratos,  0,       0,      stratos,   stratos,  stratos_state, stratos, "Saitek",  "Kasparov Stratos Chess Computer", MACHINE_NO_SOUND)
