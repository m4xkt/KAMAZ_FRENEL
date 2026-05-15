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
    
    cv::Mat row(1, profilePoints, CV_64FC1);
    for (int i = 0; i < profilePoints; ++i)
        row.at<double>(0, i) = profile[i];
    
    cv::Mat stretchedRow;
    cv::resize(row, stretchedRow, cv::Size(m_imgWidth, 1), 0, 0, cv::INTER_LINEAR);
    
    // Дифракционная картина (зелёный канал)
    for (int col = 0; col < m_imgWidth; ++col) {
        double I = stretchedRow.at<double>(0, col) * m_intensityScale;
        uchar val = static_cast<uchar>(std::min(255.0, I * 255.0));
        cv::Vec3b color(0, val, 0);
        for (int rowIdx = 0; rowIdx < m_imgHeight; ++rowIdx)
            img.at<cv::Vec3b>(rowIdx, col) = color;
    }
    
    // 1) График интенсивности (рисуется в нижней части)
    drawIntensityGraph(img, stretchedRow, xRange_m);
    
    // 2) Накладываем перекрестие и шкалу
    applyEyepieceOverlay(img, xRange_m);
    
    // 3) Выводим параметры
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
    
    // Подпись перенесена выше, чтобы не заходить на панель графика
    cv::putText(img, "Fresnel diffraction", cv::Point(20, 105),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(200,200,200), 1);
}

// ---------- Новый метод: отрисовка графика интенсивности ----------
void ImageRenderer::drawIntensityGraph(cv::Mat &img, const cv::Mat& intensityRow, double xRange_m)
{
    int graphHeight = std::min(250, m_imgHeight / 4);
    if (graphHeight < 100) graphHeight = 100;
    int y0 = m_imgHeight - graphHeight;
    int y1 = m_imgHeight - 1;
    
    // Фон панели
    cv::rectangle(img, cv::Rect(0, y0, m_imgWidth, graphHeight),
                  cv::Scalar(40, 40, 40), cv::FILLED);
    cv::rectangle(img, cv::Rect(0, y0, m_imgWidth, graphHeight),
                  cv::Scalar(80, 80, 80), 1);
    
    const int marginTop = 25;
    const int marginBottom = 30;
    const int marginLeft = 60;
    const int marginRight = 20;
    
    int y_axis = y1 - marginBottom;   // линия нулевой интенсивности
    int y_top  = y0 + marginTop;      // теперь будет соответствовать I_max_display
    
    // ---------- Автоматическое масштабирование по Y ----------
    double maxI = 0.0;
    for (int col = marginLeft; col <= m_imgWidth - marginRight; ++col) {
        double val = intensityRow.at<double>(0, col);
        if (val > maxI) maxI = val;
    }
    if (maxI < 1e-9) maxI = 1.0;                     // страховка от пустого профиля
    double I_max_display = maxI * 1.1;               // запас 10%
    // ---------------------------------------------------------
    
    // Ось Y (вертикальная)
    cv::line(img, cv::Point(marginLeft, y_top), cv::Point(marginLeft, y_axis),
             cv::Scalar(200,200,200), 1, cv::LINE_AA);
    // Ось X (горизонтальная)
    cv::line(img, cv::Point(marginLeft, y_axis), cv::Point(m_imgWidth - marginRight, y_axis),
             cv::Scalar(200,200,200), 1, cv::LINE_AA);
    
    // Деления оси Y: выбираем "красивый" шаг
    double rawDy = I_max_display / 5.0;               // примерно 5 делений
    double expY = std::floor(std::log10(rawDy));
    double mantissaY = rawDy / std::pow(10.0, expY);
    double niceMantissaY = 1.0;
    if (mantissaY >= 7.5) niceMantissaY = 10.0;
    else if (mantissaY >= 3.5) niceMantissaY = 5.0;
    else if (mantissaY >= 1.5) niceMantissaY = 2.0;
    double dy = niceMantissaY * std::pow(10.0, expY);
    
    for (double val = 0.0; val <= I_max_display + dy*0.1; val += dy) {
        int y = y_axis - static_cast<int>((val / I_max_display) * (y_axis - y_top));
        cv::line(img, cv::Point(marginLeft - 5, y), cv::Point(marginLeft, y),
                 cv::Scalar(200,200,200), 1, cv::LINE_AA);
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2g", val);
        cv::putText(img, buf, cv::Point(marginLeft - 45, y + 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.35, cv::Scalar(200,200,200), 1);
    }
    cv::putText(img, "I", cv::Point(marginLeft - 20, y_top - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(200,200,200), 1);
    
    // Деления оси X (без изменений)
    double halfRange = xRange_m / 2.0;
    double rawDx = 2.0 * halfRange / 6.0;
    double expX = std::floor(std::log10(rawDx));
    double mantissaX = rawDx / std::pow(10.0, expX);
    double niceMantissaX = 1.0;
    if (mantissaX >= 7.5) niceMantissaX = 10.0;
    else if (mantissaX >= 3.5) niceMantissaX = 5.0;
    else if (mantissaX >= 1.5) niceMantissaX = 2.0;
    double dx_m = niceMantissaX * std::pow(10.0, expX);
    
    for (double x = -halfRange; x <= halfRange + dx_m * 0.1; x += dx_m) {
        double col_d = (x / xRange_m + 0.5) * m_imgWidth;
        int col = static_cast<int>(col_d + 0.5);
        if (col >= marginLeft && col <= m_imgWidth - marginRight) {
            cv::line(img, cv::Point(col, y_axis - 3), cv::Point(col, y_axis + 3),
                     cv::Scalar(200,200,200), 1, cv::LINE_AA);
            double x_mm = x * 1000.0;
            char buf[32];
            snprintf(buf, sizeof(buf), "%.2f", x_mm);
            cv::Size textSize = cv::getTextSize(buf, cv::FONT_HERSHEY_SIMPLEX, 0.3, 1, nullptr);
            cv::putText(img, buf, cv::Point(col - textSize.width/2, y_axis + 15),
                        cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(200,200,200), 1);
        }
    }
    // Метка «0» в центре
    int col0 = m_imgWidth / 2;
    if (col0 >= marginLeft && col0 <= m_imgWidth - marginRight) {
        cv::line(img, cv::Point(col0, y_axis - 5), cv::Point(col0, y_axis + 5),
                 cv::Scalar(255,255,255), 2, cv::LINE_AA);
        cv::putText(img, "0", cv::Point(col0 - 10, y_axis + 20),
                    cv::FONT_HERSHEY_SIMPLEX, 0.35, cv::Scalar(255,255,255), 1);
    }
    cv::putText(img, "x, mm", cv::Point(m_imgWidth - marginRight - 60, y_axis + 20),
                cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(200,200,200), 1);
    
    // Кривая интенсивности (масштабируется на I_max_display)
    if (intensityRow.cols != m_imgWidth) return;
    std::vector<cv::Point> pts;
    for (int col = marginLeft; col <= m_imgWidth - marginRight; ++col) {
        double I = intensityRow.at<double>(0, col);
        if (I < 0.0) I = 0.0;
        if (I > I_max_display) I = I_max_display;   // на случай выбросов
        int y = y_axis - static_cast<int>((I / I_max_display) * (y_axis - y_top));
        pts.push_back(cv::Point(col, y));
    }
    if (pts.size() > 1) {
        cv::polylines(img, pts, false, cv::Scalar(0, 255, 255), 2, cv::LINE_AA);
    }
}