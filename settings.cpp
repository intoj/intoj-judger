#include "settings.h"
#include <QFile>
#include <QDebug>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

Settings::Settings(QObject *parent) : QObject(parent) {}

static Settings settings;

bool Settings::readConfig()
{
    QFile configFile("config.json");
    if (!configFile.open(QIODevice::ReadOnly))
    {
        qDebug("Cannot open config.json\n");
        return false;
    }

    QByteArray configData = configFile.readAll();

    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(configData, &jsonError));

    if (jsonError.error != QJsonParseError::NoError)
    {
        qDebug() << "config.json error:" << jsonError.errorString();
        return false;
    }

    qDebug() << "Reading config file...\n";

    QJsonObject jsonObj = jsonDoc.object();

    if (jsonObj.value("dataDir").isString())
    {
        dataDir = jsonObj.value("dataDir").toString();
        qDebug() << "dataDir:" << qPrintable(dataDir);
    }
    else
    {
        qDebug() << "Cannot read dataDir.";
        return false;
    }

    if (jsonObj.value("tempDir").isString())
    {
        tempDir = jsonObj.value("tempDir").toString();
        qDebug() << "tempDir:" << qPrintable(tempDir);
    }
    else
    {
        qDebug() << "Cannot read tempDir.";
        return false;
    }

    if (jsonObj.value("redis").isObject())
    {
        QJsonObject redis = jsonObj.value("redis").toObject();
        if (redis.value("ip").isString())
        {
            redisIP = redis.value("ip").toString();
//            qDebug() << "redis.ip: " << redisIP;
        }
        else
        {
            qDebug() << "Cannot read redis config.";
            return false;
        }
        if (redis.value("port").isDouble())
        {
            redisPort = redis.value("port").toInt();
//            qDebug() << "redis.port: " << redis;
        }
        else
        {
            qDebug() << "Cannot read redis config.";
            return false;
        }
        qDebug() << "redis:" << qPrintable(redisIP) << ":" << redisPort;
    }
    else
    {
        qDebug() << "Cannot read redis config.";
        return false;
    }

    if (jsonObj.value("database").isObject())
    {
        QJsonObject database = jsonObj.value("database").toObject();
        if (database.value("name").isString())
            databaseName = database.value("name").toString();
        else
        {
            qDebug() << "Cannot read database config.";
            return false;
        }
        if (database.value("user").isString())
            databaseUser = database.value("user").toString();
        else
        {
            qDebug() << "Cannot read database config.";
            return false;
        }
        if (database.value("host").isString())
            databaseHost = database.value("host").toString();
        else
        {
            qDebug() << "Cannot read database config.";
            return false;
        }
        if (database.value("pass").isString())
            databasePass = database.value("pass").toString();
        else
        {
            qDebug() << "Cannot read database config.";
            return false;
        }
        qDebug() << "databaseuser:" << qPrintable(databaseUser) << "@" << qPrintable(databaseHost) << "\ndatabase:" << qPrintable(databaseName);
    }
    else
    {
        qDebug() << "Cannot read database config.";
        return false;
    }

    if (jsonObj.value("spjTimeLimit").isDouble())
    {
        spjTimeLimit = jsonObj.value("spjTimeLimit").toInt();
        qDebug() << "spj time limit:" << spjTimeLimit << "ms";
    }
    else
    {
        qDebug() << "Cannot read spj time limit.";
        return false;
    }

    if (jsonObj.value("compileTimeLimit").isDouble())
    {
        compileTimeLimit = jsonObj.value("compileTimeLimit").toInt();
        qDebug() << "compile time limit:" << compileTimeLimit << "ms";
    }
    else
    {
        qDebug() << "Cannot read compile time limit.";
        return false;
    }

    if (jsonObj.value("language").isArray())
    {
        QJsonArray language = jsonObj.value("language").toArray();
        for (int i = 0; i < language.count(); ++i)
        {
            if (!language.at(i).isObject())
            {
                qDebug() << "Cannot read language config.";
                return false;
            }
            QJsonObject languageObject = language.at(i).toObject();
            if (!languageObject.value("name").isString() || !languageObject.value("ext").isString() || !languageObject.value("command").isString())
            {
                qDebug() << "Cannot read language config.";
                return false;
            }
            compilers.insert(languageObject.value("name").toString(), qMakePair(languageObject.value("ext").toString(), languageObject.value("command").toString()));
        }
    }
    else
    {
        qDebug() << "Cannot read language config.";
        return false;
    }

    qDebug() << "\nFinished reading config file.\n";

    return true;
}

const QString &Settings::getDataDir() const
{
    return dataDir;
}

const QString &Settings::getTempDir() const
{
    return tempDir;
}

const QString &Settings::getRedisIP() const
{
    return redisIP;
}

int Settings::getRedisPort() const
{
    return redisPort;
}

const QString &Settings::getDatabaseName() const
{
    return databaseName;
}

const QString &Settings::getDatabaseUser() const
{
    return databaseUser;
}

const QString &Settings::getDatabaseHost() const
{
    return databaseHost;
}

const QString &Settings::getDatabasePass() const
{
    return databasePass;
}

int Settings::getSpjTimeLimit() const
{
    return spjTimeLimit;
}

int Settings::getcompileTimeLimit() const
{
    return compileTimeLimit;
}

bool Settings::checkLanguage(const QString &lan) const
{
    return compilers.count(lan);
}

const QString &Settings::getExt(const QString &lan) const
{
    return compilers.find(lan).value().first;
}

const QString &Settings::getCommand(const QString &lan) const
{
    return compilers.find(lan).value().second;
}
