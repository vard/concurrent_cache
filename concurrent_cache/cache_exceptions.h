#ifndef CACHE_EXCEPTIONS_H
#define CACHE_EXCEPTIONS_H

#include <stdexcept>
#include <string>

#include <valgrind/drd.h>
#define _GLIBCXX_SYNCHRONIZATION_HAPPENS_BEFORE(addr) ANNOTATE_HAPPENS_BEFORE(addr)
#define _GLIBCXX_SYNCHRONIZATION_HAPPENS_AFTER(addr)  ANNOTATE_HAPPENS_AFTER(addr)

namespace concurrent_cache {


class CacheTimeoutException : public std::logic_error{
    public:
        CacheTimeoutException()
            :std::logic_error{"Cache timeout exception"}{
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


class CacheInternalException : public std::logic_error{
    public:
        CacheInternalException(const char* message)
            :std::logic_error{message}{
        }
};


} // namespace
#endif // CACHE_EXCEPTIONS_H
