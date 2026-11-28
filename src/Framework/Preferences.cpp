#include <Bibim/Config.h>
#include <Bibim/Preferences.h>
#include <Bibim/Environment.h>
#include <Bibim/FileStream.h>
#include <yyjson.h>
#include <climits>
#include <cstdlib>
#include <vector>

namespace Bibim
{
    Preferences::Preferences(const String& name)
        : name(name),
          isModified(false),
          isLoaded(false)
    {
        const char* buffer = name.CStr();
        const int length = name.GetLength();
        BBAssert(length >= 4);

        static const char ALLOWED_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_-";
        static const int  ALLOWED_CHARS_COUNT = sizeof(ALLOWED_CHARS) / sizeof(ALLOWED_CHARS[0]);

        for (int i = 0; i < length; i++)
        {
            const char c = buffer[i];
            bool allowed = false;
            for (int k = 0; k < ALLOWED_CHARS_COUNT; k++)
            {
                if (c == ALLOWED_CHARS[k])
                {
                    allowed = true;
                    break;
                }
            }

            if (allowed == false)
            {
                // 허용되지 않은 문자열이 존재합니다.
                BBBreak();
            }
        }
    }

    Preferences::~Preferences()
    {
        if (IsModified())
        {
            // 변경된 데이터가 있는데 저장을 안했습니다.
            BBBreak();
        }
    }

    const Any& Preferences::GetValue(const String& key)
    {
        if (IsLoaded() == false)
            Update();

        Dictionary::const_iterator it = items.find(key);
        if (it != items.end())
            return (*it).second;
        else
            return Any::Void;
    }

    void Preferences::SetValue(const String& key, const Any& value)
    {
        if (key.IsEmpty())
            return;

        if (IsLoaded() == false)
            Update();

        if (value.GetType() != Any::VoidType)
        {
            if (CanStore(value.GetType()))
            {
                items[key] = value;
                isModified = true;
            }
        }
        else
        {
            Dictionary::iterator it = items.find(key);
            if (it != items.end())
            {
                items.erase(it);
                isModified = true;
            }
        }
    }

    void Preferences::Update()
    {
        isLoaded = true;

        const String documentString = Load();
        // Files written with RapidJSON may hold invalid UTF-8.
        yyjson_doc* document = yyjson_read(documentString.CStr(), documentString.GetLength(), YYJSON_READ_ALLOW_INVALID_UNICODE);
        yyjson_val* root = yyjson_doc_get_root(document);
        if (yyjson_is_obj(root) == false)
        {
            yyjson_doc_free(document);
            return;
        }

        items.clear();

        size_t index = 0;
        size_t count = 0;
        yyjson_val* name = nullptr;
        yyjson_val* item = nullptr;
        yyjson_obj_foreach(root, index, count, name, item)
        {
            const String key = String(yyjson_get_str(name), 0, static_cast<int>(yyjson_get_len(name)));
            Any value = Any::Void;
            if (yyjson_is_bool(item))
                value = yyjson_get_bool(item) ? Any::True : Any::False;
            else if (yyjson_is_str(item))
                value = Any(String(yyjson_get_str(item), 0, static_cast<int>(yyjson_get_len(item))));
            else if (yyjson_is_sint(item) && yyjson_get_sint(item) >= INT_MIN)
                value = Any(static_cast<int>(yyjson_get_sint(item)));
            else if (yyjson_is_uint(item) && yyjson_get_uint(item) <= INT_MAX)
                value = Any(static_cast<int>(yyjson_get_uint(item)));

            if (value != Any::Void)
                items[key] = value;
        }

        yyjson_doc_free(document);
    }

    void Preferences::Commit()
    {
        if (IsModified() == false || IsLoaded() == false)
            return;

        yyjson_mut_doc* document = yyjson_mut_doc_new(nullptr);
        if (document == nullptr)
            return;

        yyjson_mut_val* root = yyjson_mut_obj(document);
        yyjson_mut_doc_set_root(document, root);
        bool complete = root != nullptr;
        for (Dictionary::const_iterator it = items.begin(); complete && it != items.end(); ++it)
        {
            const String& key = (*it).first;
            const Any& value = (*it).second;
            yyjson_mut_val* item = nullptr;
            switch (value.GetType())
            {
                case Any::BoolType:
                    item = yyjson_mut_bool(document, value.CastBool());
                    break;
                case Any::IntType:
                    item = yyjson_mut_sint(document, value.CastInt());
                    break;
                case Any::StringType:
                {
                    const String& s = value.CastString();
                    item = yyjson_mut_strn(document, s.CStr(), s.GetLength());
                    break;
                }
                default:
                    break;
            }

            complete = yyjson_mut_obj_add(root, yyjson_mut_strn(document, key.CStr(), key.GetLength()), item);
        }

        size_t length = 0;
        char* output = complete ? yyjson_mut_write(document, YYJSON_WRITE_ALLOW_INVALID_UNICODE, &length) : nullptr;
        yyjson_mut_doc_free(document);
        if (output == nullptr)
            return;

        Save(String(output, 0, static_cast<int>(length)));
        std::free(output);

        isModified = false;
    }

    String Preferences::Load()
    {
        const String path = Environment::GetAppDataPath(GetName(), "preferences");
        FileStreamPtr stream = new FileStream(path, FileStream::ReadOnly);
        if (stream->CanRead() == false)
            return String::Empty;

        const int length = stream->GetLength();
        if (length <= 0)
            return String::Empty;

        std::vector<char> buffer(length + 1, '\0');
        stream->Read(&buffer[0], length);
        stream->Close();

        return String(&buffer[0], 0, length);
    }

    void Preferences::Save(const String& document)
    {
        const String path = Environment::GetAppDataPath(GetName(), "preferences");
        FileStreamPtr stream = new FileStream(path, FileStream::WriteOnly);
        if (stream->CanWrite() == false)
            return;

        stream->Write(document.CStr(), document.GetLength());
        stream->Close();
    }
}
