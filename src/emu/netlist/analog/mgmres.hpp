
#ifndef __MGMRES
#define __MGMRES

class gmres_t
{
public:
	gmres_t(const int n);
	~gmres_t();

	int pmgmres_ilu_cr (const int nz_num, int ia[], int ja[], double a[],
	  double x[], const double rhs[], const int itr_max, const int mr, const double tol_abs,
	  const double tol_rel );

private:

	void diagonal_pointer_cr(const int nz_num, const int ia[], const int ja[]);
	void rearrange_cr (const int nz_num, int ia[], int ja[], double a[] );
	inline double r8vec_dot (const double a1[], const double a2[] );
	inline double r8vec_dot2 (const double a1[]);

	void ax_cr(const int * RESTRICT ia, const int * RESTRICT ja, const double * RESTRICT a,
			const double * RESTRICT x, double * RESTRICT w);

	void lus_cr (const int * RESTRICT ia, const int * RESTRICT ja, double * RESTRICT r);

	void ilu_cr (const int nz_num, const int ia[], const int ja[], const double a[]);

	const int m_n;


	double * RESTRICT m_c;
	double * RESTRICT m_g;
	double ** RESTRICT m_h;
	double * RESTRICT m_l;
	double * RESTRICT m_r;
	double * RESTRICT m_s;
	double ** RESTRICT m_v;
	int * RESTRICT m_ua;
	double * RESTRICT m_y;
};

#endif
