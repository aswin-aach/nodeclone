#include <uv.h>
#include <stdlib.h>

#define EXPORT __attribute__((visibility("default")))

// Declare Lisp callback
typedef void (*lisp_cb_t)(int);
static lisp_cb_t lisp_cb = NULL;

// Exported so Lisp can register
EXPORT void uv_register_lisp_callback(void* cb) {
    lisp_cb = (lisp_cb_t) cb;
}

static uv_loop_t loop;

typedef struct {
    int callback_id;
    uv_timer_t timer;
} timer_data_t;

void on_timer(uv_timer_t* handle) {
    timer_data_t* data = (timer_data_t*) handle->data;
    if (data && lisp_cb) {
        lisp_cb(data->callback_id);
    }
    free(data);
    free(handle);
}

// Exported API
EXPORT void uv_host_init() {
    uv_loop_init(&loop);
}

EXPORT void uv_host_run_once() {
    uv_run(&loop, UV_RUN_NOWAIT);
}

EXPORT void uv_host_set_timeout(int callback_id, int delay_ms) {
    uv_timer_t* timer = malloc(sizeof(uv_timer_t));
    timer_data_t* data = malloc(sizeof(timer_data_t));

    data->callback_id = callback_id;
    data->timer = *timer;
    timer->data = data;

    uv_timer_init(&loop, timer);
    uv_timer_start(timer, on_timer, delay_ms, 0);
}
