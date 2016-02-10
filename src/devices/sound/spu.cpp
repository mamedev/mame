// license:BSD-3-Clause
// copyright-holders:pSXAuthor, R. Belmont
/*

    Sony PlayStation SPU (CXD2922BQ/CXD2925Q) emulator
    by pSXAuthor
    MAME adaptation by R. Belmont

*/

#include "emu.h"
#include "spu.h"
#include "spureverb.h"

//
//
//

//#define warn_if_sweep_used
#define assert_if_sweep_used
//#define debug_spu_registers
//#define debug_spu
//#define log_loop_cache
//#define show_xa_debug

//#ifndef _FINAL
//  #define show_cache_update
//#endif

#ifdef show_xa_debug
	#define debug_xa printf
#else
	#define debug_xa if (0)
#endif

// device type definition
const device_type SPU = &device_creator<spu_device>;

//
//
//
static inline unsigned int min(unsigned int a, unsigned int b)
{
	return (a > b) ? b : a;
}

static inline unsigned int max(unsigned int a, unsigned int b)
{
	return (a > b) ? a : b;
}

static inline double mindb(double a, double b)
{
	return (a > b) ? b : a;
}

static inline double maxdb(double a, double b)
{
	return (a > b) ? a : b;
}

enum spu_registers
{
	spureg_voice=0,
	spureg_voice_last=0x17f,
	spureg_mvol_l=0x180,
	spureg_mvol_r=0x182,
	spureg_rvol_l=0x184,
	spureg_rvol_r=0x186,
	spureg_keyon=0x188,
	spureg_keyoff=0x18c,
	spureg_fm=0x190,
	spureg_noise=0x194,
	spureg_reverb=0x198,
	spureg_chon=0x19c,
	spureg_reverb_addr=0x1a2,
	spureg_irq_addr=0x1a4,
	spureg_trans_addr=0x1a6,
	spureg_data=0x1a8,
	spureg_ctrl=0x1aa,
	spureg_status=0x1ac,
	spureg_cdvol_l=0x1b0,
	spureg_cdvol_r=0x1b2,
	spureg_exvol_l=0x1b4,
	spureg_exvol_r=0x1b6,
	spureg_reverb_config=0x1c0,
	spureg_last=0x1ff
};

enum spu_ctrl
{
	spuctrl_irq_enable=0x40,
	spuctrl_noise_shift=8,
	spuctrl_noise_mask=0x3f<<spuctrl_noise_shift
};

enum
{
	adpcmflag_end=1,
	adpcmflag_loop=2,
	adpcmflag_loop_start=4
};

struct adpcm_packet
{
	unsigned char info,
								flags,
								data[14];
};

enum adsl_flags
{
	adsl_am=0x8000,
	adsl_ar_shift=8,
	adsl_ar_mask=0x7f<<adsl_ar_shift,
	adsl_dr_shift=4,
	adsl_dr_mask=0xf<<adsl_dr_shift,
	adsl_sl_mask=0xf
};

enum srrr_flags
{
	srrr_sm=0x8000,
	srrr_sd=0x4000,
	srrr_sr_shift=6,
	srrr_sr_mask=0x7f<<srrr_sr_shift,
	srrr_rm=0x20,
	srrr_rr_mask=0x1f
};

static const unsigned int /*sound_buffer_size=65536*4,*/
													xa_sector_size=(18*28*8)<<1,
													xa_buffer_sectors=16,
													cdda_sector_size=2352,
													cdda_buffer_sectors=16,
													num_loop_cache_packets=4,
													num_loop_cache_samples=num_loop_cache_packets*28,
													spu_ram_size=512*1024,
													spu_infinity=0xffffffff,

													output_buffer_size=65536/8/*,

                                                    sample_loop_cache_pool_size=64,
                                                    sample_loop_cache_extend_size=64,
                                                    sample_cache_pool_size=64,
                                                    sample_cache_extend_size=64,

                                                    stream_marker_pool_size=64,
                                                    stream_marker_extend_size=64*/;

//
//
//

static const int filter_coef[5][2]=
{
	{ 0,0 },
	{ 60,0 },
	{ 115,-52 },
	{ 98,-55 },
	{ 122,-60 },
};

//
//
//

#ifdef debug_spu_registers
	#define _voice_registers(_voice)    \
		"voice"#_voice".voll",      \
		"voice"#_voice".volr",      \
		"voice"#_voice".pitch",     \
		"voice"#_voice".addr",      \
		"voice"#_voice".adsl",      \
		"voice"#_voice".srrr",      \
		"voice"#_voice".curvol",    \
		"voice"#_voice".repaddr"

	#define _voice_mask_register(_name) \
		_name##"0-15",                    \
		_name##"16-23"

	static const char *spu_register_names[256]=
	{
		_voice_registers(0),
		_voice_registers(1),
		_voice_registers(2),
		_voice_registers(3),
		_voice_registers(4),
		_voice_registers(5),
		_voice_registers(6),
		_voice_registers(7),
		_voice_registers(8),
		_voice_registers(9),
		_voice_registers(10),
		_voice_registers(11),
		_voice_registers(12),
		_voice_registers(13),
		_voice_registers(14),
		_voice_registers(15),
		_voice_registers(16),
		_voice_registers(17),
		_voice_registers(18),
		_voice_registers(19),
		_voice_registers(20),
		_voice_registers(21),
		_voice_registers(22),
		_voice_registers(23),
		"mvoll",
		"mvolr",
		"rvoll",
		"rvolr",
		"keyon0-15", "keyon16-23",
		"keyoff0-15", "keyoff16-23",
		"fm0-15", "fm16-23",
		"noise0-15", "noise16-23",
		"reverb0-15", "reverb16-23",
		"chon0-15", "chon16-23",
		"unknown",
		"reverbaddr",
		"irqaddr",
		"transaddr",
		"data",
		"ctrl",
		"statusl",
		"statush",
		"cdvoll",
		"cdvolr",
		"exvoll",
		"exvolr"
	};

	const char *get_register_name(const unsigned int addr)
	{
		return spu_register_names[(addr&0x1ff)>>1];
	}
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

reverb_params *spu_reverb_cfg=nullptr;

float spu_device::freq_multiplier=1.0f;

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

class adpcm_decoder
{
	int l0,l1;

public:
	adpcm_decoder()
	{
		reset();
	}

	adpcm_decoder(const adpcm_decoder &other)
	{
		operator =(other);
	}

	adpcm_decoder &operator =(const adpcm_decoder &other)
	{
		l0=other.l0;
		l1=other.l1;
		return *this;
	}

	void reset()
	{
		l0=l1=0;
	}

	signed short *decode_packet(adpcm_packet *ap, signed short *dp);
};

//
//
//

struct spu_device::sample_cache
{
public:
	unsigned int start,
					end,
					invalid_start,
					invalid_end,
					loopaddr,
					last_update_end;
	signed short *data,*loop,*dend;
	adpcm_decoder decoder, update_decoder;
	mutable int ref_count;
	bool valid,
				is_loop;
	sample_loop_cache *loop_cache;

	static unsigned int cache_size;

	sample_cache()
		:   invalid_start(0xffffffff),
			invalid_end(0),
			last_update_end(0xffffffff),
			data(nullptr),
			ref_count(0),
			valid(false),
			is_loop(false),
			loop_cache(nullptr)
	{
	}

	~sample_cache();

	void add_ref() const { ref_count++; }
	void remove_ref() const
	{
		ref_count--;
		if (ref_count==0)
		{
			cache_size-=(dend-data)<<1;
			global_free(this);
		}
	}

	signed short *get_sample_pointer(const unsigned int addr);
	bool get_sample_pointer(const unsigned int addr, cache_pointer *cp);
	bool get_loop_pointer(cache_pointer *cp);
	unsigned int get_sample_address(const signed short *ptr) const;
	sample_loop_cache *find_loop_cache(const unsigned int lpend, const unsigned int lpstart);
	void add_loop_cache(sample_loop_cache *lc);

	bool is_valid_pointer(signed short *ptr) const;

	bool try_update(spu_device *spu);
};

unsigned int spu_device::sample_cache::cache_size;

//
//
//

struct spu_device::sample_loop_cache
{
public:
	unsigned int loopend,
								loopstart,
								len;
	signed short data[num_loop_cache_samples];
	sample_loop_cache *next;

	sample_loop_cache()
		:   next(nullptr)
	{
		sample_cache::cache_size+=num_loop_cache_samples<<1;
	}

	~sample_loop_cache()
	{
		sample_cache::cache_size-=num_loop_cache_samples<<1;

		#ifdef log_loop_cache
			log(log_spu,"spu: destroy loop cache %08x\n",this);
		#endif
	}
};

//
//
//

struct spu_device::cache_pointer
{
	signed short *ptr;
	sample_cache *cache;

	cache_pointer()
		: ptr(nullptr),
			cache(nullptr)
	{
	}

	cache_pointer(const cache_pointer &other)
		: ptr(nullptr),
			cache(nullptr)
	{
		operator =(other);
	}

	cache_pointer(signed short *_ptr, sample_cache *_cache)
		: ptr(_ptr),
			cache(_cache)
	{
		if (cache) cache->add_ref();
	}

	~cache_pointer()
	{
		reset();
	}

	void reset();
	cache_pointer &operator =(const cache_pointer &other);
	bool update(spu_device *spu);

	unsigned int get_address() const
	{
		if (cache)
		{
			return cache->get_sample_address(ptr);
		} else
		{
			return -1;
		}
	}

	operator bool() const { return cache!=nullptr; }

	bool is_valid() const { return ((cache) && (cache->is_valid_pointer(ptr))); }
};

//
//
//

struct spu_device::voiceinfo
{
	cache_pointer play,loop;
	sample_loop_cache *loop_cache;
	unsigned int dptr,
								lcptr;

	int env_state;
	float env_ar,
				env_dr,
				env_sr,
				env_rr,
				env_sl,
				env_level,
				env_delta,

				//>>
				sweep_vol[2],
				sweep_rate[2];
	int vol[2];
				//<<

	unsigned int pitch,
								samplestoend,
								samplestoirq,
								envsamples;
	bool hitirq,
				inloopcache,
				forceloop,
				_pad;
	INT64 keyontime;
};

//
//
//

class stream_buffer
{
public:
	struct stream_marker
	{
	public:
		unsigned int sector,
									offset;
		stream_marker *next,
									*prev;
	};

	dynamic_buffer buffer;
	unsigned int head,
								tail,
								in,
								sector_size,
								num_sectors,
								buffer_size;
	stream_marker *marker_head,
								*marker_tail;

	stream_buffer(const unsigned int _sector_size,
								const unsigned int _num_sectors)
		:   head(0),
			tail(0),
			in(0),
			sector_size(_sector_size),
			num_sectors(_num_sectors),
			marker_head(nullptr),
			marker_tail(nullptr)
	{
		buffer_size=sector_size*num_sectors;
		buffer.resize(buffer_size);
		memset(&buffer[0], 0, buffer_size);
	}

	~stream_buffer()
	{
		flush_all();
	}

	unsigned char *add_sector(const unsigned int sector)
	{
		auto xam=new stream_marker;
		xam->sector=sector;
		xam->offset=head;
		xam->next=nullptr;
		xam->prev=marker_tail;
		if (marker_tail)
		{
			marker_tail->next=xam;
		} else
		{
			marker_head=xam;
		}
		marker_tail=xam;

		unsigned char *ret=&buffer[head];
		head=(head+sector_size)%buffer_size;
		in+=sector_size;
		return ret;
	}

	void flush(const unsigned int sector)
	{
		// Remove markers from the end of the buffer if they are after
		// the specified sector

		while ((marker_tail) && (marker_tail->sector>=sector))
		{
//          debug_xa("flushing: %d\n", marker_tail->sector);

			stream_marker *xam=marker_tail;
			head=xam->offset;
			marker_tail=xam->prev;
			if (marker_tail) marker_tail->next=nullptr;
			global_free(xam);
		}

		// Set marker head to NULL if the list is now empty

		if (! marker_tail) marker_head=nullptr;

		// Adjust buffer size counter

		int sz=(head-tail);
		if (sz<0) sz+=buffer_size;
		assert(sz<=(int)in);
		in=sz;
	}

	void flush_all()
	{
		// NOTE: ??what happens to the markers??

		while (marker_head)
		{
			stream_marker *m=marker_head;
			marker_head=marker_head->next;
			global_free(m);
		}

		marker_head=marker_tail=nullptr;
		head=tail=in=0;
	}

	void delete_markers(const unsigned int oldtail)
	{
		while (marker_head)
		{
			int olddist=marker_head->offset-oldtail,
					dist=marker_head->offset-tail;
			if (olddist<0) olddist+=buffer_size;
			if (dist<0) dist+=buffer_size;
			bool passed=(((olddist==0) && (dist!=0)) || (dist>olddist));
			if (! passed) break;

//          debug_xa("passed: %d\n",marker_head->sector);

			stream_marker *xam=marker_head;
			marker_head=xam->next;
			global_free(xam);
			if (marker_head) marker_head->prev=nullptr;
		}

		if (! marker_head) marker_head=marker_tail=nullptr;
	}

	unsigned int get_bytes_in() const { return in; }
	unsigned int get_bytes_free() const { return buffer_size-in; }

	unsigned char *get_tail_ptr() { return &buffer[tail]; }
	unsigned char *get_tail_ptr(const unsigned int offset)
	{
		return &buffer[((tail+offset)%buffer_size)];
	}
	unsigned int get_tail_offset() const { return tail; }
	void increment_tail(const unsigned int offset)
	{
		tail=(tail+offset)%buffer_size;
		in-=offset;
	}
};

//
//
//

static inline int clamp(const int v)
{
	if (v<-32768) return -32768;
	if (v>32767) return 32767;
	return v;
}

//
//
//

spu_device::sample_cache::~sample_cache()
{
	global_free_array(data);
	while (loop_cache)
	{
		sample_loop_cache *lc=loop_cache;
		loop_cache=lc->next;
		global_free(lc);
	}
}

//
//
//

signed short *spu_device::sample_cache::get_sample_pointer(const unsigned int addr)
{
	if ((addr>=start) && (addr<end))
	{
		return data+(((addr-start)>>4)*28);
	} else
	{
		return nullptr;
	}
}

//
//
//

bool spu_device::sample_cache::get_sample_pointer(const unsigned int addr, cache_pointer *cp)
{
	cp->reset();
	if ((cp->ptr=get_sample_pointer(addr)))
	{
		cp->cache=this;
		add_ref();
		return true;
	}
	return false;
}

//
//
//

bool spu_device::sample_cache::get_loop_pointer(cache_pointer *cp)
{
	cp->reset();
	if ((cp->ptr=loop))
	{
		cp->cache=this;
		add_ref();
		return true;
	}
	return false;
}

//
//
//

unsigned int spu_device::sample_cache::get_sample_address(const signed short *ptr) const
{
	if ((ptr>=data) && (ptr<=dend))
	{
		return start+(((ptr-data)/28)<<4);
	} else
	{
		return -1;
	}
}

//
//
//

spu_device::sample_loop_cache *spu_device::sample_cache::find_loop_cache(const unsigned int lpend, const unsigned int lpstart)
{
	sample_loop_cache *lc;
	for (lc=loop_cache; lc; lc=lc->next)
		if ((lc->loopend==lpend) && (lc->loopstart==lpstart)) break;
	return lc;
}

//
//
//

void spu_device::sample_cache::add_loop_cache(sample_loop_cache *lc)
{
	lc->next=loop_cache;
	loop_cache=lc;
}

//
//
//

bool spu_device::sample_cache::is_valid_pointer(signed short *ptr) const
{
	if ((ptr>=data) && (data<=dend)) return true;
	for (sample_loop_cache *slc=loop_cache; slc; slc=slc->next)
		if ((ptr>=slc->data) && (ptr<(slc->data+num_loop_cache_samples)))
			return true;
	return false;
}

//
//
//

bool spu_device::sample_cache::try_update(spu_device *spu)
{
	if ((invalid_start>=start) && (invalid_end<=end))
	{
		adpcm_packet *ap=(adpcm_packet *)(spu->spu_ram+start);
		unsigned int a;
		unsigned int loop=0;

		for (a=start; a<=end; a+=16, ap++)
		{
			if ((ap->flags&adpcmflag_loop_start) && (ap->flags&adpcmflag_loop)) loop=a;
			if (ap->flags&adpcmflag_end) break;
		}

		if ((a==(end-16)) && (loop==loopaddr))
		{
			#ifdef show_cache_update
				printf("updating %p: ",this);
			#endif

			if (invalid_start==start)
			{
				#ifdef show_cache_update
					printf("using end values");
				#endif

				update_decoder=decoder;
			} else
			if (invalid_start!=last_update_end)
			{
				#ifdef show_cache_update
					printf("resetting decoder (istrt=%08x lupd=%08x)",invalid_start,last_update_end);
				#endif

				update_decoder.reset();
			}
			#ifdef show_cache_update
				printf("\n");
			#endif

			signed short *dp=data+(((invalid_start-start)>>4)*28);
			ap=(adpcm_packet *)(spu->spu_ram+invalid_start);
			for (a=invalid_start; a<invalid_end; a+=16, ap++)
				dp=update_decoder.decode_packet(ap,dp);

			if (invalid_end==end)
			{
				#ifdef show_cache_update
					printf("updating end values\n");
				#endif
				decoder=update_decoder;
			}
			last_update_end=invalid_end;

			for (sample_loop_cache *lc=loop_cache; lc; lc=lc->next)
			{
				if (invalid_start==lc->loopstart)
				{
					adpcm_decoder tmp=decoder;
					dp=lc->data;
					signed short *dpend=dp+lc->len;
					unsigned int adr=lc->loopstart;
					for (unsigned int i=0; ((i<num_loop_cache_packets) && (dp<dpend)); i++, adr+=16)
						dp=tmp.decode_packet((adpcm_packet *)(spu->spu_ram+adr),dp);
				}
			}

			invalid_end=0;
			invalid_start=0xffffffff;
			valid=true;

			for (a=start; a<end; a+=16, ap++)
			{
				spu->cache[a>>4]=this;
			}

			add_ref();

			return true;
		}
	}

	return false;
}

//
//
//

void spu_device::cache_pointer::reset()
{
	if (cache)
	{
		ptr=nullptr;
		cache->remove_ref();
		cache=nullptr;
	}
}

//
//
//

spu_device::cache_pointer &spu_device::cache_pointer::operator =(const cache_pointer &other)
{
	if (cache) cache->remove_ref();
	ptr=other.ptr;
	cache=other.cache;
	if (cache) cache->add_ref();
	return *this;
}

//
//
//

bool spu_device::cache_pointer::update(spu_device *spu)
{
	if ((cache) && (! cache->valid))
	{
/*      log(log_spu,"cache_pointer::update: block %08x-%08x invalidated %08x-%08x\n",
                                cache->start,
                                cache->end,
                                cache->invalid_start,
                                cache->invalid_end);*/

		if (! cache->try_update(spu))
		{
			// Cache is invalid, calculate play address offset from start of
			// old cache block

			unsigned int off=ptr-cache->data,
										addr=cache->start;

			// Release cache block and get updated one

			spu->translate_sample_addr(addr,this);

			// Calculate play address in new cache block

			ptr=cache->data+off;

			if (ptr>=cache->dend)
			{
				// Play address is out of bounds in new cache block, release it and get a
				// new one starting at the current play address

				spu->translate_sample_addr(addr+((off/28)<<4),this);
			}
		}
	}

	// Return false if we do not have a cache block or the play address is invalid

	if ((cache) && ((ptr>=cache->data) && (ptr<cache->dend)))
	{
		return true;
	} else
	{
		reset();
		return false;
	}
}

//
//
//

signed short *adpcm_decoder::decode_packet(adpcm_packet *ap, signed short *dp)
{
	int shift=ap->info&0xf,
			filter=ap->info>>4,
			f0=filter_coef[filter][0],
			f1=filter_coef[filter][1];

	for (int i=0; i<14; i++)
	{
		unsigned char b=ap->data[i];
		short bl=(b&0xf)<<12,
					bh=(b>>4)<<12;

		bl=(bl>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		*dp++=bl;
		l1=l0;
		l0=bl;

		bh=(bh>>shift)+(((l0*f0)+(l1*f1)+32)>>6);
		*dp++=bh;
		l1=l0;
		l0=bh;
	}

	return dp;
}

//
//
//

static int shift_register15(int &shift)
{
	int bit0, bit1, bit14;

	bit0 = shift & 1;
	bit1 = (shift & 2) >> 1;
	bit14 = (bit0 ^ bit1) ^ 1;
	shift >>= 1;
	shift |= (bit14 << 14);
	return bit0;
}

//
//
//

//-------------------------------------------------
//  spu_device - constructor
//-------------------------------------------------

spu_device::spu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SPU, "SPU", tag, owner, clock, "spu", __FILE__),
	device_sound_interface(mconfig, *this),
	m_irq_handler(*this),
	dirty_flags(-1),
	status_enabled(false),
	xa_voll(0x8000),
	xa_volr(0x8000),
	changed_xa_vol(0)
{
}

//-------------------------------------------------
//  static_set_irqf - configuration helper to set
//  the IRQ callback
//-------------------------------------------------

void spu_device::device_start()
{
	m_irq_handler.resolve_safe();

	voice=new voiceinfo [24];
	spu_ram=new unsigned char [spu_ram_size];

	xa_buffer=new stream_buffer(xa_sector_size,xa_buffer_sectors);
	cdda_buffer=new stream_buffer(cdda_sector_size,cdda_buffer_sectors);

	init_stream();

	cache=new sample_cache *[spu_ram_size>>4];
	memset(cache,0,(spu_ram_size>>4)*sizeof(sample_cache *));

	// register save state stuff
	save_item(NAME(reg));           // this covers all spureg.* plus the reverb parameter block
	save_item(NAME(xa_cnt));
	save_item(NAME(cdda_cnt));
	save_item(NAME(xa_freq));
	save_item(NAME(cdda_freq));
	save_item(NAME(xa_channels));
	save_item(NAME(xa_spf));
	save_item(NAME(cur_frame_sample));
	save_item(NAME(cur_generate_sample));
	save_pointer(NAME(spu_ram), spu_ram_size);

	save_item(NAME(xa_buffer->head));
	save_item(NAME(xa_buffer->tail));
	save_item(NAME(xa_buffer->in));
	save_item(NAME(xa_buffer->sector_size));
	save_item(NAME(xa_buffer->num_sectors));
	save_item(NAME(xa_buffer->buffer_size));
	save_item(NAME(xa_buffer->buffer));

	save_item(NAME(cdda_buffer->head));
	save_item(NAME(cdda_buffer->tail));
	save_item(NAME(cdda_buffer->in));
	save_item(NAME(cdda_buffer->sector_size));
	save_item(NAME(cdda_buffer->num_sectors));
	save_item(NAME(cdda_buffer->buffer_size));
	save_item(NAME(cdda_buffer->buffer));
}

void spu_device::device_reset()
{
	cur_reverb_preset = nullptr;
	cur_frame_sample = 0;
	cur_generate_sample = 0;

	sample_cache::cache_size = 0;

	status_enabled = false;
	xa_voll = xa_volr = 0x8000;
	dirty_flags = -1;
	changed_xa_vol = 0;

	xa_cnt=0;
	xa_freq=0;
	xa_channels=2;
	xa_spf=0;
	xa_out_ptr=0;
	xa_playing=false;
	memset(xa_last,0,sizeof(xa_last));

	cdda_cnt=0;
	cdda_playing=false;
	m_cd_out_ptr = 0;

	memset(spu_ram,0,spu_ram_size);
	memset(reg,0,0x200);
	memset(voice,0,sizeof(voiceinfo)*24);

	spureg.status|=(1<<7)|(1<<10);

	memset(cache,0,(spu_ram_size>>4)*sizeof(sample_cache *));

	for (auto & elem : output_buf)
		elem=new unsigned char [output_buffer_size];
	output_head=output_tail=output_size=0;

	noise_t=0;
	noise_seed=12345;
	noise_cur=shift_register15(noise_seed)?0x7fff:0x8000;
}

void spu_device::device_post_load()
{
	// invalidate the SPURAM cache
	invalidate_cache(0, spu_ram_size);
	flush_output_buffer();

	// mark everything dirty
	dirty_flags = -1;

	// kill and reallocate reverb to avoid artifacts
	global_free(rev);
	rev = new reverb(44100);

	// and do some update processing
	update_reverb();
	update_key();
	update_voice_state();
	update_irq_event();
}

//
//
//
void spu_device::device_stop()
{
	for (auto & elem : output_buf)
		global_free_array(elem);

	kill_stream();

	global_free_array(spu_ram);
	invalidate_cache(0,spu_ram_size);
	global_free_array(cache);
	global_free(xa_buffer);
	global_free(cdda_buffer);
	global_free_array(voice);
}
//
//
//

void spu_device::init_stream()
{
	const unsigned int hz=44100;

	m_stream = machine().sound().stream_alloc(*this, 0, 2, hz);

	rev=new reverb(hz);

	cdda_freq=(unsigned int)((44100.0f/(float)hz)*4096.0f);
	freq_multiplier=(float)spu_base_frequency_hz/(float)hz;
}

//
//
//

void spu_device::kill_stream()
{
	global_free(rev);
	rev=nullptr;
}

//
//
//

void spu_device::reinit_sound()
{
	kill_stream();
	init_stream();
	flush_output_buffer();
	dirty_flags|=dirtyflag_voice_mask;
}

//
//
//

void spu_device::kill_sound()
{
	kill_stream();
}

//
//
//

READ16_MEMBER( spu_device::read )
{
	unsigned short ret, *rp=(unsigned short *)(reg+((offset*2)&0x1ff));

	m_stream->update();

	ret=*rp;

	#ifdef debug_spu_registers
		printf("spu: read word %08x = %04x [%s]\n",
													offset*2,
													ret,
													get_register_name(offset*2));
	#endif

	return ret;
}

//
//
//

WRITE16_MEMBER( spu_device::write )
{
	#ifdef debug_spu_registers
		printf("spu: write %08x = %04x [%s]\n",
													offset*2,
													data,
													get_register_name(offset*2));
	#endif

	m_stream->update();

	const unsigned int a=(offset*2)&0x1ff;
	switch (a)
	{
		case spureg_trans_addr:
			spureg.trans_addr=data;
			taddr=data<<3;
			break;

		case spureg_data:
			dirty_flags|=dirtyflag_ram;
			write_data(data);
			break;

		default:
		{
			unsigned short *rp=(unsigned short *)(reg+a);

			if ((a==spureg_irq_addr) ||
					((a==spureg_ctrl) && ((rp[0]^data)&spuctrl_irq_enable)))
				dirty_flags|=dirtyflag_irq;

			*rp=data;
			break;
		}
	}

	if ((a>spureg_reverb_config) && (a<=spureg_last))
		dirty_flags|=dirtyflag_reverb;

	if (a<=spureg_voice_last)
	{
		unsigned int v=(a>>4),r=(a&0xf);
		if (r==0xe)
		{
			voice[v].forceloop=true;
		}

		dirty_flags|=(1<<v);
	}

	update_key();
	update_vol(a);
	update_voice_state();
	update_irq_event();
}

//
//
//

void spu_device::update_vol(const unsigned int addr)
{
	if (addr<0x180)
	{
		unsigned int ch=(addr&0xf)>>1;
		if (ch<2)
		{
			unsigned int v=addr>>4;
			unsigned short newval=*(unsigned short *)(reg+addr);

			if (newval&0x8000)
			{
				#if 0
				printf("cur=%04x on=%d",voice[v].vol[ch],(spureg.chon>>ch)&1);
				switch ((newval>>13)&3)
				{
					case 0: printf("linear inc: phase=%d val=%02x\n",(newval>>12)&1,newval&0x7f); break;
					case 1: printf("linear dec: phase=%d val=%02x\n",(newval>>12)&1,newval&0x7f); break;
					case 2: printf("exp inc: phase=%d val=%02x\n",(newval>>12)&1,newval&0x7f); break;
					case 3: printf("exp dec: phase=%d val=%02x\n",(newval>>12)&1,newval&0x7f); break;
				}
				#endif
			}
			else
			{
				voice[v].vol[ch]=((int)newval<<17)>>17;
			}
		}
	}
}

//
//
//

void spu_device::write_data(const unsigned short data)
{
	#ifdef debug_spu_registers
		printf("spu: write data %04x @ %04x\n",data,taddr);
	#endif

	assert(taddr<spu_ram_size);
	if (cache[taddr>>4]) flush_cache(cache[taddr>>4],taddr,taddr+2);
	*((unsigned short *)(spu_ram+taddr))=data;
	taddr+=2;
}

//
//
//

void spu_device::update_key()
{
	dirty_flags|=((spureg.keyon|spureg.keyoff)&dirtyflag_voice_mask);

	if (spureg.keyoff)
	{
		unsigned int d=spureg.keyoff;
		for (int i=0; i<24; i++, d>>=1)
			if (d&1) key_off(i);
	}

	if (spureg.keyon)
	{
		unsigned int d=spureg.keyon;
		for (int i=0; i<24; i++, d>>=1)
			if (d&1) key_on(i);
		spureg.chon|=spureg.keyon;
	}

	spureg.keyon=spureg.keyoff=0;
}

//
//
//

void spu_device::flush_cache(sample_cache *sc,
													const unsigned int istart,
													const unsigned int iend)
{
	for (unsigned int a=sc->start; a<sc->end; a+=16)
		cache[a>>4]=nullptr;

/*  log_static(log_spu,"cache_invalidate: %08x->%08x\n",
                                         sc->start,
                                         sc->end);*/

	sc->invalid_start=min(sc->invalid_start,istart);
	sc->invalid_end=max(sc->invalid_end,iend);
	sc->valid=false;
	sc->remove_ref();
}

//
//
//

void spu_device::invalidate_cache(const unsigned int st, const unsigned int en)
{
	for (unsigned int a=st; a<en; a+=16)
		if (cache[a>>4]) flush_cache(cache[a>>4],st,en);
}

//
//
//

spu_device::sample_cache *spu_device::get_sample_cache(const unsigned int addr)
{
//  log_static(log_spu,"get_sample_cache: %08x\n",addr);

	assert(addr<spu_ram_size);
	sample_cache *sc=cache[addr>>4];
	if (sc) return sc;

	unsigned int loop=0;

	sc=new sample_cache;
	sc->valid=true;
	sc->start=addr;
	sc->loop=nullptr;

	adpcm_packet *ap=(adpcm_packet *)(spu_ram+sc->start);
	unsigned int a;
	for (a=addr; a<(512*1024); a+=16, ap++)
	{
		if (cache[a>>4]) flush_cache(cache[a>>4],a,a+16);
		cache[a>>4]=sc;

		if ((ap->flags&adpcmflag_loop_start) && (ap->flags&adpcmflag_loop)) loop=a;
		if (ap->flags&adpcmflag_end) break;
	}

	if ((a < 0x80000) && (ap->flags&adpcmflag_loop)) sc->is_loop=true;

	sc->end=min(spu_ram_size,a+16);

	unsigned int sz=((sc->end-sc->start)>>4)*28;
	sc->data=new signed short [sz];
	sample_cache::cache_size+=sz<<1;
	sc->loopaddr=loop;
	if (loop) sc->loop=sc->data+(((loop-sc->start)>>4)*28);

	signed short *dp=sc->data;
	ap=(adpcm_packet *)(spu_ram+sc->start);

	for (a=sc->start; a<sc->end; a+=16, ap++)
		dp=sc->decoder.decode_packet(ap,dp);

	sc->dend=dp;
	sc->add_ref();

/*  log_static(log_spu,"cache_add: %08x->%08x\n",
                                         sc->start,
                                         sc->end);*/

	return sc;
}

//
//
//

bool spu_device::translate_sample_addr(const unsigned int addr, cache_pointer *cp)
{
	assert((addr&0xf)==0);
	cp->reset();
	if ((cp->cache=get_sample_cache(addr)))
	{
		cp->ptr=cp->cache->data+(((addr-cp->cache->start)>>4)*28);
		cp->cache->add_ref();
		return true;
	}
	return false;
}

//
// Get distance in input samples to next IRQ for voice
//

unsigned int spu_device::get_irq_distance(const voiceinfo *vi)
{
	if (spureg.ctrl&spuctrl_irq_enable)
	{
		unsigned int irq_addr=spureg.irq_addr<<3;
		signed short *irq_ptr;

		if ((irq_ptr=vi->play.cache->get_sample_pointer(irq_addr)))
		{
			// IRQ address is inside this voices current cache block.  Return distance
			// if current play address is lower, or equal (and irq has not already
			// triggered)

			if ((vi->play.ptr<irq_ptr) ||
					((! vi->hitirq) && (vi->play.ptr==irq_ptr)))
			{
				return irq_ptr-vi->play.ptr;
			}
		}

		if ((vi->loop) &&
				(irq_ptr=vi->loop.cache->get_sample_pointer(irq_addr)) &&
				(irq_ptr>=vi->loop.ptr))
		{
			// IRQ address is inside this voices loop cache, return distance

			return (vi->play.cache->dend-vi->play.ptr)+
							(irq_ptr-vi->loop.ptr);
		}
	}

	// IRQs not enabled, or IRQ address not reachable by voice, distance is spu_infinity

	return spu_infinity;
}

//
//
//

void spu_device::update_voice_events(voiceinfo *vi)
{
	if (vi->pitch)
	{
		// Calculate time until end of sample in output samples

		vi->samplestoend=(unsigned int)((((INT64)(vi->play.cache->dend-vi->play.ptr)<<12)-vi->dptr)+(vi->pitch-1))/vi->pitch;
		if (vi->inloopcache)
		{
			// Voice is inside loop cache, return time until end of that if lower

			assert(vi->lcptr<vi->loop_cache->len);
			vi->samplestoend=min(vi->samplestoend,
														(unsigned int)((((INT64)(vi->loop_cache->len-vi->lcptr)<<12)-vi->dptr)+(vi->pitch-1))/vi->pitch);
		}

		// Calculate time until next IRQ in output samples

		unsigned int irqdist=get_irq_distance(vi);
		if (irqdist!=spu_infinity)
		{
			// Convert IRQ input sample distance to output samples

			vi->samplestoirq=(unsigned int)(((((INT64)irqdist)<<12)-vi->dptr)+(vi->pitch-1))/vi->pitch;
		} else
		{
			vi->samplestoirq=spu_infinity;
		}
	} else
	{
		// Voice pitch is 0, distance to sample end and IRQ is spu_infinity

		vi->samplestoend=vi->samplestoirq=spu_infinity;
	}
}

//
//
//

bool spu_device::update_voice_state(const unsigned int v)
{
	voicereg *vr=&spureg.voice[v];
	voiceinfo *vi=&voice[v];

	// Update sample cache if necessary

	if (! vi->play.update(this))
		return false;
	assert(vi->play.ptr<vi->play.cache->dend);

	// Get pitch from voice register and apply frequency multiplier if
	// there is one in effect

	vi->pitch=vr->pitch;
	vi->pitch=(unsigned int)(vi->pitch*freq_multiplier);

	// Update event times

	update_voice_events(vi);

	return true;
}

//
//
//

spu_device::sample_loop_cache *spu_device::get_loop_cache(sample_cache *cache, const unsigned int lpen, sample_cache *lpcache, const unsigned int lpst)
{
	// Check for existing loop cache

	sample_loop_cache *ret=lpcache->find_loop_cache(lpen,lpst);
	if (! ret)
	{
		// No loop cache exists for this address pair, create a new one

		auto lc=new sample_loop_cache;
		lc->loopend=lpen;
		lc->loopstart=lpst;
		lpcache->add_loop_cache(lc);
		ret=lc;

		// Decode samples from start address using decoder state at end address

		unsigned int adr=lpst;
		adpcm_decoder tmp=cache->decoder;
		signed short *dp=lc->data;
		for (unsigned int i=0; ((i<num_loop_cache_packets) &&
														(adr<lpcache->end)); i++, adr+=16)
			dp=tmp.decode_packet((adpcm_packet *)(spu_ram+adr),dp);

		#ifdef log_loop_cache
			log(log_spu,"spu: add loop cache %08x %08x->%08x (end at %08x)\n",lc,lpen,lpst,adr);
		#endif

		lc->len=dp-lc->data;
	}

	return ret;
}

//
//
//

void spu_device::update_voice_loop(const unsigned int v)
{
//  voicereg *vr=&spureg.voice[v];
	voiceinfo *vi=&voice[v];
	unsigned int ra = 0;

	// Check for looping using the voices repeat address register and get
	// a pointer to the loop position if enabled

	vi->loop.reset();

	// If loop address is not forced get the loop pointer from the cache
	// block (if present)

	if ((! voice[v].forceloop) &&
			(vi->play.cache->get_loop_pointer(&vi->loop)))
	{
		ra=vi->play.cache->loopaddr;
	}

	// Otherwise use the address set in repaddr (if set)

	if ((! vi->loop) && (vi->play.cache->is_loop))
	{
		ra=spureg.voice[v].repaddr<<3;
		ra=(ra+0xf)&~0xf;
		const adpcm_packet *ap=ra?(adpcm_packet *)(spu_ram+ra):nullptr;

		if (ap)
		{
			if (ap->flags&adpcmflag_loop)
			{
				// Repeat address points to a block with loop flag set

				if (! vi->play.cache->get_sample_pointer(ra,&vi->loop))
				{
					// Repeat address is in a different block

					translate_sample_addr(ra,&vi->loop);
				}
			}
		}
	}

	// Update loop cache

	if (vi->loop)
	{
		vi->loop_cache=get_loop_cache(vi->play.cache,vi->play.cache->end,vi->loop.cache,ra);
	}
}

//
//
//

void spu_device::update_voice_state()
{
	// If RAM or irq state is dirty make all voices dirty

	if (dirty_flags&(dirtyflag_ram|dirtyflag_irq))
	{
		dirty_flags|=dirtyflag_voice_mask;
		dirty_flags&=~(dirtyflag_ram|dirtyflag_irq);
	}

	// Update state for dirty voices

	if (dirty_flags&dirtyflag_voice_mask)
	{
		unsigned int voicemask=1;
		for (unsigned int i=0; i<24; i++, voicemask<<=1)
			if (dirty_flags&voicemask)
			{
				update_voice_state(i);
				dirty_flags&=~voicemask;
			}
	}
}

//
// Process voice state and build output segments
//
//  Input:  const unsigned int v        Voice number
//                  const unsigned int sz       Amount of time to process (in output samples)
//          unsigned int *tleft         Returned number of output samples remaining
//
//  Output: bool                                        true if voice is still playing
//

bool spu_device::process_voice(const unsigned int v,
														const unsigned int sz,
														void *ptr,
														void *fmnoise_ptr,
														void *outxptr,
														unsigned int *tleft)
{
	bool ret=true;
	unsigned int voice_mask=1<<v,
								num=sz,
								off=0;
	bool noise=((spureg.noise&voice_mask)!=0),
				fm=((spureg.fm&voice_mask)!=0);
	voiceinfo *vi=&voice[v];

	// Early exit if we don't have a sample cache block

	if (! vi->play)
	{
		*tleft=sz;
		return false;
	}

	// Generate samples

	while (num)
	{
		// Play up to end of sample, envelope event, or IRQ, whichever comes first

		unsigned int ntoplay=fm?1:num,
									nextevent=min(vi->samplestoend,
																min(vi->samplestoirq,vi->envsamples));
		ntoplay=min(ntoplay,nextevent);

		if (ntoplay)
		{
			signed short *noisep=nullptr;

			if (fm)
			{
				int fmv=((signed short *)fmnoise_ptr)[off<<1];
				vi->pitch=spureg.voice[v].pitch;
				vi->pitch=(unsigned int)(vi->pitch*freq_multiplier);
				vi->pitch=(vi->pitch*(fmv+32768))>>15;
			} else
			if (noise)
			{
				noisep=(signed short *)fmnoise_ptr;
				noisep+=(off<<1);
			}

			signed short *dp=(signed short *)ptr,
										*outxp=(signed short *)outxptr;
			dp+=off<<1;
			if (outxp) outxp+=off<<1;

			generate_voice(v, dp, noisep, outxp, ntoplay);

			num-=ntoplay;
			off+=ntoplay;

			vi->samplestoend-=ntoplay;
			if (vi->samplestoirq!=spu_infinity) vi->samplestoirq-=ntoplay;
			if (vi->envsamples!=spu_infinity) vi->envsamples-=ntoplay;
			vi->hitirq=false;
		}

		// Determine which event(s) we hit

		bool hitend=fm?(vi->play.ptr>=vi->play.cache->dend)
									:(vi->samplestoend==0),
					hitirq=(vi->samplestoirq==0),
					hitenv=(vi->envsamples==0);

		// Update loop cache pointer if we are playing a loop cache

		if ((vi->inloopcache) && (vi->lcptr>=vi->loop_cache->len))
		{
			vi->inloopcache=false;
			hitend=(vi->play.ptr>=vi->play.cache->dend);

			#ifdef log_loop_cache
				log(log_spu,"spu: %d leave loop cache %08x, lcptr=%d, hitend=%d\n",
										v,
										vi->loop_cache,
										vi->lcptr,
										hitend);
			#endif
		}

		bool stopped=false;

		if (hitend)
		{
			// End of sample reached, calculate how far we overshot

			unsigned int poff=vi->play.ptr-vi->play.cache->dend;

			// Make sure loop info is up to date and end the current output segment

			update_voice_loop(v);
			if (vi->loop)
			{
				// We are looping, set play address to loop address and account for
				// overshoot

				vi->play=vi->loop;
				vi->play.ptr+=poff;
				vi->lcptr=poff;
				vi->inloopcache=(poff<vi->loop_cache->len);

				#ifdef log_loop_cache
					if (vi->inloopcache)
						log(log_spu,"spu: %d enter loop cache %08x, lcptr=%d\n",
												v,
												vi->loop_cache,
												vi->lcptr);
				#endif

				// Check for IRQ at/just after repeat address

				if (spureg.ctrl&spuctrl_irq_enable)
				{
					if (spureg.voice[v].repaddr==spureg.irq_addr)
						hitirq=true;

					signed short *irq_ptr;
					unsigned int irq_addr=spureg.irq_addr<<3;

					if ((irq_ptr=vi->loop.cache->get_sample_pointer(irq_addr)))
					{
						if ((irq_ptr>=vi->loop.ptr) &&
								(vi->play.ptr>=irq_ptr))
							hitirq=true;
					}
				}
			} else
			{
				// Not looping, stop voice

				spureg.reverb&=~(1<<v);
				stopped=true;

				// Check for IRQ at repeat address

				if (spureg.ctrl&spuctrl_irq_enable)
				{
					if (spureg.voice[v].repaddr==spureg.irq_addr)
						hitirq=true;
				}
			}

			assert((stopped) || (vi->play.ptr<vi->play.cache->dend));
		} else
		{
			assert(vi->play.ptr<vi->play.cache->dend);
		}

		if (hitirq)
		{
			// Went past IRQ address, trigger IRQ
			m_irq_handler(1);

			vi->samplestoirq=spu_infinity;
			vi->hitirq=true;
		}

		if (hitenv)
		{
			// Envelope event, update the envelope (stop if necessary), and start
			// a new output segment

			stopped=((stopped) || (! update_envelope(v)));
		}

		if (stopped)
		{
			// Voice is now stopped

			ret=false;
			break;
		}

		// Update voice event times

		update_voice_events(vi);
	}

	// Set current volume register

	spureg.voice[v].curvol=(unsigned short)(vi->env_level*32767.0f);

	// Return how much time is left and whether or not the voice is still playing

	*tleft=num;
	return ret;
}

//
// Generate voice output samples
//
//  Inputs: const unsigned int v        Voice number
//            void *ptr                             Output buffer (if no reverb)
//          const unsigned int sz       Number of samples to output
//

void spu_device::generate_voice(const unsigned int v,
															void *ptr,
															void *noiseptr,
															void *outxptr,
															const unsigned int sz)
{
	voiceinfo *vi=&voice[v];
	signed short *fp,*sp;
	unsigned int n=sz;

	// Get input pointer

	if (vi->inloopcache)
	{
		sp=vi->loop_cache->data+vi->lcptr;
	} else
	{
		sp=vi->play.ptr;
	}
	fp=sp;

	unsigned int dptr=vi->dptr;

	// Get output pointer (and advance output offset)

	signed short *dp=(signed short *)ptr;
	signed short *outxp=(signed short *)outxptr;

	// Calculate fixed point envelope levels/deltas premultiplied by channel volume

	int vol_l=outxptr?0x3fff:vi->vol[0],
			vol_r=outxptr?0x3fff:vi->vol[1],
			env_l=(int)(vi->env_level*2.0f*vol_l),
			env_r=(int)(vi->env_level*2.0f*vol_r),
			envdelta_l=(int)(vi->env_delta*2.0f*vol_l),
			envdelta_r=(int)(vi->env_delta*2.0f*vol_r);

	// Update the segments envelope level

	vi->env_level+=(n*vi->env_delta);

	if (noiseptr)
	{
		INT64 dptr=((INT64)n*vi->pitch)+vi->dptr;
		unsigned int d=(unsigned int)(dptr>>12);
		vi->dptr=(unsigned int)(dptr&0xfff);
		vi->play.ptr+=d;
		if (vi->inloopcache) vi->lcptr+=d;

		sp=(signed short *)noiseptr;

		if (outxp)
		{
			while (n--)
			{
				int vl=*sp++,
						vr=*sp++,
						l=(vl*env_l)>>15,
						r=(vr*env_r)>>15;
				env_l+=envdelta_l;
				env_r+=envdelta_r;

				outxp[0]=l;
				outxp[1]=r;
				outxp+=2;

				l=(l*vi->vol[0])>>15;
				r=(r*vi->vol[1])>>15;

				dp[0]=clamp(l+dp[0]);
				dp[1]=clamp(r+dp[1]);
				dp+=2;
			}
		} else
		{
			while (n--)
			{
				int vl=*sp++,
						vr=*sp++,
						l=(vl*env_l)>>15,
						r=(vr*env_r)>>15;
				env_l+=envdelta_l;
				env_r+=envdelta_r;

				dp[0]=clamp(l+dp[0]);
				dp[1]=clamp(r+dp[1]);
				dp+=2;
			}
		}
	} else
	{
		if (1) //settings.sound_interpolate)
		{
			unsigned int num_stitch=0;
			signed short *ep;

			// Linear interpolation enabled, calculate how many samples we will
			// read from input data

			INT64 fracend=(((INT64)(n-1))*vi->pitch)+dptr;
			unsigned int end=(unsigned int)(fracend>>12);

			// Get pointer to last sample of input data

			if (vi->inloopcache)
			{
				ep=vi->loop_cache->data+vi->loop_cache->len-1;
			} else
			{
				ep=vi->play.cache->dend-1;
			}

			// If we read the last sample "stitching" will be necessary (inerpolation
			// from last sample of segment to first sample of next segment)

			if (((sp+end)>=ep) && (vi->pitch))
			{
				num_stitch=min(n,max(0x1fff/vi->pitch,1));
				n-=num_stitch;
			}

			// Generate samples

			if (outxp)
			{
				while (n--)
				{
					int v=sp[0];

					v+=((sp[1]-v)*(int)dptr)>>12;

					int l=(v*env_l)>>15,
							r=(v*env_r)>>15;
					env_l+=envdelta_l;
					env_r+=envdelta_r;

					outxp[0]=l;
					outxp[1]=r;
					outxp+=2;

					l=(l*vi->vol[0])>>15;
					r=(r*vi->vol[1])>>15;

					dp[0]=clamp(l+dp[0]);
					dp[1]=clamp(r+dp[1]);
					dp+=2;

					dptr+=vi->pitch;
					sp+=(dptr>>12);
					dptr&=0xfff;
				}
			}
			else
			{
				while (n--)
				{
					int v=sp[0];

					v+=((sp[1]-v)*(int)dptr)>>12;

					int l=(v*env_l)>>15,
							r=(v*env_r)>>15;
					env_l+=envdelta_l;
					env_r+=envdelta_r;

					dp[0]=clamp(l+dp[0]);
					dp[1]=clamp(r+dp[1]);
					dp+=2;

					dptr+=vi->pitch;
					sp+=(dptr>>12);
					dptr&=0xfff;
				}
			}

			if (num_stitch)
			{
				// Stitch samples, get the first sample of the next segment

				signed short *nsp=nullptr;

				if (vi->inloopcache)
				{
					nsp=vi->play.ptr+(vi->loop_cache->len-vi->lcptr);
					if (nsp>=vi->play.cache->dend)
						nsp=nullptr;
				}

				if (! nsp)
				{
					update_voice_loop(v);
					if (vi->loop) nsp=vi->loop_cache->data;
				}

				int ns=nsp?nsp[0]:0;

				// Generate stitch samples

				if (outxp)
				{
					while (num_stitch--)
					{
						int v=sp[0],
								nv=(sp>=ep)?ns:sp[1];

						v+=((nv-v)*(int)dptr)>>12;

						int l=(v*env_l)>>15,
								r=(v*env_r)>>15;
						env_l+=envdelta_l;
						env_r+=envdelta_r;

						outxp[0]=l;
						outxp[1]=r;
						outxp+=2;

						l=(l*vi->vol[0])>>15;
						r=(r*vi->vol[1])>>15;

						dp[0]=clamp(l+dp[0]);
						dp[1]=clamp(r+dp[1]);
						dp+=2;

						dptr+=vi->pitch;
						sp+=(dptr>>12);
						dptr&=0xfff;
					}
				} else
				{
					while (num_stitch--)
					{
						int v=sp[0],
								nv=(sp>=ep)?ns:sp[1];

						v+=((nv-v)*(int)dptr)>>12;

						int l=(v*env_l)>>15,
								r=(v*env_r)>>15;
						env_l+=envdelta_l;
						env_r+=envdelta_r;

						dp[0]=clamp(l+dp[0]);
						dp[1]=clamp(r+dp[1]);
						dp+=2;

						dptr+=vi->pitch;
						sp+=(dptr>>12);
						dptr&=0xfff;
					}
				}
			}
		} else
		{
			// Generate samples with no interpolation

			if (outxp)
			{
				while (n--)
				{
					int l=(sp[0]*env_l)>>15,
							r=(sp[0]*env_r)>>15;
					env_l+=envdelta_l;
					env_r+=envdelta_r;

					outxp[0]=l;
					outxp[1]=r;
					outxp+=2;

					l=(l*vi->vol[0])>>15;
					r=(r*vi->vol[1])>>15;

					dp[0]=clamp(l+dp[0]);
					dp[1]=clamp(r+dp[1]);
					dp+=2;

					dptr+=vi->pitch;
					sp+=(dptr>>12);
					dptr&=0xfff;
				}
			} else
			{
				while (n--)
				{
					int l=(sp[0]*env_l)>>15,
							r=(sp[0]*env_r)>>15;
					env_l+=envdelta_l;
					env_r+=envdelta_r;

					dp[0]=clamp(l+dp[0]);
					dp[1]=clamp(r+dp[1]);
					dp+=2;

					dptr+=vi->pitch;
					sp+=(dptr>>12);
					dptr&=0xfff;
				}
			}
		}

		// Update segment pointer

		vi->play.ptr+=sp-fp;
		vi->dptr=dptr;
		if (vi->inloopcache)
			vi->lcptr=sp-vi->loop_cache->data;
	}
}

//
//
//

bool spu_device::update_envelope(const int v)
{
	while (voice[v].envsamples==0)
	{
		voice[v].env_state++;

		switch (voice[v].env_state)
		{
			case 1: // decay
				voice[v].env_level=1.0f;
				voice[v].env_delta=voice[v].env_dr;
				if (voice[v].env_dr!=0.0f)
				{
					voice[v].envsamples=(unsigned int)((voice[v].env_sl-1.0f)/voice[v].env_dr);
				} else
				{
					voice[v].envsamples=spu_infinity;
				}
				break;

			case 2: // sustain
				voice[v].env_level=voice[v].env_sl;
				voice[v].env_delta=voice[v].env_sr;

				if (voice[v].env_sr>0.0f)
				{
					voice[v].envsamples=(unsigned int)((1.0f-voice[v].env_level)/voice[v].env_sr);
				} else
				if (voice[v].env_sr<0.0f)
				{
					voice[v].envsamples=(unsigned int)(voice[v].env_level/-voice[v].env_sr);
				} else
				{
					voice[v].envsamples=spu_infinity;
				}
				break;

			case 3: // sustain end
				voice[v].envsamples=spu_infinity;
				voice[v].env_delta=0.0f;
				if (voice[v].env_sr<=0.0f)
				{
					voice[v].env_level=0.0f;
					return false;
				} else
				{
					voice[v].env_level=1.0f;
				}
				break;

			case 4: // release
				voice[v].env_level=mindb(1.0f,maxdb(0.0f,voice[v].env_level));
				voice[v].env_delta=voice[v].env_rr;
				if (voice[v].env_rr == -0.0f)   // 0.0 release means infinite time
				{
					voice[v].envsamples=spu_infinity;
				}
				else
				{
					voice[v].envsamples=(unsigned int)(voice[v].env_level/-voice[v].env_rr);
				}
				break;

			case 5: // release end
				voice[v].env_level=0.0f;
				voice[v].env_delta=0.0f;
				voice[v].envsamples=spu_infinity;
				return false;
		}
	}
	return true;
}

//
//
//

void spu_device::key_on(const int v)
{
	voice[v].loop.reset();

//  printf("key_on: %d @ %x (pitch %x)\n", v, spureg.voice[v].addr<<3, spureg.voice[v].pitch);

	translate_sample_addr(spureg.voice[v].addr<<3,&voice[v].play);
	assert(voice[v].play.ptr<voice[v].play.cache->dend);

	voice[v].keyontime=0; //get_system_time();

	voice[v].dptr=0;
	voice[v].inloopcache=false;
	voice[v].lcptr=-1;
	voice[v].env_level=0.0f;
	voice[v].env_state=0;
	voice[v].forceloop=false;

	// Note: ChronoCross has note hang problems if this is 0 immediately
	//           after key on
	spureg.voice[v].curvol=1;

	for (unsigned int ch=0; ch<2; ch++)
	{
		{
			voice[v].sweep_vol[ch]=1.0f;
		}
	}

	#ifdef warn_if_sweep_used
		static bool sweepused;
		if ((spureg.voice[v].vol_l|spureg.voice[v].vol_r)&0x8000)
		{
			if (! sweepused)
			{
				printf("sweep\n");
				sweepused=true;
			}
		}
	#endif

	#ifdef assert_if_sweep_used
		assert(((spureg.voice[v].vol_l|spureg.voice[v].vol_r)&0x8000)==0);
	#endif

	if (spureg.voice[v].adsl&adsl_am)
	{
		voice[v].env_ar=get_pos_exp_rate((spureg.voice[v].adsl&adsl_ar_mask)>>adsl_ar_shift);
	} else
	{
		voice[v].env_ar=get_linear_rate((spureg.voice[v].adsl&adsl_ar_mask)>>adsl_ar_shift);
	}

	voice[v].env_dr=-get_decay_rate((spureg.voice[v].adsl&adsl_dr_mask)>>adsl_dr_shift);
	voice[v].env_sl=get_sustain_level(spureg.voice[v].adsl&adsl_sl_mask);

	if (spureg.voice[v].srrr&srrr_sm)
	{
		if (spureg.voice[v].srrr&srrr_sd)
		{
			voice[v].env_sr=get_neg_exp_rate((spureg.voice[v].srrr&srrr_sr_mask)>>srrr_sr_shift);
		} else
		{
			voice[v].env_sr=get_pos_exp_rate((spureg.voice[v].srrr&srrr_sr_mask)>>srrr_sr_shift);
		}
	} else
	{
		voice[v].env_sr=get_linear_rate((spureg.voice[v].srrr&srrr_sr_mask)>>srrr_sr_shift);
		if (spureg.voice[v].srrr&srrr_sd)
			voice[v].env_sr=-voice[v].env_sr;
	}

	if (spureg.voice[v].srrr&srrr_rm)
	{
		voice[v].env_rr=-get_exp_release_rate(spureg.voice[v].srrr&srrr_rr_mask);
	} else
	{
		voice[v].env_rr=-get_linear_release_rate(spureg.voice[v].srrr&srrr_rr_mask);
	}

	voice[v].envsamples=(unsigned int)(1.0f/voice[v].env_ar);
	voice[v].env_delta=voice[v].env_ar;
}

//
//
//

void spu_device::set_xa_format(const float _freq, const int channels)
{
	// Adjust frequency to compensate for slightly slower/faster frame rate
//  float freq=44100.0; //(_freq*get_adjusted_frame_rate())/ps1hw.rcnt->get_vertical_refresh();

	xa_freq=(unsigned int)((_freq/44100.0f)*4096.0f);
	xa_channels=channels;
	xa_spf=(unsigned int)(_freq/60.0f)*channels;
}

//
//
//

void spu_device::generate_xa(void *ptr, const unsigned int sz)
{
	if (xa_buffer->get_bytes_in())
	{
		// Don't start playing until 8 frames worth of data are in

	if ((! xa_playing) && (xa_buffer->get_bytes_in()<(xa_spf<<3)))
		{
//          debug_xa("waiting...\n");
		return;
		}

		xa_playing=true;

		// Init buffer pointers/counters

		int n=sz>>2;
		signed short *sp=(signed short *)xa_buffer->get_tail_ptr(),
									*dp=(signed short *)ptr;
		unsigned int noff=(1<<xa_channels),
									oldtail=xa_buffer->get_tail_offset();

		assert((xa_channels==1) || (xa_channels==2));

		// Calculate volume

		int voll=spureg.cdvol_l,
				volr=spureg.cdvol_r;
		voll=(voll*xa_voll)>>14;
		volr=(volr*xa_volr)>>14;

		// Generate requested number of XA samples

		while ((xa_buffer->get_bytes_in()) && (n--))
		{
			// Get left/right input samples

			int vl=sp[0],
					vr=sp[xa_channels-1];

			// Linear interpolation

			if (1) //settings.sound_interpolate)
			{
				signed short *nsp=(signed short *)xa_buffer->get_tail_ptr(noff);
				int vdl=nsp[0]-vl,
						vdr=nsp[xa_channels-1]-vr;

				vl+=(vdl*(int)xa_cnt)>>12;
				vr+=(vdr*(int)xa_cnt)>>12;
			}

			// Multiply by

			vl=(vl*voll)>>15;
			vr=(vr*volr)>>15;

			// Write to SPU XA buffer (for emulation purposes - some games read this
			// back to do analysers, etc...)

			*(signed short *)(spu_ram+xa_out_ptr)=vl;
			*(signed short *)(spu_ram+xa_out_ptr+0x800)=vr;
			xa_out_ptr=(xa_out_ptr+2)&0x7ff;

			// Mix samples into output buffer

			dp[0]=clamp(dp[0]+vl);
			dp[1]=clamp(dp[1]+vr);
			dp+=2;

			// Advance input counter/pointer

			xa_cnt+=xa_freq;
			int ss=(xa_cnt>>12);
			xa_cnt&=0xfff;

			if (ss)
			{
				ss<<=xa_channels;
				ss=min(ss,(int)xa_buffer->get_bytes_in());

				xa_buffer->increment_tail(ss);
				sp=(signed short *)xa_buffer->get_tail_ptr();
			}
		}

		// Delete buffer markers we have passed

		xa_buffer->delete_markers(oldtail);
	}

	// If we run out of input set status to stopped and clear the SPU XA buffer

	if (! xa_buffer->get_bytes_in())
	{
		xa_playing=false;

		memset(spu_ram,0,0x1000);
		xa_out_ptr=0;
	}
}

//
//
//

void spu_device::generate_cdda(void *ptr, const unsigned int sz)
{
	if (cdda_buffer->get_bytes_in())
	{
		unsigned int cdda_spf=(44100*4)/60.0,
									freq=(unsigned int)((cdda_freq*60.0)/60.0);

		if ((! cdda_playing) && (cdda_buffer->get_bytes_in()<(cdda_spf<<3)))
			return;

		cdda_playing=true;

		int n=sz>>2;
		signed short *sp=(signed short *)cdda_buffer->get_tail_ptr(),
									*dp=(signed short *)ptr;
		unsigned int oldtail=cdda_buffer->get_tail_offset();

		int voll=spureg.cdvol_l,
				volr=spureg.cdvol_r;

		while ((cdda_buffer->get_bytes_in()) && (n--))
		{
			INT16 vl = ((sp[0]*voll)>>15);
			INT16 vr = ((sp[1]*volr)>>15);

			// if the volume adjusted samples are stored here, vibribbon does nothing
			*(signed short *)(spu_ram+m_cd_out_ptr)=sp[0];
			*(signed short *)(spu_ram+m_cd_out_ptr+0x400)=sp[1];
			m_cd_out_ptr=(m_cd_out_ptr+2)&0x3ff;

			//if((m_cd_out_ptr == ((spureg.irq_addr << 3) & ~0x400)) && (spureg.ctrl & spuctrl_irq_enable))
			//  m_irq_handler(1);

			dp[0]=clamp(dp[0]+vl);
			dp[1]=clamp(dp[1]+vr);
			dp+=2;

			cdda_cnt+=freq;
			int ss=(cdda_cnt>>12);
			cdda_cnt&=0xfff;

			if (ss)
			{
				ss<<=2;

				cdda_buffer->increment_tail(ss);
				sp=(signed short *)cdda_buffer->get_tail_ptr();
			}
		}

		cdda_buffer->delete_markers(oldtail);

		if (! cdda_buffer->get_bytes_in())
			cdda_playing=false;

//      if (n>0) printf("cdda buffer underflow (n=%d cdda_in=%d spf=%d)\n",n,cdda_buffer->get_bytes_in(),cdda_spf);
	}
	else if(((spureg.irq_addr << 3) < 0x800) && (spureg.ctrl & spuctrl_irq_enable))
	{
		UINT16 irq_addr = (spureg.irq_addr << 3) & ~0x400;
		UINT32 end = m_cd_out_ptr + (sz >> 1);
		if(((m_cd_out_ptr < irq_addr) && (end > irq_addr)) || ((m_cd_out_ptr > (end & 0x3ff)) && ((end & 0x3ff) > irq_addr)))
			m_irq_handler(1);
		m_cd_out_ptr =  end & 0x3fe;
	}
}

//
//
//

void spu_device::key_off(const int v)
{
//  printf("key_off: %d\n", v);

	if (voice[v].env_state<=3)
	{
		voice[v].env_state=3;
		voice[v].envsamples=0;
	}
}

//
//
//

void spu_device::update_reverb()
{
	if (dirty_flags&dirtyflag_reverb)
	{
		cur_reverb_preset=find_reverb_preset((unsigned short *)&reg[0x1c0]);

		if (cur_reverb_preset==nullptr)
		{
//          printf("spu: reverb=unknown (reg 1c0 = %x)\n", reg[0x1c0]);
		} else
		{
//          printf("spu: reverb=%s\n",cur_reverb_preset->name);
			spu_reverb_cfg=&cur_reverb_preset->cfg;

			if ((core_stricmp("reverb off",cur_reverb_preset->name)) && (spu_reverb_cfg->band_gain<=0.0f))
			{
//              printf("spu: no reverb config for %s\n",cur_reverb_preset->name);
			}
		}

		dirty_flags&=~dirtyflag_reverb;
	}
}

//
//
//

void spu_device::flush_output_buffer()
{
	output_head=output_tail=output_size=0;
}

//
//
//

void spu_device::generate(void *ptr, const unsigned int sz)
{
	cur_generate_sample+=sz>>2;
	process_until(cur_generate_sample);

	update_reverb();

	unsigned int left=sz;
	unsigned char *dp=(unsigned char *)ptr;

	while ((left) && (output_size))
	{
		unsigned int n=min(min(left,output_buffer_size-output_head),output_size);
		memcpy(dp,output_buf[0]+output_head,n);

		rev->process((signed short *)dp,
									(signed short *)(output_buf[1]+output_head),
									spu_reverb_cfg,
									(signed short)spureg.rvol_l,
									(signed short)spureg.rvol_r,
									n);

		output_size-=n;
		output_head+=n;
		output_head&=(output_buffer_size-1);
		dp+=n;
		left-=n;
	}

	if (left)
	{
		memset(dp,0,left);
	}

	generate_xa(ptr,sz);
	generate_cdda(ptr,sz);
}

//
//
//

void spu_device::update_irq_event()
{
	if (spureg.ctrl&spuctrl_irq_enable)
	{
		unsigned int samplestoirq=spu_infinity;
		for (int i=0; i<24; i++)
			if (voice[i].samplestoirq!=spu_infinity)
			{
				if (voice[i].samplestoirq==0)
				{
					m_irq_handler(1);

					voice[i].samplestoirq=spu_infinity;
					voice[i].hitirq=true;
				} else
				{
					samplestoirq=min(samplestoirq,voice[i].samplestoirq);
				}
			}
	}
}


//
//
//

void spu_device::generate_noise(void *ptr, const unsigned int num)
{
	unsigned int np=(unsigned int)(65536.0f/(0x40-((spureg.ctrl&spuctrl_noise_mask)>>spuctrl_noise_shift)));
	np=((np<<1)+np)>>1;

	signed short *dp=(signed short *)ptr;

	for (unsigned int i=0; i<num; i++)
	{
		signed short v=noise_cur;
		*dp++=v;
		*dp++=v;
		noise_t+=np;

		if (noise_t>0xffff)
		{
			noise_t-=0xffff;
			shift_register15(noise_seed);
			noise_cur=noise_seed<<1;
		}
	}
}

//
//
//

void spu_device::process_until(const unsigned int tsample)
{
	while (tsample>cur_frame_sample)
	{
		unsigned int process_samples=(unsigned int)(tsample-cur_frame_sample);

		// Drop samples from the head of the queue if its full

		process_samples=min(process_samples,output_buffer_size>>2);
		unsigned int nsz=output_size+(process_samples<<2);
		if (nsz>output_buffer_size)
		{
			nsz-=output_buffer_size;

			output_head+=nsz;
			output_size-=nsz;
			output_head&=(output_buffer_size-1);
		}

		// Decide how many samples to process taking into account buffer
		// wrap in output queue.  Get pointers to the queues.

		process_samples=min(process_samples,
												(output_buffer_size-output_tail)>>2);

		unsigned char *outptr=output_buf[0]+output_tail,
									*reverbptr=output_buf[1]+output_tail,
									*fmptr=output_buf[2]+output_tail,
									*noiseptr=output_buf[3]+output_tail;

		output_tail+=process_samples<<2;
		output_tail&=(output_buffer_size-1);
		output_size+=process_samples<<2;
		assert(output_size<=output_buffer_size);

		// Intialise the output samples to 0 (process_voice always adds samples)

		memset(outptr,0,process_samples<<2);
		memset(reverbptr,0,process_samples<<2);

		// If noise is enabled for any channels generate noise samples

		if (spureg.noise&0xffffff)
			generate_noise(noiseptr,process_samples);

		unsigned int mask=1;
		for (int i=0; i<24; i++, mask<<=1)
		{
			unsigned int tleft=process_samples;
			bool isfmin=((i<23) && (spureg.fm&(1<<(i+1)))),
						isfm=(spureg.fm&(1<<i))!=0,
						isnoise=(spureg.noise&(1<<i))!=0,
						isreverb=(spureg.reverb&(1<<i))!=0;

			// This channel is an FM input for the next channel - clear the
			// FM input buffer

			if (isfmin)
				memset(fmptr,0,process_samples<<2);

			if (spureg.chon&mask)
			{
				// Generate samples

				if (! process_voice(i,
														process_samples,
														isreverb?reverbptr:outptr,
														isnoise?noiseptr
																:(isfm?fmptr:nullptr),
														isfmin?fmptr:nullptr,
														&tleft))
				{
					spureg.chon&=~mask;
					//spureg.reverb&=~mask;

					voice[i].play.reset();
					voice[i].loop.reset();
				}
			} else
			{
				spureg.voice[i].curvol=0;
			}
		}

		cur_frame_sample+=process_samples;
	}
}

//
//
//

void spu_device::update_timing()
{
	samples_per_frame=44100.0/60.0; //get_adjusted_frame_rate();
	samples_per_cycle=samples_per_frame/60*(44100*768); //ps1hw.rcnt->get_vertical_cycles();

}

//
//
//

void spu_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *outL, *outR;
	INT16 temp[44100], *src;

	outL = outputs[0];
	outR = outputs[1];

	generate(temp, samples*4);  // second parameter is bytes, * 2 (size of INT16) * 2 (stereo)

	src = &temp[0];
	for (int i = 0; i < samples; i++)
	{
		*outL++ = *src++;
		*outR++ = *src++;
	}
}

//
//
//

void spu_device::start_dma(UINT8 *mainram, bool to_spu, UINT32 size)
{
	UINT32 st=spureg.trans_addr<<3, en=st+size;

	if (en>(512*1024))
	{
		en=512*1024;
		size=en-st;
	}

	if (to_spu)
	{
		invalidate_cache(st,en);

		memcpy(spu_ram+(spureg.trans_addr<<3), mainram, size);

		dirty_flags|=dirtyflag_ram;
	}
	else
	{
		memcpy(mainram, spu_ram+(spureg.trans_addr<<3), size);
	}
}

//
//
//

void spu_device::decode_xa_mono(const unsigned char *xa,
															unsigned char *ptr)
{
	signed short *dp=(signed short *)ptr;

	int l0=xa_last[0],
			l1=xa_last[1];

	for (int b=0; b<18; b++)
	{
		for (int s=0; s<4; s++)
		{
			unsigned char flags=xa[4+(s<<1)],
										shift=flags&0xf,
										filter=flags>>4;
			int f0=filter_coef[filter][0],
					f1=filter_coef[filter][1];
			int i;

			for (i=0; i<28; i++)
			{
				short d=(xa[16+(i<<2)+s]&0xf)<<12;
				d=clamp((d>>shift)+(((l0*f0)+(l1*f1)+32)>>6));
				*dp++=d;
				l1=l0;
				l0=d;
			}

			flags=xa[5+(s<<1)];
			shift=flags&0xf;
			filter=flags>>4;
			f0=filter_coef[filter][0];
			f1=filter_coef[filter][1];

			for (i=0; i<28; i++)
			{
				short d=(xa[16+(i<<2)+s]>>4)<<12;
				d=clamp((d>>shift)+(((l0*f0)+(l1*f1)+32)>>6));
				*dp++=d;
				l1=l0;
				l0=d;
			}
		}

		xa+=128;
	}

	xa_last[0]=l0;
	xa_last[1]=l1;
}

//
//
//

void spu_device::decode_xa_stereo(const unsigned char *xa,
																unsigned char *ptr)
{
	signed short *dp=(signed short *)ptr;

	int l0=xa_last[0],
			l1=xa_last[1],
			l2=xa_last[2],
			l3=xa_last[3];

	for (int b=0; b<18; b++)
	{
		for (int s=0; s<4; s++)
		{
			unsigned char flags0=xa[4+(s<<1)],
										shift0=flags0&0xf,
										filter0=flags0>>4,
										flags1=xa[5+(s<<1)],
										shift1=flags1&0xf,
										filter1=flags1>>4;

			int f0=filter_coef[filter0][0],
					f1=filter_coef[filter0][1],
					f2=filter_coef[filter1][0],
					f3=filter_coef[filter1][1];

			for (int i=0; i<28; i++)
			{
				short d=xa[16+(i<<2)+s],
							d0=(d&0xf)<<12,
							d1=(d>>4)<<12;
				d0=clamp((int)(d0>>shift0)+(((l0*f0)+(l1*f1)+32)>>6));
				*dp++=d0;
				l1=l0;
				l0=d0;

				d1=clamp((int)(d1>>shift1)+(((l2*f2)+(l3*f3)+32)>>6));
				*dp++=d1;
				l3=l2;
				l2=d1;
			}
		}

		xa+=128;
	}

	xa_last[0]=l0;
	xa_last[1]=l1;
	xa_last[2]=l2;
	xa_last[3]=l3;
}

//
//
//

/*
enum
{
    xaencoding_stereo_mask=3,
    xaencoding_freq_shift=2,
    xaencoding_freq_mask=3<<xaencoding_freq_shift,
    xaencoding_bps_shift=4,
    xaencoding_bps_mask=3<<xaencoding_bps_shift,
    xaencoding_emphasis=(1<<6)
};
*/

bool spu_device::play_xa(const unsigned int sector, const unsigned char *xa)
{
	// Don't process the sector if the buffer is full

	if (xa_buffer->get_bytes_free()<xa_sector_size) return false;

//  debug_xa("play_xa: %d\n",sector);

	// Get XA format from sector header

	const unsigned char *hdr=xa+4;
	float freq;
	int channels;

	switch (hdr[3]&0x3f)    // ignore emphasis and reserved bits
	{
		case 0:
			channels=1;
			freq=37800.0f;  //18900.0f;
			break;

		case 1:
			channels=2;
			freq=37800.0f;
			break;

		case 4:
			channels=1;
			freq=18900.0f;  ///2.0f;
			break;

		case 5:
			channels=2;
			freq=18900.0f;  //37800.0f/2.0f;
			break;

		default:
			printf("play_xa: unhandled xa mode %08x\n",hdr[3]);
			return true;
	}

	set_xa_format(freq,channels);

	// Store XA marker

	unsigned char *ptr=xa_buffer->add_sector(sector);

	// Decode the sector

	if (channels==2)
	{
		decode_xa_stereo(xa+8,ptr);
	} else
	{
		decode_xa_mono(xa+8,ptr);
	}

	// Return that we processed the sector
	return true;
}

//
// Flush everything after a given sector in the XA buffer
//

void spu_device::flush_xa(const unsigned int sector)
{
//  debug_xa("flush_xa: %d\n",sector);

	if (xa_playing)
	{
		xa_buffer->flush(sector);
	} else
	{
		// Not playing, flush the entire buffer

		xa_buffer->flush_all();
		xa_cnt=0;
	}
}

//
//
//

bool spu_device::play_cdda(const unsigned int sector, const unsigned char *cdda)
{
	if (cdda_buffer->get_bytes_free()<cdda_sector_size) return false;

	signed short *dp=(signed short *)cdda_buffer->add_sector(sector);
	memcpy(dp,cdda,cdda_sector_size);

	// data coming in in MAME is big endian as stored on the CD
	unsigned char *flip = (unsigned char *)dp;
	for (int i = 0; i < cdda_sector_size; i+= 2)
	{
		unsigned char temp = flip[i];
		flip[i] = flip[i+1];
		flip[i+1] = temp;
	}
	// this should be done in generate but sound_stream_update may not be called frequently enough
	if(((spureg.irq_addr << 3) < 0x800) && (spureg.ctrl & spuctrl_irq_enable))
		m_irq_handler(1);

	return true;
}

void spu_device::flush_cdda(const unsigned int sector)
{
//  debug_xa("flush_cdda: %d\n",sector);

	if (cdda_playing)
	{
		cdda_buffer->flush(sector);
	} else
	{
		cdda_buffer->flush_all();
		cdda_cnt=0;
	}
}

void spu_device::dma_read( UINT32 *p_n_ram, UINT32 n_address, INT32 n_size )
{
	UINT8 *psxram = (UINT8 *)p_n_ram;

	start_dma(psxram + n_address, false, n_size*4);
}

void spu_device::dma_write( UINT32 *p_n_ram, UINT32 n_address, INT32 n_size )
{
	UINT8 *psxram = (UINT8 *)p_n_ram;

//  printf("SPU DMA write from %x, size %x\n", n_address, n_size);

	start_dma(psxram + n_address, true, n_size*4);
}
