#include <opencv2/opencv.hpp>
#include "FresnelSimulation.h"
#include <iostream>
#include <cmath>

// Глобальные переменные для трекбаров
FresnelSimulation g_sim(515e-9); // зелёный
cv::Mat g_image;
std::string g_windowName = "Fresnel Diffraction on a Slit";

// Обработчики трекбаров
void onSlitChange(int pos, void*) {
    // pos в условных единицах: 0..200 -> 0.1..2.0 мм
    double width_mm = 0.1 + pos * 0.01; // шаг 0.01 мм
    g_sim.setSlitWidth(width_mm * 1e-3);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
}

void onDistanceChange(int pos, void*) {
    // pos: 0..100 -> 1..50 см
    double z_cm = 1.0 + pos * 0.5; // шаг 0.5 см
    g_sim.setDistance(z_cm * 1e-2);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
}

void onIntensityChange(int pos, void*) {
    // pos: 0..20 -> 0.1..2.0
    double scale = 0.1 + pos * 0.1;
    g_sim.setIntensityScale(scale);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
}

int main() {
    std::cout << "Fresnel Diffraction Simulation\n";
    std::cout << "Use trackbars to change slit width, distance, and brightness.\n";
    std::cout << "Press ESC to exit.\n";

    // Начальные параметры
    g_sim.setImageSize(1000, 700);
    g_sim.setSlitWidth(0.5e-3); // 0.5 мм
    g_sim.setDistance(0.15);    // 15 см
    g_sim.setIntensityScale(1.0);

    // Создание окна
    cv::namedWindow(g_windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(g_windowName, 1000, 700);

    // Трекбары
    int slitPos = 40; // соответствует 0.5 мм (0.1 + 40*0.01 = 0.5)
    int distPos = 28; // 1 + 28*0.5 = 15 см
    int intenPos = 9; // 0.1 + 9*0.1 = 1.0

    cv::createTrackbar("Slit width [x0.01 mm]", g_windowName, &slitPos, 190, onSlitChange);
    cv::createTrackbar("Distance [x0.5 cm]", g_windowName, &distPos, 98, onDistanceChange);
    cv::createTrackbar("Brightness [x0.1]", g_windowName, &intenPos, 19, onIntensityChange);

    // Первоначальное отображение
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);

    // Главный цикл
    while (true) {
        int key = cv::waitKey(30);
        if (key == 27) break; // ESC
        // Можно добавить ручное обновление, но трекбары уже вызывают обновление
    }

    cv::destroyAllWindows();
    return 0;
}