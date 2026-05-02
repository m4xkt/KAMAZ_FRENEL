#include "FresnelSimulation.h"
#include "fresnel.h"
#include <cmath>
#include <algorithm>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FresnelSimulation::FresnelSimulation(double lambda, double deltaLambda)
    : m_lambda(lambda), m_dLambda(deltaLambda), m_b(0.5e-3), m_z(0.15),
      m_imgWidth(1400), m_imgHeight(900), m_intensityScale(1.0), m_zoom(1.0),
      m_resolution(5e-6)
{
}

void FresnelSimulation::setSlitWidth(double width_m) { m_b = std::max(1e-6, width_m); }
void FresnelSimulation::setDistance(double z_m) { m_z = std::max(0.001, z_m); }
void FresnelSimulation::setImageSize(int width, int height) { m_imgWidth = width; m_imgHeight = height; }
void FresnelSimulation::setIntensityScale(double scale) { m_intensityScale = std::max(0.1, std::min(10.0, scale)); }
void FresnelSimulation::setZoom(double zoom) { m_zoom = std::max(0.1, std::min(10.0, zoom)); }
void FresnelSimulation::setResolution(double sigma_um) { m_resolution = std::max(1e-6, sigma_um * 1e-6); }

std::vector<double> FresnelSimulation::computeMonochromaticProfile(int numPoints, double xRange_m, double lambda) {
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

std::vector<double> FresnelSimulation::computeProfile(int numPoints, double xRange_m) {
    auto profile = computeMonochromaticProfile(numPoints, xRange_m, m_lambda);
    double maxVal = *std::max_element(profile.begin(), profile.end());
    if (maxVal > 1e-12)
        for (auto& v : profile) v /= maxVal;
    return profile;
}

cv::Mat FresnelSimulation::generateImage() {
    int profilePoints = 8000;

    // Выбираем поле зрения: 8 зон Френеля
    double fringeWidth = std::sqrt(m_lambda * m_z);   // характерный размер одной зоны
    double autoRange = 2.0 * 8.0 * fringeWidth;       // 8 зон в каждую сторону
    // При очень широкой щели расширяем обзор, чтобы видеть края щели
    if (m_b > autoRange) autoRange = m_b * 1.5;

    autoRange = std::max(0.0005, std::min(0.02, autoRange)); // 0.5 мм ... 2 см
    double xRange = autoRange / m_zoom;
    xRange = std::max(0.0001, std::min(0.05, xRange));       // не более 5 см

    std::vector<double> profile = computeProfile(profilePoints, xRange);

    cv::Mat img(m_imgHeight, m_imgWidth, CV_8UC3, cv::Scalar(30, 30, 30));

    cv::Mat row(1, profilePoints, CV_64FC1);
    for (int i = 0; i < profilePoints; ++i)
        row.at<double>(0, i) = profile[i];

    cv::Mat stretchedRow;
    cv::resize(row, stretchedRow, cv::Size(m_imgWidth, 1), 0, 0, cv::INTER_LINEAR);

    for (int col = 0; col < m_imgWidth; ++col) {
        double I = stretchedRow.at<double>(0, col) * m_intensityScale;
        uchar val = static_cast<uchar>(std::min(255.0, I * 255.0));
        cv::Vec3b color(0, val, 0);
        for (int rowIdx = 0; rowIdx < m_imgHeight; ++rowIdx)
            img.at<cv::Vec3b>(rowIdx, col) = color;
    }

    applyEyepieceOverlay(img, xRange);

    char buf[64];
    snprintf(buf, sizeof(buf), "b = %.3f mm", m_b * 1000);
    cv::putText(img, buf, cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255,255,255), 1);
    snprintf(buf, sizeof(buf), "z = %.1f cm", m_z * 100);
    cv::putText(img, buf, cv::Point(20, 55), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255,255,255), 1);
    snprintf(buf, sizeof(buf), "zoom = %.2fx", m_zoom);
    cv::putText(img, buf, cv::Point(20, 80), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255,255,255), 1);
    cv::putText(img, "Fresnel diffraction", cv::Point(20, m_imgHeight-20),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(200,200,200), 1);

    return img;
}

void FresnelSimulation::applyEyepieceOverlay(cv::Mat &img, double xRange_m) {
    int cx = img.cols / 2, cy = img.rows / 2;
    int radius = static_cast<int>(std::min(cx, cy) * 0.85);
    cv::line(img, cv::Point(cx - 25, cy), cv::Point(cx + 25, cy), cv::Scalar(255,255,255), 1, cv::LINE_AA);
    cv::line(img, cv::Point(cx, cy - 25), cv::Point(cx, cy + 25), cv::Scalar(255,255,255), 1, cv::LINE_AA);

    int scaleY = cy + 40;
    int scaleLen = static_cast<int>(radius * 0.7);
    cv::line(img, cv::Point(cx - scaleLen, scaleY), cv::Point(cx + scaleLen, scaleY), cv::Scalar(200,200,200), 1, cv::LINE_AA);
    double mmPerPixel = xRange_m / img.cols;
    double tickStep_m = 0.0005; // 0.5 мм
    int tickStep_pix = static_cast<int>(tickStep_m / mmPerPixel);
    if (tickStep_pix > 5) {
        for (int x = cx - scaleLen; x <= cx + scaleLen; x += tickStep_pix)
            cv::line(img, cv::Point(x, scaleY - 5), cv::Point(x, scaleY + 5), cv::Scalar(200,200,200), 1, cv::LINE_AA);
    }
    cv::putText(img, "0.5 mm", cv::Point(cx + scaleLen - 40, scaleY - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(200,200,200), 1);
}