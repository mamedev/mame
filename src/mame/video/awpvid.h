/*************************************************************************************

    AWP Hardware video simulation system

*************************************************************************************/
#ifndef AWP_VIDEO
#define AWP_VIDEO

extern const char layout_awpvid14[];	/* main layout positioning 6 reels, a lamp matrix and a 14seg VFD */
extern const char layout_awpvid16[];	/* main layout positioning 6 reels, a lamp matrix and a 16seg VFD */
extern void awp_draw_reel(int rno);

extern void awp_reel_setup(void);

#endif
