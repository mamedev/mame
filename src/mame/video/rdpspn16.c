#if defined(MAGICDITHER)
	#if defined(SHADE)
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#else
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#endif
#elif defined(BAYERDITHER)
	#if defined(SHADE)
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_f_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_f_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_f_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_f_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#else
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_f_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_f_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_f_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_f_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#endif
#else
	#if defined(SHADE)
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_z_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_z_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_t_nz_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_t_nz_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_z_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_z_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_s_nt_nz_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_s_nt_nz_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#else
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_z_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_z_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_t_nz_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_t_nz_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_z_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_z_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
							static void render_spans_16_c1_ns_nt_nz_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
							static void render_spans_16_c1_ns_nt_nz_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#endif
#endif
{
	UINT16 *fb = (UINT16*)&rdram[fb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
#if defined(ZUPDATE) || defined(ZCOMPARE)
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];
#endif

	int i, j;

	int clipx1, clipx2, clipy1, clipy2;

#if defined(SHADE)
	SPAN_PARAM dr = span[0].dr;
	SPAN_PARAM dg = span[0].dg;
	SPAN_PARAM db = span[0].db;
	SPAN_PARAM da = span[0].da;
#if defined(FLIP)
	int drinc = dr.w;
	int dginc = dg.w;
	int dbinc = db.w;
	int dainc = da.w;
#else
	int drinc = -dr.w;
	int dginc = -dg.w;
	int dbinc = -db.w;
	int dainc = -da.w;
#endif
#endif

#if defined(TEXTURE)
	UINT32 prim_tile = tex_tile->num;
	UINT32 disable_lod = 0;
	int tilenum;

	SPAN_PARAM ds = span[0].ds;
	SPAN_PARAM dt = span[0].dt;
	SPAN_PARAM dw = span[0].dw;
#if defined(FLIP)
	int dsinc = ds.w;
	int dtinc = dt.w;
	int dwinc = dw.w;
#else
	int dsinc = -ds.w;
	int dtinc = -dt.w;
	int dwinc = -dw.w;
#endif
#endif

#if defined(ZBUF)
	SPAN_PARAM dz = span[0].dz;
	int dzpix = span[0].dzpix;
#if defined(FLIP)
	int dzinc = dz.w;
#else
	int dzinc = -dz.w;
#endif
#else
	int sz = 0, dzpix = 0;

	if (other_modes.z_source_sel)
	{
		sz = (((UINT32)primitive_z) << 3) & 0x3ffff;
		dzpix = primitive_delta_z;
	}
#endif

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

#if defined(TEXTURE)
	calculate_clamp_diffs(tex_tile->num);

	if (!other_modes.tex_lod_en)
	{
		tilenum = prim_tile;
		tex_tile = &tile[tilenum];
	}
	else
	{
		disable_lod = 1; // Used by World Driver Championship
	}
#endif

	if (start < clipy1)
	{
		start = clipy1;
	}
	if (start >= clipy2)
	{
		start = clipy2 - 1;
	}
	if (end < clipy1)
	{
		end = clipy1;
	}
	if (end >= clipy2) // Needed by 40 Winks
	{
		end = clipy2 - 1;
	}

#if !defined(SHADE)
	shade_color.c = prim_color.c;
#endif

	CACHE_TEXTURE_PARAMS(tex_tile);

	for (i = start; i <= end; i++)
	{
		int xstart = span[i].lx;
		int xend = span[i].rx;

		int x;

		int fb_index = fb_width * i;
		int length;

#if defined(SHADE)
		SPAN_PARAM r = span[i].r;
		SPAN_PARAM g = span[i].g;
		SPAN_PARAM b = span[i].b;
		SPAN_PARAM a = span[i].a;
#endif
#if defined(TEXTURE)
		SPAN_PARAM s = span[i].s;
		SPAN_PARAM t = span[i].t;
		SPAN_PARAM w = span[i].w;
#endif
#if defined(ZBUF)
		SPAN_PARAM z = span[i].z;
#endif

		x = xend;

#if defined(FLIP)
		length = (xstart - xend);
#else
		length = (xend - xstart);
#endif

		for (j = 0; j <= length; j++)
		{
#if defined(SHADE)
			int sr = r.h.h;
			int sg = g.h.h;
			int sb = b.h.h;
			int sa = a.h.h;
#endif
#if defined(TEXTURE)
			int ss = s.h.h;
			int st = t.h.h;
			int sw = w.h.h;
			int sss = 0, sst = 0;
#endif
#if defined(ZBUF)
			int sz = z.w >> 13;
#endif
			COLOR c1;
			//c1.c = 0;

#if defined(ZBUF)
			if (other_modes.z_source_sel)
			{
				sz = (((UINT32)primitive_z) << 3) & 0x3ffff;
				dzpix = primitive_delta_z;
			}
#endif

			if (x >= clipx1 && x < clipx2)
			{
#if defined(ZCOMPARE)
				int z_compare_result = 1;
#endif

				curpixel_cvg=span[i].cvg[x];

				if (curpixel_cvg)
				{
					int curpixel = fb_index + x;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
#if defined(ZUPDATE) || defined(ZCOMPARE)
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
#endif

#if defined(TEXTURE)
					if (other_modes.persp_tex_en)
					{
						tcdiv(ss, st, sw, &sss, &sst);
					}
					else // Hack for Bust-a-Move 2
					{
						sss = ss;
						sst = st;
					}
#endif

#if defined(SHADE)
					if (sr > 0xff) shade_color.i.r = 0xff;
					else if (sr < 0) shade_color.i.r = 0;
					else shade_color.i.r = sr;
					if (sg > 0xff) shade_color.i.g = 0xff;
					else if (sg < 0) shade_color.i.g = 0;
					else shade_color.i.g = sg;
					if (sb > 0xff) shade_color.i.b = 0xff;
					else if (sb < 0) shade_color.i.b = 0;
					else shade_color.i.b = sb;
					if (sa > 0xff) shade_color.i.a = 0xff;
					else if (sa < 0) shade_color.i.a = 0;
					else shade_color.i.a = sa;
#endif

#if defined(TEXTURE)
					texel0_color.c = TEXTURE_PIPELINE(sss, sst, tex_tile);
#endif

					c1.c = COLOR_COMBINER1();

#if defined(ZCOMPARE)
#if !defined(ZBUF)
					if (other_modes.z_source_sel)
#endif
					{
						z_compare_result = z_compare(fbcur, hbcur, zbcur, zhbcur, sz, dzpix);
					}

					if(z_compare_result)
#endif
					{
#if defined(ZUPDATE)
						int rendered = 0;
#endif
#if defined(MAGICDITHER)
						int dith = magic_matrix[(((i) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
#elif defined(BAYERDITHER)
						int dith = bayer_matrix[(((i) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
#endif

#if defined(ZUPDATE)
#if defined(MAGICDITHER) || defined(BAYERDITHER)
						rendered = BLENDER1_16_DITH(fbcur, hbcur, c1, dith);
#else
						rendered = BLENDER1_16_NDITH(fbcur, hbcur, c1);
#endif

						if (rendered)
						{
							z_store(zbcur, zhbcur, sz, dzpix);
						}
#else
#if defined(MAGICDITHER) || defined(BAYERDITHER)
						BLENDER1_16_DITH(fbcur, hbcur, c1, dith);
#else
						BLENDER1_16_NDITH(fbcur, hbcur, c1);
#endif
#endif
					}
				}
			}

#if defined(SHADE)
			r.w += drinc;
			g.w += dginc;
			b.w += dbinc;
			a.w += dainc;
#endif
#if defined(TEXTURE)
			s.w += dsinc;
			t.w += dtinc;
			w.w += dwinc;
#endif
#if defined(ZBUF)
			z.w += dzinc;
#endif

#if defined(FLIP)
			x++;
#else
			x--;
#endif
		}
	}
}

#if defined(MAGICDITHER)
	#if defined(SHADE)
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#else
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_f_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_f_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_f_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_f_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_nf_zc_zu_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_nf_nzc_zu_dm(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_nf_zc_dm(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_nf_nzc_dm(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#endif
#elif defined(BAYERDITHER)
	#if defined(SHADE)
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_f_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_f_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_f_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_f_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#else
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_f_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_f_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_f_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_f_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_f_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_f_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_f_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_nf_zc_zu_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_nf_nzc_zu_db(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_nf_zc_db(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_nf_nzc_db(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#endif
#else
	#if defined(SHADE)
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_z_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_z_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_t_nz_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_t_nz_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_z_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_z_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_s_nt_nz_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_s_nt_nz_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#else
		#if defined(TEXTURE)
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_z_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_z_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_t_nz_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_t_nz_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#else
			#if defined(ZBUF)
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_z_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_z_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#else
				#if defined(FLIP)
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_f_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_f_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_f_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_f_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#else
					#if defined(ZUPDATE)
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_nf_zc_zu_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_nf_nzc_zu_dn(int start, int end, TILE* tex_tile)
						#endif
					#else
						#if defined(ZCOMPARE)
						static void render_spans_16_c2_ns_nt_nz_nf_zc_dn(int start, int end, TILE* tex_tile)
						#else
						static void render_spans_16_c2_ns_nt_nz_nf_nzc_dn(int start, int end, TILE* tex_tile)
						#endif
					#endif
				#endif
			#endif
		#endif
	#endif
#endif
{
	UINT16 *fb = (UINT16*)&rdram[fb_address / 4];
	UINT8 *hb = &hidden_bits[fb_address >> 1];
#if defined(ZUPDATE) || defined(ZCOMPARE)
	UINT16 *zb = (UINT16*)&rdram[zb_address / 4];
	UINT8 *zhb = &hidden_bits[zb_address >> 1];
#endif

	int i, j;

	int clipx1, clipx2, clipy1, clipy2;

#if defined(SHADE)
	SPAN_PARAM dr = span[0].dr;
	SPAN_PARAM dg = span[0].dg;
	SPAN_PARAM db = span[0].db;
	SPAN_PARAM da = span[0].da;
#if defined(FLIP)
	int drinc = dr.w;
	int dginc = dg.w;
	int dbinc = db.w;
	int dainc = da.w;
#else
	int drinc = -dr.w;
	int dginc = -dg.w;
	int dbinc = -db.w;
	int dainc = -da.w;
#endif
#endif

#if defined(TEXTURE)
	UINT32 prim_tile = tex_tile->num;
	UINT32 tilenum2 = 0;
	TILE *tex_tile2 = NULL;
	int tilenum;

	int LOD = 0;
	INT32 horstep, vertstep;
	INT32 l_tile;
	UINT32 magnify = 0;
	UINT32 distant = 0;

	int nexts, nextt, nextsw;
	int lodclamp = 0;

	SPAN_PARAM ds = span[0].ds;
	SPAN_PARAM dt = span[0].dt;
	SPAN_PARAM dw = span[0].dw;
#if defined(FLIP)
	int dsinc = ds.w;
	int dtinc = dt.w;
	int dwinc = dw.w;
#else
	int dsinc = -ds.w;
	int dtinc = -dt.w;
	int dwinc = -dw.w;
#endif
#endif

#if defined(ZBUF)
	SPAN_PARAM dz = span[0].dz;
	int dzpix = span[0].dzpix;
#if defined(FLIP)
	int dzinc = dz.w;
#else
	int dzinc = -dz.w;
#endif
#else
	int sz = 0, dzpix = 0;

	if (other_modes.z_source_sel)
	{
		sz = (((UINT32)primitive_z) << 3) & 0x3ffff;
		dzpix = primitive_delta_z;
	}
#endif

#if defined(TEXTURE)
	calculate_clamp_diffs(tex_tile->num);

	if (!other_modes.tex_lod_en)
	{
		tilenum = prim_tile;
		tex_tile = &tile[tilenum];
		tilenum2 = (prim_tile + 1) & 7;
		tex_tile2 = &tile[tilenum2];
	}
#endif

	clipx1 = clip.xh / 4;
	clipx2 = clip.xl / 4;
	clipy1 = clip.yh / 4;
	clipy2 = clip.yl / 4;

	if (start < clipy1)
	{
		start = clipy1;
	}
	if (start >= clipy2)
	{
		start = clipy2 - 1;
	}
	if (end < clipy1)
	{
		end = clipy1;
	}
	if (end >= clipy2) // Needed by 40 Winks
	{
		end = clipy2 - 1;
	}

#if !defined(SHADE)
	shade_color.c = prim_color.c;
#endif

	for (i = start; i <= end; i++)
	{
		int xstart = span[i].lx;
		int xend = span[i].rx;
#if defined(SHADE)
		SPAN_PARAM r = span[i].r;
		SPAN_PARAM g = span[i].g;
		SPAN_PARAM b = span[i].b;
		SPAN_PARAM a = span[i].a;
#endif
#if defined(TEXTURE)
		SPAN_PARAM s = span[i].s;
		SPAN_PARAM t = span[i].t;
		SPAN_PARAM w = span[i].w;
#endif
#if defined(ZBUF)
		SPAN_PARAM z = span[i].z;
#endif

		int x;

		int fb_index = fb_width * i;
		int length;

		x = xend;

#if defined(FLIP)
		length = (xstart - xend);
#else
		length = (xend - xstart);
#endif

		for (j = 0; j <= length; j++)
		{
#if defined(SHADE)
			int sr = r.h.h;
			int sg = g.h.h;
			int sb = b.h.h;
			int sa = a.h.h;
#endif
#if defined(TEXTURE)
			int ss = s.h.h;
			int st = t.h.h;
			int sw = w.h.h;
			int sss = 0, sst = 0;
#endif
#if defined(ZBUF)
			int sz = z.w >> 13;
#endif
			COLOR c1, c2;
			//c1.c = 0;
			//c2.c = 0;

#if defined(ZBUF)
			if (other_modes.z_source_sel)
			{
				sz = (((UINT32)primitive_z) << 3) & 0x3ffff;
				dzpix = primitive_delta_z;
			}
#endif

			if (x >= clipx1 && x < clipx2)
			{
#if defined(ZCOMPARE)
				int z_compare_result = 1;
#endif

				curpixel_cvg=span[i].cvg[x];

				if (curpixel_cvg)
				{
					int curpixel = fb_index + x;
					UINT16* fbcur = &fb[curpixel ^ WORD_ADDR_XOR];
					UINT8* hbcur = &hb[curpixel ^ BYTE_ADDR_XOR];
#if defined(ZUPDATE) || defined(ZCOMPARE)
					UINT16* zbcur = &zb[curpixel ^ WORD_ADDR_XOR];
					UINT8* zhbcur = &zhb[curpixel ^ BYTE_ADDR_XOR];
#endif

#if defined(SHADE)
					if (sr > 0xff) sr = 0xff;
					if (sg > 0xff) sg = 0xff;
					if (sb > 0xff) sb = 0xff;
					if (sa > 0xff) sa = 0xff;
					if (sr < 0) sr = 0;
					if (sg < 0) sg = 0;
					if (sb < 0) sb = 0;
					if (sa < 0) sa = 0;
					shade_color.i.r = sr;
					shade_color.i.g = sg;
					shade_color.i.b = sb;
					shade_color.i.a = sa;
#endif

#if defined(TEXTURE)
					if (other_modes.persp_tex_en)
					{
						tcdiv(ss, st, sw, &sss, &sst);
					}
					else // Hack for Bust-a-Move 2
					{
						sss = ss;
						sst = st;
					}

					if (other_modes.tex_lod_en)
					{
						if (other_modes.persp_tex_en)
						{
							nextsw = (w.w + dwinc) >> 16;
							nexts = (s.w + dsinc) >> 16;
							nextt = (t.w + dtinc) >> 16;
							tcdiv(nexts, nextt, nextsw, &nexts, &nextt);
						}
						else
						{
							nexts = (s.w + dsinc)>>16;
							nextt = (t.w + dtinc)>>16;
						}

						lodclamp = 0;

						horstep = SIGN17(nexts & 0x1ffff) - SIGN17(sss & 0x1ffff);
						vertstep = SIGN17(nextt & 0x1ffff) - SIGN17(sst & 0x1ffff);
						if (horstep & 0x20000)
						{
							horstep = ~horstep & 0x1ffff;
						}
						if (vertstep & 0x20000)
						{
							vertstep = ~vertstep & 0x1ffff;
						}
						LOD = ((horstep >= vertstep) ? horstep : vertstep);
						LOD = (LOD >= span[0].dymax) ? LOD : span[0].dymax;

						if ((LOD & 0x1c000) || lodclamp)
						{
							LOD = 0x7fff;
						}
						if (LOD < min_level)
						{
							LOD = min_level;
						}

						magnify = (LOD < 32) ? 1: 0;
						l_tile = getlog2((LOD >> 5) & 0xff);
						distant = ((LOD & 0x6000) || (l_tile >= max_level)) ? 1 : 0;

						lod_frac = ((LOD << 3) >> l_tile) & 0xff;

						if (distant)
						{
							l_tile = max_level;
						}
						if(!other_modes.sharpen_tex_en && !other_modes.detail_tex_en && magnify)
						{
							lod_frac = 0;
						}
						if(!other_modes.sharpen_tex_en && !other_modes.detail_tex_en && distant)
						{
							lod_frac = 0xff;
						}
						if(other_modes.sharpen_tex_en && magnify)
						{
							lod_frac |= 0x100;
						}

						if (!other_modes.detail_tex_en)
						{
							tilenum = (prim_tile + l_tile);
							tilenum &= 7;
							if (other_modes.sharpen_tex_en)
							{
								tilenum2 = (tilenum + 1) & 7;
							}
							else if (!distant)
							{
								tilenum2 = (tilenum + 1) & 7;
							}
							else
							{
								tilenum2 = tilenum;
							}
							tex_tile = &tile[tilenum];
							tex_tile2 = &tile[tilenum2];
						}
						else
						{
							if (!magnify)
							{
								tilenum = (prim_tile + l_tile + 1);
							}
							else
							{
								tilenum = (prim_tile + l_tile);
							}
							tilenum &= 7;

							if (!distant && !magnify)
							{
								tilenum2 = (prim_tile + l_tile + 2) & 7;
							}
							else
							{
								tilenum2 = (prim_tile + l_tile + 1) & 7;
							}
							tex_tile = &tile[tilenum];
							tex_tile2 = &tile[tilenum2];
						}
					}

					CACHE_TEXTURE_PARAMS(tex_tile);
					texel0_color.c = TEXTURE_PIPELINE(sss, sst, tex_tile);

					CACHE_TEXTURE_PARAMS(tex_tile2);
					texel1_color.c = TEXTURE_PIPELINE(sss, sst, tex_tile2);
#endif

					c1.c = COLOR_COMBINER2_C0();
					c2.c = COLOR_COMBINER2_C1();

#if defined(ZCOMPARE)
#if !defined(ZBUF)
					if (other_modes.z_source_sel)
#endif
					{
						z_compare_result = z_compare(fbcur, hbcur, zbcur, zhbcur, sz, dzpix);
					}

					if(z_compare_result)
#endif
					{
#if defined(ZUPDATE)
						int rendered = 0;
#endif
#if defined(MAGICDITHER)
						int dith = magic_matrix[(((i) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
#elif defined(BAYERDITHER)
						int dith = bayer_matrix[(((i) & 3) << 2) + ((x ^ WORD_ADDR_XOR) & 3)];
#endif

#if defined(ZUPDATE)
#if defined(MAGICDITHER) || defined(BAYERDITHER)
						rendered = BLENDER2_16_DITH(fbcur, hbcur, c1, c2, dith);
#else
						rendered = BLENDER2_16_NDITH(fbcur, hbcur, c1, c2);
#endif

						if (rendered)
						{
							z_store(zbcur, zhbcur, sz, dzpix);
						}
#else
#if defined(MAGICDITHER) || defined(BAYERDITHER)
						BLENDER2_16_DITH(fbcur, hbcur, c1, c2, dith);
#else
						BLENDER2_16_NDITH(fbcur, hbcur, c1, c2);
#endif

#endif
					}
				}
			}

#if defined(SHADE)
			r.w += drinc;
			g.w += dginc;
			b.w += dbinc;
			a.w += dainc;
#endif
#if defined(TEXTURE)
			s.w += dsinc;
			t.w += dtinc;
			w.w += dwinc;
#endif
#if defined(ZBUF)
			z.w += dzinc;
#endif

#if defined(FLIP)
			x++;
#else
			x--;
#endif
		}
	}
}
