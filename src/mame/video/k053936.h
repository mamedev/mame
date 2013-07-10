#pragma once
#ifndef __K053936_H__
#define __K053936_H__


/* */





extern UINT16 *K053936_0_ctrl,*K053936_0_linectrl;
//extern UINT16 *K053936_1_ctrl,*K053936_1_linectrl;
void K053936_0_zoom_draw(bitmap_ind16 &bitmap,const rectangle &cliprect,tilemap_t *tmap,int flags,UINT32 priority, int glfgreat_hack);
void K053936_wraparound_enable(int chip, int status);
void K053936_set_offset(int chip, int xoffs, int yoffs);

#endif
