#ifndef FRESNEL_SIMULATION_H
#define FRESNEL_SIMULATION_H

#include <opencv2/opencv.hpp>
#include "DifrCalc.h"
#include "ImageRender.h"

class FresnelSimulation {
public:
    FresnelSimulation(double lambda = 515e-9, double deltaLambda = 36e-9);
    
    // Публичный интерфейс для UI
    void setSlitWidth(double width_m);
    void setDistance(double z_m);
    void setImageSize(int width, int height);
    void setIntensityScale(double scale);
    void setZoom(double zoom);
    void setResolution(double sigma_um);
    void setWavelength(double lambda_m);
    double getWavelength() const { return m_lambda; }
    
    double getSlitWidth() const { return m_slitWidth; }
    double getDistance() const { return m_distance; }
    double getZoom() const { return m_zoom; }
    
    // Основной метод
    cv::Mat generateImage();
    
private:
    // Конфигурация (единый источник истины)
    double m_lambda, m_dLambda;
    double m_slitWidth, m_distance;
    int m_imgWidth, m_imgHeight;
    double m_intensityScale, m_zoom, m_resolution;
    
    // Компоненты
    DiffractionCalculator m_calculator;
    ImageRenderer m_renderer;
    
    // Вспомогательные методы
    void syncConfigToComponents();
    double computeAutoRange();
};

#endif // FRESNEL_SIMULATION_H