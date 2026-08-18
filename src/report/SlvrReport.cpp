/**
 * @file slvrReport.cpp
 * @author Steven Darwin
 * @version 0.0.1
 * @date Created : 2026-08-17
 * @date Last Modified : 2026-08-17
 *
 * @brief [Implementation] Mediator class for slvr report creation.
 */

#include <fstream>

#include "utility/ConfigReader.hpp"
#include "report/SlvrReport.hpp"

SlvrReport& SlvrReport::instance() {
    static SlvrReport singleton;
    return singleton;
}

void SlvrReport::setInitBdrySetupItem(InitBdrySetupItem data) {
    _initBdrySetupItemList.push_back(data);
}

void SlvrReport::addErrorProgression(double value) {
    _errorValueList.push_back(value);
}

void SlvrReport::addTimePoint(std::string label, std::chrono::time_point<std::chrono::system_clock> timestamp) {
    _timePointList.push_back({ label, timestamp });
}

void SlvrReport::addFileSuffix(std::string type, std::string file_suffix, std::string extension) {
    _fileSuffixList.push_back({ type, file_suffix, extension });
}

void SlvrReport::exportData() {
    std::ofstream report_file(
        ConfigReader::instance().getRuntimeConfigValue("scmp", "staging_directory_path") + "/" + ConfigReader::instance().getRuntimeConfigValue("scmp", "file_name_prefix") + "_report.txt",
        std::ios::app
    );

    std::ostringstream slvr_data_in_str;

    // Header
    slvr_data_in_str << paddingCenteredString("", "=", 8) << "\n";
    slvr_data_in_str << paddingCenteredString("SOLVER", " ", 8) << "\n";
    slvr_data_in_str << paddingCenteredString("", "=", 8) << "\n";
    slvr_data_in_str << "\n";

    // Configuration
    slvr_data_in_str << "> Configuration" << "\n";
    slvr_data_in_str << "\n";
    slvr_data_in_str << "strategy: " << ConfigReader::instance().getRuntimeConfigValue("slvr", "strategy") << "\n";
    slvr_data_in_str << "max_iteration: " << ConfigReader::instance().getRuntimeConfigValue("slvr", "gauss_seidel_max_iter") << "\n";
    slvr_data_in_str << "error_tolerance: " << ConfigReader::instance().getRuntimeConfigValue("slvr", "gauss_seidel_error_tolerance") << "\n";
    slvr_data_in_str << "relaxation_factor: " << ConfigReader::instance().getRuntimeConfigValue("slvr", "gauss_seidel_relaxation_factor") << "\n";
    slvr_data_in_str << "\n";

    // Milestone
    slvr_data_in_str << "> Milestone" << "\n";
    slvr_data_in_str << "\n";

    for (auto iter = _timePointList.begin(); iter != _timePointList.end(); iter++) {
        slvr_data_in_str << std::get<0>(*iter) << " -> ";

        const std::time_t temp_time = std::chrono::system_clock::to_time_t(std::get<1>(*iter));
        slvr_data_in_str << std::put_time(std::gmtime(&temp_time), "%F %T");
        slvr_data_in_str << "." << std::chrono::duration_cast<std::chrono::milliseconds>(std::get<1>(*iter).time_since_epoch()).count() % 1000 << "\n";
    }

    slvr_data_in_str << "\n";

    // Initial Condition
    slvr_data_in_str << "> Initial Condition" << "\n";
    slvr_data_in_str << "\n";

    slvr_data_in_str << "zone | parameter | mode | value" << "\n";
    for (auto iter = _initBdrySetupItemList.begin(); iter != _initBdrySetupItemList.end(); iter++) {
        if (iter->condition_type == "initial") {
            slvr_data_in_str << iter->zone_name << " | " << iter->parameter << " | " << iter->mode << " | " << iter->value << "\n";
        }
    }
    slvr_data_in_str << "\n";

    // Boundary Condition
    slvr_data_in_str << "> Boundary Condition" << "\n";
    slvr_data_in_str << "\n";

    slvr_data_in_str << "zone | parameter | mode | value" << "\n";
    for (auto iter = _initBdrySetupItemList.begin(); iter != _initBdrySetupItemList.end(); iter++) {
        if (iter->condition_type == "boundary") {
            slvr_data_in_str << iter->zone_name << " | " << iter->parameter << " | " << iter->mode << " | " << iter->value << "\n";
        }
    }

    slvr_data_in_str << "\n";

    // Error Progression
    slvr_data_in_str << "> Error Progression" << "\n";
    slvr_data_in_str << "\n";

    slvr_data_in_str << "iter | error" << "\n";
    unsigned int error_iter = 0;
    for (auto error_per_iter : _errorValueList) {
        slvr_data_in_str << ++error_iter << " | " << error_per_iter << "\n";
    }
    slvr_data_in_str << "\n";

    // I/O Files
    slvr_data_in_str << "> I/O Files" << "\n";
    slvr_data_in_str << "\n";

    for (auto iter = _fileSuffixList.begin(); iter != _fileSuffixList.end(); iter++) {
        slvr_data_in_str << std::get<0>(*iter) << " | ";
        slvr_data_in_str << ConfigReader::instance().getRuntimeConfigValue("scmp", "file_name_prefix");
        if (std::get<1>(*iter) != "") slvr_data_in_str << "." + std::get<1>(*iter);
        slvr_data_in_str << "." + std::get<2>(*iter) << "\n";
    }

    slvr_data_in_str << "\n";

    report_file << slvr_data_in_str.str();
    report_file.close();
}

std::string SlvrReport::paddingCenteredString(std::string text, std::string padding_char, unsigned int line_width) {
    if (text.length() >= line_width) {
        return text;
    }
    else {
        while (text.length() < line_width) {
            text = padding_char + text + padding_char;
        }

        return text;
    }
}