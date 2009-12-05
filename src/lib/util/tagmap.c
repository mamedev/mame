/***************************************************************************

    tagmap.c

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

****************************************************************************/

#include "tagmap.h"


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static tagmap_error tagmap_add_common(tagmap *map, const char *tag, void *object, UINT8 replace_if_duplicate, UINT8 unique_hash);



/***************************************************************************
    MAP ALLOCATION AND MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    tagmap_alloc - allocate a new tagmap
-------------------------------------------------*/

tagmap *tagmap_alloc(void)
{
	tagmap *map = malloc(sizeof(*map));
	if (map != NULL)
		memset(map, 0, sizeof(*map));
	return map;
}


/*-------------------------------------------------
    tagmap_free - free a tagmap, and all
    entries within it
-------------------------------------------------*/

void tagmap_free(tagmap *map)
{
	tagmap_reset(map);
	free(map);
}


/*-------------------------------------------------
    tagmap_reset - reset a tagmap by freeing
    all entries
-------------------------------------------------*/

void tagmap_reset(tagmap *map)
{
	UINT32 hashindex;

	for (hashindex = 0; hashindex < ARRAY_LENGTH(map->table); hashindex++)
	{
		tagmap_entry *entry, *next;

		for (entry = map->table[hashindex]; entry != NULL; entry = next)
		{
			next = entry->next;
			free(entry);
		}
	}
}



/***************************************************************************
    MAP ALLOCATION AND MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    tagmap_add - add a new object to the
    tagmap
-------------------------------------------------*/

tagmap_error tagmap_add(tagmap *map, const char *tag, void *object, UINT8 replace_if_duplicate)
{
	return tagmap_add_common(map, tag, object, replace_if_duplicate, FALSE);
}


/*-------------------------------------------------
    tagmap_add_unique_hash - add a new entry to a
    tagmap, ensuring it has a unique hash value
-------------------------------------------------*/

tagmap_error tagmap_add_unique_hash(tagmap *map, const char *tag, void *object, UINT8 replace_if_duplicate)
{
	return tagmap_add_common(map, tag, object, replace_if_duplicate, TRUE);
}


/*-------------------------------------------------
    tagmap_remove - remove an object from a
    tagmap
-------------------------------------------------*/

void tagmap_remove(tagmap *map, const char *tag)
{
	UINT32 fullhash = tagmap_hash(tag);
	tagmap_entry **entryptr;

	for (entryptr = &map->table[fullhash % ARRAY_LENGTH(map->table)]; *entryptr != NULL; entryptr = &(*entryptr)->next)
		if ((*entryptr)->fullhash == fullhash && strcmp((*entryptr)->tag, tag) == 0)
		{
			tagmap_entry *entry = *entryptr;
			*entryptr = entry->next;
			free(entry);
			break;
		}
}



/***************************************************************************
    LOCAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    tagmap_add_common - core implementation of
    a tagmap addition
-------------------------------------------------*/

static tagmap_error tagmap_add_common(tagmap *map, const char *tag, void *object, UINT8 replace_if_duplicate, UINT8 unique_hash)
{
	UINT32 fullhash = tagmap_hash(tag);
	UINT32 hashindex = fullhash % ARRAY_LENGTH(map->table);
	tagmap_entry *entry;

	/* first make sure we don't have a duplicate */
	for (entry = map->table[hashindex]; entry != NULL; entry = entry->next)
		if (entry->fullhash == fullhash)
			if (unique_hash || strcmp(tag, entry->tag) == 0)
			{
				if (replace_if_duplicate)
					entry->object = object;
				return TMERR_DUPLICATE;
			}

	/* now allocate a new entry */
	entry = malloc(sizeof(*entry) + strlen(tag));
	if (entry == NULL)
		return TMERR_OUT_OF_MEMORY;

	/* fill in the entry */
	entry->object = object;
	entry->fullhash = fullhash;
	strcpy(entry->tag, tag);

	/* add it to the head of the list */
	entry->next = map->table[hashindex];
	map->table[hashindex] = entry;
	return TMERR_NONE;
}
