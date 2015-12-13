// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*************************************************************************

    TX-1/Buggy Boy hardware

*************************************************************************/


#define TX1_PIXEL_CLOCK     (XTAL_18MHz / 3)
#define TX1_HBSTART         256
#define TX1_HBEND           0
#define TX1_HTOTAL          384
#define TX1_VBSTART         240
#define TX1_VBEND           0
#define TX1_VTOTAL          264

/*
 * HACK! Increased VTOTAL to 'fix' a timing issue
 * that prevents one of the start countdown tones
 * from playing.
 */
#define BB_PIXEL_CLOCK      (XTAL_18MHz / 3)
#define BB_HBSTART          256
#define BB_HBEND            0
#define BB_HTOTAL           384
#define BB_VBSTART          240
#define BB_VBEND            0
#define BB_VTOTAL           288 + 1

#define CPU_MASTER_CLOCK    (XTAL_15MHz)
#define BUGGYBOY_ZCLK       (CPU_MASTER_CLOCK / 2)

struct math_t
{
	UINT16  cpulatch;
	UINT16  promaddr;
	UINT16  inslatch;
	UINT32  mux;
	UINT16  ppshift;
	UINT32  i0ff;
	UINT16  retval;
	UINT16  muxlatch;   // TX-1
	int     dbgaddr;
	int     dbgpc;
};

/*
    SN74S516 16x16 Multiplier/Divider
*/
struct sn74s516_t
{
	INT16   X;
	INT16   Y;

	union
	{
	#ifdef LSB_FIRST
		struct { UINT16 W; INT16 Z; } as16bit;
	#else
		struct { INT16 Z; UINT16 W; } as16bit;
	#endif
		INT32 ZW32;
	} ZW;

	int     code;
	int     state;
	int     ZWfl;
};

struct vregs_t
{
	UINT16  scol;       /* Road colours */
	UINT32  slock;      /* Scroll lock */
	UINT8   flags;      /* Road flags */

	UINT32  ba_val;     /* Accumulator */
	UINT32  ba_inc;
	UINT32  bank_mode;

	UINT16  h_val;      /* Accumulator */
	UINT16  h_inc;
	UINT16  h_init;

	UINT8   slin_val;   /* Accumulator */
	UINT8   slin_inc;

	/* Buggyboy only */
	UINT8   wa8;
	UINT8   wa4;

	UINT16  wave_lfsr;
	UINT8   sky;
	UINT16  gas;
	UINT8   shift;
};


class tx1_state : public driver_device
{
public:
	tx1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "main_cpu"),
			m_mathcpu(*this, "math_cpu"),
			m_audiocpu(*this, "audio_cpu"),
			m_math_ram(*this, "math_ram"),
			m_vram(*this, "vram"),
			m_objram(*this, "objram"),
			m_rcram(*this, "rcram"),
			m_z80_ram(*this, "z80_ram"),
			m_char_tiles(*this, "char_tiles"),
			m_obj_tiles(*this, "obj_tiles"),
			m_road_rom(*this, "road"),
			m_obj_map(*this, "obj_map"),
			m_obj_luts(*this, "obj_luts"),
			m_proms(*this, "proms"),
			m_screen(*this, "screen") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mathcpu;
	required_device<cpu_device> m_audiocpu;
	required_shared_ptr<UINT16> m_math_ram;
	required_shared_ptr<UINT16> m_vram;
	required_shared_ptr<UINT16> m_objram;
	required_shared_ptr<UINT16> m_rcram;
	required_shared_ptr<UINT8> m_z80_ram;

	required_region_ptr<UINT8> m_char_tiles;
	required_region_ptr<UINT8> m_obj_tiles;
	required_region_ptr<UINT8> m_road_rom;
	required_region_ptr<UINT8> m_obj_map;
	required_region_ptr<UINT8> m_obj_luts;
	required_region_ptr<UINT8> m_proms;

	required_device<screen_device> m_screen;

	emu_timer *m_interrupt_timer;

	UINT8 m_ppi_latch_a;
	UINT8 m_ppi_latch_b;
	UINT32 m_ts;


	math_t m_math;
	sn74s516_t m_sn74s516;

	vregs_t m_vregs;
	UINT8 *m_chr_bmp;
	UINT8 *m_obj_bmp;
	UINT8 *m_rod_bmp;
	bitmap_ind16 *m_bitmap;

	bool m_needs_update;

	DECLARE_READ16_MEMBER(tx1_math_r);
	DECLARE_WRITE16_MEMBER(tx1_math_w);
	DECLARE_READ16_MEMBER(tx1_spcs_rom_r);
	DECLARE_READ16_MEMBER(tx1_spcs_ram_r);
	DECLARE_WRITE16_MEMBER(tx1_spcs_ram_w);
	DECLARE_READ16_MEMBER(buggyboy_math_r);
	DECLARE_WRITE16_MEMBER(buggyboy_math_w);
	DECLARE_READ16_MEMBER(buggyboy_spcs_rom_r);
	DECLARE_WRITE16_MEMBER(buggyboy_spcs_ram_w);
	DECLARE_READ16_MEMBER(buggyboy_spcs_ram_r);
	DECLARE_READ16_MEMBER(tx1_crtc_r);
	DECLARE_WRITE16_MEMBER(tx1_crtc_w);
	DECLARE_WRITE16_MEMBER(tx1_bankcs_w);
	DECLARE_WRITE16_MEMBER(tx1_slincs_w);
	DECLARE_WRITE16_MEMBER(tx1_slock_w);
	DECLARE_WRITE16_MEMBER(tx1_scolst_w);
	DECLARE_WRITE16_MEMBER(tx1_flgcs_w);
	DECLARE_WRITE16_MEMBER(buggyboy_gas_w);
	DECLARE_WRITE16_MEMBER(buggyboy_sky_w);
	DECLARE_WRITE16_MEMBER(buggyboy_scolst_w);
	DECLARE_WRITE16_MEMBER(z80_busreq_w);
	DECLARE_WRITE16_MEMBER(resume_math_w);
	DECLARE_WRITE16_MEMBER(halt_math_w);
	DECLARE_WRITE8_MEMBER(z80_intreq_w);
	DECLARE_READ16_MEMBER(z80_shared_r);
	DECLARE_WRITE16_MEMBER(z80_shared_w);
	DECLARE_READ16_MEMBER(dipswitches_r);
	DECLARE_WRITE8_MEMBER(ts_w);
	DECLARE_READ8_MEMBER(ts_r);
	DECLARE_WRITE8_MEMBER(tx1_ppi_latch_w);
	DECLARE_READ8_MEMBER(bb_analog_r);
	DECLARE_READ8_MEMBER(bbjr_analog_r);
	DECLARE_WRITE8_MEMBER(tx1_coin_cnt_w);
	DECLARE_WRITE8_MEMBER(bb_coin_cnt_w);
	DECLARE_READ8_MEMBER(tx1_ppi_porta_r);
	DECLARE_READ8_MEMBER(tx1_ppi_portb_r);
	DECLARE_MACHINE_RESET(tx1);
	DECLARE_VIDEO_START(tx1);
	DECLARE_PALETTE_INIT(tx1);
	DECLARE_MACHINE_RESET(buggyboy);
	DECLARE_VIDEO_START(buggyboy);
	DECLARE_PALETTE_INIT(buggyboy);
	DECLARE_VIDEO_START(buggybjr);

	void tx1_draw_char(UINT8 *bitmap);
	void tx1_draw_road_pixel(int screen, UINT8 *bmpaddr,
								UINT8 apix[3], UINT8 bpix[3], UINT32 pixnuma, UINT32 pixnumb,
								UINT8 stl, UINT8 sld, UINT8 selb,
								UINT8 bnk, UINT8 rorev, UINT8 eb, UINT8 r, UINT8 delr);
	void tx1_draw_road(UINT8 *bitmap);
	void tx1_draw_objects(UINT8 *bitmap);
	void tx1_update_layers();
	void tx1_combine_layers(bitmap_ind16 &bitmap, int screen);

	void buggyboy_draw_char(UINT8 *bitmap, bool wide);
	void buggyboy_get_roadpix(int screen, int ls161, UINT8 rva0_6, UINT8 sld, UINT32 *_rorev,
								UINT8 *rc0, UINT8 *rc1, UINT8 *rc2, UINT8 *rc3);
	void buggyboy_draw_road(UINT8 *bitmap);
	void buggybjr_draw_road(UINT8 *bitmap);
	void buggyboy_draw_objs(UINT8 *bitmap, bool wide);
	void bb_combine_layers(bitmap_ind16 &bitmap, int screen);
	void bb_update_layers();

	UINT32 screen_update_tx1_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tx1_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_tx1_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_buggyboy_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_buggyboy_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_buggyboy_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_buggybjr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_tx1(screen_device &screen, bool state);
	void screen_eof_buggyboy(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(z80_irq);
	TIMER_CALLBACK_MEMBER(interrupt_callback);
};

/*----------- defined in audio/tx1.c -----------*/

/*************************************
 *
 *  8253 Programmable Interval Timer
 *
 *************************************/
struct pit8253_state
{
	union
	{
#ifdef LSB_FIRST
		struct { UINT8 LSB; UINT8 MSB; } as8bit;
#else
		struct { UINT8 MSB; UINT8 LSB; } as8bit;
#endif
		UINT16 val;
	} counts[3];

	int idx[3];
};

class tx1_sound_device : public device_t,
							public device_sound_interface
{
public:
	tx1_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tx1_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~tx1_sound_device() {}

	DECLARE_READ8_MEMBER( pit8253_r );
	DECLARE_WRITE8_MEMBER( pit8253_w );
	DECLARE_WRITE8_MEMBER( ay8910_a_w );
	DECLARE_WRITE8_MEMBER( ay8910_b_w );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// internal state
	sound_stream *m_stream;
	UINT32 m_freq_to_step;
	UINT32 m_step0;
	UINT32 m_step1;
	UINT32 m_step2;

	pit8253_state m_pit8253;

	UINT8 m_ay_outputa;
	UINT8 m_ay_outputb;

	stream_sample_t m_pit0;
	stream_sample_t m_pit1;
	stream_sample_t m_pit2;

	double m_weights0[4];
	double m_weights1[3];
	double m_weights2[3];
	int m_eng0[4];
	int m_eng1[4];
	int m_eng2[4];

	int m_noise_lfsra;
	int m_noise_lfsrb;
	int m_noise_lfsrc;
	int m_noise_lfsrd;
	int m_noise_counter;
	UINT8 m_ym1_outputa;
	UINT8 m_ym2_outputa;
	UINT8 m_ym2_outputb;
	UINT16 m_eng_voltages[16];
};

extern const device_type TX1;

class buggyboy_sound_device : public tx1_sound_device
{
public:
	buggyboy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( ym1_a_w );
	DECLARE_WRITE8_MEMBER( ym2_a_w );
	DECLARE_WRITE8_MEMBER( ym2_b_w );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
};

extern const device_type BUGGYBOY;
