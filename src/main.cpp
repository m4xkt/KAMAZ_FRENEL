#include "GuiManager.h"
#include "SimulationController.h"
#include <iostream>

int main() {
    try {
        std::cout << "=== Fresnel Diffraction Simulation ===\n";
        SimulationController controller;
        GuiManager gui(controller);
        gui.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}