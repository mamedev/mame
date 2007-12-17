#ifndef BEEP_H
#define BEEP_H

#ifdef __cplusplus
extern "C" {
#endif

void beep_set_state(int,int);
void beep_set_frequency(int,int);
void beep_set_volume(int,int);

#ifdef __cplusplus
}
#endif

#endif
