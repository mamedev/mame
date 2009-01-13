#pragma once

#ifndef __N63701X_H__
#define __N63701X_H__

void namco_63701x_write(int offset,int data);

SND_GET_INFO( namco_63701x );
#define SOUND_NAMCO_63701X SND_GET_INFO_NAME( namco_63701x )

#endif /* __N63701X_H__ */
