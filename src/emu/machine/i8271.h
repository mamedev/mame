// license:BSD-3-Clause
// copyright-holders:Kevin Thacker
/*****************************************************************************
 *
 * machine/i8271.h
 *
 ****************************************************************************/

#ifndef I8271_H_
#define I8271_H_

#include "imagedev/flopdrv.h"

#define MCFG_I8271_IRQ_CALLBACK(_write) \
	devcb = &i8271_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_I8271_DRQ_CALLBACK(_write) \
	devcb = &i8271_device::set_drq_wr_callback(*device, DEVCB_##_write);

#define MCFG_I8271_FLOPPIES(_tag1, _tag2) \
	i8271_device::set_floppy_tags(*device, _tag1, _tag2);

/***************************************************************************
    MACROS
***************************************************************************/

class i8271_device : public device_t
{
public:
	i8271_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~i8271_device() {}

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<i8271_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_drq_wr_callback(device_t &device, _Object object) { return downcast<i8271_device &>(device).m_write_drq.set_callback(object); }

	static void set_floppy_tags(device_t &device, const char *tag1, const char *tag2)
	{
		i8271_device &dev = downcast<i8271_device &>(device);
		dev.m_floppy_tag1 = tag1;
		dev.m_floppy_tag2 = tag2;
	}

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_READ8_MEMBER(dack_r);
	DECLARE_WRITE8_MEMBER(dack_w);

	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE8_MEMBER(data_w);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// internal state
	enum
	{
		TIMER_DATA_CALLBACK,
		TIMER_TIMED_COMMAND_COMPLETE
	};

	devcb_write_line m_write_irq;
	devcb_write_line m_write_drq;

	const char *m_floppy_tag1, *m_floppy_tag2;
	legacy_floppy_image_device *m_floppy[2];

	int m_flags;
	int m_state;
	unsigned char m_Command;
	unsigned char m_StatusRegister;
	unsigned char m_CommandRegister;
	unsigned char m_ResultRegister;
	unsigned char m_ParameterRegister;
	unsigned char m_ResetRegister;
	unsigned char m_data;

	/* number of parameters required after command is specified */
	unsigned long m_ParameterCount;
	/* number of parameters written so far */
	unsigned long m_ParameterCountWritten;

	unsigned char m_CommandParameters[8];

	/* current track for each drive */
	unsigned long   m_CurrentTrack[2];

	/* 2 bad tracks for drive 0, followed by 2 bad tracks for drive 1 */
	unsigned long   m_BadTracks[4];

	/* mode special register */
	unsigned long m_Mode;


	/* drive outputs */
	int m_drive;
	int m_side;

	/* drive control output special register */
	int m_drive_control_output;
	/* drive control input special register */
	int m_drive_control_input;

	unsigned long m_StepRate;
	unsigned long m_HeadSettlingTime;
	unsigned long m_IndexCountBeforeHeadUnload;
	unsigned long m_HeadLoadTime;

	/* id on disc to find */
	//int m_ID_C;
	//int m_ID_H;
	int m_ID_R;
	int m_ID_N;

	/* id of data for read/write */
	int m_data_id;

	int m_ExecutionPhaseTransferCount;
	char *m_pExecutionPhaseData;
	int m_ExecutionPhaseCount;

	/* sector counter and id counter */
	int m_Counter;

	/* ==0, to cpu, !=0 =from cpu */
	//int m_data_direction;

	emu_timer *m_data_timer;
	emu_timer *m_command_complete_timer;

	void seek_to_track(int track);
	void load_bad_tracks(int surface);
	void write_bad_track(int surface, int track, int data);
	void write_current_track(int surface, int track);
	int read_current_track(int surface);
	int read_bad_track(int surface, int track);
	void get_drive();
	void check_all_parameters_written();
	void update_state();
	void initialise_execution_phase_read(int transfer_size);
	void initialise_execution_phase_write(int transfer_size);
	void command_execute();
	void command_continue();
	void command_complete(int result, int int_rq);
	void timed_command_complete();
	void data_request();
	void clear_data_request();
	void timed_data_request();
	/* locate sector for read/write operation */
	int find_sector();
	/* do a read operation */
	void do_read();
	void do_write();
	void do_read_id();
	void set_irq_state(int);
	void set_dma_drq();
};

extern const device_type I8271;

#endif /* I8271_H_ */
