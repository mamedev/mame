#ifndef _DIGITALKER_H_
#define _DIGITALKER_H_

void digitalker_0_cs_w(int line);
void digitalker_0_cms_w(int line);
void digitalker_0_wr_w(int line);
int digitalker_0_intr_r(void);
WRITE8_HANDLER(digitalker_0_data_w);

SND_GET_INFO(digitalker);
#define SOUND_DIGITALKER SND_GET_INFO_NAME(digitalker)

#endif
