#include "GuiManager.h"
#include <iostream>

GuiManager::GuiManager(SimulationController& controller)
    : m_controller(&controller)
{
    cv::namedWindow(m_windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(m_windowName, 1400, 900);
    
    // Создание трекбаров с передачей указателя на this
    cv::createTrackbar("Slit width [x0.01 mm]", m_windowName, nullptr, 490, onSlitWidth, this);
    cv::createTrackbar("Distance [x0.5 cm]",     m_windowName, nullptr, 198, onDistance, this);
    cv::createTrackbar("Brightness [x0.1]",      m_windowName, nullptr,  49, onIntensity, this);
    cv::createTrackbar("Zoom [x0.05]",           m_windowName, nullptr, 100, onZoom, this);
    cv::createTrackbar("Resolution [um]",        m_windowName, nullptr, 100, onResolution, this);
    cv::createTrackbar("Wavelength [nm]",        m_windowName, nullptr, 300, onWavelength, this);
    
    // Установка начальных положений (соответствуют значениям в контроллере)
    cv::setTrackbarPos("Slit width [x0.01 mm]", m_windowName, 40);   // 0.5 мм
    cv::setTrackbarPos("Distance [x0.5 cm]",     m_windowName, 28);   // 15 см
    cv::setTrackbarPos("Brightness [x0.1]",      m_windowName,  9);   // 1.0
    cv::setTrackbarPos("Zoom [x0.05]",           m_windowName, 16);   // 1.0
    cv::setTrackbarPos("Resolution [um]",        m_windowName,  8);   // 5.0
    cv::setTrackbarPos("Wavelength [nm]",        m_windowName,115);   // 515 нм
}

void GuiManager::run() {
    updateAndShow();
    while (true) {
        int key = cv::waitKey(30);
        if (key == 27) break;                     // ESC – выход
        if (key == 's' || key == 'S') {           // S – сохранить профиль
            m_controller->getSimulation().saveProfileToFile("../src/fresnel_profile_cpp.txt");
            std::cout << "Saved profile to fresnel_profile_cpp.txt" << std::endl;
        }
    }
    cv::destroyAllWindows();
}

void GuiManager::updateAndShow() {
    m_currentImage = m_controller->getCurrentImage();
    cv::imshow(m_windowName, m_currentImage);
}

// Статические callback-функции
void GuiManager::onSlitWidth(int value, void* userdata) {
    GuiManager* self = static_cast<GuiManager*>(userdata);
    double width_mm = 0.1 + value * 0.01;
    self->m_controller->setSlitWidth(width_mm);
    self->updateAndShow();
}

void GuiManager::onDistance(int value, void* userdata) {
    GuiManager* self = static_cast<GuiManager*>(userdata);
    double z_cm = 1.0 + value * 0.5;
    self->m_controller->setDistance(z_cm);
    self->updateAndShow();
}

void GuiManager::onIntensity(int value, void* userdata) {
    GuiManager* self = static_cast<GuiManager*>(userdata);
    double scale = 0.1 + value * 0.1;
    self->m_controller->setIntensityScale(scale);
    self->updateAndShow();
}

void GuiManager::onZoom(int value, void* userdata) {
    GuiManager* self = static_cast<GuiManager*>(userdata);
    double zoom = 0.2 + value * 0.05;
    self->m_controller->setZoom(zoom);
    self->updateAndShow();
}

void GuiManager::onResolution(int value, void* userdata) {
    GuiManager* self = static_cast<GuiManager*>(userdata);
    double sigma_um = 1.0 + value * 0.5;
    self->m_controller->setResolution(sigma_um);
    self->updateAndShow();
}

void GuiManager::onWavelength(int value, void* userdata) {
    GuiManager* self = static_cast<GuiManager*>(userdata);
    double lambda_nm = 400 + value;   // от 400 до 700 нм
    self->m_controller->setWavelength(lambda_nm);
    self->updateAndShow();
}