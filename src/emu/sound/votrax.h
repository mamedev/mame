#pragma once

#ifndef __VOTRAX_H__
#define __VOTRAX_H__

void votrax_w(int data);
int votrax_status_r(void);

SND_GET_INFO( votrax );
#define SOUND_VOTRAX SND_GET_INFO_NAME( votrax )

#endif /* __VOTRAX_H__ */
