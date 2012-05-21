/*************************************************************************************

    AWP Hardware video simulation system

*************************************************************************************/
#ifndef AWP_VIDEO
#define AWP_VIDEO

extern const char layout_awpvid14[];	/* main layout positioning 6 reels, a lamp matrix and a 14seg VFD */
extern const char layout_awpvid16[];	/* main layout positioning 6 reels, a lamp matrix and a 16seg VFD */
void awp_draw_reel(int rno);

#endif
