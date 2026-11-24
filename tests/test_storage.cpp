#include "storage.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

namespace {

QByteArray wrap(const QByteArray &tracks, const QByteArray &version = "1",
                const QByteArray &variant = "2")
{
    return "{\"formatVersion\": " + version + ", \"variant\": " + variant
           + ", \"state\": {\"tracks\": " + tracks + "}}";
}

Program sampleProgram()
{
    Program program;
    program.addBack(Track{11, "Вступление", "Оркестр", 120});
    program.addBack(Track{12, "Весенний вальс", "Анна Петрова", 185});
    program.addFront(Track{10, "Увертюра", "Оркестр", 240});
    return program;
}

QString dataFile(const QString &name)
{
    return QString(DATA_DIR) + "/" + name;
}

}

class TestStorage : public QObject
{
    Q_OBJECT

private slots:
    void saveAndLoadRestoresState();
    void saveAndLoadEmptyProgram();
    void rejectsBrokenJson();
    void rejectsWrongVersionAndVariant_data();
    void rejectsWrongVersionAndVariant();
    void rejectsInvalidTrackData_data();
    void rejectsInvalidTrackData();
    void rejectsTooLargeFile();
    void rejectsMissingFile();
    void loadsExampleFiles();
};

void TestStorage::saveAndLoadRestoresState()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("program.json");
    const Program original = sampleProgram();

    QVERIFY(Storage::save(original, path).ok);
    Program loaded;
    const Result result = Storage::load(path, loaded);
    QVERIFY2(result.ok, qPrintable(result.message));
    QCOMPARE(loaded.size(), 3);
    QVERIFY(loaded.tracks() == original.tracks());
    QCOMPARE(loaded.totalDuration(), qint64(545));
}

void TestStorage::saveAndLoadEmptyProgram()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("empty.json");
    QVERIFY(Storage::save(Program{}, path).ok);

    Program loaded = sampleProgram();
    QVERIFY(Storage::load(path, loaded).ok);
    QVERIFY(loaded.isEmpty());
}

void TestStorage::rejectsBrokenJson()
{
    Program target = sampleProgram();
    const Result result = Storage::fromJson("{\"formatVersion\": 1, \"variant\": 2, \"sta", target);
    QVERIFY(!result.ok);
    QVERIFY(!result.message.isEmpty());
    QVERIFY(!Storage::fromJson("[1, 2, 3]", target).ok);
    QVERIFY(!Storage::fromJson("", target).ok);
    QVERIFY(target.tracks() == sampleProgram().tracks());
}

void TestStorage::rejectsWrongVersionAndVariant_data()
{
    QTest::addColumn<QByteArray>("json");
    QTest::newRow("version 2") << wrap("[]", "2");
    QTest::newRow("version as string") << wrap("[]", "\"1\"");
    QTest::newRow("fractional version") << wrap("[]", "1.5");
    QTest::newRow("variant 3") << wrap("[]", "1", "3");
    QTest::newRow("no variant") << QByteArray("{\"formatVersion\": 1, \"state\": {\"tracks\": []}}");
    QTest::newRow("no state") << QByteArray("{\"formatVersion\": 1, \"variant\": 2}");
    QTest::newRow("tracks not array") << wrap("{}");
}

void TestStorage::rejectsWrongVersionAndVariant()
{
    QFETCH(QByteArray, json);
    Program target = sampleProgram();
    QVERIFY(!Storage::fromJson(json, target).ok);
    QVERIFY(target.tracks() == sampleProgram().tracks());
}

void TestStorage::rejectsInvalidTrackData_data()
{
    QTest::addColumn<QByteArray>("tracks");
    QTest::newRow("duplicate code")
        << QByteArray(R"([{"code": 1, "title": "А", "artist": "Б", "duration": 60},
                          {"code": 1, "title": "В", "artist": "Г", "duration": 60}])");
    QTest::newRow("zero duration")
        << QByteArray(R"([{"code": 1, "title": "А", "artist": "Б", "duration": 0}])");
    QTest::newRow("too long duration")
        << QByteArray(R"([{"code": 1, "title": "А", "artist": "Б", "duration": 86401}])");
    QTest::newRow("negative code")
        << QByteArray(R"([{"code": -4, "title": "А", "artist": "Б", "duration": 60}])");
    QTest::newRow("fractional code")
        << QByteArray(R"([{"code": 1.5, "title": "А", "artist": "Б", "duration": 60}])");
    QTest::newRow("code as string")
        << QByteArray(R"([{"code": "1", "title": "А", "artist": "Б", "duration": 60}])");
    QTest::newRow("missing title")
        << QByteArray(R"([{"code": 1, "artist": "Б", "duration": 60}])");
    QTest::newRow("empty artist")
        << QByteArray(R"([{"code": 1, "title": "А", "artist": "   ", "duration": 60}])");
    QTest::newRow("track not object") << QByteArray("[42]");
}

void TestStorage::rejectsInvalidTrackData()
{
    QFETCH(QByteArray, tracks);
    Program target = sampleProgram();
    const Result result = Storage::fromJson(wrap(tracks), target);
    QVERIFY(!result.ok);
    QVERIFY(result.message.contains("запись"));
    QVERIFY(target.tracks() == sampleProgram().tracks());
}

void TestStorage::rejectsTooLargeFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("large.json");
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(wrap("[]"));
    file.write(QByteArray(Storage::maxFileSize, ' '));
    file.close();

    Program target = sampleProgram();
    QVERIFY(!Storage::load(path, target).ok);
    QVERIFY(target.tracks() == sampleProgram().tracks());
}

void TestStorage::rejectsMissingFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Program target = sampleProgram();
    QVERIFY(!Storage::load(dir.filePath("absent.json"), target).ok);
    QCOMPARE(target.size(), 3);
}

void TestStorage::loadsExampleFiles()
{
    Program program;
    const Result result = Storage::load(dataFile("example.json"), program);
    QVERIFY2(result.ok, qPrintable(result.message));
    QCOMPARE(program.size(), 5);
    QCOMPARE(program.totalDuration(), qint64(1085));
    QCOMPARE(program.tracks().front().code, 101);

    QVERIFY(!Storage::load(dataFile("broken.json"), program).ok);
    QVERIFY(!Storage::load(dataFile("wrong_version.json"), program).ok);
    QVERIFY(!Storage::load(dataFile("duplicate_code.json"), program).ok);
    QCOMPARE(program.size(), 5);
}

QTEST_GUILESS_MAIN(TestStorage)

#include "test_storage.moc"
