// ImageRenderer.h
#ifndef IMAGE_RENDERER_H
#define IMAGE_RENDERER_H

#include <opencv2/opencv.hpp>
#include <vector>

class ImageRenderer {
public:
    ImageRenderer(int width = 1400, int height = 900);
    
    // Параметры отображения
    void setImageSize(int width, int height);
    void setIntensityScale(double scale);
    void setZoom(double zoom);
    void setResolution(double sigma_m);
    
    // Геттеры для координатора
    double getZoom() const { return m_zoom; }

    // Основной метод рендеринга
    cv::Mat renderProfile(const std::vector<double>& profile, 
                         double xRange_m,
                         double slitWidth_m,
                         double distance_m);
    
private:
    void applyEyepieceOverlay(cv::Mat &img, double xRange_m);
    void drawParameterText(cv::Mat &img, double slitWidth_m, 
                          double distance_m, double zoom);
    void drawIntensityGraph(cv::Mat &img, const cv::Mat& intensityRow, 
                           double xRange_m);   // <-- новый метод

    int m_imgWidth;
    int m_imgHeight;
    double m_intensityScale;
    double m_zoom;
    double m_resolution;
};

#endif // IMAGE_RENDERER_H