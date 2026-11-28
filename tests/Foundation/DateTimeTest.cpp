#include <Bibim/Config.h>
#include <Bibim/DateTime.h>
#include <doctest.h>
#include <time.h>

using namespace Bibim;

namespace
{
    bool IsLocalTimeOf(const DateTime& d, time_t time)
    {
        const struct tm* t = localtime(&time);
        DateTime local(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
        return local == d;
    }

    bool HasDateOf(const DateTime& d, time_t time)
    {
        const struct tm* t = localtime(&time);
        return d.Year == t->tm_year + 1900 && d.Month == t->tm_mon + 1 && d.Day == t->tm_mday;
    }
}

TEST_CASE("DateTime constructors set the given fields and zero the rest")
{
    const DateTime zero;
    CHECK(zero.Year == 0);
    CHECK(zero.Month == 0);
    CHECK(zero.Day == 0);
    CHECK(zero.Hour == 0);
    CHECK(zero.Minute == 0);
    CHECK(zero.Second == 0);

    const DateTime date(2026, 11, 28);
    CHECK(date.Year == 2026);
    CHECK(date.Month == 11);
    CHECK(date.Day == 28);
    CHECK(date.Hour == 0);
    CHECK(date.Minute == 0);
    CHECK(date.Second == 0);

    const DateTime full(2026, 11, 28, 13, 45, 59);
    CHECK(full.Year == 2026);
    CHECK(full.Month == 11);
    CHECK(full.Day == 28);
    CHECK(full.Hour == 13);
    CHECK(full.Minute == 45);
    CHECK(full.Second == 59);
}

TEST_CASE("DateTime copies and compares every field")
{
    DateTime original(2026, 11, 28, 13, 45, 59);

    DateTime copy(original);
    CHECK((copy == original));
    CHECK_FALSE((copy != original));

    DateTime assigned;
    assigned = original;
    CHECK((assigned == original));

    DateTime different[] =
    {
        DateTime(2025, 11, 28, 13, 45, 59),
        DateTime(2026, 10, 28, 13, 45, 59),
        DateTime(2026, 11, 27, 13, 45, 59),
        DateTime(2026, 11, 28, 12, 45, 59),
        DateTime(2026, 11, 28, 13, 44, 59),
        DateTime(2026, 11, 28, 13, 45, 58),
    };
    for (int i = 0; i < 6; i++)
    {
        CAPTURE(i);
        CHECK((different[i] != original));
        CHECK_FALSE((different[i] == original));
    }

    DateTime date(2026, 11, 28);
    DateTime midnight(2026, 11, 28, 0, 0, 0);
    CHECK((date == midnight));
}

TEST_CASE("DateTime::Now returns the current local time")
{
    const time_t before = time(nullptr);
    const DateTime now = DateTime::Now();
    const time_t after = time(nullptr);

    bool found = false;
    for (time_t t = before; t <= after; t++)
        found = found || IsLocalTimeOf(now, t);
    CHECK(found);
}

TEST_CASE("DateTime::Today returns the current local date at midnight")
{
    const time_t before = time(nullptr);
    const DateTime today = DateTime::Today();
    const time_t after = time(nullptr);

    CHECK((HasDateOf(today, before) || HasDateOf(today, after)));
    CHECK(today.Hour == 0);
    CHECK(today.Minute == 0);
    CHECK(today.Second == 0);
}
