#pragma once

#ifndef __BEEP_H__
#define __BEEP_H__

#ifdef __cplusplus
extern "C" {
#endif

void beep_set_state(int,int);
void beep_set_frequency(int,int);
void beep_set_volume(int,int);

SND_GET_INFO( beep );

#ifdef __cplusplus
}
#endif

#endif /* __BEEP_H__ */
