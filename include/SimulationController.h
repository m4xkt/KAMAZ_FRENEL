#pragma once
#include "FresnelSimulation.h"

class SimulationController {
public:
    SimulationController();
    void setSlitWidth(double width_mm);
    void setDistance(double distance_cm);
    void setIntensityScale(double scale);
    void setZoom(double zoom);
    void setResolution(double sigma_um);
    void setWavelength(double lambda_nm);
    
    cv::Mat getCurrentImage();
    FresnelSimulation& getSimulation() { return m_sim; }
    
private:
    FresnelSimulation m_sim;
};