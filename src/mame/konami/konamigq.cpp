// license:BSD-3-Clause
// copyright-holders:R. Belmont, smf
/***************************************************************************

  Konami GQ System - Arcade PSX Hardware
  ======================================
  Driver by R. Belmont & smf

  Crypt Killer
  Konami, 1995

  PCB Layout
  ----------

  GQ420  PWB354905B
  |----------------------------------------------------------------|
  |CN14          420A01.2G          420A02.3M         CN6 CN7 CN8  |
  |           056602  6264    |------| 424800                      |
  |           LA4705  6264    |058141|                             |
  |                   68000   |------| 424800                      |
  |     18.432MHz   PAL(000607)                                    |
  |     32MHz 48kHz PAL       |------| 424800                      |
  |                (000608)   |058800|                             |
  |    RESET_SW               |------| M514256                     |
  |                         |--------| M514256                     |
  |J                        |TMS57002|                             |
  |A                        |--------| MACH110                     |
  |M  000180          53.693175MHz    (000619A)                    |
  |M                        |--------|                             |
  |A  KM4216V256 KM4216V256 |CXD8538Q|           PAL (000613)      |
  |   KM4216V256 KM4216V256 |        |           PAL (000612)      |
  | TEST_SW                 |--------|           PAL (000618)      |
  |   CXD2923AR        67.7376MHz           93C46   NCR53CF96-2    |
  | DIPSW(8)           |---------|          420B03.27P    HDD_LED  |
  |  KM48V514 KM48V514 |CXD8530BQ|             |-----------------| |
  |  KM48V514 KM48V514 |         |             |543MB SCSI HDRIVE| |
  |CN3  UPA1556        |---------|             |TOSHIBA MK1924FBV| |
  |  KM48V514 KM48V514                         |(420UAA04)       | |
  |  KM48V514 KM48V514                         |-----------------| |
  |----------------------------------------------------------------|

  Notes:
        CN6/7/8 - 5-pin connector for guns. All 3 connectors are wired the same.
                  Pinout is...
                  1- gun opto
                  2- ground
                  3- trigger
                  4- +5v
                  5- reload (see below)

                  The Crypt Killer shotguns have 6 wires coming out of the gun.
                  The wire colors and connections are....
                  Black  - Ground; joined to CN6/7/8 pin 2
                  Red    - +5v; joined to CN6/7/8 pin 4
                  Yellow - Gun opto; joined to CN6/7/8 pin 1
                  White  - Trigger; joined to CN6/7/8 pin 3
                  Purple - Reload; joined to CN6/7/8 pin 5
                  Grey   - Reload ground; this wire must be joined to any ground for the reload to work

        CN3     - For connection of extra controls/buttons (e.g. player 3 start etc)
        CN14    - For connection of additional speaker for stereo output
        68000   - Clock input 8.000MHz [32/4]
        53CF96  - Clock input 16.000MHz [32/2]
        TMS57002- Clock input 24.000MHz [48/2]
        056602  - Konami custom ceramic module. Clock inputs 18.432MHz, 1.536MHz[18.432/12], 48kHz
        000180  - Konami custom ceramic module containing LM1203 and resistors/caps etc
        Vsync   - 59.825Hz
        HSync   - 15.613kHz
*/

#include "emu.h"

#include "bus/nscsi/hd.h"
#include "cpu/m68000/m68000.h"
#include "cpu/psx/psx.h"
#include "cpu/tms57002/tms57002.h"
#include "machine/eepromser.h"
#include "machine/mb89371.h"
#include "machine/ncr53c90.h"
#include "machine/ram.h"
#include "sound/k054539.h"
#include "sound/k056800.h"
#include "video/psx.h"

#include "screen.h"
#include "speaker.h"

#include "endianness.h"
#include "multibyte.h"


namespace {

class konamigq_state : public driver_device
{
public:
	konamigq_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_dasp(*this, "dasp"),
		m_ncr53cf96(*this, "scsi:7:ncr53cf96"),
		m_k056800(*this, "k056800"),
		m_pcmram(*this, "pcmram")
	{
	}

	void konamigq(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	emu_timer *m_dma_timer;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<tms57002_device> m_dasp;
	required_device<ncr53cf96_device> m_ncr53cf96;
	required_device<k056800_device> m_k056800;
	required_shared_ptr<uint8_t> m_pcmram;

	uint8_t m_sound_ctrl;
	uint8_t m_sound_intck;

	uint32_t *m_dma_data_ptr;
	uint32_t m_dma_offset;
	int32_t m_dma_size;
	bool m_dma_is_write;
	bool m_dma_requested;

	void eeprom_w(uint16_t data);
	void pcmram_w(offs_t offset, uint8_t data);
	uint8_t pcmram_r(offs_t offset);
	uint16_t tms57002_status_word_r();
	void tms57002_control_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	INTERRUPT_GEN_MEMBER(tms_sync);
	void k054539_irq_gen(int state);

	TIMER_CALLBACK_MEMBER(scsi_dma_transfer);

	void scsi_dma_read( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size );
	void scsi_dma_write( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size );
	void scsi_drq(int state);

	void konamigq_dasp_map(address_map &map) ATTR_COLD;
	void konamigq_k054539_map(address_map &map) ATTR_COLD;
	void konamigq_map(address_map &map) ATTR_COLD;
	void konamigq_sound_map(address_map &map) ATTR_COLD;
};

/* EEPROM */

static const uint16_t konamigq_def_eeprom[64] =
{
	0x292b, 0x5256, 0x2094, 0x4155, 0x0041, 0x1414, 0x0003, 0x0101,
	0x0103, 0x0000, 0x0707, 0x0001, 0xaa00, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
};

void konamigq_state::eeprom_w(uint16_t data)
{
	ioport("EEPROMOUT")->write(data & 0x07, 0xff);
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ( data & 0x40 ) ? CLEAR_LINE : ASSERT_LINE );
}


/* PCM RAM */

void konamigq_state::pcmram_w(offs_t offset, uint8_t data)
{
	m_pcmram[ offset ] = data;
}

uint8_t konamigq_state::pcmram_r(offs_t offset)
{
	return m_pcmram[ offset ];
}

/* Video */

void konamigq_state::konamigq_map(address_map &map)
{
	map(0x1f000000, 0x1f00001f).m(m_ncr53cf96, FUNC(ncr53cf96_device::map)).umask16(0x00ff);
	map(0x1f100000, 0x1f10001f).rw(m_k056800, FUNC(k056800_device::host_r), FUNC(k056800_device::host_w)).umask32(0x00ff00ff);
	map(0x1f180000, 0x1f180001).w(FUNC(konamigq_state::eeprom_w));
	map(0x1f198000, 0x1f198003).nopw();            /* cabinet lamps? */
	map(0x1f1a0000, 0x1f1a0003).nopw();            /* indicates gun trigger */
	map(0x1f200000, 0x1f200003).portr("GUNX1");
	map(0x1f208000, 0x1f208003).portr("GUNY1");
	map(0x1f210000, 0x1f210003).portr("GUNX2");
	map(0x1f218000, 0x1f218003).portr("GUNY2");
	map(0x1f220000, 0x1f220003).portr("GUNX3");
	map(0x1f228000, 0x1f228003).portr("GUNY3");
	map(0x1f230000, 0x1f230003).portr("P1_P2");
	map(0x1f230004, 0x1f230007).portr("P3_SERVICE");
	map(0x1f238000, 0x1f238003).portr("DSW");
	map(0x1f300000, 0x1f5fffff).rw(FUNC(konamigq_state::pcmram_r), FUNC(konamigq_state::pcmram_w)).umask32(0x00ff00ff);
	map(0x1f680000, 0x1f68001f).rw("mb89371", FUNC(mb89371_device::read), FUNC(mb89371_device::write)).umask32(0x00ff00ff);
	map(0x1f780000, 0x1f780003).nopw(); /* watchdog? */
}

/* SOUND CPU */

INTERRUPT_GEN_MEMBER(konamigq_state::tms_sync)
{
	// DASP is synced to the LRCLK of the 058141
	if (m_sound_ctrl & 0x20)
		m_dasp->sync_w(1);
}

uint16_t konamigq_state::tms57002_status_word_r()
{
	return (m_dasp->dready_r() ? 4 : 0) |
		(m_dasp->pc0_r() ? 2 : 0) |
		(m_dasp->empty_r() ? 1 : 0);
}

void konamigq_state::tms57002_control_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if (!(data & 1))
			m_soundcpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);

		m_dasp->pload_w(data & 4);
		m_dasp->cload_w(data & 8);
		m_dasp->set_input_line(INPUT_LINE_RESET, data & 0x10 ? CLEAR_LINE : ASSERT_LINE);

		m_sound_ctrl = data;
	}
}

/* 68000 memory handling - near identical to Konami GX */
void konamigq_state::konamigq_sound_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x200000, 0x2004ff).rw("k054539_1", FUNC(k054539_device::read), FUNC(k054539_device::write)).umask16(0xff00);
	map(0x200000, 0x2004ff).rw("k054539_2", FUNC(k054539_device::read), FUNC(k054539_device::write)).umask16(0x00ff);
	map(0x300001, 0x300001).rw(m_dasp, FUNC(tms57002_device::data_r), FUNC(tms57002_device::data_w));
	map(0x400000, 0x40001f).rw(m_k056800, FUNC(k056800_device::sound_r), FUNC(k056800_device::sound_w)).umask16(0x00ff);
	map(0x500000, 0x500001).rw(FUNC(konamigq_state::tms57002_status_word_r), FUNC(konamigq_state::tms57002_control_word_w));
	map(0x580000, 0x580001).nopw(); // 'NRES' - D2: K056602 /RESET
}


/* TMS57002 memory handling */
void konamigq_state::konamigq_dasp_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
}


/* K058141 memory handling */
void konamigq_state::konamigq_k054539_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("k054539", 0);
	map(0x080000, 0x3fffff).ram().share("pcmram");
}


/* 058141 */
void konamigq_state::k054539_irq_gen(int state)
{
	if (m_sound_ctrl & 1)
	{
		// Trigger an interrupt on the rising edge
		if (!m_sound_intck && state)
			m_soundcpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);
	}

	m_sound_intck = state;
}

/* SCSI */

void konamigq_state::scsi_dma_read( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size )
{
	m_dma_data_ptr = p_n_psxram;
	m_dma_offset = n_address;
	m_dma_size = n_size * 4;
	m_dma_is_write = false;
	m_dma_timer->adjust(attotime::zero);
}

void konamigq_state::scsi_dma_write( uint32_t *p_n_psxram, uint32_t n_address, int32_t n_size )
{
	m_dma_data_ptr = p_n_psxram;
	m_dma_offset = n_address;
	m_dma_size = n_size * 4;
	m_dma_is_write = true;
	m_dma_timer->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(konamigq_state::scsi_dma_transfer)
{
	// TODO: Figure out proper DMA timings
	while (m_dma_requested && m_dma_data_ptr != nullptr && m_dma_size > 0)
	{
		if (m_dma_is_write)
			m_ncr53cf96->dma_w(util::little_endian_cast<const uint8_t>(m_dma_data_ptr)[m_dma_offset]);
		else
			util::little_endian_cast<uint8_t>(m_dma_data_ptr)[m_dma_offset] = m_ncr53cf96->dma_r();

		m_dma_offset++;
		m_dma_size--;
	}
}

void konamigq_state::scsi_drq(int state)
{
	if (!m_dma_requested && state)
		m_dma_timer->adjust(attotime::zero);

	m_dma_requested = state;
}

void konamigq_state::machine_start()
{
	save_item(NAME(m_sound_ctrl));
	save_item(NAME(m_sound_intck));

	m_dma_timer = timer_alloc(FUNC(konamigq_state::scsi_dma_transfer), this);
}

void konamigq_state::machine_reset()
{
	m_sound_ctrl = 0;
	m_sound_intck = 0;

	m_dma_timer->adjust(attotime::never);
	m_dma_data_ptr = nullptr;
	m_dma_offset = 0;
	m_dma_size = 0;
	m_dma_requested = m_dma_is_write = false;
}

void konamigq_state::konamigq(machine_config &config)
{
	/* basic machine hardware */
	CXD8530BQ(config, m_maincpu, XTAL(67'737'600));
	m_maincpu->set_addrmap(AS_PROGRAM, &konamigq_state::konamigq_map);
	m_maincpu->subdevice<psxdma_device>("dma")->install_read_handler(5, psxdma_device::read_delegate(&konamigq_state::scsi_dma_read, this));
	m_maincpu->subdevice<psxdma_device>("dma")->install_write_handler(5, psxdma_device::write_delegate(&konamigq_state::scsi_dma_write, this));
	m_maincpu->subdevice<ram_device>("ram")->set_default_size("4M");

	M68000(config, m_soundcpu, XTAL(32'000'000)/4); /* 8MHz - measured */
	m_soundcpu->set_addrmap(AS_PROGRAM, &konamigq_state::konamigq_sound_map);

	TMS57002(config, m_dasp, XTAL(48'000'000)/2); /* 24MHz - measured */
	m_dasp->set_addrmap(AS_DATA, &konamigq_state::konamigq_dasp_map);
	m_dasp->set_periodic_int(FUNC(konamigq_state::tms_sync), attotime::from_hz(48000));

	MB89371(config, "mb89371", 0);

	EEPROM_93C46_16BIT(config, "eeprom").default_data(konamigq_def_eeprom, 128);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0").option_set("harddisk", NSCSI_HARDDISK);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr53cf96", NCR53CF96).clock(32_MHz_XTAL/2).machine_config(
			[this] (device_t *device)
			{
				ncr53cf96_device &adapter = downcast<ncr53cf96_device &>(*device);
				adapter.irq_handler_cb().set(":maincpu:irq", FUNC(psxirq_device::intin10));
				adapter.drq_handler_cb().set(*this, FUNC(konamigq_state::scsi_drq));
			});

	/* video hardware */
	CXD8538Q(config, "gpu", XTAL(53'693'175), 0x200000, subdevice<psxcpu_device>("maincpu")).set_screen("screen");

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	K056800(config, m_k056800, XTAL(18'432'000));
	m_k056800->int_callback().set_inputline(m_soundcpu, M68K_IRQ_1);

	k054539_device &k054539_1(K054539(config, "k054539_1", XTAL(18'432'000)));
	k054539_1.set_addrmap(0, &konamigq_state::konamigq_k054539_map);
	k054539_1.timer_handler().set(FUNC(konamigq_state::k054539_irq_gen));
	k054539_1.add_route(0, "speaker", 1.0, 0);
	k054539_1.add_route(1, "speaker", 1.0, 1);

	k054539_device &k054539_2(K054539(config, "k054539_2", XTAL(18'432'000)));
	k054539_2.set_addrmap(0, &konamigq_state::konamigq_k054539_map);
	k054539_2.add_route(0, "speaker", 1.0, 0);
	k054539_2.add_route(1, "speaker", 1.0, 1);
}

static INPUT_PORTS_START( konamigq )
	PORT_START("GUNX1")
	PORT_BIT( 0x01ff, 0x011d, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x007d, 0x01bc ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNY1")
	PORT_BIT( 0x00ff, 0x0078, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("GUNX2")
	PORT_BIT( 0x01ff, 0x011d, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x007d, 0x01bc ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("GUNY2")
	PORT_BIT( 0x00ff, 0x0078, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("GUNX3")
	PORT_BIT( 0x01ff, 0x011d, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x007d, 0x01bc ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(3)

	PORT_START("GUNY3")
	PORT_BIT( 0x00ff, 0x0078, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0x00ef ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(3)

	PORT_START("P1_P2")
	PORT_BIT( 0x000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x000002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) /* trigger */
	PORT_BIT( 0x000010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) /* reload */
	PORT_BIT( 0x000020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x010000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x020000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x080000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* trigger */
	PORT_BIT( 0x100000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* reload */
	PORT_BIT( 0x200000, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x800000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_SERVICE")
	PORT_BIT( 0x000001, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x000002, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000008, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) /* trigger */
	PORT_BIT( 0x000010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) /* reload */
	PORT_BIT( 0x000020, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x010000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x040000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x200000, IP_ACTIVE_LOW )
	PORT_BIT( 0x400000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x800000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00000001, 0x01, DEF_STR( Stereo ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Stereo ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Mono ) )
	PORT_DIPNAME( 0x00000002, 0x00, "Stage Set" )
	PORT_DIPSETTING(    0x02, "Endless" )
	PORT_DIPSETTING(    0x00, "6st End" )
	PORT_DIPNAME( 0x00000004, 0x04, "Mirror" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00000008, 0x08, "Woofer" )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x00000010, 0x10, "Number of Players" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPNAME( 0x00000020, 0x20, "Coin Mechanism (2p only)" )
	PORT_DIPSETTING(    0x20, "Common" )
	PORT_DIPSETTING(    0x00, "Independent" )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::do_read))

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
INPUT_PORTS_END

ROM_START( cryptklr )
	ROM_REGION( 0x80000, "soundcpu", 0 ) /* 68000 sound program */
	ROM_LOAD16_WORD_SWAP( "420a01.2g", 0x000000, 0x080000, CRC(84fc2613) SHA1(e06f4284614d33c76529eb43b168d095200a9eac) )

	ROM_REGION( 0x80000, "k054539", 0 )
	ROM_LOAD( "420a02.3m",    0x000000, 0x080000, CRC(2169c3c4) SHA1(6d525f10385791e19eb1897d18f0bab319640162) )

	ROM_REGION32_LE( 0x080000, "maincpu:rom", 0 ) /* bios */
	ROM_LOAD( "420b03.27p",   0x0000000, 0x080000, CRC(aab391b1) SHA1(bf9dc7c0c8168c22a4be266fe6a66d3738df916b) )

	DISK_REGION( "scsi:0:harddisk" )
	DISK_IMAGE( "420uaa04", 0, SHA1(67cb1418fc0de2a89fc61847dc9efb9f1bebb347) )
ROM_END

} // Anonymous namespace


GAME( 1995, cryptklr, 0, konamigq, konamigq, konamigq_state, empty_init, ROT0, "Konami", "Crypt Killer (GQ420 UAA)", 0 )
