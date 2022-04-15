#include "io_context.h"

#if __cplusplus >= 201103L
#include <unordered_map>
typedef std::unordered_map<void *, io_context_t> io_context_map;
#define MAP_HAVE_SECOND 1
#elif defined(_MSC_VER) && _MSC_VER >= 1500
#include <unordered_map>
typedef std::tr1::unordered_map<void *, io_context_t> io_context_map;
#define MAP_HAVE_SECOND 1
#elif defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#include <tr1/unordered_map>
typedef std::tr1::unordered_map<void *, io_context_t> io_context_map;
#define MAP_HAVE_SECOND 1
#elif defined(__GNUC__) && __GNUC__ == 2 && __GNUC_MINOR__ < 8
#include <map.h>
typedef map<void *, io_context_t, less<void *> > io_context_map;
#elif defined(_MSC_VER) && _MSC_VER < 1100
#include <map>
typedef map<void *, io_context_t, less<void *>, allocator<io_context_t> > io_context_map;
#elif defined(__WATCOMC__) && __WATCOMC__ < 1200
#include <map>
typedef std::map<void *, io_context_t, less<void *> > io_context_map;
#define MAP_HAVE_SECOND 1
#else
#include <map>
typedef std::map<void *, io_context_t> io_context_map;
#define MAP_HAVE_SECOND 1
#endif

static io_context_map& get_map_inst() {
    static io_context_map inst;
    return inst;
}

void alloc_io_context(void *key) {
    io_context_t io_context;
    io_context.fd = key;
    io_context.buf = NULL;
    io_context.bufsize = TCP_BUF_SIZE;
    io_context.recvbytes = 0;
    io_context.sendbytes = 0;
    io_context.sendagain = 0;
    io_context.buf = (char *)malloc(io_context.bufsize);
    if (io_context.buf == NULL) {
        print_error("alloc_io_context failed");
        abort();
    }
    memset(io_context.buf, 0x00, io_context.bufsize);
    io_context_map &ctx_map = get_map_inst();
#if defined(__WATCOMC__)
    io_context_map::iterator &i = ctx_map.find(key);
#else
    const io_context_map::iterator &i = ctx_map.find(key);
#endif
    if (i == ctx_map.end()) {
        ctx_map[key] = io_context;
    }
}

void free_io_context(void *key) {
    io_context_map &ctx_map = get_map_inst();
#if defined(__WATCOMC__)
    io_context_map::iterator &i = ctx_map.find(key);
#else
    const io_context_map::iterator &i = ctx_map.find(key);
#endif
    if (i != ctx_map.end()) {
#ifdef MAP_HAVE_SECOND
        io_context_t &value = i->second;
#else
        io_context_t &value = ctx_map[key];
#endif
        free(value.buf);
        value.buf = NULL;
        ctx_map.erase(key);
    }
}

io_context_t *get_io_context(void *key) {
    io_context_map &ctx_map = get_map_inst();
    io_context_t *ret = NULL;
#if defined(__WATCOMC__)
    io_context_map::iterator &i = ctx_map.find(key);
#else
    const io_context_map::iterator &i = ctx_map.find(key);
#endif
    if (i != ctx_map.end()) {
#ifdef MAP_HAVE_SECOND
        ret = &i->second;
#else
        ret = &ctx_map[key];
#endif
    }
    return ret;
}

