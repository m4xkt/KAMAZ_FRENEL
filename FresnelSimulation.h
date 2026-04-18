#ifndef FRESNEL_SIMULATION_H
#define FRESNEL_SIMULATION_H

#include <opencv2/opencv.hpp>
#include <vector>

class FresnelSimulation {
public:
    FresnelSimulation(double lambda = 515e-9); // зелёный свет по умолчанию

    void setSlitWidth(double width_m);    // ширина щели в метрах
    void setDistance(double z_m);          // расстояние до плоскости наблюдения в метрах
    void setImageSize(int width, int height);
    void setIntensityScale(double scale);  // множитель яркости

    // Расчёт одномерного профиля интенсивности (относительные единицы)
    std::vector<double> computeProfile(int numPoints, double xRange_m);

    // Генерация изображения, имитирующего вид в окуляр микроскопа
    cv::Mat generateImage();

    // Вспомогательные методы для получения текущих параметров
    double getSlitWidth() const { return m_b; }
    double getDistance() const { return m_z; }

private:
    double m_lambda;       // длина волны [м]
    double m_b;            // ширина щели [м]
    double m_z;            // расстояние [м]
    int m_imgWidth;
    int m_imgHeight;
    double m_intensityScale;

    // Применение маски поля зрения, перекрестия, шкалы
    void applyEyepieceOverlay(cv::Mat &img, double xRange_m);
};

#endif // FRESNEL_SIMULATION_H