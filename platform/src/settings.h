#ifndef SETTINGS_H
#define SETTINGS_H

#include <QJsonValue>
#include <QJsonObject>
#include <QJSValue>
#include <QMap>
#include <QQmlPropertyMap>

class QJSEngine;

class Settings
    : public QQmlPropertyMap // TODO: Improve interface by using QJsonObject or dynamic meta objects
{
    Q_OBJECT

public:
    inline static Settings* I() { if( s_pInstance == nullptr ) s_pInstance = new Settings(); return s_pInstance; }
    inline static void destroyI() { delete s_pInstance; }

    void loadSettings();
    void loadTypes();

    Q_INVOKABLE QJsonValue opt(QString key_path);
    Q_INVOKABLE void setOpt(QString key_path, QJsonObject option);

    Q_INVOKABLE QJsonValue val(QString key_path);
    Q_INVOKABLE void setVal(QString key_path, QJsonValue value);
    Q_INVOKABLE void unsetVal(QString key_path);

    Q_INVOKABLE bool validate(QJsonObject option, QJsonValue value);

private:
    static Settings *s_pInstance;
    explicit Settings();
    ~Settings() override;

    void loadDefinitions();
    void setDefinition(QString key_path, QJsonObject option, bool custom = false);
    void loadType(QJsonObject option);
    void updateVal(QString key_path, QJsonObject value);
    void saveSettings();
    QJsonObject flattenSettings(QJsonObject *settings);

    QString m_path;
    QJsonObject *m_settings;
    QJSEngine *m_jsengine;
    QMap<QString, QJSValue> m_types;
};

#endif // SETTINGS_H
