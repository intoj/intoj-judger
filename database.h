#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <mysql.h>
#include <mutex>
#include <QSqlDatabase>
#include "settings.h"

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    void setSettings(Settings *);
    bool init();
    void initRecord(int);
    int getProblemID(int);
    QString getLanguage(int);
    QString getSource(int);
    int getTimeLimit(int);
    int getMemoryLimit(int);
    void updateStatus(int, int);
    void updateCompilation(int, const QString &);
    void updatesystemMessage(int, const QString &);
    void updateResult(int, const QString &);
    void updateTime(int, int);
    void updateMemory(int, int);
    void updateScore(int, int);

private:
    Settings *settings;
    MYSQL *database;
    std::mutex database_mutex;
    void Connect();
};

#endif // DATABASE_H
