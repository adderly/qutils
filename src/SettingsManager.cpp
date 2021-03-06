#include "qutils/SettingsManager.h"
// Qt
#include <QLocale>
#include <QStandardPaths>
#include <QList>
#include <QDir>
// qutils
#include "qutils/Macros.h"

#define COL_SETTING_NAME "setting_name"
#define COL_SETTING_VALUE "setting_value"
#define COL_SETTING_TYPE "setting_type"
#define DATABASE_CHECK() do { if (m_Database.isOpen() == false) { openDatabase(); createTable(); } } while (0)

namespace zmc
{

QList<SettingsManager *> SettingsManager::m_Instances = QList<SettingsManager *>();
int SettingsManager::m_InstanceLastIndex = 0;

SettingsManager::SettingsManager(QString databaseName, QString tableName, QObject *parent)
    : QObject(parent)
    , m_InstanceIndex(m_InstanceLastIndex)
    , m_DatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + databaseName)
    , m_SettingsTableName(tableName)
    , m_SqlManager()
    , m_Database()
{
    m_Instances.append(this);
    m_InstanceLastIndex++;

    // Create the app data location folder if it doesn't exist.
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (dir.exists() == false) {
        dir.mkpath(dir.path());
    }
}

SettingsManager::~SettingsManager()
{
    m_Instances[m_InstanceIndex] = nullptr;
}

QString SettingsManager::getSystemLanguage() const
{
    QLocale locale;
    QString name = locale.name();
    return name.left(name.indexOf("_"));
}

bool SettingsManager::write(const QString &key, const QVariant &value)
{
    DATABASE_CHECK();

    bool successful = false;
    const QList<SqliteManager::Constraint> values {
        std::make_tuple(COL_SETTING_NAME, key, "AND")
    };

    const QList<QMap<QString, QVariant>> existingData = m_SqlManager.getFromTable(m_Database, m_SettingsTableName, -1, &values);
    const bool exists = existingData.size() > 0;
    QMap<QString, QVariant> newMap;
    newMap[COL_SETTING_NAME] = key;
    newMap[COL_SETTING_VALUE] = value.toByteArray();
    newMap[COL_SETTING_TYPE] = QVariant::fromValue<int>(value.type());

    if (exists) {
        const QVariantMap oldMap = existingData.at(0);
        successful = m_SqlManager.updateInTable(m_Database, m_SettingsTableName, newMap, values);
        QVariant oldValue = oldMap[COL_SETTING_VALUE];
        oldValue.convert(oldMap[COL_SETTING_VALUE].toInt());

        emitSettingChangedInAllInstances(key, oldValue, value);
    }
    else {
        successful = m_SqlManager.insertIntoTable(m_Database, m_SettingsTableName, newMap);
        emitSettingChangedInAllInstances(key, "", value);
    }

    return successful;
}

QVariant SettingsManager::read(const QString &key)
{
    DATABASE_CHECK();

    const QList<SqliteManager::Constraint> values {
        std::make_tuple(COL_SETTING_NAME, key, "AND")
    };

    QVariant value;
    const QList<QMap<QString, QVariant>> existingData = m_SqlManager.getFromTable(m_Database, m_SettingsTableName, -1, &values);
    const bool exists = existingData.size() > 0;
    if (exists) {
        value = existingData.at(0)[COL_SETTING_VALUE];
        value.convert(existingData.at(0)[COL_SETTING_TYPE].toInt());
    }

    return value;
}

bool SettingsManager::remove(const QString &key)
{
    DATABASE_CHECK();

    const QList<SqliteManager::Constraint> constraints {
        std::make_tuple(COL_SETTING_NAME, key, "AND")
    };

    return m_SqlManager.deleteInTable(m_Database, m_SettingsTableName, constraints);
}

bool SettingsManager::exists(const QString &key)
{
    DATABASE_CHECK();

    const QList<SqliteManager::Constraint> constraints {
        std::make_tuple(COL_SETTING_NAME, key, "AND")
    };

    return m_SqlManager.exists(m_Database, m_SettingsTableName, constraints);
}

QString SettingsManager::getDatabaseName() const
{
    return m_DatabaseName;
}

void SettingsManager::setDatabaseName(const QString &databaseName)
{
    if (QDir::isAbsolutePath(databaseName)) {
        LOG_ERROR("Absolute path is given! Database name is just the file name.");
        return;
    }

    m_DatabaseName = databaseName;
    emit databasePathChanged();

    restartDatabase();
    createTable();
}

QString SettingsManager::getSettingsTableName() const
{
    return m_SettingsTableName;
}

void SettingsManager::setSettingsTableName(const QString &tableName)
{
    m_SettingsTableName = tableName;
    emit settingsTableNameChanged();

    restartDatabase();
    createTable();
}

void SettingsManager::createTable()
{
    DATABASE_CHECK();

    QList<SqliteManager::ColumnDefinition> columns {
        SqliteManager::ColumnDefinition(false, SqliteManager::ColumnTypes::TEXT, COL_SETTING_NAME),
        SqliteManager::ColumnDefinition(false, SqliteManager::ColumnTypes::BLOB, COL_SETTING_VALUE),
        SqliteManager::ColumnDefinition(false, SqliteManager::ColumnTypes::INTEGER, COL_SETTING_TYPE)
    };

    m_SqlManager.createTable(m_Database, columns, m_SettingsTableName);
}

void SettingsManager::openDatabase()
{
    if (m_Database.isOpen() == false) {
        m_Database = m_SqlManager.openDatabase(m_DatabaseName);
        emit databaseOpened();
    }
}

void SettingsManager::restartDatabase()
{
    if (m_Database.isOpen()) {
        m_SqlManager.closeDatabase(m_Database);
        emit databaseClosed();
    }

    m_Database = m_SqlManager.openDatabase(m_DatabaseName);
    emit databaseOpened();
}

void SettingsManager::emitSettingChangedInAllInstances(const QString &settingName, const QVariant &oldSettingValue, const QVariant &newSettingValue)
{
    if (oldSettingValue != newSettingValue) {
        for (SettingsManager *man : m_Instances) {
            if (man) {
                man->emitSettingChanged(settingName, oldSettingValue, newSettingValue);
            }
        }
    }
}

void SettingsManager::emitSettingChanged(const QString &settingName, const QVariant &oldSettingValue, const QVariant &newSettingValue)
{
    emit settingChanged(settingName, oldSettingValue, newSettingValue);
}

}
