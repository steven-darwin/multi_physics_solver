/**
 * @file SolverCore.hpp
 * @author Steven Darwin
 * @version 0.0.1
 * @date Created : 2026-04-18
 * @date Last Modified : 2026-04-18
 *
 * @brief [Header] ...
 */

#ifndef SOLVER_CORE_HPP
#define SOLVER_CORE_HPP

#include "general/Generic.hpp"

#include "geometry-topology/GeometryTopology.hpp"

class SolverCore {
public:
    /** Constructor of SolverCore object
     */
    SolverCore();

    /** Destructor of SolverCore object */
    ~SolverCore();

    void setup();

private:
    /** Attribute to store input adapter metadata */
    scmp::AdapterInfo _inputAdapterInfo;

    /** Attribute to store output adapter metadata */
    scmp::AdapterInfo _outputAdapterInfo;
};

#endif