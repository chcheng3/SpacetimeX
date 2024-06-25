#include "integrate.hxx"
#include "interpolate.hxx"
#include "multipole.hxx"
#include "sphericalharmonic.hxx"
#include "utils.hxx"

#include <assert.h>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <string>

#include <loop_device.hxx>

namespace Multipole {
using namespace std;

static const int max_vars = 10;

// since each variable can have at most one spin weight, max_vars is a good
// upper limit for the number of spin weights to expect
static const int max_spin_weights = max_vars;

// modes are 0...max_l_modes-1
static const int max_l_modes = 10;
static const int max_m_modes = 2 * max_l_modes + 1;

static void fillVariable(int idx, const char *optString, void *callbackArg) {
  assert(idx >= 0);
  assert(callbackArg != nullptr);

  VariableParseArray *vs = static_cast<VariableParseArray *>(callbackArg);

  assert(vs->numVars < max_vars);              // Ensure we don't exceed max_vars
  VariableParse *v = &vs->vars[vs->numVars++]; // Increment numVars and get
                                              // reference to next VariableParse

  v->index = idx;

  // Initialize default values
  v->imagIndex = -1;
  v->spinWeight = 0;
  v->name = string(CCTK_VarName(v->index));

  if (optString != nullptr) {
    int table = Util_TableCreateFromString(optString);

    if (table >= 0) {
      const int bufferLength = 256;
      char buffer[bufferLength];

      Util_TableGetInt(table, &v->spinWeight, "sw");
      if (Util_TableGetString(table, bufferLength, buffer, "cmplx") >= 0) {
        v->imagIndex = CCTK_VarIndex(buffer);
      }
      if (Util_TableGetString(table, bufferLength, buffer, "name") >= 0) {
        v->name = string(buffer);
      }

      const int ierr = Util_TableDestroy(table);
      if (ierr) {
        CCTK_VError(__LINE__, __FILE__, CCTK_THORNSTRING,
                    "Could not destroy table: %d", ierr);
      }
    }
  }
}

static void parse_variables_string(const string &var_string,
                                   VariableParse v[max_vars],
                                   int *n_variables) {
  VariableParseArray vars;

  vars.numVars = 0;
  vars.vars = v;

  int ierr = CCTK_TraverseString(var_string.c_str(), fillVariable, &vars,
                                 CCTK_GROUP_OR_VAR);
  assert(ierr >= 0);

  *n_variables = vars.numVars;
}

static void get_spin_weights(VariableParse vars[], int n_vars,
                             int spin_weights[max_spin_weights],
                             int *n_weights) {
  int n_spin_weights = 0;

  for (int i = 0; i < n_vars; i++) {
    if (!int_in_array(vars[i].spinWeight, spin_weights, n_spin_weights)) {
      assert(n_spin_weights < max_spin_weights);
      spin_weights[n_spin_weights] = vars[i].spinWeight;
      n_spin_weights++;
    }
  }
  *n_weights = n_spin_weights;
}

static void
setup_harmonics(const int spin_weights[max_spin_weights], int n_spin_weights,
                int lmax, CCTK_REAL th[], CCTK_REAL ph[], int array_size,
                CCTK_REAL *reY[max_spin_weights][max_l_modes][max_m_modes],
                CCTK_REAL *imY[max_spin_weights][max_l_modes][max_m_modes]) {
  for (int si = 0; si < max_spin_weights; si++) {
    for (int l = 0; l < max_l_modes; l++) {
      for (int mi = 0; mi < max_m_modes; mi++) {
        reY[si][l][mi] = 0;
        imY[si][l][mi] = 0;
      }
    }
  }
  for (int si = 0; si < n_spin_weights; si++) {
    int sw = spin_weights[si];

    for (int l = 0; l <= lmax; l++) {
      for (int m = -l; m <= l; m++) {
        reY[si][l][m + l] = new CCTK_REAL[array_size];
        imY[si][l][m + l] = new CCTK_REAL[array_size];

        HarmonicSetup(sw, l, m, array_size, th, ph, reY[si][l][m + l],
                      imY[si][l][m + l]);
      }
    }
  }
}

static void output_modes(CCTK_ARGUMENTS, const VariableParse vars[],
                         const CCTK_REAL radii[], const ModeArray &modes) {
  DECLARE_CCTK_ARGUMENTS;
  DECLARE_CCTK_PARAMETERS;

  if (output_tsv) {
    if (CCTK_MyProc(cctkGH) == 0) {
      for (int v = 0; v < modes.getNumVars(); v++) {
        for (int i = 0; i < modes.getNumRadii(); i++) {
          const CCTK_REAL rad = radii[i];
          for (int l = 0; l <= modes.getMaxL(); l++) {
            for (int m = -l; m <= l; m++) {
              ostringstream name;
              name << "mp_" << vars[v].name << "_l" << l << "_m" << m << "_r"
                   << setiosflags(ios::fixed) << setprecision(2) << rad
                   << ".tsv";
              OutputComplexToFile(CCTK_PASS_CTOC, name.str(),
                                  modes(v, i, l, m, 0), modes(v, i, l, m, 1));
            }
          }
        }
      }
    }
  }
  if (output_hdf5) {
    if (CCTK_MyProc(cctkGH) == 0) {
      OutputComplexToH5File(CCTK_PASS_CTOC, vars, radii, modes);
    }
  }
}

static void output_1d(CCTK_ARGUMENTS, const VariableParse *v, CCTK_REAL rad,
                      CCTK_REAL *th, CCTK_REAL *ph, CCTK_REAL *real,
                      CCTK_REAL *imag, int array_size) {
  DECLARE_CCTK_ARGUMENTS;
  DECLARE_CCTK_PARAMETERS;

  if (CCTK_MyProc(cctkGH) == 0 && output_tsv) {
    if (out_1d_every != 0 && cctk_iteration % out_1d_every == 0) {
      ostringstream real_base;
      real_base << "mp_" << string(CCTK_VarName(v->index)) << "_r"
                << setiosflags(ios::fixed) << setprecision(2) << rad;
      Output1D(CCTK_PASS_CTOC, real_base.str() + string(".th.tsv"), array_size,
               th, ph, mp_theta, real);
      Output1D(CCTK_PASS_CTOC, real_base.str() + string(".ph.tsv"), array_size,
               th, ph, mp_phi, real);

      if (v->imagIndex != -1) {
        ostringstream imag_base;
        imag_base << "mp_" << string(CCTK_VarName(v->imagIndex)) << "_r"
                  << setiosflags(ios::fixed) << setprecision(2) << rad;
        Output1D(CCTK_PASS_CTOC, imag_base.str() + string(".th.tsv"),
                 array_size, th, ph, mp_theta, imag);
        Output1D(CCTK_PASS_CTOC, imag_base.str() + string(".ph.tsv"),
                 array_size, th, ph, mp_phi, imag);
      }
    }
  }
}

// Sets harmonic coefficients to zero at init.
extern "C" void Multipole_Init(CCTK_ARGUMENTS) {
  DECLARE_CCTK_ARGUMENTSX_Multipole_Init;
  DECLARE_CCTK_PARAMETERS;

  grid.loop_int<0, 0, 0>(grid.nghostzones,
                         [=] CCTK_DEVICE(const Loop::PointDesc &p)
                             CCTK_ATTRIBUTE_ALWAYS_INLINE {
                               harmonic_re(p.I) = 0;
                               harmonic_im(p.I) = 0;
                             });
}

// This is the main scheduling file.  Because we are completely local here
// and do not use cactus arrays etc, we schedule only one function and then
// like program like one would in C, C++ with this function taking the
// place of int main(void).
// This function calls functions to accomplish 3 things:
//   1) Interpolate psi4 onto a sphere
//   2) Integrate psi4 with the ylm's over that sphere
//   3) Output the mode decomposed psi4
extern "C" void Multipole_Calc(CCTK_ARGUMENTS) {
  DECLARE_CCTK_ARGUMENTS_Multipole_Calc;
  DECLARE_CCTK_PARAMETERS;

  static CCTK_REAL *xs, *ys, *zs;
  static CCTK_REAL *xhat, *yhat, *zhat;
  static CCTK_REAL *th, *ph;
  static CCTK_REAL *real = 0, *imag = 0;
  static CCTK_REAL *reY[max_spin_weights][max_l_modes][max_m_modes];
  static CCTK_REAL *imY[max_spin_weights][max_l_modes][max_m_modes];
  static VariableParse vars[max_vars];
  static int n_variables = 0;
  static int spin_weights[max_spin_weights];
  static int n_spin_weights = 0;

  static bool initialized = false;

  const int array_size = (ntheta + 1) * (nphi + 1);

  if (out_every == 0 || cctk_iteration % out_every != 0)
    return;

  int lmax = l_max;

  assert(lmax + 1 <= max_l_modes);

  if (!initialized) {
    real = new CCTK_REAL[array_size];
    imag = new CCTK_REAL[array_size];
    th = new CCTK_REAL[array_size];
    ph = new CCTK_REAL[array_size];
    xs = new CCTK_REAL[array_size];
    ys = new CCTK_REAL[array_size];
    zs = new CCTK_REAL[array_size];
    xhat = new CCTK_REAL[array_size];
    yhat = new CCTK_REAL[array_size];
    zhat = new CCTK_REAL[array_size];

    parse_variables_string(string(variables), vars, &n_variables);
    get_spin_weights(vars, n_variables, spin_weights, &n_spin_weights);
    CoordSetup(xhat, yhat, zhat, th, ph);
    setup_harmonics(spin_weights, n_spin_weights, lmax, th, ph, array_size, reY,
                    imY);
    initialized = true;
    CCTK_VINFO("initialized arrays");
  }

  ModeArray modes(n_variables, nradii, lmax);

  for (int v = 0; v < n_variables; v++) {
    // assert(vars[v].spinWeight == -2);
    int si =
        find_int_in_array(vars[v].spinWeight, spin_weights, n_spin_weights);
    assert(si != -1);

    for (int i = 0; i < nradii; i++) {
      // Compute x^i = r * \hat x^i
      ScaleCartesian(ntheta, nphi, radius[i], xhat, yhat, zhat, xs, ys, zs);

      // Interpolate Psi4r and Psi4i
      Interp(CCTK_PASS_CTOC, xs, ys, zs, vars[v].index, vars[v].imagIndex,
             real, imag);

      for (int l = 0; l <= lmax; l++) {
        for (int m = -l; m <= l; m++) {
          // Integrate sYlm (real + i imag) over the sphere at radius r
          Integrate(array_size, ntheta, reY[si][l][m + l], imY[si][l][m + l],
                    real, imag, th, ph, &modes(v, i, l, m, 0),
                    &modes(v, i, l, m, 1));

        } // loop over m
      } // loop over l
      output_1d(CCTK_PASS_CTOC, &vars[v], radius[i], th, ph, real, imag,
                array_size);
    } // loop over radii
  } // loop over variables

  output_modes(CCTK_PASS_CTOC, vars, radius, modes);
}

} // namespace Multipole
