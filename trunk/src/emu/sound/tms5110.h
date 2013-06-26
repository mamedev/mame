#pragma once

#ifndef __TMS5110_H__
#define __TMS5110_H__

#include "emu.h"

#define FIFO_SIZE               64 // TODO: technically the tms51xx chips don't have a fifo at all

/* TMS5110 commands */
										/* CTL8  CTL4  CTL2  CTL1  |   PDC's  */
										/* (MSB)             (LSB) | required */
#define TMS5110_CMD_RESET        (0) /*    0     0     0     x  |     1    */
#define TMS5110_CMD_LOAD_ADDRESS (2) /*    0     0     1     x  |     2    */
#define TMS5110_CMD_OUTPUT       (4) /*    0     1     0     x  |     3    */
#define TMS5110_CMD_SPKSLOW      (6) /*    0     1     1     x  |     1    | Note: this command is undocumented on the datasheets, it only appears on the patents. It might not actually work properly on some of the real chips as manufactured. Acts the same as CMD_SPEAK, but makes the interpolator take two A cycles whereever it would normally only take one, effectively making speech of any given word take about twice as long as normal. */
#define TMS5110_CMD_READ_BIT     (8) /*    1     0     0     x  |     1    */
#define TMS5110_CMD_SPEAK       (10) /*    1     0     1     x  |     1    */
#define TMS5110_CMD_READ_BRANCH (12) /*    1     1     0     x  |     1    */
#define TMS5110_CMD_TEST_TALK   (14) /*    1     1     1     x  |     3    */

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

struct tms5110_interface
{
	/* legacy interface */
	int (*M0_callback)(device_t *device);   /* function to be called when chip requests another bit */
	void (*load_address)(device_t *device, int addr);   /* speech ROM load address callback */
	/* new rom controller interface */
	devcb_write_line m0_func;       /* the M0 line */
	devcb_write_line m1_func;       /* the M1 line */
	devcb_write8 addr_func;         /* Write to ADD1,2,4,8 - 4 address bits */
	devcb_read_line data_func;      /* Read one bit from ADD8/Data - voice data */
	/* on a real chip rom_clk is running all the time
	 * Here, we only use it to properly emulate the protocol.
	 * Do not rely on it to be a timed signal.
	 */
	devcb_write_line romclk_func;   /* rom clock - Only used to drive the data lines */
};

class tms5110_device : public device_t,
									public device_sound_interface
{
public:
	tms5110_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms5110_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	DECLARE_WRITE8_MEMBER( ctl_w );
	DECLARE_READ8_MEMBER( ctl_r );
	DECLARE_WRITE_LINE_MEMBER( pdc_w );

	/* this is only used by cvs.c
	 * it is not related at all to the speech generation
	 * and conflicts with the new rom controller interface.
	 */
	DECLARE_READ8_MEMBER( romclk_hack_r );

	int ready_r();
	void set_frequency(int frequency);

	int _speech_rom_read_bit();
	void _speech_rom_set_addr(int addr);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	void set_variant(int variant);

private:
	void new_int_write(UINT8 rc, UINT8 m0, UINT8 m1, UINT8 addr);
	void new_int_write_addr(UINT8 addr);
	UINT8 new_int_read();
	void register_for_save_states();
	void FIFO_data_write(int data);
	int extract_bits(int count);
	void request_bits(int no);
	void perform_dummy_read();
	void process(INT16 *buffer, unsigned int size);
	void PDC_set(int data);
	void parse_frame();

	/* these contain data that describes the 64 bits FIFO */
	UINT8 m_fifo[FIFO_SIZE];
	UINT8 m_fifo_head;
	UINT8 m_fifo_tail;
	UINT8 m_fifo_count;

	/* these contain global status bits */
	UINT8 m_PDC;
	UINT8 m_CTL_pins;
	UINT8 m_speaking_now;
protected:  UINT8 m_talk_status; private:
	UINT8 m_state;

	/* Rom interface */
	UINT32 m_address;
	UINT8  m_next_is_address;
	UINT8  m_schedule_dummy_read;
	UINT8  m_addr_bit;

	/* external callback */
	int (*m_M0_callback)(device_t *);
	void (*m_set_load_address)(device_t *, int);

	/* callbacks */
	devcb_resolved_write_line m_m0_func;      /* the M0 line */
	devcb_resolved_write_line m_m1_func;      /* the M1 line */
	devcb_resolved_write8 m_addr_func;        /* Write to ADD1,2,4,8 - 4 address bits */
	devcb_resolved_read_line m_data_func;     /* Read one bit from ADD8/Data - voice data */
	devcb_resolved_write_line m_romclk_func;  /* rom clock - Only used to drive the data lines */

	/* these contain data describing the current and previous voice frames */
	UINT16 m_old_energy;
	UINT16 m_old_pitch;
	INT32 m_old_k[10];

	UINT16 m_new_energy;
	UINT16 m_new_pitch;
	INT32 m_new_k[10];


	/* these are all used to contain the current state of the sound generation */
	UINT16 m_current_energy;
	UINT16 m_current_pitch;
	INT32 m_current_k[10];

	UINT16 m_target_energy;
	UINT16 m_target_pitch;
	INT32 m_target_k[10];

	UINT8 m_interp_count;       /* number of interp periods (0-7) */
	UINT8 m_sample_count;       /* sample number within interp (0-24) */
	INT32 m_pitch_count;

	INT32 m_x[11];

	INT32 m_RNG;  /* the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13 */

	INT32 m_speech_rom_bitnum;

	UINT8 m_romclk_hack_timer_started;
	UINT8 m_romclk_hack_state;

	/* coefficient tables */
	int m_variant;                /* Variant of the 5110 - see tms5110.h */

	/* coefficient tables */
	const struct tms5100_coeffs *m_coeff;

protected: sound_stream *m_stream; private:
	emu_timer *m_romclk_hack_timer;
	const tms5110_interface *m_intf;
	const UINT8 *m_table;
};

extern const device_type TMS5110;

class tms5100_device : public tms5110_device
{
public:
	tms5100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type TMS5100;

class tms5110a_device : public tms5110_device
{
public:
	tms5110a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type TMS5110A;

class cd2801_device : public tms5110_device
{
public:
	cd2801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type CD2801;

class tmc0281_device : public tms5110_device
{
public:
	tmc0281_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type TMC0281;

class cd2802_device : public tms5110_device
{
public:
	cd2802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type CD2802;

class m58817_device : public tms5110_device
{
public:
	m58817_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( status_r );

protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type M58817;



/* PROM controlled TMS5110 interface */

struct tmsprom_interface
{
	const char *prom_region;        /* prom memory region - sound region is automatically assigned */
	UINT32 rom_size;                /* individual rom_size */
	UINT8 pdc_bit;                  /* bit # of pdc line */
	/* virtual bit 8: constant 0, virtual bit 9:constant 1 */
	UINT8 ctl1_bit;                 /* bit # of ctl1 line */
	UINT8 ctl2_bit;                 /* bit # of ctl2 line */
	UINT8 ctl4_bit;                 /* bit # of ctl4 line */
	UINT8 ctl8_bit;                 /* bit # of ctl8 line */
	UINT8 reset_bit;                /* bit # of rom reset */
	UINT8 stop_bit;                 /* bit # of stop */
	devcb_write_line pdc_func;      /* tms pdc func */
	devcb_write8 ctl_func;          /* tms ctl func */
};

class tmsprom_device : public device_t
{
public:
	tmsprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	WRITE_LINE_MEMBER( m0_w );
	READ_LINE_MEMBER( data_r );

	/* offset is rom # */
	DECLARE_WRITE8_MEMBER( rom_csq_w );
	DECLARE_WRITE8_MEMBER( bit_w );
	WRITE_LINE_MEMBER( enable_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	void register_for_save_states();
	void update_prom_cnt();

	// internal state
	/* Rom interface */
	UINT32 m_address;
	/* ctl lines */
	UINT8  m_m0;
	UINT8  m_enable;
	UINT32  m_base_address;
	UINT8  m_bit;

	int m_prom_cnt;

	devcb_resolved_write_line m_pdc_func;     /* tms pdc func */
	devcb_resolved_write8 m_ctl_func;         /* tms ctl func */

	int    m_clock;
	emu_timer *m_romclk_timer;
	const tmsprom_interface *m_intf;
	const UINT8 *m_rom;
	const UINT8 *m_prom;
};

extern const device_type TMSPROM;


#endif /* __TMS5110_H__ */
