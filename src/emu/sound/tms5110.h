#pragma once

#ifndef __TMS5110_H__
#define __TMS5110_H__

#include "devlegcy.h"

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

DECLARE_WRITE8_DEVICE_HANDLER( tms5110_ctl_w );
DECLARE_READ8_DEVICE_HANDLER( tms5110_ctl_r );
WRITE_LINE_DEVICE_HANDLER( tms5110_pdc_w );

/* this is only used by cvs.c
 * it is not related at all to the speech generation
 * and conflicts with the new rom controller interface.
 */
DECLARE_READ8_DEVICE_HANDLER( tms5110_romclk_hack_r );

/* m58817 status line */
DECLARE_READ8_DEVICE_HANDLER( m58817_status_r );

int tms5110_ready_r(device_t *device);

void tms5110_set_frequency(device_t *device, int frequency);

class tms5110_device : public device_t,
									public device_sound_interface
{
public:
	tms5110_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms5110_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	~tms5110_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type TMS5110;

class tms5100_device : public tms5110_device
{
public:
	tms5100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type TMS5100;

class tms5110a_device : public tms5110_device
{
public:
	tms5110a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type TMS5110A;

class cd2801_device : public tms5110_device
{
public:
	cd2801_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type CD2801;

class tmc0281_device : public tms5110_device
{
public:
	tmc0281_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type TMC0281;

class cd2802_device : public tms5110_device
{
public:
	cd2802_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
};

extern const device_type CD2802;

class m58817_device : public tms5110_device
{
public:
	m58817_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
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

WRITE_LINE_DEVICE_HANDLER( tmsprom_m0_w );
READ_LINE_DEVICE_HANDLER( tmsprom_data_r );

/* offset is rom # */
DECLARE_WRITE8_DEVICE_HANDLER( tmsprom_rom_csq_w );
DECLARE_WRITE8_DEVICE_HANDLER( tmsprom_bit_w );
WRITE_LINE_DEVICE_HANDLER( tmsprom_enable_w );

class tmsprom_device : public device_t
{
public:
	tmsprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tmsprom_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
private:
	// internal state
	void *m_token;
};

extern const device_type TMSPROM;


#endif /* __TMS5110_H__ */
