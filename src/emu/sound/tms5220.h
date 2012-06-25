#pragma once

#ifndef __TMS5220_H__
#define __TMS5220_H__

#include "devlegcy.h"

#define FIFO_SIZE 16

/* clock rate = 80 * output sample rate,     */
/* usually 640000 for 8000 Hz sample rate or */
/* usually 800000 for 10000 Hz sample rate.  */

typedef struct _tms5220_interface tms5220_interface;
struct _tms5220_interface
{
	devcb_write_line irq_func;		/* IRQ callback function, active low, i.e. state=0 */
	devcb_write_line readyq_func;	/* Ready callback function, active low, i.e. state=0 */

	int (*read)(device_t *device, int count);			/* speech ROM read callback */
	void (*load_address)(device_t *device, int data);	/* speech ROM load address callback */
	void (*read_and_branch)(device_t *device);		/* speech ROM read and branch callback */
};

/* Control lines - once written to will switch interface into
 * "true" timing behaviour.
 */

/* all lines with suffix q are active low! */

WRITE_LINE_DEVICE_HANDLER( tms5220_rsq_w );
WRITE_LINE_DEVICE_HANDLER( tms5220_wsq_w );

WRITE8_DEVICE_HANDLER( tms5220_data_w );
READ8_DEVICE_HANDLER( tms5220_status_r );

READ_LINE_DEVICE_HANDLER( tms5220_readyq_r );
READ_LINE_DEVICE_HANDLER( tms5220_intq_r );


double tms5220_time_to_ready(device_t *device);

void tms5220_set_frequency(device_t *device, int frequency);

DECLARE_LEGACY_SOUND_DEVICE(TMS5220C, tms5220c);
DECLARE_LEGACY_SOUND_DEVICE(TMS5220, tms5220);
DECLARE_LEGACY_SOUND_DEVICE(TMC0285, tmc0285);
DECLARE_LEGACY_SOUND_DEVICE(TMS5200, tms5200);


/***************************************************************************
        New class implementation
        Michael Zapf, June 2012
***************************************************************************/

extern const device_type TMS5220N;
extern const device_type TMS5220CN;
extern const device_type TMC0285N;
extern const device_type TMS5200N;


typedef struct _tms52xx_config
{
	devcb_write_line		irq_func;					// IRQ callback function, active low, i.e. state=0  (TODO: change to ASSERT/CLEAR)
	devcb_write_line		readyq_func;				// Ready callback function, active low, i.e. state=0

	devcb_read8				read_mem;					// speech ROM read callback
	devcb_write8			load_address;				// speech ROM load address callback
	devcb_write8			read_and_branch;			// speech ROM read and branch callback

} tms52xx_config;

// Change back from 5220n to 5220 when all client drivers will be converted
class tms52xx_device : public device_t, public device_sound_interface
{
public:
	tms52xx_device(const machine_config &mconfig, device_type type,  const char *name, const char *tag, const struct tms5100_coeffs* coeffs, const int var, device_t *owner, UINT32 clock);
	void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	DECLARE_WRITE_LINE_MEMBER( rsq_w );
	DECLARE_WRITE_LINE_MEMBER( wsq_w );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	DECLARE_READ_LINE_MEMBER( readyq );
	DECLARE_READ_LINE_MEMBER( intq );

	double time_to_ready();

protected:
	void device_start();
	void device_reset();

private:
	// Methods
	void set_interrupt_state(int state);
	void register_for_save_states();
	void update_ready_state();
	void set_frequency(int frequency);
	void process(INT16 *buffer, unsigned int size);
	void data_write(int data);
	void update_status_and_ints();
	void parse_frame();

	int extract_bits(int count);
	int status_read();
	int cycles_to_ready();
	INT32 lattice_filter();
	void process_command(unsigned char cmd);

	inline bool ready_read();
	inline int int_read();

	// coefficient tables
	const int m_variant;				// Variant of the 5xxx - see tms5110r.h

	// coefficient tables
	const struct tms5100_coeffs *m_coeff;

	// callbacks
	devcb_resolved_write_line	m_irq_func;
	devcb_resolved_write_line	m_readyq_func;
	devcb_resolved_read8		m_read_mem;
	devcb_resolved_write8		m_load_address;
	devcb_resolved_write8		m_read_and_branch;

	// these contain data that describes the 128-bit data FIFO
	UINT8	m_fifo[FIFO_SIZE];
	UINT8	m_fifo_head;
	UINT8	m_fifo_tail;
	int		m_fifo_count;
	int		m_fifo_bits_taken;

	// these contain global status bits
	bool	m_speaking_now;		// True only if actual speech is being generated right now. Is set when a speak vsm command happens OR when speak external happens and buffer low becomes nontrue; Is cleared when speech halts after the last stop frame or the last frame after talk status is otherwise cleared.
	bool	m_speak_external;	// If 1, DDIS is 1, i.e. Speak External command in progress, writes go to FIFO.
	bool	m_talk_status;		// If 1, TS status bit is 1, i.e. speak or speak external is in progress and we have not encountered a stop frame yet; talk_status differs from speaking_now in that speaking_now is set as soon as a speak or speak external command is started; talk_status does NOT go active until after 8 bytes are written to the fifo on a speak external command, otherwise the two are the same. TS is cleared by 3 things: 1. when a STOP command has just been processed as a new frame in the speech stream; 2. if the fifo runs out in speak external mode; 3. on power-up/during a reset command; When it gets cleared, speak_external is also cleared, an interrupt is generated, and speaking_now will be cleared when the next frame starts.
	bool	m_buffer_low;		// If 1, FIFO has less than 8 bytes in it
	bool	m_buffer_empty;		// If 1, FIFO is empty
	int		m_irq_pin;			// state of the IRQ pin (output)
	int		m_ready_pin;		// state of the READY pin (output)

	// these contain data describing the current and previous voice frames
#define M_OLD_FRAME_SILENCE_FLAG m_OLDE // 1 if E=0, 0 otherwise.
#define M_OLD_FRAME_UNVOICED_FLAG m_OLDP // 1 if P=0 (unvoiced), 0 if voiced
	bool	m_OLDE;
	bool	m_OLDP;

#define M_NEW_FRAME_STOP_FLAG (m_new_frame_energy_idx == 0xF)		// 1 if this is a stop (Energy = 0xF) frame
#define M_NEW_FRAME_SILENCE_FLAG (m_new_frame_energy_idx == 0)	// ditto as above
#define M_NEW_FRAME_UNVOICED_FLAG (m_new_frame_pitch_idx == 0)	// ditto as above
	int		m_new_frame_energy_idx;
	int		m_new_frame_pitch_idx;
	int		m_new_frame_k_idx[10];

	// these are all used to contain the current state of the sound generation
#ifndef PERFECT_INTERPOLATION_HACK
	INT16	m_current_energy;
	INT16	m_current_pitch;
	INT16	m_current_k[10];

	INT16	m_target_energy;
	INT16	m_target_pitch;
	INT16	m_target_k[10];
#else
	int 	m_old_frame_energy_idx;
	int		m_old_frame_pitch_idx;
	int		m_old_frame_k_idx[10];

	INT32	m_current_energy;
	INT32	m_current_pitch;
	INT32	m_current_k[10];

	INT32	m_target_energy;
	INT32	m_target_pitch;
	INT32	m_target_k[10];
#endif

	UINT16	m_previous_energy;	// needed for lattice filter to match patent
	int 	m_subcycle;			// contains the current subcycle for a given PC: 0 is A' (only used on SPKSLOW mode on 51xx), 1 is A, 2 is B
	int 	m_subc_reload;		// contains 1 for normal speech, 0 when SPKSLOW is active
	int		m_PC;				// current parameter counter (what param is being interpolated), ranges from 0 to 12

	// TODO/NOTE: the current interpolation period, counts 1,2,3,4,5,6,7,0 for divide by 8,8,8,4,4,4,2,1
	int 	m_interp_period;	// the current interpolation period
	bool	m_inhibit;			// If 1, interpolation is inhibited until the DIV1 period
	UINT8	m_tms5220c_rate;	// only relevant for tms5220C's multi frame rate feature; is the actual 4 bit value written on a 0x2* or 0x0* command
	UINT16	m_pitch_count;		// pitch counter; provides chirp rom address

	INT32	m_u[11];
	INT32	m_x[10];

	UINT16	m_RNG;      // the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13
	INT16	m_excitation_data;

	// R Nabet : These have been added to emulate speech Roms
	bool	m_schedule_dummy_read;	// set after each load address, so that next read operation is preceded by a dummy read
	UINT8	m_data_register;		// data register, used by read command
	bool	m_RDB_flag;				// whether we should read data register or status register

	// io_ready: page 3 of the datasheet specifies that READY will be asserted until
	// data is available or processed by the system.
	int		m_io_ready;

	// flag for "true" timing involving rs/ws
	bool	m_true_timing;

	// rsws - state, rs bit 1, ws bit 0
	int		m_rs_ws;
	UINT8	m_read_latch;
	UINT8	m_write_latch;

	// The TMS52xx has two different ways of providing output data: the
	// analog speaker pin (which was usually used) and the Digital I/O pin.
	// The internal DAC used to feed the analog pin is only 8 bits, and has the
	// funny clipping/clamping logic, while the digital pin gives full 12? bit
	// resolution of the output data.
	// TODO: add a way to set/reset this other than the FORCE_DIGITAL define
	bool			m_digital_select;

	sound_stream	*m_stream;

	emu_timer		*m_ready_timer;
};

class tms5220n_device : public tms52xx_device
{
public:
	tms5220n_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class tms5220cn_device : public tms52xx_device
{
public:
	tms5220cn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class tmc0285n_device : public tms52xx_device
{
public:
	tmc0285n_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class tms5200n_device : public tms52xx_device
{
public:
	tms5200n_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


#endif /* __TMS5220_H__ */
