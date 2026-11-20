#pragma once
#include <Bibim/String.h>

// A new, empty directory in the system's temporary directory. It is removed
// with everything in it when the object is destroyed. Paths use '/'.
class TemporaryDirectory
{
    public:
        TemporaryDirectory();
        ~TemporaryDirectory();

        const Bibim::String& GetPath() const { return path; }
        Bibim::String Get(const char* name) const;

    private:
        Bibim::String path;
};
