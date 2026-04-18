#include "FresnelSimulation.h"
#include "fresnel.h"
#include <cmath>
#include <algorithm>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FresnelSimulation::FresnelSimulation(double lambda)
    : m_lambda(lambda), m_b(0.5e-3), m_z(0.15), m_imgWidth(800), m_imgHeight(600), m_intensityScale(1.0)
{
}

void FresnelSimulation::setSlitWidth(double width_m) {
    m_b = std::max(1e-6, width_m); // не менее 1 мкм
}

void FresnelSimulation::setDistance(double z_m) {
    m_z = std::max(0.001, z_m);    // не менее 1 мм
}

void FresnelSimulation::setImageSize(int width, int height) {
    m_imgWidth = width;
    m_imgHeight = height;
}

void FresnelSimulation::setIntensityScale(double scale) {
    m_intensityScale = std::max(0.1, std::min(10.0, scale));
}

std::vector<double> FresnelSimulation::computeProfile(int numPoints, double xRange_m) {
    std::vector<double> intensity(numPoints, 0.0);
    if (numPoints < 2) return intensity;

    double k = 2.0 * M_PI / m_lambda;
    double factor = std::sqrt(2.0 / (m_lambda * m_z));
    double halfWidth = m_b / 2.0;

    // Для нормировки вычислим интенсивность в центре при полностью открытом фронте
    // (но можно обойтись относительной шкалой)

    for (int i = 0; i < numPoints; ++i) {
        double x = -xRange_m/2.0 + i * xRange_m / (numPoints - 1);
        double u1 = factor * (x - halfWidth);
        double u2 = factor * (x + halfWidth);

        double C1, S1, C2, S2;
        Fresnel::fresnel(u1, C1, S1);
        Fresnel::fresnel(u2, C2, S2);

        double dC = C2 - C1;
        double dS = S2 - S1;
        double I = dC*dC + dS*dS;
        intensity[i] = I;
    }

    // Нормировка на максимум (чтобы диапазон был 0..1)
    double maxI = *std::max_element(intensity.begin(), intensity.end());
    if (maxI > 1e-12) {
        for (auto& val : intensity) {
            val = val / maxI;
        }
    }

    return intensity;
}

cv::Mat FresnelSimulation::generateImage() {
    // Расчёт профиля с достаточным разрешением
    int profilePoints = 1000;
    // Определим физический размер поля зрения. Примерно 2-3 ширины щели плюс дифракционное уширение
    double typicalWidth = std::max(m_b, 5e-3); // не менее 5 мм
    double xRange = std::max(2.0 * m_b, 5.0 * std::sqrt(m_lambda * m_z)); // оценка ширины дифракционной картины
    xRange = std::min(xRange, 0.05); // ограничим 5 см (разумно для микроскопа)
    xRange = std::max(xRange, m_b * 1.5);

    std::vector<double> profile = computeProfile(profilePoints, xRange);

    // Создаём одноканальное изображение нужного размера
    cv::Mat img(m_imgHeight, m_imgWidth, CV_8UC1, cv::Scalar(0));

    // Масштабируем профиль на ширину изображения
    for (int col = 0; col < m_imgWidth; ++col) {
        // Соответствующая координата x в метрах
        double x = -xRange/2.0 + col * xRange / (m_imgWidth - 1);
        // Найти индекс в профиле
        double idx = (x + xRange/2.0) / xRange * (profilePoints - 1);
        int idx0 = static_cast<int>(std::floor(idx));
        int idx1 = std::min(idx0 + 1, profilePoints - 1);
        double frac = idx - idx0;
        double I = (1.0 - frac) * profile[idx0] + frac * profile[idx1];
        // Применяем масштаб яркости
        I = std::pow(I, 0.7) * m_intensityScale; // гамма-коррекция для визуального восприятия
        uchar val = static_cast<uchar>(std::min(255.0, I * 255.0));
        // Заполняем столбец
        for (int row = 0; row < m_imgHeight; ++row) {
            img.at<uchar>(row, col) = val;
        }
    }

    // Применяем эффект окуляра (маска, перекрестие)
    applyEyepieceOverlay(img, xRange);

    // Добавляем текстовую информацию
    std::string text1 = "Slit: " + std::to_string(m_b * 1000).substr(0,5) + " mm";
    std::string text2 = "z: " + std::to_string(m_z * 100).substr(0,5) + " cm";
    cv::putText(img, text1, cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(200), 1);
    cv::putText(img, text2, cv::Point(20, 55), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(200), 1);
    cv::putText(img, "Fresnel diffraction", cv::Point(20, m_imgHeight-20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(180), 1);

    return img;
}

void FresnelSimulation::applyEyepieceOverlay(cv::Mat &img, double xRange_m) {
    int cx = img.cols / 2;
    int cy = img.rows / 2;
    int radius = static_cast<int>(std::min(cx, cy) * 0.9);

    // Виньетирование (затемнение к краям)
    for (int y = 0; y < img.rows; ++y) {
        for (int x = 0; x < img.cols; ++x) {
            double dx = x - cx;
            double dy = y - cy;
            double r = std::sqrt(dx*dx + dy*dy);
            if (r > radius) {
                img.at<uchar>(y, x) = 0; // полностью чёрное за полем зрения
            } else {
                // Плавное падение яркости к краю (виньетка)
                double vignette = 0.7 + 0.3 * std::cos(M_PI * 0.5 * r / radius);
                img.at<uchar>(y, x) = cv::saturate_cast<uchar>(img.at<uchar>(y, x) * vignette);
            }
        }
    }

    // Перекрестие
    int crossLen = 25;
    cv::line(img, cv::Point(cx - crossLen, cy), cv::Point(cx + crossLen, cy), cv::Scalar(255), 1, cv::LINE_AA);
    cv::line(img, cv::Point(cx, cy - crossLen), cv::Point(cx, cy + crossLen), cv::Scalar(255), 1, cv::LINE_AA);

    // Шкала (горизонтальная линия с засечками)
    int scaleY = cy + 30;
    int scaleLen = static_cast<int>(radius * 0.7);
    cv::line(img, cv::Point(cx - scaleLen, scaleY), cv::Point(cx + scaleLen, scaleY), cv::Scalar(200), 1, cv::LINE_AA);
    // Несколько отметок через 0.5 мм (если позволяет размер)
    double mmPerPixel = xRange_m / img.cols;
    double tickStep_m = 0.0005; // 0.5 мм
    int tickStep_pix = static_cast<int>(tickStep_m / mmPerPixel);
    if (tickStep_pix > 5) {
        for (int x = cx - scaleLen; x <= cx + scaleLen; x += tickStep_pix) {
            cv::line(img, cv::Point(x, scaleY - 5), cv::Point(x, scaleY + 5), cv::Scalar(200), 1, cv::LINE_AA);
        }
    }

    // Подпись "0.5 mm"
    cv::putText(img, "0.5 mm", cv::Point(cx + scaleLen - 50, scaleY - 10), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(200), 1);
}