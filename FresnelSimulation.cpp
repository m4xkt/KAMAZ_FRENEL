#include "FresnelSimulation.h"
#include "fresnel.h"
#include <cmath>
#include <algorithm>
#include <string>
#include <numeric>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FresnelSimulation::FresnelSimulation(double lambda, double deltaLambda)
    : m_lambda(lambda), m_dLambda(deltaLambda), m_b(0.5e-3), m_z(0.15),
      m_imgWidth(1400), m_imgHeight(900), m_intensityScale(1.0), m_zoom(1.0),
      m_resolution(5e-6)
{
}

void FresnelSimulation::setSlitWidth(double width_m) {
    m_b = std::max(1e-6, width_m);
}

void FresnelSimulation::setDistance(double z_m) {
    m_z = std::max(0.001, z_m);
}

void FresnelSimulation::setImageSize(int width, int height) {
    m_imgWidth = width;
    m_imgHeight = height;
}

void FresnelSimulation::setIntensityScale(double scale) {
    m_intensityScale = std::max(0.1, std::min(10.0, scale));
}

void FresnelSimulation::setZoom(double zoom) {
    m_zoom = std::max(0.2, std::min(5.0, zoom));
}

void FresnelSimulation::setResolution(double sigma_um) {
    m_resolution = std::max(1e-6, sigma_um * 1e-6);
}

// Расчёт монохроматического профиля
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

// Основной метод расчёта профиля с учётом ширины спектра и аппаратной функции
std::vector<double> FresnelSimulation::computeProfile(int numPoints, double xRange_m) {
    // Усреднение по длинам волн
    const int nLambdas = 5;
    std::vector<double> wavelengths(nLambdas);
    std::vector<double> weights(nLambdas);
    double sigma_lambda = m_dLambda / (2.0 * std::sqrt(2.0 * std::log(2.0)));

    double sumW = 0.0;
    for (int i = 0; i < nLambdas; ++i) {
        double t = -1.0 + 2.0 * i / (nLambdas - 1.0);
        wavelengths[i] = m_lambda + t * m_dLambda * 0.6;
        double w = std::exp(-t*t * (m_dLambda*m_dLambda) / (2.0*sigma_lambda*sigma_lambda));
        weights[i] = w;
        sumW += w;
    }
    for (auto& w : weights) w /= sumW;

    std::vector<double> profile(numPoints, 0.0);
    for (int i = 0; i < nLambdas; ++i) {
        auto mono = computeMonochromaticProfile(numPoints, xRange_m, wavelengths[i]);
        for (int j = 0; j < numPoints; ++j) {
            profile[j] += weights[i] * mono[j];
        }
    }

    // Свёртка с гауссовой аппаратной функцией
    double dx = xRange_m / (numPoints - 1);
    double sigma_pix = m_resolution / dx;
    int kernelRadius = static_cast<int>(std::ceil(3.0 * sigma_pix));
    std::vector<double> kernel(2*kernelRadius + 1);
    double sumK = 0.0;
    for (int k = -kernelRadius; k <= kernelRadius; ++k) {
        double val = std::exp(-(k*k) / (2.0 * sigma_pix * sigma_pix));
        kernel[k + kernelRadius] = val;
        sumK += val;
    }
    for (auto& k : kernel) k /= sumK;

    std::vector<double> smoothed(numPoints, 0.0);
    for (int i = 0; i < numPoints; ++i) {
        double sum = 0.0;
        for (int k = -kernelRadius; k <= kernelRadius; ++k) {
            int idx = i + k;
            if (idx >= 0 && idx < numPoints) {
                sum += profile[idx] * kernel[k + kernelRadius];
            }
        }
        smoothed[i] = sum;
    }

    // Нормировка на максимальное значение (чтобы пик был около 1.0)
    double maxVal = *std::max_element(smoothed.begin(), smoothed.end());
    if (maxVal > 1e-12) {
        for (auto& v : smoothed) v /= maxVal;
    }

    return smoothed;
}

cv::Mat FresnelSimulation::generateImage() {
    int profilePoints = 2000;

    // Простой и надёжный расчёт ширины поля зрения
    double fringe = std::sqrt(m_lambda * m_z);
    double xRange = 2.0 * (m_b + 5.0 * fringe);
    xRange = std::max(0.002, std::min(0.08, xRange));   // от 2 мм до 8 см
    xRange /= m_zoom;
    xRange = std::max(0.001, std::min(0.12, xRange));   // с учётом зума

    std::vector<double> profile = computeProfile(profilePoints, xRange);

    // Тёмно-серый фон
    cv::Mat img(m_imgHeight, m_imgWidth, CV_8UC3, cv::Scalar(30, 30, 30));

    // Масштабируем профиль на ширину изображения
    cv::Mat row(1, profilePoints, CV_64FC1);
    for (int i = 0; i < profilePoints; ++i)
        row.at<double>(0, i) = profile[i];

    cv::Mat stretchedRow;
    cv::resize(row, stretchedRow, cv::Size(m_imgWidth, 1), 0, 0, cv::INTER_LINEAR);

    // Заполняем столбцы зелёным свечением с гамма-коррекцией и масштабом яркости
    for (int col = 0; col < m_imgWidth; ++col) {
        double I = stretchedRow.at<double>(0, col);
        I = std::pow(std::min(1.0, I), 0.8);  // гамма для улучшения видимости слабых деталей
        I = I * m_intensityScale;
        uchar val = static_cast<uchar>(std::min(255.0, I * 255.0));
        cv::Vec3b color(0, val, 0);
        for (int rowIdx = 0; rowIdx < m_imgHeight; ++rowIdx)
            img.at<cv::Vec3b>(rowIdx, col) = color;
    }

    applyEyepieceOverlay(img, xRange);

    // Текстовые подписи
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
    int cx = img.cols / 2;
    int cy = img.rows / 2;
    int radius = static_cast<int>(std::min(cx, cy) * 0.85);

    int crossLen = 25;
    cv::line(img, cv::Point(cx - crossLen, cy), cv::Point(cx + crossLen, cy),
             cv::Scalar(255,255,255), 1, cv::LINE_AA);
    cv::line(img, cv::Point(cx, cy - crossLen), cv::Point(cx, cy + crossLen),
             cv::Scalar(255,255,255), 1, cv::LINE_AA);

    int scaleY = cy + 40;
    int scaleLen = static_cast<int>(radius * 0.7);
    cv::line(img, cv::Point(cx - scaleLen, scaleY), cv::Point(cx + scaleLen, scaleY),
             cv::Scalar(200,200,200), 1, cv::LINE_AA);

    double mmPerPixel = xRange_m / img.cols;
    double tickStep_m = 0.001; // 1 мм
    int tickStep_pix = static_cast<int>(tickStep_m / mmPerPixel);
    if (tickStep_pix > 8) {
        for (int x = cx - scaleLen; x <= cx + scaleLen; x += tickStep_pix) {
            cv::line(img, cv::Point(x, scaleY - 5), cv::Point(x, scaleY + 5),
                     cv::Scalar(200,200,200), 1, cv::LINE_AA);
        }
    }

    cv::putText(img, "1 mm", cv::Point(cx + scaleLen - 50, scaleY - 10),
                cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(200,200,200), 1);
}