/**
 * @file SlvrReport.hpp
 * @author Steven Darwin
 * @version 0.0.1
 * @date Created : 2026-08-17
 * @date Last Modified : 2026-08-17
 *
 * @brief [Header] Mediator class for solver report creation.
 */

#ifndef SLVR_REPORT_HPP
#define SLVR_REPORT_HPP

#include <string>
#include <unordered_map>
#include <array>
#include <vector>
#include <tuple>
#include <chrono>

#include "geometry-topology/GeometryTopology.hpp"
#include "initial-boundary/SlvrZoning.hpp"

class SlvrReport {
public:
    struct InitBdrySetupItem {
        std::string condition_type;
        std::string zone_name;
        std::string parameter;
        std::string mode;
        double value;
    };

    static SlvrReport& instance();

    SlvrReport(const SlvrReport&) = delete;
    SlvrReport& operator=(const SlvrReport&) = delete;

    void setInitBdrySetupItem(InitBdrySetupItem data);

    void addErrorProgression(double value);
    void addTimePoint(std::string label, std::chrono::time_point<std::chrono::system_clock> timestamp);
    void addFileSuffix(std::string type, std::string file_suffix, std::string extension);

    void exportData();

private:
    /** Constructor of MeshReport object */
    SlvrReport() = default;

    std::vector<InitBdrySetupItem> _initBdrySetupItemList;
    std::vector<double> _errorValueList;
    std::vector<std::tuple<std::string, std::chrono::time_point<std::chrono::system_clock>>> _timePointList;
    std::vector<std::tuple<std::string, std::string, std::string>> _fileSuffixList;

    std::string paddingCenteredString(std::string text, std::string padding_char, unsigned int line_width);
};

#endif