// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    zippath.h

    File/directory/path operations that work with ZIP files

***************************************************************************/

#pragma once

#ifndef __ZIPPATH_H__
#define __ZIPPATH_H__

#include "corefile.h"
#include <string>
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
std::string &zippath_parent(std::string &dst, const char *path);

/* retrieves the parent directory basename */
std::string &zippath_parent_basename(std::string &dst, const char *path);

/* combines two paths */
std::string &zippath_combine(std::string &dst, const char *path1, const char *path2);


/* ----- file operations ----- */

/* opens a zip path file */
file_error zippath_fopen(const char *filename, UINT32 openflags, core_file *&file, std::string &revised_path);


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
