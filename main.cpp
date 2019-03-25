#include <hiredis/hiredis.h>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QDebug>
#include <QString>
#include "judge.h"
#include "settings.h"
#include "database.h"

static Settings settings;
static Database db;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (!settings.readConfig())
    {
        qDebug() << "Cannot read config file.";
        return -1;
    }

    redisContext *c = redisConnect(settings.getRedisIP().toLatin1().data(), settings.getRedisPort());
    int retrytimes = 0;
    while ((c == nullptr || c->err) && retrytimes < 10)
    {
        qDebug("Cannot connect to redis, retrying... (times = %d)", ++retrytimes);
        QElapsedTimer t;
        t.start();
        while (t.elapsed() <= 1000)
            QCoreApplication::processEvents();
        c = redisConnect(settings.getRedisIP().toLatin1().data(), settings.getRedisPort());
    }

    if (c == nullptr || c->err)
    {
        if (c == nullptr)
            qDebug("\nCannot allocate redis context.");
        else
            qDebug("\nError: %s", c->errstr);
        qDebug("Please restart the judger.");
        return -1;
    }

    db.setSettings(&settings);

    if (!db.init())
    {
        qDebug("Cannot connect to database.\nPlease restart the judger.");
        return -1;
    }

    qDebug() << "Judger started successfully. Waiting for submitting...";

    while (true)
    {
        redisReply *reply = static_cast<redisReply *>(redisCommand(c, "LPOP intoj-waiting"));
        if (reply == nullptr || reply->type == REDIS_REPLY_NIL)
        {
            QElapsedTimer t;
            t.start();
            while (t.elapsed() <= 200)
                QCoreApplication::processEvents();
            continue;
        }
        int runid = QString(reply->str).toInt();
        qDebug() << "got submit: " << runid;
        judgeThread *judge = new judgeThread();
        judge->setDatabase(&db);
        judge->setSettings(&settings);
        judge->setTask(runid);
        judge->run();
        delete judge;
        QCoreApplication::processEvents();
    }

    return a.exec();
}
