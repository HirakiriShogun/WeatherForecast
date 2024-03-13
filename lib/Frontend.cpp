#include "Frontend.h"

void WeatherScreen::RenderingAndListen(Fetcher& config) {
    system("cls");
    WeatherScreen::FillInformation(config.GetCities()[current_city_index]->GetName(), current_day);
    Sleep(500);

    Listen(config);
}


void WeatherScreen::Listen(Fetcher& config) {
    static int cities_amount = config.GetCities().size();

    while (true) {
        if ((GetAsyncKeyState('n') | GetAsyncKeyState('N')) & KEY_STATE_FLAG) {
            current_day = 0;
            current_city_index = (current_city_index + 1) % cities_amount;
            system("cls");
            WeatherScreen::FillInformation(config.GetCities()[current_city_index]->GetName(), current_day);
            Sleep(500);
        }
        else if ((GetAsyncKeyState('p') | GetAsyncKeyState('P')) & KEY_STATE_FLAG) {
            current_day = 0;
            current_city_index = (current_city_index - 1 + cities_amount) % cities_amount;
            system("cls");
            WeatherScreen::FillInformation(config.GetCities()[current_city_index]->GetName(), current_day);
            Sleep(500);
        }
        else if (GetAsyncKeyState(VK_OEM_PLUS) & KEY_STATE_FLAG) {
            if (current_day == 6) {
                continue;
            }
            ++current_day;
            WeatherScreen::FillInformation(config.GetCities()[current_city_index]->GetName(), current_day);
            Sleep(500);
        }
        else if (GetAsyncKeyState(VK_OEM_MINUS) & KEY_STATE_FLAG) {
            if (current_day == 0) {
                continue;
            }
            --current_day;
            system("cls");
            for (int i = 0; i <= current_day; ++i) {
                WeatherScreen::FillInformation(config.GetCities()[current_city_index]->GetName(), i);
            }
            Sleep(500);
        }
        else if (GetAsyncKeyState(VK_ESCAPE) & KEY_STATE_FLAG) {
            break;
        }
    }
}

void WeatherScreen::sleep(const Fetcher& config) {
    int duration = config.GetFrequency();
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(duration));
        mtx.lock();
        config.ForecastRequest();
        system("cls");
        for (int i = 0; i <= current_day; ++i) {
            WeatherScreen::FillInformation(config.GetCities()[current_city_index]->GetName(), i);
        }
        mtx.unlock();
        Sleep(500);
    }
}
//-------

std::string WeatherScreen::FormatDate(const std::string& original_date) {
    std::tm tm = {};
    std::istringstream ss(original_date);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M");

    std::stringstream result;
    result << std::put_time(&tm, "%d.%m.%Y");
    return result.str();
}


std::wstring WeatherScreen::TrimZeros(const std::wstring& string) {
    size_t dot_pos = string.find(L'.');
    if (dot_pos != std::wstring::npos) {
        size_t last_digit = string.find_last_not_of(L'0');
        return string.substr(0, (last_digit == dot_pos) ? dot_pos : last_digit + 1);
    }
    return string;
}

std::string WeatherScreen::WeatherCode(double weather_code) {
    std::map<int, std::string> weather_code_map = {
    {0,  "Ясное небо"},
    {1,  "В основном ясно"},
    {2,  "Частичная облачность"},
    {3,  "Пасмурно"},
    {45, "Туман"},
    {48, "Осадки в виде инея в тумане"},
    {51, "Легкая морось"},
    {53, "Умеренная морось"},
    {55, "Интенсивная морось"},
    {56, "Легкий морозный дождик"},
    {57, "Интенсивный морозный дождик"},
    {61, "Небольшой дождь"},
    {63, "Умеренный дождь"},
    {65, "Сильный дождь"},
    {66, "Легкий замораживающий дождик"},
    {67, "Сильный замораживающий дождик"},
    {71, "Небольшой снегопад"},
    {73, "Умеренный снегопад"},
    {75, "Сильный снегопад"},
    {77, "Снежные зерна"},
    {80, "Небольшие дожди"},
    {81, "Умеренные дожди"},
    {82, "Сильные дожди"},
    {85, "Небольшие снегопады"},
    {86, "Сильные снегопады"},
    {95, "Небольшая или умеренная гроза"},
    {96, "Небольшой град при грозе"},
    {99, "Сильный град при грозе"}
    };
    auto it = weather_code_map.find(static_cast<int>(weather_code));
    if (it != weather_code_map.end()) {
        return it->second;
    }
    else {
        return "Неизвестно";
    }
}

std::string WeatherScreen::WindDirection(double wind_code) {
    int sector_size = 360 / 8;
    int sector = (static_cast<int>(wind_code) + sector_size / 2) % 360 / sector_size;

    switch (sector) {
    case 0:
        return "Север";
    case 1:
        return "Северо-восток";
    case 2:
        return "Восток";
    case 3:
        return "Юго-восток";
    case 4:
        return "Юг";
    case 5:
        return "Юго-запад";
    case 6:
        return "Запад";
    case 7:
        return "Северо-запад";
    default:
        return "Неизвестное направление";
    }
}

void WeatherScreen::DrawTable(const std::string& city_name, double** city_info, const std::string& time, int current_day) {
    using namespace ftxui;

    auto city_name_element = hbox({
        text(L"Город: ") | color(Color::Cyan1),
        text(to_wstring(city_name))
        }) | center | border | size(WIDTH, EQUAL, 60) | center;

    std::vector<std::wstring> time_labels = { L"Ночь", L"Утро", L"День", L"Вечер" };
    Elements time_elements;

    std::string formatted_date = FormatDate(time);
    auto date_element = hbox({
        text(L"Дата: ") | color(Color::Pink1),
        text(to_wstring(formatted_date)),
        }) | center | border | size(WIDTH, EQUAL, 240);

    for (int time_of_day = 0; time_of_day < 4; ++time_of_day) {
        auto time_label = hbox({
            text(time_labels[time_of_day]) | color(Color::Cyan1),
            }) | center | border | size(WIDTH, EQUAL, 60);

        time_elements.push_back(time_label);
    }

    auto time_frame = hbox(time_elements) | center;

    Elements table_elements;

    for (int time_of_day = 0; time_of_day < 4; ++time_of_day) {
        std::string weather_code = WeatherCode(city_info[time_of_day][0]);
        std::string wind_direction = WindDirection(city_info[time_of_day][4]);
        auto row = vbox({
            hbox({
                text(L"Погода: ") | color(Color::Green),
                text(to_wstring(weather_code)),
            }),
            hbox({
                text(L"Температура: ") | color(Color::Yellow),
                text(TrimZeros(std::to_wstring(city_info[time_of_day][1])) + L" °C"),
            }),
            hbox({
                text(L"Ощущается как: ") | color(Color::Yellow),
                text(TrimZeros(std::to_wstring(city_info[time_of_day][2])) + L" °C"),
            }),
            hbox({
                text(L"Скорость ветра: ") | color(Color::Yellow),
                text(TrimZeros(std::to_wstring(city_info[time_of_day][3])) + L" км\\ч"),
            }),
            hbox({
                text(L"Направление ветра: ") | color(Color::MediumPurple1),
                text(to_wstring(wind_direction)),
            }),
            hbox({
                text(L"Вероятность осадков: ") | color(Color::MediumPurple1),
                text(TrimZeros(std::to_wstring(city_info[time_of_day][5])) + L" мм | " + TrimZeros(std::to_wstring(city_info[time_of_day][6])) + L"%"),
            }),
            }) | border | size(WIDTH, EQUAL, 60);

        table_elements.push_back(row);
    }

    auto table = hbox(table_elements);

    if (current_day == 0) {
        auto document = vbox({
            city_name_element,
            text(L""),
            text(L""),
            date_element,
            time_frame,
            table,
            });
        auto screen = Screen::Create(
            Dimension::Full(),
            Dimension::Fit(document)
        );
        Render(screen, document);
        screen.Print();
    }
    else {
        auto document = vbox({
            date_element,
            time_frame,
            table,
            });
        auto screen = Screen::Create(
            Dimension::Full(),
            Dimension::Fit(document)
        );
        Render(screen, document);
        screen.Print();
    }
}

void WeatherScreen::FillInformation(const std::string& city_name, int current_day) {
    std::ifstream weather;
    weather.open("weather.json", std::ios::in);
    nlohmann::json json_weather = nlohmann::json::parse(weather);

    double** city_info = new double* [4];
    for (int i = 0; i < 4; ++i) {
        city_info[i] = new double[7];
    }

    nlohmann::json city_forecast = json_weather[city_name]["hourly"];
    int time_of_day = 0;
    for (int time_period = 2 + (current_day * 24); time_period < 24 + (current_day * 24); time_period += 6) {
        int weather_code = city_forecast["weather_code"][time_period];
        double temperature = city_forecast["temperature_2m"][time_period];
        double apparent_temperature = city_forecast["apparent_temperature"][time_period];
        double wind_speed = city_forecast["wind_speed_10m"][time_period];
        int wind_direction = city_forecast["wind_direction_10m"][time_period];
        double precipitation = city_forecast["precipitation"][time_period];
        int precipitation_probability = city_forecast["precipitation_probability"][time_period];
        std::string time = city_forecast["time"][time_period];


        city_info[time_of_day][0] = weather_code;
        city_info[time_of_day][1] = temperature;
        city_info[time_of_day][2] = apparent_temperature;
        city_info[time_of_day][3] = wind_speed;
        city_info[time_of_day][4] = wind_direction;
        city_info[time_of_day][5] = precipitation;
        city_info[time_of_day][6] = precipitation_probability;

        ++time_of_day;
        if (time_of_day == 4) {
            DrawTable(city_name, city_info, time, current_day);
            time_of_day = 0;
            for (int i = 0; i < 4; ++i) {
                delete[] city_info[i];
            }
            delete[] city_info;
        }
    }
}

