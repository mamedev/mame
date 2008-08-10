/* namcona1.h */

#pragma once

#ifndef __NAMCONA_H__
#define __NAMCONA_H__

typedef struct _namcona_interface namcona_interface;
struct _namcona_interface
{
    void *memory_base;
    int metadata_offset;
};

#endif /* __NAMCONA_H__ */
