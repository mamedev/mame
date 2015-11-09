// license:BSD-3-Clause
// copyright-holders:Aaron Giles
//============================================================
//
//  minidir.c - Minimal core directory access functions
//
//============================================================

#include "osdcore.h"


//============================================================
//  osd_opendir
//============================================================

osd_directory *osd_opendir(const char *dirname)
{
	// since there are no standard C library routines for walking directories,
	// we always return an error here
	return NULL;
}


//============================================================
//  osd_readdir
//============================================================

const osd_directory_entry *osd_readdir(osd_directory *dir)
{
	// since there are no standard C library routines for walking directories,
	// we always return an error here
	return NULL;
}


//============================================================
//  osd_closedir
//============================================================

void osd_closedir(osd_directory *dir)
{
	// since there are no standard C library routines for walking directories,
	// we do nothing
}


//============================================================
//  osd_is_absolute_path
//============================================================

int osd_is_absolute_path(const char *path)
{
	// assume no for everything
	return FALSE;
}
