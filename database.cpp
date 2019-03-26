#include <QDebug>
#include <QElapsedTimer>
#include <QSqlError>
#include <QCoreApplication>
#include <unistd.h>
#include "database.h"

Database::Database(QObject *parent) : QObject(parent) {}

void Database::setSettings(Settings *_settings)
{
    settings = _settings;
}

bool Database::init()
{
    database = mysql_init(nullptr);
    if (!database)
        return false;
    database = mysql_real_connect(database, settings->getDatabaseHost().toUtf8().data(),
                                  settings->getDatabaseUser().toUtf8().data(),
                                  settings->getDatabasePass().toUtf8().data(),
                                  settings->getDatabaseName().toUtf8().data(),
                                  0, nullptr, 0);
    if (!database)
        return false;
    mysql_set_character_set(database, "utf8");
    return true;
}

void Database::Connect()
{
    while (mysql_ping(database))
    {
        if (!init())
        {
            qDebug("Cannot connet to database. retrying...");
            sleep(1);
        }
    }
}

int Database::getProblemID(int runid)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    if (mysql_query(database, QString("SELECT problem_id from records WHERE id=%1").arg(runid).toUtf8().data()))
    {
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when query problem_id.";
        exit(-1);
    }
    MYSQL_RES *result = mysql_store_result(database);
    if (result == nullptr)
    {
        qDebug() << "Error occured when query problem_id.";
        exit(-1);
    }
    MYSQL_ROW row;
    int res = -1;
    while ((row = mysql_fetch_row(result)))
    {
        if (row[0])
            res = QString(row[0]).toInt();
    }
    mysql_free_result(result);
    return res;
}

QString Database::getSource(int runid)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    if (mysql_query(database, QString("SELECT code from records WHERE id=%1").arg(runid).toUtf8().data()))
    {
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when query code.";
        exit(-1);
    }
    MYSQL_RES *result = mysql_store_result(database);
    if (result == nullptr)
    {
        qDebug() << "Error occured when query code.";
        exit(-1);
    }
    MYSQL_ROW row;
    QString res;
    while ((row = mysql_fetch_row(result)))
    {
        if (row[0])
            res = QString::fromUtf8(row[0]);
    }
    mysql_free_result(result);
    return res;
}

QString Database::getLanguage(int runid)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    if (mysql_query(database, QString("SELECT language from records WHERE id=%1").arg(runid).toUtf8().data()))
    {
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when query language.";
        exit(-1);
    }
    MYSQL_RES *result = mysql_store_result(database);
    if (result == nullptr)
    {
        qDebug() << "Error occured when query language.";
        exit(-1);
    }
    MYSQL_ROW row;
    QString res;
    while ((row = mysql_fetch_row(result)))
    {
        if (row[0])
            res = QString(row[0]);
    }
    mysql_free_result(result);
    return res;
}

int Database::getTimeLimit(int problemid)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    if (mysql_query(database, QString("SELECT time_limit from problems WHERE id=%1").arg(problemid).toUtf8().data()))
    {
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when query time_limit.";
        exit(-1);
    }
    MYSQL_RES *result = mysql_store_result(database);
    if (result == nullptr)
    {
        qDebug() << "Error occured when query time_limit.";
        exit(-1);
    }
    MYSQL_ROW row;
    int res = -1;
    while ((row = mysql_fetch_row(result)))
    {
        if (row[0])
            res = QString(row[0]).toInt();
    }
    mysql_free_result(result);
    return res;
}

int Database::getMemoryLimit(int problemid)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    if (mysql_query(database, QString("SELECT memory_limit from problems WHERE id=%1").arg(problemid).toUtf8().data()))
    {
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when query memory_limit.";
        exit(-1);
    }
    MYSQL_RES *result = mysql_store_result(database);
    if (result == nullptr)
    {
        qDebug() << "Error occured when query memory_limit.";
        exit(-1);
    }
    MYSQL_ROW row;
    int res = -1;
    while ((row = mysql_fetch_row(result)))
    {
        if (row[0])
            res = QString(row[0]).toInt();
    }
    mysql_free_result(result);
    return res;
}

void Database::initRecord(int runid)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    if (mysql_query(database, QString("UPDATE records SET status=0,score=0,time_usage=0,memory_usage=0,result='{\"subtasks\":[]}',compilation='',system_message='' WHERE id=%1").arg(runid).toUtf8().data()))
    {
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when init record.";
        exit(-1);
    }
}

void Database::updateStatus(int runid, int status)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    if (mysql_query(database, QString("UPDATE records SET status=%1 WHERE id=%2").arg(status).arg(runid).toUtf8().data()))
    {
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when update status.";
        exit(-1);
    }
}

void Database::updateCompilation(int runid, const QString &message)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    char *buffer = new char[message.size() * 2 + 1];
    mysql_real_escape_string(database, buffer, message.toUtf8().data(), message.toUtf8().size());

    if (mysql_query(database, QString("UPDATE records SET compilation='%1' WHERE id=%2").arg(buffer).arg(runid).toUtf8().data()))
    {
        delete [] buffer;
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when update compilation.";
        exit(-1);
    }
    delete [] buffer;
}

void Database::updatesystemMessage(int runid, const QString &message)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    char *buffer = new char[message.size() * 2 + 1];
    mysql_real_escape_string(database, buffer, message.toUtf8().data(), message.toUtf8().size());

    if (mysql_query(database, QString("UPDATE records SET system_message='%1' WHERE id=%2").arg(buffer).arg(runid).toUtf8().data()))
    {
        delete [] buffer;
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when update system_message.";
        exit(-1);
    }
    delete [] buffer;
}

void Database::updateResult(int runid, const QString &message)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    char *buffer = new char[message.size() * 2 + 1];
    mysql_real_escape_string(database, buffer, message.toUtf8().data(), message.toUtf8().size());

    if (mysql_query(database, QString("UPDATE records SET result='{\"subtasks\": %1}' WHERE id=%2").arg(buffer).arg(runid).toUtf8().data()))
    {
        delete [] buffer;
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when update result.";
        exit(-1);
    }
    delete [] buffer;
}

void Database::updateTime(int runid, int timeUsed)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    if (mysql_query(database, QString("UPDATE records SET time_usage=%1 WHERE id=%2").arg(timeUsed).arg(runid).toUtf8().data()))
    {
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when update time_usage.";
        exit(-1);
    }
}

void Database::updateMemory(int runid, int memoryUsed)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    if (mysql_query(database, QString("UPDATE records SET memory_usage=%1 WHERE id=%2").arg(memoryUsed).arg(runid).toUtf8().data()))
    {
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when update memory_usage.";
        exit(-1);
    }
}

void Database::updateScore(int runid, int score)
{
    std::unique_lock<std::mutex> Lock(database_mutex);
    Connect();

    if (mysql_query(database, QString("UPDATE records SET score=%1 WHERE id=%2").arg(score).arg(runid).toUtf8().data()))
    {
        qDebug() << mysql_error(database);
        qDebug() << "Error occured when update score.";
        exit(-1);
    }
}
