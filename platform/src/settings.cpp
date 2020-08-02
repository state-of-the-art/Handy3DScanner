#include "settings.h"
#include "plugins.h"

#include <QDebug>

#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QJSEngine>
#include <QJsonArray>
#include <QJsonDocument>

Settings* Settings::s_pInstance = nullptr;

Settings::Settings()
    : QQmlPropertyMap(this, nullptr)
    , m_path(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation))
    , m_settings(new QJsonObject())
    , m_jsengine(new QJSEngine())
{
    QDir(m_path).mkpath(".");
    m_path += "/settings.json";

    loadDefinitions();
    loadSettings();
    loadTypes();
}

Settings::~Settings()
{
    saveSettings();
    delete m_jsengine;
    delete m_settings;
}

QJsonValue Settings::opt(QString key_path)
{
    if( key_path.isEmpty() )
        return QJsonValue(*m_settings);

    QJsonValue result;
    QStringList path = key_path.split('.');
    QJsonObject curr_level = *m_settings;

    QStringListIterator i(path);
    while( i.hasNext() ) {
        result = curr_level[i.next()];
        if( result.isUndefined() )
            return result;
        if( i.hasNext() ) {
            if( ! result.isObject() )
                return QJsonValue();
            curr_level = result.toObject();
        }
    }
    return result;
}

void Settings::setOpt(QString key_path, QJsonObject option)
{
    qDebug() << "Set option for" << key_path;
    setDefinition(key_path, option, true);
}

void Settings::setDefinition(QString key_path, QJsonObject option, bool custom)
{
    qDebug() << "Set definition for" << key_path << option;
    QStringList path = key_path.split('.');
    QList<QJsonValue> objects;
    QJsonObject curr_level = *m_settings;

    // Creating required objects
    QStringListIterator i(path);
    QString key;
    QJsonValue data;
    while( i.hasNext() ) {
        key = i.next();

        if( !curr_level.contains(key) )
            curr_level[key] = QJsonObject();

        data = curr_level[key];
        curr_level = data.toObject();

        if( !i.hasNext() ) {
            if( !curr_level.contains("type") )
                qDebug() << "Replacing existing definition" << key_path << curr_level;
            if( curr_level.contains("value") && validate(option, curr_level.value("value")) )
                option["value"] = curr_level.value("value");
            curr_level = option;
            option["custom"] = custom;
            data = option;
        }

        objects.append(data);
    }

    // Collecting objects back
    for( int j = path.size()-2; j >= 0; j-- ) {
        QString prev_key(path[j+1]);
        key = path[j];
        curr_level = objects[j].toObject();
        curr_level[prev_key] = objects[j+1];
        objects[j] = curr_level;
    }

    m_settings->insert(path.first(), objects.first());

    updateVal(key_path, option);
}

QJsonValue Settings::val(QString key_path)
{
    QJsonValue value = opt(key_path);
    if( value.isObject() ) {
        QJsonObject def = value.toObject();
        return def.contains("value") ? def.value("value") : def.value("default");
    }

    qWarning() << "Option is not found for key" << key_path;
    return QJsonValue();
}

void Settings::setVal(QString key_path, QJsonValue value)
{
    qDebug() << "Set option value for" << key_path << value;
    QStringList path = key_path.split('.');
    QList<QJsonValue> objects;
    QJsonObject curr_level = *m_settings;

    // Creating required objects
    QStringListIterator i(path);
    QString key;
    QJsonValue data;
    while( i.hasNext() ) {
        key = i.next();

        if( !curr_level.contains(key) ) {
            qWarning() << "Unable to set undefined option:" << key_path << "key:" << key;
            return;
        }

        data = curr_level[key];
        curr_level = data.toObject();

        if( !i.hasNext() ) {
            if( !curr_level.contains("type") ) {
                qWarning() << "Unable to find a definition type for option" << key_path << curr_level;
                return;
            }
            if( value.isUndefined() )
                curr_level.remove("value");
            else if( !validate(curr_level, value) )
                return;
            else
                curr_level.insert("value", value);
            data = curr_level;
        }

        objects.append(data);
    }

    // Collecting objects back
    for( int j = path.size()-2; j >= 0; j-- ) {
        QString prev_key(path[j+1]);
        key = path[j];
        curr_level = objects[j].toObject();
        curr_level[prev_key] = objects[j+1];
        objects[j] = curr_level;
    }

    m_settings->insert(path.first(), objects.first());

    updateVal(key_path, data.toObject());
}

void Settings::unsetVal(QString key_path)
{
    setVal(key_path, QJsonValue(QJsonValue::Undefined));
}

bool Settings::validate(QJsonObject option, QJsonValue value)
{
    if( !option.contains("type") ) {
        qWarning() << "Wrong option specification provided" << option;
        return false;
    }

    loadType(option);
    QString type = option["type"].toString();

    if( !m_types.contains(type) ) {
        qWarning() << "No type is registered for validation" << option;
        return false;
    }

    QJSValue validation = m_types[type].property("validate");
    QJSValueList args;
    args << m_jsengine->toScriptValue(option) << m_jsengine->toScriptValue(value);
    QJSValue result = validation.call(args);

    if( !result.isBool() || !result.toBool() ) {
        qWarning() << "Unsuccessful validation of option" << type << "with value" << value << result.toString();
        if( result.isError() ) {
            qDebug() << "Uncaught exception at line"
                     << result.property("lineNumber").toInt()
                     << " " << result.property("message").toString()
                     << result.property("stack").toString()
                     << ":" << result.toString();
        }
        return false;
    }

    return true;
}

void Settings::loadDefinitions()
{
    qDebug("Loading settings definitions...");
    QFile definitions_file(":/settings/definitions.json");
    definitions_file.open(QFile::ReadOnly);
    QJsonObject definitions(QJsonDocument::fromJson(definitions_file.readAll()).object());
    definitions = flattenSettings(&definitions);
    for( const QString& key :  definitions.keys() ) {
        setDefinition(key, definitions.value(key).toObject());
    }

    // Loading plugins list and enable-disable feature
    QJsonObject plugin_active;
    plugin_active["type"] = "boolean";
    plugin_active["default"] = false;
    QString key_path;
    for( const QLatin1String& name : Plugins::I()->listPlugins() ) {
        key_path = "Plugins." + name + ".enabled";
        plugin_active["description"] = QLatin1String("Enable plugin '%1'").arg(name);
        setDefinition(key_path, plugin_active);
        Plugins::I()->settingActivePlugin(key_path, name);
        // Notify plugin to load
        for( const QLatin1String& interface : Plugins::I()->listInterfaces(name) ) {
            plugin_active["description"] = QLatin1String("Activate plugin interface '%1'").arg(interface);
            key_path = "Plugins." + name + '.';
            key_path += QString(interface).split(QLatin1Char('.')).last() + ".active";
            setDefinition(key_path, plugin_active);
            Plugins::I()->settingActiveInterface(key_path, name, interface);
        }
    }
    // Connect plugins to listen on settings changed
    connect(this, &Settings::valueChanged, Plugins::I(), &Plugins::settingChanged);
}

void Settings::loadSettings()
{
    qDebug() << "Loading user settings...";
    QFile settings_file(m_path);
    try {
        if( settings_file.exists() ) {
            settings_file.open(QFile::ReadOnly);
            QJsonObject settings = QJsonDocument::fromJson(settings_file.readAll()).object();
            settings = flattenSettings(&settings);
            for( const QString& key : settings.keys() ) {
                setVal(key, settings.value(key));
            }
            return;
        }
        qDebug() << "No settings file is available" << m_path;
    } catch( QJsonParseError e ) {
        qWarning() << "Unable to parse settings file" << m_path << ":" << e.errorString();
    }
}

void Settings::loadTypes()
{
    qDebug() << "Loading used options types...";
    QJsonObject settings = flattenSettings(m_settings);
    for( const QString& key : settings.keys() ) {
        QJsonValue option = settings.value(key);
        if( !option.isObject() || !option.toObject().contains("type") ) {
            qWarning() << "Unable to load type for improper option definition" << key << option;
            continue;
        }
        loadType(option.toObject());
    }
}

void Settings::loadType(QJsonObject option)
{
    QString type = option.value("type").toString();
    if( m_types.contains(type) )
        return;

    qDebug() << "Loading logic for option type" << type;
    QFile type_file(":/settings/type/"+type.remove('/')+".jss");
    if( type_file.exists() && type_file.open(QFile::ReadOnly) ) {
        QJSValue type_logic = m_jsengine->evaluate(type_file.readAll(), type_file.fileName());
        if( type_logic.isError() )
            qWarning() << "Unable to load type logic due to found error:" << type_logic.toString();
        else
            m_types[type] = type_logic;
    } else
        qDebug() << "No type file logic found" << type_file.fileName();

    if( !m_types.contains(type) ) {
        qDebug() << "Loading default type logic for" << type;
        m_types[type] = m_jsengine->evaluate("(function(){return{'validate': function(OPT, data){return typeof data === OPT.type}}})()");
    }
}

void Settings::updateVal(QString key_path, QJsonObject option)
{
    QJsonValue val = option.contains("value") ? option.value("value") : option.value("default");
    if( QQmlPropertyMap::value(key_path) != val.toVariant() ) {
        insert(key_path, val);
        emit valueChanged(key_path, val.toVariant());
    }
}

void Settings::saveSettings()
{
    qDebug() << "Saving settings to" << m_path;
    QFile settings_file(m_path);
    settings_file.open(QFile::WriteOnly);

    if( settings_file.isWritable() ) {
        QJsonObject settings = flattenSettings(m_settings);
        for( const QString& key : settings.keys() ) {
            QJsonValue value = settings.value(key);
            if( ! value.isObject() ) {
                qWarning() << "Found orphan option value:" << key << value;
                continue;
            }
            QJsonObject def = value.toObject();
            if( def.contains("value") )
                settings[key] = def.value("value");
            else
                settings.remove(key);
        }

        settings_file.write( QJsonDocument(settings).toJson() );
    } else
        qWarning() << "Unable to write settings file" << m_path;
}

QJsonObject Settings::flattenSettings(QJsonObject *settings)
{
    QJsonObject out;
    for( const QString& key : settings->keys() ) {
        QJsonValue value = settings->value(key);
        if( value.isObject() ) {
            QJsonObject obj = value.toObject();
            if( obj.contains("type") )
                out[key] = obj;
            else {
                QJsonObject ret = flattenSettings(&obj);
                for( const QString& ret_key : ret.keys() ) {
                    out[key+'.'+ret_key] = ret.value(ret_key);
                }
            }
        } else
            out[key] = value;
    }

    return out;
}
