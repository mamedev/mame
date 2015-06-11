# include <cstdlib>
# include <iostream>
# include <fstream>
# include <iomanip>
# include <cmath>
# include <ctime>

using namespace std;

#include "mgmres.hpp"

//****************************************************************************80
// http://people.sc.fsu.edu/~jburkardt/cpp_src/mgmres/mgmres.html
//****************************************************************************80

void gmres_t::ax_cr(const int * RESTRICT ia, const int * RESTRICT ja, const double * RESTRICT a,
		const double * RESTRICT x, double * RESTRICT w)

//****************************************************************************80
//
//  Purpose:
//
//    AX_CR computes A*x for a matrix stored in sparse compressed row form.
//
//  Discussion:
//
//    The Sparse Compressed Row storage format is used.
//
//    The matrix A is assumed to be sparse.  To save on storage, only
//    the nonzero entries of A are stored.  The vector JA stores the
//    column index of the nonzero value.  The nonzero values are sorted
//    by row, and the compressed row vector IA then has the property that
//    the entries in A and JA that correspond to row I occur in indices
//    IA[I] through IA[I+1]-1.
//
//    For this version of MGMRES, the row and column indices are assumed
//    to use the C/C++ convention, in which indexing begins at 0.
//
//    If your index vectors IA and JA are set up so that indexing is based 
//    at 1, then each use of those vectors should be shifted down by 1.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    18 July 2007
//
//  Author:
//
//    Original C version by Lili Ju.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Richard Barrett, Michael Berry, Tony Chan, James Demmel,
//    June Donato, Jack Dongarra, Victor Eijkhout, Roidan Pozo,
//    Charles Romine, Henk van der Vorst,
//    Templates for the Solution of Linear Systems:
//    Building Blocks for Iterative Methods,
//    SIAM, 1994,
//    ISBN: 0898714710,
//    LC: QA297.8.T45.
//
//    Tim Kelley,
//    Iterative Methods for Linear and Nonlinear Equations,
//    SIAM, 2004,
//    ISBN: 0898713528,
//    LC: QA297.8.K45.
//
//    Yousef Saad,
//    Iterative Methods for Sparse Linear Systems,
//    Second Edition,
//    SIAM, 2003,
//    ISBN: 0898715342,
//    LC: QA188.S17.
//
//  Parameters:
//
//    Input, int N, the order of the system.
//
//    Input, int NZ_NUM, the number of nonzeros.
//
//    Input, int IA[N+1], JA[NZ_NUM], the row and column indices
//    of the matrix values.  The row vector has been compressed.
//
//    Input, double A[NZ_NUM], the matrix values.
//
//    Input, double X[N], the vector to be multiplied by A.
//
//    Output, double W[N], the value of A*X.
//
{
	const int n = m_n;
	for ( int i = 0; i < n; i++ )
	{
		double tmp = 0.0;
		int k1 = ia[i];
		int k2 = ia[i+1];

		for (int k = k1; k < k2; k++ )
		{
			tmp += a[k] * x[ja[k]];
		}
		w[i] = tmp;
	}
	return;
}

//****************************************************************************80

void gmres_t::diagonal_pointer_cr (const int nz_num, const int ia[], const int ja[])

//****************************************************************************80
//
//  Purpose:
//
//    DIAGONAL_POINTER_CR finds diagonal entries in a sparse compressed row matrix.
//
//  Discussion:
//
//    The matrix A is assumed to be stored in compressed row format.  Only
//    the nonzero entries of A are stored.  The vector JA stores the
//    column index of the nonzero value.  The nonzero values are sorted
//    by row, and the compressed row vector IA then has the property that
//    the entries in A and JA that correspond to row I occur in indices
//    IA[I] through IA[I+1]-1.
//
//    The array UA can be used to locate the diagonal elements of the matrix.
//
//    It is assumed that every row of the matrix includes a diagonal element,
//    and that the elements of each row have been ascending sorted.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    18 July 2007
//
//  Author:
//
//    Original C version by Lili Ju.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Input, int N, the order of the system.
//
//    Input, int NZ_NUM, the number of nonzeros.
//
//    Input, int IA[N+1], JA[NZ_NUM], the row and column indices
//    of the matrix values.  The row vector has been compressed.  On output,
//    the order of the entries of JA may have changed because of the sorting.
//
//    Output, int UA[N], the index of the diagonal element of each row.
//
{
	const int n = m_n;

	for (int i = 0; i < n; i++)
	{
		m_ua[i] = -1;
		const int j1 = ia[i];
		const int j2 = ia[i + 1];

		for (int j = j1; j < j2; j++)
		{
			if (ja[j] == i)
			{
				m_ua[i] = j;
			}
		}

	}
  return;
}
//****************************************************************************80

void gmres_t::ilu_cr (const int nz_num, const int ia[], const int ja[], const double a[])

//****************************************************************************80
//
//  Purpose:
//
//    ILU_CR computes the incomplete LU factorization of a matrix.
//
//  Discussion:
//
//    The matrix A is assumed to be stored in compressed row format.  Only
//    the nonzero entries of A are stored.  The vector JA stores the
//    column index of the nonzero value.  The nonzero values are sorted
//    by row, and the compressed row vector IA then has the property that
//    the entries in A and JA that correspond to row I occur in indices
//    IA[I] through IA[I+1]-1.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    25 July 2007
//
//  Author:
//
//    Original C version by Lili Ju.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Input, int N, the order of the system.
//
//    Input, int NZ_NUM, the number of nonzeros.
//
//    Input, int IA[N+1], JA[NZ_NUM], the row and column indices
//    of the matrix values.  The row vector has been compressed.
//
//    Input, double A[NZ_NUM], the matrix values.
//
//    Input, int UA[N], the index of the diagonal element of each row.
//
//    Output, double L[NZ_NUM], the ILU factorization of A.
//
{
	int *iw;
	int i;
	int j;
	int jj;
	int jrow;
	int jw;
	int k;
	double tl;

	const int n = m_n;

	iw = new int[n];
//
//  Copy A.
//
	for (k = 0; k < nz_num; k++)
	{
		m_l[k] = a[k];
	}

	for (i = 0; i < n; i++)
	{
//
//  IW points to the nonzero entries in row I.
//
		for (j = 0; j < n; j++)
		{
			iw[j] = -1;
		}

		for (k = ia[i]; k <= ia[i + 1] - 1; k++)
		{
			iw[ja[k]] = k;
		}

		j = ia[i];
		do
		{
			jrow = ja[j];
			if (i <= jrow)
			{
				break;
			}
			tl = m_l[j] * m_l[m_ua[jrow]];
			m_l[j] = tl;
			for (jj = m_ua[jrow] + 1; jj <= ia[jrow + 1] - 1; jj++)
			{
				jw = iw[ja[jj]];
				if (jw != -1)
				{
					m_l[jw] = m_l[jw] - tl * m_l[jj];
				}
			}
			j = j + 1;
		} while (j <= ia[i + 1] - 1);

		m_ua[i] = j;

		if (jrow != i)
		{
			cout << "\n";
			cout << "ILU_CR - Fatal error!\n";
			cout << "  JROW != I\n";
			cout << "  JROW = " << jrow << "\n";
			cout << "  I    = " << i << "\n";
			exit(1);
		}

		if (m_l[j] == 0.0)
		{
			cout << "\n";
			cout << "ILU_CR - Fatal error!\n";
			cout << "  Zero pivot on step I = " << i << "\n";
			cout << "  L[" << j << "] = 0.0\n";
			exit(1);
		}

		m_l[j] = 1.0 / m_l[j];
	}

	for (k = 0; k < n; k++)
	{
		m_l[m_ua[k]] = 1.0 / m_l[m_ua[k]];
	}

	delete[] iw;
}
//****************************************************************************80

void gmres_t::lus_cr (const int * RESTRICT ia, const int * RESTRICT ja, double * RESTRICT r)

//****************************************************************************80
//
//  Purpose:
//
//    LUS_CR applies the incomplete LU preconditioner.
//
//  Discussion:
//
//    The linear system M * Z = R is solved for Z.  M is the incomplete
//    LU preconditioner matrix, and R is a vector supplied by the user.
//    So essentially, we're solving L * U * Z = R.
//
//    The matrix A is assumed to be stored in compressed row format.  Only
//    the nonzero entries of A are stored.  The vector JA stores the
//    column index of the nonzero value.  The nonzero values are sorted
//    by row, and the compressed row vector IA then has the property that
//    the entries in A and JA that correspond to row I occur in indices
//    IA[I] through IA[I+1]-1.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    18 July 2007
//
//  Author:
//
//    Original C version by Lili Ju.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Input, int N, the order of the system.
//
//    Input, int NZ_NUM, the number of nonzeros.
//
//    Input, int IA[N+1], JA[NZ_NUM], the row and column indices
//    of the matrix values.  The row vector has been compressed.
//
//    Input, double L[NZ_NUM], the matrix values.
//
//    Input, int UA[N], the index of the diagonal element of each row.
//
//    Input, double R[N], the right hand side.
//
//    Output, double Z[N], the solution of the system M * Z = R.
//
{
	const int n = m_n;
//
//  Solve L * w = w where L is unit lower triangular.
//
	for (int i = 1; i < n; i++ )
	{
		double tmp = 0.0;
		for (int j = ia[i]; j < m_ua[i]; j++ )
		{
			tmp +=  m_l[j] * r[ja[j]];
		}
		r[i] -= tmp;
	}
//
//  Solve U * w = w, where U is upper triangular.
//
	for (int i = n - 1; 0 <= i; i-- )
	{
		double tmp = 0.0;
		for (int j = m_ua[i] + 1; j < ia[i+1]; j++ )
		{
			tmp += m_l[j] * r[ja[j]];
		}
		r[i] = (r[i] - tmp) / m_l[m_ua[i]];
	}

	return;
}
//****************************************************************************80

static inline void mult_givens ( const double c, const double s, const int k, double g[] )

//****************************************************************************80
//
//  Purpose:
//
//    MULT_GIVENS applies a Givens rotation to two successive entries of a vector.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    08 August 2006
//
//  Author:
//
//    Original C version by Lili Ju.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Richard Barrett, Michael Berry, Tony Chan, James Demmel,
//    June Donato, Jack Dongarra, Victor Eijkhout, Roidan Pozo,
//    Charles Romine, Henk van der Vorst,
//    Templates for the Solution of Linear Systems:
//    Building Blocks for Iterative Methods,
//    SIAM, 1994,
//    ISBN: 0898714710,
//    LC: QA297.8.T45.
//
//    Tim Kelley,
//    Iterative Methods for Linear and Nonlinear Equations,
//    SIAM, 2004,
//    ISBN: 0898713528,
//    LC: QA297.8.K45.
//
//    Yousef Saad,
//    Iterative Methods for Sparse Linear Systems,
//    Second Edition,
//    SIAM, 2003,
//    ISBN: 0898715342,
//    LC: QA188.S17.
//
//  Parameters:
//
//    Input, double C, S, the cosine and sine of a Givens
//    rotation.
//
//    Input, int K, indicates the location of the first vector entry.
//
//    Input/output, double G[K+2], the vector to be modified.  On output,
//    the Givens rotation has been applied to entries G(K) and G(K+1).
//
{

  double g1;
  double g2;

  g1 = c * g[k] - s * g[k+1];
  g2 = s * g[k] + c * g[k+1];

  g[k]   = g1;
  g[k+1] = g2;

  return;
}
//****************************************************************************80


gmres_t::gmres_t(const int n)
: m_n(n)
{
	int mr=n; /* FIXME: maximum iterations locked in here */

	m_c = new double[mr+1];
	m_g = new double[mr+1];
	m_h = new double *[mr];
	for (int i = 0; i < mr; i++)
		m_h[i] = new double[mr+1];
	/* This is using too much memory, but we are interested in speed for now*/
	m_l = new double[n*n]; //[ia[n]+1];
	m_r = new double[n];
	m_s = new double[mr+1];
	m_ua = new int[n];
	m_v = new double *[n];
	for (int i = 0; i < n; i++)
		m_v[i] = new double[mr+1];
	m_y = new double[mr+1];
}

gmres_t::~gmres_t()
{
	  delete [] m_c;
	  delete [] m_g;
	  delete [] m_h;
	  delete [] m_l;
	  delete [] m_r;
	  delete [] m_s;
	  delete [] m_ua;
	  delete [] m_v;
	  delete [] m_y;
}

int gmres_t::pmgmres_ilu_cr (const int nz_num, int ia[], int ja[], double a[],
  double x[], const double rhs[], const int itr_max, const int mr, const double tol_abs,
  const double tol_rel )

//****************************************************************************80
//
//  Purpose:
//
//    PMGMRES_ILU_CR applies the preconditioned restarted GMRES algorithm.
//
//  Discussion:
//
//    The matrix A is assumed to be stored in compressed row format.  Only
//    the nonzero entries of A are stored.  The vector JA stores the
//    column index of the nonzero value.  The nonzero values are sorted
//    by row, and the compressed row vector IA then has the property that
//    the entries in A and JA that correspond to row I occur in indices
//    IA[I] through IA[I+1]-1.
//
//    This routine uses the incomplete LU decomposition for the
//    preconditioning.  This preconditioner requires that the sparse
//    matrix data structure supplies a storage position for each diagonal
//    element of the matrix A, and that each diagonal element of the
//    matrix A is not zero.
//
//    Thanks to Jesus Pueblas Sanchez-Guerra for supplying two
//    corrections to the code on 31 May 2007.
//
//
//    This implementation of the code stores the doubly-dimensioned arrays
//    H and V as vectors.  However, it follows the C convention of storing
//    them by rows, rather than my own preference for storing them by
//    columns.   I may come back and change this some time.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    26 July 2007
//
//  Author:
//
//    Original C version by Lili Ju.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Richard Barrett, Michael Berry, Tony Chan, James Demmel,
//    June Donato, Jack Dongarra, Victor Eijkhout, Roidan Pozo,
//    Charles Romine, Henk van der Vorst,
//    Templates for the Solution of Linear Systems:
//    Building Blocks for Iterative Methods,
//    SIAM, 1994.
//    ISBN: 0898714710,
//    LC: QA297.8.T45.
//
//    Tim Kelley,
//    Iterative Methods for Linear and Nonlinear Equations,
//    SIAM, 2004,
//    ISBN: 0898713528,
//    LC: QA297.8.K45.
//
//    Yousef Saad,
//    Iterative Methods for Sparse Linear Systems,
//    Second Edition,
//    SIAM, 2003,
//    ISBN: 0898715342,
//    LC: QA188.S17.
//
//  Parameters:
//
//    Input, int N, the order of the linear system.
//
//    Input, int NZ_NUM, the number of nonzero matrix values.
//
//    Input, int IA[N+1], JA[NZ_NUM], the row and column indices
//    of the matrix values.  The row vector has been compressed.
//
//    Input, double A[NZ_NUM], the matrix values.
//
//    Input/output, double X[N]; on input, an approximation to
//    the solution.  On output, an improved approximation.
//
//    Input, double RHS[N], the right hand side of the linear system.
//
//    Input, int ITR_MAX, the maximum number of (outer) iterations to take.
//
//    Input, int MR, the maximum number of (inner) iterations to take.
//    MR must be less than N.
//
//    Input, double TOL_ABS, an absolute tolerance applied to the
//    current residual.
//
//    Input, double TOL_REL, a relative tolerance comparing the
//    current residual to the initial residual.
//
{
	double av;
	double delta = 1.0e-03;
	double htmp;
	int itr;
	int itr_used = 0;
	int k_copy = 0;
	double mu;
	double rho;
	double rho_tol = 0;
	const int verbose = 0;
	const bool pre_ilu = true;

	const int n = m_n;

	rearrange_cr(nz_num, ia, ja, a);

	if (pre_ilu)
	{
		diagonal_pointer_cr(nz_num, ia, ja);
		ilu_cr(nz_num, ia, ja, a);
	}

	if (verbose)
	{
		cout << "\n";
		cout << "PMGMRES_ILU_CR\n";
		cout << "  Number of unknowns = " << n << "\n";
	}

	for (itr = 0; itr < itr_max; itr++)
	{
		ax_cr(ia, ja, a, x, m_r);

		for (int i = 0; i < n; i++)
			m_r[i] = rhs[i] - m_r[i];

		if (pre_ilu)
			lus_cr(ia, ja, m_r);

		rho = sqrt(r8vec_dot2(m_r));

		if (verbose)
			cout << "  ITR = " << itr << "  Residual = " << rho << "\n";

		if (itr == 0)
			rho_tol = rho * tol_rel;

		double rhoq = 1.0 / rho;

		for (int i = 0; i < n; i++)
			m_v[0][i] = m_r[i] * rhoq;

		for (int i = 0; i < mr + 1; i++)
			m_g[i] = 0.0;

		m_g[0] = rho;

		for (int i = 0; i < mr; i++)
			for (int j = 0; j < mr + 1; j++)
				m_h[i][j] = 0.0;

		for (int k = 0; k < mr; k++)
		{
			k_copy = k;

			ax_cr(ia, ja, a, m_v[k], m_v[k + 1]);

			if (pre_ilu)
				lus_cr(ia, ja, m_v[k + 1]);

			av = sqrt(r8vec_dot2(m_v[k + 1]));

			for (int j = 0; j <= k; j++)
			{
				m_h[j][k] = r8vec_dot(m_v[k + 1], m_v[j]);
				for (int i = 0; i < n; i++)
				{
					m_v[k + 1][i] = m_v[k + 1][i]
							- m_h[j][k] * m_v[j][i];
				}
			}
			m_h[k + 1][k] = sqrt(r8vec_dot2(m_v[k + 1]));

			if ((av + delta * m_h[k + 1][k]) == av)
			{
				for (int j = 0; j < k + 1; j++)
				{
					htmp = r8vec_dot(m_v[k + 1], m_v[j]);
					m_h[j][k] = m_h[j][k] + htmp;
					for (int i = 0; i < n; i++)
					{
						m_v[k + 1][i] = m_v[k + 1][i] - htmp * m_v[j][i];
					}
				}
				m_h[k + 1][k] = sqrt(r8vec_dot2(m_v[k + 1]));
			}

			if (m_h[k + 1][k] != 0.0)
			{
				for (int i = 0; i < n; i++)
				{
					m_v[k + 1][i] = m_v[k + 1][i]
							/ m_h[k + 1][k];
				}
			}

			if (0 < k)
			{
				for (int i = 0; i < k + 2; i++)
				{
					m_y[i] = m_h[i][k];
				}
				for (int j = 0; j < k; j++)
				{
					mult_givens(m_c[j], m_s[j], j, m_y);
				}
				for (int i = 0; i < k + 2; i++)
				{
					m_h[i][k] = m_y[i];
				}
			}
			mu = sqrt(
					m_h[k][k] * m_h[k][k]
							+ m_h[k + 1][k] * m_h[k + 1][k]);
			m_c[k] = m_h[k][k] / mu;
			m_s[k] = -m_h[k + 1][k] / mu;
			m_h[k][k] = m_c[k] * m_h[k][k]
					- m_s[k] * m_h[k + 1][k];
			m_h[k + 1][k] = 0.0;
			mult_givens(m_c[k], m_s[k], k, m_g);

			rho = std::abs(m_g[k + 1]);

			itr_used = itr_used + 1;

			if (verbose)
			{
				cout << "  K   = " << k << "  Residual = " << rho << "\n";
			}

			if (rho <= rho_tol && rho <= tol_abs)
			{
				break;
			}
		}

		m_y[k_copy] = m_g[k_copy] / m_h[k_copy][k_copy];
		for (int i = k_copy - 1; 0 <= i; i--)
		{
			double tmp = m_g[i];
			for (int j = i + 1; j < k_copy + 1; j++)
			{
				tmp -= m_h[i][j] * m_y[j];
			}
			m_y[i] = tmp / m_h[i][i];
		}
		double cerr = 0;
		for (int i = 0; i < n; i++)
		{
			double tmp = 0.0;
			for (int j = 0; j < k_copy + 1; j++)
			{
				tmp += m_v[j][i] * m_y[j];
			}
			cerr = std::max(std::abs(tmp), cerr);
			x[i] += tmp;
		}
#if 0
		if (cerr < 1e-8)
			return 1; //break;
#else
		if (rho <= rho_tol && rho <= tol_abs)
		{
			break;
		}
#endif
	}

	if (verbose)
	{
		cout << "\n";
		;
		cout << "PMGMRES_ILU_CR:\n";
		cout << "  Iterations = " << itr_used << "\n";
		cout << "  Final residual = " << rho << "\n";
	}

	//if ( rho >= tol_abs )
//	  printf("missed!\n");

	return (itr_used - 1) * mr + k_copy;
}
//****************************************************************************80

double gmres_t::r8vec_dot (const double a1[], const double a2[] )
{
	const int n = m_n;
	double value = 0.0;

	for ( int i = 0; i < n; i++ )
		value = value + a1[i] * a2[i];
	return value;
}

double gmres_t::r8vec_dot2 (const double a1[])
{
	const int n = m_n;
	double value = 0.0;

	for ( int i = 0; i < n; i++ )
		value = value + a1[i] * a1[i];
	return value;
}


//****************************************************************************80

//****************************************************************************80

void gmres_t::rearrange_cr (const int nz_num, int ia[], int ja[], double a[] )

//****************************************************************************80
//
//  Purpose:
//
//    REARRANGE_CR sorts a sparse compressed row matrix.
//
//  Discussion:
//
//    This routine guarantees that the entries in the CR matrix
//    are properly sorted.
//
//    After the sorting, the entries of the matrix are rearranged in such
//    a way that the entries of each column are listed in ascending order
//    of their column values.
//
//    The matrix A is assumed to be stored in compressed row format.  Only
//    the nonzero entries of A are stored.  The vector JA stores the
//    column index of the nonzero value.  The nonzero values are sorted
//    by row, and the compressed row vector IA then has the property that
//    the entries in A and JA that correspond to row I occur in indices
//    IA[I] through IA[I+1]-1.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    18 July 2007
//
//  Author:
//
//    Original C version by Lili Ju.
//    C++ version by John Burkardt.
//
//  Parameters:
//
//    Input, int N, the order of the system.
//
//    Input, int NZ_NUM, the number of nonzeros.
//
//    Input, int IA[N+1], the compressed row index.
//
//    Input/output, int JA[NZ_NUM], the column indices.  On output,
//    the order of the entries of JA may have changed because of the sorting.
//
//    Input/output, double A[NZ_NUM], the matrix values.  On output, the
//    order of the entries may have changed because of the sorting.
//
{
  double dtemp;
  int i;
  int is;
  int itemp;
  int j;
  int j1;
  int j2;
  int k;

  const int n = m_n;

  for ( i = 0; i < n; i++ )
  {
    j1 = ia[i];
    j2 = ia[i+1];
    is = j2 - j1;

    for ( k = 1; k < is; k++ ) 
    {
      for ( j = j1; j < j2 - k; j++ ) 
      {
        if ( ja[j+1] < ja[j] ) 
        {
          itemp = ja[j+1];
          ja[j+1] =  ja[j];
          ja[j] =  itemp;

          dtemp = a[j+1];
          a[j+1] =  a[j];
          a[j] = dtemp;
        }
      }
    }
  }
  return;
}
//****************************************************************************80

