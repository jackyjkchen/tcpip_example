#ifndef _THREAD_POOL_C_H_
#define _THREAD_POOL_c_H_

#ifdef __cplusplus
extern "C" {
#endif

struct THREADPOOL_CTX {
    void *ctx;
};

typedef void (*thrd_callback)(void *param);

extern int threadpool_init(struct THREADPOOL_CTX *ctx, int max_thrd_num);

extern void threadpool_clean(struct THREADPOOL_CTX *ctx);

extern void threadpool_addtask(struct THREADPOOL_CTX *ctx, thrd_callback func, void *param);

extern void threadpool_waitalltask(struct THREADPOOL_CTX *ctx);

extern void threadpool_waitallthrd(struct THREADPOOL_CTX *ctx);

#ifdef __cplusplus
}
#endif

#endif
