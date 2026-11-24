#pragma once

#include <QString>

struct Result {
    bool ok = true;
    QString message;

    static Result success() { return {}; }
    static Result failure(const QString &text) { return {false, text}; }
};
