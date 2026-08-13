#include "FileContextState.h"

#include <chrono>
#include <cstdio>
#include <ctime>

std::string FileContextState::permissionsToString(std::filesystem::perms p) {
    using std::filesystem::perms;
    std::string result = "---------";
    if ((p & perms::owner_read) != perms::none) result[0] = 'r';
    if ((p & perms::owner_write) != perms::none) result[1] = 'w';
    if ((p & perms::owner_exec) != perms::none) result[2] = 'x';
    if ((p & perms::group_read) != perms::none) result[3] = 'r';
    if ((p & perms::group_write) != perms::none) result[4] = 'w';
    if ((p & perms::group_exec) != perms::none) result[5] = 'x';
    if ((p & perms::others_read) != perms::none) result[6] = 'r';
    if ((p & perms::others_write) != perms::none) result[7] = 'w';
    if ((p & perms::others_exec) != perms::none) result[8] = 'x';
    return result;
}

namespace {
    std::string jsonEscape(const std::string& input)
    {
        std::string out;
        out.reserve(input.size() + 8);
        for (char c : input)
        {
            switch (c)
            {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20)
                    {
                        char buf[8];
                        std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                        out += buf;
                    }
                    else
                    {
                        out += c;
                    }
            }
        }
        return out;
    }

    std::string fileTimeToIso8601(std::filesystem::file_time_type ftime)
    {
        #if defined(__cpp_lib_chrono) && __cpp_lib_chrono >= 201907L
                // C++20:
                auto sctp = std::chrono::clock_cast<std::chrono::system_clock>(ftime);
        #else
                // C++17: file_clock & system_clock
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - std::filesystem::file_time_type::clock::now()
                          + std::chrono::system_clock::now());
        #endif
                std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
                std::tm tm{};
        #ifdef _WIN32
                gmtime_s(&tm, &tt);
        #else
                gmtime_r(&tt, &tm);
        #endif
                char buf[32];
                std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
                return buf;
    }


    std::string toString(FileType type)
    {
        switch (type)
        {
            case FileType::RegularFile: return "regular_file";
            case FileType::Directory:   return "directory";
            default:                    return "unknown";
        }
    }
}

std::string FileSystemEntry::toJson() const {
    {
        std::string json;
        json.reserve(128);
        json += "{\"path\":\"";
        json += jsonEscape(path.generic_string());
        json += "\",\"type\":\"";
        json += toString(type);
        if (type == FileType::RegularFile) {
            json += "\",\"extension\":\"";
            json += jsonEscape(extension);
        }
        json += "\"}";
        return json;
    }
}

std::string FileContextState::getMetaDataJson() const
{
    std::string json;
    json.reserve(512 + 128 * (neighbors.size() + entries.size()));

    json += "{";

    //targetFile metaData
    json += "\"path\":\"" + jsonEscape(path.generic_string()) + "\",";
    json += "\"type\":\"" + toString(type) + "\",";
    if (type == FileType::RegularFile)
        json += "\"extension\":\"" + jsonEscape(extension) + "\",";
    json += "\"file_size\":" + std::to_string(fileSizeInBytes) + ",";
    json += "\"last_modified\":\"" + fileTimeToIso8601(lastModifiedTime) + "\",";
    json += "\"permission\":\"" + permissionsToString(permission) + "\",";

    //neighbors metaData
    json += "\"neighbors\":[";
    for (std::size_t i = 0; i < neighbors.size(); ++i)
    {
        json += neighbors[i].toJson();
        if (i < neighbors.size() - 1) json += ",";

    }
    json += "],";

    //entries metaData (if its directory)
    json += "\"entries\":[";

    for (std::size_t i = 0; i < entries.size(); ++i){
        json += entries[i].toJson();
        if (i < entries.size() - 1) json += ",";
    }

    json += "]}";
    return json;
}