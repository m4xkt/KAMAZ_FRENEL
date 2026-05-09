// ImageRenderer.cpp
#include "ImageRender.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

ImageRenderer::ImageRenderer(int width, int height)
    : m_imgWidth(width), m_imgHeight(height), 
      m_intensityScale(1.0), m_zoom(1.0), m_resolution(5e-6)
{
}

void ImageRenderer::setImageSize(int width, int height) { 
    m_imgWidth = width; m_imgHeight = height; 
}
void ImageRenderer::setIntensityScale(double scale) { 
    m_intensityScale = std::max(0.1, std::min(10.0, scale)); 
}
void ImageRenderer::setZoom(double zoom) { 
    m_zoom = std::max(0.1, std::min(10.0, zoom)); 
}
void ImageRenderer::setResolution(double sigma_m) { 
    m_resolution = std::max(1e-6, sigma_m); 
}

cv::Mat ImageRenderer::renderProfile(const std::vector<double>& profile,
                                     double xRange_m,
                                     double slitWidth_m,
                                     double distance_m)
{
    cv::Mat img(m_imgHeight, m_imgWidth, CV_8UC3, cv::Scalar(30, 30, 30));
    if (profile.empty()) return img;
    
    int profilePoints = static_cast<int>(profile.size());
    
    // Интерполяция профиля на ширину изображения
    cv::Mat row(1, profilePoints, CV_64FC1);
    for (int i = 0; i < profilePoints; ++i)
        row.at<double>(0, i) = profile[i];
    
    cv::Mat stretchedRow;
    cv::resize(row, stretchedRow, cv::Size(m_imgWidth, 1), 0, 0, cv::INTER_LINEAR);
    
    // Отрисовка дифракционной картины (зелёный канал)
    for (int col = 0; col < m_imgWidth; ++col) {
        double I = stretchedRow.at<double>(0, col) * m_intensityScale;
        uchar val = static_cast<uchar>(std::min(255.0, I * 255.0));
        cv::Vec3b color(0, val, 0);
        for (int rowIdx = 0; rowIdx < m_imgHeight; ++rowIdx)
            img.at<cv::Vec3b>(rowIdx, col) = color;
    }
    
    applyEyepieceOverlay(img, xRange_m);
    drawParameterText(img, slitWidth_m, distance_m, m_zoom);
    
    return img;
}

void ImageRenderer::applyEyepieceOverlay(cv::Mat &img, double xRange_m) {
    int cx = img.cols / 2, cy = img.rows / 2;
    int radius = static_cast<int>(std::min(cx, cy) * 0.85);
    
    // Перекрестие
    cv::line(img, cv::Point(cx - 25, cy), cv::Point(cx + 25, cy), 
             cv::Scalar(255,255,255), 1, cv::LINE_AA);
    cv::line(img, cv::Point(cx, cy - 25), cv::Point(cx, cy + 25), 
             cv::Scalar(255,255,255), 1, cv::LINE_AA);
    
    // Шкала масштаба
    int scaleY = cy + 40;
    int scaleLen = static_cast<int>(radius * 0.7);
    cv::line(img, cv::Point(cx - scaleLen, scaleY), 
             cv::Point(cx + scaleLen, scaleY), 
             cv::Scalar(200,200,200), 1, cv::LINE_AA);
    
    double mmPerPixel = xRange_m / img.cols;
    double tickStep_m = 0.0005;
    int tickStep_pix = static_cast<int>(tickStep_m / mmPerPixel);
    
    if (tickStep_pix > 5) {
        for (int x = cx - scaleLen; x <= cx + scaleLen; x += tickStep_pix)
            cv::line(img, cv::Point(x, scaleY - 5), 
                     cv::Point(x, scaleY + 5), 
                     cv::Scalar(200,200,200), 1, cv::LINE_AA);
    }
    cv::putText(img, "0.5 mm", cv::Point(cx + scaleLen - 40, scaleY - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(200,200,200), 1);
}

void ImageRenderer::drawParameterText(cv::Mat &img, double slitWidth_m, 
                                      double distance_m, double zoom)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "b = %.3f mm", slitWidth_m * 1000);
    cv::putText(img, buf, cv::Point(20, 30), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255,255,255), 1);
    
    snprintf(buf, sizeof(buf), "z = %.1f cm", distance_m * 100);
    cv::putText(img, buf, cv::Point(20, 55), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255,255,255), 1);
    
    snprintf(buf, sizeof(buf), "zoom = %.2fx", zoom);
    cv::putText(img, buf, cv::Point(20, 80), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255,255,255), 1);
    
    cv::putText(img, "Fresnel diffraction", cv::Point(20, img.rows-20),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(200,200,200), 1);
}