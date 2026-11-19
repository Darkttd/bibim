#include <Bibim/Config.h>
#if (defined(BIBIM_USE_SDL3))

#include <Bibim/IME.SDL3.h>

namespace Bibim
{
    IME::IME()
    {
    }

    IME::IME(Window* window)
        : Base(window)
    {
    }

    IME::~IME()
    {
    }

    void IME::OnRequest(const Request& request)
    {
        // No text input yet. Cancelling releases the caller's callback, which
        // would otherwise stay registered after the caller is deleted.
        request.Cancel();
    }
}

#endif
