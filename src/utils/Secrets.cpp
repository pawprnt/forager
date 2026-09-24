// Wrap glib headers before Qt to avoid signals/slots keyword conflict
#include <libsecret/secret.h>

#include "utils/Secrets.h"

static const char* SCHEMA_NAME = "com.pawprnt.forager";

namespace secrets {

static SecretSchema* getSchema()
{
    return secret_schema_new(
        SCHEMA_NAME, SECRET_SCHEMA_NONE,
        "key", SECRET_SCHEMA_ATTRIBUTE_STRING,
        nullptr);
}

bool store(const QString& key, const QString& value)
{
    GError* err = nullptr;
    SecretSchema* schema = getSchema();
    QByteArray label = (QString("forager") + " \u2014 " + key).toUtf8();
    gboolean ok = secret_password_store_sync(
        schema,
        SECRET_COLLECTION_DEFAULT,
        label.constData(),
        value.toUtf8().constData(),
        nullptr,
        &err,
        "key", key.toUtf8().constData(),
        nullptr);
    secret_schema_unref(schema);
    return ok && !err;
}

QString lookup(const QString& key)
{
    GError* err = nullptr;
    SecretSchema* schema = getSchema();
    gchar* raw = secret_password_lookup_sync(
        schema,
        nullptr,
        &err,
        "key", key.toUtf8().constData(),
        nullptr);
    secret_schema_unref(schema);
    if (err) {
        g_error_free(err);
        return {};
    }
    if (!raw) return {};
    QString result = QString::fromUtf8(raw);
    secret_password_free(raw);
    return result;
}

bool clear(const QString& key)
{
    GError* err = nullptr;
    SecretSchema* schema = getSchema();
    gboolean ok = secret_password_clear_sync(
        schema,
        nullptr,
        &err,
        "key", key.toUtf8().constData(),
        nullptr);
    secret_schema_unref(schema);
    return ok && !err;
}

} // namespace secrets
