// license:BSD-3-Clause
// copyright-holders:Angelo Salese, R. Belmont, Anthony Kruize, Fabio Priuli, Ryan Holtz
#ifndef _SNES_H_
#define _SNES_H_

#include "cpu/spc700/spc700.h"
#include "cpu/g65816/g65816.h"
#include "audio/snes_snd.h"
#include "video/snes_ppu.h"

/*
    SNES timing theory:

    the master clock drives the CPU and PPU
    4  MC ticks = 1 PPU dot
    6  MC ticks = 1 65816 cycle for 3.58 MHz (3.579545)
    8  MC ticks = 1 65816 cycle for 2.68 MHz (2.684659)
    12 MC ticks = 1 65816 cycle for 1.78 MHz (1.789772)

    Each scanline has 341 readable positions and 342 actual dots.
    This is because 2 dots are "long" dots that last 6 MC ticks, resulting in 1 extra dot per line.
*/

// Useful definitions
#define SNES_DMA_BASE         0x4300    /* Base DMA register address */
#define SNES_MODE_20          0x01      /* Lo-ROM cart */
#define SNES_MODE_21          0x02      /* Hi-ROM cart */
#define SNES_MODE_22          0x04      /* Extended Lo-ROM cart - SDD-1 */
#define SNES_MODE_25          0x08      /* Extended Hi-ROM cart */
#define SNES_MODE_BSX         0x10
#define SNES_MODE_BSLO        0x20
#define SNES_MODE_BSHI        0x40
#define SNES_MODE_ST          0x80
#define SNES_EXROM_START      0x1000000

// some PPU registers we still use in machine/snes.c
#define INIDISP        0x2100
#define OAMADDL        0x2102
#define OAMADDH        0x2103
#define SETINI         0x2133
#define MPYL           0x2134
#define MPYM           0x2135
#define MPYH           0x2136

#define APU00          0x2140
#define APU01          0x2141
#define APU02          0x2142
#define APU03          0x2143

#define WMDATA         0x2180
#define WMADDL         0x2181
#define WMADDM         0x2182
#define WMADDH         0x2183

// Definitions for CPU Memory-Mapped registers
#define OLDJOY1        0x4016
#define OLDJOY2        0x4017
#define NMITIMEN       0x4200
#define WRIO           0x4201
//#define WRMPYA         0x4202
//#define WRMPYB         0x4203
//#define WRDIVL         0x4204
//#define WRDIVH         0x4205
//#define WRDVDD         0x4206
#define HTIMEL         0x4207
#define HTIMEH         0x4208
#define VTIMEL         0x4209
#define VTIMEH         0x420A
#define MDMAEN         0x420B
#define HDMAEN         0x420C
//#define MEMSEL         0x420D
#define RDNMI          0x4210
#define TIMEUP         0x4211
#define HVBJOY         0x4212
#define RDIO           0x4213
//#define RDDIVL         0x4214
//#define RDDIVH         0x4215
//#define RDMPYL         0x4216
//#define RDMPYH         0x4217
#define JOY1L          0x4218
#define JOY1H          0x4219
#define JOY2L          0x421A
#define JOY2H          0x421B
#define JOY3L          0x421C
#define JOY3H          0x421D
#define JOY4L          0x421E
#define JOY4H          0x421F
/* DMA */
#define DMAP0          0x4300
#define BBAD0          0x4301
#define A1T0L          0x4302
#define A1T0H          0x4303
#define A1B0           0x4304
#define DAS0L          0x4305
#define DAS0H          0x4306
#define DSAB0          0x4307
#define A2A0L          0x4308
#define A2A0H          0x4309
#define NTRL0          0x430A
#define DMAP1          0x4310
#define BBAD1          0x4311
#define A1T1L          0x4312
#define A1T1H          0x4313
#define A1B1           0x4314
#define DAS1L          0x4315
#define DAS1H          0x4316
#define DSAB1          0x4317
#define A2A1L          0x4318
#define A2A1H          0x4319
#define NTRL1          0x431A
#define DMAP2          0x4320
#define BBAD2          0x4321
#define A1T2L          0x4322
#define A1T2H          0x4323
#define A1B2           0x4324
#define DAS2L          0x4325
#define DAS2H          0x4326
#define DSAB2          0x4327
#define A2A2L          0x4328
#define A2A2H          0x4329
#define NTRL2          0x432A
#define DMAP3          0x4330
#define BBAD3          0x4331
#define A1T3L          0x4332
#define A1T3H          0x4333
#define A1B3           0x4334
#define DAS3L          0x4335
#define DAS3H          0x4336
#define DSAB3          0x4337
#define A2A3L          0x4338
#define A2A3H          0x4339
#define NTRL3          0x433A
#define DMAP4          0x4340
#define BBAD4          0x4341
#define A1T4L          0x4342
#define A1T4H          0x4343
#define A1B4           0x4344
#define DAS4L          0x4345
#define DAS4H          0x4346
#define DSAB4          0x4347
#define A2A4L          0x4348
#define A2A4H          0x4349
#define NTRL4          0x434A
#define DMAP5          0x4350
#define BBAD5          0x4351
#define A1T5L          0x4352
#define A1T5H          0x4353
#define A1B5           0x4354
#define DAS5L          0x4355
#define DAS5H          0x4356
#define DSAB5          0x4357
#define A2A5L          0x4358
#define A2A5H          0x4359
#define NTRL5          0x435A
#define DMAP6          0x4360
#define BBAD6          0x4361
#define A1T6L          0x4362
#define A1T6H          0x4363
#define A1B6           0x4364
#define DAS6L          0x4365
#define DAS6H          0x4366
#define DSAB6          0x4367
#define A2A6L          0x4368
#define A2A6H          0x4369
#define NTRL6          0x436A
#define DMAP7          0x4370
#define BBAD7          0x4371
#define A1T7L          0x4372
#define A1T7H          0x4373
#define A1B7           0x4374
#define DAS7L          0x4375
#define DAS7H          0x4376
#define DSAB7          0x4377
#define A2A7L          0x4378
#define A2A7H          0x4379
#define NTRL7          0x437A
/* Definitions for sound DSP */
#define DSP_V0_VOLL     0x00
#define DSP_V0_VOLR     0x01
#define DSP_V0_PITCHL   0x02
#define DSP_V0_PITCHH   0x03
#define DSP_V0_SRCN     0x04
#define DSP_V0_ADSR1    0x05    /* gdddaaaa = g:gain enable | d:decay | a:attack */
#define DSP_V0_ADSR2    0x06    /* llllrrrr = l:sustain left | r:sustain right */
#define DSP_V0_GAIN     0x07
#define DSP_V0_ENVX     0x08
#define DSP_V0_OUTX     0x09
#define DSP_V1_VOLL     0x10
#define DSP_V1_VOLR     0x11
#define DSP_V1_PITCHL   0x12
#define DSP_V1_PITCHH   0x13
#define DSP_V1_SRCN     0x14
#define DSP_V1_ADSR1    0x15
#define DSP_V1_ADSR2    0x16
#define DSP_V1_GAIN     0x17
#define DSP_V1_ENVX     0x18
#define DSP_V1_OUTX     0x19
#define DSP_V2_VOLL     0x20
#define DSP_V2_VOLR     0x21
#define DSP_V2_PITCHL   0x22
#define DSP_V2_PITCHH   0x23
#define DSP_V2_SRCN     0x24
#define DSP_V2_ADSR1    0x25
#define DSP_V2_ADSR2    0x26
#define DSP_V2_GAIN     0x27
#define DSP_V2_ENVX     0x28
#define DSP_V2_OUTX     0x29
#define DSP_V3_VOLL     0x30
#define DSP_V3_VOLR     0x31
#define DSP_V3_PITCHL   0x32
#define DSP_V3_PITCHH   0x33
#define DSP_V3_SRCN     0x34
#define DSP_V3_ADSR1    0x35
#define DSP_V3_ADSR2    0x36
#define DSP_V3_GAIN     0x37
#define DSP_V3_ENVX     0x38
#define DSP_V3_OUTX     0x39
#define DSP_V4_VOLL     0x40
#define DSP_V4_VOLR     0x41
#define DSP_V4_PITCHL   0x42
#define DSP_V4_PITCHH   0x43
#define DSP_V4_SRCN     0x44
#define DSP_V4_ADSR1    0x45
#define DSP_V4_ADSR2    0x46
#define DSP_V4_GAIN     0x47
#define DSP_V4_ENVX     0x48
#define DSP_V4_OUTX     0x49
#define DSP_V5_VOLL     0x50
#define DSP_V5_VOLR     0x51
#define DSP_V5_PITCHL   0x52
#define DSP_V5_PITCHH   0x53
#define DSP_V5_SRCN     0x54
#define DSP_V5_ADSR1    0x55
#define DSP_V5_ADSR2    0x56
#define DSP_V5_GAIN     0x57
#define DSP_V5_ENVX     0x58
#define DSP_V5_OUTX     0x59
#define DSP_V6_VOLL     0x60
#define DSP_V6_VOLR     0x61
#define DSP_V6_PITCHL   0x62
#define DSP_V6_PITCHH   0x63
#define DSP_V6_SRCN     0x64
#define DSP_V6_ADSR1    0x65
#define DSP_V6_ADSR2    0x66
#define DSP_V6_GAIN     0x67
#define DSP_V6_ENVX     0x68
#define DSP_V6_OUTX     0x69
#define DSP_V7_VOLL     0x70
#define DSP_V7_VOLR     0x71
#define DSP_V7_PITCHL   0x72
#define DSP_V7_PITCHH   0x73
#define DSP_V7_SRCN     0x74
#define DSP_V7_ADSR1    0x75
#define DSP_V7_ADSR2    0x76
#define DSP_V7_GAIN     0x77
#define DSP_V7_ENVX     0x78
#define DSP_V7_OUTX     0x79
#define DSP_MVOLL       0x0C
#define DSP_MVOLR       0x1C
#define DSP_EVOLL       0x2C
#define DSP_EVOLR       0x3C
#define DSP_KON         0x4C    /* 01234567 = Key on for voices 0-7 */
#define DSP_KOF         0x5C    /* 01234567 = Key off for voices 0-7 */
#define DSP_FLG         0x6C    /* rme--n-- = r:Soft reset | m:Mute | e:External memory through echo | n:Clock of noise generator */
#define DSP_ENDX        0x7C
#define DSP_EFB         0x0D    /* sfffffff = s: sign bit | f: feedback */
#define DSP_PMOD        0x2D
#define DSP_NON         0x3D
#define DSP_EON         0x4D
#define DSP_DIR         0x5D
#define DSP_ESA         0x6D
#define DSP_EDL         0x7D    /* ----dddd = d: echo delay */
#define DSP_FIR_C0      0x0F
#define DSP_FIR_C1      0x1F
#define DSP_FIR_C2      0x2F
#define DSP_FIR_C3      0x3F
#define DSP_FIR_C4      0x4F
#define DSP_FIR_C5      0x5F
#define DSP_FIR_C6      0x6F
#define DSP_FIR_C7      0x7F


#define SNES_CPU_REG(a) m_cpu_regs[a - 0x4200]  // regs 0x4200-0x421f

struct snes_cart_info
{
	UINT8 *m_rom;
	UINT32 m_rom_size;
	std::unique_ptr<UINT8[]> m_nvram;
	UINT32 m_nvram_size;
	UINT8  mode;        /* ROM memory mode */
	UINT32 sram_max;    /* Maximum amount sram in cart (based on ROM mode) */
	int    slot_in_use; /* this is needed by Sufami Turbo slots (to check if SRAM has to be saved) */
	UINT8 rom_bank_map[0x100];
};

class snes_state : public driver_device
{
public:
	enum
	{
		TIMER_NMI_TICK,
		TIMER_HIRQ_TICK,
		TIMER_RESET_OAM_ADDRESS,
		TIMER_RESET_HDMA,
		TIMER_UPDATE_IO,
		TIMER_SCANLINE_TICK,
		TIMER_HBLANK_TICK,
		TIMER_SNES_LAST
	};

	snes_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_spc700(*this, "spc700"),
		m_ppu(*this, "ppu"),
		m_screen(*this, "screen") { }

	/* misc */
	UINT16                m_hblank_offset;
	UINT32                m_wram_address;
	UINT16                m_htime;
	UINT16                m_vtime;

	/* non-SNES HW-specific flags / variables */
	UINT8                 m_is_nss;
	UINT8                 m_input_disabled;
	UINT8                 m_game_over_flag;
	UINT8                 m_joy_flag;
	UINT8                 m_is_sfcbox;

	/* timers */
	emu_timer             *m_scanline_timer;
	emu_timer             *m_hblank_timer;
	emu_timer             *m_nmi_timer;
	emu_timer             *m_hirq_timer;
//  emu_timer             *m_div_timer;
//  emu_timer             *m_mult_timer;
	emu_timer             *m_io_timer;

	/* DMA/HDMA-related */
	struct
	{
		UINT8  dmap;
		UINT8  dest_addr;
		UINT16 src_addr;
		UINT16 trans_size;
		UINT8  bank;
		UINT8  ibank;
		UINT16 hdma_addr;
		UINT16 hdma_iaddr;
		UINT8  hdma_line_counter;
		UINT8  unk;

		int    do_transfer;

		int    dma_disabled;    // used to stop DMA if HDMA is enabled (currently not implemented, see machine/snes.c)
	} m_dma_channel[8];
	UINT8                 m_hdmaen; /* channels enabled for HDMA */
	UINT8                 m_dma_regs[0x80];
	UINT8                 m_cpu_regs[0x20];
	UINT8                 m_oldjoy1_latch;

	/* input-related */
	UINT16                m_data1[4];   // JOY1/JOY2 + 3rd & 4th only used by multitap (hacky support)
	UINT16                m_data2[4];   // JOY3/JOY4 + 3rd & 4th only used by multitap (hacky support)
	UINT8                 m_read_idx[4];    // 3rd & 4th only used by multitap (hacky support)

	/* cart related */
	snes_cart_info m_cart;   // used by NSS/SFCBox only! to be moved in a derived class!
	void rom_map_setup(UINT32 size);

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	/* devices */
	required_device<_5a22_device> m_maincpu;
	required_device<spc700_device> m_soundcpu;
	required_device<snes_sound_device> m_spc700;
	required_device<snes_ppu_device> m_ppu;
	required_device<screen_device> m_screen;


	DECLARE_DRIVER_INIT(snes);
	DECLARE_DRIVER_INIT(snes_hirom);
	DECLARE_DRIVER_INIT(snes_mess);
	DECLARE_DRIVER_INIT(snesst);

	inline int dma_abus_valid(UINT32 address);
	inline UINT8 abus_read(address_space &space, UINT32 abus);
	inline void dma_transfer(address_space &space, UINT8 dma, UINT32 abus, UINT16 bbus);
	inline int is_last_active_channel(int dma);
	inline UINT32 get_hdma_addr(int dma);
	inline UINT32 get_hdma_iaddr(int dma);
	void dma(address_space &space, UINT8 channels);
	void hdma(address_space &space);
	void hdma_init(address_space &space);
	void hdma_update(address_space &space, int dma);
	void hirq_tick();
	virtual void write_joy_latch(UINT8 data);
	virtual void wrio_write(UINT8 data);
	inline UINT8 snes_rom_access(UINT32 offset);

	void snes_init_ram();

	// input related
	virtual DECLARE_WRITE8_MEMBER(io_read);
	virtual UINT8 oldjoy1_read(int latched);
	virtual UINT8 oldjoy2_read(int latched);

	DECLARE_READ8_MEMBER(snes_r_io);
	DECLARE_WRITE8_MEMBER(snes_w_io);
	DECLARE_READ8_MEMBER(snes_io_dma_r);
	DECLARE_WRITE8_MEMBER(snes_io_dma_w);
	DECLARE_READ8_MEMBER(snes_r_bank1);
	DECLARE_READ8_MEMBER(snes_r_bank2);
	DECLARE_WRITE8_MEMBER(snes_w_bank1);
	DECLARE_WRITE8_MEMBER(snes_w_bank2);
	DECLARE_READ8_MEMBER(snes_open_bus_r);
	TIMER_CALLBACK_MEMBER(snes_nmi_tick);
	TIMER_CALLBACK_MEMBER(snes_hirq_tick_callback);
	TIMER_CALLBACK_MEMBER(snes_reset_oam_address);
	TIMER_CALLBACK_MEMBER(snes_reset_hdma);
	TIMER_CALLBACK_MEMBER(snes_update_io);
	TIMER_CALLBACK_MEMBER(snes_scanline_tick);
	TIMER_CALLBACK_MEMBER(snes_hblank_tick);
	DECLARE_WRITE_LINE_MEMBER(snes_extern_irq_w);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(snes_cart);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(sufami_cart);
	void snes_init_timers();
	virtual void machine_start() override;
	virtual void machine_reset() override;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/* Special chips, checked at init and used in memory handlers */
enum
{
	HAS_NONE = 0,
	HAS_DSP1,
	HAS_DSP2,
	HAS_DSP3,
	HAS_DSP4,
	HAS_SUPERFX,
	HAS_SA1,
	HAS_SDD1,
	HAS_OBC1,
	HAS_RTC,
	HAS_Z80GB,
	HAS_CX4,
	HAS_ST010,
	HAS_ST011,
	HAS_ST018,
	HAS_SPC7110,
	HAS_SPC7110_RTC,
	HAS_UNK
};

#endif /* _SNES_H_ */
