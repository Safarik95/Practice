#pragma once

#include <QString>

struct Track {
    int code = 0;
    QString title;
    QString artist;
    int duration = 0;

    bool operator==(const Track &other) const = default;
};
