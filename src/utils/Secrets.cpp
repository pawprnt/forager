// Wrap glib headers before Qt to avoid signals/slots keyword conflict
#include <libsecret/secret.h>

#include "utils/Secrets.h"

static const char* SCHEMA_NAME = "com.pawprnt.forager";
static const int OP_TIMEOUT_MS = 2000;

namespace secrets {

static SecretSchema* getSchema()
{
    return secret_schema_new(
        SCHEMA_NAME, SECRET_SCHEMA_NONE,
        "key", SECRET_SCHEMA_ATTRIBUTE_STRING,
        nullptr);
}

struct AsyncCtx {
    GMainLoop* loop = nullptr;
    GCancellable* cancellable = nullptr;
    QString value;
    bool ok = false;
    bool finished = false;
};

static void finishOnce(AsyncCtx* ctx)
{
    if (ctx->finished) return;
    ctx->finished = true;
    if (ctx->loop) g_main_loop_quit(ctx->loop);
}

static gboolean onTimeout(gpointer data)
{
    auto* ctx = static_cast<AsyncCtx*>(data);
    if (ctx->cancellable) g_cancellable_cancel(ctx->cancellable);
    finishOnce(ctx);
    return G_SOURCE_REMOVE;
}

static void runLoop(AsyncCtx* ctx)
{
    guint timer = g_timeout_add(OP_TIMEOUT_MS, onTimeout, ctx);
    g_main_loop_run(ctx->loop);
    g_source_remove(timer);
}

bool store(const QString& key, const QString& value)
{
    GError* err = nullptr;
    SecretSchema* schema = getSchema();
    QByteArray label = (QString("forager") + " — " + key).toUtf8();

    AsyncCtx ctx;
    ctx.loop = g_main_loop_new(nullptr, FALSE);
    ctx.cancellable = g_cancellable_new();
    bool result = false;

    secret_password_store(
        schema,
        SECRET_COLLECTION_DEFAULT,
        label.constData(),
        value.toUtf8().constData(),
        ctx.cancellable,
        [](GObject*, GAsyncResult* res, gpointer data) {
            auto* c = static_cast<AsyncCtx*>(data);
            GError* error = nullptr;
            c->ok = secret_password_store_finish(res, &error) != FALSE;
            if (error) g_error_free(error);
            finishOnce(c);
        },
        &ctx,
        "key", key.toUtf8().constData(),
        nullptr);

    runLoop(&ctx);
    result = ctx.ok;
    g_cancellable_cancel(ctx.cancellable);
    g_object_unref(ctx.cancellable);
    g_main_loop_unref(ctx.loop);
    secret_schema_unref(schema);
    if (err) g_error_free(err);
    return result;
}

QString lookup(const QString& key)
{
    GError* err = nullptr;
    SecretSchema* schema = getSchema();

    AsyncCtx ctx;
    ctx.loop = g_main_loop_new(nullptr, FALSE);
    ctx.cancellable = g_cancellable_new();

    secret_password_lookup(
        schema,
        ctx.cancellable,
        [](GObject*, GAsyncResult* res, gpointer data) {
            auto* c = static_cast<AsyncCtx*>(data);
            GError* error = nullptr;
            gchar* raw = secret_password_lookup_finish(res, &error);
            if (!error && raw) {
                c->value = QString::fromUtf8(raw);
                c->ok = true;
                secret_password_free(raw);
            }
            if (error) g_error_free(error);
            finishOnce(c);
        },
        &ctx,
        "key", key.toUtf8().constData(),
        nullptr);

    runLoop(&ctx);
    QString result = ctx.value;
    g_cancellable_cancel(ctx.cancellable);
    g_object_unref(ctx.cancellable);
    g_main_loop_unref(ctx.loop);
    secret_schema_unref(schema);
    if (err) g_error_free(err);
    return result;
}

bool clear(const QString& key)
{
    GError* err = nullptr;
    SecretSchema* schema = getSchema();

    AsyncCtx ctx;
    ctx.loop = g_main_loop_new(nullptr, FALSE);
    ctx.cancellable = g_cancellable_new();

    secret_password_clear(
        schema,
        ctx.cancellable,
        [](GObject*, GAsyncResult* res, gpointer data) {
            auto* c = static_cast<AsyncCtx*>(data);
            GError* error = nullptr;
            c->ok = secret_password_clear_finish(res, &error) != FALSE;
            if (error) g_error_free(error);
            finishOnce(c);
        },
        &ctx,
        "key", key.toUtf8().constData(),
        nullptr);

    runLoop(&ctx);
    bool result = ctx.ok;
    g_cancellable_cancel(ctx.cancellable);
    g_object_unref(ctx.cancellable);
    g_main_loop_unref(ctx.loop);
    secret_schema_unref(schema);
    if (err) g_error_free(err);
    return result;
}

} // namespace secrets
