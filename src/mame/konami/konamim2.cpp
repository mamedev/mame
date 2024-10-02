// license:BSD-3-Clause
// copyright-holders:Phil Bennett

/***************************************************************************

    Konami M2 hardware

    driver by Phil Bennett

    TODO:
    * Fix Heat of Eleven '98 soft-lock when selecting Japan as a team
    * Fix incorrect speed in Tobe! Polystars
    * Fix texture compression
    * Sort out CD images
    * Fix PowerPC 602 Protection Only mode handling.

    DONE
    * Fix Polystars blending
    * Fix missing music in Polystars
    * Fix music playing too early
    * Fix missing music and sound in Hell Night/Evil Night
    * Fix incorrect speed in Heat of 11 and Total Vice (partially)

    // Polystars/Total Vice
    if (pc == 0x40035958)
        gpr[11] = 1;

    // Everything else
    if (pc == 0x400385c8)
        gpr[11] = 0;




Konami M2 Hardware Overview
Konami, 1997-1998

This hardware is 3DO-based with two IBM Power PC CPUs.

There were only 5 known games on this hardware. They include....

Game                                                 Year    CD Codes                                  Konami Part#
-------------------------------------------------------------------------------------------------------------------
Battle Tryst                                         1998    636JAC02
Evil Night                                           1998    810UBA02
Hell Night (alt. Region title, same as Evil Night)   1998    810EAA02
Heat Of Eleven '98                                   1998    703EAA02
Tobe! Polystars                                      1997    623JAA02                                  003894
Total Vice                                           1997    639UAC01, 639EAD01, 639JAD01, 639AAB01


PCB Layouts
-----------

Top Board

[M]DFUP0882ZAM1
FZ-20B1AK 7BKSA03500 (sticker)
|---------------------------------------------------|
|            |--------------------|    |----------| |
|            |--------------------|    |----------| |
|    2902             |---|  |--------|             |
| AK4309 CY2292S|---| |*2 |  |  3DO   |  |-------|  |
|               |*1 | |---|  |        |  |IBM    |  |
|        18MHz  |---|        |        |  |POWERPC|  |
|                            |        |  |602    |  |
|                            |--------|  |-------|  |
|    D4516161  D4516161                             |
|                                 |---|  |-------|  |
|DSW                    |-------| |*3 |  |IBM    |  |
|                       |       | |---|  |POWERPC|  |
|    D4516161  D4516161 |  *4   |        |602    |  |
|                       |       |        |-------|  |
|                       |-------|                   |
|---------------------------------------------------|
Notes:
      AK4309  - Asahi Kasei Microsystems AK4309-VM Digital to Analog Converter (SOIC24)
      2902    - Japan Radio Co. JRC2902 Quad Operational Amplifier (SOIC14)
      CY2292S - Cypress CY2292S Three-PLL General-Purpose EPROM Programmable Clock Generator (SOIC16)
                XTALIN - 18.000MHz, XTALOUT - 18.000MHz, XBUF - 18.000MHz, CPUCLK - 25.2000MHz
                CLKA - , CLKB -  , CLKC - 16.9345MHz, CLKD -
      3DO     - 9701 B861131 VY21118- CDE2 3DO 02473-001-0F (QFP208)
      *1      - [M] JAPAN ASUKA 9651HX001 044 (QFP44)
      *2      - Motorola MC44200FT
      *3      - [M] BIG BODY 2 BU6244KS 704 157 (QFP56)
      *4      - Unknown BGA chip (Graphics Engine, with heatsink attached)
      DSW     - 2 position DIP switch


Bottom Board

PWB403045B (C) 1997 KONAMI CO., LTD.
|----------------------------------------------------------|
|           CN16    |--------------------|    |----------| |
|LA4705             |--------------------|    |----------| |
|       NJM5532D                    9.83MHz                |
|                                   19.66MHz               |
|J                |--------|   93C46.7K                    |-|
|A                | 058232 |                BOOTROM.8Q     | |
|M                |--------|   |------|                    | |
|M       |------|              |003461|                    | |
|A       |056879|              |      |                    | |CN15
|        |      |              |------|                    | |
| TEST   |------|                                          | |
|                                                          | |
|   DSW                                                    | |
|                                                          |-|
|                                                          |
|----------------------------------------------------------|
Notes:
      056879     - Konami custom IC, location 10E (QFP120)
      058232     - Konami custom ceramic flat pack IC, DAC?
      003461     - Konami custom IC, location 11K (QFP100)
      CN16       - 4 pin connector for CD-DA in from CDROM
      CN15       - Standard (PC-compatible) 40 pin IDE CDROM flat cable connector and 4 pin power plug connector,
                   connected to Panasonic CR-583 8-speed CDROM drive.
      LA4705     - LA4705 Power Amplifier
      DSW        - 8 position dip switch
      BOOTROM.8Q - 16MBit MASKROM. Location 8Q (DIP42)
                   Battle Tryst       - 636A01.8Q
                   Evil Night         -       .8Q
                   Heat Of Eleven '98 -       .8Q
                   Polystars          - 623B01.8Q
                   Total Vice         -       .8Q
      93C46.7K   - 128bytes x8bit Serial EEPROM. Location 7K (DIP8)
                   NOTE! There is very mild protection to stop game-swapping. It is based on the information in the EEPROM
                   being the same as the Time Keeper NVRAM.
                   For example, in Evil Night, the first line of the NVRAM in hex is 474E38313000000019984541410002A601FEFE01
                   Looking at it in ascii:  GN810.....EAA.......
                   Hex 474E383130 = GN810
                   1998 = the year of the game
                   Hex 454141 = EAA (the version = europe english)
                   The numbers after this appear to be unimportant (at least with regards to swapping games anyway).
                   All the other data after the first line is used for high scores tables etc.
                   The important part is that the data in the EEPROM should be the same as the NVRAM, but the EEPROM data
                   is byte-swapped! If the two don't match, the check on 7K or the NVRAM will fail and the PCB will reboot
                   forever.

Some lower boards have two connectors underneath for a protection sub-board or sound board. These are detailed below....

GX636-PWB(A) (C) 1997 KONAMI CO., LTD.
|-------------------------|
| CN4 CN3  |---------|    |
|          |---------|CN2 |
|          PAL            |
|                         |
|             NVRAM       |
|                         |
|          |---------|    |
|          |---------|CN1 |
|-------------------------|
Notes:
      NVRAM  - With Heat of Eleven '98, uses Dallas DS1643 NonVolatile TimeKeeping RAM
               With Battle Tryst, uses ST M48T58Y-70PC1 NonVolatile TimeKeeping RAM
               With Poly Stars, a sub board is not used at all
      PAL    - PALCE16V8Q, stamped 'X636A1'
      CN3    - 4-pin sound cable tied to CN16 (CD-DA Input) on main lower board
      CN4    - 4-pin sound cable tied to CDROM analog audio output connector

GQ639 PWB 403327(A)
|-----------------------------------------|
|       639JAA02.xx                       |
|                                         |
|                                         |
|                   |---------|           |
|                   |---------|           |
|                                         |
|                                         |
|               PAL                       |
|                                         |
|                                         |
|                                         |
|                                         |
|                   |---------|           |
|      YMZ280B      |---------|           |
|                                         |
|      16.9344MHz                         |
|                                         |
|                                         |
|-----------------------------------------|
Notes:
      This PCB is used on Total Vice only.
      639JAA02.xx - 8MBit Sound data ROM (DIP42)
      PAL         - PAL16V8H stampd '       '


PWB0000047043 (C) 1998 KONAMI CO., LTD.
|-----------------------------------------|
| CN4     CN3                             |
|                                         |
|                                         |
|                   |---------|           |
|                   |---------|           |
|        16.9344MHz              M48T58Y  |
|                      PAL                |
|          YMZ280B                        |
|                                         |
|                                         |
|                                         |
|                                         |
|                   |---------|           |
|                   |---------|           |
|                                         |
|                                         |
|                                         |
|              810A03.16H                 |
|-----------------------------------------|
Notes:
      This PCB is used on Evil Night/Hell Night only.
      810A03.16H - 16MBit Sound data ROM (DIP42, byte mode)
      PAL        - PAL16V8H stamped 'N810B1'
      M48T58Y    - ST M48T58Y-70PC1 NonVolatile TimeKeeping RAM
      CN3        - 4-pin sound cable tied to CN16 (CD-DA Input) on main lower board
      CN4        - 4-pin sound cable tied to CDROM analog audio output connector

***************************************************************************/

#include "emu.h"
#include "3dom2.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/cr589.h"
#include "cpu/powerpc/ppc.h"
#include "machine/eepromser.h"
#include "machine/timekpr.h"
#include "sound/dac.h"
#include "sound/ymz280b.h"

#include "debug/debugcon.h"
#include "debugger.h"
#include "screen.h"
#include "speaker.h"

#include "cdrom.h"


namespace {

#define M2_CLOCK        XTAL(66'666'700)

#define ENABLE_SDBG     0


/*************************************
 *
 *  driver state class
 *
 *************************************/

class konamim2_state : public driver_device
{
public:
	konamim2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ppc1(*this, "ppc1"),
		m_ppc2(*this, "ppc2"),
		m_bda(*this, "bda"),
		m_cde(*this, "cde"),
		m_eeprom(*this, "eeprom"),
		m_ldac(*this, "ldac"),
		m_rdac(*this, "rdac"),
		m_ata(*this, "ata"),
		m_screen(*this, "screen"),
		m_m48t58(*this, "m48t58"),
		m_ymz280b(*this, "ymz")
	{
	}

	void konamim2(machine_config &config);
	void set_ntsc(machine_config &config);
	void set_ntsc2(machine_config &config);
	void set_arcres(machine_config &config);
	void add_ymz280b(machine_config &config);
	void add_mt48t58(machine_config &config);
	void polystar(machine_config &config);
	void totlvice(machine_config &config);
	void btltryst(machine_config &config);
	void heatof11(machine_config &config);
	void evilngt(machine_config &config);
	void hellngt(machine_config &config);

	static void cr589_config(device_t *device);

	void m2_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

public:
	void ppc1_int(int state);
	void ppc2_int(int state);

	void cde_sdbg_out(uint32_t data);

	void ldac_out(uint16_t data);
	void rdac_out(uint16_t data);

	void ata_int(int state);

	uint16_t konami_io0_r(offs_t offset);
	void konami_io0_w(offs_t offset, uint16_t data);
	uint16_t konami_sio_r(offs_t offset, uint16_t mem_mask = ~0);
	void konami_sio_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t konami_io1_r(offs_t offset);
	void konami_io1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void konami_eeprom_w(uint16_t data);

	void init_totlvice();
	void init_btltryst();
	void init_hellngt();

	void konami_atapi_unk_w(uint16_t data)
	{
		// 8000 = /Reset
		// 4000 = C000 ... DOIO DMA ... 4000
//      m_ata->write_dmack(data & 0x4000 ? ASSERT_LINE : CLEAR_LINE);

		if (!(data & 0x8000))
		{
			logerror("ATAPI RESET!\n");
		}
	}

private:
	void install_ymz280b();
	void install_m48t58();

	required_device<ppc602_device> m_ppc1;
	optional_device<ppc602_device> m_ppc2;
	required_device<m2_bda_device> m_bda;
	required_device<m2_cde_device> m_cde;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	required_device<ata_interface_device> m_ata;
	required_device<screen_device> m_screen;

	optional_device<m48t58_device> m_m48t58;
	optional_device<ymz280b_device> m_ymz280b;

	// Konami SIO
	uint16_t    m_sio_data = 0;

	uint32_t    m_ata_int = 0; // TEST
	emu_timer *m_atapi_timer = nullptr;

	TIMER_CALLBACK_MEMBER( atapi_delay )
	{
		m_atapi_timer->adjust( attotime::never );
		m_ata_int = param;
	}

	void debug_help_command(const std::vector<std::string_view> &params);
	void debug_commands(const std::vector<std::string_view> &params);

	void dump_task_command(const std::vector<std::string_view> &params);
};



/*************************************
 *
 *  Trampolines - Remove ME
 *
 *************************************/

void konamim2_state::ppc1_int(int state)
{
	m_ppc1->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}

void konamim2_state::ppc2_int(int state)
{
	m_ppc2->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}

void konamim2_state::cde_sdbg_out(uint32_t data)
{
	if (data == 0xd)
		putc('\n', stdout);
	else if (data != 0)
		putc(data, stdout);

	fflush(stdout);

#if ENABLE_SDBG
	// Dummy write to enable serial out
	if (data == 0x3c)
		m_cde->sdbg_in(space, 0, 0xffffffff);
#endif
}

void konamim2_state::ldac_out(uint16_t data)
{
	m_ldac->write(data);
}

void konamim2_state::rdac_out(uint16_t data)
{
	m_rdac->write(data);
}


/*************************************
 *
 *  ATAPI (Temporary - remove)
 *
 *************************************/

void konamim2_state::ata_int(int state)
{
//  m_atapi_timer->adjust( attotime::from_msec(10), state );
	m_ata_int = state;
}


/*************************************
 *
 *  Konami I/O
 *
 *************************************/

uint16_t konamim2_state::konami_io0_r(offs_t offset)
{
//  printf("IO R: %08X\n", offset);

	switch (offset)
	{
		/*
		    0 =    160
		    1 =     32
		    2 =    -96
		   10 =  -1888
		   FF = -32480

		  100 =    160
		  1FF = -32480

		  200 =    159
		  2FF = -32481

		  300 =    159
		  3FF = -32481

		  400 =    158

		  800 =    156

		  C00 =    154

		  E00
		  F00 =    153

		 1000 =    152
		 2000 =    144
		 4000 =    128
		 8000 =     96

		 E000 =     48
		 F000 =     40
		 FF00 =     33

		 7FFF = -32543
		 FFFF = -32607
		 */
		case 0:
		{
			return swapendian_int16((int16_t)ioport("GUNX1")->read());
		}
		case 1: return 0;
		case 2: return 0; // P3 X?
		case 3: return 0xffff; // ?
		case 4:
		{
			return swapendian_int16((int16_t)ioport("GUNY1")->read());
		}
		case 5: return 0; // P2 Y
		case 6: return 0; // P3 Y?
		case 7: return 0; //??
		case 8: return ioport("P5")->read();
	}

	//return machine().rand();
	return 0;
}

void konamim2_state::konami_io0_w(offs_t offset, uint16_t data)
{
	// 9: 0000, 0xFFF
//  printf("IO W: %08x %08x\n", offset, data);
}

/*
     FEDCBA98 76543210
 0: |........ ........|

 1: |........ ........|

 2: |........ ........|

 3: |........ .......x| Coin 1
    |........ ......x.| Coin 2
    |........ ....x...| Service coin
    |.......x ........| EEPROM D0
    |.....x.. ........| ADC
    |...xx... ........| SIO related? (set to 3 NOTE: Total Vice doesn't like this!)
    |..x..... ........| ATAPI/CD status?

 4: |........ ........| Player 1/2 inputs

 5: |........ ........| Unknown

 6: |........ ........| Unknown

 7: |........ ........| Unknown
*/

uint16_t konamim2_state::konami_io1_r(offs_t offset)
{
	uint16_t data = 0;

//  printf("%s: PORT R: [%x] MASK:%.8x\n", machine().describe_context(), offset, mem_mask);

	switch (offset)
	{
		case 0:
			data = 0xffff;
			break;

		case 1:
			data = 0xffff;
			break;

		case 2: // DIP switches
			data = ioport("P2")->read();
			break;

		case 3:
		{
			data = ioport("P1")->read();
#if M2_BAD_TIMING
			static uint32_t d = 0;
			data |= d;
			d ^= 0x2000;
#else
			data |= m_ata_int ? 0x2000 : 0;
#endif
			data |= (1 << 10);
			data |= (3 << 11); // TODO: 3 normally
			break;
		}

		case 4: // Buttons
			data = ioport("P4")->read();
			break;

		case 5: // Changing this has a tendency to stop evilngt from booting...
			data = 0xffff;
			break;

		case 6: // Buttons
			data = ioport("P6")->read();
			break;

		case 7:
			data = 0xffff;
			break;

		default:
			logerror("%s: Unknown read: %x\n", machine().describe_context(), offset);
			break;
	}

	return data;
}

void konamim2_state::konami_io1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// 0x0200 = ADC?
	// 0x0800 = Coin counter 1
	// 0x1000 = Coin counter 2
	// 0x2000 = CD-MUTE
	// 0x8000 = ?
	logerror("%s: PORT W: [%x] %x, MASK:%.8x\n", machine().describe_context(), offset, data, mem_mask);

//  printf("CDDA is: %s\n", data & 0x2000 ? "ENABLED" : "MUTE");

	machine().bookkeeping().coin_counter_w(0, (data >> 11) & 1);
	machine().bookkeeping().coin_counter_w(1, (data >> 12) & 1);

//  m_cdda->set_output_gain(0, data & 0x2000 ? 1.0 : 0.0);
//  m_cdda->set_output_gain(1, data & 0x2000 ? 1.0 : 0.0);
}



/**************************************
 *
 *  Konami 003461 SIO
 *
 *************************************/

/*
     FEDCBA98 76543210
 0: |xxxxxxxx xxxxxxxx| Data R/W

 1: |........ ........|

 2: |........ ........|

 3: |........ ........|

 4: |........ ........|

 5: |........ ..x.....| Transmit ready

 6: |........ ........|

 7: |xxxxxxxx xxxxxxxx| Register R/W
*/

uint16_t konamim2_state::konami_sio_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	switch (offset)
	{
		case 7:
			data = m_sio_data;
			break;
		//default:
			//logerror("%s: SIO_R: %x %x\n", machine().describe_context(), offset, mem_mask);
	}

	return data;
}

void konamim2_state::konami_sio_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
		case 7:
			m_sio_data = data;
			break;
		//default:
			//printf("%s: SIO_W: %x %x %x\n", machine().describe_context(), offset, data, mem_mask);
	}
}

// TODO: Use output port
void konamim2_state::konami_eeprom_w(uint16_t data)
{
	// 3 = CS
	// 2 = CLK
	// 1 = DATA
	// 0 = ? (From Port)
	m_eeprom->cs_write(data & 0x80 ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->di_write(data & 0x20 ? 1 : 0);
	m_eeprom->clk_write(data & 0x40 ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void konamim2_state::machine_start()
{
	m_ppc1->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);
	m_ppc2->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	m_ppc1->ppcdrc_add_fastram(m_bda->ram_start(), m_bda->ram_end(), false, m_bda->ram_ptr());
	m_ppc2->ppcdrc_add_fastram(m_bda->ram_start(), m_bda->ram_end(), false, m_bda->ram_ptr());

	// TODO: REMOVE
	m_atapi_timer = timer_alloc(FUNC(konamim2_state::atapi_delay), this);
	m_atapi_timer->adjust( attotime::never );

	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("m2", CMDFLAG_NONE, 1, 4, std::bind(&konamim2_state::debug_commands, this, _1));
	}
}

/*************************************
 *
 *  Address map
 *
 *************************************/

void konamim2_state::m2_map(address_map &map)
{
	map(0x20000000, 0x201fffff).rom().region("boot", 0); // BIOBUS Slot 0
	map(0xfff00000, 0xffffffff).rom().region("boot", 0);
	map(0x37400000, 0x37400007).w(FUNC(konamim2_state::konami_eeprom_w)).umask64(0xffff000000000000ULL);
	map(0x37600000, 0x3760000f).w(FUNC(konamim2_state::konami_atapi_unk_w)).umask64(0xffff000000000000ULL);
	map(0x37a00020, 0x37a0003f).rw(FUNC(konamim2_state::konami_io0_r), FUNC(konamim2_state::konami_io0_w));
	map(0x37c00010, 0x37c0001f).rw(FUNC(konamim2_state::konami_sio_r), FUNC(konamim2_state::konami_sio_w));
	map(0x37e00000, 0x37e0000f).rw(FUNC(konamim2_state::konami_io1_r), FUNC(konamim2_state::konami_io1_w));
	map(0x3f000000, 0x3fffffff).rw(m_ata, FUNC(ata_interface_device::cs0_swap_r), FUNC(ata_interface_device::cs0_swap_w));
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( konamim2 )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Video Res" )
	PORT_DIPSETTING(    0x00, "High Res" )
	PORT_DIPSETTING(    0x01, "Low Res" )

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNUSED ) // ATAPI?
	PORT_BIT( 0xDE00, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( btltryst )
	PORT_INCLUDE( konamim2 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Video Res" )
	PORT_DIPSETTING(    0x00, "High Res" )
	PORT_DIPSETTING(    0x01, "Low Res" )

	PORT_START("P2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P5")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6")
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( polystar )
	PORT_INCLUDE( konamim2 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Video Res" )
	PORT_DIPSETTING(    0x00, "High Res" )
	PORT_DIPSETTING(    0x01, "Low Res" )

	PORT_START("P2")
	PORT_DIPNAME( 0x01, 0x01, "Sound Output" )
	PORT_DIPSETTING(    0x01, "Mono" )
	PORT_DIPSETTING(    0x00, "Stereo" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P5")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6")
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( totlvice )
	PORT_INCLUDE( konamim2 )

	PORT_START("P2") // TODO: VERIFY
	PORT_DIPNAME( 0x01, 0x00, "Sound Output" )
	PORT_DIPSETTING(    0x01, "Mono" )
	PORT_DIPSETTING(    0x00, "Stereo" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("GUNX1")
	PORT_BIT( 0xffff, 0x0000, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0, 640) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNY1")
	PORT_BIT( 0xffff, 0x0000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0, 240) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("P5") // Gun switches
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6")
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( heatof11 )
	PORT_INCLUDE( konamim2 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Video Res" )
	PORT_DIPSETTING(    0x00, "High Res" )
	PORT_DIPSETTING(    0x01, "Low Res" )

	PORT_START("P2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P5")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6")
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( hellngt )
	PORT_INCLUDE( konamim2 )

	PORT_START("P2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("GUNX1")
	PORT_BIT( 0xffff, 0x0000, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0, 320*2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNY1")
	PORT_BIT( 0xffff, 0x0000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0, 240 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("P5") // Gun switches
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P6")
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW )
	PORT_BIT( 0xfffb, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void konamim2_state::cr589_config(device_t *device)
{
	device->subdevice<cdda_device>("cdda")->add_route(0, ":lspeaker", 0.5);
	device->subdevice<cdda_device>("cdda")->add_route(1, ":rspeaker", 0.5);
	device = device->subdevice("cdda");
}

void konamim2_state::konamim2(machine_config &config)
{
	// Basic machine hardware
	PPC602(config, m_ppc1, M2_CLOCK);
	m_ppc1->set_bus_frequency(M2_CLOCK / 2);
	m_ppc1->set_addrmap(AS_PROGRAM, &konamim2_state::m2_map);

	PPC602(config, m_ppc2, M2_CLOCK);
	m_ppc2->set_bus_frequency(M2_CLOCK / 2);
	m_ppc2->set_addrmap(AS_PROGRAM, &konamim2_state::m2_map);

	// M2 hardware
	M2_BDA(config, m_bda, M2_CLOCK, m_ppc1, m_ppc2, m_cde);
	m_bda->set_ram_size(m2_bda_device::RAM_8MB, m2_bda_device::RAM_8MB);
	m_bda->subdevice<m2_powerbus_device>("powerbus")->int_handler().set(FUNC(konamim2_state::ppc1_int));
	m_bda->subdevice<m2_memctl_device>("memctl")->gpio_out_handler<3>().set(FUNC(konamim2_state::ppc2_int)).invert();
	m_bda->subdevice<m2_vdu_device>("vdu")->set_screen("screen");
	m_bda->videores_in().set_ioport("DSW");
	m_bda->ldac_handler().set(FUNC(konamim2_state::ldac_out));
	m_bda->rdac_handler().set(FUNC(konamim2_state::rdac_out));

	M2_CDE(config, m_cde, M2_CLOCK, m_ppc1, m_bda);
	m_cde->int_handler().set(":bda:powerbus", FUNC(m2_powerbus_device::int_line<BDAINT_EXTD4_LINE>));
	m_cde->set_syscfg(SYSCONFIG_ARCADE);
	m_cde->sdbg_out().set(FUNC(konamim2_state::cde_sdbg_out));

	// Common devices
	EEPROM_93C46_16BIT(config, m_eeprom);

	ATA_INTERFACE(config, m_ata, 0);
	m_ata->irq_handler().set(FUNC(konamim2_state::ata_int));

	m_ata->slot(0).option_add("cr589", CR589);
	m_ata->slot(0).set_option_machine_config("cr589", cr589_config);
	m_ata->slot(0).set_default_option("cr589");

	// Video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update("bda:vdu", FUNC(m2_vdu_device::screen_update));

	// Sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	// TODO!
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_ldac, 0).add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	DAC_16BIT_R2R_TWOS_COMPLEMENT(config, m_rdac, 0).add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}



/*************************************
 *
 *  Machine fragments
 *
 *************************************/

void konamim2_state::set_ntsc(machine_config &config)
{
//  m_screen->set_raw(11750000, 766, 126, 126+640, 260, 20, 20+240); // TODO
	m_screen->set_refresh_hz(59.360001);
	m_screen->set_size(768, 262);
	m_screen->set_visarea(126, 126+640-1, 20, 20+240-1);
}

void konamim2_state::set_ntsc2(machine_config &config)
{
	//m_screen->set_raw(11750000, 766, 126, 126+640, 260, 20, 20+240); // TODO
	m_screen->set_refresh_hz(59.360001);
	m_screen->set_size(768, 262*2); // TOTAL VICE ONLY WORKS WITH THIS!
	m_screen->set_visarea(126, 126+640-1, 20, 20+240-1);
}

void konamim2_state::set_arcres(machine_config &config)
{
	m_screen->set_raw(16934500, 684, 104, 104+512, 416, 26, 26+384);
}

void konamim2_state::add_ymz280b(machine_config &config)
{
	// TODO: The YMZ280B outputs are actually routed to a speaker in each gun
	YMZ280B(config, m_ymz280b, XTAL(16'934'400));
	m_ymz280b->add_route(0, "lspeaker", 0.5);
	m_ymz280b->add_route(1, "rspeaker", 0.5);
}

void konamim2_state::add_mt48t58(machine_config &config)
{
	M48T58(config, m_m48t58);
}



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void konamim2_state::polystar(machine_config &config)
{
	konamim2(config);
	m_bda->set_ram_size(m2_bda_device::RAM_4MB, m2_bda_device::RAM_4MB);
	set_ntsc(config);
}

void konamim2_state::totlvice(machine_config &config)
{
	konamim2(config);
	add_ymz280b(config);
//  set_arcres(config);
	set_ntsc2(config);
}

void konamim2_state::btltryst(machine_config &config)
{
	konamim2(config);
	add_mt48t58(config);
	set_ntsc(config);
}

void konamim2_state::heatof11(machine_config &config)
{
	konamim2(config);
	add_mt48t58(config);
	set_arcres(config);
}

void konamim2_state::evilngt(machine_config &config)
{
	konamim2(config);
	add_mt48t58(config);
	add_ymz280b(config);
	set_ntsc(config);
}

void konamim2_state::hellngt(machine_config &config)
{
	konamim2(config);
	add_mt48t58(config);
	add_ymz280b(config);
	set_arcres(config);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( polystar )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // EEPROM default contents
	ROM_LOAD( "93c46.7k", 0x000000, 0x000080, CRC(fab5a203) SHA1(153e22aa8cfce80b77ba200957685f796fc99b1c) )

	DISK_REGION( "ata:0:cr589" ) // Has 1s of silence near the start of the first audio track
	DISK_IMAGE_READONLY( "623jaa02", 0, BAD_DUMP SHA1(e7d9e628a3e0e085e084e4e3630fa5e3a7345547) )
ROM_END

ROM_START( btltryst )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.7k",  0x000000, 0x000080, CRC(cc2c5640) SHA1(694cf2b3700f52ed80252b013052c90020e58ce6) )

	ROM_REGION( 0x2000, "m48t58", 0 ) // timekeeper SRAM
	ROM_LOAD( "m48t58", 0x000000, 0x002000, CRC(71ee073b) SHA1(cc8002d7ee8d1695aebbbb2a3a1e97a7e16948c1) )

	DISK_REGION( "ata:0:cr589" )
	DISK_IMAGE_READONLY( "636jac02", 0, SHA1(d36556a3a4b91058100924a9e9f1a58983399c6e) )
ROM_END

#if 0
ROM_START( btltrysta )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	ROM_REGION( 0x2000, "m48t58", 0 ) // timekeeper SRAM
	ROM_LOAD( "m48t58y", 0x000000, 0x002000, CRC(8611ff09) SHA1(6410236947d99c552c4a1f7dd5fd8c7a5ae4cba1) )

	DISK_REGION( "ata:0:cr589" )
	DISK_IMAGE_READONLY( "636jaa02", 0, SHA1(d36556a3a4b91058100924a9e9f1a58983399c6e) )
ROM_END
#endif

ROM_START( heatof11 )
	ROM_REGION64_BE( 0x200000, "boot", 0 )  // boot ROM
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // EEPROM default contents
	ROM_LOAD( "93c46.7k",  0x000000, 0x000080, CRC(e7029938) SHA1(ae41340dbcb600debe246629dc36fb371d1a5b05) )

	ROM_REGION( 0x2000, "m48t58", 0 ) // timekeeper SRAM
	ROM_LOAD( "dallas.5e",  0x000000, 0x002000, CRC(5b74eafd) SHA1(afbf5f1f5a27407fd6f17c764bbb7fae4ab779f5) )

	DISK_REGION( "ata:0:cr589" )
	/* Ring codes found on the disc:
	      703EAA02 PN.0000046809  1 + + + + +  IFPI L251
	      IFPI 42MO */
	DISK_IMAGE_READONLY( "703eaa02", 0, SHA1(f8a87eacfdbbd22659f39c7a72e3895f0a7697b7) )
ROM_END

ROM_START( evilngt )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // EEPROM default contents
	ROM_LOAD( "93c46.7k", 0x000000, 0x000080, CRC(60ae825e) SHA1(fd61db9667c53dd12700a0fe202fcd1e3d35d206) )

	ROM_REGION( 0x2000, "m48t58", 0 ) // timekeeper SRAM
	ROM_LOAD( "m48t58y.9n", 0x000000, 0x002000, CRC(e887ca1f) SHA1(54205f01b1ceba1d5f4d979fc30be1add8116e90) )

	ROM_REGION( 0x400000, "ymz", 0 ) // YMZ280B sound ROM on sub board
	ROM_LOAD( "810a03.16h", 0x000000, 0x400000, CRC(05112d3a) SHA1(0df2a167b7bc08a32d983b71614d59834efbfb59) )

	DISK_REGION( "ata:0:cr589" )
	DISK_IMAGE_READONLY( "810uba02", 0, SHA1(e570470c1cbfe187d5bba8125616412f386264ba) )
ROM_END

ROM_START( hellngt )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "636a01.8q", 0x000000, 0x200000, CRC(7b1dc738) SHA1(32ae8e7ddd38fcc70b4410275a2cc5e9a0d7d33b) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 ) // EEPROM default contents
	ROM_LOAD( "93c46.7k",    0x000000, 0x000080, CRC(53b41f68) SHA1(f75f59808a5b04b1e49f2cca0592a2466b82f019) )

	ROM_REGION( 0x2000, "m48t58", 0 )
	ROM_LOAD( "m48t58y.9n",  0x000000, 0x002000, CRC(ff8e78a1) SHA1(02e56f55264dd0bf3a08808726a6366e9cb6031e) )

	ROM_REGION( 0x400000, "ymz", 0 ) // YMZ280B sound ROM on sub board
	ROM_LOAD( "810a03.16h",  0x000000, 0x400000, CRC(05112d3a) SHA1(0df2a167b7bc08a32d983b71614d59834efbfb59) )

	DISK_REGION( "ata:0:cr589" )
	DISK_IMAGE_READONLY( "810eaa02", 0, SHA1(d701b900eddc7674015823b2cb33e887bf107fa8) )
ROM_END

ROM_START( totlvice )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.7k", 0x000000, 0x000080, CRC(25aa0bd1) SHA1(cc461e0629ff71c3a868882f1f67af0e19135c1a) )

	ROM_REGION( 0x100000, "ymz", 0 ) // YMZ280B sound rom on sub board
	ROM_LOAD( "639jaa02.bin",  0x000000, 0x100000, CRC(c6163818) SHA1(b6f8f2d808b98610becc0a5be5443ece3908df0b) )

	DISK_REGION( "ata:0:cr589" )
	DISK_IMAGE_READONLY( "639eba01", 0, BAD_DUMP SHA1(d95c13575e015169b126f7e8492d150bd7e5ebda) )
ROM_END

#if 0
// NB: Dumped by Phil, hasn't been converted to CHD yet
ROM_START( totlvicd )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION( 0x100000, "ymz", 0 ) // YMZ280B sound rom on sub board
	ROM_LOAD( "639jaa02.bin",  0x000000, 0x100000, CRC(c6163818) SHA1(b6f8f2d808b98610becc0a5be5443ece3908df0b) )

	DISK_REGION( "ata:0:cr589" )
	DISK_IMAGE_READONLY( "639ead01", 0, SHA1(9d1085281aeb14185e2e78f3f21e7004a591039c) )
ROM_END
#endif

ROM_START( totlvicu )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION( 0x100000, "ymz", 0 ) // YMZ280B sound rom on sub board
	ROM_LOAD( "639jaa02.bin",  0x000000, 0x100000, CRC(c6163818) SHA1(b6f8f2d808b98610becc0a5be5443ece3908df0b) )

	DISK_REGION( "ata:0:cr589" )
	DISK_IMAGE_READONLY( "639uac01", 0, BAD_DUMP SHA1(88431b8a0ce83c156c8b19efbba1af901b859404) )
ROM_END

ROM_START( totlvica )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION( 0x100000, "ymz", 0 ) // YMZ280B sound rom on sub board
	ROM_LOAD( "639jaa02.bin",  0x000000, 0x100000, CRC(c6163818) SHA1(b6f8f2d808b98610becc0a5be5443ece3908df0b) )

	DISK_REGION( "ata:0:cr589" )
	DISK_IMAGE_READONLY( "639aab01", 0, SHA1(34f34b26399cc04ffb0207df69f52eba42892eb6) )
ROM_END

ROM_START( totlvicj )
	ROM_REGION64_BE( 0x200000, "boot", 0 )
	ROM_LOAD16_WORD( "623b01.8q", 0x000000, 0x200000, CRC(bd879f93) SHA1(e2d63bfbd2b15260a2664082652442eadea3eab6) )

	ROM_REGION( 0x100000, "ymz", 0 ) // YMZ280B sound rom on sub board
	ROM_LOAD( "639jaa02.bin",  0x000000, 0x100000, CRC(c6163818) SHA1(b6f8f2d808b98610becc0a5be5443ece3908df0b) )

	DISK_REGION( "ata:0:cr589" ) // Need a re-image
	DISK_IMAGE_READONLY( "639jad01", 0, BAD_DUMP SHA1(39d41d5a9d1c40636d174c8bb8172b1121e313f8) )
ROM_END

#if 0 // FIXME
ROM_START( 3do_m2 )
	ROM_REGION64_BE( 0x100000, "boot", 0 )
	ROM_SYSTEM_BIOS( 0, "panafz35", "Panasonic FZ-35S (3DO M2)" )
	ROMX_LOAD( "fz35_jpn.bin", 0x000000, 0x100000, CRC(e1c5bfd3) SHA1(0a3e27d672be79eeee1d2dc2da60d82f6eba7934), ROM_BIOS(1) )
ROM_END
#endif

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void konamim2_state::install_m48t58()
{
	read8sm_delegate read_delegate(*m_m48t58, FUNC(m48t58_device::read));
	write8sm_delegate write_delegate(*m_m48t58, FUNC(m48t58_device::write));

	m_ppc1->space(AS_PROGRAM).install_readwrite_handler(0x36c00000, 0x36c03fff, read_delegate, write_delegate, 0xff00ff00ff00ff00ULL);
	m_ppc2->space(AS_PROGRAM).install_readwrite_handler(0x36c00000, 0x36c03fff, read_delegate, write_delegate, 0xff00ff00ff00ff00ULL);
}

void konamim2_state::install_ymz280b()
{
	read8sm_delegate read_delegate(*m_ymz280b, FUNC(ymz280b_device::read));
	write8sm_delegate write_delegate(*m_ymz280b, FUNC(ymz280b_device::write));

	m_ppc1->space(AS_PROGRAM).install_readwrite_handler(0x3e800000, 0x3e80000f, read_delegate, write_delegate, 0xff00ff0000000000ULL);
	m_ppc2->space(AS_PROGRAM).install_readwrite_handler(0x3e800000, 0x3e80000f, read_delegate, write_delegate, 0xff00ff0000000000ULL);
}

void konamim2_state::init_totlvice()
{
	install_ymz280b();
}

void konamim2_state::init_btltryst()
{
	install_m48t58();
}

void konamim2_state::init_hellngt()
{
	install_m48t58();
	install_ymz280b();
}


/*************************************
 *
 *  Debugging Aids
 *
 *************************************/

void konamim2_state::debug_help_command(const std::vector<std::string_view> &params)
{
	debugger_console &con = machine().debugger().console();

	con.printf("Available M2 commands:\n");
	con.printf("  konm2 dump_task,<address> -- Dump task object at <address>\n");
	con.printf("  konm2 dump_dspp,<address> -- Dump DSPP object at <address>\n");
}

void konamim2_state::debug_commands(const std::vector<std::string_view> &params)
{
	if (params.size() < 1)
		return;

	if (params[0] == "help")
		debug_help_command(params);
	else if (params[0] == "dump_task")
		dump_task_command(params);
	else if (params[0] == "dump_dspp")
		subdevice<dspp_device>("bda:dspp")->dump_state();
}

void konamim2_state::dump_task_command(const std::vector<std::string_view> &params)
{
	typedef uint32_t   Item;
	typedef uint32_t   m2ptr;

	typedef struct TimerTicks
	{
		uint32_t tt_Hi;
		uint32_t tt_Lo;
	} TimerTicks;

	struct ItemNode
	{
		m2ptr pn_Next;                /* pointer to next in list              */ // 0
		m2ptr pn_Prev;                /* pointer to previous in list          */ // 4
		uint8_t     n_SubsysType;     /* what component manages this node     */ // 8
		uint8_t     n_Type;           /* what type of node for the component  */ // 9
		uint8_t     n_Priority;       /* queueing priority                    */ // A
		uint8_t     n_Flags;          /* misc flags, see below                */ // B
		int32_t     n_Size;           /* total size of node including hdr     */ // C
		m2ptr    pn_Name;             /* name of item, or NULL                */ // 10
		uint8_t     n_Version;        /* version of of this Item              */ // 14
		uint8_t     n_Revision;       /* revision of this Item                */ // 15
		uint8_t     n_Reserved0;      /* reserved for future use              */ // 16
		uint8_t     n_ItemFlags;      /* additional system item flags         */ // 17
		Item      n_Item;             /* Item number representing this struct */ //18
		Item      n_Owner;            /* creator, present owner, disposer     */ // 1C
		m2ptr     pn_Reserved1;       /* reserved for future use              */ // 20
	};

	struct Task
	{
		ItemNode     t;
		m2ptr       pt_ThreadTask;      /* I am a thread of what task?  */
		uint32_t     t_WaitBits;        /* signals being waited for     */
		uint32_t     t_SigBits;         /* signals received             */
		uint32_t     t_AllocatedSigs;   /* signals allocated            */
		m2ptr        pt_StackBase;      /* base of stack                */
		int32_t      t_StackSize;       /* size of stack                */
		uint32_t     t_MaxUSecs;        /* quantum length in usecs      */
		TimerTicks   t_ElapsedTime;     /* time spent running this task */
		uint32_t     t_NumTaskLaunch;   /* # times launched this task   */
		uint32_t     t_Flags;           /* task flags                   */
		Item         t_Module;          /* the module we live within    */
		Item         t_DefaultMsgPort;  /* default task msgport         */
		m2ptr         pt_UserData;      /* user-private data            */
	};

	debugger_console &con = machine().debugger().console();
	uint64_t addr;
	offs_t address;

	if (params.size() < 1)
		return;

	if (!con.validate_number_parameter(params[1], addr))
		return;

	address = (offs_t)addr;
	address = 0x40FB54E8;
	address_space *tspace;
	if (!m_ppc1->translate(AS_PROGRAM, device_memory_interface::TR_READ, address, tspace))
	{
		con.printf("Address is unmapped.\n");
		return;
	}

	Task task;

	task.t.pn_Next = tspace->read_dword(address + offsetof(ItemNode, pn_Next));
	task.t.pn_Prev = tspace->read_dword(address + offsetof(ItemNode, pn_Prev));
	task.t.n_SubsysType = tspace->read_byte(address + offsetof(ItemNode, n_SubsysType));
	task.t.n_Type = tspace->read_byte(address + offsetof(ItemNode, n_Type));
	task.t.n_Priority = tspace->read_byte(address + offsetof(ItemNode, n_Priority));
	task.t.n_Flags = tspace->read_byte(address + offsetof(ItemNode, n_Flags));
	task.t.n_Size = tspace->read_dword(address + offsetof(ItemNode, n_Size));
	task.t.pn_Name = tspace->read_dword(address + offsetof(ItemNode, pn_Name));

	char name[128];
	char *ptr = name;
	uint32_t nameptr = task.t.pn_Name;

	do
	{
		*ptr = tspace->read_byte(nameptr++);
	} while (*ptr++ != 0);

	task.t.n_Version = tspace->read_byte(address + offsetof(ItemNode, n_Version));
	task.t.n_Revision = tspace->read_byte(address + offsetof(ItemNode, n_Revision));
	task.t.n_Reserved0 = tspace->read_byte(address + offsetof(ItemNode, n_Reserved0));
	task.t.n_ItemFlags = tspace->read_byte(address + offsetof(ItemNode, n_ItemFlags));
	task.t.n_Item = tspace->read_dword(address + offsetof(ItemNode, n_Item));
	task.t.n_Owner = tspace->read_dword(address + offsetof(ItemNode, n_Owner));
	task.t.pn_Reserved1 = tspace->read_dword(address + offsetof(ItemNode, pn_Reserved1));

	task.pt_ThreadTask = tspace->read_dword(address + offsetof(Task, pt_ThreadTask));
	task.t_WaitBits = tspace->read_dword(address + offsetof(Task, t_WaitBits));
	task.t_SigBits = tspace->read_dword(address + offsetof(Task, t_SigBits));
	task.t_AllocatedSigs = tspace->read_dword(address + offsetof(Task, t_AllocatedSigs));
	task.pt_StackBase = tspace->read_dword(address + offsetof(Task, pt_StackBase));
	task.t_StackSize = tspace->read_dword(address + offsetof(Task, t_StackSize));
	task.t_MaxUSecs = tspace->read_dword(address + offsetof(Task, t_MaxUSecs));
	task.t_ElapsedTime.tt_Hi = tspace->read_dword(address + offsetof(Task, t_ElapsedTime)+0);
	task.t_ElapsedTime.tt_Lo = tspace->read_dword(address + offsetof(Task, t_ElapsedTime)+4);
	task.t_NumTaskLaunch = tspace->read_dword(address + offsetof(Task, t_NumTaskLaunch));
	task.t_Flags = tspace->read_dword(address + offsetof(Task, t_Flags));
	task.t_Module = tspace->read_dword(address + offsetof(Task, t_Module));
	task.t_DefaultMsgPort = tspace->read_dword(address + offsetof(Task, t_DefaultMsgPort));
	task.pt_UserData = tspace->read_dword(address + offsetof(Task, pt_UserData));

//  m2ptr       pt_ThreadTask;      /* I am a thread of what task?  */
//  uint32_t     t_WaitBits;        /* signals being waited for     */
//  uint32_t     t_SigBits;         /* signals received             */
//  uint32_t     t_AllocatedSigs;   /* signals allocated            */
//  m2ptr        pt_StackBase;      /* base of stack                */
//  int32_t      t_StackSize;       /* size of stack                */
//  uint32_t     t_MaxUSecs;        /* quantum length in usecs      */
//  TimerTicks   t_ElapsedTime;     /* time spent running this task */
//  uint32_t     t_NumTaskLaunch;   /* # times launched this task   */
//  uint32_t     t_Flags;           /* task flags                   */
//  Item         t_Module;          /* the module we live within    */
//  Item         t_DefaultMsgPort;  /* default task msgport         */
//  m2ptr         pt_UserData;      /* user-private data            */

	con.printf("**** Task Info @ %08X ****\n", address);
	con.printf("Next:        %08X\n", task.t.pn_Next);
	con.printf("Prev:        %08X\n", task.t.pn_Prev);
	con.printf("SubsysType:  %X\n", task.t.n_SubsysType);
	con.printf("Type:        %X\n", task.t.n_Type);
	con.printf("Priority:    %X\n", task.t.n_Priority);
	con.printf("Flags:       %X\n", task.t.n_Flags);
	con.printf("Size:        %08X\n", task.t.n_Size);
	con.printf("Name:        %s\n", name);
	con.printf("Version:     %X\n", task.t.n_Version);
	con.printf("Revision:    %X\n", task.t.n_Revision);
	con.printf("Reserved0:   %X\n", task.t.n_Reserved0);
	con.printf("ItemFlags:   %X\n", task.t.n_ItemFlags);
	con.printf("Item:        %08X\n", task.t.n_Item);
	con.printf("Owner:       %08X\n", task.t.n_Owner);
	con.printf("Reserved1:   %08X\n", task.t.pn_Reserved1);
	con.printf("ThreadTask:  %08X\n", task.pt_ThreadTask);
	con.printf("WaitBits:    %08X\n", task.t_WaitBits);
	con.printf("SigBits:     %08X\n", task.t_SigBits);
	con.printf("AllocSigs:   %08X\n", task.t_AllocatedSigs);
	con.printf("StackBase:   %08X\n", task.pt_StackBase);
	con.printf("StackSize:   %08X\n", task.t_StackSize);
	con.printf("MaxUSecs:    %08X\n", task.t_MaxUSecs);
	con.printf("ElapsedTime: %016llu\n", (uint64_t)task.t_ElapsedTime.tt_Lo + ((uint64_t)task.t_ElapsedTime.tt_Hi << 32ull));
	con.printf("NumTaskLaunch:  %u\n", task.t_NumTaskLaunch);
	con.printf("Flags:          %08X\n", task.t_Flags);
	con.printf("Module:         %08X\n", task.t_Module);
	con.printf("DefaultMsgPort: %08X\n", task.t_DefaultMsgPort);
	con.printf("UserData:       %08X\n", task.pt_UserData);
	con.printf("\n");
}

} // anonymous namespace



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1997, polystar,  0,        polystar, polystar, konamim2_state, empty_init,    ROT0, "Konami", "Tobe! Polystars (ver JAA)",    MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_SOUND )
GAME( 1997, totlvice,  0,        totlvice, totlvice, konamim2_state, init_totlvice, ROT0, "Konami", "Total Vice (ver EBA)",         MACHINE_IMPERFECT_TIMING )
//GAME( 1997, totlvicd, totlvice, totlvice, totlvice, konamim2_state, init_totlvice, ROT0, "Konami", "Total Vice (ver EAD)",         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING )
GAME( 1997, totlvicj,  totlvice, totlvice, totlvice, konamim2_state, init_totlvice, ROT0, "Konami", "Total Vice (ver JAD)",         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING )
GAME( 1997, totlvica,  totlvice, totlvice, totlvice, konamim2_state, init_totlvice, ROT0, "Konami", "Total Vice (ver AAB)",         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING )
GAME( 1997, totlvicu,  totlvice, totlvice, totlvice, konamim2_state, init_totlvice, ROT0, "Konami", "Total Vice (ver UAC)",         MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING )
GAME( 1998, btltryst,  0,        btltryst, btltryst, konamim2_state, init_btltryst, ROT0, "Konami", "Battle Tryst (ver JAC)",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
//GAME( 1998, btltrysta, btltryst, btltryst, btltryst, konamim2_state, init_btltryst, ROT0, "Konami", "Battle Tryst (ver JAA)",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, heatof11,  0,        heatof11, heatof11, konamim2_state, init_btltryst, ROT0, "Konami", "Heat of Eleven '98 (ver EAA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS)
GAME( 1998, evilngt,   0,        evilngt,  hellngt,  konamim2_state, init_hellngt,  ROT0, "Konami", "Evil Night (ver UBA)",         MACHINE_IMPERFECT_TIMING )
GAME( 1998, hellngt,   evilngt,  hellngt,  hellngt,  konamim2_state, init_hellngt,  ROT0, "Konami", "Hell Night (ver EAA)",         MACHINE_IMPERFECT_TIMING )

//CONS( 199?, 3do_m2,     0,      0,    3do_m2,    m2,    driver_device, 0,      "3DO",  "3DO M2",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING | MACHINE_NO_SOUND )
