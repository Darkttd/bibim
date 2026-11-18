#include <Bibim/Config.h>
#if (defined(BIBIM_PLATFORM_LINUX))

#include <Bibim/FileStream.Linux.h>
#include <Bibim/Path.h>
#include <sys/stat.h>

namespace Bibim
{
    FileStream::FileStream(const String& path, AccessMode accessMode)
        : handle(nullptr),
          canRead(false),
          canWrite(false)
    {
        String p = path;
        p.Replace('\\', '/');

        const char* mode = (accessMode == WriteOnly) ? "wb" : "rb";
        handle = std::fopen(p.CStr(), mode);
        if (handle == nullptr && accessMode == WriteOnly)
        {
            // Like the Windows version, create one missing directory level.
            const String directory = Path::GetDirectory(p);
            if (directory.IsEmpty() == false && ::mkdir(directory.CStr(), 0755) == 0)
                handle = std::fopen(p.CStr(), mode);
        }

        if (handle == nullptr)
            return;

        canRead  = (accessMode == ReadOnly);
        canWrite = (accessMode == WriteOnly);
    }

    FileStream::~FileStream()
    {
        Close();
    }

    void FileStream::Close()
    {
        if (handle)
        {
            std::fclose(handle);
            handle = nullptr;
        }
    }

    int FileStream::Read(void* buffer, int size)
    {
        if (handle == nullptr || size <= 0 || canRead == false)
            return 0;

        return static_cast<int>(std::fread(buffer, 1, size, handle));
    }

    int FileStream::Write(const void* buffer, int size)
    {
        if (handle == nullptr || size <= 0 || canWrite == false)
            return 0;

        return static_cast<int>(std::fwrite(buffer, 1, size, handle));
    }

    void FileStream::Flush()
    {
        if (handle == nullptr || canWrite == false)
            return;

        std::fflush(handle);
    }

    int FileStream::Seek(int offset, SeekOrigin origin)
    {
        if (handle == nullptr)
            return 0;

        int whence = SEEK_SET;
        switch (origin)
        {
            case FromBegin:
                whence = SEEK_SET;
                break;
            case FromEnd:
                whence = SEEK_END;
                break;
            case FromCurrent:
                whence = SEEK_CUR;
                break;
        }

        std::fseek(handle, offset, whence);
        return static_cast<int>(std::ftell(handle));
    }

    int FileStream::GetPosition()
    {
        if (handle == nullptr)
            return 0;

        return static_cast<int>(std::ftell(handle));
    }

    int FileStream::GetLength()
    {
        if (handle == nullptr)
            return 0;

        std::fflush(handle);
        struct stat status;
        if (::fstat(fileno(handle), &status) != 0)
            return 0;

        return static_cast<int>(status.st_size);
    }

    bool FileStream::CanRead() const
    {
        return canRead;
    }

    bool FileStream::CanWrite() const
    {
        return canWrite;
    }

    bool FileStream::CanSeek() const
    {
        return true;
    }
}

#endif
