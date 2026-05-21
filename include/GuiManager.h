#pragma once
#include <opencv2/opencv.hpp>
#include "SimulationController.h"

class GuiManager {
public:
    GuiManager(SimulationController& controller);
    void run(); // запускает главный цикл OpenCV
    
private:
    static void onSlitWidth(int value, void* userdata);
    static void onDistance(int value, void* userdata);
    static void onIntensity(int value, void* userdata);
    static void onZoom(int value, void* userdata);
    static void onResolution(int value, void* userdata);
    static void onWavelength(int value, void* userdata);
    
    void updateAndShow();
    
    SimulationController* m_controller;
    cv::Mat m_currentImage;
    std::string m_windowName = "Fresnel Diffraction on a Slit";
};