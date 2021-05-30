#include "log.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

#include <string_view>

namespace tt::log {
void log(const Level level, const Subsystem subsystem, const std::string_view& msg) {
    fmt::color color;
    switch (level) {
        case Level::Debug:
            color = Debug_Color;
            break;
        case Level::Warning:
            color = Warn_Color;
            break;
        case Level::Fatal:
            color = Fatal_Color;
            break;
        default:
            color = fmt::color::white;
    }
    fmt::print(fg(color), "[{}], [{}] {}\n", level, subsystem, msg);
}

}  // namespace tt::log