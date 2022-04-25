#include "thread_pool_c.h"
#include "thread_pool.h"

int threadpool_startup(struct THREADPOOL_CTX *ctx, int max_thrd_num) {
    int ret = 0;
    ThreadPool *pool = new ThreadPool(max_thrd_num);
    if (pool != NULL) {
        if (!pool->init()) {
            delete pool;
            ret = -1;
        }
        ctx->ctx = pool;
    } else {
        ret = -1;
    }
    if (ret == -1) {
        ctx->ctx = NULL;
    }
    return ret;
}

void threadpool_cleanup(struct THREADPOOL_CTX *ctx) {
    ThreadPool *pool = (ThreadPool *)ctx->ctx;
    delete pool;
    ctx->ctx = NULL;
}

void threadpool_addtask(struct THREADPOOL_CTX *ctx, thrd_callback func, void *param) {
    ThreadPool *pool = (ThreadPool *)ctx->ctx;
    pool->add_task(func, param);
}

void threadpool_waitalltask(struct THREADPOOL_CTX *ctx) {
    ThreadPool *pool = (ThreadPool *)ctx->ctx;
    pool->wait_all_task();
}

void threadpool_waitallthrd(struct THREADPOOL_CTX *ctx) {
    ThreadPool *pool = (ThreadPool *)ctx->ctx;
    pool->wait_all_thrd();
}

