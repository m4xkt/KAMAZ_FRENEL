// FresnelSimulation.cpp
#include "FresnelSimulation.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FresnelSimulation::FresnelSimulation(double lambda, double deltaLambda)
    : m_lambda(lambda), m_dLambda(deltaLambda),
      m_slitWidth(0.5e-3), m_distance(0.15),
      m_imgWidth(1400), m_imgHeight(900),
      m_intensityScale(1.0), m_zoom(1.0), m_resolution(5e-6),
      m_calculator(lambda, deltaLambda),
      m_renderer(1400, 900)
{
    syncConfigToComponents();
}

void FresnelSimulation::syncConfigToComponents() {
    m_calculator.setWavelength(m_lambda);
    m_calculator.setSpectralWidth(m_dLambda);
    m_calculator.setSlitWidth(m_slitWidth);
    m_calculator.setDistance(m_distance);
    
    m_renderer.setImageSize(m_imgWidth, m_imgHeight);
    m_renderer.setIntensityScale(m_intensityScale);
    m_renderer.setZoom(m_zoom);
    m_renderer.setResolution(m_resolution);

}

// Сеттеры обновляют состояние и синхронизируют компоненты
void FresnelSimulation::setSlitWidth(double width_m) { 
    m_slitWidth = std::max(1e-6, width_m); 
    m_calculator.setSlitWidth(m_slitWidth); 
}
void FresnelSimulation::setDistance(double z_m) { 
    m_distance = std::max(0.001, z_m); 
    m_calculator.setDistance(m_distance); 
}
void FresnelSimulation::setImageSize(int width, int height) { 
    m_imgWidth = width; m_imgHeight = height;
    m_renderer.setImageSize(width, height);
}
void FresnelSimulation::setIntensityScale(double scale) { 
    m_intensityScale = std::max(0.1, std::min(10.0, scale));
    m_renderer.setIntensityScale(m_intensityScale);
}
void FresnelSimulation::setZoom(double zoom) { 
    m_zoom = std::max(0.1, std::min(10.0, zoom));
    m_renderer.setZoom(m_zoom);
}
void FresnelSimulation::setResolution(double sigma_um) { 
    m_resolution = std::max(1e-6, sigma_um * 1e-6);
    m_renderer.setResolution(m_resolution);
}

double FresnelSimulation::computeAutoRange() {
    double fringeWidth = std::sqrt(m_lambda * m_distance);
    double autoRange = 2.0 * 8.0 * fringeWidth;
    if (m_slitWidth > autoRange) autoRange = m_slitWidth * 1.5;
    autoRange = std::max(0.0005, std::min(0.02, autoRange));
    return autoRange;
}

cv::Mat FresnelSimulation::generateImage() {
    const int profilePoints = 8000;
    
    double autoRange = computeAutoRange();
    double xRange = autoRange / m_zoom;
    xRange = std::max(0.0001, std::min(0.05, xRange));
    
    auto profile = m_calculator.computeProfile(profilePoints, xRange);
    return m_renderer.renderProfile(profile, xRange, m_slitWidth, m_distance);
}