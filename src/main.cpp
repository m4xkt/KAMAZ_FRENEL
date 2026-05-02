#include <opencv2/opencv.hpp>
#include "FresnelSimulation.h"
#include <iostream>

using namespace std;

FresnelSimulation g_sim(515e-9, 36e-9);
cv::Mat g_image;
string g_windowName = "Fresnel Diffraction on a Slit";

void onSlitChange(int, void*) {
    int pos = cv::getTrackbarPos("Slit width [x0.01 mm]", g_windowName);
    double width_mm = 0.1 + pos * 0.01;
    g_sim.setSlitWidth(width_mm * 1e-3);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
}

void onDistanceChange(int, void*) {
    int pos = cv::getTrackbarPos("Distance [x0.5 cm]", g_windowName);
    double z_cm = 1.0 + pos * 0.5;
    g_sim.setDistance(z_cm * 1e-2);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
}

void onIntensityChange(int, void*) {
    int pos = cv::getTrackbarPos("Brightness [x0.1]", g_windowName);
    double scale = 0.1 + pos * 0.1;
    g_sim.setIntensityScale(scale);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
}

void onZoomChange(int, void*) {
    int pos = cv::getTrackbarPos("Zoom [x0.05]", g_windowName);
    double zoom = 0.2 + pos * 0.05;
    g_sim.setZoom(zoom);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
}

void onResolutionChange(int, void*) {
    int pos = cv::getTrackbarPos("Resolution [um]", g_windowName);
    double sigma_um = 1.0 + pos * 0.5;
    g_sim.setResolution(sigma_um);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
}

int main() {
    cout << "Fresnel Diffraction Simulation\n";
    cout << "Press ESC to exit.\n";

    g_sim.setImageSize(1400, 900);
    g_sim.setSlitWidth(0.5e-3);
    g_sim.setDistance(0.15);
    g_sim.setIntensityScale(1.0);
    g_sim.setZoom(1.0);
    g_sim.setResolution(5.0);

    cv::namedWindow(g_windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(g_windowName, 1400, 900);

    // Трекбары без указателей на переменные (безопасный способ)
    cv::createTrackbar("Slit width [x0.01 mm]", g_windowName, nullptr, 490, onSlitChange);
    cv::createTrackbar("Distance [x0.5 cm]", g_windowName, nullptr, 198, onDistanceChange);
    cv::createTrackbar("Brightness [x0.1]", g_windowName, nullptr, 49, onIntensityChange);
    cv::createTrackbar("Zoom [x0.05]", g_windowName, nullptr, 100, onZoomChange);
    cv::createTrackbar("Resolution [um]", g_windowName, nullptr, 100, onResolutionChange);

    // Установка начальных значений трекбаров
    cv::setTrackbarPos("Slit width [x0.01 mm]", g_windowName, 40);   // 0.5 мм
    cv::setTrackbarPos("Distance [x0.5 cm]", g_windowName, 28);       // 15 см
    cv::setTrackbarPos("Brightness [x0.1]", g_windowName, 9);         // 1.0
    cv::setTrackbarPos("Zoom [x0.05]", g_windowName, 16);             // 1.0
    cv::setTrackbarPos("Resolution [um]", g_windowName, 8);           // 5 мкм

    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);

    while (true) {
        int key = cv::waitKey(30);
        if (key == 27) break;
    }
    cv::destroyAllWindows();
    return 0;
}