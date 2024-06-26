#include <math.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <loop_device.hxx>

#include "cctk.h"
#include "cctk_Parameters.h"
#include "cctk_Arguments.h"
#include "sphericalharmonic.hh"

static const CCTK_REAL PI = acos(-1.0);

static double factorial(int n)
{
  double returnval = 1;
  for (int i = n; i >= 1; i--)
  {
    returnval *= i;
  }
  return returnval;
}

static inline double combination(int n, int m)
{
  // Binomial coefficient is undefined if these conditions do not hold
  assert(n >= 0);
  assert(m >= 0);
  assert(m <= n);

  return factorial(n) / (factorial(m) * factorial(n-m));
}

static inline int imin(int a, int b)
{
  return a < b ? a : b;
}

static inline int imax(int a, int b)
{
  return a > b ? a : b;
}

void Multipole_SphericalHarmonic(int s, int l, int m, 
                                 CCTK_REAL th, CCTK_REAL ph,
                                 CCTK_REAL *reY, CCTK_REAL *imY)
{
//  assert(s == -2 && l == 2 && m == 2);
//  *reY = 1.0/2.0 * sqrt(5/PI) * pow(cos(th/2), 4) * cos(2*ph);
//  *imY = 1.0/2.0 * sqrt(5/PI) * pow(cos(th/2), 4) * sin(2*ph);
  double all_coeff = 0, sum = 0;
  all_coeff = pow(-1.0, m);
  all_coeff *= sqrt(factorial(l+m)*factorial(l-m)*(2*l+1) / (4.*PI*factorial(l+s)*factorial(l-s)));
  sum = 0.;
  for (int i = imax(m - s, 0); i <= imin(l + m, l - s); i++)
  {
    double sum_coeff = combination(l-s, i) * combination(l+s, i+s-m);
    sum += sum_coeff * pow(-1.0, l-i-s) * pow(cos(th/2.), 2 * i + s - m) * 
      pow(sin(th/2.), 2*(l-i)+m-s);
  }
  *reY = all_coeff*sum*cos(m*ph);
  *imY = all_coeff*sum*sin(m*ph);
}

void Multipole_HarmonicSetup(int s, int l, int m,
                             int array_size, CCTK_REAL const th[], CCTK_REAL const ph[],
                             CCTK_REAL reY[], CCTK_REAL imY[])
{
  for (int i = 0; i < array_size; i++)
  {
    Multipole_SphericalHarmonic(s,l,m,th[i],ph[i],&reY[i], &imY[i]);
  }
}


// Fill a grid function with a given spherical harmonic
extern "C" void Multipole_SetHarmonic(CCTK_ARGUMENTS)
{
  using namespace Loop;
  using namespace std;
  DECLARE_CCTK_ARGUMENTS_Multipole_SetHarmonic;
  DECLARE_CCTK_PARAMETERS;
  
  const int dim = 3;
  
  const array<int, dim> indextype = {0, 0, 0};
  const GF3D2layout layout(cctkGH, indextype);
  
  const GF3D2<CCTK_REAL> harmonic_re_(layout, harmonic_re);
  const GF3D2<CCTK_REAL> harmonic_im_(layout, harmonic_im);
  
  const GridDescBaseDevice grid(cctkGH);
  grid.loop_int<0, 0, 0>(grid.nghostzones,
                                [=] CCTK_DEVICE(const PointDesc &p)
                                    CCTK_ATTRIBUTE_ALWAYS_INLINE {
    CCTK_REAL vcoordr = sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
    CCTK_REAL theta = acos(p.z/vcoordr);
    if (vcoordr == 0) theta = 0;
    CCTK_REAL phi = atan2(p.y,p.x);

		CCTK_REAL re = 0;
		CCTK_REAL im = 0;

		Multipole_SphericalHarmonic(test_sw,test_l,test_m,theta,phi,
																&re, &im);

		CCTK_REAL fac = test_mode_proportional_to_r ? vcoordr : 1.0;
		harmonic_re_(p.I) = re * fac;
		harmonic_im_(p.I) = im * fac;
                                    });

  // for (int k = 0; k < cctk_lsh[2]+1; k++)
  // {
  //   for (int j = 0; j < cctk_lsh[1]+1; j++)
  //   {
  //     for (int i = 0; i < cctk_lsh[0]+1; i++)
  //     {
  //       int index = i + j * (cctk_lsh[0]+1) + k * (cctk_lsh[0]+1)*(cctk_lsh[1]+1)  ;
  //       vcoordr = sqrt(vcoordx[index]*vcoordx[index] + vcoordy[index]*vcoordy[index] + vcoordz[index]*vcoordz[index]);

  //       CCTK_REAL theta = acos(vcoordz[index]/vcoordr);
  //       if (vcoordr == 0) theta = 0;
  //       CCTK_REAL phi = atan2(vcoordy[index],vcoordx[index]);

  //       CCTK_REAL re = 0;
  //       CCTK_REAL im = 0;

  //       Multipole_SphericalHarmonic(test_sw,test_l,test_m,theta,phi,
  //                                   &re, &im);

  //       CCTK_REAL fac = test_mode_proportional_to_r ? vcoordr : 1.0;

  //       harmonic_re[index] = re * fac;
  //       harmonic_im[index] = im * fac;
  //     }
  //   }
  // }
  return;
}


extern "C" void Multipole_SetHarmonicWeyl(CCTK_ARGUMENTS)
{
  using namespace Loop;
  using namespace std;
  DECLARE_CCTK_ARGUMENTS_Multipole_SetHarmonicWeyl;
  DECLARE_CCTK_PARAMETERS;
  
  const int dim = 3;
  
  const array<int, dim> indextype = {0, 0, 0};
  const GF3D2layout layout(cctkGH, indextype);
  
  const GF3D2<CCTK_REAL> Psi4r_(layout, Psi4r);
  const GF3D2<CCTK_REAL> Psi4i_(layout, Psi4i);
  
  const GridDescBaseDevice grid(cctkGH);
  grid.loop_int<0, 0, 0>(grid.nghostzones,
                                [=] CCTK_DEVICE(const PointDesc &p)
                                    CCTK_ATTRIBUTE_ALWAYS_INLINE {
    CCTK_REAL vcoordr = sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
    CCTK_REAL theta = acos(p.z/vcoordr);
    if (vcoordr == 0) theta = 0;
    CCTK_REAL phi = atan2(p.y,p.x);

		CCTK_REAL re22 = 0;
		CCTK_REAL im22 = 0;

		Multipole_SphericalHarmonic(test_sw,2,2,theta,phi,
																&re22, &im22);

		CCTK_REAL re2m2 = 0;
		CCTK_REAL im2m2 = 0;

		Multipole_SphericalHarmonic(test_sw,2,-2,theta,phi,
																&re2m2, &im2m2);

		CCTK_REAL re31 = 0;
		CCTK_REAL im31 = 0;

		Multipole_SphericalHarmonic(test_sw,3,1,theta,phi,
																&re31, &im31);

		CCTK_REAL fac = test_mode_proportional_to_r ? vcoordr : 1.0;
		Psi4r_(p.I) = (re22 + 0.5 * re2m2 + 0.25 * re31) * fac;
		Psi4i_(p.I) = (im22 + 0.5 * im2m2 + 0.25 * im31) * fac;
                                    });

  return;
}
