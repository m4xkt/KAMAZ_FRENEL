#include "fresnel.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Fresnel {

static void fresnel_series(double x, double &C, double &S) {
    const double eps = 1e-10;
    const int maxIter = 5000;
    double sumC = 0.0, sumS = 0.0;
    double termC = x;
    double termS = x*x*x/3.0;
    double x2 = x*x;
    double x4 = x2*x2;
    int n = 1;
    while ((std::abs(termC) > eps || std::abs(termS) > eps) && n < maxIter) {
        sumC += termC;
        sumS += termS;
        double coeffC = - (4.0*n - 3.0) / (2.0 * n * (4.0*n + 1.0));
        double coeffS = - (4.0*n - 1.0) / ((2.0*n + 1.0) * (4.0*n + 3.0));
        termC *= coeffC * x4;
        termS *= coeffS * x4;
        ++n;
    }
    C = sumC;
    S = sumS;
}

static void fresnel_asymp(double x, double &C, double &S) {
    double ax = std::abs(x);
    double p = M_PI * ax * ax / 2.0;
    double sinp = std::sin(p);
    double cosp = std::cos(p);
    double inv_pi_x = 1.0 / (M_PI * ax);
    double inv_pi2_x3 = 1.0 / (M_PI * M_PI * ax * ax * ax);
    double f = inv_pi_x;
    double g = inv_pi2_x3;
    C = 0.5 + f * sinp - g * cosp;
    S = 0.5 - f * cosp - g * sinp;
    if (x < 0) {
        C = -C;
        S = -S;
    }
}

void fresnel(double x, double &C, double &S) {
    double ax = std::abs(x);
    if (ax < 1e-9) {
        C = S = 0.0;
        return;
    }
    if (ax < 4.0) {
        fresnel_series(x, C, S);
    } else {
        fresnel_asymp(x, C, S);
    }
}

} // namespace Fresnel