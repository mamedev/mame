// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __PCE_CD_H
#define __PCE_CD_H

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

#include "imagedev/chd_cd.h"
#include "machine/nvram.h"
#include "sound/cdda.h"
#include "sound/msm5205.h"

#define PCE_BRAM_SIZE               0x800
#define PCE_ADPCM_RAM_SIZE          0x10000
#define PCE_ACARD_RAM_SIZE          0x200000
#define PCE_CD_COMMAND_BUFFER_SIZE  0x100

#define PCE_CD_IRQ_TRANSFER_READY       0x40
#define PCE_CD_IRQ_TRANSFER_DONE        0x20
#define PCE_CD_IRQ_BRAM                 0x10 /* ??? */
#define PCE_CD_IRQ_SAMPLE_FULL_PLAY     0x08
#define PCE_CD_IRQ_SAMPLE_HALF_PLAY     0x04

#define PCE_CD_ADPCM_PLAY_FLAG      0x08
#define PCE_CD_ADPCM_STOP_FLAG      0x01

#define PCE_CD_DATA_FRAMES_PER_SECOND   75

enum {
	PCE_CD_CDDA_OFF = 0,
	PCE_CD_CDDA_PLAYING,
	PCE_CD_CDDA_PAUSED
};



// ======================> pce_cd_device

class pce_cd_device : public device_t
{
public:
	// construction/destruction
	pce_cd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~pce_cd_device() {}

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_reset();

	void update();

	void late_setup();

	DECLARE_WRITE8_MEMBER(bram_w);
	DECLARE_WRITE8_MEMBER(intf_w);
	DECLARE_WRITE8_MEMBER(acard_w);
	DECLARE_WRITE_LINE_MEMBER(msm5205_int);
	DECLARE_READ8_MEMBER(bram_r);
	DECLARE_READ8_MEMBER(intf_r);
	DECLARE_READ8_MEMBER(acard_r);

	void nvram_init(nvram_device &nvram, void *data, size_t size);

private:
	void adpcm_stop(UINT8 irq_flag);
	void adpcm_play();
	void reply_status_byte(UINT8 status);
	void test_unit_ready();
	void read_6();
	void nec_set_audio_start_position();
	void nec_set_audio_stop_position();
	void nec_pause();
	void nec_get_subq();
	void nec_get_dir_info();
	void end_of_list();
	void handle_data_output();
	void handle_data_input();
	void handle_message_output();
	void handle_message_input();
	void set_irq_line(int num, int state);
	void set_adpcm_ram_byte(UINT8 val);
	UINT8 get_cd_data_byte();
	UINT8 get_adpcm_ram_byte();

	TIMER_CALLBACK_MEMBER(data_timer_callback);
	TIMER_CALLBACK_MEMBER(cdda_fadeout_callback);
	TIMER_CALLBACK_MEMBER(cdda_fadein_callback);
	TIMER_CALLBACK_MEMBER(adpcm_fadeout_callback);
	TIMER_CALLBACK_MEMBER(adpcm_fadein_callback);
	TIMER_CALLBACK_MEMBER(clear_ack);
	TIMER_CALLBACK_MEMBER(adpcm_dma_timer_callback);

	required_device<cpu_device> m_maincpu;

	UINT8   m_regs[16];
	UINT8   *m_bram;
	UINT8   *m_adpcm_ram;
	int     m_bram_locked;
	int     m_adpcm_read_ptr;
	UINT8   m_adpcm_read_buf;
	int     m_adpcm_write_ptr;
	UINT8   m_adpcm_write_buf;
	int     m_adpcm_length;
	int     m_adpcm_clock_divider;
	UINT32  m_msm_start_addr;
	UINT32  m_msm_end_addr;
	UINT32  m_msm_half_addr;
	UINT8   m_msm_nibble;
	UINT8   m_msm_idle;
	UINT8   m_msm_repeat;

	/* SCSI signals */
	int     m_scsi_BSY;   /* Busy. Bus in use */
	int     m_scsi_SEL;   /* Select. Initiator has won arbitration and has selected a target */
	int     m_scsi_CD;    /* Control/Data. Target is sending control (data) information */
	int     m_scsi_IO;    /* Input/Output. Target is sending (receiving) information */
	int     m_scsi_MSG;   /* Message. Target is sending or receiving a message */
	int     m_scsi_REQ;   /* Request. Target is requesting a data transfer */
	int     m_scsi_ACK;   /* Acknowledge. Initiator acknowledges that it is ready for a data transfer */
	int     m_scsi_ATN;   /* Attention. Initiator has a message ready for the target */
	int     m_scsi_RST;   /* Reset. Initiator forces all targets and any other initiators to do a warm reset */
	int     m_scsi_last_RST;  /* To catch setting of RST signal */
	int     m_cd_motor_on;
	int     m_selected;
	UINT8   *m_command_buffer;
	int     m_command_buffer_index;
	int     m_status_sent;
	int     m_message_after_status;
	int     m_message_sent;
	UINT8   *m_data_buffer;
	int     m_data_buffer_size;
	int     m_data_buffer_index;
	int     m_data_transferred;

	/* Arcade Card specific */
	UINT8   *m_acard_ram;
	UINT8   m_acard_latch;
	UINT8   m_acard_ctrl[4];
	UINT32  m_acard_base_addr[4];
	UINT16  m_acard_addr_offset[4];
	UINT16  m_acard_addr_inc[4];
	UINT32  m_acard_shift;
	UINT8   m_acard_shift_reg;

	UINT32  m_current_frame;
	UINT32  m_end_frame;
	UINT32  m_last_frame;
	UINT8   m_cdda_status;
	UINT8   m_cdda_play_mode;
	UINT8   *m_subcode_buffer;
	UINT8   m_end_mark;

	required_device<msm5205_device> m_msm;
	required_device<cdda_device> m_cdda;
	required_device<nvram_device> m_nvram;
	required_device<cdrom_image_device> m_cdrom;

	cdrom_file  *m_cd_file;
	const cdrom_toc*    m_toc;
	emu_timer   *m_data_timer;
	emu_timer   *m_adpcm_dma_timer;

	emu_timer   *m_cdda_fadeout_timer;
	emu_timer   *m_cdda_fadein_timer;
	double  m_cdda_volume;
	emu_timer   *m_adpcm_fadeout_timer;
	emu_timer   *m_adpcm_fadein_timer;
	double  m_adpcm_volume;
};



// device type definition
extern const device_type PCE_CD;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define MCFG_PCE_CD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PCE_CD, 0)


#endif
