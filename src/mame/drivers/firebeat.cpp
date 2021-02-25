// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Konami FireBeat

    Driver by Ville Linde



    Hardware overview:

    GQ972 PWB(A2) 0000070609 Main board
    -----------------------------------
        OSC 66.0000MHz
        IBM PowerPC 403GCX at 66MHz
        (2x) Konami 0000057714 (2D object processor)
        Yamaha YMZ280B (ADPCM sound chip)
        Epson RTC65271 RTC/NVRAM
        National Semiconductor PC16552DV (dual UART)

    GQ974 PWB(A2) 0000070140 Extend board
    -------------------------------------
        ADC0808CCN
        FDC37C665GT (floppy disk controller)
        National Semiconductor PC16552DV (dual UART)
        ADM223LJR (RS-232 driver/receiver)

    GQ971 PWB(A2) 0000070254 SPU
    ----------------------------
        Motorola MC68HC000FN16 at 16MHz (?)
        Xilinx XC9572 CPLD
        Ricoh RF5c400 sound chip

    GQ971 PWB(B2) 0000067784 Backplane
    ----------------------------------
        3x PCB Slots with 2x DIN96S connectors (for main and extend PCBs)
        40-pin ATA connector for each slot

    GQ986 PWB(A1) 0000073015 Backplane
    ----------------------------------
        2x PCB Slots with 2x DIN96S connectors (for main and extend PCBs)
        40-pin ATA connector for each slot

    GQ972 PWB(D2) Controller interface on Beatmania III (?)
    GQ972 PWB(G1) Sound Amp (?)
    GQ971 PWB(C) Sound Amp


    Hardware configurations:

    Beatmania III
    -------------
        GQ971 Backplane
        GQ972 Main Board
        GQ974 Extend Board
        GQ971 SPU
        GQ972 Controller Interface
        GQ972 Sound Amp
        Hard drive in Slot 1
        DVD drive in Slot 2

    Keyboardmania
    -------------
        GQ971 Backplane
        GQ972 Main Board
        GQ974 Extend Board
        Yamaha XT446 board (for keyboard sounds) (the board layout matches the Yamaha MU100 Tone Generator)
        GQ971 Sound Amp
        CD-ROM drive in Slot 1
        CD-ROM drive in Slot 2

    ParaParaParadise
    ----------------
        GQ986 Backplane
        GQ972 Main Board
        2x CD-ROM drive in Slot 1



    Games that run on this hardware:

    BIOS       Game ID        Year    Game
    ------------------------------------------------------------------
    GQ972      GQ972          2000    Beatmania III
    GQ972      GCA05          2000    Beatmania III Append Core Remix
    GCA21      GCA21          2001    Beatmania III Append 6th Mix
    GCA21      GCB07          2002    Beatmania III Append 7th Mix
    GCA21      GCC01          2003    Beatmania III The Final
    GQ974      GQ974          2000    Keyboardmania
    GQ974      GCA01          2000    Keyboardmania 2nd Mix
    GQ974      GCA12          2001    Keyboardmania 3rd Mix
    GQ977      GQ977          2000    Para Para Paradise
    GQ977      GQ977          2000    Para Para Dancing
    GQ977      GC977          2000    Para Para Paradise 1.1
    GQ977      GQA11          2000    Para Para Paradise 1st Mix+
    GQA02(?)   GQ986          2000    Pop'n Music 4
    ???        G?A04          2000    Pop'n Music 5
    ???        GQA16          2001    Pop'n Music 6
    GQA02      GCB00          2001    Pop'n Music 7
    ???        GQB30          2002    Pop'n Music 8
    ???        GQ976          2000    Pop'n Music Mickey Tunes
    ???        GQ976          2000    Pop'n Music Mickey Tunes!
    ???        GQ987          2000    Pop'n Music Animelo
    ???        GEA02          2001    Pop'n Music Animelo 2

Dumpable pieces missing
-----------------------
Beatmania III - CD, HDD, EPROM on main board, EPROM on SPU board
Beatmania III Append 6th Mix - HDD, dongle, EPROM on main board, EPROM on SPU board
Para Para Paradise - dongle, program CD
Para Para Paradise 1.1 - dongle
Para Para Dancing - dongle, program CD
Pop'n Music Animelo - dongle, CD, DVD
Keyboard Mania  - dongle, program CD, audio CD
Keyboard Mania 2nd Mix - dongle, program CD, audio CD

    TODO:
        - The external Yamaha MIDI sound board is not emulated (no keyboard sounds).


        - Notes on how the video is supposed to work from Ville / Ian Patterson:

        There are four "display contexts" that are set up via registers 20-4E. They are
        basically just raw framebuffers. 40-4E sets the base framebuffer pointer, 30-3E
        sets the size, 20-2E may set the minimum x and y coordinates but I haven't seen
        them set to something other than 0 yet. One context is set as the one the RAMDAC
        outputs to the monitor (not sure how this is selected yet, probably the lower
        bits of register 12). Thestartup test in the popn BIOS checks all of VRAM, so
        it moves the currentdisplay address around so you don't see crazy colors, which
        is very helpful in figuring out how this part works.

        The other new part is that there are two VRAM write ports, managed by registers
        60+68+70 and 64+6A+74, with status read from the lower bits of reg 7A. Each context
        can either write to VRAM as currently emulated, or the port can be switched in to
        "immediate mode" via registers 68/6A. Immedate mode can be used to run GCU commands
        at any point during the frame. It's mainly used to call display lists, which is where
        the display list addresses come from. Some games use it to send other commands, so
        it appears to be a 4-dword FIFO or something along those lines.
*/

#include "emu.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/atapicdr.h"
#include "bus/ata/idehd.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "machine/ins8250.h"
#include "machine/intelfsh.h"
#include "machine/mb8421.h"
#include "machine/midikbd.h"
#include "machine/rtc65271.h"
#include "machine/timer.h"
#include "sound/cdda.h"
#include "sound/rf5c400.h"
#include "sound/ymz280b.h"
#include "video/k057714.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "osdcomm.h"

#include "firebeat.lh"


namespace {

struct IBUTTON_SUBKEY
{
	uint8_t identifier[8];
	uint8_t password[8];
	uint8_t data[0x30];
};

struct IBUTTON
{
	IBUTTON_SUBKEY subkey[3];
};

/*****************************************************************************/
static void firebeat_ata_devices(device_slot_interface &device)
{
	device.option_add("cdrom", ATAPI_FIXED_CDROM);
	device.option_add("hdd", IDE_HARDDISK);
}

static void cdrom_config(device_t *device)
{
	device->subdevice<cdda_device>("cdda")->add_route(0, "^^lspeaker", 0.5);
	device->subdevice<cdda_device>("cdda")->add_route(1, "^^rspeaker", 0.5);
}

static void dvdrom_config(device_t *device)
{
	downcast<atapi_cdrom_device &>(*device).set_ultra_dma_mode(0x0102);
}

class firebeat_state : public driver_device
{
public:
	firebeat_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_work_ram(*this, "work_ram"),
		m_ata(*this, "ata"),
		m_gcu(*this, "gcu"),
		m_duart_com(*this, "duart_com"),
		m_status_leds(*this, "status_led_%u", 0U),
		m_io_inputs(*this, "IN%u", 0U)
	{ }

	void firebeat(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void device_resolve_objects() override;

	uint32_t screen_update_firebeat_0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void init_firebeat();

	void firebeat_map(address_map &map);
	void ymz280b_map(address_map &map);

	void init_lights(write32s_delegate out1, write32s_delegate out2, write32s_delegate out3);
	void lamp_output_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void lamp_output2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void lamp_output3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	INTERRUPT_GEN_MEMBER(firebeat_interrupt);
	DECLARE_WRITE_LINE_MEMBER(ata_interrupt);
	DECLARE_WRITE_LINE_MEMBER(gcu_interrupt);
	DECLARE_WRITE_LINE_MEMBER(sound_irq_callback);

	int m_cabinet_info;

	uint8_t m_extend_board_irq_enable;
	uint8_t m_extend_board_irq_active;

	required_device<ppc4xx_device> m_maincpu;
	required_shared_ptr<uint32_t> m_work_ram;
	required_device<ata_interface_device> m_ata;
	required_device<k057714_device> m_gcu;

private:
	uint32_t cabinet_r(offs_t offset, uint32_t mem_mask = ~0);

	void set_ibutton(uint8_t *data);
	int ibutton_w(uint8_t data);
	void security_w(uint8_t data);

	uint8_t extend_board_irq_r(offs_t offset);
	void extend_board_irq_w(offs_t offset, uint8_t data);

	uint8_t input_r(offs_t offset);

	uint16_t ata_command_r(offs_t offset, uint16_t mem_mask = ~0);
	void ata_command_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t ata_control_r(offs_t offset, uint16_t mem_mask = ~0);
	void ata_control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

//  uint32_t comm_uart_r(offs_t offset, uint32_t mem_mask = ~ 0);
//  void comm_uart_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	IBUTTON m_ibutton;
	int m_ibutton_state;
	int m_ibutton_read_subkey_ptr;
	uint8_t m_ibutton_subkey_data[0x40];

	required_device<pc16552_device> m_duart_com;

	output_finder<8> m_status_leds;

	required_ioport_array<4> m_io_inputs;
};

class firebeat_spu_state : public firebeat_state
{
public:
	firebeat_spu_state(const machine_config &mconfig, device_type type, const char *tag) :
		firebeat_state(mconfig, type, tag),
		m_spuata(*this, "spu_ata"),
		m_audiocpu(*this, "audiocpu"),
		m_dpram(*this, "spuram"),
		m_waveram(*this, "rf5c400"),
		m_spu_status_leds(*this, "spu_status_led_%u", 0U)
	{ }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_resolve_objects() override;

	void firebeat_spu_base(machine_config &config);
	void firebeat_spu_map(address_map &map);
	void spu_map(address_map &map);
	void rf5c400_map(address_map& map);

	DECLARE_WRITE_LINE_MEMBER(spu_ata_dmarq);
	DECLARE_WRITE_LINE_MEMBER(spu_ata_interrupt);
	TIMER_CALLBACK_MEMBER(spu_dma_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(spu_timer_callback);

	required_device<ata_interface_device> m_spuata;

private:
	void spu_status_led_w(uint16_t data);
	void spu_irq_ack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void spu_ata_dma_low_w(uint16_t data);
	void spu_ata_dma_high_w(uint16_t data);
	void spu_wavebank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t firebeat_waveram_r(offs_t offset);
	void firebeat_waveram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	emu_timer *m_dma_timer;
	bool m_sync_ata_irq;

	uint32_t m_spu_ata_dma;
	int m_spu_ata_dmarq;
	uint32_t m_wave_bank;

	required_device<m68000_device> m_audiocpu;
	required_device<cy7c131_device> m_dpram;
	required_shared_ptr<uint16_t> m_waveram;

	output_finder<8> m_spu_status_leds;
};

/*****************************************************************************/

class firebeat_ppp_state : public firebeat_state
{
public:
	firebeat_ppp_state(const machine_config &mconfig, device_type type, const char *tag) :
		firebeat_state(mconfig, type, tag),
		m_stage_leds(*this, "stage_led_%u", 0U),
		m_top_leds(*this, "top_led_%u", 0U),
		m_lamps(*this, "lamp_%u", 0U),
		m_cab_led_left(*this, "left"),
		m_cab_led_right(*this, "right"),
		m_cab_led_door_lamp(*this, "door_lamp"),
		m_cab_led_ok(*this, "ok"),
		m_cab_led_slim(*this, "slim"),
		m_io_sensors(*this, "SENSOR%u", 1U)
	{ }

	void firebeat_ppp(machine_config &config);
	void init_ppd();
	void init_ppp();

private:
	virtual void device_resolve_objects() override;

	void firebeat_ppp_map(address_map &map);

	uint16_t sensor_r(offs_t offset);

	void lamp_output_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void lamp_output2_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void lamp_output3_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	output_finder<8> m_stage_leds;
	output_finder<8> m_top_leds;
	output_finder<4> m_lamps;
	output_finder<> m_cab_led_left;
	output_finder<> m_cab_led_right;
	output_finder<> m_cab_led_door_lamp;
	output_finder<> m_cab_led_ok;
	output_finder<> m_cab_led_slim;

	required_ioport_array<4> m_io_sensors;
};

class firebeat_kbm_state : public firebeat_state
{
public:
	firebeat_kbm_state(const machine_config &mconfig, device_type type, const char *tag) :
		firebeat_state(mconfig, type, tag),
		m_duart_midi(*this, "duart_midi"),
		m_kbd(*this, "kbd%u", 0),
		m_gcu_sub(*this, "gcu_sub"),
		m_lamps(*this, "lamp_%u", 1U),
		m_cab_led_door_lamp(*this, "door_lamp"),
		m_cab_led_start1p(*this, "start1p"),
		m_cab_led_start2p(*this, "start2p"),
		m_lamp_neon(*this, "neon"),
		m_io_wheels(*this, "WHEEL_P%u", 1U)
	{ }

	uint32_t screen_update_firebeat_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void init_kbm();
	void firebeat_kbm(machine_config &config);

private:
	virtual void device_resolve_objects() override;

	void firebeat_kbm_map(address_map &map);

	void init_keyboard();

	uint8_t keyboard_wheel_r(offs_t offset);
	uint8_t midi_uart_r(offs_t offset);
	void midi_uart_w(offs_t offset, uint8_t data);

	void lamp_output_kbm_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

//  TIMER_CALLBACK_MEMBER(keyboard_timer_callback);
	DECLARE_WRITE_LINE_MEMBER(midi_uart_ch0_irq_callback);
	DECLARE_WRITE_LINE_MEMBER(midi_uart_ch1_irq_callback);

//  emu_timer *m_keyboard_timer;
//  int m_keyboard_state[2];

	required_device<pc16552_device> m_duart_midi;
	required_device_array<midi_keyboard_device, 2> m_kbd;
	required_device<k057714_device> m_gcu_sub;

	output_finder<3> m_lamps;
	output_finder<> m_cab_led_door_lamp;
	output_finder<> m_cab_led_start1p;
	output_finder<> m_cab_led_start2p;
	output_finder<> m_lamp_neon;

	required_ioport_array<2> m_io_wheels;
};

class firebeat_bm3_state : public firebeat_spu_state
{
public:
	firebeat_bm3_state(const machine_config &mconfig, device_type type, const char *tag) :
		firebeat_spu_state(mconfig, type, tag),
		m_io(*this, "IO%u", 1U),
		m_io_turntables(*this, "TURNTABLE_P%u", 1U),
		m_io_effects(*this, "EFFECT%u", 1U)
	{ }

	void firebeat_bm3(machine_config &config);
	void init_bm3();

private:
	void firebeat_bm3_map(address_map &map);

	uint32_t spectrum_analyzer_r(offs_t offset);
	uint16_t sensor_r(offs_t offset);

	// TODO: Floppy disk implementation
	uint32_t fdd_unk_r(offs_t offset, uint32_t mem_mask = ~0);
	void fdd_unk_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	DECLARE_WRITE_LINE_MEMBER( bm3_vblank );

	required_ioport_array<4> m_io;
	required_ioport_array<2> m_io_turntables;
	required_ioport_array<7> m_io_effects;
};

class firebeat_popn_state : public firebeat_spu_state
{
public:
	firebeat_popn_state(const machine_config &mconfig, device_type type, const char *tag) :
		firebeat_spu_state(mconfig, type, tag)
	{ }

	void firebeat_popn(machine_config &config);
	void init_popn();
};

/*****************************************************************************/

uint32_t firebeat_state::screen_update_firebeat_0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return m_gcu->draw(screen, bitmap, cliprect); }
uint32_t firebeat_kbm_state::screen_update_firebeat_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return m_gcu_sub->draw(screen, bitmap, cliprect); }

void firebeat_state::machine_start()
{
	/* set conservative DRC options */
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x01ffffff, false, m_work_ram);
}

void firebeat_state::device_resolve_objects()
{
	m_status_leds.resolve();
}

void firebeat_state::init_firebeat()
{
	uint8_t *rom = memregion("user2")->base();

//  pc16552d_init(machine(), 0, 19660800, comm_uart_irq_callback, 0);     // Network UART

	m_extend_board_irq_enable = 0x3f;
	m_extend_board_irq_active = 0x00;

	m_cabinet_info = 0;

	m_maincpu->ppc4xx_spu_set_tx_handler(write8smo_delegate(*this, FUNC(firebeat_state::security_w)));

	set_ibutton(rom);

	init_lights(write32s_delegate(*this), write32s_delegate(*this), write32s_delegate(*this));
}

void firebeat_state::firebeat(machine_config &config)
{
	/* basic machine hardware */
	PPC403GCX(config, m_maincpu, XTAL(66'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &firebeat_state::firebeat_map);
	m_maincpu->set_vblank_int("screen", FUNC(firebeat_state::firebeat_interrupt));

	RTC65271(config, "rtc", 0);

	FUJITSU_29F016A(config, "flash_main");
	FUJITSU_29F016A(config, "flash_snd1");
	FUJITSU_29F016A(config, "flash_snd2");

	ATA_INTERFACE(config, m_ata).options(firebeat_ata_devices, "cdrom", "cdrom", true);
	m_ata->irq_handler().set(FUNC(firebeat_state::ata_interrupt));
	m_ata->slot(1).set_option_machine_config("cdrom", cdrom_config);

	/* video hardware */
	PALETTE(config, "palette", palette_device::RGB_555);

	K057714(config, m_gcu, 0);
	m_gcu->irq_callback().set(FUNC(firebeat_state::gcu_interrupt));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 384);
	screen.set_visarea(0, 511, 0, 383);
	screen.set_screen_update(FUNC(firebeat_state::screen_update_firebeat_0));
	screen.set_palette("palette");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16934400));
	ymz.irq_handler().set(FUNC(firebeat_state::sound_irq_callback));
	ymz.set_addrmap(0, &firebeat_state::ymz280b_map);
	ymz.add_route(1, "lspeaker", 1.0);
	ymz.add_route(0, "rspeaker", 1.0);

	PC16552D(config, "duart_com", 0);
	NS16550(config, "duart_com:chan0", XTAL(19'660'800));
	NS16550(config, "duart_com:chan1", XTAL(19'660'800));
}

void firebeat_state::firebeat_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram().share("work_ram");
	map(0x70006000, 0x70006003).w(FUNC(firebeat_state::extend_board_irq_w));
	map(0x7000a000, 0x7000a003).r(FUNC(firebeat_state::extend_board_irq_r));
	map(0x74000000, 0x740003ff).noprw(); // SPU shared RAM
	map(0x7d000200, 0x7d00021f).r(FUNC(firebeat_state::cabinet_r));
	map(0x7d000400, 0x7d000401).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
	map(0x7d000800, 0x7d000803).r(FUNC(firebeat_state::input_r));
	map(0x7d400000, 0x7d5fffff).rw("flash_main", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write));
	map(0x7d800000, 0x7d9fffff).rw("flash_snd1", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write));
	map(0x7da00000, 0x7dbfffff).rw("flash_snd2", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write));
	map(0x7dc00000, 0x7dc0000f).rw(m_duart_com, FUNC(pc16552_device::read), FUNC(pc16552_device::write));
	map(0x7e000000, 0x7e00003f).rw("rtc", FUNC(rtc65271_device::rtc_r), FUNC(rtc65271_device::rtc_w));
	map(0x7e000100, 0x7e00013f).rw("rtc", FUNC(rtc65271_device::xram_r), FUNC(rtc65271_device::xram_w));
	map(0x7e800000, 0x7e8000ff).rw(m_gcu, FUNC(k057714_device::read), FUNC(k057714_device::write));
	map(0x7e800100, 0x7e8001ff).noprw(); // Secondary GCU, only used by Keyboardmania but is written to during the bootloader of other games
	map(0x7fe00000, 0x7fe0000f).rw(FUNC(firebeat_state::ata_command_r), FUNC(firebeat_state::ata_command_w));
	map(0x7fe80000, 0x7fe8000f).rw(FUNC(firebeat_state::ata_control_r), FUNC(firebeat_state::ata_control_w));
	map(0x7ff80000, 0x7fffffff).rom().region("user1", 0);       /* System BIOS */
}

void firebeat_state::ymz280b_map(address_map &map)
{
	map.global_mask(0x3fffff);
	map(0x000000, 0x1fffff).r("flash_snd1", FUNC(fujitsu_29f016a_device::read));
	map(0x200000, 0x3fffff).r("flash_snd2", FUNC(fujitsu_29f016a_device::read));
}

/*****************************************************************************/

uint32_t firebeat_state::cabinet_r(offs_t offset, uint32_t mem_mask)
{
//  printf("cabinet_r: %08X, %08X\n", offset, mem_mask);
	switch (offset)
	{
		case 0: return m_cabinet_info << 28;

		// These never seem to be anything other than 0?
		case 2: return 0;
		case 4: return 0;
	}

	return 0;
}

/*****************************************************************************/

/* Security dongle is a Dallas DS1411 RS232 Adapter with a DS1991 Multikey iButton */

/* popn7 supports 8 different dongles:
   - Manufacture
   - Service
   - Event
   - Oversea
   - No Hardware
   - Rental
   - Debug
   - Normal
*/

enum
{
	DS1991_STATE_NORMAL,
	DS1991_STATE_READ_SUBKEY
};

void firebeat_state::set_ibutton(uint8_t *data)
{
	int i, j;

	for (i=0; i < 3; i++)
	{
		// identifier
		for (j=0; j < 8; j++)
		{
			m_ibutton.subkey[i].identifier[j] = *data++;
		}

		// password
		for (j=0; j < 8; j++)
		{
			m_ibutton.subkey[i].password[j] = *data++;
		}

		// data
		for (j=0; j < 48; j++)
		{
			m_ibutton.subkey[i].data[j] = *data++;
		}
	}
}

int firebeat_state::ibutton_w(uint8_t data)
{
	int r = -1;

	switch (m_ibutton_state)
	{
		case DS1991_STATE_NORMAL:
		{
			switch (data)
			{
				//
				// DS2408B Serial 1-Wire Line Driver with Load Sensor
				//
				case 0xc1:          // DS2480B reset
				{
					r = 0xcd;
					break;
				}
				case 0xe1:          // DS2480B set data mode
				{
					break;
				}
				case 0xe3:          // DS2480B set command mode
				{
					break;
				}

				//
				// DS1991 MultiKey iButton
				//
				case 0x66:          // DS1991 Read SubKey
				{
					r = 0x66;
					m_ibutton_state = DS1991_STATE_READ_SUBKEY;
					m_ibutton_read_subkey_ptr = 0;
					break;
				}
				case 0xcc:          // DS1991 skip rom
				{
					r = 0xcc;
					m_ibutton_state = DS1991_STATE_NORMAL;
					break;
				}
				default:
				{
					fatalerror("ibutton: unknown normal mode cmd %02X\n", data);
				}
			}
			break;
		}

		case DS1991_STATE_READ_SUBKEY:
		{
			if (m_ibutton_read_subkey_ptr == 0)      // Read SubKey, 2nd command byte
			{
				int subkey = (data >> 6) & 0x3;
		//      printf("iButton SubKey %d\n", subkey);
				r = data;

				if (subkey < 3)
				{
					memcpy(&m_ibutton_subkey_data[0],  m_ibutton.subkey[subkey].identifier, 8);
					memcpy(&m_ibutton_subkey_data[8],  m_ibutton.subkey[subkey].password, 8);
					memcpy(&m_ibutton_subkey_data[16], m_ibutton.subkey[subkey].data, 0x30);
				}
				else
				{
					memset(&m_ibutton_subkey_data[0], 0, 0x40);
				}
			}
			else if (m_ibutton_read_subkey_ptr == 1) // Read SubKey, 3rd command byte
			{
				r = data;
			}
			else
			{
				r = m_ibutton_subkey_data[m_ibutton_read_subkey_ptr-2];
			}
			m_ibutton_read_subkey_ptr++;
			if (m_ibutton_read_subkey_ptr >= 0x42)
			{
				m_ibutton_state = DS1991_STATE_NORMAL;
			}
			break;
		}
	}

	return r;
}

void firebeat_state::security_w(uint8_t data)
{
	int r = ibutton_w(data);
	if (r >= 0)
		m_maincpu->ppc4xx_spu_receive_byte(r);
}

/*****************************************************************************/

// Extend board IRQs
// 0x01: MIDI UART channel 2
// 0x02: MIDI UART channel 1
// 0x04: ?
// 0x08: ?
// 0x10: ?
// 0x20: ?

uint8_t firebeat_state::extend_board_irq_r(offs_t offset)
{
	return ~m_extend_board_irq_active;
}

void firebeat_state::extend_board_irq_w(offs_t offset, uint8_t data)
{
//  printf("extend_board_irq_w: %08X, %08X\n", data, offset);

	m_extend_board_irq_active &= ~(data & 0xff);
	m_extend_board_irq_enable = data & 0xff;
}

/*****************************************************************************/

uint8_t firebeat_state::input_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return (m_io_inputs[0]->read() & 0xff);
		case 1: return (m_io_inputs[1]->read() & 0xff);
		case 2: return (m_io_inputs[2]->read() & 0xff);
		case 3: return (m_io_inputs[3]->read() & 0xff);
	}

	return 0;
}

/*****************************************************************************/
/* ATA Interface */

uint16_t firebeat_state::ata_command_r(offs_t offset, uint16_t mem_mask)
{
// printf("ata_command_r: %08X, %08X\n", offset, mem_mask);
	uint16_t r = m_ata->cs0_r(offset, swapendian_int16(mem_mask));
	return swapendian_int16(r);
}

void firebeat_state::ata_command_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
//  printf("ata_command_w: %08X, %08X, %08X\n", data, offset, mem_mask);
	m_ata->cs0_w(offset, swapendian_int16(data & 0xffff), swapendian_int16(mem_mask & 0xffff));
}


uint16_t firebeat_state::ata_control_r(offs_t offset, uint16_t mem_mask)
{
//  printf("ata_control_r: %08X, %08X\n", offset, mem_mask);
	uint16_t r = m_ata->cs1_r(offset, swapendian_int16(mem_mask));
	return swapendian_int16(r);
}

void firebeat_state::ata_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata->cs1_w(offset, swapendian_int16(data & 0xffff), swapendian_int16(mem_mask & 0xffff));
}


/*****************************************************************************/

/*
uint32_t firebeat_state::comm_uart_r(offs_t offset, uint32_t mem_mask)
{
    uint32_t r = 0;

    if (ACCESSING_BITS_24_31)
    {
        r |= pc16552d_0_r((offset*4)+0) << 24;
    }
    if (ACCESSING_BITS_16_23)
    {
        r |= pc16552d_0_r((offset*4)+1) << 16;
    }
    if (ACCESSING_BITS_8_15)
    {
        r |= pc16552d_0_r((offset*4)+2) << 8;
    }
    if (ACCESSING_BITS_0_7)
    {
        r |= pc16552d_0_r((offset*4)+3) << 0;
    }

    return r;
}

void firebeat_state::comm_uart_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
    if (ACCESSING_BITS_24_31)
    {
        pc16552d_0_w((offset*4)+0, (data >> 24) & 0xff);
    }
    if (ACCESSING_BITS_16_23)
    {
        pc16552d_0_w((offset*4)+1, (data >> 16) & 0xff);
    }
    if (ACCESSING_BITS_8_15)
    {
        pc16552d_0_w((offset*4)+2, (data >> 8) & 0xff);
    }
    if (ACCESSING_BITS_0_7)
    {
        pc16552d_0_w((offset*4)+3, (data >> 0) & 0xff);
    }
}

static void comm_uart_irq_callback(running_machine &machine, int channel, int value)
{
    // TODO
    //m_maincpu->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
}
*/


/*****************************************************************************/

void firebeat_state::init_lights(write32s_delegate out1, write32s_delegate out2, write32s_delegate out3)
{
	if (out1.isnull()) out1 = write32s_delegate(*this, FUNC(firebeat_state::lamp_output_w));
	if (out2.isnull()) out2 = write32s_delegate(*this, FUNC(firebeat_state::lamp_output2_w));
	if (out3.isnull()) out3 = write32s_delegate(*this, FUNC(firebeat_state::lamp_output3_w));

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x7d000804, 0x7d000807, out1);
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x7d000320, 0x7d000323, out2);
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x7d000324, 0x7d000327, out3);
}

void firebeat_state::lamp_output_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// -------- -------- -------- xxxxxxxx   Status LEDs (active low)
	if (ACCESSING_BITS_0_7)
	{
		m_status_leds[0] = BIT(~data, 0);
		m_status_leds[1] = BIT(~data, 1);
		m_status_leds[2] = BIT(~data, 2);
		m_status_leds[3] = BIT(~data, 3);
		m_status_leds[4] = BIT(~data, 4);
		m_status_leds[5] = BIT(~data, 5);
		m_status_leds[6] = BIT(~data, 6);
		m_status_leds[7] = BIT(~data, 7);
	}

//  printf("lamp_output_w: %08X, %08X, %08X\n", data, offset, mem_mask);
}

void firebeat_state::lamp_output2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("lamp_output2_w: %08X, %08X, %08X\n", data, offset, mem_mask);
}

void firebeat_state::lamp_output3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("lamp_output3_w: %08X, %08X, %08X\n", data, offset, mem_mask);
}

/*****************************************************************************/

INTERRUPT_GEN_MEMBER(firebeat_state::firebeat_interrupt)
{
	// IRQs
	// IRQ 0: VBlank
	// IRQ 1: Extend board IRQ
	// IRQ 2: Main board UART
	// IRQ 3: SPU mailbox interrupt
	// IRQ 4: ATA

	device.execute().set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

WRITE_LINE_MEMBER(firebeat_state::ata_interrupt)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ4, state);
}

WRITE_LINE_MEMBER(firebeat_state::gcu_interrupt)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

WRITE_LINE_MEMBER(firebeat_state::sound_irq_callback)
{
}

/*****************************************************************************/

void firebeat_spu_state::machine_start()
{
	m_dma_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(firebeat_spu_state::spu_dma_callback), this));
}

void firebeat_spu_state::machine_reset()
{
	m_spu_ata_dma = 0;
	m_spu_ata_dmarq = 0;
	m_wave_bank = 0;
	m_sync_ata_irq = 0;
}

void firebeat_spu_state::device_resolve_objects()
{
	firebeat_state::device_resolve_objects();
	m_spu_status_leds.resolve();
}

void firebeat_spu_state::firebeat_spu_base(machine_config &config)
{
	firebeat(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &firebeat_spu_state::firebeat_spu_map);

	M68000(config, m_audiocpu, 16000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &firebeat_spu_state::spu_map);

	CY7C131(config, m_dpram);
	m_dpram->intl_callback().set_inputline(m_audiocpu, INPUT_LINE_IRQ4); // address 0x3fe triggers M68K interrupt
	m_dpram->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ3); // address 0x3ff triggers PPC interrupt

	rf5c400_device &rf5c400(RF5C400(config, "rf5c400", XTAL(16'934'400)));
	rf5c400.set_addrmap(0, &firebeat_spu_state::rf5c400_map);
	rf5c400.add_route(0, "lspeaker", 0.5);
	rf5c400.add_route(1, "rspeaker", 0.5);
}

void firebeat_spu_state::firebeat_spu_map(address_map &map)
{
	firebeat_map(map);
	map(0x74000000, 0x740003ff).rw(m_dpram, FUNC(cy7c131_device::right_r), FUNC(cy7c131_device::right_w)); // SPU shared RAM
}

void firebeat_spu_state::spu_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x13ffff).ram();
	map(0x200000, 0x200001).portr("SPU_DSW");
	map(0x220000, 0x220001).w(FUNC(firebeat_spu_state::spu_status_led_w));
	map(0x230000, 0x230001).w(FUNC(firebeat_spu_state::spu_irq_ack_w));
	map(0x240000, 0x240003).w(FUNC(firebeat_spu_state::spu_ata_dma_low_w)).nopr();
	map(0x250000, 0x250003).w(FUNC(firebeat_spu_state::spu_ata_dma_high_w)).nopr();
	map(0x260000, 0x260001).w(FUNC(firebeat_spu_state::spu_wavebank_w)).nopr();
	map(0x280000, 0x2807ff).rw(m_dpram, FUNC(cy7c131_device::left_r), FUNC(cy7c131_device::left_w)).umask16(0x00ff);
	map(0x300000, 0x30000f).rw(m_spuata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));
	map(0x340000, 0x34000f).rw(m_spuata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w));
	map(0x400000, 0x400fff).rw("rf5c400", FUNC(rf5c400_device::rf5c400_r), FUNC(rf5c400_device::rf5c400_w));
	map(0x800000, 0xffffff).rw(FUNC(firebeat_spu_state::firebeat_waveram_r), FUNC(firebeat_spu_state::firebeat_waveram_w));
}

void firebeat_spu_state::rf5c400_map(address_map& map)
{
	map(0x0000000, 0x1ffffff).ram().share("rf5c400");
}


/*  SPU board M68K IRQs

    IRQ1: Executes all commands stored in a buffer.
          The buffer can contain up to 8 commands.
          This seems to be unused.

    IRQ2: Executes one command stored in a different buffer from IRQ1.
          The buffer can contain up to 8 commands.
          The command index counter increments after each IRQ2 call.
          If there is no command in the slot at the current counter then it just increments without executing a command.

          Timing matters. In particular if the speed of the IRQ 2 calls is too fast then the volume and frequency animations will be wrong.
          The most common issue with bad timing is keysounds will be cut off.
          pop'n music Animelo 2 also has an issue when playing CHA-LA HEAD CHA-LA where one of the beginning keysounds will stay on a
          very high pitched frequency. The lower(?) the IRQ 2 frequency, the longer the keysound stays played it seems.

          For beatmania III:
            cmd[0] = nop
            cmd[1] = 0x91bc -> Send stop command for all rf5c400 channels that are done playing
            cmd[2] = 0x310a -> Error checking? Sending some kind of state to main CPU???
            cmd[3] = 0x29c6 -> Increment a timer for each running DMA(ATA command?)
                Each timer must count up to 0x02e8 (744) before it will move on to the next DMA, which I believe is the time out counter.

                In another part of the program (0x363c for a21jca03.bin) is the following code for determining when to start and stop the DMA:

                start_dma();
                while (get_dma_timer() < dma_max_timer) {
                    if (irq6_called_flag) {
                        break;
                    }
                }
                end_dma();

                irq6_called_flag is set only when IRQ6 is called.
                get_dma_timer is the timer that is incremented by 0x29c6.
            cmd[4] = 0x94de -> Animates rf5c400 channel volumes
            cmd[5] = 0x7b2c -> Send some kind of buffer status flags to spu_status_led_w. Related to IRQ4 since commands come from PPC to set buffer data
            cmd[6] = 0x977e -> Animates rf5c400 channel frequencies
            cmd[7] = 0x9204 -> Sends current state of rf5c400 channels as well as a list (bitmask integer) of usable channels up to main CPU memory.
                               Also sends a flag to to spu_status_led_w that shows if there are available SE slots.
                               If there are no available SE slots then it will set bit 3 to .

    IRQ4: Dual-port RAM mailbox (when PPC writes to 0x3FE)
          Handles commands from PPC (bytes 0x00 and 0x01)

    IRQ6: ATA
*/

void firebeat_spu_state::spu_status_led_w(uint16_t data)
{
	// Verified with real hardware that the patterns match the status LEDs on the board

	// Set when clearing waveram memory during initialization:
	// uint16_t bank = ((~data) >> 6) & 3;
	// uint16_t offset = (!!((~data) & (1 << 5))) * 8192;
	// uint16_t verify = !!((~data) & (1 << 4)); // 0 = writing memory, 1 = verifying memory

	// For IRQ2:
	// Command 5 (0x7b2c):
	// if buffer[4] == 0 and buffer[0] < buffer[5], it writes what buffer(thread?) is currently busy(?)
	// There are 8 buffers/threads total
	//
	// Command 7 (0x9204):
	// If all 30 SE channels are in use, bit 3 will be set to 1

	// -------- -------- -------- xxxxxxxx   Status LEDs (active low)
	m_spu_status_leds[0] = BIT(~data, 0);
	m_spu_status_leds[1] = BIT(~data, 1);
	m_spu_status_leds[2] = BIT(~data, 2);
	m_spu_status_leds[3] = BIT(~data, 3);
	m_spu_status_leds[4] = BIT(~data, 4);
	m_spu_status_leds[5] = BIT(~data, 5);
	m_spu_status_leds[6] = BIT(~data, 6);
	m_spu_status_leds[7] = BIT(~data, 7);
}

void firebeat_spu_state::spu_irq_ack_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if (BIT(data, 0))
			m_audiocpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
		if (BIT(data, 1))
			m_audiocpu->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
		if (BIT(data, 3))
			m_audiocpu->set_input_line(INPUT_LINE_IRQ6, CLEAR_LINE);
	}
}

void firebeat_spu_state::spu_ata_dma_low_w(uint16_t data)
{
	m_spu_ata_dma = (m_spu_ata_dma & ~0xffff) | data;
}

void firebeat_spu_state::spu_ata_dma_high_w(uint16_t data)
{
	m_spu_ata_dma = (m_spu_ata_dma & 0xffff) | ((uint32_t)data << 16);
}

void firebeat_spu_state::spu_wavebank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_wave_bank = data * (4 * 1024 * 1024);
}

uint16_t firebeat_spu_state::firebeat_waveram_r(offs_t offset)
{
	return m_waveram[offset + m_wave_bank];
}

void firebeat_spu_state::firebeat_waveram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_waveram[offset + m_wave_bank]);
}

WRITE_LINE_MEMBER(firebeat_spu_state::spu_ata_dmarq)
{
	if (m_spuata != nullptr && m_spu_ata_dmarq != state)
	{
		m_spu_ata_dmarq = state;

		if (m_spu_ata_dmarq)
		{
			m_spuata->write_dmack(ASSERT_LINE);
			m_dma_timer->adjust(attotime::zero);
		}
	}
}

TIMER_CALLBACK_MEMBER(firebeat_spu_state::spu_dma_callback)
{
	uint16_t data = m_spuata->read_dma();
	m_waveram[m_wave_bank+m_spu_ata_dma] = data;
	m_spu_ata_dma++;

	if (m_spu_ata_dmarq)
	{
		// This timer adjust value was picked because
		// it reduces stuttering issues/performance issues
		m_dma_timer->adjust(attotime::from_nsec(350));
	}
	else
	{
		m_spuata->write_dmack(CLEAR_LINE);
		m_dma_timer->enable(false);
	}
}

WRITE_LINE_MEMBER(firebeat_spu_state::spu_ata_interrupt)
{
	if (state == 0)
		m_audiocpu->set_input_line(INPUT_LINE_IRQ2, state);

	m_sync_ata_irq = state;
}

TIMER_DEVICE_CALLBACK_MEMBER(firebeat_spu_state::spu_timer_callback)
{
	if (m_sync_ata_irq)
		m_audiocpu->set_input_line(INPUT_LINE_IRQ6, 1);
	else
		m_audiocpu->set_input_line(INPUT_LINE_IRQ2, 1);
}

/*****************************************************************************
* beatmania III
******************************************************************************/
void firebeat_bm3_state::firebeat_bm3(machine_config &config)
{
	firebeat_spu_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &firebeat_bm3_state::firebeat_bm3_map);

	// beatmania III is the only game on the Firebeat platform to use 640x480
	screen_device *screen = subdevice<screen_device>("screen");
	screen->set_size(640, 480);
	screen->set_visarea(0, 639, 0, 479);
	screen->screen_vblank().set(FUNC(firebeat_bm3_state::bm3_vblank));

	ATA_INTERFACE(config, m_spuata).options(firebeat_ata_devices, "hdd", nullptr, true);
	m_spuata->irq_handler().set(FUNC(firebeat_bm3_state::spu_ata_interrupt));
	m_spuata->dmarq_handler().set(FUNC(firebeat_bm3_state::spu_ata_dmarq));
	m_spuata->slot(0).set_fixed(true);

	// 500 hz seems ok for beatmania III.
	// Any higher makes things act weird.
	// Lower doesn't have that huge of an effect compared to pop'n? (limited tested).
	TIMER(config, "spu_timer").configure_periodic(FUNC(firebeat_bm3_state::spu_timer_callback), attotime::from_hz(500));
}

void firebeat_bm3_state::init_bm3()
{
	init_firebeat();
	init_lights(write32s_delegate(*this), write32s_delegate(*this), write32s_delegate(*this));
}

void firebeat_bm3_state::firebeat_bm3_map(address_map &map)
{
	firebeat_spu_map(map);

	map(0x7d000330, 0x7d00033f).nopw(); // ?
	map(0x7d000340, 0x7d00035f).r(FUNC(firebeat_bm3_state::sensor_r));
	map(0x70001fc0, 0x70001fdf).rw(FUNC(firebeat_bm3_state::fdd_unk_r), FUNC(firebeat_bm3_state::fdd_unk_w));
	map(0x70008000, 0x7000807f).r(FUNC(firebeat_bm3_state::spectrum_analyzer_r));
}

uint32_t firebeat_bm3_state::spectrum_analyzer_r(offs_t offset)
{
	// Visible in the sound test menu and most likely the spectral analyzer game skin
	//
	// Notes about where this could be coming from...
	// - It's not the ST-224: Only sends audio in and out, with a MIDI in
	// - It's not the RF5C400: There are no unimplemented registers or anything of that sort that could give this info
	// - The memory address mapping is the same as Keyboardmania's wheel, which plugs into a connector on extend board
	//   but there's nothing actually plugged into that spot on a beatmania III configuration, so it's not external
	// - Any place where the audio is directed somewhere (amps, etc) does not have a way to get back to the PCBs
	//   from what I can tell based on looking at the schematics in the beatmania III manual
	// - I think it's probably calculated somewhere within one of the main boards (main/extend/SPU) but couldn't find any
	//   potentially interesting chips at a glance of PCB pics
	// - The manual does not seem to make mention of this feature *at all* much less troubleshooting it, so no leads there

	// 6 notch spectrum analyzer
	// No idea what frequency range each notch corresponds but it does not affect core gameplay in any way.
	// Notch 1: 0x0c
	// Notch 2: 0x0a
	// Notch 3: 0x08
	// Notch 4: 0x06
	// Notch 5: 0x04
	// Notch 6: 0x02

	// TODO: Fill in logic (reuse vgm_visualizer in some way?)
	// auto ch = offset & 0xf0; // 0 = Left, 1 = Right
	auto data = 0;

	return data;
}

uint16_t firebeat_bm3_state::sensor_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return m_io[0]->read() | 0x0100;
		case 1: return m_io[1]->read() | 0x0100;
		case 2: return m_io[2]->read() | 0x0100;
		case 3: return m_io[3]->read() | 0x0100;
		case 5: return (m_io_turntables[0]->read() >> 8) | 0x0100;
		case 6: return (m_io_turntables[0]->read() & 0xff) | 0x0100;
		case 7: return (m_io_turntables[1]->read() >> 8) | 0x0100;
		case 8: return (m_io_turntables[1]->read() & 0xff) | 0x0100;
		case 9: return m_io_effects[0]->read() | 0x0100;
		case 10: return m_io_effects[1]->read() | 0x0100;
		case 11: return m_io_effects[2]->read() | 0x0100;
		case 12: return m_io_effects[3]->read() | 0x0100;
		case 13: return m_io_effects[4]->read() | 0x0100;
		case 14: return m_io_effects[5]->read() | 0x0100;
		case 15: return m_io_effects[6]->read() | 0x0100;
	}

	return 0;
}

uint32_t firebeat_bm3_state::fdd_unk_r(offs_t offset, uint32_t mem_mask)
{
	// printf("%s: fdd_unk_r: %08X, %08X\n", machine().describe_context().c_str(), offset, mem_mask);

	return 0;
}

void firebeat_bm3_state::fdd_unk_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// printf("%s: fdd_unk_w: %08X, %08X, %08X\n", machine().describe_context().c_str(), data, offset, mem_mask);
}

WRITE_LINE_MEMBER(firebeat_bm3_state::bm3_vblank)
{
	// Patch out FDD errors since the game otherwise runs fine without it
	// and the FDD might not be implemented for a while
	if (strcmp(machine().system().name, "bm3") == 0)
	{
		if (m_work_ram[0x918C / 4] == 0x4809202D) m_work_ram[0x918C / 4] = 0x38600000;
		if (m_work_ram[0x3380C / 4] == 0x4BFFFD99) m_work_ram[0x3380C / 4] = 0x38600000;
		if (m_work_ram[0x33834 / 4] == 0x4BFFFD71) m_work_ram[0x33834 / 4] = 0x38600000;
	}
	else if (strcmp(machine().system().name, "bm3core") == 0)
	{
		if (m_work_ram[0x91E4 / 4] == 0x480A19F5) m_work_ram[0x91E4 / 4] = 0x38600000;
		if (m_work_ram[0x37BB0 / 4] == 0x4BFFFD71) m_work_ram[0x37BB0 / 4] = 0x38600000;
		if (m_work_ram[0x37BD8 / 4] == 0x4BFFFD49) m_work_ram[0x37BD8 / 4] = 0x38600000;
	}
	else if (strcmp(machine().system().name, "bm36th") == 0)
	{
		if (m_work_ram[0x91E4 / 4] == 0x480BC8BD) m_work_ram[0x91E4 / 4] = 0x38600000;
		if (m_work_ram[0x451D8 / 4] == 0x4BFFFD75) m_work_ram[0x451D8 / 4] = 0x38600000;
		if (m_work_ram[0x45200 / 4] == 0x4BFFFD4D) m_work_ram[0x45200 / 4] = 0x38600000;
	}
	else if (strcmp(machine().system().name, "bm37th") == 0)
	{
		if (m_work_ram[0x91E4 / 4] == 0x480CF62D) m_work_ram[0x91E4 / 4] = 0x38600000;
		if (m_work_ram[0x46A58 / 4] == 0x4BFFFD45) m_work_ram[0x46A58 / 4] = 0x38600000;
		if (m_work_ram[0x46AB8 / 4] == 0x4BFFFCE5) m_work_ram[0x46AB8 / 4] = 0x38600000;
	}
	else if (strcmp(machine().system().name, "bm3final") == 0)
	{
		if (m_work_ram[0x47F8 / 4] == 0x480CEF91) m_work_ram[0x47F8 / 4] = 0x38600000;
		if (m_work_ram[0x3FAF4 / 4] == 0x4BFFFD59) m_work_ram[0x3FAF4 / 4] = 0x38600000;
		if (m_work_ram[0x3FB54 / 4] == 0x4BFFFCF9) m_work_ram[0x3FB54 / 4] = 0x38600000;
	}
}

/*****************************************************************************
* pop'n music
******************************************************************************/
void firebeat_popn_state::firebeat_popn(machine_config &config)
{
	firebeat_spu_base(config);

	ATA_INTERFACE(config, m_spuata).options(firebeat_ata_devices, "cdrom", nullptr, true);
	m_spuata->irq_handler().set(FUNC(firebeat_popn_state::spu_ata_interrupt));
	m_spuata->dmarq_handler().set(FUNC(firebeat_popn_state::spu_ata_dmarq));
	m_spuata->slot(0).set_option_machine_config("cdrom", dvdrom_config);
	m_spuata->slot(0).set_fixed(true);

	// 500 hz works best for pop'n music.
	// Any lower and sometimes you'll hear buzzing from certain keysounds, or fades take too long.
	// Any higher and keysounds get cut short.
	TIMER(config, "spu_timer").configure_periodic(FUNC(firebeat_popn_state::spu_timer_callback), attotime::from_hz(500));
}

void firebeat_popn_state::init_popn()
{
	init_firebeat();
	init_lights(write32s_delegate(*this), write32s_delegate(*this), write32s_delegate(*this));
}


/*****************************************************************************
* ParaParaParadise / ParaParaDancing
******************************************************************************/
void firebeat_ppp_state::device_resolve_objects()
{
	firebeat_state::device_resolve_objects();
	m_stage_leds.resolve();
	m_top_leds.resolve();
	m_lamps.resolve();
	m_cab_led_left.resolve();
	m_cab_led_right.resolve();
	m_cab_led_door_lamp.resolve();
	m_cab_led_ok.resolve();
	m_cab_led_slim.resolve();
}

void firebeat_ppp_state::firebeat_ppp(machine_config &config)
{
	firebeat(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &firebeat_ppp_state::firebeat_ppp_map);
}

void firebeat_ppp_state::init_ppp()
{
	init_firebeat();
	init_lights(write32s_delegate(*this, FUNC(firebeat_ppp_state::lamp_output_ppp_w)), write32s_delegate(*this, FUNC(firebeat_ppp_state::lamp_output2_ppp_w)), write32s_delegate(*this, FUNC(firebeat_ppp_state::lamp_output3_ppp_w)));

	m_cabinet_info = 8;
}

void firebeat_ppp_state::init_ppd()
{
	init_firebeat();
	init_lights(write32s_delegate(*this, FUNC(firebeat_ppp_state::lamp_output_ppp_w)), write32s_delegate(*this, FUNC(firebeat_ppp_state::lamp_output2_ppp_w)), write32s_delegate(*this, FUNC(firebeat_ppp_state::lamp_output3_ppp_w)));

	m_cabinet_info = 9;
}

void firebeat_ppp_state::firebeat_ppp_map(address_map &map)
{
	firebeat_map(map);
	map(0x7d000340, 0x7d00035f).r(FUNC(firebeat_ppp_state::sensor_r));
}

uint16_t firebeat_ppp_state::sensor_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return m_io_sensors[0]->read() | 0x0100;
		case 1: return m_io_sensors[1]->read() | 0x0100;
		case 2: return m_io_sensors[2]->read() | 0x0100;
		case 3: return m_io_sensors[3]->read() | 0x0100;
	}

	return 0;
}

void firebeat_ppp_state::lamp_output_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	lamp_output_w(offset, data, mem_mask);

	// ParaParaParadise lamps (active high)
	// 0x00000100 Left
	// 0x00000200 Right
	// 0x00000400 Door Lamp
	// 0x00000800 OK
	// 0x00008000 Slim
	// 0x01000000 Stage LED 0
	// 0x02000000 Stage LED 1
	// 0x04000000 Stage LED 2
	// 0x08000000 Stage LED 3
	// 0x00010000 Stage LED 4
	// 0x00020000 Stage LED 5
	// 0x00040000 Stage LED 6
	// 0x00080000 Stage LED 7
	if (ACCESSING_BITS_8_15)
	{
		m_cab_led_left = BIT(data, 8);
		m_cab_led_right = BIT(data, 9);
		m_cab_led_door_lamp = BIT(data, 10);
		m_cab_led_ok = BIT(data, 11);
		m_cab_led_slim = BIT(data, 15);
	}
	if (ACCESSING_BITS_24_31)
	{
		m_stage_leds[0] = BIT(data, 24);
		m_stage_leds[1] = BIT(data, 25);
		m_stage_leds[2] = BIT(data, 26);
		m_stage_leds[3] = BIT(data, 27);
	}
	if (ACCESSING_BITS_16_23)
	{
		m_stage_leds[4] = BIT(data, 16);
		m_stage_leds[5] = BIT(data, 17);
		m_stage_leds[6] = BIT(data, 18);
		m_stage_leds[7] = BIT(data, 19);
	}
}

void firebeat_ppp_state::lamp_output2_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	lamp_output2_w(offset, data, mem_mask);

	// ParaParaParadise lamps (active high)
	// 0x00010000 Top LED 0
	// 0x00020000 Top LED 1
	// 0x00040000 Top LED 2
	// 0x00080000 Top LED 3
	// 0x00000001 Top LED 4
	// 0x00000002 Top LED 5
	// 0x00000004 Top LED 6
	// 0x00000008 Top LED 7
	if (ACCESSING_BITS_16_23)
	{
		m_top_leds[0] = BIT(data, 16);
		m_top_leds[1] = BIT(data, 17);
		m_top_leds[2] = BIT(data, 18);
		m_top_leds[3] = BIT(data, 19);
	}
	if (ACCESSING_BITS_0_7)
	{
		m_top_leds[4] = BIT(data, 0);
		m_top_leds[5] = BIT(data, 1);
		m_top_leds[6] = BIT(data, 2);
		m_top_leds[7] = BIT(data, 3);
	}
}

void firebeat_ppp_state::lamp_output3_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	lamp_output3_w(offset, data, mem_mask);

	// ParaParaParadise lamps (active high)
	// 0x00010000 Lamp 0
	// 0x00040000 Lamp 1
	// 0x00100000 Lamp 2
	// 0x00400000 Lamp 3
	if (ACCESSING_BITS_16_23)
	{
		m_lamps[0] = BIT(data, 16);
		m_lamps[1] = BIT(data, 18);
		m_lamps[2] = BIT(data, 20);
		m_lamps[3] = BIT(data, 22);
	}
}


/*****************************************************************************
* Keyboardmania
******************************************************************************/
void firebeat_kbm_state::device_resolve_objects()
{
	firebeat_state::device_resolve_objects();
	m_lamps.resolve();
	m_cab_led_door_lamp.resolve();
	m_cab_led_start1p.resolve();
	m_cab_led_start2p.resolve();
	m_lamp_neon.resolve();
}

void firebeat_kbm_state::init_kbm()
{
	init_firebeat();
	init_lights(write32s_delegate(*this, FUNC(firebeat_kbm_state::lamp_output_kbm_w)), write32s_delegate(*this), write32s_delegate(*this));
	init_keyboard();

//  pc16552d_init(machine(), 1, 24000000, midi_uart_irq_callback, 0);     // MIDI UART

	m_cabinet_info = 2;
}

void firebeat_kbm_state::init_keyboard()
{
	// set keyboard timer
//  m_keyboard_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(firebeat_state::keyboard_timer_callback),this));
//  m_keyboard_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

void firebeat_kbm_state::firebeat_kbm(machine_config &config)
{
	/* basic machine hardware */
	PPC403GCX(config, m_maincpu, XTAL(66'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &firebeat_kbm_state::firebeat_kbm_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(firebeat_kbm_state::firebeat_interrupt));

	RTC65271(config, "rtc", 0);

	FUJITSU_29F016A(config, "flash_main");
	FUJITSU_29F016A(config, "flash_snd1");
	FUJITSU_29F016A(config, "flash_snd2");

	ATA_INTERFACE(config, m_ata).options(firebeat_ata_devices, "cdrom", "cdrom", true);
	m_ata->irq_handler().set(FUNC(firebeat_kbm_state::ata_interrupt));
	m_ata->slot(1).set_option_machine_config("cdrom", cdrom_config);
	m_ata->slot(1).set_fixed(true);

	/* video hardware */
	PALETTE(config, "palette", palette_device::RGB_555);

	K057714(config, m_gcu, 0);
	m_gcu->irq_callback().set(FUNC(firebeat_kbm_state::gcu_interrupt));

	K057714(config, m_gcu_sub, 0);
	m_gcu_sub->irq_callback().set(FUNC(firebeat_kbm_state::gcu_interrupt));

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(60);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	lscreen.set_size(512, 384);
	lscreen.set_visarea(0, 511, 0, 383);
	lscreen.set_screen_update(FUNC(firebeat_kbm_state::screen_update_firebeat_0));
	lscreen.set_palette("palette");

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(60);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	rscreen.set_size(512, 384);
	rscreen.set_visarea(0, 511, 0, 383);
	rscreen.set_screen_update(FUNC(firebeat_kbm_state::screen_update_firebeat_1));
	rscreen.set_palette("palette");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16934400));
	ymz.irq_handler().set(FUNC(firebeat_kbm_state::sound_irq_callback));
	ymz.set_addrmap(0, &firebeat_kbm_state::ymz280b_map);
	ymz.add_route(1, "lspeaker", 1.0);
	ymz.add_route(0, "rspeaker", 1.0);

	PC16552D(config, "duart_com", 0);
	NS16550(config, "duart_com:chan0", XTAL(19'660'800));
	NS16550(config, "duart_com:chan1", XTAL(19'660'800));

	PC16552D(config, "duart_midi", 0);
	ns16550_device &midi_chan0(NS16550(config, "duart_midi:chan0", XTAL(24'000'000)));
	midi_chan0.out_int_callback().set(FUNC(firebeat_kbm_state::midi_uart_ch0_irq_callback));
	ns16550_device &midi_chan1(NS16550(config, "duart_midi:chan1", XTAL(24'000'000)));
	midi_chan1.out_int_callback().set(FUNC(firebeat_kbm_state::midi_uart_ch1_irq_callback));

	MIDI_KBD(config, m_kbd[0], 31250).tx_callback().set(midi_chan1, FUNC(ins8250_uart_device::rx_w));
	MIDI_KBD(config, m_kbd[1], 31250).tx_callback().set(midi_chan0, FUNC(ins8250_uart_device::rx_w));
}

void firebeat_kbm_state::firebeat_kbm_map(address_map &map)
{
	firebeat_map(map);
	map(0x70000000, 0x70000fff).rw(FUNC(firebeat_kbm_state::midi_uart_r), FUNC(firebeat_kbm_state::midi_uart_w)).umask32(0xff000000);
	map(0x70008000, 0x7000800f).r(FUNC(firebeat_kbm_state::keyboard_wheel_r));
	map(0x7e800100, 0x7e8001ff).rw(m_gcu_sub, FUNC(k057714_device::read), FUNC(k057714_device::write));
}

uint8_t firebeat_kbm_state::keyboard_wheel_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return m_io_wheels[0]->read(); // Keyboard Wheel (P1)
		case 8: return m_io_wheels[1]->read(); // Keyboard Wheel (P2)
	}

	return 0;
}

uint8_t firebeat_kbm_state::midi_uart_r(offs_t offset)
{
	return m_duart_midi->read(offset >> 6);
}

void firebeat_kbm_state::midi_uart_w(offs_t offset, uint8_t data)
{
	m_duart_midi->write(offset >> 6, data);
}

WRITE_LINE_MEMBER(firebeat_kbm_state::midi_uart_ch0_irq_callback)
{
	if (BIT(m_extend_board_irq_enable, 1) == 0 && state != CLEAR_LINE)
	{
		m_extend_board_irq_active |= 0x02;
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

WRITE_LINE_MEMBER(firebeat_kbm_state::midi_uart_ch1_irq_callback)
{
	if (BIT(m_extend_board_irq_enable, 0) == 0 && state != CLEAR_LINE)
	{
		m_extend_board_irq_active |= 0x01;
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

/*
static const int keyboard_notes[24] =
{
    0x3c,   // C1
    0x3d,   // C1#
    0x3e,   // D1
    0x3f,   // D1#
    0x40,   // E1
    0x41,   // F1
    0x42,   // F1#
    0x43,   // G1
    0x44,   // G1#
    0x45,   // A1
    0x46,   // A1#
    0x47,   // B1
    0x48,   // C2
    0x49,   // C2#
    0x4a,   // D2
    0x4b,   // D2#
    0x4c,   // E2
    0x4d,   // F2
    0x4e,   // F2#
    0x4f,   // G2
    0x50,   // G2#
    0x51,   // A2
    0x52,   // A2#
    0x53,   // B2
};

TIMER_CALLBACK_MEMBER(firebeat_kbm_state::keyboard_timer_callback)
{
    static const int kb_uart_channel[2] = { 1, 0 };
    static const char *const keynames[] = { "KEYBOARD_P1", "KEYBOARD_P2" };
    int keyboard;
    int i;

    for (keyboard=0; keyboard < 2; keyboard++)
    {
        uint32_t kbstate = ioport(keynames[keyboard])->read();
        int uart_channel = kb_uart_channel[keyboard];

        if (kbstate != m_keyboard_state[keyboard])
        {
            for (i=0; i < 24; i++)
            {
                int kbnote = keyboard_notes[i];

                if ((m_keyboard_state[keyboard] & (1 << i)) != 0 && (kbstate & (1 << i)) == 0)
                {
                    // key was on, now off -> send Note Off message
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x80);
                    pc16552d_rx_data(machine(), 1, uart_channel, kbnote);
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x7f);
                }
                else if ((m_keyboard_state[keyboard] & (1 << i)) == 0 && (kbstate & (1 << i)) != 0)
                {
                    // key was off, now on -> send Note On message
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x90);
                    pc16552d_rx_data(machine(), 1, uart_channel, kbnote);
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x7f);
                }
            }
        }
        else
        {
            // no messages, send Active Sense message instead
            pc16552d_rx_data(machine(), 1, uart_channel, 0xfe);
        }

        m_keyboard_state[keyboard] = kbstate;
    }
}
*/

void firebeat_kbm_state::lamp_output_kbm_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	lamp_output_w(offset, data, mem_mask);

	if (ACCESSING_BITS_24_31)
	{
		m_cab_led_door_lamp = BIT(data, 28);
		m_cab_led_start1p = BIT(data, 24);
		m_cab_led_start2p = BIT(data, 25);
	}
	if (ACCESSING_BITS_8_15)
	{
		m_lamps[0] = BIT(data, 8);
		m_lamps[1] = BIT(data, 9);
		m_lamps[2] = BIT(data, 10);
		m_lamp_neon = BIT(data, 11);
	}
}

/*****************************************************************************/

static INPUT_PORTS_START( firebeat )
	PORT_START("IN3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "DIP SW:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "DIP SW:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "DIP SW:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "DIP SW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "DIP SW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "DIP SW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "DIP SW:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "DIP SW:1" )

	PORT_START("IN1")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Fixes "FLASH RAM DATA ERROR" in some games (Mickey Tunes)
INPUT_PORTS_END

static INPUT_PORTS_START( firebeat_spu )
	PORT_START("SPU_DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SPU DSW:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SPU DSW:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SPU DSW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SPU DSW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SPU DSW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SPU DSW:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SPU DSW:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SPU DSW:8" )
INPUT_PORTS_END

static INPUT_PORTS_START(ppp)
	PORT_INCLUDE( firebeat )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )            // Left
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )            // Right
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW)            // Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)      // Service
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )              // Coin
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )             // Start / Ok
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )             // Fixes booting in PPP with certain dongle types
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// ParaParaParadise has 24 sensors, grouped into groups of 3 for each sensor bar
	// Sensors 15...23 are only used by the Korean version of PPP, which has 8 sensor bars

	PORT_START("SENSOR1")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_BUTTON3 )     // Sensor 0, 1, 2  (Sensor bar 1)
	PORT_BIT( 0x0038, IP_ACTIVE_HIGH, IPT_BUTTON4 )     // Sensor 3, 4, 5  (Sensor bar 2)
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_BUTTON5 )     // Sensor 6, 7, 8  (Sensor bar 3)

	PORT_START("SENSOR2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON5 )     // Sensor 6, 7, 8  (Sensor bar 3)
	PORT_BIT( 0x000e, IP_ACTIVE_HIGH, IPT_BUTTON6 )     // Sensor 9, 10,11 (Sensor bar 4)

	PORT_START("SENSOR3")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_BUTTON7 )     // Sensor 12,13,14 (Sensor bar 5)
	PORT_BIT( 0x0038, IP_ACTIVE_HIGH, IPT_BUTTON8 )     // Sensor 15,16,17 (Sensor bar 6)   (unused by PPP)
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_BUTTON9 )     // Sensor 18,19,20 (Sensor bar 7)   (unused by PPP)

	PORT_START("SENSOR4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 )     // Sensor 18,19,20 (Sensor bar 7)   (unused by PPP)
	PORT_BIT( 0x000e, IP_ACTIVE_HIGH, IPT_BUTTON10 )    // Sensor 21,22,23 (Sensor bar 8)   (unused by PPP)

INPUT_PORTS_END

static INPUT_PORTS_START(kbm)
	PORT_INCLUDE( firebeat )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )             // Start P1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )             // Start P2
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW)            // Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)      // Service
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )              // Coin
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )           // e-Amusement
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )            // e-Amusement (Keyboardmania)
	PORT_BIT( 0xde, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("WHEEL_P1")          // Keyboard modulation wheel (P1)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("WHEEL_P2")          // Keyboard modulation wheel (P2)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE_V ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

/*
    PORT_START("KEYBOARD_P1")
    PORT_BIT( 0x000001, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C1") PORT_CODE(KEYCODE_Q)
    PORT_BIT( 0x000002, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C1#") PORT_CODE(KEYCODE_W)
    PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D1") PORT_CODE(KEYCODE_E)
    PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D1#") PORT_CODE(KEYCODE_R)
    PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 E1") PORT_CODE(KEYCODE_T)
    PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F1") PORT_CODE(KEYCODE_Y)
    PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F1#") PORT_CODE(KEYCODE_U)
    PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G1") PORT_CODE(KEYCODE_I)
    PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G1#") PORT_CODE(KEYCODE_O)
    PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A1") PORT_CODE(KEYCODE_A)
    PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A1#") PORT_CODE(KEYCODE_S)
    PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 B1") PORT_CODE(KEYCODE_D)
    PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C2") PORT_CODE(KEYCODE_F)
    PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C2#") PORT_CODE(KEYCODE_G)
    PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D2") PORT_CODE(KEYCODE_H)
    PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D2#") PORT_CODE(KEYCODE_J)
    PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 E2") PORT_CODE(KEYCODE_K)
    PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F2") PORT_CODE(KEYCODE_L)
    PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F2#") PORT_CODE(KEYCODE_Z)
    PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G2") PORT_CODE(KEYCODE_X)
    PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G2#") PORT_CODE(KEYCODE_C)
    PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A2") PORT_CODE(KEYCODE_V)
    PORT_BIT( 0x400000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A2#") PORT_CODE(KEYCODE_B)
    PORT_BIT( 0x800000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 B2") PORT_CODE(KEYCODE_N)

    PORT_START("KEYBOARD_P2")
    PORT_BIT( 0x000001, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C1") PORT_CODE(KEYCODE_Q)
    PORT_BIT( 0x000002, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C1#") PORT_CODE(KEYCODE_W)
    PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D1") PORT_CODE(KEYCODE_E)
    PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D1#") PORT_CODE(KEYCODE_R)
    PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 E1") PORT_CODE(KEYCODE_T)
    PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F1") PORT_CODE(KEYCODE_Y)
    PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F1#") PORT_CODE(KEYCODE_U)
    PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G1") PORT_CODE(KEYCODE_I)
    PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G1#") PORT_CODE(KEYCODE_O)
    PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A1") PORT_CODE(KEYCODE_A)
    PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A1#") PORT_CODE(KEYCODE_S)
    PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 B1") PORT_CODE(KEYCODE_D)
    PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C2") PORT_CODE(KEYCODE_F)
    PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C2#") PORT_CODE(KEYCODE_G)
    PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D2") PORT_CODE(KEYCODE_H)
    PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D2#") PORT_CODE(KEYCODE_J)
    PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 E2") PORT_CODE(KEYCODE_K)
    PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F2") PORT_CODE(KEYCODE_L)
    PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F2#") PORT_CODE(KEYCODE_Z)
    PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G2") PORT_CODE(KEYCODE_X)
    PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G2#") PORT_CODE(KEYCODE_C)
    PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A2") PORT_CODE(KEYCODE_V)
    PORT_BIT( 0x400000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A2#") PORT_CODE(KEYCODE_B)
    PORT_BIT( 0x800000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 B2") PORT_CODE(KEYCODE_N)
*/
INPUT_PORTS_END

static INPUT_PORTS_START(popn)
	PORT_INCLUDE( firebeat )
	PORT_INCLUDE( firebeat_spu )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )            // Switch 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )            // Switch 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )            // Switch 3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )            // Switch 4
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )            // Switch 5
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )            // Switch 6
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 )            // Switch 7
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 )            // Switch 8

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 )            // Switch 9
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )              // Coin
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW)            // Test
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)      // Service
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START(bm3)
	PORT_INCLUDE( firebeat )
	PORT_INCLUDE( firebeat_spu )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE) PORT_NAME(DEF_STR(Test))              // Test
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service")                // Service
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("A")                      // A
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("B")                      // B
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Foot") // P1 Foot Pedal
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Foot") // P2 Foot Pedal

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IO1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // P1 Button 1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // P1 Button 2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // P1 Button 3
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) // P1 Button 4
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) // P1 Button 5
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )                 // P1 Start Button

	PORT_START("IO2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) // Coin sensor

	PORT_START("IO3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // P2 Button 1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // P2 Button 2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) // P2 Button 3
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) // P2 Button 4
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) // P2 Button 5
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )                 // P2 Start Button

	PORT_START("IO4")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TURNTABLE_P1")
	PORT_BIT( 0x03ff, 0x00, IPT_TRACKBALL_X) PORT_PLAYER(1) PORT_NAME("Turntable") PORT_MINMAX(0x00,0x3ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(15)

	PORT_START("TURNTABLE_P2")
	PORT_BIT( 0x03ff, 0x00, IPT_TRACKBALL_X) PORT_PLAYER(2) PORT_NAME("Turntable") PORT_MINMAX(0x00,0x3ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(15)

	PORT_START("EFFECT1")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 1") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT2")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 2") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT3")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 3") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT4")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 4") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT5")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 5") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT6")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 6") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT7")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 7") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

INPUT_PORTS_END

/*****************************************************************************/

ROM_START( ppp )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc0, "user2", 0)    // Security dongle
	ROM_LOAD("gq977-ja", 0x00, 0xc0, BAD_DUMP CRC(5b17d0c7) SHA1(c2de4c0510ad6a48ad5769780a27ce80e44fb380))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "977jaa01", 0, BAD_DUMP SHA1(59c03d8eb366167feef741d42d9d8b54bfeb3c1e) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "977jaa02", 0, SHA1(bd07c25ee3e1edc962997f6a5bb1700897231fb2) )
ROM_END

ROM_START( ppp1mp )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc0, "user2", 0)    // Security dongle
	ROM_LOAD("gqa11-ja", 0x00, 0xc0, BAD_DUMP CRC(207a99b2) SHA1(d19788e1c377771141527660311ff84653039c32))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a11jaa01", 0, SHA1(539ec6f1c1d198b0d6ce5543eadcbb4d9917fa42) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "a11jaa02", 0, SHA1(575069570cb4a2b58b199a1329d45b189a20fcc9) )
ROM_END

ROM_START( ppd )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq977-ko", 0x00, 0xc0, BAD_DUMP CRC(b275de3c) SHA1(d23fcf0e87da2e561bc112851d26b3e78079f40a))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "977kaa01", 0, BAD_DUMP SHA1(7af9f4949ffa10ea5fc18b6c88c2abc710df3cf9) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "977kaa02", 1, SHA1(0feb5ac56269ad4a8401fcfe3bb98b01a0169177) )
ROM_END

ROM_START( ppp11 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq977-ja", 0x00, 0xc0, BAD_DUMP CRC(09b446c4) SHA1(54e43b69a2daeb4f6e69466152a70ce63f6267b5))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gc977jaa01", 0, SHA1(7ed1f4b55105c93fec74468436bfb1d540bce944) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "gc977jaa02", 1, SHA1(74ce8c90575fd562807def7d561392d0f91f2bc6) )
ROM_END

ROM_START( kbm )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq974-ja", 0x00, 0xc0, BAD_DUMP CRC(4bda8987) SHA1(9c5c89808a57aa5aeb7976a3cf3ca96e9797beea))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "974jac01", 0, BAD_DUMP SHA1(c6145d7090e44c87f71ba626620d2ae2596a75ca) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "974jaa02", 1, BAD_DUMP SHA1(3b9946083239eb5687f66a49df24568bffa4fbbd) )
ROM_END

ROM_START( kbm2nd )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gca01-ja", 0x00, 0xc0, BAD_DUMP CRC(25784881) SHA1(99ba9456a91af3043baed2bbf72feed73df564b2))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a01jaa01", 0, BAD_DUMP SHA1(37bc3879719b3d3c6bc8a5691abd7aa4aec87d45) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "a01jaa02", 1, BAD_DUMP SHA1(a3fdeee0f85a7a9718c0fb1cc642ac22d3eff8db) )
ROM_END

ROM_START( kbm3rd )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))

	ROM_REGION(0xc8, "user2", 0)    // Security dongle
	ROM_LOAD("gca12-ja_gca12-aa.bin", 0x00, 0xc8, CRC(96b12482) SHA1(199f20d9fa53108b3fe02d91d6793af1554b0f6f))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a12jaa01", 0, SHA1(ea30bf1273bce772f09063bfc8a74df360c743a7) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "a12jaa02", 0, SHA1(6bf7adbd637a0ce0c19b57187d3e46fabea99363) )
ROM_END

ROM_START( popn4 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq986_gc986.bin", 0x00, 0xc8, CRC(5c11469a) SHA1(efbe2e4a8cbb1285122fad22e3cfe9d47310da02))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gq986jaa01", 0, SHA1(e5368ac029b0bdf29943ae66677b5521ae1176e1) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE( "gq986jaa02", 0, SHA1(53367d3d5f91422fe386c42716492a0ae4332390) )
ROM_END

ROM_START( popn5 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP( "a02jaa03.21e", 0x000000, 0x080000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599) )

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gea04-ja_gca04-ja_gca04-jb.bin", 0x00, 0xc8, CRC(f48f62f7) SHA1(e28b3fd41fa4136cf9c271967d0e68f21a67ba1a))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP( "a02jaa04.3q",  0x000000, 0x080000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683) )

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a04jaa01", 0, SHA1(87136ddad1d786b4d5f04381fcbf679ab666e6c9) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "a04jaa02", 0, SHA1(49a017dde76f84829f6e99a678524c40665c3bfd) )
ROM_END

ROM_START( popn6 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gqa16-ja", 0x00, 0xc0, BAD_DUMP CRC(f2094180) SHA1(36559307f73fe4d9e21533041e8ff8f9297773ed))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gqa16jaa01", 0, SHA1(7a7e475d06c74a273f821fdfde0743b33d566e4c) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE( "gqa16jaa02", 0, SHA1(e39067300e9440ff19cb98c1abc234fa3d5b26d1) )
ROM_END

ROM_START( popn7 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gcb00-ja", 0x00, 0xc0, BAD_DUMP CRC(45223b93) SHA1(b05a260ddc3dbedc3209509b9848d2ccf2319584))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "b00jab01", 0, SHA1(259c733ca4d30281205b46b7bf8d60c9d01aa818) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "b00jaa02", 0, SHA1(c8ce2f8ee6aeeedef9c110a59e68fcec7b669ad6) )
ROM_END

ROM_START( popn8 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gqb30-ja", 0x00, 0xc0, BAD_DUMP CRC(a2276ca5) SHA1(eff519ef21befa5f843f84f3b81d3397a173fe99))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gqb30jaa01", 0, SHA1(0ff3e40e3717ce23337b3a2438bdaca01cba9e30) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gqb30jaa02", 0, SHA1(f067d502c23efe0267aada5706f5bc7a54605942) )
ROM_END

ROM_START( popnanm )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq987-ja", 0x00, 0xc0, BAD_DUMP CRC(201327bd) SHA1(461b422382cc35b0027eb5426bd94d1297b5f98c))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gq987jaa01", 0, SHA1(ee1f9cf480c01ef356451cec30e5303d6c433758) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gq987jaa02", 0, SHA1(d72515bac3fcd9f28c39fa1402292009734df678) )
ROM_END

ROM_START( popnanm2 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gea02-ja", 0x00, 0xc0, BAD_DUMP CRC(ab66b22f) SHA1(7ad73a6ccca513e51b3ab96f4ecd0bdf0a6d1475))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gea02jaa01", 0, SHA1(e81203b6812336c4d00476377193340031ef11b1) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gea02jaa02", 0, SHA1(7212e399779f37a5dcb8317a8f635a3b3f620aa9) )
ROM_END

ROM_START( popnmt )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP( "a02jaa03.21e", 0x000000, 0x080000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599) )

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq976-ja", 0x00, 0xc0, BAD_DUMP CRC(673c2478) SHA1(97e7952a503a93f5334faf1882e8b0394148cf84))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP( "a02jaa04.3q",  0x000000, 0x080000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683) )

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "976jaa01", 0, SHA1(622a9350107e9fb17609ea1a234ca35489915da7) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "976jaa02", 0, SHA1(3881bb1e4deb829ba272c541cb7d203924571f3b) )
ROM_END

ROM_START( popnmt2 )
	// This is an updated version of popnmt released a few months later which has NET@MANIA
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP( "a02jaa03.21e", 0x000000, 0x080000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599) )

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq976-ja", 0x00, 0xc0, BAD_DUMP CRC(673c2478) SHA1(97e7952a503a93f5334faf1882e8b0394148cf84))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP( "a02jaa04.3q",  0x000000, 0x080000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683) )

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "976jba01", 0, SHA1(f8a70ca0718dc222cebbef238b5954494503d315) )

	DISK_REGION( "spu_ata:0:cdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "976jaa02", 0, SHA1(3881bb1e4deb829ba272c541cb7d203924571f3b) )
ROM_END

ROM_START( bm3 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("972maina01.21e", 0x00000, 0x80000, CRC(9de35bfd) SHA1(57290e0015ea24fa46efdfe1e8299003b7754a3b))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gq972-jc", 0x000000, 0x0000c0, BAD_DUMP CRC(25003a96) SHA1(6c9cca4eba6f4334d3fb04744b2929c801b710c0) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("972spua01.3q", 0x00000, 0x80000, CRC(308dbcff) SHA1(87d11eb3e28cb4f3a8f88e3c57a28809dc429ccd))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gc97201", 0, SHA1(216ced68f2082bf891dc3e89fb0663f559cc4915) )

	DISK_REGION( "spu_ata:0:hdd:image" ) // HDD
	DISK_IMAGE_READONLY( "gc97202", 0, SHA1(84049bab473d29eca3c6d536956ef20ae410967d) )
ROM_END

ROM_START( bm3core )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("972maina01.21e", 0x00000, 0x80000, CRC(9de35bfd) SHA1(57290e0015ea24fa46efdfe1e8299003b7754a3b))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gca05-jc.bin", 0x00, 0xc8, CRC(a4c67c80) SHA1(a87609052fa879116350564353df7f5b70ef3ae5) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("972spua01.3q", 0x00000, 0x80000, CRC(308dbcff) SHA1(87d11eb3e28cb4f3a8f88e3c57a28809dc429ccd))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a05jca01", 0, SHA1(b89eced8a1325b087e3f875d1a643bebe9bad5c0) )

	DISK_REGION( "spu_ata:0:hdd:image" ) // HDD
	DISK_IMAGE_READONLY( "a05jca02", 0, NO_DUMP )
ROM_END

ROM_START( bm36th )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("972maina01.21e", 0x00000, 0x80000, CRC(9de35bfd) SHA1(57290e0015ea24fa46efdfe1e8299003b7754a3b))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gca21-jc", 0x000000, 0x0000c0, BAD_DUMP CRC(5cf45b41) SHA1(14f9ff701df79c621d47d20fe3e6b8b579975a1e) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a21jca03.3q", 0x00000, 0x80000, CRC(98ea367a) SHA1(f0b72bdfbba4d265e7b08d606cd82424947d97b9))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a21jca01", 0, SHA1(d1b888379cc0b2c2ab58fa2c5be49258043c3ea1) )

	DISK_REGION( "spu_ata:0:hdd:image" ) // HDD
	DISK_IMAGE_READONLY( "a21jca02", 0, NO_DUMP )
ROM_END

ROM_START( bm37th )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("972maina01.21e", 0x00000, 0x80000, CRC(9de35bfd) SHA1(57290e0015ea24fa46efdfe1e8299003b7754a3b))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gcb07-jc", 0x000000, 0x0000c0, BAD_DUMP CRC(18b32076) SHA1(6f5a44a0c2ed033bc00e73e95f9fbd8301719054) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a21jca03.3q", 0x00000, 0x80000, CRC(98ea367a) SHA1(f0b72bdfbba4d265e7b08d606cd82424947d97b9))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gcb07jca01", 0, SHA1(f906379bdebee314e2ca97c7756259c8c25897fd) )

	DISK_REGION( "spu_ata:0:hdd:image" ) // HDD
	DISK_IMAGE_READONLY( "gcb07jca02", 0, SHA1(6b8e17635825a6a43dc8d2721fe2eb0e0f39e940) )
ROM_END

ROM_START( bm3final )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("972maina01.21e", 0x00000, 0x80000, CRC(9de35bfd) SHA1(57290e0015ea24fa46efdfe1e8299003b7754a3b))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gcc01-jc", 0x000000, 0x0000c0, BAD_DUMP CRC(92eb85c4) SHA1(4cab9c20d072c00a61583731c37aa7fab61857c5) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a21jca03.3q", 0x00000, 0x80000, CRC(98ea367a) SHA1(f0b72bdfbba4d265e7b08d606cd82424947d97b9))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gcc01jca01", 0, SHA1(3e7af83670d791591ad838823422959987f7aab9) )

	DISK_REGION( "spu_ata:0:hdd:image" ) // HDD
	DISK_IMAGE_READONLY( "gcc01jca02", 0, SHA1(823e29bab11cb67069d822f5ffb2b90b9d3368d2) )
ROM_END

} // Anonymous namespace


/*****************************************************************************/

GAME( 2000, ppp,    0,   firebeat_ppp, ppp, firebeat_ppp_state, init_ppp, ROT0, "Konami", "ParaParaParadise", MACHINE_NOT_WORKING )
GAME( 2000, ppd,    0,   firebeat_ppp, ppp, firebeat_ppp_state, init_ppd, ROT0, "Konami", "ParaParaDancing", MACHINE_NOT_WORKING )
GAME( 2000, ppp11,  0,   firebeat_ppp, ppp, firebeat_ppp_state, init_ppp, ROT0, "Konami", "ParaParaParadise v1.1", MACHINE_NOT_WORKING )
GAME( 2000, ppp1mp, ppp, firebeat_ppp, ppp, firebeat_ppp_state, init_ppp, ROT0, "Konami", "ParaParaParadise 1st Mix Plus", MACHINE_NOT_WORKING )

GAMEL( 2000, kbm,    0, firebeat_kbm, kbm, firebeat_kbm_state, init_kbm, ROT270, "Konami", "Keyboardmania", MACHINE_NOT_WORKING, layout_firebeat )
GAMEL( 2000, kbm2nd, 0, firebeat_kbm, kbm, firebeat_kbm_state, init_kbm, ROT270, "Konami", "Keyboardmania 2nd Mix", MACHINE_NOT_WORKING, layout_firebeat )
GAMEL( 2001, kbm3rd, 0, firebeat_kbm, kbm, firebeat_kbm_state, init_kbm, ROT270, "Konami", "Keyboardmania 3rd Mix", MACHINE_NOT_WORKING, layout_firebeat )

GAME( 2000, popn4,    0,      firebeat_popn, popn, firebeat_popn_state, init_popn, ROT0, "Konami", "Pop'n Music 4", MACHINE_NOT_WORKING )
GAME( 2000, popn5,    0,      firebeat_popn, popn, firebeat_popn_state, init_popn, ROT0, "Konami", "Pop'n Music 5", MACHINE_NOT_WORKING )
GAME( 2001, popn6,    0,      firebeat_popn, popn, firebeat_popn_state, init_popn, ROT0, "Konami", "Pop'n Music 6", MACHINE_NOT_WORKING )
GAME( 2001, popn7,    0,      firebeat_popn, popn, firebeat_popn_state, init_popn, ROT0, "Konami", "Pop'n Music 7", MACHINE_NOT_WORKING )
GAME( 2002, popn8,    0,      firebeat_popn, popn, firebeat_popn_state, init_popn, ROT0, "Konami", "Pop'n Music 8", MACHINE_NOT_WORKING )
GAME( 2000, popnmt,   0,      firebeat_popn, popn, firebeat_popn_state, init_popn, ROT0, "Konami", "Pop'n Music Mickey Tunes", MACHINE_NOT_WORKING )
GAME( 2000, popnmt2,  popnmt, firebeat_popn, popn, firebeat_popn_state, init_popn, ROT0, "Konami", "Pop'n Music Mickey Tunes!", MACHINE_NOT_WORKING )
GAME( 2000, popnanm,  0,      firebeat_popn, popn, firebeat_popn_state, init_popn, ROT0, "Konami", "Pop'n Music Animelo", MACHINE_NOT_WORKING )
GAME( 2001, popnanm2, 0,      firebeat_popn, popn, firebeat_popn_state, init_popn, ROT0, "Konami", "Pop'n Music Animelo 2", MACHINE_NOT_WORKING )

GAME( 2000, bm3,      0, firebeat_bm3, bm3, firebeat_bm3_state, init_bm3, ROT0, "Konami", "Beatmania III", MACHINE_NOT_WORKING )
GAME( 2000, bm3core,  0, firebeat_bm3, bm3, firebeat_bm3_state, init_bm3, ROT0, "Konami", "Beatmania III Append Core Remix", MACHINE_NOT_WORKING )
GAME( 2001, bm36th,   0, firebeat_bm3, bm3, firebeat_bm3_state, init_bm3, ROT0, "Konami", "Beatmania III Append 6th Mix", MACHINE_NOT_WORKING )
GAME( 2002, bm37th,   0, firebeat_bm3, bm3, firebeat_bm3_state, init_bm3, ROT0, "Konami", "Beatmania III Append 7th Mix", MACHINE_NOT_WORKING )
GAME( 2003, bm3final, 0, firebeat_bm3, bm3, firebeat_bm3_state, init_bm3, ROT0, "Konami", "Beatmania III The Final", MACHINE_NOT_WORKING )
