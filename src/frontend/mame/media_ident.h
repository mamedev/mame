// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    media_ident.h

    Media identify.

***************************************************************************/
#ifndef MAME_FRONTEND_MEDIA_IDENT_H
#define MAME_FRONTEND_MEDIA_IDENT_H

// media_identifier class identifies media by hash via a search in
// the driver database
class media_identifier
{
public:
	// construction/destruction
	media_identifier(emu_options &options);

	// getters
	int total() const { return m_total; }
	int matches() const { return m_matches; }
	int nonroms() const { return m_nonroms; }

	// operations
	void reset() { m_total = m_matches = m_nonroms = 0; }
	void identify(const char *name);
	void identify_file(const char *name);
	void identify_data(const char *name, const UINT8 *data, int length);
	int find_by_hash(const util::hash_collection &hashes, int length);

private:
	// internal state
	driver_enumerator   m_drivlist;
	int                 m_total;
	int                 m_matches;
	int                 m_nonroms;
};


#endif  /* MAME_FRONTEND_MEDIA_IDENT_H */
