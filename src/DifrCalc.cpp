// DiffractionCalculator.cpp
#include "DifrCalc.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DiffractionCalculator::DiffractionCalculator(double lambda, double deltaLambda)
    : m_lambda(lambda), m_dLambda(deltaLambda), m_b(0.5e-3), m_z(0.15)
{
}

void DiffractionCalculator::setWavelength(double lambda_m) { 
    m_lambda = std::max(1e-9, lambda_m); 
}
void DiffractionCalculator::setSpectralWidth(double deltaLambda_m) { 
    m_dLambda = std::max(0.0, deltaLambda_m); 
}
void DiffractionCalculator::setSlitWidth(double width_m) { 
    m_b = std::max(1e-6, width_m); 
}
void DiffractionCalculator::setDistance(double z_m) { 
    m_z = std::max(0.001, z_m); 
}

std::vector<double> DiffractionCalculator::computeMonochromaticProfile(
    int numPoints, double xRange_m, double lambda) 
{
    std::vector<double> intensity(numPoints, 0.0);
    if (numPoints < 2) return intensity;
    
    double factor = std::sqrt(2.0 / (lambda * m_z));
    double halfWidth = m_b / 2.0;
    
    for (int i = 0; i < numPoints; ++i) {
        double x = -xRange_m/2.0 + i * xRange_m / (numPoints - 1);
        double u1 = factor * (x - halfWidth);
        double u2 = factor * (x + halfWidth);
        
        double C1, S1, C2, S2;
        Fresnel::fresnel(u1, C1, S1);
        Fresnel::fresnel(u2, C2, S2);
        
        double dC = C2 - C1;
        double dS = S2 - S1;
        intensity[i] = 0.5 * (dC*dC + dS*dS);   // нормировка
    }
    return intensity;
}

std::vector<double> DiffractionCalculator::computeProfile(int numPoints, double xRange_m) {
    auto profile = computePolychromaticProfile(numPoints, xRange_m);
    return profile;
}
std::vector<double> DiffractionCalculator::computePolychromaticProfile(
    int numPoints, double xRange_m)
{
    std::vector<double> total(numPoints, 0.0);
    if (m_dLambda <= 0.0 || m_spectralPoints < 2) {
        // Монохроматический случай
        return computeMonochromaticProfile(numPoints, xRange_m, m_lambda);
    }

    double sigma = m_dLambda / (2.0 * std::sqrt(2.0 * std::log(2.0))); // FWHM -> sigma
    double lambdaMin = m_lambda - 3.0 * sigma;
    double lambdaMax = m_lambda + 3.0 * sigma;
    double dLambdaStep = (lambdaMax - lambdaMin) / (m_spectralPoints - 1);

    double weightSum = 0.0;
    for (int i = 0; i < m_spectralPoints; ++i) {
        double lambda = lambdaMin + i * dLambdaStep;
        if (lambda <= 0.0) continue;
        double weight = std::exp(-0.5 * std::pow((lambda - m_lambda) / sigma, 2));
        weightSum += weight;

        auto monoProfile = computeMonochromaticProfile(numPoints, xRange_m, lambda);
        for (int j = 0; j < numPoints; ++j) {
            total[j] += weight * monoProfile[j];
        }
    }

    if (weightSum > 1e-12) {
        for (auto& v : total) v /= weightSum;
    }
    return total;
}