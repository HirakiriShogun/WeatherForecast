#pragma once
#include "Backend.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iomanip>
#include <sstream>

#include <ctime>
#include <thread>
#include <chrono>
#include <mutex>

std::mutex mtx;

constexpr int KEY_STATE_FLAG = 0x8001;
static int current_city_index = 0;
static int current_day = 0;

class WeatherScreen {
public:
    static void sleep(const Fetcher& config);
    static void Listen(Fetcher& config);
    static void RenderingAndListen(Fetcher& config);
    static void DrawTable(const std::string& city_name, double** city_info, const std::string& time, int current_day);
    static void FillInformation(const std::string& city_name, int current_day);

private:
    static std::string FormatDate(const std::string& original_date);
    static std::wstring TrimZeros(const std::wstring& string);
    static std::string WeatherCode(double weather_code);
    static std::string WindDirection(double wind_code);

};