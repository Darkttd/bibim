// Tests for the Ease functions behind the standard easing curves.
#include <Bibim/Config.h>
#include <Bibim/Ease.h>
#include <doctest.h>

using namespace Bibim;

namespace
{
    typedef float (*EaseFunction)(float);

    struct Curve
    {
        const char* Name;
        EaseFunction Function;
    };

    struct InOutPair
    {
        const char* Name;
        EaseFunction In;
        EaseFunction Out;
    };

    float InElastic(float t)    { return Ease::InElastic(t, 1.0f, 0.3f); }
    float OutElastic(float t)   { return Ease::OutElastic(t, 1.0f, 0.3f); }
    float InOutElastic(float t) { return Ease::InOutElastic(t, 1.0f, 0.45f); }

    const Curve Curves[] =
    {
        { "InQuadratic",   Ease::InQuadratic },   { "OutQuadratic",   Ease::OutQuadratic },   { "InOutQuadratic",   Ease::InOutQuadratic },
        { "InCubic",       Ease::InCubic },       { "OutCubic",       Ease::OutCubic },       { "InOutCubic",       Ease::InOutCubic },
        { "InQuartic",     Ease::InQuartic },     { "OutQuartic",     Ease::OutQuartic },     { "InOutQuartic",     Ease::InOutQuartic },
        { "InQuintic",     Ease::InQuintic },     { "OutQuintic",     Ease::OutQuintic },     { "InOutQuintic",     Ease::InOutQuintic },
        { "InSinusoidal",  Ease::InSinusoidal },  { "OutSinusoidal",  Ease::OutSinusoidal },  { "InOutSinusoidal",  Ease::InOutSinusoidal },
        { "InExponential", Ease::InExponential }, { "OutExponential", Ease::OutExponential }, { "InOutExponential", Ease::InOutExponential },
        { "InCircular",    Ease::InCircular },    { "OutCircular",    Ease::OutCircular },    { "InOutCircular",    Ease::InOutCircular },
        { "InBack",        Ease::InBack },        { "OutBack",        Ease::OutBack },        { "InOutBack",        Ease::InOutBack },
        { "InElastic",     InElastic },           { "OutElastic",     OutElastic },           { "InOutElastic",     InOutElastic },
    };

    const InOutPair Pairs[] =
    {
        { "Quadratic",   Ease::InQuadratic,   Ease::OutQuadratic },
        { "Cubic",       Ease::InCubic,       Ease::OutCubic },
        { "Quartic",     Ease::InQuartic,     Ease::OutQuartic },
        { "Quintic",     Ease::InQuintic,     Ease::OutQuintic },
        { "Sinusoidal",  Ease::InSinusoidal,  Ease::OutSinusoidal },
        { "Exponential", Ease::InExponential, Ease::OutExponential },
        { "Circular",    Ease::InCircular,    Ease::OutCircular },
        { "Back",        Ease::InBack,        Ease::OutBack },
        { "Elastic",     InElastic,           OutElastic },
    };

    const Curve InOutCurves[] =
    {
        { "InOutQuadratic",   Ease::InOutQuadratic },
        { "InOutCubic",       Ease::InOutCubic },
        { "InOutQuartic",     Ease::InOutQuartic },
        { "InOutQuintic",     Ease::InOutQuintic },
        { "InOutSinusoidal",  Ease::InOutSinusoidal },
        { "InOutExponential", Ease::InOutExponential },
        { "InOutCircular",    Ease::InOutCircular },
        { "InOutBack",        Ease::InOutBack },
        { "InOutElastic",     InOutElastic },
    };

    // Float math in the curves (sin, pow, sqrt) is accurate to a few ulps.
    doctest::Approx Near(float value)
    {
        return doctest::Approx(value).epsilon(1e-5);
    }

    const int Steps = 20;

    float StepAt(int i)
    {
        return static_cast<float>(i) / Steps;
    }
}

TEST_CASE("Ease::Linear returns its input")
{
    for (int i = 0; i <= Steps; i++)
        CHECK(Ease::Linear(StepAt(i)) == StepAt(i));
}

TEST_CASE("Every easing curve starts at 0 and ends at 1")
{
    for (const Curve& curve : Curves)
    {
        CAPTURE(curve.Name);
        CHECK(curve.Function(0.0f) == Near(0.0f));
        CHECK(curve.Function(1.0f) == Near(1.0f));
    }
}

TEST_CASE("Each Out curve is its In curve played backwards")
{
    // Out(t) = 1 - In(1 - t)
    for (const InOutPair& pair : Pairs)
    {
        CAPTURE(pair.Name);
        for (int i = 0; i <= Steps; i++)
        {
            const float t = StepAt(i);
            CAPTURE(t);
            CHECK(pair.Out(t) == Near(1.0f - pair.In(1.0f - t)));
        }
    }
}

TEST_CASE("Each InOut curve is symmetric about its midpoint")
{
    // InOut(t) + InOut(1 - t) = 1, so InOut(0.5) = 0.5
    for (const Curve& curve : InOutCurves)
    {
        CAPTURE(curve.Name);
        CHECK(curve.Function(0.5f) == Near(0.5f));
        for (int i = 0; i <= Steps; i++)
        {
            const float t = StepAt(i);
            CAPTURE(t);
            CHECK(curve.Function(t) + curve.Function(1.0f - t) == Near(1.0f));
        }
    }
}

TEST_CASE("Back curves overshoot the 0..1 range")
{
    CHECK(Ease::InBack(0.2f) < 0.0f);
    CHECK(Ease::OutBack(0.8f) > 1.0f);
    // With no overshoot, InBack is t^3.
    CHECK(Ease::InBack(0.5f, 0.0f) == Near(Ease::InCubic(0.5f)));
}
