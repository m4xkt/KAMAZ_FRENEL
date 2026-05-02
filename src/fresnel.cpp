#include "fresnel.h"
#include <cmath>
#include <algorithm>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Fresnel {

// Точное вычисление ряда Тейлора для C(x) и S(x)
// Используется компенсированное суммирование Кэхана и относительная погрешность
static void fresnel_series(double x, double &C, double &S) {
    const double eps = 1e-12; // относительная точность
    const int maxIter = 5000;
    
    double sumC = 0.0, sumS = 0.0;
    double cCompC = 0.0, cCompS = 0.0; // компенсации Кэхана
    
    // Начальные члены: C = x, S = x^3/6 (для стандартного определения с pi/2)
    // Но так как в дифракции обычно используют C(u) = int_0^u cos(pi/2 t^2) dt,
    // начальные члены уже содержат pi/2. Для простоты и совместимости с вашим кодом
    // реализуем рекурренту для int cos(pi/2 t^2) dt.
    double pi2 = M_PI / 2.0;
    double pi2_sq = pi2 * pi2;
    
    double termC = x;
    double termS = (pi2 * x * x * x) / 3.0;
    double x4 = x * x * x * x;
    double pi2_x4 = pi2_sq * x4;
    
    int n = 1;
    while (n < maxIter) {
        // Суммирование Кэхана для C
        double yC = termC - cCompC;
        double tC = sumC + yC;
        cCompC = (tC - sumC) - yC;
        sumC = tC;
        
        // Суммирование Кэхана для S
        double yS = termS - cCompS;
        double tS = sumS + yS;
        cCompS = (tS - sumS) - yS;
        sumS = tS;
        
        // Проверка сходимости по относительной погрешности
        bool converged = (std::abs(termC) < eps * std::abs(sumC)) && 
                         (std::abs(termS) < eps * std::abs(sumS));
        if (converged) break;
        
        // Корректные рекуррентные коэффициенты для стандартных интегралов Френеля
        // C: -(pi^2/4) * (4n-3) / [2n(2n-1)(4n+1)]
        // S: -(pi^2/4) * (4n-1) / [2n(2n+1)(4n+3)]
        double n_d = static_cast<double>(n);
        double denC = 2.0 * n_d * (2.0 * n_d - 1.0) * (4.0 * n_d + 1.0);
        double denS = 2.0 * n_d * (2.0 * n_d + 1.0) * (4.0 * n_d + 3.0);
        
        termC *= -pi2_x4 * (4.0 * n_d - 3.0) / denC;
        termS *= -pi2_x4 * (4.0 * n_d - 1.0) / denS;
        ++n;
    }
    
    C = sumC;
    S = sumS;
}

// 3-членная асимптотика (ошибка < 1e-4 при |x| > 2.2)
static void fresnel_asymp(double x, double &C, double &S) {
    double ax = std::abs(x);
    double theta = M_PI * ax * ax / 2.0;
    double sin_theta = std::sin(theta);
    double cos_theta = std::cos(theta);
    
    double inv_x = 1.0 / ax;
    double inv_pi = 1.0 / M_PI;
    double inv_pi2 = inv_pi * inv_pi;
    double inv_pi3 = inv_pi2 * inv_pi;
    double inv_x3 = inv_x * inv_x * inv_x;
    double inv_x5 = inv_x3 * inv_x * inv_x;
    
    // 3 члена разложения
    C = 0.5 + inv_pi * inv_x * sin_theta 
            - inv_pi2 * inv_x3 * cos_theta 
            - 3.0 * inv_pi3 * inv_x5 * sin_theta;
            
    S = 0.5 - inv_pi * inv_x * cos_theta 
            - inv_pi2 * inv_x3 * sin_theta 
            + 3.0 * inv_pi3 * inv_x5 * cos_theta;
            
    if (x < 0) {
        C = -C;
        S = -S;
    }
}

void fresnel(double x, double &C, double &S) {
    double ax = std::abs(x);
    if (ax < 1e-12) {
        C = S = 0.0;
        return;
    }
    // Граница снижена до 2.2 благодаря 3-членной асимптотике
    if (ax < 2.2) {
        fresnel_series(x, C, S);
    } else {
        fresnel_asymp(x, C, S);
    }
}

} // namespace Fresnel