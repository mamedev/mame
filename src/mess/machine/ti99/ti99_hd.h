// license:MAME|LGPL-2.1+
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

#define MFMHD_0 "mfmhd0"
#define MFMHD_1 "mfmhd1"
#define MFMHD_2 "mfmhd2"

extern const device_type TI99_MFMHD;

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

class mfm_harddisk_device : public device_t
{
public:
	mfm_harddisk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void    read_sector(int cylinder, int head, int sector, UINT8 *buf);
	void    write_sector(int cylinder, int head, int sector, UINT8 *buf);
	void    read_track(int head, UINT8 *buffer);
	void    write_track(int head, UINT8 *buffer, int data_count);
	UINT8   get_status();
	void    seek(int direction);
	void    get_next_id(int head, chrn_id_hd *id);
	int     get_track_length();

protected:
	virtual void    device_start();
	virtual void    device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

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

class ide_harddisk_device : public device_t
{
public:
	ide_harddisk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	virtual void    device_start() { };
	virtual void    device_reset() { };
	virtual machine_config_constructor device_mconfig_additions() const;
};

#define MCFG_MFMHD_3_DRIVES_ADD()           \
	MCFG_DEVICE_ADD(MFMHD_0, TI99_MFMHD, 0)     \
	MCFG_DEVICE_ADD(MFMHD_1, TI99_MFMHD, 0)     \
	MCFG_DEVICE_ADD(MFMHD_2, TI99_MFMHD, 0)

#endif
