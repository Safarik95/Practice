#pragma once

#include "program.h"
#include "result.h"

#include <QByteArray>
#include <QString>

namespace Storage {

constexpr int formatVersion = 1;
constexpr int variant = 2;
constexpr qint64 maxFileSize = 1024 * 1024;

QByteArray toJson(const Program &program);
Result fromJson(const QByteArray &data, Program &target);
Result save(const Program &program, const QString &path);
Result load(const QString &path, Program &target);

}
