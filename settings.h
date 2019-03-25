#ifndef SETTINGS_H
#define SETTINGS_H

#include <QMap>
#include <QObject>

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent = nullptr);
    bool readConfig();
    const QString &getDataDir() const;
    const QString &getTempDir() const;
    const QString &getRedisIP() const;
    int getRedisPort() const;
    const QString &getDatabaseName() const;
    const QString &getDatabaseUser() const;
    const QString &getDatabaseHost() const;
    const QString &getDatabasePass() const;
    int getSpjTimeLimit() const;
    int getcompileTimeLimit() const;
    bool checkLanguage(const QString &) const;
    const QString &getExt(const QString &) const;
    const QString &getCommand(const QString &) const;

private:
    QString dataDir;
    QString tempDir;
    QString redisIP;
    int redisPort;
    QString databaseName;
    QString databaseUser;
    QString databaseHost;
    QString databasePass;
    int spjTimeLimit;
    int compileTimeLimit;
    QMap<QString, QPair<QString, QString>> compilers;
};

#endif // SETTINGS_H
