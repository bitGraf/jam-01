#include "Save_Data.h"

#include "logging.h"

// json parser
#include <fstream>
#include <json.hpp>
using json = nlohmann::json;

//////////////////////////////////////////////////////////////////////////////////
// Sys_Config_Data
//////////////////////////////////////////////////////////////////////////////////

bool Sys_Config_Data::Serialize_From_Json(const char* filename) {
    std::ifstream file(filename);
    try {
        if (!file.is_open()) {return false;}

        json data = json::parse(file);

        // Window Title
        if (data.find("Title") != data.end()) {
            window_title = data["Title"].get<std::string>();
        }
        
        // if not find width or height, use default
        if (data.find("Window_Width") != data.end()) {
            window_width = data["Window_Width"].get<uint16>();
        }
        if (data.find("Window_Height") != data.end()) {
            window_height = data["Window_Height"].get<uint16>();
        }

        // Master_Volume:
        if (data.find("Master_Volume") != data.end()) {
            master_volume = data["Master_Volume"].get<int32>();
        }

        if (data.find("Music_Volume") != data.end()) {
            music_volume = data["Music_Volume"].get<int32>();
        }
    } catch (json::parse_error e) {
        log_error("Json parse exception: [%s]", e.what());
        return false;
    } catch (json::type_error e) {
        log_error("Json type exception: [%s]", e.what());
        return false;
    } catch (json::other_error e) {
        log_error("Json other exception: [%s]", e.what());
        return false;
    }

    return true;
}
bool Sys_Config_Data::Serialize_To_Json(const char* filename) const {
    try {
        json data = {
            {"Title", window_title},
            {"Window_Width", window_width},
            {"Window_Height", window_height},
            {"Master_Volume", master_volume},
            {"Music_Volume",  music_volume}
        };

        std::ofstream fp(filename);
        fp << std::setw(2) << data << std::endl;
        fp.close();
        log_info("Sys config written to '%s'", filename);
    } catch (json::parse_error e) {
        log_error("Json parse exception: [%s]", e.what());
        return false;
    } catch (json::type_error e) {
        log_error("Json type exception: [%s]", e.what());
        return false;
    } catch (json::other_error e) {
        log_error("Json other exception: [%s]", e.what());
        return false;
    }

    return true;
}




//////////////////////////////////////////////////////////////////////////////////
// Game_Save_Data
//////////////////////////////////////////////////////////////////////////////////

bool Game_Save_Data::Serialize_From_Json(const char* filename) {
    std::ifstream file(filename);
    try {
        if (!file.is_open()) {return false;}

        json data = json::parse(file);

        // Food
        if (data.find("Food") != data.end()) {
            Food = data["Food"].get<int16>();
        }
    } catch (json::parse_error e) {
        log_error("Json parse exception: [%s]", e.what());
        return false;
    } catch (json::type_error e) {
        log_error("Json type exception: [%s]", e.what());
        return false;
    } catch (json::other_error e) {
        log_error("Json other exception: [%s]", e.what());
        return false;
    }

    return true;
}
bool Game_Save_Data::Serialize_To_Json(const char* filename) const {
    try {
        json data = {
            {"Food", Food}
        };

        std::ofstream fp(filename);
        fp << std::setw(2) << data << std::endl;
        fp.close();
        log_info("Sys config written to '%s'", filename);
    } catch (json::parse_error e) {
        log_error("Json parse exception: [%s]", e.what());
        return false;
    } catch (json::type_error e) {
        log_error("Json type exception: [%s]", e.what());
        return false;
    } catch (json::other_error e) {
        log_error("Json other exception: [%s]", e.what());
        return false;
    }

    return true;
}