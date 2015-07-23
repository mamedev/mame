// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont
/***************************************************************************

Sega Saturn CD-Block HLE device implementation

***************************************************************************/
#include "cdrom.h"

#pragma once

#ifndef __SEGACDBLOCKDEV_H__
#define __SEGACDBLOCKDEV_H__

#define MAX_BLOCKS  (200)
#define MAX_FILTERS (24)
#define MAX_DIR_SIZE    (256*1024)

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SEGACDBLOCK_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, SEGACDBLOCK, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> segacdblock_device

class segacdblock_device : public device_t,
						   public device_memory_interface
{
public:
	// construction/destruction
	segacdblock_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );

	DECLARE_READ16_MEMBER( cr0_r );
	DECLARE_READ16_MEMBER( cr1_r );
	DECLARE_READ16_MEMBER( cr2_r );
	DECLARE_READ16_MEMBER( cr3_r );

	DECLARE_WRITE16_MEMBER( cr0_w );
	DECLARE_WRITE16_MEMBER( cr1_w );
	DECLARE_WRITE16_MEMBER( cr2_w );
	DECLARE_WRITE16_MEMBER( cr3_w );

	DECLARE_READ16_MEMBER( hirq_r );
	DECLARE_WRITE16_MEMBER( hirq_w );

	DECLARE_READ16_MEMBER( hirq_mask_r );
	DECLARE_WRITE16_MEMBER( hirq_mask_w );

	DECLARE_READ16_MEMBER( datatrns_r );
	DECLARE_READ32_MEMBER( datatrns32_r );
	
protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual machine_config_constructor device_mconfig_additions() const;

	address_space_config        m_space_config;
	address_space *m_space;

private:
	static const device_timer_id PERI_TIMER = 0;
	static const device_timer_id CMD_TIMER = 1;
	static const device_timer_id CD_TIMER = 2;

	UINT16 m_cr[4];
	UINT16 m_dr[4];
	UINT16 m_hirq_mask;
	UINT16 m_hirq;
	UINT32 m_FAD;
	INT32 m_FADEnd;
	UINT16 m_cd_state;
	UINT8 m_cmd_issued;
	UINT8 m_data_read;
	bool m_sh1_inited;
	bool m_isDiscInTray;
	//bool m_TOCPhase;
	bool m_TransferActive;
	UINT32 m_DiscLeadOut;

	cdrom_file *cdrom;

	struct direntryT
	{
		UINT8 record_size;
		UINT8 xa_record_size;
		UINT32 firstfad;        // first sector of file
		UINT32 length;      // length of file
		UINT8 year;
		UINT8 month;
		UINT8 day;
		UINT8 hour;
		UINT8 minute;
		UINT8 second;
		UINT8 gmt_offset;
		UINT8 flags;        // iso9660 flags
		UINT8 file_unit_size;
		UINT8 interleave_gap_size;
		UINT16 volume_sequencer_number;
		UINT8 name[128];
	};

	
	enum transT
	{
		CDDMA_STOPPED,
		CDDMA_INPROGRESS
		/*XFERTYPE_TOC,
		XFERTYPE_FILEINFO_1,
		XFERTYPE_FILEINFO_254,
		XFERTYPE_SUBQ,
		XFERTYPE_SUBRW*/
	};
	
	enum sourceT
	{
		SOURCE_NONE,
		SOURCE_TOC,
		SOURCE_DATA,
		SOURCE_AUDIO
	};

	struct blockT
	{
		INT32 size; // size of block
		INT32 FAD;  // FAD on disc
		UINT8 data[CD_MAX_SECTOR_DATA];
		UINT8 chan; // channel
		UINT8 fnum; // file number
		UINT8 subm; // subchannel mode
		UINT8 cinf; // coding information
	};

	struct partitionT
	{
		INT32 size;
		blockT *blocks[MAX_BLOCKS];
		UINT8 bnum[MAX_BLOCKS];
		UINT8 numblks;
	};
	
	transT xfertype;
	sourceT sourcetype;
	UINT32 m_dma_src,m_dma_size;

	struct filterT
	{
		UINT8 mode;
		UINT8 chan;
		UINT8 smmask;
		UINT8 cimask;
		UINT8 fid;
		UINT8 smval;
		UINT8 cival;
		UINT8 condtrue;
		UINT8 condfalse;
		UINT32 fad;
		UINT32 range;
	};
	


	filterT CDFilters[MAX_FILTERS];
	filterT *CDDeviceConnection;
	partitionT partitions[MAX_FILTERS];
	partitionT *transpart;
	blockT blocks[MAX_BLOCKS];
	blockT curblock;
	partitionT *cd_read_filtered_sector(INT32 fad, UINT8 *p_ok);
	partitionT *cd_filterdata(filterT *flt, int trktype, UINT8 *p_ok);

	void dma_setup();

	UINT8 tocbuf[102*4]; /**< @todo Subchannel Q of lead-in */
	UINT8 finfbuf[256];
	UINT8 *m_DMABuffer;
	UINT8 m_LastBuffer;
	void TOCRetrieve();

	void cd_standard_return(bool isPeri);
	blockT *cd_alloc_block(UINT8 *blknum);
	void cd_free_block(blockT *blktofree);
	int freeblocks;

	// 0x00
	void cd_cmd_status();
	void cd_cmd_get_hw_info();
	void cd_cmd_get_toc();
	void cd_cmd_get_session_info(UINT8 param);
	void cd_cmd_init(UINT8 init_flags);
	void cd_cmd_end_transfer();

	// 0x10
	void cd_cmd_play_disc();
	void cd_cmd_seek();
	
	// 0x30
	void cd_cmd_set_device_connection(UINT8 param);
	// 0x40
	void cd_cmd_set_filter_range(UINT8 filter_number);
	void cd_cmd_set_filter_mode(UINT8 mode, UINT8 filter_number);
	void cd_cmd_get_filter_subheader_conditions(UINT8 filter_number);
	void cd_cmd_get_filter_mode(UINT8 filter_number);
	void cd_cmd_set_filter_subheader_conditions();
	void cd_cmd_set_filter_connection(UINT8 filter_number);
	void cd_cmd_reset_selector(UINT8 reset_flags, UINT8 buffer_number);
	
	// 0x50
	void cd_cmd_get_buffer_size();
	void cd_cmd_get_sector_number(UINT8 buffer_number);
	void cd_cmd_calculate_actual_size();
	void cd_cmd_get_sector_info(UINT8 sector_offset, UINT8 buffer_number);
	void cd_cmd_get_actual_size();
	
	// 0x60
	void cd_cmd_set_sector_length(UINT8 length_in, UINT8 length_out);
	void cd_cmd_get_sector(bool AutoDelete);
	void cd_cmd_delete_sector();
//	void cd_cmd_get_then_delete_sector();
	void cd_cmd_get_copy_error();

	// 0x70
	void cd_cmd_change_dir(UINT32 dir_entry);
	void cd_cmd_read_directory(UINT8 filter_number);
	void cd_cmd_get_file_system_scope();
	void cd_cmd_get_file_info(UINT32 fileid);
	void cd_cmd_read_file();
	void cd_cmd_abort();

	// 0xe0
	void cd_cmd_auth_device(UINT16 AuthType);
	void cd_cmd_device_auth_status(UINT16 AuthType);
	void SH2SendsCommand();
	void SH1CommandExecute();

	void set_flag(UINT16 which);
	void clear_flag(UINT16 which);
	//emu_timer *m_sh1_timer;
	emu_timer *m_peri_timer;
	emu_timer *m_cmd_timer;
	emu_timer *m_cd_timer;
	UINT16 read_cd_state();
	void write_cd_state(UINT16 which);
	void write_fad();
	int m_SectorLengthIn;
	int m_SectorLengthOut;
	void read_new_dir(UINT32 fileno);
	void make_dir_current(UINT32 fad);
	void cd_readblock(UINT32 fad, UINT8 *dat);
	direntryT curroot;       // root entry of current filesystem
	std::vector<direntryT> curdir;       // current directory
	int numfiles;            // # of entries in current directory
	int firstfile;           // first non-directory file
	void cd_getsectoroffsetnum(UINT32 bufnum, UINT32 *sectoffs, UINT32 *sectnum);
	UINT32 xferoffs, xfersect, xfersectpos, xfersectnum, xferdnum;
	bool DeleteSectorMode;
	void cd_defragblocks(partitionT *part);
	bool m_playtype;
	int sega_cdrom_get_adr_control(cdrom_file *file, int track);
	int m_CurrentTrack;
	int m_CalculateActualSize;
	bool m_TempPause;
	bool m_TOCPhase;
	//bool m_BufferFull;
	
	void debug_cd_return_garbage_data();
};


// device type definition
extern const device_type SEGACDBLOCK;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
