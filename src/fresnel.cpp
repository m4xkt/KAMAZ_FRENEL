#include "fresnel.h"
#include <cmath>
#include <complex>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Fresnel {
    using Complex = std::complex<double>;
    
    // точность вычислений
    constexpr double TOL = 1e-12;

    //ряд тейлора
    void fresnel_taylor(double x, double &C, double &S) {
        Complex sum(0.0, 0.0);
        Complex term = x; // Первый член ряда: k=0
        Complex i_pi(0.0, M_PI);
        double x_sq = x * x;
        
        int k = 0;
        while (std::abs(term) > TOL && k < 100) {
            sum += term;
            k++;
            // Рекуррентная формула: T_k = T_{k-1} * (i*pi*x^2) / (2k*(2k+1))
            term *= i_pi * x_sq / (2.0 * k * (2.0 * k + 1.0));
        }
        
        C = sum.real();
        S = sum.imag();
    }

    //правило трапеций
    void fresnel_trapezoid(double x, double &C, double &S) {
        // Согласно статье, N2=12 обеспечивает точность лучше 1e-16
        const int N = 12;
        double A_N = std::sqrt(N + 0.5);
        double pi_A_N = M_PI * A_N;
        double inv_A2 = 1.0 / (A_N * A_N);
        
        Complex sum_val(0.0, 0.0);
        for (int k = 1; k <= N; ++k) {
            double k_half = k - 0.5;
            double exp_arg = -M_PI * k_half * k_half * inv_A2;
            // Знаменатель: x^2 + i*2*(k-1/2)^2*A_N^{-2}
            Complex denom(x * x, 2.0 * k_half * k_half * inv_A2);
            sum_val += std::exp(exp_arg) / denom;
        }
        
        Complex i(0.0, 1.0);
        
        // Первое слагаемое: (1+i)/2 - (1+i)/(exp((1-i)*pi*A_N*x) + 1)
        // exp((1-i)*pi*A_N*x) = exp(pi*A_N*x) * exp(-i*pi*A_N*x)
        Complex exp_arg1(pi_A_N * x, -pi_A_N * x);
        Complex exp_term1 = std::exp(exp_arg1);
        Complex term1 = (1.0 + i) / (exp_term1 + 1.0);
        
        // Второе слагаемое: (2ix*exp(i*pi*x^2/2)/(pi*A_N)) * sum_val
        Complex exp_arg2(0.0, M_PI * x * x / 2.0);
        Complex exp_term2 = std::exp(exp_arg2);
        Complex term2 = (2.0 * i * x * exp_term2 / (M_PI * A_N)) * sum_val;
        
        // Итоговая формула (6)-(7)
        Complex G = (1.0 + i) / 2.0 - term1 - term2;
        
        C = G.real();
        S = G.imag();
    }

    //асимптотическое разложение
    void fresnel_asymp(double x, double &C, double &S) {
        Complex sum_val(0.0, 0.0);
        double theta = M_PI * x * x / 2.0;
        Complex exp_theta(0.0, theta);
        exp_theta = std::exp(exp_theta);
        
        double inv_pi_x = 1.0 / (M_PI * x);
        
        // k=0: член = -i/(pi*x)
        Complex term(0.0, -inv_pi_x);
        int k = 0;
        
        while (std::abs(term) > TOL && k < 50) {
            sum_val += term;
            k++;
            // Рекуррентная формула: T_k = T_{k-1} * (2k-1)*(-i) / (pi*x^2)
            term *= (2.0 * k - 1.0) * Complex(0.0, -1.0) * inv_pi_x / x;
        }
        
        // Формула (11): Q_N(x) = (1+i)/2 + exp(i*pi*x^2/2) * sum
        Complex G = Complex(0.5, 0.5) + exp_theta * sum_val;
        
        C = G.real();
        S = G.imag();
    }

    void fresnel(double x, double &C, double &S) {
        double ax = std::abs(x);
        
        if (ax < 1e-15) {
            C = S = 0.0;
            return;
        }
        
        //выбор метода
        if (ax <= 6.0) {
            fresnel_trapezoid(ax, C, S);
        } else {
            fresnel_asymp(ax, C, S);
        }
        
        // Интегралы Френеля - нечётные функции: G(-x) = -G(x)
        if (x < 0) {
            C = -C;
            S = -S;
        }
    }
} // namespace Fresnel