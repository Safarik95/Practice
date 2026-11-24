#pragma once

#include "result.h"
#include "track.h"

#include <deque>
#include <vector>

class Program
{
public:
    static constexpr int maxTracks = 1000;
    static constexpr int maxCode = 1000000;
    static constexpr int maxDuration = 86400;
    static constexpr int maxTextLength = 100;

    Result addFront(const Track &track);
    Result addBack(const Track &track);
    Result removeFirst();
    Result removeLast();
    Result moveFirstToEnd(int k);
    Result changeDuration(int code, int duration);

    std::vector<Track> search(const QString &fragment) const;
    std::vector<Track> reversed() const;
    const std::deque<Track> &tracks() const;
    const Track *findByCode(int code) const;
    qint64 totalDuration() const;
    int size() const;
    bool isEmpty() const;

    static Result validate(Track &track);

private:
    Result prepare(Track &track) const;

    std::deque<Track> items;
};
