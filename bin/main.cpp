#include <Backend.h>
#include <Frontend.h>


int main() {
    const std::string config_path = "../config/config.json";

    Fetcher config(config_path);    // Open and parse config file.

    std::thread update_info(WeatherScreen::sleep, std::ref(config));

    config.Run();

    WeatherScreen::RenderingAndListen(config);    // Functional part of the UI.

    update_info.detach();

    return 0;
}