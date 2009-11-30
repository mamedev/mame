/***************************************************************************

    tagmap.h

    Simple tag->object mapping functions.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __TAGMAP_H__
#define __TAGMAP_H__

#include "osdcore.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TAGMAP_HASH_SIZE	97


enum _tagmap_error
{
	TMERR_NONE,
	TMERR_OUT_OF_MEMORY,
	TMERR_DUPLICATE
};
typedef enum _tagmap_error tagmap_error;



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _tagmap_entry tagmap_entry;
struct _tagmap_entry
{
	tagmap_entry *		next;
	void *				object;
	UINT32				fullhash;
	char				tag[1];
};


typedef struct _tagmap tagmap;
struct _tagmap
{
	tagmap_entry *		table[TAGMAP_HASH_SIZE];
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- map allocation and management ----- */

/* allocate a new tagmap */
tagmap *tagmap_alloc(void);

/* free a tagmap, and all entries within it */
void tagmap_free(tagmap *map);

/* reset a tagmap by freeing all entries */
void tagmap_reset(tagmap *map);


/* ----- object management ----- */

/* add a new entry to a tagmap */
tagmap_error tagmap_add(tagmap *map, const char *tag, void *object);

/* add a new entry to a tagmap, ensuring it has a unique hash value */
tagmap_error tagmap_add_unique_hash(tagmap *map, const char *tag, void *object);

/* remove an entry from a tagmap */
void tagmap_remove(tagmap *map, const char *tag);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    tagmap_hash - compute the hash of a tag
-------------------------------------------------*/

INLINE UINT32 tagmap_hash(const char *string)
{
	UINT32 hash = (string[0] << 5) + string[1];
	char c;

	string += 2;
	while ((c = *string++) != 0)
		hash = ((hash << 5) | (hash >> 27)) + c;
	return hash;
}


/*-------------------------------------------------
    tagmap_find_prehashed - find an object
    associated with a tag, given the tag's
    hash
-------------------------------------------------*/

INLINE void *tagmap_find_prehashed(tagmap *map, const char *tag, UINT32 fullhash)
{
	tagmap_entry *entry;

	for (entry = map->table[fullhash % ARRAY_LENGTH(map->table)]; entry != NULL; entry = entry->next)
		if (entry->fullhash == fullhash && strcmp(entry->tag, tag) == 0)
			return entry->object;
	return NULL;
}


/*-------------------------------------------------
    tagmap_find - find an object associated
    with a tag
-------------------------------------------------*/

INLINE void *tagmap_find(tagmap *map, const char *tag)
{
	return tagmap_find_prehashed(map, tag, tagmap_hash(tag));
}


/*-------------------------------------------------
    tagmap_find_hash_only - find an object
    associated with a tag using only the hash;
    this generally works well but may occasionally
    return a false positive
-------------------------------------------------*/

INLINE void *tagmap_find_hash_only(tagmap *map, const char *tag)
{
	UINT32 fullhash = tagmap_hash(tag);
	tagmap_entry *entry;

	for (entry = map->table[fullhash % ARRAY_LENGTH(map->table)]; entry != NULL; entry = entry->next)
		if (entry->fullhash == fullhash)
			return entry->object;
	return NULL;
}


#endif /* __TAGMAP_H__ */
