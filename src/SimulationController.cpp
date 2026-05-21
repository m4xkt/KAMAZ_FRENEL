#include "SimulationController.h"
#include <algorithm>

SimulationController::SimulationController() 
    : m_sim(515e-9, 36e-9)
{
    // Начальные значения
    m_sim.setSlitWidth(0.5e-3);
    m_sim.setDistance(0.15);
    m_sim.setIntensityScale(1.0);
    m_sim.setZoom(1.0);
    m_sim.setResolution(5.0);
}

void SimulationController::setSlitWidth(double width_mm) {
    m_sim.setSlitWidth(width_mm * 1e-3);
}

void SimulationController::setDistance(double distance_cm) {
    m_sim.setDistance(distance_cm * 1e-2);
}

void SimulationController::setIntensityScale(double scale) {
    m_sim.setIntensityScale(scale);
}

void SimulationController::setZoom(double zoom) {
    m_sim.setZoom(zoom);
}

void SimulationController::setResolution(double sigma_um) {
    m_sim.setResolution(sigma_um);
}

void SimulationController::setWavelength(double lambda_nm) {
    m_sim.setWavelength(lambda_nm * 1e-9);
}

cv::Mat SimulationController::getCurrentImage() {
    return m_sim.generateImage();
}