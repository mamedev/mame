// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont

#ifndef MAME_MACHINE_STVCD_H
#define MAME_MACHINE_STVCD_H

#pragma once

#include "cdrom.h"
#include "imagedev/chd_cd.h"
#include "machine/timer.h"
#include "sound/cdda.h"

class stvcd_device : public device_t, public device_mixer_interface
{
	static constexpr unsigned MAX_FILTERS = 24;
	static constexpr unsigned MAX_BLOCKS = 200;
	static constexpr uint32_t MAX_DIR_SIZE = 256*1024;

public:
	stvcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ32_MEMBER(stvcd_r);
	DECLARE_WRITE32_MEMBER(stvcd_w);

	void set_tray_open();
	void set_tray_close();

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

private:
	TIMER_DEVICE_CALLBACK_MEMBER( stv_sector_cb );
	TIMER_DEVICE_CALLBACK_MEMBER( stv_sh1_sim );

	struct direntryT
	{
		uint8_t record_size;
		uint8_t xa_record_size;
		uint32_t firstfad;        // first sector of file
		uint32_t length;      // length of file
		uint8_t year;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		uint8_t gmt_offset;
		uint8_t flags;        // iso9660 flags
		uint8_t file_unit_size;
		uint8_t interleave_gap_size;
		uint16_t volume_sequencer_number;
		uint8_t name[128];
	};

	struct filterT
	{
		uint8_t mode;
		uint8_t chan;
		uint8_t smmask;
		uint8_t cimask;
		uint8_t fid;
		uint8_t smval;
		uint8_t cival;
		uint8_t condtrue;
		uint8_t condfalse;
		uint32_t fad;
		uint32_t range;
	};

	struct blockT
	{
		int32_t size; // size of block
		int32_t FAD;  // FAD on disc
		uint8_t data[CD_MAX_SECTOR_DATA];
		uint8_t chan; // channel
		uint8_t fnum; // file number
		uint8_t subm; // subchannel mode
		uint8_t cinf; // coding information
	};

	struct partitionT
	{
		int32_t size;
		blockT *blocks[MAX_BLOCKS];
		uint8_t bnum[MAX_BLOCKS];
		uint8_t numblks;
	};

	// 16-bit transfer types
	enum transT
	{
		XFERTYPE_INVALID,
		XFERTYPE_TOC,
		XFERTYPE_FILEINFO_1,
		XFERTYPE_FILEINFO_254,
		XFERTYPE_SUBQ,
		XFERTYPE_SUBRW
	};

	// 32-bit transfer types
	enum trans32T
	{
		XFERTYPE32_INVALID,
		XFERTYPE32_GETSECTOR,
		XFERTYPE32_GETDELETESECTOR,
		XFERTYPE32_PUTSECTOR,
		XFERTYPE32_MOVESECTOR
	};

	int get_track_index(uint32_t fad);
	int sega_cdrom_get_adr_control(cdrom_file *file, int track);
	void cr_standard_return(uint16_t cur_status);
	void mpeg_standard_return(uint16_t cur_status);
	void cd_free_block(blockT *blktofree);
	void cd_defragblocks(partitionT *part);
	void cd_getsectoroffsetnum(uint32_t bufnum, uint32_t *sectoffs, uint32_t *sectnum);

	uint16_t cd_readWord(uint32_t addr);
	void cd_writeWord(uint32_t addr, uint16_t data);
	uint32_t cd_readLong(uint32_t addr);
	void cd_writeLong(uint32_t addr, uint32_t data);

	void cd_readTOC();
	void cd_readblock(uint32_t fad, uint8_t *dat);
	void cd_playdata();

	void cd_exec_command( void );
	// iso9660 utilities
	void make_dir_current(uint32_t fad);
	void read_new_dir(uint32_t fileno);

	blockT *cd_alloc_block(uint8_t *blknum);
	partitionT *cd_filterdata(filterT *flt, int trktype, uint8_t *p_ok);
	partitionT *cd_read_filtered_sector(int32_t fad, uint8_t *p_ok);

	cdrom_file *cdrom;// = (cdrom_file *)nullptr;

	// local variables
	partitionT partitions[MAX_FILTERS];
	partitionT *transpart;

	blockT blocks[MAX_BLOCKS];
	blockT curblock;

	uint8_t tocbuf[102*4];
	uint8_t subqbuf[5*2];
	uint8_t subrwbuf[12*2];
	uint8_t finfbuf[256];

	int32_t sectlenin, sectlenout;

	uint8_t lastbuf, playtype;

	transT xfertype;
	trans32T xfertype32;
	uint32_t xfercount, calcsize;
	uint32_t xferoffs, xfersect, xfersectpos, xfersectnum, xferdnum;

	filterT filters[MAX_FILTERS];
	filterT *cddevice;
	int cddevicenum;

	uint16_t cr1, cr2, cr3, cr4;
	uint16_t prev_cr1, prev_cr2, prev_cr3, prev_cr4;
	uint8_t status_type;
	uint16_t hirqmask, hirqreg;
	uint16_t cd_stat;
	uint32_t cd_curfad;// = 0;
	uint32_t cd_fad_seek;
	uint32_t fadstoplay;// = 0;
	uint32_t in_buffer;// = 0;    // amount of data in the buffer
	int oddframe;// = 0;
	int buffull, sectorstore, freeblocks;
	int cur_track;
	uint8_t cmd_pending;
	uint8_t cd_speed;
	uint8_t cdda_maxrepeat;
	uint8_t cdda_repeat_count;
	uint8_t tray_is_closed;
	int get_timing_command( void );

	direntryT curroot;       // root entry of current filesystem
	std::vector<direntryT> curdir;       // current directory
	int numfiles;            // # of entries in current directory
	int firstfile;           // first non-directory file

	required_device<cdrom_image_device> m_cdrom_image;
	required_device<timer_device> m_sector_timer;
	required_device<timer_device> m_sh1_timer;
	required_device<cdda_device> m_cdda;
};

// device type definition
DECLARE_DEVICE_TYPE(STVCD, stvcd_device)

#endif // MAME_MACHINE_STVCD_H
