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
	unsigned total() const { return m_total; }
	unsigned matches() const { return m_matches; }
	unsigned nonroms() const { return m_nonroms; }

	// operations
	void reset() { m_total = m_matches = m_nonroms = 0; }
	void identify(const char *name);
	void identify_file(const char *name);
	void identify_data(const char *name, const uint8_t *data, int length);
	int find_by_hash(const util::hash_collection &hashes, int length);

private:

	// internal state
	driver_enumerator   m_drivlist;
	unsigned            m_total;
	unsigned            m_matches;
	unsigned            m_nonroms;
};


#endif  /* MAME_FRONTEND_MEDIA_IDENT_H */
