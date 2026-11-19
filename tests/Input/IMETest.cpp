#include <Bibim/Config.h>
#include <Bibim/IME.h>
#include <doctest.h>

using namespace Bibim;

namespace
{
    class RecordingCallback : public IME::Callback
    {
        public:
            int edited = 0;
            int cancelled = 0;

            virtual void OnTextEdited(const String& /*text*/) { edited++; }
            virtual void OnTextEditCancelled() { cancelled++; }
    };
}

TEST_CASE("SDL3 IME cancels every edit request")
{
    IME* ime = new IME();
    RecordingCallback callback;

    ime->Edit("text", "title", "description", IME::Plain, IME::NO_MAX_LENGTH, &callback);
    ime->Edit("", "", "", IME::Number, 12, &callback);

    CHECK(callback.edited == 0);
    CHECK(callback.cancelled == 2);
    delete ime;
}
