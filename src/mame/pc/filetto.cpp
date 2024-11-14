// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Chris Hardy
/**************************************************************************************************

Filetto (c) 1990 Novarmatic

TODO:
- Add a proper FDC device;
- UM5100 sample clock;
- buzzer sound;

===================================================================================================
The PCB is a un-modified IBM-PC with a CGA adapter & a prototyping card that controls the
interface between the pc and the Jamma connectors.Additionally there's also a UM5100 sound
chip for the sound.
PCB Part Number: S/N 90289764 NOVARXT
PCB Contents:
1x UMC 8923S-UM5100 voice processor (upper board)
1x MMI PAL16L8ACN-940CRK9 (upper board)
1x AMD AMPAL16R8APC-8804DM (upper board)
1x AMD P8088-1 main processor 8.000MHz (lower board)
1x Proton PT8010AF PLCC 28.636MHz (lower board)
1x UMC 8928LP-UM8272A floppy disk controller (lower board)
1x UMC 8935CS-UM82C11 Printer Adapter Interface (lower board)
1x UMC 8936CS-UM8250B Programmable asynchronous communications element (lower board)
1x UMC 8937NS-UM82C8167 Real Time Clock (lower board)
1x Yamaha V6363 CMDC QFP (lower board)
There isn't any keyboard found connected to the pcb.

===================================================================================================
The software of this game can be extracted with a normal Windows program extractor.
The files names are:
-command.com  (1)
-ibmbio.com   (1)
-ibmdos.com   (1)
-ansi.sys     (1)
-config.sys   (2)
-autoexec.bat (3)
-x.exe        (4)
(1)This is an old Italian version of MS-DOS (v3.30 18th March 1987).
(2)Contains "device=ansi.sys",it's an hook-up for the graphics used by the BIOS.
(3)It has an Echo off (as you can notice from the game itself) and then the loading of the
main program (x.exe).
(4)The main program,done in plain Basic with several Italian comments in it.The date of
the main program is 9th October 1990.

**************************************************************************************************/

#include "emu.h"
#include "bus/isa/cga.h"
#include "cpu/i86/i86.h"
#include "machine/bankdev.h"
#include "machine/genpc.h"
#include "sound/hc55516.h"

class isa8_cga_filetto_device : public isa8_cga_device
{
public:
	// construction/destruction
	isa8_cga_filetto_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DEFINE_DEVICE_TYPE(ISA8_CGA_FILETTO, isa8_cga_filetto_device, "filetto_cga", "ISA8_CGA_FILETTO")

//-------------------------------------------------
//  isa8_cga_filetto_device - constructor
//-------------------------------------------------

isa8_cga_filetto_device::isa8_cga_filetto_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_cga_device(mconfig, ISA8_CGA_FILETTO, tag, owner, clock)
{
}

ROM_START( filetto_cga )
	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD("u67.bin", 0x0000, 0x2000, CRC(09710122) SHA1(de84bdd9245df287bbd3bb808f0c3531d13a3545) )
ROM_END

const tiny_rom_entry *isa8_cga_filetto_device::device_rom_region() const
{
	return ROM_NAME( filetto_cga );
}

namespace {

class filetto_state : public driver_device
{
public:
	filetto_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mb(*this, "mb")
		, m_bank(*this, "bank")
		, m_cvsd(*this, "voice")
		, m_samples(*this, "samples")
	{ }

	void filetto(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sample_tick);

private:
	int m_lastvalue;
	uint8_t m_disk_data[2];
	uint8_t m_port_b_data;
	uint8_t m_status;
	uint8_t m_clr_status;
	uint8_t m_voice, m_bit;
	uint32_t m_vaddr;
	emu_timer *m_sample;

	uint8_t disk_iobank_r(offs_t offset);
	void disk_iobank_w(offs_t offset, uint8_t data);
	uint8_t fdc765_status_r();
	uint8_t fdc765_data_r();
	void fdc765_data_w(uint8_t data);
	void fdc_dor_w(uint8_t data);
	uint8_t port_a_r();
	uint8_t port_b_r();
	uint8_t port_c_r();
	void port_b_w(uint8_t data);
	void voice_start_w(uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_device<pc_noppi_mb_device> m_mb;
	required_device<address_map_bank_device> m_bank;
	required_device<hc55516_device> m_cvsd;
	required_memory_region m_samples;
	void bank_map(address_map &map) ATTR_COLD;
	void filetto_io(address_map &map) ATTR_COLD;
	void filetto_map(address_map &map) ATTR_COLD;
};


uint8_t filetto_state::disk_iobank_r(offs_t offset)
{
	//printf("Read Prototyping card [%02x] @ PC=%05x\n",offset,m_maincpu->pc());
	//if(offset == 0) return ioport("DSW")->read();
	if(offset == 1) return ioport("IN1")->read();

	return m_disk_data[offset];
}

void filetto_state::disk_iobank_w(offs_t offset, uint8_t data)
{
/*
    BIOS does a single out $0310,$F0 on reset

    Then does 2 outs to set the bank..

        X1  X2

        $F0 $F2 = m0
        $F1 $F2 = m1
        $F0 $F3 = m2
        $F1 $F3 = m3

    The sequence of

    out $0310,X1
    out $0310,X2

    sets the selected rom that appears in $C0000-$CFFFF

*/
	int bank = 0;

//  printf("bank %d set to %02X\n", offset,data);

	if (data == 0xF0)
	{
		bank = 0;
	}
	else
	{
		if((m_lastvalue == 0xF0) && (data == 0xF2))
			bank = 0;
		else if ((m_lastvalue == 0xF1) && (data == 0xF2))
			bank = 1;
		else if ((m_lastvalue == 0xF0) && (data == 0xF3))
			bank = 2;
		else if ((m_lastvalue == 0xF1) && (data == 0xF3))
			bank = 3;
	}

	if (!(data & 0xf0))
	{
		int bit = (data >> 1) - 2;
		m_voice &= ~(1 << bit);
		m_voice |= BIT(data, 0) << bit;
	}

	m_bank->set_bank(bank);

	m_lastvalue = data;

	m_disk_data[offset] = data;
}

uint8_t filetto_state::port_a_r()
{
	return 0xaa;//harmless keyboard error occurs without this
}

uint8_t filetto_state::port_b_r()
{
	return m_port_b_data;
}

uint8_t filetto_state::port_c_r()
{
	return 0x00;// DIPS?
}

// Filetto uses this for either beep and um5100 sound routing,probably there's a I/O select somewhere ...
void filetto_state::port_b_w(uint8_t data)
{
	m_mb->m_pit8253->write_gate2(BIT(data, 0));
	m_mb->pc_speaker_set_spkrdata(BIT(data, 1));
	m_port_b_data = data;
//  m_cvsd->digit_w(data);
}

/*Floppy Disk Controller 765 device*/
/*Currently we only emulate it at a point that the BIOS will pass the checks*/

#define FDC_BUSY 0x10
#define FDC_WRITE 0x40
#define FDC_READ 0x00 /*~0x40*/

uint8_t filetto_state::fdc765_status_r()
{
	uint8_t tmp;
	tmp = m_status | 0x80;
	m_clr_status++;
	if(m_clr_status == 0x10)
	{
		m_status = 0;
		m_clr_status = 0;
	}
	return tmp;
}

uint8_t filetto_state::fdc765_data_r()
{
	m_status = (FDC_READ);
	m_mb->m_pic8259->ir6_w(0);
	return 0xc0;
}

void filetto_state::fdc765_data_w(uint8_t data)
{
	m_status = (FDC_WRITE);
}


void filetto_state::fdc_dor_w(uint8_t data)
{
	m_mb->m_pic8259->ir6_w(1);
}

// TODO: move to a real um5100 device
void filetto_state::voice_start_w(uint8_t data)
{
	// TODO: accurate pitch frequency
	m_sample->adjust(attotime::zero, 0, attotime::from_hz(44150));
	m_bit = 7;
	m_vaddr = ((m_voice & 0xf / 5) | (BIT(m_voice, 4) << 2)) * 0x8000;
	logerror("%x %x\n",m_voice,m_vaddr);
}

void filetto_state::filetto_map(address_map &map)
{
	map(0xc0000, 0xcffff).m(m_bank, FUNC(address_map_bank_device::amap8));
	map(0xf0000, 0xfffff).rom().region("bios", 0);
}

void filetto_state::filetto_io(address_map &map)
{
	map.global_mask(0x3ff);
	map(0x0000, 0x00ff).m(m_mb, FUNC(pc_noppi_mb_device::map));
	map(0x0060, 0x0060).r(FUNC(filetto_state::port_a_r));  //not a real 8255
	map(0x0061, 0x0061).rw(FUNC(filetto_state::port_b_r), FUNC(filetto_state::port_b_w));
	map(0x0062, 0x0062).r(FUNC(filetto_state::port_c_r));
	map(0x0201, 0x0201).portr("COIN"); // game port
	map(0x0310, 0x0311).rw(FUNC(filetto_state::disk_iobank_r), FUNC(filetto_state::disk_iobank_w)); //Prototyping card
	map(0x0312, 0x0312).portr("IN0"); // Prototyping card, r/o
	map(0x0313, 0x0313).w(FUNC(filetto_state::voice_start_w));
	map(0x03f2, 0x03f2).w(FUNC(filetto_state::fdc_dor_w));
	map(0x03f4, 0x03f4).r(FUNC(filetto_state::fdc765_status_r)); //765 Floppy Disk Controller (FDC) Status
	map(0x03f5, 0x03f5).rw(FUNC(filetto_state::fdc765_data_r), FUNC(filetto_state::fdc765_data_w));//FDC Data
}


void filetto_state::bank_map(address_map &map)
{
	map(0x00000, 0x3ffff).rom().region("game_prg", 0);
}

static INPUT_PORTS_START( filetto )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Extra Play" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, "Play at 6th match reached" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "pcvideo_cga_config" )
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

TIMER_CALLBACK_MEMBER(filetto_state::sample_tick)
{
	m_cvsd->digit_w(BIT(m_samples->as_u8(m_vaddr), m_bit));
	m_cvsd->clock_w(1);
	m_cvsd->clock_w(0);
	if (m_bit == 0)
	{
		m_vaddr++;
		m_bit = 8;
		if (!(m_vaddr % 0x8000))
			m_sample->adjust(attotime::never);
	}
	m_bit--;
}

void filetto_state::machine_start()
{
	m_sample = timer_alloc(FUNC(filetto_state::sample_tick), this);

	m_status = 0;
	m_clr_status = 0;
}

void filetto_state::machine_reset()
{
	m_lastvalue = -1;
	m_voice = 0;
}

static void filetto_isa8_cards(device_slot_interface &device)
{
	device.option_add_internal("filetto",  ISA8_CGA_FILETTO);
}


void filetto_state::filetto(machine_config &config)
{
	I8088(config, m_maincpu, XTAL(14'318'181)/3);
	m_maincpu->set_addrmap(AS_PROGRAM, &filetto_state::filetto_map);
	m_maincpu->set_addrmap(AS_IO, &filetto_state::filetto_io);
	m_maincpu->set_irq_acknowledge_callback("mb:pic8259", FUNC(pic8259_device::inta_cb));

	PCNOPPI_MOTHERBOARD(config, m_mb, 0).set_cputag(m_maincpu);
	m_mb->int_callback().set_inputline(m_maincpu, 0);
	m_mb->nmi_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	ISA8_SLOT(config, "isa1", 0, "mb:isa", filetto_isa8_cards, "filetto", true); // FIXME: determine ISA bus clock

	HC55516(config, m_cvsd, 0).add_route(ALL_OUTPUTS, "mb:mono", 0.60); //8923S-UM5100 is a HC55536 with ROM hook-up

	RAM(config, RAM_TAG).set_default_size("640K");

	ADDRESS_MAP_BANK(config, m_bank).set_map(&filetto_state::bank_map).set_options(ENDIANNESS_LITTLE, 8, 18, 0x10000);
}

ROM_START( filetto )
	ROM_REGION( 0x10000, "bios", 0 )
	ROM_LOAD("u49.bin", 0xc000, 0x2000, CRC(1be6948a) SHA1(9c433f63d347c211ee4663f133e8417221bc4bf0))
	ROM_RELOAD(         0x8000, 0x2000 )
	ROM_RELOAD(         0x4000, 0x2000 )
	ROM_RELOAD(         0x0000, 0x2000 )
	ROM_LOAD("u55.bin", 0xe000, 0x2000, CRC(1e455ed7) SHA1(786d18ce0ab1af45fc538a2300853e497488f0d4) )
	ROM_RELOAD(         0xa000, 0x2000 )
	ROM_RELOAD(         0x6000, 0x2000 )
	ROM_RELOAD(         0x2000, 0x2000 )

	ROM_REGION( 0x40000, "game_prg", 0 ) // program data
	ROM_LOAD( "m0.u1", 0x00000, 0x10000, CRC(2408289d) SHA1(eafc144a557a79b58bcb48545cb9c9778e61fcd3) )
	ROM_LOAD( "m1.u2", 0x10000, 0x10000, CRC(5b623114) SHA1(0d9a14e6b7f57ce4fa09762343b610a973910f58) )
	ROM_LOAD( "m2.u3", 0x20000, 0x10000, CRC(abc64869) SHA1(564fc9d90d241a7b7776160b3fd036fb08037355) )
	ROM_LOAD( "m3.u4", 0x30000, 0x10000, CRC(0c1e8a67) SHA1(f1b9280c65fcfcb5ec481cae48eb6f52d6cdbc9d) )

	ROM_REGION( 0x40000, "samples", 0 ) // UM5100 sample roms
	ROM_LOAD("v1.u15",  0x00000, 0x20000, CRC(613ddd07) SHA1(ebda3d559315879819cb7034b5696f8e7861fe42) )
	ROM_LOAD("v2.u14",  0x20000, 0x20000, CRC(427e012e) SHA1(50514a6307e63078fe7444a96e39d834684db7df) )
ROM_END


} // anonymous namespace


GAME( 1990, filetto,  0, filetto,  filetto,  filetto_state, empty_init, ROT0,  "Novarmatic", "Filetto (v1.05 901009)", MACHINE_IMPERFECT_SOUND )
