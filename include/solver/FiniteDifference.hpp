/**
 * @file FiniteDifference.hpp
 * @author Steven Darwin
 * @version 0.0.1
 * @date Created : 2026-05-30
 * @date Last Modified : 2026-05-30
 *
 * @brief [Header] ...
 */

#ifndef FINITE_DIFFERENCE_HPP
#define FINITE_DIFFERENCE_HPP

#include "geometry-topology/GeometryTopology.hpp"

class FiniteDifference {
public:
    /** Constructor of FiniteDifference object
     */
    FiniteDifference(std::shared_ptr<GeometryTopology> neutral_geometry_topology);

    /** Destructor of FiniteDifference object */
    ~FiniteDifference();

    /** Method to solve the differential equation using point iterative method */
    std::shared_ptr<GeometryTopology> pointIterative(unsigned int step_count, double relaxation_factor, unsigned int iter_count, double error_tolerance);

    /** Method to solve the differential equation using predictive corrective method */
    void predictiveCorrective();

private:
    /** Attribute to store nwutral geometry-topology data */
    std::shared_ptr<GeometryTopology> _neutralGeometryTopology;
};

#endif