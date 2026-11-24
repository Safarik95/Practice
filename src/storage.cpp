#include "storage.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>

#include <cmath>
#include <limits>

namespace {

bool readInt(const QJsonObject &object, const QString &key, int &value)
{
    const QJsonValue item = object.value(key);
    if (!item.isDouble())
        return false;
    const double number = item.toDouble();
    if (number != std::floor(number) || number < std::numeric_limits<int>::min()
        || number > std::numeric_limits<int>::max())
        return false;
    value = static_cast<int>(number);
    return true;
}

bool readString(const QJsonObject &object, const QString &key, QString &value)
{
    const QJsonValue item = object.value(key);
    if (!item.isString())
        return false;
    value = item.toString();
    return true;
}

}

QByteArray Storage::toJson(const Program &program)
{
    QJsonArray tracks;
    for (const Track &track : program.tracks()) {
        tracks.append(QJsonObject{{"code", track.code},
                                  {"title", track.title},
                                  {"artist", track.artist},
                                  {"duration", track.duration}});
    }
    const QJsonObject root{{"formatVersion", formatVersion},
                           {"variant", variant},
                           {"state", QJsonObject{{"tracks", tracks}}}};
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

Result Storage::fromJson(const QByteArray &data, Program &target)
{
    if (data.size() > maxFileSize)
        return Result::failure("файл больше 1 МиБ.");

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError)
        return Result::failure(QString("файл повреждён: %1.").arg(error.errorString()));
    if (!document.isObject())
        return Result::failure("файл повреждён: ожидался JSON-объект.");

    const QJsonObject root = document.object();
    int version = 0;
    if (!readInt(root, "formatVersion", version))
        return Result::failure("нет корректного поля formatVersion.");
    if (version != formatVersion)
        return Result::failure(QString("неподдерживаемая версия формата %1.").arg(version));

    int fileVariant = 0;
    if (!readInt(root, "variant", fileVariant))
        return Result::failure("нет корректного поля variant.");
    if (fileVariant != variant)
        return Result::failure(
            QString("файл относится к варианту %1, а не %2.").arg(fileVariant).arg(variant));

    const QJsonValue state = root.value("state");
    if (!state.isObject())
        return Result::failure("нет объекта state.");
    const QJsonValue tracksValue = state.toObject().value("tracks");
    if (!tracksValue.isArray())
        return Result::failure("в state нет массива tracks.");
    const QJsonArray tracks = tracksValue.toArray();
    if (tracks.size() > Program::maxTracks)
        return Result::failure(QString("в файле больше %1 номеров.").arg(Program::maxTracks));

    Program loaded;
    for (qsizetype i = 0; i < tracks.size(); ++i) {
        const QString place = QString("запись %1: ").arg(i + 1);
        if (!tracks.at(i).isObject())
            return Result::failure(place + "ожидался объект.");
        const QJsonObject object = tracks.at(i).toObject();
        Track track;
        if (!readInt(object, "code", track.code))
            return Result::failure(place + "поле code должно быть целым числом.");
        if (!readString(object, "title", track.title))
            return Result::failure(place + "поле title должно быть строкой.");
        if (!readString(object, "artist", track.artist))
            return Result::failure(place + "поле artist должно быть строкой.");
        if (!readInt(object, "duration", track.duration))
            return Result::failure(place + "поле duration должно быть целым числом.");
        const Result result = loaded.addBack(track);
        if (!result.ok)
            return Result::failure(place + result.message);
    }

    target = std::move(loaded);
    return Result::success();
}

Result Storage::save(const Program &program, const QString &path)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return Result::failure(QString("не удалось открыть файл для записи: %1.").arg(file.errorString()));
    const QByteArray data = toJson(program);
    if (file.write(data) != data.size()) {
        file.cancelWriting();
        return Result::failure(QString("ошибка записи: %1.").arg(file.errorString()));
    }
    if (!file.commit())
        return Result::failure(QString("не удалось сохранить файл: %1.").arg(file.errorString()));
    return Result::success();
}

Result Storage::load(const QString &path, Program &target)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return Result::failure(QString("не удалось открыть файл: %1.").arg(file.errorString()));
    if (file.size() > maxFileSize)
        return Result::failure("файл больше 1 МиБ.");
    return fromJson(file.readAll(), target);
}
