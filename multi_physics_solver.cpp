#include "core/SlvrCore.hpp"
#include "initial-boundary/SlvrZoning.hpp"

#include <stdlib.h>
#include <iostream>
#include <memory>

#include "CoolProp.h"
#include "AbstractState.h"

int main(int argc, char* argv[]) {
    SlvrZoning init_bdry_program(argv[1]);
    init_bdry_program.setupPhase();

    SlvrCore solver_program(argv[1]);
    solver_program.setup();

    return 0;

    //std::cout << "Entry" << std::endl;

    //std::shared_ptr<CoolProp::AbstractState> Water(CoolProp::AbstractState::factory("REFPROP", "Water"));

    //std::cout << "Construction is done" << std::endl;

    //Water->update(CoolProp::PQ_INPUTS, 101325, 0);  // SI units

    //std::cout << "State is updated" << std::endl;

    //std::cout << "T: " << Water->T() << " K" << std::endl;
    //std::cout << "rho': " << Water->rhomass() << " kg/m^3" << std::endl;
    //std::cout << "rho': " << Water->rhomolar() << " mol/m^3" << std::endl;
    //std::cout << "h': " << Water->hmass() << " J/kg" << std::endl;
    //std::cout << "h': " << Water->hmolar() << " J/mol" << std::endl;
    //std::cout << "s': " << Water->smass() << " J/kg/K" << std::endl;
    //std::cout << "s': " << Water->smolar() << " J/mol/K" << std::endl;

    //return EXIT_SUCCESS;
}