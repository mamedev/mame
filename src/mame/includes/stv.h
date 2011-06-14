/*----------- defined in drivers/stv.c -----------*/

class saturn_state : public driver_device
{
public:
	saturn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32    *m_workram_l;
	UINT32    *m_workram_h;
	UINT8     *m_smpc_ram;
	UINT32    *m_backupram;
	UINT32    *m_scu_regs;
	UINT16    *m_sound_ram;
	UINT16    *m_scsp_regs;
	UINT32    *m_vdp2_regs;
	UINT32    *m_vdp2_vram;
	UINT32    *m_vdp2_cram;
    UINT32    *m_vdp1_vram;
    UINT16    *m_vdp1_regs;

	UINT8     m_NMI_reset;
	UINT8     m_en_68k;

	struct {
		UINT32    src[3];		/* Source DMA lv n address*/
		UINT32    dst[3];		/* Destination DMA lv n address*/
		UINT32    src_add[3];	/* Source Addition for DMA lv n*/
		UINT32    dst_add[3];	/* Destination Addition for DMA lv n*/
		INT32     size[3];		/* Transfer DMA size lv n*/
		UINT32    index[3];
		int       start_factor[3];
		UINT8     enable_mask[3];
	}m_scu;

	int       m_minit_boost;
	int       m_sinit_boost;
	attotime  m_minit_boost_timeslice;
	attotime  m_sinit_boost_timeslice;

	struct {
		UINT16    **framebuffer_display_lines;
		int       framebuffer_mode;
		int       framebuffer_double_interlace;
		int	      fbcr_accessed;
        int       framebuffer_width;
        int       framebuffer_height;
        int       framebuffer_current_display;
        int	      framebuffer_current_draw;
        int	      framebuffer_clear_on_next_frame;
		rectangle system_cliprect;
		rectangle user_cliprect;
        UINT16	  *framebuffer[2];
        UINT16	  **framebuffer_draw_lines;
	    UINT8     *gfx_decode;

		int       local_x;
		int       local_y;
	}m_vdp1;

	struct {
	    UINT8     *gfx_decode;
	    bitmap_t  *roz_bitmap[2];
	    UINT8     dotsel;
	    UINT8     pal;
	}m_vdp2;

	struct {
		UINT8 IOSEL1;
        UINT8 IOSEL2;
        UINT8 EXLE1;
        UINT8 EXLE2;
        UINT8 PDR1;
        UINT8 PDR2;
        int   intback_stage;
        int   smpcSR;
        int   pmode;
        UINT8 SMEM[4];
        UINT8 intback;
	}m_smpc;

	/* Saturn specific*/
	int m_saturn_region;
	UINT8 m_cart_type;
	UINT32 *m_cart_dram;

	/* ST-V specific */
	UINT8     m_stv_multi_bank;
	UINT8     m_prev_bankswitch;
    emu_timer *m_stv_rtc_timer;
	UINT32    *m_ioga;
	UINT8     m_instadma_hack;

	legacy_cpu_device* m_maincpu;
	legacy_cpu_device* m_slave;
	legacy_cpu_device* m_audiocpu;
};

#define MASTER_CLOCK_352 57272800
#define MASTER_CLOCK_320 53748200
#define CEF_1	state->m_vdp1_regs[0x010/2]|=0x0002
#define CEF_0   state->m_vdp1_regs[0x010/2]&=~0x0002
#define BEF_1	state->m_vdp1_regs[0x010/2]|=0x0001
#define BEF_0	state->m_vdp1_regs[0x010/2]&=~0x0001
#define STV_VDP1_TVMR ((state->m_vdp1_regs[0x000/2])&0xffff)
#define STV_VDP1_VBE  ((STV_VDP1_TVMR & 0x0008) >> 3)
#define STV_VDP1_TVM  ((STV_VDP1_TVMR & 0x0007) >> 0)


DRIVER_INIT ( stv );


/*----------- defined in drivers/stvinit.c -----------*/

UINT8 get_vblank(running_machine &machine);
void install_stvbios_speedups(running_machine &machine);
DRIVER_INIT(mausuke);
DRIVER_INIT(puyosun);
DRIVER_INIT(shienryu);
DRIVER_INIT(prikura);
DRIVER_INIT(hanagumi);
DRIVER_INIT(cottonbm);
DRIVER_INIT(cotton2);
DRIVER_INIT(fhboxers);
DRIVER_INIT(dnmtdeka);
DRIVER_INIT(groovef);
DRIVER_INIT(danchih);
DRIVER_INIT(astrass);
DRIVER_INIT(thunt);
DRIVER_INIT(grdforce);
DRIVER_INIT(batmanfr);
DRIVER_INIT(winterht);
DRIVER_INIT(seabass);
DRIVER_INIT(vfremix);
DRIVER_INIT(diehard);
DRIVER_INIT(sss);
DRIVER_INIT(othellos);
DRIVER_INIT(sasissu);
DRIVER_INIT(gaxeduel);
DRIVER_INIT(suikoenb);
DRIVER_INIT(sokyugrt);
DRIVER_INIT(znpwfv);
DRIVER_INIT(twcup98);
DRIVER_INIT(smleague);
DRIVER_INIT(maruchan);
DRIVER_INIT(sandor);
DRIVER_INIT(colmns97);
DRIVER_INIT(pblbeach);
DRIVER_INIT(shanhigw);
DRIVER_INIT(finlarch);
DRIVER_INIT(elandore);
DRIVER_INIT(rsgun);
DRIVER_INIT(ffreveng);
DRIVER_INIT(decathlt);
DRIVER_INIT(nameclv3);

/*----------- defined in video/stvvdp1.c -----------*/

extern UINT16	**stv_framebuffer_display_lines;
extern int stv_framebuffer_double_interlace;
extern int stv_framebuffer_mode;
extern UINT8* stv_vdp1_gfx_decode;

int stv_vdp1_start ( running_machine &machine );
void video_update_vdp1(running_machine &machine);
void stv_vdp2_dynamic_res_change(running_machine &machine);

READ16_HANDLER ( saturn_vdp1_regs_r );
READ32_HANDLER ( saturn_vdp1_vram_r );
READ32_HANDLER ( saturn_vdp1_framebuffer0_r );

WRITE16_HANDLER ( saturn_vdp1_regs_w );
WRITE32_HANDLER ( saturn_vdp1_vram_w );
WRITE32_HANDLER ( saturn_vdp1_framebuffer0_w );

/*----------- defined in video/stvvdp2.c -----------*/

READ32_HANDLER ( saturn_vdp2_vram_r );
READ32_HANDLER ( saturn_vdp2_cram_r );
READ32_HANDLER ( saturn_vdp2_regs_r );

WRITE32_HANDLER ( saturn_vdp2_vram_w );
WRITE32_HANDLER ( saturn_vdp2_cram_w );
WRITE32_HANDLER ( saturn_vdp2_regs_w );

VIDEO_START ( stv_vdp2 );
SCREEN_UPDATE( stv_vdp2 );

