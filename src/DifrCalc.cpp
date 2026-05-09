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
        intensity[i] = dC*dC + dS*dS;
    }
    return intensity;
}

std::vector<double> DiffractionCalculator::computeProfile(int numPoints, double xRange_m) {
    auto profile = computeMonochromaticProfile(numPoints, xRange_m, m_lambda);
    
    // Нормализация к [0, 1]
    double maxVal = *std::max_element(profile.begin(), profile.end());
    if (maxVal > 1e-12) {
        for (auto& v : profile) v /= maxVal;
    }
    return profile;
}