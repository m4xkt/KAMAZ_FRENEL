// ImageRenderer.h
#ifndef IMAGE_RENDERER_H
#define IMAGE_RENDERER_H

#include <opencv2/opencv.hpp>
#include <vector>

class ImageRenderer {
public:
    ImageRenderer(int width = 1400, int height = 900);
    
    void setImageSize(int width, int height);
    void setIntensityScale(double scale);
    void setZoom(double zoom);
    void setResolution(double sigma_m);
    void setColorFromWavelength(double lambda_nm);   // новый метод
    
    double getZoom() const { return m_zoom; }
    
    cv::Mat renderProfile(const std::vector<double>& profile, 
                         double xRange_m,
                         double slitWidth_m,
                         double distance_m,
                         double lambda_nm);   // добавили параметр lambda
    
private:
    void applyEyepieceOverlay(cv::Mat &img, double xRange_m);
    void drawParameterText(cv::Mat &img, double slitWidth_m, 
                          double distance_m, double zoom, double lambda_nm);
    
    int m_imgWidth;
    int m_imgHeight;
    double m_intensityScale;
    double m_zoom;
    double m_resolution;
    cv::Scalar m_colorBGR;   // цвет в формате BGR
};

#endif