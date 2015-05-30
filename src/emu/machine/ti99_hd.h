// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Hard disk support
    See ti99_hd.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef __TI99_HD__
#define __TI99_HD__

#include "emu.h"
#include "imagedev/harddriv.h"

class mfm_harddisk_device : public harddisk_image_device,
							public device_slot_card_interface
{
public:
	mfm_harddisk_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	typedef delegate<void (mfm_harddisk_device*, int)> index_pulse_cb;
	typedef delegate<void (mfm_harddisk_device*, int)> seek_complete_cb;

	void setup_index_pulse_cb(index_pulse_cb cb);
	void setup_seek_complete_cb(seek_complete_cb cb);

	// Active low lines. We're using ASSERT=0 / CLEAR=1
	line_state      ready_r() { return m_ready? ASSERT_LINE : CLEAR_LINE; }
	line_state      seek_complete_r() { return m_seek_complete? ASSERT_LINE : CLEAR_LINE; } ;
	line_state      trk00_r() { return m_current_cylinder==0? ASSERT_LINE : CLEAR_LINE; }

	// Step
	void            step_w(line_state line);
	void            direction_in_w(line_state line);

protected:
	void                device_start();
	void                device_reset();
	emu_timer           *m_index_timer, *m_spinup_timer, *m_seek_timer;
	index_pulse_cb      m_index_pulse_cb;
	seek_complete_cb    m_seek_complete_cb;
	void                device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	bool        m_ready;
	int         m_current_cylinder;
	int         m_track_delta;
	int         m_step_phase;
	bool        m_seek_complete;
	bool        m_seek_inward;
	//bool      m_seeking;
	bool        m_autotruncation;
	line_state  m_step_line;    // keep the last state

	void        head_move();
};

class mfm_hd_generic_device : public mfm_harddisk_device
{
public:
	mfm_hd_generic_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type MFM_HD_GENERIC;

// ===========================================================================
// Legacy implementation
// ===========================================================================
#define MFMHD_0 "mfmhd0"
#define MFMHD_1 "mfmhd1"
#define MFMHD_2 "mfmhd2"

extern const device_type TI99_MFMHD_LEG;

/*
    Needed to adapt to higher cylinder numbers. Floppies do not have such
    high numbers.
*/
struct chrn_id_hd
{
	UINT16 C;
	UINT8 H;
	UINT8 R;
	UINT8 N;
	int data_id;            // id for read/write data command
	unsigned long flags;
};

class mfm_harddisk_legacy_device : public device_t
{
public:
	mfm_harddisk_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void    read_sector(int cylinder, int head, int sector, UINT8 *buf);
	void    write_sector(int cylinder, int head, int sector, UINT8 *buf);
	void    read_track(int head, UINT8 *buffer);
	void    write_track(int head, UINT8 *buffer, int data_count);
	UINT8   get_status();
	void    seek(int direction);
	void    get_next_id(int head, chrn_id_hd *id);
	int     get_track_length();

protected:
	void    device_start();
	void    device_reset();
	machine_config_constructor device_mconfig_additions() const;

private:
	int     find_block(const UINT8 *buffer, int start, int stop, UINT8 byte, size_t number);
	UINT8   cylinder_to_ident(int cylinder);
	bool    harddisk_chs_to_lba(hard_disk_file *hdfile, int cylinder, int head, int sector, UINT32 *lba);

	int     m_current_cylinder;
	int     m_current_head;
	bool    m_seeking;
	int     m_status;
	int     m_id_index; /* position in track for seeking the sector; counts the sector number */

	harddisk_image_device *m_drive;
};

class ide_harddisk_legacy_device : public device_t
{
public:
	ide_harddisk_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	virtual void    device_start() { };
	virtual void    device_reset() { };
	virtual machine_config_constructor device_mconfig_additions() const;
};

#define MCFG_MFMHD_3_DRIVES_ADD()           \
	MCFG_DEVICE_ADD(MFMHD_0, TI99_MFMHD_LEG, 0)     \
	MCFG_DEVICE_ADD(MFMHD_1, TI99_MFMHD_LEG, 0)     \
	MCFG_DEVICE_ADD(MFMHD_2, TI99_MFMHD_LEG, 0)

#endif
