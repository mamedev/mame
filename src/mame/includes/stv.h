/*----------- defined in drivers/stv.c -----------*/

extern UINT32 *stv_workram_h;
extern UINT32 *stv_workram_l;

extern int stv_enable_slave_sh2;

extern int minit_boost,sinit_boost;
extern attotime minit_boost_timeslice, sinit_boost_timeslice;

extern cpu_device *stv_maincpu;
extern cpu_device *stv_slave;
extern cpu_device *stv_audiocpu;

DRIVER_INIT ( stv );


/*----------- defined in drivers/stvinit.c -----------*/

void install_stvbios_speedups(running_machine *machine);
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

extern UINT32* stv_vdp1_vram;
extern UINT16	**stv_framebuffer_display_lines;
extern int stv_framebuffer_double_interlace;
extern int stv_framebuffer_mode;
extern UINT8* stv_vdp1_gfx_decode;

int stv_vdp1_start ( running_machine *machine );
void video_update_vdp1(running_machine *machine);

READ32_HANDLER( stv_vdp1_regs_r );
WRITE32_HANDLER( stv_vdp1_regs_w );
READ32_HANDLER ( stv_vdp1_vram_r );
WRITE32_HANDLER ( stv_vdp1_vram_w );

WRITE32_HANDLER ( stv_vdp1_framebuffer0_w );
READ32_HANDLER ( stv_vdp1_framebuffer0_r );

/*----------- defined in video/stvvdp2.c -----------*/

extern UINT32* stv_vdp2_regs;
extern UINT32* stv_vdp2_vram;
extern int stv_vblank,stv_hblank;
extern UINT32* stv_vdp2_cram;

UINT8 stv_get_vblank(running_machine *machine);

WRITE32_HANDLER ( stv_vdp2_vram_w );
READ32_HANDLER ( stv_vdp2_vram_r );

WRITE32_HANDLER ( stv_vdp2_cram_w );
READ32_HANDLER ( stv_vdp2_cram_r );

WRITE32_HANDLER ( stv_vdp2_regs_w );
READ32_HANDLER ( stv_vdp2_regs_r );

VIDEO_START ( stv_vdp2 );
VIDEO_UPDATE( stv_vdp2 );

