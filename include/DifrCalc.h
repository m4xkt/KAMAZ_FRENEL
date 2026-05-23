// DiffractionCalculator.h
#ifndef DIFFR_CALC_H
#define DIFFR_CALC_H

#include <vector>
#include "fresnel.h"
#include <string>

class DiffractionCalculator {
public:
    DiffractionCalculator(double lambda = 515e-9, double deltaLambda = 36e-9);
    
    // Параметры физической модели
    void setWavelength(double lambda_m);
    void setSpectralWidth(double deltaLambda_m);
    void setSlitWidth(double width_m);
    void setDistance(double z_m);
    void saveProfileToFile(const std::string& filename, int numPoints, double xRange_m);

    // Геттеры
    double getWavelength() const { return m_lambda; }
    double getSpectralWidth() const { return m_dLambda; }
    double getSlitWidth() const { return m_b; }
    double getDistance() const { return m_z; }
    
    // Основной метод расчёта
    std::vector<double> computeProfile(int numPoints, double xRange_m);
    
private:
    std::vector<double> computeMonochromaticProfile(int numPoints, double xRange_m, double lambda);
    
    double m_lambda;      // центральная длина волны [м]
    double m_dLambda;     // ширина спектра (FWHM) [м]
    double m_b;           // ширина щели [м]
    double m_z;           // расстояние до экрана [м]
};

#endif 