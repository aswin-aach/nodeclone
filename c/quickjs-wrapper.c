#include "quickjs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CALLBACKS 1024
#define EXPORT __attribute__((visibility("default")))

typedef struct {
    int id;
    JSValue callback;
    JSContext* ctx;
} CallbackEntry;

static JSRuntime *rt;
static JSContext *ctx;
static CallbackEntry callback_registry[MAX_CALLBACKS];
static int callback_counter = 0;

// Declare function from libuv
extern void uv_host_set_timeout(int callback_id, int delay_ms);

// Declare Lisp callback
typedef void (*lisp_callback_fn)(int);
static lisp_callback_fn lisp_enqueue_cb = NULL;

// Exported for Lisp to register its callback
EXPORT void qjs_register_lisp_callback(void* cb) {
    lisp_enqueue_cb = (lisp_callback_fn) cb;
}

// Store JS function closure
int register_callback(JSContext* ctx, JSValue fn) {
    if (callback_counter >= MAX_CALLBACKS) return -1;
    int id = callback_counter++;
    callback_registry[id].id = id;
    callback_registry[id].ctx = ctx;
    callback_registry[id].callback = JS_DupValue(ctx, fn);
    return id;
}

// Call JS function by ID
EXPORT void invoke_callback(int id) {
    if (id < 0 || id >= callback_counter) return;
    CallbackEntry *entry = &callback_registry[id];
    if (JS_IsFunction(entry->ctx, entry->callback)) {
        JSValue result = JS_Call(entry->ctx, entry->callback, JS_UNDEFINED, 0, NULL);
        JS_FreeValue(entry->ctx, result);
    }
    JS_FreeValue(entry->ctx, entry->callback);
    entry->callback = JS_UNDEFINED;
}

// `setTimeout` exposed to JS
JSValue js_set_timeout(JSContext *ctx, JSValueConst this_val,
                       int argc, JSValueConst *argv) {
    if (argc < 2 || !JS_IsFunction(ctx, argv[0]) || !JS_IsNumber(argv[1]))
        return JS_ThrowTypeError(ctx, "Expected (function, delay)");

    int64_t delay;
    JS_ToInt64(ctx, &delay, argv[1]);
    int callback_id = register_callback(ctx, argv[0]);
	/*
    if (lisp_enqueue_cb) {
        lisp_enqueue_cb(callback_id);
    }
	*/
    uv_host_set_timeout(callback_id, (int) delay);
    return JS_UNDEFINED;
}

JSValue js_std_log(JSContext *ctx, JSValueConst this_val,
                   int argc, JSValueConst *argv) {
    if (argc > 0) {
        const char *msg = JS_ToCString(ctx, argv[0]);
        if (msg) {
            printf("[console.log] %s\n", msg);
            JS_FreeCString(ctx, msg);
        }
    }
    return JS_UNDEFINED;
}


// Inject host APIs
void qjs_register_host(JSContext *ctx) {
    JSValue global = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global, "setTimeout",
                      JS_NewCFunction(ctx, js_set_timeout, "setTimeout", 2));
    JS_FreeValue(ctx, global);

	// std_log function
	JS_SetPropertyStr(ctx, global, "std_log",
    JS_NewCFunction(ctx, js_std_log, "std_log", 1));

	// console.log binding
	const char* inject_console = "globalThis.console = { log: std_log };";
	JS_Eval(ctx, inject_console, strlen(inject_console), "<init>", JS_EVAL_TYPE_GLOBAL);

	JS_FreeValue(ctx, global);

}

// Public API
EXPORT void qjs_init() {
    rt = JS_NewRuntime();
    ctx = JS_NewContext(rt);
    qjs_register_host(ctx);
}

EXPORT const char* qjs_eval(const char* code) {
    static char result[1024];
    JSValue val = JS_Eval(ctx, code, strlen(code), "<input>", JS_EVAL_TYPE_GLOBAL);
    const char *str = JS_ToCString(ctx, val);
    snprintf(result, sizeof(result), "%s", str ? str : "[error]");
    JS_FreeCString(ctx, str);
    JS_FreeValue(ctx, val);
    return result;
}

EXPORT void qjs_cleanup() {
    for (int i = 0; i < callback_counter; i++) {
        if (!JS_IsUndefined(callback_registry[i].callback)) {
            JS_FreeValue(callback_registry[i].ctx, callback_registry[i].callback);
        }
    }
    callback_counter = 0;

    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
}
