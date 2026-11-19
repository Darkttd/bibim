#include <Bibim/Config.h>
#if (defined(BIBIM_USE_SDL3))

#include <Bibim/Clipboard.SDL3.h>

namespace Bibim
{
    Clipboard::Clipboard()
    {
    }

    Clipboard::~Clipboard()
    {
    }

    String Clipboard::GetText() const
    {
        return String::Empty;
    }

    void Clipboard::SetText(const String& /*value*/)
    {
    }
}

#endif
