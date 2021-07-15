//
// Created by Matt on 5/8/2020.
//

#pragma once

namespace rocor
{
    typedef enum
    {
        FRONT_L = 0, FRONT_R, BACK_L, BACK_R, LISTEN_L, LISTEN_R, LISTEN_BACK, LISTEN_FRONT
    } CapturePosition;

    static constexpr std::array<CapturePosition, 8> ordered_capture_positions =
        { FRONT_L, FRONT_R, BACK_L, BACK_R, LISTEN_L, LISTEN_R, LISTEN_BACK, LISTEN_FRONT };

    static constexpr std::array<const char*, 8> capture_position_names =
    {
        "FRONT_L",
        "FRONT_R",
        "BACK_L",
        "BACK_R",
        "LISTEN_L",
        "LISTEN_R",
        "LISTEN_BACK",
        "LISTEN_FRONT"
    };

    template <typename MapType>
    using CaptureMap = std::map<CapturePosition, MapType>;
}
