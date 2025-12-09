// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont

#ifndef MAME_SEGA_STVCD_H
#define MAME_SEGA_STVCD_H

#pragma once

#include "cdrom.h"
#include "imagedev/cdromimg.h"
#include "machine/timer.h"
#include "sound/cdda.h"

class stvcd_device : public device_t,
					 public device_mixer_interface,
					 public device_memory_interface
{
	static constexpr unsigned MAX_FILTERS = 24;
	static constexpr unsigned MAX_BLOCKS = 200;
	static constexpr uint32_t MAX_DIR_SIZE = 256*1024;

public:
	stvcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t stvcd_r(offs_t offset, uint32_t mem_mask = ~0);
	void stvcd_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void set_tray_open();
	void set_tray_close();

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	const address_space_config      m_space_config;

	void io_regs(address_map &map) ATTR_COLD;

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
		uint8_t data[cdrom_file::MAX_SECTOR_DATA];
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
	int sega_cdrom_get_adr_control(int track);
	void cr_standard_return(uint16_t cur_status);
	void mpeg_standard_return(uint16_t cur_status);
	void cd_free_block(blockT *blktofree);
	void cd_defragblocks(partitionT *part);
	void cd_getsectoroffsetnum(uint32_t bufnum, uint32_t *sectoffs, uint32_t *sectnum);

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

	// CDC commands
	// 0x00
	void cmd_get_status();
	void cmd_get_hw_info();
	void cmd_get_toc();
	void cmd_get_session_info();
	void cmd_init_cdsystem();
	void cmd_end_data_transfer();
	// 0x10
	void cmd_play_disc();
	void cmd_seek_disc();
	void cmd_ffwd_rew_disc();
	// 0x20
	void cmd_get_subcode_q_rw_channel();
	// 0x30
	void cmd_set_cddevice_connection();
	void cmd_get_cddevice_connection();
	void cmd_last_buffer_destination();
	// 0x40
	void cmd_set_filter_range();
	void cmd_get_filter_range();
	void cmd_set_filter_subheader_conditions();
	void cmd_get_filter_subheader_conditions();
	void cmd_set_filter_mode();
	void cmd_get_filter_mode();
	void cmd_set_filter_connection();
	void cmd_reset_selector();
	// 0x50
	void cmd_get_buffer_size();
	void cmd_get_buffer_partition_sector_number();
	void cmd_calculate_actual_data_size();
	void cmd_get_actual_data_size();
	void cmd_get_sector_information();
	// 0x60
	void cmd_set_sector_length();
	void cmd_get_sector_data();
	void cmd_delete_sector_data();
	void cmd_get_and_delete_sector_data();
	void cmd_put_sector_data();
	void cmd_move_sector_data();
	void cmd_copy_sector_data();
	void cmd_get_sector_data_copy_or_move_error();
	// 0x70
	void cmd_change_directory();
	void cmd_read_directory();
	void cmd_get_file_scope();
	void cmd_get_target_file_info();
	void cmd_read_file();
	void cmd_abort_file();
	// 0x90
	void cmd_mpeg_get_status();
	void cmd_mpeg_get_irq();
	void cmd_mpeg_set_irq_mask();
	void cmd_mpeg_init();
	void cmd_mpeg_set_mode();
	// 0xe0
	void cmd_check_copy_protection();
	void cmd_get_disc_region();
	void cmd_get_mpeg_card_boot_rom();

	// comms
	uint32_t datatrns_r(offs_t offset, uint32_t mem_mask = ~0);
	void datatrns_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	inline u32 dataxfer_long_r();
	inline u16 dataxfer_word_r();
	inline void dataxfer_long_w(u32 data);
	uint16_t cr1_r();
	uint16_t cr2_r();
	uint16_t cr3_r();
	uint16_t cr4_r();
	void cr1_w(uint16_t data);
	void cr2_w(uint16_t data);
	void cr3_w(uint16_t data);
	void cr4_w(uint16_t data);

	uint16_t hirq_r();
	void hirq_w(uint16_t data);
	uint16_t hirqmask_r();
	void hirqmask_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
};

// device type definition
DECLARE_DEVICE_TYPE(STVCD, stvcd_device)

#endif // MAME_SEGA_STVCD_H
