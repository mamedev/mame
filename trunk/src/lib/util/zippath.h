/***************************************************************************

    zippath.h

    File/directory/path operations that work with ZIP files

***************************************************************************/

#pragma once

#ifndef __ZIPPATH_H__
#define __ZIPPATH_H__

#include "corefile.h"
#include "astring.h"
#include "unzip.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class zippath_directory;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- path operations ----- */

/* retrieves the parent directory */
astring &zippath_parent(astring &dst, const char *path);

/* retrieves the parent directory basename */
astring &zippath_parent_basename(astring &dst, const char *path);

/* combines two paths */
astring &zippath_combine(astring &dst, const char *path1, const char *path2);


/* ----- file operations ----- */

/* opens a zip path file */
file_error zippath_fopen(const char *filename, UINT32 openflags, core_file *&file, astring &revised_path);


/* ----- directory operations ----- */

/* opens a directory */
file_error zippath_opendir(const char *path, zippath_directory **directory);

/* closes a directory */
void zippath_closedir(zippath_directory *directory);

/* reads a directory entry */
const osd_directory_entry *zippath_readdir(zippath_directory *directory);

/* returns TRUE if this path is a ZIP path or FALSE if not */
int zippath_is_zip(zippath_directory *directory);



#endif /* __ZIPPATH_H__ */
