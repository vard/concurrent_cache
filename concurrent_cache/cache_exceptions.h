#ifndef CACHE_EXCEPTIONS_H
#define CACHE_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace concurrent_cache {


class CacheTimeoutException : public std::logic_error{
    public:
        CacheTimeoutException()
            :std::logic_error{""}{
        }
};


class CacheInvalidArgument : public std::logic_error{
    public:
        CacheInvalidArgument(const char* whatMessage)
            :std::logic_error(whatMessage){
        }
};


class QueueEmpty : public std::logic_error{
    public:
        QueueEmpty()
            :std::logic_error{""}{
        }
};


class DbInitException : public std::logic_error{
    public:
        DbInitException()
            :std::logic_error{""}{
        }
};


class DbParseException : public std::logic_error{
    public:
        DbParseException(const char* message)
            :std::logic_error{message}{
        }
};


} // namespace
#endif // CACHE_EXCEPTIONS_H
