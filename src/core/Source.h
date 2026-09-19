#pragma once

#include <cstdint>

enum class Source : uint8_t {
    Steam,
    Minecraft,
    Standalone,
};

inline const char* sourceName(Source s) {
    switch (s) {
        case Source::Steam:      return "Steam";
        case Source::Minecraft:  return "Minecraft";
        case Source::Standalone: return "Standalone";
    }
    return "Unknown";
}
