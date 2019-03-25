#ifndef JUDGE_H
#define JUDGE_H

#include <QtCore>
#include <QThread>
#include <QJsonArray>
#include "settings.h"
#include "database.h"

class judgeThread : public QThread
{
    Q_OBJECT
public:
    explicit judgeThread(QObject *parent = nullptr);
    ~judgeThread();
    void setDatabase(Database *);
    void setSettings(Settings *);
    void setTask(int);
    void setContestantName(const QString&);
    void run();

private:
    Database *db;
    Settings *settings;
    int runid;
    int problem_id;
    QString language;
    int timeLimit;
    int memoryLimit;
    QList<QString> inputFile;
    QList<QString> extraTest;
    QString compileMessage;
    QString systemMessage;
    QJsonArray results;
    int timeUsed;
    int memoryUsed;
    int score;
    int status;
    bool readFile();
    bool checkFile();
    bool compile();
    int compare(const QString &, const QString &, QString &);
    void judge();
    void judgeExtraTest();
    void clearPath(const QString &curDir);
};

#endif // JUDGE_H
