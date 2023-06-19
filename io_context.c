#include "io_context.h"
#include "cmap.h"

typedef map_t(void *, io_context_t) io_context_map;

static void free_map_inst(void);

static io_context_map* get_map_inst(void) {
    static io_context_map inst;
	static int inited = 0;
    if (!inited) {
	    map_init(&inst, NULL, NULL);
        inited = 1;
        atexit(free_map_inst);
    }
    return &inst;
}

static void free_map_inst(void) {
    map_delete(get_map_inst());
}

void alloc_io_context(void *key) {
    io_context_t io_context;
    io_context_map *ctx_map = NULL;
    io_context_t *value = NULL;
    io_context.fd = key;
    io_context.buf = NULL;
    io_context.bufsize = TCP_BUF_SIZE;
    io_context.recvbytes = 0;
    io_context.sendbytes = 0;
    io_context.sendagain = 0;
    io_context.recvdone = 0;
    io_context.buf = (char *)malloc(io_context.bufsize);
    if (io_context.buf == NULL) {
        print_error("alloc_io_context failed");
        abort();
    }
    memset(io_context.buf, 0x00, io_context.bufsize);
    ctx_map = get_map_inst();
    value = map_get(ctx_map, key);
    if (value == NULL) {
        map_set(ctx_map, key, io_context);
    }
}

void free_io_context(void *key) {
    io_context_map *ctx_map = get_map_inst();
    io_context_t *value = map_get(ctx_map, key);
    if (value != NULL) {
        free(value->buf);
        value->buf = NULL;
        map_remove(ctx_map, key);
    }
}

io_context_t *get_io_context(void *key) {
    io_context_map *ctx_map = get_map_inst();
    return map_get(ctx_map, key);
}

