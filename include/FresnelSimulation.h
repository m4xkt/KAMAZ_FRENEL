#ifndef FRESNEL_SIMULATION_H
#define FRESNEL_SIMULATION_H

#include <opencv2/opencv.hpp>
#include <vector>

class FresnelSimulation {
public:
    FresnelSimulation(double lambda = 515e-9, double deltaLambda = 36e-9);

    void setSlitWidth(double width_m);
    void setDistance(double z_m);
    void setImageSize(int width, int height);
    void setIntensityScale(double scale);
    void setZoom(double zoom);
    void setResolution(double sigma_um);   // ширина аппаратной функции в мкм

    double getSlitWidth() const { return m_b; }
    double getDistance() const { return m_z; }

    std::vector<double> computeProfile(int numPoints, double xRange_m);
    cv::Mat generateImage();

private:
    double m_lambda;           // центральная длина волны
    double m_dLambda;          // ширина спектра (FWHM)
    double m_b;                // ширина щели
    double m_z;                // расстояние
    int m_imgWidth;
    int m_imgHeight;
    double m_intensityScale;
    double m_zoom;
    double m_resolution;       // σ аппаратной функции [м]

    // Вспомогательные методы
    std::vector<double> computeMonochromaticProfile(int numPoints, double xRange_m, double lambda);
    void applyEyepieceOverlay(cv::Mat &img, double xRange_m);
};

#endif