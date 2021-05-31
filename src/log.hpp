#pragma once

#include <fmt/color.h>
#include <fmt/format.h>

#include <string_view>

namespace tt::log {
const fmt::color Warn_Color = fmt::color::orange;
const fmt::color Debug_Color = fmt::color::white;
const fmt::color Fatal_Color = fmt::color::red;

enum class Level { Debug, Warning, Fatal };

enum class Subsystem { Peer, Tracker, Torrent };

void log(const Level level, const Subsystem subsystem, const std::string_view& msg);

}  // namespace tt::log

// --- Formatting boilerplate ---

// This has to be specialized in the global namespace
template <>
struct fmt::formatter<tt::log::Level> : fmt::formatter<string_view> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(tt::log::Level l, FormatContext& ctx) {
        string_view name = "UNKNOWN";
        switch (l) {
            case tt::log::Level::Debug:
                name = "DEBUG";
                break;
            case tt::log::Level::Warning:
                name = "WARN";
                break;
            case tt::log::Level::Fatal:
                name = "FATAL";
                break;
            default:
                name = "UNKNOWN";
                break;
        }
        return fmt::formatter<string_view>::format(name, ctx);
    }
};

template <>
struct fmt::formatter<tt::log::Subsystem> : fmt::formatter<string_view> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(tt::log::Subsystem s, FormatContext& ctx) {
        string_view name = "UNKNOWN";
        switch (s) {
            case tt::log::Subsystem::Peer:
                name = "PEER";
                break;
            case tt::log::Subsystem::Tracker:
                name = "TRACKER";
                break;
            case tt::log::Subsystem::Torrent:
                name = "TORRENT";
                break;
            default:
                name = "UNKNOWN";
                break;
        }
        return fmt::formatter<string_view>::format(name, ctx);
    }
};