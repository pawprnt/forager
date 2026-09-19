#pragma once

#include <QString>
#include <utility>
#include <vector>

class QApplication;

namespace theme {
    namespace C {
        constexpr const char* BG = "#0a0a0a";
        constexpr const char* COLOR_1 = "#111111";
        constexpr const char* COLOR_2 = "#1e1e1e";
        constexpr const char* COLOR_3 = "#141414";
        constexpr const char* COLOR_4 = "#181818";
        constexpr const char* COLOR_5 = "#26292c";
        constexpr const char* COLOR_6 = "#262629";
        constexpr const char* ACCENT_1 = "#666cff";
        constexpr const char* ACCENT_2 = "#878cff";
        constexpr const char* RED = "#f04a4a";
        constexpr const char* RED_HOVER = "#f26363";
        constexpr const char* GREEN = "#24a65a";
        constexpr const char* GREEN_HOVER = "#27b964";
        constexpr const char* BLUE = "#4b89ef";
        constexpr const char* BLUE_HOVER = "#649af2";
        constexpr const char* YELLOW = "#ef8d4b";
        constexpr const char* TEXT = "#ffffff";
        constexpr const char* TEXT_DIM = "#8e8e8e";
        constexpr const char* TEXT_MUTED = "#a3aab9";
        constexpr int RADIUS = 8;
    } // namespace C

    struct DisplaySize {
        const char* key;
        const char* label;
        int width;
        int height;
    };

    inline const DisplaySize DISPLAY_SIZES[] = {
        {"small",  "Small",  120, 180},
        {"medium", "Medium", 165, 248},
        {"large",  "Large",  250, 375},
    };

    inline std::pair<int,int> resolveCardSize(const QString& key) {
        for (const auto& s : DISPLAY_SIZES) {
            if (key == s.key) return {s.width, s.height};
        }
        return {165, 248};
    }

    void applyTheme(QApplication* app);
    QString stylesheet();
} // namespace theme
