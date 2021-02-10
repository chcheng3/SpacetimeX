#include "derivs.hxx"
#include "physics.hxx"
#include "tensor.hxx"
#include "z4c_vars.hxx"

#include <loop.hxx>
#include <mempool.hxx>

#include <cctk.h>
#include <cctk_Arguments_Checked.h>
#include <cctk_Parameters.h>

#include <cmath>

namespace Z4c {
using namespace Loop;
using namespace std;

extern "C" void Z4c_ADM2(CCTK_ARGUMENTS) {
  DECLARE_CCTK_ARGUMENTS_Z4c_ADM2;
  DECLARE_CCTK_PARAMETERS;

  for (int d = 0; d < 3; ++d)
    if (cctk_nghostzones[d] < deriv_order / 2 + 1)
      CCTK_VERROR("Need at least %d ghost zones", deriv_order / 2 + 1);

  const vec3<CCTK_REAL, UP> dx{
      CCTK_DELTA_SPACE(0),
      CCTK_DELTA_SPACE(1),
      CCTK_DELTA_SPACE(2),
  };

  //

  const array<int, dim> indextype = {0, 0, 0};
  const array<int, dim> nghostzones = {cctk_nghostzones[0], cctk_nghostzones[1],
                                       cctk_nghostzones[2]};
  const array<int, dim> noghosts = {0, 0, 0};

  const GF3D1<const CCTK_REAL> gf_chi_(cctkGH, indextype, nghostzones, chi);

  const mat3<GF3D1<const CCTK_REAL>, DN, DN> gf_gammat_(
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, gammatxx),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, gammatxy),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, gammatxz),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, gammatyy),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, gammatyz),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, gammatzz));

  const GF3D1<const CCTK_REAL> gf_Kh_(cctkGH, indextype, nghostzones, Kh);

  const mat3<GF3D1<const CCTK_REAL>, DN, DN> gf_At_(
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, Atxx),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, Atxy),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, Atxz),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, Atyy),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, Atyz),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, Atzz));

  const vec3<GF3D1<const CCTK_REAL>, UP> gf_Gamt_(
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, Gamtx),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, Gamty),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, Gamtz));

  const GF3D1<const CCTK_REAL> gf_Theta_(cctkGH, indextype, nghostzones, Theta);

  const GF3D1<const CCTK_REAL> gf_alphaG_(cctkGH, indextype, nghostzones,
                                          alphaG);

  const vec3<GF3D1<const CCTK_REAL>, UP> gf_betaG_(
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, betaGx),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, betaGy),
      GF3D1<const CCTK_REAL>(cctkGH, indextype, nghostzones, betaGz));

  //

  static mempool_set_t mempools;
  mempool_t &restrict mempool = mempools.get_mempool();

  const auto make_gf = [&]() {
    return GF3D1<CCTK_REAL>(cctkGH, indextype, noghosts, mempool);
  };
  const auto make_vec_gf = [&](int) { return make_gf(); };
  const auto make_mat_gf = [&](int, int) { return make_gf(); };
  const auto make_vec_vec_gf = [&](int) {
    return [&](int) { return make_gf(); };
  };
  const auto make_vec_mat_gf = [&](int) {
    return [&](int, int) { return make_gf(); };
  };
  const auto make_mat_vec_gf = [&](int, int) {
    return [&](int) { return make_gf(); };
  };
  const auto make_mat_mat_gf = [&](int, int) {
    return [&](int, int) { return make_gf(); };
  };

  const vec3<GF3D1<CCTK_REAL>, DN> gf_dchi_(make_vec_gf);
  const mat3<GF3D1<CCTK_REAL>, DN, DN> gf_ddchi_(make_mat_gf);
  calc_derivs2(cctkGH, gf_chi_, gf_dchi_, gf_ddchi_);

  const mat3<vec3<GF3D1<CCTK_REAL>, DN>, DN, DN> gf_dgammat_(make_mat_vec_gf);
  const mat3<mat3<GF3D1<CCTK_REAL>, DN, DN>, DN, DN> gf_ddgammat_(
      make_mat_mat_gf);
  calc_derivs2(cctkGH, gf_gammat_, gf_dgammat_, gf_ddgammat_);

  const vec3<GF3D1<CCTK_REAL>, DN> gf_dKh_(make_vec_gf);
  calc_derivs(cctkGH, gf_Kh_, gf_dKh_);

  const mat3<vec3<GF3D1<CCTK_REAL>, DN>, DN, DN> gf_dAt_(make_mat_vec_gf);
  calc_derivs(cctkGH, gf_At_, gf_dAt_);

  const vec3<vec3<GF3D1<CCTK_REAL>, DN>, UP> gf_dGamt_(make_vec_vec_gf);
  calc_derivs(cctkGH, gf_Gamt_, gf_dGamt_);

  const vec3<GF3D1<CCTK_REAL>, DN> gf_dTheta_(make_vec_gf);
  calc_derivs(cctkGH, gf_Theta_, gf_dTheta_);

  const vec3<GF3D1<CCTK_REAL>, DN> gf_dalphaG_(make_vec_gf);
  const mat3<GF3D1<CCTK_REAL>, DN, DN> gf_ddalphaG_(make_mat_gf);
  calc_derivs2(cctkGH, gf_alphaG_, gf_dalphaG_, gf_ddalphaG_);

  const vec3<vec3<GF3D1<CCTK_REAL>, DN>, UP> gf_dbetaG_(make_vec_vec_gf);
  const vec3<mat3<GF3D1<CCTK_REAL>, DN, DN>, UP> gf_ddbetaG_(make_vec_mat_gf);
  calc_derivs2(cctkGH, gf_betaG_, gf_dbetaG_, gf_ddbetaG_);

  //

  const GF3D1<CCTK_REAL> gf_dtkxx_(cctkGH, indextype, nghostzones, dtkxx);
  const GF3D1<CCTK_REAL> gf_dtkxy_(cctkGH, indextype, nghostzones, dtkxy);
  const GF3D1<CCTK_REAL> gf_dtkxz_(cctkGH, indextype, nghostzones, dtkxz);
  const GF3D1<CCTK_REAL> gf_dtkyy_(cctkGH, indextype, nghostzones, dtkyy);
  const GF3D1<CCTK_REAL> gf_dtkyz_(cctkGH, indextype, nghostzones, dtkyz);
  const GF3D1<CCTK_REAL> gf_dtkzz_(cctkGH, indextype, nghostzones, dtkzz);

  const GF3D1<CCTK_REAL> gf_dt2alp_(cctkGH, indextype, nghostzones, dt2alp);

  const GF3D1<CCTK_REAL> gf_dt2betax_(cctkGH, indextype, nghostzones, dt2betax);
  const GF3D1<CCTK_REAL> gf_dt2betay_(cctkGH, indextype, nghostzones, dt2betay);
  const GF3D1<CCTK_REAL> gf_dt2betaz_(cctkGH, indextype, nghostzones, dt2betaz);

  //

  loop_int<0, 0, 0>(cctkGH, [&](const PointDesc &p) Z4C_INLINE {
    // Load and calculate

    const CCTK_REAL rho{0};
    const vec3<CCTK_REAL, DN> Si{0, 0, 0};
    const mat3<CCTK_REAL, DN, DN> Sij{0, 0, 0};

    const z4c_vars<CCTK_REAL> vars(
        kappa1, kappa2, f_mu_L, f_mu_S, eta,                  //
        gf_chi_(p.I), gf_dchi_(p.I), gf_ddchi_(p.I),          //
        gf_gammat_(p.I), gf_dgammat_(p.I), gf_ddgammat_(p.I), //
        gf_Kh_(p.I), gf_dKh_(p.I),                            //
        gf_At_(p.I), gf_dAt_(p.I),                            //
        gf_Gamt_(p.I), gf_dGamt_(p.I),                        //
        gf_Theta_(p.I), gf_dTheta_(p.I),                      //
        gf_alphaG_(p.I), gf_dalphaG_(p.I), gf_ddalphaG_(p.I), //
        gf_betaG_(p.I), gf_dbetaG_(p.I), gf_ddbetaG_(p.I),    //
        rho,                                                  //
        Si,                                                   //
        Sij);

    // Store
    vars.K_rhs.store(gf_dtkxx_, gf_dtkxy_, gf_dtkxz_, gf_dtkyy_, gf_dtkyz_,
                     gf_dtkzz_, p.I);
    gf_dt2alp_(p.I) = vars.dtalpha_rhs;
    vars.dtbeta_rhs.store(gf_dt2betax_, gf_dt2betay_, gf_dt2betaz_, p.I);
  });
}

} // namespace Z4c
