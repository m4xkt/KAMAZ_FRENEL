#include <opencv2/opencv.hpp>
#include "FresnelSimulation.h"
#include <iostream>
#include <sstream>

using namespace std;

FresnelSimulation g_sim(515e-9, 36e-9);
cv::Mat g_image;
string g_windowName = "Fresnel Diffraction on a Slit";

void updateWindowTitle() {
    ostringstream oss;
    oss << "Fresnel: b=" << g_sim.getSlitWidth()*1000 << "mm, z=" << g_sim.getDistance()*100 << "cm";
    cv::setWindowTitle(g_windowName, oss.str());
}

void onSlitChange(int pos, void*) {
    double width_mm = 0.1 + pos * 0.01;
    g_sim.setSlitWidth(width_mm * 1e-3);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
    updateWindowTitle();
}

void onDistanceChange(int pos, void*) {
    double z_cm = 1.0 + pos * 0.5;
    g_sim.setDistance(z_cm * 1e-2);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
    updateWindowTitle();
}

void onIntensityChange(int pos, void*) {
    double scale = 0.1 + pos * 0.1;   // теперь до 50*0.1=5.0
    g_sim.setIntensityScale(scale);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
}

void onZoomChange(int pos, void*) {
    double zoom = 0.2 + pos * 0.05;
    g_sim.setZoom(zoom);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
}

void onResolutionChange(int pos, void*) {
    double sigma_um = 1.0 + pos * 0.5;
    g_sim.setResolution(sigma_um);
    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
}

int main() {
    cout << "Fresnel Diffraction Simulation\n";
    cout << "Parameters: slit width (0.1-5.0 mm), distance (1-100 cm), brightness (0.1-5.0), zoom, resolution\n";
    cout << "Press ESC to exit.\n";

    g_sim.setImageSize(1400, 900);
    g_sim.setSlitWidth(0.5e-3);
    g_sim.setDistance(0.15);
    g_sim.setIntensityScale(1.0);
    g_sim.setZoom(1.0);
    g_sim.setResolution(5.0);

    cv::namedWindow(g_windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(g_windowName, 1400, 900);

    int slitPos = 40;       // 0.5 mm
    int distPos = 28;       // 15 cm
    int intenPos = 19;       // 1.0
    int zoomPos = 16;       // 1.0
    int resPos = 8;         // 5 um

    cv::createTrackbar("Slit width [x0.01 mm]", g_windowName, &slitPos, 490, onSlitChange);
    cv::createTrackbar("Distance [x0.5 cm]", g_windowName, &distPos, 198, onDistanceChange);
    cv::createTrackbar("Brightness [x0.1]", g_windowName, &intenPos, 49, onIntensityChange); // до 5.0 // до 5.0
    cv::createTrackbar("Zoom [x0.05]", g_windowName, &zoomPos, 100, onZoomChange);
    cv::createTrackbar("Resolution [um]", g_windowName, &resPos, 100, onResolutionChange);

    g_image = g_sim.generateImage();
    cv::imshow(g_windowName, g_image);
    updateWindowTitle();

    while (true) {
        int key = cv::waitKey(30);
        if (key == 27) break;
    }

    cv::destroyAllWindows();
    return 0;
}