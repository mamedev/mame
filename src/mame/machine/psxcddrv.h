#ifndef _included_psxcddriver_
#define _included_psxcddriver_

const unsigned int raw_sector_size=2352;

class io_status
{
protected:
	virtual ~io_status() {}

public:
	io_status() { }

	virtual void cancel()=0;
	virtual void release()=0;
	virtual bool complete() const=0;
	virtual bool block_until_complete() const=0;
	virtual INT64 bytes_transferred() const=0;
};

enum cdromtype
{
	cdromtype_cd=0,
	cdromtype_dvd
};

enum tracktype
{
	tracktype_unknown=0,
	tracktype_mode1,
	tracktype_mode2,
	tracktype_audio
};

class cdrom_driver
{
	int pf_head,
			pf_tail,
			num_pf,
			pf_head_sector;
	unsigned char *pf_buffer;
	io_status *pf_status,
						*last_pf_status;
	INT64 pf_timeout_begin;

	int native_sector_size;

	virtual io_status *read_sectors(const unsigned int sec, const unsigned int numsec, unsigned char *buf)=0;

	bool is_prefetch_sector_loaded(const unsigned int pfsec);

protected:
	void cancel_io();
	void set_native_sector_size(const unsigned int sz);
	UINT32 timestamp_frequency;
	cdrom_file	*m_cd;
private:
	running_machine *m_machine;

public:
	cdrom_driver();
	virtual ~cdrom_driver();

	void set_machine(const running_machine &machine);
	void set_cdrom_file(cdrom_file *cdfile) { m_cd = cdfile; }

	virtual bool is_usable(char *error_msg=NULL, const int msglen=0) const=0;
	virtual bool read_toc()=0;

	void prefetch_sector(const unsigned int sec);
	bool read_sector(const unsigned int sec,
									 unsigned char *buf,
									 const bool block=true);
	unsigned char *get_prefetch_sector(const unsigned int pfsec,
																		 unsigned int *sz);

	virtual unsigned int get_first_track() const=0;
	virtual unsigned int get_num_tracks() const=0;
	virtual bool get_track_address(const unsigned int track,
																 unsigned char *address) const=0;
	virtual tracktype get_track_type(const unsigned int track) const=0;
	virtual unsigned int find_track(const unsigned int sector,
																	unsigned int *start_sector=NULL,
																	unsigned int *end_sector=NULL) const=0;

	virtual cdromtype get_type() const { return cdromtype_cd; }
};

//
//
//

inline unsigned int msf_to_sector(const unsigned char *msf)
{
	unsigned int sec=msf[2]+(msf[1]*75)+(msf[0]*(60*75));
	if (sec>=150)
	{
		return sec-150;
	}
	else
	{
		return 0;
	}
}

inline void sector_to_msf(const unsigned int sec, unsigned char *msf)
{
	unsigned int s=sec+150;
	msf[0]=s/(60*75);
	s-=msf[0]*(60*75);
	msf[1]=s/75;
	s-=msf[1]*75;
	msf[2]=s;
}

//
//
//

inline unsigned char bcd_to_decimal(const unsigned char bcd)
{
	return ((bcd>>4)*10)+(bcd&0xf);
}

inline unsigned char decimal_to_bcd(const unsigned char dec)
{
	//assert(dec<100);
	return ((dec/10)<<4)|(dec%10);
}

cdrom_driver *open_mess_drv();

#endif
