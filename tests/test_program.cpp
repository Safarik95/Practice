#include "program.h"

#include <QtTest>

namespace {

Track makeTrack(int code, const QString &title, int duration = 60)
{
    return Track{code, title, "Исполнитель", duration};
}

Program makeProgram(const QStringList &titles)
{
    Program program;
    int code = 1;
    for (const QString &title : titles)
        program.addBack(makeTrack(code++, title));
    return program;
}

template <typename Container>
QStringList titlesOf(const Container &tracks)
{
    QStringList result;
    for (const Track &track : tracks)
        result << track.title;
    return result;
}

}

class TestProgram : public QObject
{
    Q_OBJECT

private slots:
    void emptyAndMinimalProgram();
    void addFrontAndBack();
    void rejectsDuplicateCode();
    void rejectsInvalidText();
    void durationBounds();
    void codeBounds();
    void removeFirstAndLast();
    void searchIgnoresCase();
    void reverseViewKeepsOrder();
    void moveFirstToEnd();
    void moveRejectsInvalidK();
    void totalDuration();
    void changeDuration();
    void trackLimit();
};

void TestProgram::emptyAndMinimalProgram()
{
    Program program;
    QVERIFY(program.isEmpty());
    QCOMPARE(program.size(), 0);
    QCOMPARE(program.totalDuration(), qint64(0));
    QVERIFY(!program.removeFirst().ok);
    QVERIFY(!program.removeLast().ok);
    QVERIFY(program.search("").empty());
    QVERIFY(program.reversed().empty());

    QVERIFY(program.addBack(makeTrack(7, "Единственный", 45)).ok);
    QCOMPARE(program.size(), 1);
    QCOMPARE(program.totalDuration(), qint64(45));
    QVERIFY(program.removeLast().ok);
    QVERIFY(program.isEmpty());
}

void TestProgram::addFrontAndBack()
{
    Program program;
    QVERIFY(program.addBack(makeTrack(1, "Б")).ok);
    QVERIFY(program.addFront(makeTrack(2, "А")).ok);
    QVERIFY(program.addBack(makeTrack(3, "В")).ok);
    QCOMPARE(titlesOf(program.tracks()), (QStringList{"А", "Б", "В"}));
}

void TestProgram::rejectsDuplicateCode()
{
    Program program;
    QVERIFY(program.addBack(makeTrack(101, "Первая", 60)).ok);
    const Result front = program.addFront(makeTrack(101, "Другая", 30));
    const Result back = program.addBack(makeTrack(101, "Третья", 30));
    QVERIFY(!front.ok);
    QVERIFY(!back.ok);
    QVERIFY(!front.message.isEmpty());
    QCOMPARE(program.size(), 1);
    QCOMPARE(program.tracks().front().title, QString("Первая"));
}

void TestProgram::rejectsInvalidText()
{
    Program program;
    QVERIFY(!program.addBack(Track{1, "", "Исполнитель", 60}).ok);
    QVERIFY(!program.addBack(Track{1, "   ", "Исполнитель", 60}).ok);
    QVERIFY(!program.addBack(Track{1, "Песня", "  ", 60}).ok);
    QVERIFY(!program.addBack(Track{1, QString(101, QChar(u'я')), "Исполнитель", 60}).ok);
    QVERIFY(!program.addBack(Track{1, "Песня", QString(101, QChar(u'я')), 60}).ok);
    QVERIFY(program.isEmpty());

    QVERIFY(program.addBack(Track{1, "  " + QString(100, QChar(u'я')) + "  ", "  Хор  ", 60}).ok);
    QCOMPARE(program.tracks().front().title.size(), 100);
    QCOMPARE(program.tracks().front().artist, QString("Хор"));
}

void TestProgram::durationBounds()
{
    Program program;
    QVERIFY(!program.addBack(makeTrack(1, "А", 0)).ok);
    QVERIFY(!program.addBack(makeTrack(1, "А", -5)).ok);
    QVERIFY(!program.addBack(makeTrack(1, "А", 86401)).ok);
    QVERIFY(program.isEmpty());
    QVERIFY(program.addBack(makeTrack(1, "А", 1)).ok);
    QVERIFY(program.addBack(makeTrack(2, "Б", 86400)).ok);
    QCOMPARE(program.totalDuration(), qint64(86401));
}

void TestProgram::codeBounds()
{
    Program program;
    QVERIFY(!program.addBack(makeTrack(0, "А")).ok);
    QVERIFY(!program.addBack(makeTrack(-1, "А")).ok);
    QVERIFY(!program.addBack(makeTrack(1000001, "А")).ok);
    QVERIFY(program.isEmpty());
    QVERIFY(program.addBack(makeTrack(1, "А")).ok);
    QVERIFY(program.addBack(makeTrack(1000000, "Б")).ok);
    QCOMPARE(program.size(), 2);
}

void TestProgram::removeFirstAndLast()
{
    Program program = makeProgram({"А", "Б", "В"});
    QVERIFY(program.removeFirst().ok);
    QCOMPARE(titlesOf(program.tracks()), (QStringList{"Б", "В"}));
    QVERIFY(program.removeLast().ok);
    QCOMPARE(titlesOf(program.tracks()), (QStringList{"Б"}));
    QVERIFY(program.removeFirst().ok);
    QVERIFY(!program.removeLast().ok);
}

void TestProgram::searchIgnoresCase()
{
    Program program = makeProgram({"Летний Дождь", "Зима", "дождь в городе"});
    QCOMPARE(titlesOf(program.search("ДОЖДЬ")), (QStringList{"Летний Дождь", "дождь в городе"}));
    QCOMPARE(titlesOf(program.search("  зим ")), (QStringList{"Зима"}));
    QCOMPARE(program.search("").size(), std::size_t(3));
    QVERIFY(program.search("снег").empty());
    QCOMPARE(titlesOf(program.tracks()), (QStringList{"Летний Дождь", "Зима", "дождь в городе"}));
}

void TestProgram::reverseViewKeepsOrder()
{
    Program program = makeProgram({"А", "Б", "В"});
    QCOMPARE(titlesOf(program.reversed()), (QStringList{"В", "Б", "А"}));
    QCOMPARE(titlesOf(program.tracks()), (QStringList{"А", "Б", "В"}));
}

void TestProgram::moveFirstToEnd()
{
    Program program = makeProgram({"А", "Б", "В", "Г", "Д"});
    QVERIFY(program.moveFirstToEnd(2).ok);
    QCOMPARE(titlesOf(program.tracks()), (QStringList{"В", "Г", "Д", "А", "Б"}));
    QVERIFY(program.moveFirstToEnd(0).ok);
    QCOMPARE(titlesOf(program.tracks()), (QStringList{"В", "Г", "Д", "А", "Б"}));
    QVERIFY(program.moveFirstToEnd(5).ok);
    QCOMPARE(titlesOf(program.tracks()), (QStringList{"В", "Г", "Д", "А", "Б"}));
}

void TestProgram::moveRejectsInvalidK()
{
    Program program = makeProgram({"А", "Б", "В"});
    QVERIFY(!program.moveFirstToEnd(-1).ok);
    QVERIFY(!program.moveFirstToEnd(4).ok);
    QCOMPARE(titlesOf(program.tracks()), (QStringList{"А", "Б", "В"}));

    Program empty;
    QVERIFY(empty.moveFirstToEnd(0).ok);
    QVERIFY(!empty.moveFirstToEnd(1).ok);
}

void TestProgram::totalDuration()
{
    Program program;
    QVERIFY(program.addBack(makeTrack(1, "А", 60)).ok);
    QVERIFY(program.addBack(makeTrack(2, "Б", 90)).ok);
    QVERIFY(program.addBack(makeTrack(3, "В", 30)).ok);
    QCOMPARE(program.totalDuration(), qint64(180));
    QVERIFY(!program.addBack(makeTrack(2, "Повтор", 500)).ok);
    QCOMPARE(program.totalDuration(), qint64(180));
}

void TestProgram::changeDuration()
{
    Program program;
    QVERIFY(program.addBack(makeTrack(1, "А", 60)).ok);
    QVERIFY(program.addBack(makeTrack(2, "Б", 90)).ok);
    QVERIFY(program.changeDuration(2, 120).ok);
    QCOMPARE(program.findByCode(2)->duration, 120);
    QCOMPARE(program.totalDuration(), qint64(180));

    const Result unknown = program.changeDuration(99, 100);
    QVERIFY(!unknown.ok);
    QVERIFY(unknown.message.contains("99"));
    QVERIFY(!program.changeDuration(1, 0).ok);
    QVERIFY(!program.changeDuration(1, 86401).ok);
    QCOMPARE(program.findByCode(1)->duration, 60);
}

void TestProgram::trackLimit()
{
    Program program;
    for (int code = 1; code <= Program::maxTracks; ++code)
        QVERIFY(program.addBack(makeTrack(code, QString("Номер %1").arg(code))).ok);
    QCOMPARE(program.size(), Program::maxTracks);
    QVERIFY(!program.addBack(makeTrack(5000, "Лишний")).ok);
    QVERIFY(!program.addFront(makeTrack(5001, "Лишний")).ok);
    QCOMPARE(program.size(), Program::maxTracks);
    QVERIFY(program.removeFirst().ok);
    QVERIFY(program.addFront(makeTrack(5000, "Теперь помещается")).ok);
}

QTEST_GUILESS_MAIN(TestProgram)

#include "test_program.moc"
