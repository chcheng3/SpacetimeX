#ifndef PHYSICS_HXX
#define PHYSICS_HXX

#include <dual.hxx>
#include <mat.hxx>
#include <rten.hxx>
#include <simd.hxx>
#include <sum.hxx>
#include <vec.hxx>

#include <cmath>
#include <limits>

namespace Weyl {
using namespace Arith;
using namespace std;

#if 0
template <typename T, int D, dnup_t dnup1, dnup_t dnup2, symm_t symm>
gmat<T, D - 1, dnup1, dnup2, symm>
calc_submatrix(const gmat<T, D, dnup1, dnup2, symm> A, int i0, int j0) {
  return gmat<T, D - 1, dnup1, dnup2, symm>([&](int is, int js) {
    const int i = is + (is >= i0);
    const int j = js + (js >= j0);
    return A(i, j);
  });
}

template <typename T, int D, dnup_t dnup1, dnup_t dnup2, symm_t symm>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T
calc_det(const gmat<T, D, dnup1, dnup2, symm> &g) {
  if constexpr (D == 0) {
    return one<T>();
  } else {
    T detg = zero<T>();
    for (int i = 0; i < D; ++i)
      detg += bitsign(i) * calc_det(calc_submatrix(g, i, 0));
    return detg;
  }
}

template <typename T, int D, dnup_t dnup1, dnup_t dnup2, symm_t symm>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr gmat<T, D, !dnup1, !dnup2, symm>
calc_inv(const gmat<T, D, dnup1, dnup2, symm> &g, const T &detg) {
  const T detg1 = 1 / detg;
  return gmat<T, D, !dnup1, !dnup2, symm>([&](int i, int j) {
    return bitsign(i) * bitsign(j) * detg1 * calc_det(calc_submatrix(g, j, i));
  });
}
#endif

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T
calc_det(const smat<T, 0, dnup1, dnup2> &g) {
  return one<T>();
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T
calc_det(const smat<T, 1, dnup1, dnup2> &g) {
  return g(0, 0);
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T
calc_det(const smat<T, 2, dnup1, dnup2> &g) {
  return g(0, 0) * g(1, 1) - pow2(g(0, 1));
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T
calc_det(const smat<T, 3, dnup1, dnup2> &g) {
  return 2 * g(0, 1) * g(0, 2) * g(1, 2) - g(2, 2) * pow2(g(0, 1)) -
         g(1, 1) * pow2(g(0, 2)) +
         g(0, 0) * (g(1, 1) * g(2, 2) - pow2(g(1, 2)));
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T
calc_det(const smat<T, 4, dnup1, dnup2> &g) {
  return 2 * g(0, 0) * g(1, 2) * g(1, 3) * g(2, 3) -
         2 * g(0, 3) *
             (g(0, 2) * (g(1, 2) * g(1, 3) - g(1, 1) * g(2, 3)) +
              g(0, 1) * (-(g(1, 3) * g(2, 2)) + g(1, 2) * g(2, 3))) +
         g(0, 0) * g(1, 1) * g(2, 2) * g(3, 3) +
         2 * g(0, 1) * g(0, 2) * (-(g(1, 3) * g(2, 3)) + g(1, 2) * g(3, 3)) -
         g(2, 2) * g(3, 3) * pow2(g(0, 1)) - g(0, 0) * g(3, 3) * pow2(g(1, 2)) +
         pow2(g(0, 3)) * (-(g(1, 1) * g(2, 2)) + pow2(g(1, 2))) -
         g(0, 0) * g(2, 2) * pow2(g(1, 3)) +
         pow2(g(0, 2)) * (-(g(1, 1) * g(3, 3)) + pow2(g(1, 3))) -
         g(0, 0) * g(1, 1) * pow2(g(2, 3)) + pow2(g(0, 1)) * pow2(g(2, 3));
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T
calc_det(const mat<T, 0, dnup1, dnup2> &g) {
  return one<T>();
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T
calc_det(const mat<T, 1, dnup1, dnup2> &g) {
  return g(0, 0);
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T
calc_det(const mat<T, 2, dnup1, dnup2> &g) {
  return -(g(0, 1) * g(1, 0)) + g(0, 0) * g(1, 1);
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T
calc_det(const mat<T, 3, dnup1, dnup2> &g) {
  return g(0, 2) * (-(g(1, 1) * g(2, 0)) + g(1, 0) * g(2, 1)) +
         g(0, 1) * (g(1, 2) * g(2, 0) - g(1, 0) * g(2, 2)) +
         g(0, 0) * (-(g(1, 2) * g(2, 1)) + g(1, 1) * g(2, 2));
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T
calc_det(const mat<T, 4, dnup1, dnup2> &g) {
  return g(0, 1) * g(1, 3) * g(2, 2) * g(3, 0) -
         g(0, 1) * g(1, 2) * g(2, 3) * g(3, 0) -
         g(0, 0) * g(1, 3) * g(2, 2) * g(3, 1) +
         g(0, 0) * g(1, 2) * g(2, 3) * g(3, 1) -
         g(0, 1) * g(1, 3) * g(2, 0) * g(3, 2) +
         g(0, 0) * g(1, 3) * g(2, 1) * g(3, 2) +
         g(0, 1) * g(1, 0) * g(2, 3) * g(3, 2) -
         g(0, 0) * g(1, 1) * g(2, 3) * g(3, 2) +
         g(0, 3) * (g(1, 2) * (g(2, 1) * g(3, 0) - g(2, 0) * g(3, 1)) +
                    g(1, 1) * (-(g(2, 2) * g(3, 0)) + g(2, 0) * g(3, 2)) +
                    g(1, 0) * (g(2, 2) * g(3, 1) - g(2, 1) * g(3, 2))) +
         g(0, 1) * g(1, 2) * g(2, 0) * g(3, 3) -
         g(0, 0) * g(1, 2) * g(2, 1) * g(3, 3) -
         g(0, 1) * g(1, 0) * g(2, 2) * g(3, 3) +
         g(0, 0) * g(1, 1) * g(2, 2) * g(3, 3) +
         g(0, 2) * (g(1, 3) * (-(g(2, 1) * g(3, 0)) + g(2, 0) * g(3, 1)) +
                    g(1, 1) * (g(2, 3) * g(3, 0) - g(2, 0) * g(3, 3)) +
                    g(1, 0) * (-(g(2, 3) * g(3, 1)) + g(2, 1) * g(3, 3)));
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr smat<T, 0, !dnup1, !dnup2>
calc_inv(const smat<T, 0, dnup1, dnup2> &g, const T &detg) {
  return smat<T, 0, !dnup1, !dnup2>([&](int i, int j) { return one<T>()(); });
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr smat<T, 1, !dnup1, !dnup2>
calc_inv(const smat<T, 1, dnup1, dnup2> &g, const T &detg) {
  const T detg1 = 1 / detg;
  return smat<T, 1, !dnup1, !dnup2>{detg1};
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr smat<T, 2, !dnup1, !dnup2>
calc_inv(const smat<T, 2, dnup1, dnup2> &g, const T &detg) {
  const T detg1 = 1 / detg;
  return detg1 * smat<T, 2, !dnup1, !dnup2>{
                     g(1, 1),
                     -g(0, 1),

                     g(0, 0),
                 };
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr smat<T, 3, !dnup1, !dnup2>
calc_inv(const smat<T, 3, dnup1, dnup2> &g, const T &detg) {
  const T detg1 = 1 / detg;
  return detg1 * smat<T, 3, !dnup1, !dnup2>{
                     g(1, 1) * g(2, 2) - pow2(g(1, 2)),
                     g(0, 2) * g(1, 2) - g(0, 1) * g(2, 2),
                     -(g(0, 2) * g(1, 1)) + g(0, 1) * g(1, 2),
                     g(0, 0) * g(2, 2) - pow2(g(0, 2)),
                     g(0, 1) * g(0, 2) - g(0, 0) * g(1, 2),
                     g(0, 0) * g(1, 1) - pow2(g(0, 1)),
                 };
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr smat<T, 4, !dnup1, !dnup2>
calc_inv(const smat<T, 4, dnup1, dnup2> &g, const T &detg) {
  const T detg1 = 1 / detg;
  return detg1 * smat<T, 4, !dnup1, !dnup2>{
                     2 * g(1, 2) * g(1, 3) * g(2, 3) - g(3, 3) * pow2(g(1, 2)) -
                         g(2, 2) * pow2(g(1, 3)) +
                         g(1, 1) * (g(2, 2) * g(3, 3) - pow2(g(2, 3))),
                     g(0, 3) * (g(1, 3) * g(2, 2) - g(1, 2) * g(2, 3)) +
                         g(0, 2) * (-(g(1, 3) * g(2, 3)) + g(1, 2) * g(3, 3)) +
                         g(0, 1) * (-(g(2, 2) * g(3, 3)) + pow2(g(2, 3))),
                     g(0, 3) * (-(g(1, 2) * g(1, 3)) + g(1, 1) * g(2, 3)) +
                         g(0, 1) * (-(g(1, 3) * g(2, 3)) + g(1, 2) * g(3, 3)) +
                         g(0, 2) * (-(g(1, 1) * g(3, 3)) + pow2(g(1, 3))),
                     g(0, 2) * (-(g(1, 2) * g(1, 3)) + g(1, 1) * g(2, 3)) +
                         g(0, 1) * (g(1, 3) * g(2, 2) - g(1, 2) * g(2, 3)) +
                         g(0, 3) * (-(g(1, 1) * g(2, 2)) + pow2(g(1, 2))),

                     2 * g(0, 2) * g(0, 3) * g(2, 3) - g(3, 3) * pow2(g(0, 2)) -
                         g(2, 2) * pow2(g(0, 3)) +
                         g(0, 0) * (g(2, 2) * g(3, 3) - pow2(g(2, 3))),
                     -(g(0, 3) * (g(0, 2) * g(1, 3) + g(0, 1) * g(2, 3))) +
                         g(0, 1) * g(0, 2) * g(3, 3) +
                         g(0, 0) * (g(1, 3) * g(2, 3) - g(1, 2) * g(3, 3)) +
                         g(1, 2) * pow2(g(0, 3)),
                     g(0, 1) * g(0, 3) * g(2, 2) -
                         g(0, 2) * (g(0, 3) * g(1, 2) + g(0, 1) * g(2, 3)) +
                         g(0, 0) * (-(g(1, 3) * g(2, 2)) + g(1, 2) * g(2, 3)) +
                         g(1, 3) * pow2(g(0, 2)),

                     2 * g(0, 1) * g(0, 3) * g(1, 3) - g(3, 3) * pow2(g(0, 1)) -
                         g(1, 1) * pow2(g(0, 3)) +
                         g(0, 0) * (g(1, 1) * g(3, 3) - pow2(g(1, 3))),
                     -(g(0, 1) * g(0, 3) * g(1, 2)) +
                         g(0, 2) * (g(0, 3) * g(1, 1) - g(0, 1) * g(1, 3)) +
                         g(0, 0) * (g(1, 2) * g(1, 3) - g(1, 1) * g(2, 3)) +
                         g(2, 3) * pow2(g(0, 1)),

                     2 * g(0, 1) * g(0, 2) * g(1, 2) - g(2, 2) * pow2(g(0, 1)) -
                         g(1, 1) * pow2(g(0, 2)) +
                         g(0, 0) * (g(1, 1) * g(2, 2) - pow2(g(1, 2))),
                 };
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr mat<T, 0, !dnup1, !dnup2>
calc_inv(const mat<T, 0, dnup1, dnup2> &g, const T &detg) {
  return mat<T, 0, !dnup1, !dnup2>([&](int i, int j) { return one<T>()(); });
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr mat<T, 1, !dnup1, !dnup2>
calc_inv(const mat<T, 1, dnup1, dnup2> &g, const T &detg) {
  const T detg1 = 1 / detg;
  return mat<T, 1, !dnup1, !dnup2>{detg1};
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr mat<T, 2, !dnup1, !dnup2>
calc_inv(const mat<T, 2, dnup1, dnup2> &g, const T &detg) {
  const T detg1 = 1 / detg;
  return detg1 * mat<T, 2, !dnup1, !dnup2>{
                     g(1, 1),
                     -g(0, 1),

                     -g(1, 0),
                     g(0, 0),
                 };
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr mat<T, 3, !dnup1, !dnup2>
calc_inv(const mat<T, 3, dnup1, dnup2> &g, const T &detg) {
  const T detg1 = 1 / detg;
  return detg1 * mat<T, 3, !dnup1, !dnup2>{
                     -(g(1, 2) * g(2, 1)) + g(1, 1) * g(2, 2),
                     g(0, 2) * g(2, 1) - g(0, 1) * g(2, 2),
                     -(g(0, 2) * g(1, 1)) + g(0, 1) * g(1, 2),

                     g(1, 2) * g(2, 0) - g(1, 0) * g(2, 2),
                     -(g(0, 2) * g(2, 0)) + g(0, 0) * g(2, 2),
                     g(0, 2) * g(1, 0) - g(0, 0) * g(1, 2),

                     -(g(1, 1) * g(2, 0)) + g(1, 0) * g(2, 1),
                     g(0, 1) * g(2, 0) - g(0, 0) * g(2, 1),
                     -(g(0, 1) * g(1, 0)) + g(0, 0) * g(1, 1),
                 };
}

template <typename T, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr mat<T, 4, !dnup1, !dnup2>
calc_inv(const mat<T, 4, dnup1, dnup2> &g, const T &detg) {
  const T detg1 = 1 / detg;
  return detg1 * mat<T, 4, !dnup1, !dnup2>{
                     g(1, 3) * (-(g(2, 2) * g(3, 1)) + g(2, 1) * g(3, 2)) +
                         g(1, 2) * (g(2, 3) * g(3, 1) - g(2, 1) * g(3, 3)) +
                         g(1, 1) * (-(g(2, 3) * g(3, 2)) + g(2, 2) * g(3, 3)),
                     g(0, 3) * (g(2, 2) * g(3, 1) - g(2, 1) * g(3, 2)) +
                         g(0, 2) * (-(g(2, 3) * g(3, 1)) + g(2, 1) * g(3, 3)) +
                         g(0, 1) * (g(2, 3) * g(3, 2) - g(2, 2) * g(3, 3)),
                     g(0, 3) * (-(g(1, 2) * g(3, 1)) + g(1, 1) * g(3, 2)) +
                         g(0, 2) * (g(1, 3) * g(3, 1) - g(1, 1) * g(3, 3)) +
                         g(0, 1) * (-(g(1, 3) * g(3, 2)) + g(1, 2) * g(3, 3)),
                     g(0, 3) * (g(1, 2) * g(2, 1) - g(1, 1) * g(2, 2)) +
                         g(0, 2) * (-(g(1, 3) * g(2, 1)) + g(1, 1) * g(2, 3)) +
                         g(0, 1) * (g(1, 3) * g(2, 2) - g(1, 2) * g(2, 3)),

                     g(1, 3) * (g(2, 2) * g(3, 0) - g(2, 0) * g(3, 2)) +
                         g(1, 2) * (-(g(2, 3) * g(3, 0)) + g(2, 0) * g(3, 3)) +
                         g(1, 0) * (g(2, 3) * g(3, 2) - g(2, 2) * g(3, 3)),
                     g(0, 3) * (-(g(2, 2) * g(3, 0)) + g(2, 0) * g(3, 2)) +
                         g(0, 2) * (g(2, 3) * g(3, 0) - g(2, 0) * g(3, 3)) +
                         g(0, 0) * (-(g(2, 3) * g(3, 2)) + g(2, 2) * g(3, 3)),
                     g(0, 3) * (g(1, 2) * g(3, 0) - g(1, 0) * g(3, 2)) +
                         g(0, 2) * (-(g(1, 3) * g(3, 0)) + g(1, 0) * g(3, 3)) +
                         g(0, 0) * (g(1, 3) * g(3, 2) - g(1, 2) * g(3, 3)),
                     g(0, 3) * (-(g(1, 2) * g(2, 0)) + g(1, 0) * g(2, 2)) +
                         g(0, 2) * (g(1, 3) * g(2, 0) - g(1, 0) * g(2, 3)) +
                         g(0, 0) * (-(g(1, 3) * g(2, 2)) + g(1, 2) * g(2, 3)),

                     g(1, 3) * (-(g(2, 1) * g(3, 0)) + g(2, 0) * g(3, 1)) +
                         g(1, 1) * (g(2, 3) * g(3, 0) - g(2, 0) * g(3, 3)) +
                         g(1, 0) * (-(g(2, 3) * g(3, 1)) + g(2, 1) * g(3, 3)),
                     g(0, 3) * (g(2, 1) * g(3, 0) - g(2, 0) * g(3, 1)) +
                         g(0, 1) * (-(g(2, 3) * g(3, 0)) + g(2, 0) * g(3, 3)) +
                         g(0, 0) * (g(2, 3) * g(3, 1) - g(2, 1) * g(3, 3)),
                     g(0, 3) * (-(g(1, 1) * g(3, 0)) + g(1, 0) * g(3, 1)) +
                         g(0, 1) * (g(1, 3) * g(3, 0) - g(1, 0) * g(3, 3)) +
                         g(0, 0) * (-(g(1, 3) * g(3, 1)) + g(1, 1) * g(3, 3)),
                     g(0, 3) * (g(1, 1) * g(2, 0) - g(1, 0) * g(2, 1)) +
                         g(0, 1) * (-(g(1, 3) * g(2, 0)) + g(1, 0) * g(2, 3)) +
                         g(0, 0) * (g(1, 3) * g(2, 1) - g(1, 1) * g(2, 3)),

                     g(1, 2) * (g(2, 1) * g(3, 0) - g(2, 0) * g(3, 1)) +
                         g(1, 1) * (-(g(2, 2) * g(3, 0)) + g(2, 0) * g(3, 2)) +
                         g(1, 0) * (g(2, 2) * g(3, 1) - g(2, 1) * g(3, 2)),
                     g(0, 2) * (-(g(2, 1) * g(3, 0)) + g(2, 0) * g(3, 1)) +
                         g(0, 1) * (g(2, 2) * g(3, 0) - g(2, 0) * g(3, 2)) +
                         g(0, 0) * (-(g(2, 2) * g(3, 1)) + g(2, 1) * g(3, 2)),
                     g(0, 2) * (g(1, 1) * g(3, 0) - g(1, 0) * g(3, 1)) +
                         g(0, 1) * (-(g(1, 2) * g(3, 0)) + g(1, 0) * g(3, 2)) +
                         g(0, 0) * (g(1, 2) * g(3, 1) - g(1, 1) * g(3, 2)),
                     g(0, 2) * (-(g(1, 1) * g(2, 0)) + g(1, 0) * g(2, 1)) +
                         g(0, 1) * (g(1, 2) * g(2, 0) - g(1, 0) * g(2, 2)) +
                         g(0, 0) * (-(g(1, 2) * g(2, 1)) + g(1, 1) * g(2, 2)),
                 };
}

template <typename T, int D, dnup_t dnup1, dnup_t dnup2>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr T calc_trace(
    const smat<T, D, dnup1, dnup2> &A, const smat<T, D, !dnup1, !dnup2> &gu) {
  return sum_symm<D>([&](int x, int y)
                         ARITH_INLINE { return gu(x, y) * A(x, y); });
}

template <typename T, int D>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr smat<vec<T, D, DN>, D, UP, UP>
calc_dgu(const smat<T, D, UP, UP> &gu,
         const smat<vec<T, D, DN>, D, DN, DN> &dg) {
  // g_xy = g_xa g_yb g^ab
  // g_xy,c = (g_xa g_yb g^ab),c
  //        = g_xa,c g_yb g^ab + g_xa g_yb,c g^ab + g_xa g_yb g^ab,c
  // g_xa g_yb g^ab,c = g_xy,c - g_xa,c g_yb g^ab - g_xa g_yb,c g^ab
  //                  = g_xy,c - g_xy,c - g_xy,c
  //                  = - g_xy,c
  // g^ab,c = - g^ax g^by g_xy,c
  return smat<vec<T, D, DN>, D, UP, UP>([&](int a, int b) ARITH_INLINE {
    return vec<T, D, DN>([&](int c) ARITH_INLINE {
      // return sum2sym([&](int x, int y) ARITH_INLINE {
      //   return -gu(a, x) * gu(b, y) * dg(x, y)(c);
      // });
      return sum<D>([&](int x) ARITH_INLINE {
        return -gu(a, x) * sum<D>([&](int y) ARITH_INLINE {
          return gu(b, y) * dg(x, y)(c);
        });
      });
    });
  });
}

template <typename T, int D>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr smat<vec<T, D, DN>, D, UP, UP>
calc_dAu(const smat<T, D, UP, UP> &gu,
         const smat<vec<T, D, DN>, D, UP, UP> &dgu, const smat<T, D, DN, DN> &A,
         const smat<vec<T, D, DN>, D, DN, DN> &dA) {
  // A^ab,c = (g^ax g^by A_xy),c
  //        = g^ax,c g^by A_xy + g^ax g^by,c A_xy + g^ax g^by A_xy,c
  return smat<vec<T, D, DN>, D, UP, UP>([&](int a, int b) ARITH_INLINE {
    return vec<T, D, DN>([&](int c) ARITH_INLINE {
      // return sum2sym([&](int x, int y) ARITH_INLINE {
      //   return dgu(a, x)(c) * gu(b, y) * A(x, y)   //
      //          + gu(a, x) * dgu(b, y)(c) * A(x, y) //
      //          + gu(a, x) * gu(b, y) * dA(x, y)(c);
      // });
      return sum<D>([&](int x) ARITH_INLINE {
        return gu(b, x) * sum<D>([&](int y) ARITH_INLINE {
                 return dgu(a, y)(c) * A(x, y)   //
                        + dgu(b, y)(c) * A(x, y) //
                        + gu(b, y) * dA(x, y)(c);
               });
      });
    });
  });
}

template <typename T, int D>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr vec<smat<T, D, DN, DN>, D, DN>
calc_gammal(const smat<vec<T, D, DN>, D, DN, DN> &dg) {
  // Gammal_abc
  return vec<smat<T, D, DN, DN>, D, DN>([&](int a) ARITH_INLINE {
    return smat<T, D, DN, DN>([&](int b, int c) ARITH_INLINE {
      return (dg(a, b)(c) + dg(a, c)(b) - dg(b, c)(a)) / 2;
    });
  });
}

template <typename T, int D>
ARITH_INLINE ARITH_DEVICE ARITH_HOST constexpr vec<smat<T, D, DN, DN>, D, UP>
calc_gamma(const smat<T, D, UP, UP> &gu,
           const vec<smat<T, D, DN, DN>, D, DN> &Gammal) {
  // Gamma^a_bc
  return vec<smat<T, D, DN, DN>, D, UP>([&](int a) ARITH_INLINE {
    return smat<T, D, DN, DN>([&](int b, int c) ARITH_INLINE {
      return sum<D>([&](int x)
                        ARITH_INLINE { return gu(a, x) * Gammal(x)(b, c); });
    });
  });
}

template <typename T, int D>
ARITH_INLINE
    ARITH_DEVICE ARITH_HOST constexpr vec<vec<vec<T, D, UP>, D, DN>, D, DN>
    calc_gammalu(const smat<T, D, UP, UP> &gu,
                 const vec<smat<T, D, DN, DN>, D, DN> &Gammal) {
  // Gamma_ab^c
  return vec<vec<vec<T, D, UP>, D, DN>, D, DN>([&](int a) ARITH_INLINE {
    return vec<vec<T, D, UP>, D, DN>([&](int b) ARITH_INLINE {
      return vec<T, D, UP>([&](int c) ARITH_INLINE {
        return sum<D>([&](int x)
                          ARITH_INLINE { return Gammal(a)(b, x) * gu(x, c); });
      });
    });
  });
}

template <typename T, int D>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST
    vec<smat<vec<T, D, DN>, D, DN, DN>, D, DN>
    calc_dgammal(const smat<smat<T, D, DN, DN>, D, DN, DN> &ddg) {
  // Gamma_abc,d
  return vec<smat<vec<T, D, DN>, D, DN, DN>, D, DN>([&](int a) ARITH_INLINE {
    return smat<vec<T, D, DN>, D, DN, DN>([&](int b, int c) ARITH_INLINE {
      return vec<T, D, DN>([&](int d) ARITH_INLINE {
        return sum<D>([&](int x) ARITH_INLINE {
          return (ddg(a, b)(c, d) + ddg(a, c)(b, d) - ddg(b, c)(a, d)) / 2;
        });
      });
    });
  });
}

template <typename T, int D>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST
    vec<smat<vec<T, D, DN>, D, DN, DN>, D, UP>
    calc_dgamma(const smat<T, D, UP, UP> &gu,
                const smat<vec<T, D, DN>, D, UP, UP> &dgu,
                const vec<smat<T, D, DN, DN>, D, DN> &Gammal,
                const vec<smat<vec<T, D, DN>, D, DN, DN>, D, DN> &dGammal) {
  // Gamma^a_bc,d
  return vec<smat<vec<T, D, DN>, D, DN, DN>, D, UP>([&](int a) ARITH_INLINE {
    return smat<vec<T, D, DN>, D, DN, DN>([&](int b, int c) ARITH_INLINE {
      return vec<T, D, DN>([&](int d) ARITH_INLINE {
        return sum<D>([&](int x) ARITH_INLINE {
          return dgu(a, x)(d) * Gammal(x)(b, c) +
                 gu(a, x) * dGammal(x)(b, c)(d);
        });
      });
    });
  });
}

template <typename T, int D>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST rten<T, D, DN, DN, DN, DN>
calc_riemann(const smat<T, D, DN, DN> &g,
             const vec<smat<T, D, DN, DN>, D, UP> &Gamma,
             const vec<smat<vec<T, D, DN>, D, DN, DN>, D, UP> &dGamma) {
  // Rm_abcd
  return rten<T, D, DN, DN, DN, DN>(
      [&](int a, int b, int c, int d) ARITH_INLINE {
        return sum<D>([&](int x) ARITH_INLINE {
          const T rmuxbcd = dGamma(x)(d, b)(c)   //
                            - dGamma(x)(c, b)(d) //
                            + sum<D>([&](int y) ARITH_INLINE {
                                return Gamma(x)(c, y) * Gamma(y)(d, b);
                              }) //
                            - sum<D>([&](int y) ARITH_INLINE {
                                return Gamma(x)(d, y) * Gamma(y)(c, b);
                              });
          return g(a, x) * rmuxbcd;
        });
      });
}

template <typename T, int D>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST smat<T, D, DN, DN>
calc_ricci(const smat<T, D, UP, UP> &gu, const rten<T, D, DN, DN, DN, DN> &Rm) {
  // R_ab
  return smat<T, D, DN, DN>([&](int a, int b) ARITH_INLINE {
    return sum_symm<D>([&](int x, int y)
                           ARITH_INLINE { return gu(x, y) * Rm(x, a, y, b); });
  });
}

template <typename T, int D>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST rten<T, D, DN, DN, DN, DN>
calc_weyl(const smat<T, D, DN, DN> &g, const rten<T, D, DN, DN, DN, DN> &Rm,
          const smat<T, D, DN, DN> &R, const T &Rsc) {
  // C_abcd
  return rten<T, D, DN, DN, DN, DN>(
      [&](int a, int b, int c, int d) ARITH_INLINE {
        return Rm(a, b, c, d) //
               + 1 / T{D - 2} *
                     (R(a, d) * g(b, c) - R(a, c) * g(b, d)    //
                      + R(b, c) * g(a, d) - R(b, d) * g(a, c)) //
               + 1 / T{(D - 1) * (D - 2)} * Rsc *
                     (g(a, c) * g(b, d) - g(a, d) * g(b, c));
      });
}

template <typename T, int D, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<T, D, UP>
raise(const gmat<T, D, UP, UP, symm> &gu, const vec<T, D, DN> &vl) {
  return vec<T, D, UP>([&](int a) ARITH_INLINE {
    return sum<D>([&](int x) ARITH_INLINE { return gu(a, x) * vl(x); });
  });
}

template <typename T, int D, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<T, D, DN>
lower(const gmat<T, D, DN, DN, symm> &g, const vec<T, D, UP> &v) {
  return vec<T, D, DN>([&](int a) ARITH_INLINE {
    return sum<D>([&](int x) ARITH_INLINE { return g(a, x) * v(x); });
  });
}

template <typename T, int D>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST T dot(const vec<T, D, DN> &vl,
                                                     const vec<T, D, UP> &v) {
  return sum<D>([&](int x) ARITH_INLINE { return vl(x) * v(x); });
}

template <typename T, int D, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<T, D, UP>
normalized(const gmat<T, D, DN, DN, symm> &g, const vec<T, D, UP> &v) {
  return v / sqrt(dot(lower(g, v), v));
}

template <typename T, int D, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<T, D, UP>
projected(const gmat<T, D, DN, DN, symm> &g, const vec<T, D, UP> &v,
          const vec<T, D, UP> &w) {
  const auto z = zero<T>()();
  const auto wl = lower(g, w);
  const auto wlen2 = dot(wl, w);
  return if_else(wlen2 <= numeric_limits<T>::epsilon(), vec<T, D, UP>::pure(z),
                 dot(wl, v) / wlen2 * w);
}

template <typename T, int D>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<T, D, UP>
projected1(const vec<T, D, UP> &v, const vec<T, D, DN> &wl,
           const vec<T, D, UP> &w) {
  // assuming dot(wl, w) == 1
  return dot(wl, v) * w;
}

template <typename T, int D, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<T, D, UP>
rejected(const gmat<T, D, DN, DN, symm> &g, const vec<T, D, UP> &v,
         const vec<T, D, UP> &w) {
  return v - projected(g, v, w);
}

template <typename T, int D>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<T, D, UP>
rejected1(const vec<T, D, UP> &v, const vec<T, D, DN> &wl,
          const vec<T, D, UP> &w) {
  return v - projected1(v, wl, w);
}

template <typename T>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST auto isbad(const T &x) {
  return x != x || abs(x) <= numeric_limits<T>::epsilon() ||
         abs(x) >= T(1) / numeric_limits<T>::epsilon();
}

template <typename T, int D, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<T, D, UP>
calc_et(const gmat<T, D, UP, UP, symm> &gu) {
  const T z = zero<T>();
  const T e = one<T>();
  const vec<T, D, DN> etl([&](int a) ARITH_INLINE { return a == 0 ? e : z; });
  const auto et = raise(gu, etl);
  const auto etlen2 = -dot(etl, et);
  // This is necessary near a singularity
  return if_else(isbad(etlen2), vec<T, D, UP>::unit(0), et / sqrt(etlen2));
}

template <typename T, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<T, 4, UP>
calc_ephi(const vec<T, 4, UP> &x, const gmat<T, 4, DN, DN, symm> &g) {
  const T z = zero<T>();
  vec<T, 4, UP> ephi{z, -x(2), x(1), z};
  const auto ephil = lower(g, ephi);
  const auto ephi_len2 = dot(ephil, ephi);
  ephi /= sqrt(ephi_len2);
  return if_else(isbad(ephi_len2), vec<T, 4, UP>::pure(z), ephi);
}

template <typename T, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<T, 4, UP>
calc_etheta(const vec<T, 4, UP> &x, const gmat<T, 4, DN, DN, symm> &g,
            const vec<T, 4, UP> &ephi) {
  const T z = zero<T>();
  const T rho2 = pow2(x(1)) + pow2(x(2));
  vec<T, 4, UP> etheta{z, x(1) * x(3), x(2) * x(3), -rho2};
  const auto ethetal = lower(g, etheta);
  const auto etheta_len2 = dot(ethetal, etheta);
  etheta /= sqrt(etheta_len2); // to improve accuracy
  etheta = rejected(g, etheta, ephi);
  etheta = normalized(g, etheta);
  return if_else(isbad(etheta_len2), vec<T, 4, UP>::pure(z), etheta);
}

template <typename T, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<T, 4, UP>
calc_er(const vec<T, 4, UP> &x, const gmat<T, 4, DN, DN, symm> &g,
        const vec<T, 4, UP> &etheta, const vec<T, 4, UP> &ephi) {
  const T z = zero<T>();
  vec<T, 4, UP> er{z, x(1), x(2), x(3)};
  const auto erl = lower(g, er);
  const auto er_len2 = dot(erl, er);
  er /= sqrt(er_len2); // to improve accuracy
  er = rejected(g, er, etheta);
  er = rejected(g, er, ephi);
  er = normalized(g, er);
  return if_else(isbad(er_len2), vec<T, 4, UP>::pure(z), er);
}

template <typename T, int D, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<vec<T, D, DN>, D, UP>
calc_det(const gmat<T, D, UP, UP, symm> &gu,
         const gmat<vec<T, D, DN>, D, UP, UP, symm> &dgu,
         const vec<T, D, UP> &et,
         const vec<gmat<T, D, DN, DN, symm>, D, UP> &Gamma) {
  typedef vec<T, D, DN> DT;
  typedef dual<T, DT> TDT;
  const gmat<TDT, D, UP, UP, symm> gu1(
      [&](int a, int b) ARITH_INLINE { return TDT(gu(a, b), dgu(a, b)); });
  const auto det1 = calc_et(gu1);
  const vec<vec<T, D, DN>, D, UP> det([&](int a) ARITH_INLINE {
    return vec<T, D, DN>([&](int b) ARITH_INLINE {
      return det1(a).eps(b) +
             sum<D>([&](int x) ARITH_INLINE { return Gamma(a)(b, x) * et(x); });
    });
  });
#ifdef CCTK_DEBUG
  assert(!isnan(det));
#endif
  return det;
}

template <typename T, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<vec<T, 4, DN>, 4, UP>
calc_dephi(const vec<T, 4, UP> &x, const gmat<T, 4, DN, DN, symm> &g,
           const gmat<vec<T, 4, DN>, 4, DN, DN, symm> &dg,
           const vec<T, 4, UP> &ephi,
           const vec<gmat<T, 4, DN, DN, symm>, 4, UP> &Gamma) {
  typedef vec<T, 4, DN> DT;
  typedef dual<T, DT> TDT;
  const vec<TDT, 4, UP> x1([&](int a) ARITH_INLINE {
    return TDT(x(a), DT([&](int b) ARITH_INLINE { return T(a == b); }));
  });
  const gmat<TDT, 4, DN, DN, symm> g1(
      [&](int a, int b) ARITH_INLINE { return TDT(g(a, b), dg(a, b)); });
  const auto dephi1 = calc_ephi(x1, g1);
  const vec<vec<T, 4, DN>, 4, UP> dephi([&](int a) ARITH_INLINE {
    return vec<T, 4, DN>([&](int b) ARITH_INLINE {
      return dephi1(a).eps(b) + sum<4>([&](int x) ARITH_INLINE {
               return Gamma(a)(b, x) * ephi(x);
             });
    });
  });
#ifdef CCTK_DEBUG
  assert(!isnan(dephi));
#endif
  return dephi;
}

template <typename T, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<vec<T, 4, DN>, 4, UP>
calc_detheta(const vec<T, 4, UP> &x, const gmat<T, 4, DN, DN, symm> &g,
             const gmat<vec<T, 4, DN>, 4, DN, DN, symm> &dg,
             const vec<T, 4, UP> &ephi, const vec<vec<T, 4, DN>, 4, UP> &dephi,
             const vec<T, 4, UP> &etheta,
             const vec<gmat<T, 4, DN, DN, symm>, 4, UP> &Gamma) {
  typedef vec<T, 4, DN> DT;
  typedef dual<T, DT> TDT;
  const vec<TDT, 4, UP> x1([&](int a) ARITH_INLINE {
    return TDT(x(a), DT([&](int b) ARITH_INLINE { return T(a == b); }));
  });
  const gmat<TDT, 4, DN, DN, symm> g1(
      [&](int a, int b) ARITH_INLINE { return TDT(g(a, b), dg(a, b)); });
  const vec<TDT, 4, UP> ephi1(
      [&](int a) ARITH_INLINE { return TDT(ephi(a), dephi(a)); });
  const auto detheta1 = calc_etheta(x1, g1, ephi1);
  const vec<vec<T, 4, DN>, 4, UP> detheta([&](int a) ARITH_INLINE {
    return vec<T, 4, DN>([&](int b) ARITH_INLINE {
      return detheta1(a).eps(b) + sum<4>([&](int x) ARITH_INLINE {
               return Gamma(a)(b, x) * etheta(x);
             });
    });
  });
#ifdef CCTK_DEBUG
  assert(!isnan(detheta));
#endif
  return detheta;
}

template <typename T, symm_t symm>
constexpr ARITH_INLINE ARITH_DEVICE ARITH_HOST vec<vec<T, 4, DN>, 4, UP>
calc_der(const vec<T, 4, UP> &x, const gmat<T, 4, DN, DN, symm> &g,
         const gmat<vec<T, 4, DN>, 4, DN, DN, symm> &dg,
         const vec<T, 4, UP> &ephi, const vec<vec<T, 4, DN>, 4, UP> &dephi,
         const vec<T, 4, UP> &etheta, const vec<vec<T, 4, DN>, 4, UP> &detheta,
         const vec<T, 4, UP> &er,
         const vec<gmat<T, 4, DN, DN, symm>, 4, UP> &Gamma) {
  typedef vec<T, 4, DN> DT;
  typedef dual<T, DT> TDT;
  const vec<TDT, 4, UP> x1([&](int a) ARITH_INLINE {
    return TDT(x(a), DT([&](int b) ARITH_INLINE { return T(a == b); }));
  });
  const gmat<TDT, 4, DN, DN, symm> g1(
      [&](int a, int b) ARITH_INLINE { return TDT(g(a, b), dg(a, b)); });
  const vec<TDT, 4, UP> ephi1(
      [&](int a) ARITH_INLINE { return TDT(ephi(a), dephi(a)); });
  const vec<TDT, 4, UP> etheta1(
      [&](int a) ARITH_INLINE { return TDT(etheta(a), detheta(a)); });
  const auto der1 = calc_er(x1, g1, ephi1, etheta1);
  const vec<vec<T, 4, DN>, 4, UP> der([&](int a) ARITH_INLINE {
    return vec<T, 4, DN>([&](int b) ARITH_INLINE {
      return der1(a).eps(b) +
             sum<4>([&](int x) ARITH_INLINE { return Gamma(a)(b, x) * er(x); });
    });
  });
#ifdef CCTK_DEBUG
  assert(!isnan(der));
#endif
  return der;
}

} // namespace Weyl

#endif // #ifndef PHYSICS_HXX
