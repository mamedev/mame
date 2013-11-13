/**********************************************************
 *   DIABLO31 and DIABLO44 hard drive support
 *
 *   Copyright: Juergen Buchmueller <pullmoll@t-online.de>
 *
 *   Licenses: MAME, GPLv2
 **********************************************************/

#if	!defined(_DIABLO_HD_DEVICE_)
#define _DIABLO_HD_DEVICE_

#include "emu.h"
#include "imagedev/diablo.h"

#define	DIABLO_DEBUG	1

#if	DIABLO_DEBUG
#include "debug/debugcon.h"
#endif

#define DIABLO_HD_0 "diablo0"
#define DIABLO_HD_1 "diablo1"

extern const device_type DIABLO_HD;

class diablo_hd_device : public device_t
{
public:
	diablo_hd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~diablo_hd_device();

	int bits_per_sector() const;
	const char* description() const;
	int unit() const;
	attotime rotation_time() const;
	attotime sector_time() const;
	attotime bit_time() const;

	int get_seek_read_write_0() const;
	int get_ready_0() const;
	int get_sector_mark_0() const;
	int get_addx_acknowledge_0() const;
	int get_log_addx_interlock_0() const;
	int get_seek_incomplete_0() const;
	int get_cylinder() const;
	int get_head() const;
	int get_sector() const;
	int get_page() const;
	void select(int unit, int head);
	void set_strobe(int cylinder, bool restore, int strobe);
	void set_egate(int gate);
	void set_wrgate(int gate);
	void set_rdgate(int gate);
	void wr_data(int index, int wrdata);
	int rd_data(int index);
	int rd_clock(int index);

protected:
	virtual void    device_start();
	virtual void    device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

private:
#if	DIABLO_DEBUG
	int m_log_level;
	void logprintf(int level, const char* format, ...);
#	define	LOG_DRIVE(x) logprintf x

	void dump_ascii(UINT8 *src, size_t size);
	size_t dump_record(UINT8 *src, size_t addr, size_t size, const char *name, int cr);
#else
#	define	LOG_DRIVE(x)
#endif

	static const int DIABLO_UNIT_MAX = 2;			//!< max number of drive units
	static const int DIABLO_CYLINDERS = 203;		//!< number of cylinders per drive
	static const int DIABLO_CYLINDER_MASK = 0777;	//!< bit maks for cylinder number (9 bits)
	static const int DIABLO_SPT = 12;				//!< number of sectors per track
	static const int DIABLO_SECTOR_MASK = 017;		//!< bit maks for cylinder number (4 bits)
	static const int DIABLO_HEADS = 2;				//!< number of heads per drive
	static const int DIABLO_HEAD_MASK = 1;			//!< bit maks for cylinder number (4 bits)
	static const int DIABLO_PAGES = 203*2*12;		//!< number of pages per drive
	//! convert a cylinder/head/sector to a logical block address (page)
	static inline int DRIVE_PAGE(int c,int h,int s)	{ return (c * DIABLO_HEADS + h) * DIABLO_SPT + s; }

	bool m_diablo31;				//!< true, if this is a DIABLO31 drive
	int m_unit;						//!< drive unit number (0 or 1)
	char m_description[32];			//!< description of the drive(s)
	int m_packs;					//!< number of packs in drive (1 or 2)
	attotime m_rotation_time;		//!< rotation time in atto seconds
	attotime m_sector_time;			//!< sector time in atto seconds
	attotime m_sector_mark_0_time;	//!< sector mark going 0 before sector pulse time
	attotime m_sector_mark_1_time;	//!< sector mark going 1 after sector pulse time
	attotime m_bit_time;			//!< bit time in atto seconds
	int m_s_r_w_0;					//!< drive seek/read/write signal (active 0)
	int m_ready_0;					//!< drive ready signal (active 0)
	int m_sector_mark_0;			//!< sector mark (0 if new sector)
	int m_addx_acknowledge_0;		//!< address acknowledge, i.e. seek successful (active 0)
	int m_log_addx_interlock_0;		//!< log address interlock, i.e. seek in progress (active 0)
	int m_seek_incomplete_0;		//!< seek incomplete, i.e. seek in progress (active 0)
	int m_egate_0;					//!< erase gate
	int m_wrgate_0;					//!< write gate
	int m_rdgate_0;					//!< read gate
	int m_cylinder;					//!< current cylinder number
	int m_head;						//!< current head (track) number on cylinder
	int m_sector;					//!< current sector number in track
	int m_page;						//!< current page
	int m_pages;					//!< total number of pages
	UINT8** m_cache;				//!< pages raw bytes
	UINT32** m_bits;				//!< pages expanded to bits
	int m_rdfirst;					//!< set to first bit of a sector that is read from
	int m_rdlast;					//!< set to last bit of a sector that was read from
	int m_wrfirst;					//!< set to non-zero if a sector is written to
	int m_wrlast;					//!< set to last bit of a sector that was written to
	void (*m_sector_callback)();	//!< callback to call at the start of each sector
	emu_timer* m_sector_timer;		//!< sector timer
	diablo_image_device* m_image;	//!< diablo_image_device interfacing the CHD
	chd_file* m_handle;				//!< underlying CHD handle
	hard_disk_file* m_disk;			//!< underlying hard disk file

	void read_sector();				//!< translate C/H/S to a page and read the sector
	int cksum(UINT8 *src, size_t size, int start);
	size_t expand_zeroes(UINT32 *bits, size_t dst, size_t size);
	size_t expand_sync(UINT32 *bits, size_t dst, size_t size);
	size_t expand_record(UINT32 *bits, size_t dst, UINT8 *field, size_t size);
	size_t expand_cksum(UINT32 *bits, size_t dst, UINT8 *field, size_t size);
	UINT32* expand_sector();

	size_t squeeze_sync(UINT32 *bits, size_t src, size_t size);
	size_t squeeze_unsync(UINT32 *bits, size_t src, size_t size);
	size_t squeeze_record(UINT32 *bits, size_t src, UINT8 *field, size_t size);
	size_t squeeze_cksum(UINT32 *bits, size_t src, int *cksum);
	void squeeze_sector();

	void next_sector(void* ptr, int arg);
	void sector_mark_1();
	void sector_mark_0();
};

#define MCFG_DIABLO_DRIVES_ADD()	\
	MCFG_DEVICE_ADD(DIABLO_HD_0, DIABLO_HD, 0)	\
	MCFG_DEVICE_ADD(DIABLO_HD_1, DIABLO_HD, 0)	\

#endif	// !defined(_DIABLO_HD_DEVICE_)
