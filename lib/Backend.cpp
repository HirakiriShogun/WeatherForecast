#include "Backend.h"

City::City(const char* name_city, std::string latitude, std::string longitude) {
    this->name_city = name_city;
    this->latitude = latitude;
    this->longitude = longitude;
}

const std::string& City::GetName() {
    return this->name_city;
}

const std::string& City::GetLatitude() const {
    return this->latitude;
}

const std::string& City::GetLongitude() const {
    return this->longitude;
}

Fetcher::Fetcher(const std::string& config_path) {
    std::ifstream config;

    config.open(config_path, std::ios::in);
    if (!config.is_open()) {
        std::cerr << "Can not open config!\n";
        exit(EXIT_FAILURE);
    }

    this->json_config = nlohmann::json::parse(config);
    this->api_key = json_config["api_key"];
    this->forecast_days = json_config["forecast_days"];
    this->frequency = json_config["frequency"];
}

void Fetcher::Run() {
    GetCoordinates();   // Make requests in API-Ninjas and get response - longitude and latitude. Write it in weather.json.
    ForecastRequest();  // Get full information from Open-Meteo and write it in weather.json.
}

void Fetcher::GetCoordinates() {
    std::ifstream file("weather.json");
    if (file.is_open()) {
        nlohmann::json cache = nlohmann::json::parse(file);
        for (const std::string& city_name : json_config["cities"]) {
            if (cache.find(city_name) == cache.end()) {
                cpr::Response city_coordinates = cpr::Get(
                    cpr::Url{ "https://api.api-ninjas.com/v1/city?name=" + city_name },
                    cpr::Header{ {"X-Api-Key", api_key} }
                );
                if (city_coordinates.status_code == 200) {
                    nlohmann::json city_coordinate = nlohmann::json::parse(city_coordinates.text)[0];
                    double temp_latitude = city_coordinate["latitude"];
                    double temp_longitude = city_coordinate["longitude"];

                    cities.push_back(new City(city_name.c_str(), std::to_string(temp_latitude), std::to_string(temp_longitude)));

                    cache[city_name]["latitude"] = temp_latitude;
                    cache[city_name]["longitude"] = temp_longitude;
                }
                else {
                    std::cerr << "Request errorGC1!\n";
                    exit(EXIT_FAILURE);
                }
            }
            else {
                double temp_latitude = cache[city_name]["latitude"];
                double temp_longitude = cache[city_name]["longitude"];

                cities.push_back(new City(city_name.c_str(), std::to_string(temp_latitude), std::to_string(temp_longitude)));
            }
        }
        std::ofstream weather_file;
        weather_file.open("weather.json", std::ios::out);
        weather_file << std::setw(4) << cache;
    }
    else {
        nlohmann::json cache;
        for (const std::string& city_name : json_config["cities"]) {
            cpr::Response city_coordinates = cpr::Get(
                cpr::Url{ "https://api.api-ninjas.com/v1/city?name=" + city_name },
                cpr::Header{ {"X-Api-Key", api_key} }
            );
            if (city_coordinates.status_code == 200) {
                nlohmann::json city_coordinate = nlohmann::json::parse(city_coordinates.text)[0];
                double temp_latitude = city_coordinate["latitude"];
                double temp_longitude = city_coordinate["longitude"];

                cities.push_back(new City(city_name.c_str(), std::to_string(temp_latitude), std::to_string(temp_longitude)));

                cache[city_name]["latitude"] = temp_latitude;
                cache[city_name]["longitude"] = temp_longitude;
            }
            else {
                std::cerr << "Request error!\n";
                exit(EXIT_FAILURE);
            }
        }
        std::ofstream weather_file;
        weather_file.open("weather.json", std::ios::out);
        weather_file << std::setw(4) << cache;
    }
}

void Fetcher::ForecastRequest() const {
    nlohmann::json json_weather;
    for (const auto& city : cities) {
        cpr::Response response = cpr::Get(
            cpr::Url{ "https://api.open-meteo.com/v1/forecast" },
            cpr::Parameters{
                {"latitude", city->GetLatitude()},
                {"longitude", city->GetLongitude()},

                {"hourly", "weather_code"},
                {"hourly", "temperature_2m"},
                {"hourly", "apparent_temperature"},

                {"hourly", "precipitation"},
                {"hourly", "precipitation_probability"},

                {"hourly", "wind_speed_10m"},
                {"hourly", "wind_direction_10m"},

                {"forecast_days", std::to_string(Fetcher::GetForecastDays())},
                {"timezone", "GMT"}
            }
        );
        if (response.status_code == 200) {
            json_weather[city->GetName()] = nlohmann::json::parse(response.text);
            std::ofstream file;
            file.open("weather.json", std::ios::out | std::ios::trunc);
            file << json_weather;
        }
        else {
            std::cerr << "Request error! " << response.text << '\n';
            exit(EXIT_FAILURE);
        }
    }

}

const std::vector<City*>& Fetcher::GetCities() const {
    return this->cities;
}

int Fetcher::GetFrequency() const {
    return this->frequency;
}

int Fetcher::GetForecastDays() const {
    return this->forecast_days;
}

Fetcher::~Fetcher() {
    for (const auto& city : cities) {
        delete city;
    }
}
