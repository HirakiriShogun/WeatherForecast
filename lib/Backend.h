#pragma once
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <vector>


class City {
public:
    explicit City(const char* name_city, std::string latitude, std::string longitude);

    const std::string& GetName();
    const std::string& GetLatitude() const;
    const std::string& GetLongitude() const;

    ~City() = default;
private:
    std::string name_city;
    std::string latitude;
    std::string longitude;
};

class Fetcher {
public:
    explicit Fetcher(const std::string& config_path);

    void Run();

    void GetCoordinates();
    void ForecastRequest() const;

    const std::vector<City*>& GetCities() const;
    int GetFrequency() const;
    int GetForecastDays() const;

    ~Fetcher();
private:
    std::string api_key;
    std::vector<City*> cities;

    int forecast_days;
    int frequency;

    nlohmann::json json_config;
};
