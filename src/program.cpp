#include "program.h"

#include <algorithm>
#include <iterator>
#include <numeric>

Result Program::validate(Track &track)
{
    track.title = track.title.trimmed();
    track.artist = track.artist.trimmed();
    if (track.code < 1 || track.code > maxCode)
        return Result::failure(QString("код должен быть целым числом от 1 до %1.").arg(maxCode));
    if (track.title.isEmpty())
        return Result::failure("название не может быть пустым.");
    if (track.title.size() > maxTextLength)
        return Result::failure(QString("название длиннее %1 символов.").arg(maxTextLength));
    if (track.artist.isEmpty())
        return Result::failure("исполнитель не может быть пустым.");
    if (track.artist.size() > maxTextLength)
        return Result::failure(QString("имя исполнителя длиннее %1 символов.").arg(maxTextLength));
    if (track.duration < 1 || track.duration > maxDuration)
        return Result::failure(
            QString("длительность должна быть от 1 до %1 секунд.").arg(maxDuration));
    return Result::success();
}

Result Program::prepare(Track &track) const
{
    const Result result = validate(track);
    if (!result.ok)
        return result;
    if (size() >= maxTracks)
        return Result::failure(QString("в программе уже %1 номеров, это предел.").arg(maxTracks));
    if (findByCode(track.code) != nullptr)
        return Result::failure(QString("номер с кодом %1 уже есть в программе.").arg(track.code));
    return Result::success();
}

Result Program::addFront(const Track &track)
{
    Track prepared = track;
    const Result result = prepare(prepared);
    if (result.ok)
        items.push_front(std::move(prepared));
    return result;
}

Result Program::addBack(const Track &track)
{
    Track prepared = track;
    const Result result = prepare(prepared);
    if (result.ok)
        items.push_back(std::move(prepared));
    return result;
}

Result Program::removeFirst()
{
    if (items.empty())
        return Result::failure("программа пуста, отменять нечего.");
    items.pop_front();
    return Result::success();
}

Result Program::removeLast()
{
    if (items.empty())
        return Result::failure("программа пуста, отменять нечего.");
    items.pop_back();
    return Result::success();
}

Result Program::moveFirstToEnd(int k)
{
    if (k < 0 || k > size())
        return Result::failure(QString("k должно быть целым числом от 0 до %1.").arg(size()));
    std::rotate(items.begin(), items.begin() + k, items.end());
    return Result::success();
}

Result Program::changeDuration(int code, int duration)
{
    const auto it = std::find_if(items.begin(), items.end(),
                                 [code](const Track &track) { return track.code == code; });
    if (it == items.end())
        return Result::failure(QString("номер с кодом %1 не найден.").arg(code));
    if (duration < 1 || duration > maxDuration)
        return Result::failure(
            QString("длительность должна быть от 1 до %1 секунд.").arg(maxDuration));
    it->duration = duration;
    return Result::success();
}

std::vector<Track> Program::search(const QString &fragment) const
{
    const QString needle = fragment.trimmed();
    if (needle.isEmpty())
        return {items.begin(), items.end()};
    std::vector<Track> found;
    std::copy_if(items.begin(), items.end(), std::back_inserter(found),
                 [&needle](const Track &track) {
                     return track.title.contains(needle, Qt::CaseInsensitive);
                 });
    return found;
}

std::vector<Track> Program::reversed() const
{
    return {items.rbegin(), items.rend()};
}

const std::deque<Track> &Program::tracks() const
{
    return items;
}

const Track *Program::findByCode(int code) const
{
    const auto it = std::find_if(items.begin(), items.end(),
                                 [code](const Track &track) { return track.code == code; });
    return it == items.end() ? nullptr : &*it;
}

qint64 Program::totalDuration() const
{
    return std::accumulate(items.begin(), items.end(), qint64{0},
                           [](qint64 sum, const Track &track) { return sum + track.duration; });
}

int Program::size() const
{
    return static_cast<int>(items.size());
}

bool Program::isEmpty() const
{
    return items.empty();
}
