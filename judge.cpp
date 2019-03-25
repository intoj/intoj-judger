#include "judge.h"
#include "database.h"
#include <cstdio>
#include <algorithm>
#include <QJsonObject>

judgeThread::judgeThread(QObject *parent) : QThread(parent)
{
    moveToThread(this);
}

void judgeThread::setDatabase(Database *_db)
{
    db = _db;
}

void judgeThread::setSettings(Settings *_settings)
{
    settings = _settings;
}

void judgeThread::clearPath(const QString &curDir)
{
    QDir dir(curDir);
    QStringList fileList = dir.entryList(QDir::Files);
    for (int i = 0; i < fileList.size(); i ++) {
        if (! dir.remove(fileList[i])) {
            QProcess::execute(QString("chmod +w \"") + curDir + fileList[i] + "\"");
            dir.remove(fileList[i]);
        }
    }
    QStringList dirList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < dirList.size(); i ++) {
        clearPath(curDir + dirList[i] + QDir::separator());
        dir.rmdir(dirList[i]);
    }
}

judgeThread::~judgeThread()
{
    clearPath(settings->getTempDir());
}

void judgeThread::setTask(int _runid)
{
    clearPath(settings->getTempDir());
    runid = _runid;
    problem_id = db->getProblemID(runid);
    language = db->getLanguage(runid);
    timeLimit = db->getTimeLimit(problem_id);
    memoryLimit = db->getMemoryLimit(problem_id);
    score = 0;
    QFile source(settings->getTempDir() + "/source");
    if (!source.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug() << "Cannot creat source file";
        exit(-1);
    }
    source.write(db->getSource(runid).toUtf8());
    source.close();
    qDebug() << "problemID:" << problem_id;
    qDebug() << "Language:" << language;
    qDebug() << "time limit:" << timeLimit << "ms";
    qDebug() << "memory limit:" << memoryLimit << "MB";
}

bool judgeThread::compile()
{
    qDebug() << "Compiling...";
    if (!settings->checkLanguage(language))
    {
        compileMessage = "Language not found.";
        return false;
    }
    QString extName = settings->getExt(language);
    QFile source(settings->getTempDir() + "/source");
    source.rename(settings->getTempDir() + "/a." + extName);
    source.close();
    QString command = settings->getCommand(language);
    command.replace("%s.*", "a." + extName);
    command.replace("%s", "a");
    QProcess *compiler = new QProcess();
    compiler->setProcessChannelMode(QProcess::MergedChannels);
    compiler->setWorkingDirectory(settings->getTempDir());
    compiler->start(command);
    if (!compiler->waitForStarted(-1))
    {
        compileMessage = "Cannot start compiler.";
        delete compiler;
        return false;
    }
    QElapsedTimer t;
    t.start();
    bool isStopped = false;
    while (t.elapsed() < settings->getcompileTimeLimit())
    {
        if (compiler->state() != QProcess::Running)
        {
            isStopped = true;
            break;
        }
        QCoreApplication::processEvents();
        msleep(10);
    }
    bool res = true;
    if (!isStopped)
    {
        compiler->kill();
        compileMessage = "Compile time limit.";
        res = false;
    }
    else
    {
        compileMessage = QString::fromUtf8(compiler->readAllStandardOutput().data());
        if (compiler->exitCode() != 0)
            res = false;
    }
    delete compiler;
    return res;
}

bool judgeThread::readFile()
{
    qDebug() << "Reading data files...";
    QDir dir(settings->getDataDir() + "/" + QString::number(problem_id));
    if (!dir.exists())
        return false;
    QStringList filters;
    filters << QString("*.in");
    QStringList fileList = dir.entryList(filters, QDir::Files);
    for (const QString &s : fileList)
        if (s.mid(0, 3) == "ex_")
            extraTest.push_back(s);
        else
            inputFile.push_back(s);
    std::sort(extraTest.begin(), extraTest.end(), [](const QString &s1, const QString &s2) {
        return s1.size() < s2.size() || (s1.size() == s2.size() && s1 < s2);
    });
    std::sort(inputFile.begin(), inputFile.end(), [](const QString &s1, const QString &s2) {
        return s1.size() < s2.size() || (s1.size() == s2.size() && s1 < s2);
    });
    return !inputFile.empty();
}

bool judgeThread::checkFile()
{
    for (QString s : inputFile)
    {
        *++s.rbegin() = 'o';
        *s.rbegin() = 'u';
        s.push_back('t');
        if (!QFile(settings->getDataDir() + "/" + QString::number(problem_id) + "/" + s).exists())
        {
            return false;
        }
    }
    for (QString s : extraTest)
    {
        *++s.rbegin() = 'o';
        *s.rbegin() = 'u';
        s.push_back('t');
        if (!QFile(settings->getDataDir() + "/" + QString::number(problem_id) + "/" + s).exists())
        {
            return false;
        }
    }
    return true;
}

int judgeThread::compare(const QString &contestantOutput, const QString &outputFile, QString &message)
{
        FILE *contestantOutputFile = fopen(contestantOutput.toLocal8Bit().data(), "r");
        FILE *standardOutputFile = fopen(outputFile.toLocal8Bit().data(), "r");

        if (contestantOutputFile == nullptr || standardOutputFile == nullptr)
            return -1;

        int ch1 = '\n', ch2 = '\n';
        char str1[20], str2[20];
        int flag1, flag2;
        while (true) {
            if (ch1 == '\n' || ch1 == '\r' || ch1 == EOF) {
                if (ch1 == '\r') {
                    ch1 = fgetc(contestantOutputFile);
                    if (ch1 == '\n') ch1 = fgetc(contestantOutputFile);
                } else {
                    ch1 = fgetc(contestantOutputFile);
                }
                while (ch1 == ' ' || ch1 == '\t') {
                    ch1 = fgetc(contestantOutputFile);
                }
                flag1 = 2;
            } else {
                if (ch1 == ' ' || ch1 == '\t') {
                    while (ch1 == ' ' || ch1 == '\t') {
                        ch1 = fgetc(contestantOutputFile);
                    }
                    if (ch1 == '\n' || ch1 == '\r' || ch1 == EOF) {
                        if (ch1 == '\r') {
                            ch1 = fgetc(contestantOutputFile);
                            if (ch1 == '\n') ch1 = fgetc(contestantOutputFile);
                        } else {
                            ch1 = fgetc(contestantOutputFile);
                        }
                        while (ch1 == ' ' || ch1 == '\t') {
                            ch1 = fgetc(contestantOutputFile);
                        }
                        flag1 = 2;
                    } else {
                        flag1 = 1;
                    }
                } else {
                    flag1 = 0;
                }
            }

            if (ch2 == '\n' || ch2 == '\r' || ch2 == EOF) {
                if (ch2 == '\r') {
                    ch2 = fgetc(standardOutputFile);
                    if (ch2 == '\n') ch2 = fgetc(standardOutputFile);
                } else {
                    ch2 = fgetc(standardOutputFile);
                }
                while (ch2 == ' ' || ch2 == '\t')
                    ch2 = fgetc(standardOutputFile);
                flag2 = 2;
            } else {
                if (ch2 == ' ' || ch2 == '\t') {
                    while (ch2 == ' ' || ch2 == '\t') {
                        ch2 = fgetc(standardOutputFile);
                    }
                    if (ch2 == '\n' || ch2 == '\r' || ch2 == EOF) {
                        if (ch2 == '\r') {
                            ch2 = fgetc(standardOutputFile);
                            if (ch2 == '\n') ch2 = fgetc(standardOutputFile);
                        } else {
                            ch2 = fgetc(standardOutputFile);
                        }
                        while (ch2 == ' ' || ch2 == '\t') {
                            ch2 = fgetc(standardOutputFile);
                        }
                        flag2 = 2;
                    } else {
                        flag2 = 1;
                    }
                } else {
                    flag2 = 0;
                }
            }

            if (flag1 != flag2) {
                message = "Presentation error";
                fclose(contestantOutputFile);
                fclose(standardOutputFile);
                return 0;
            }

            int len1 = 0;
            while (len1 < 10) {
                if (ch1 != ' ' && ch1 != '\t' && ch1 != '\n' && ch1 != '\r' && ch1 != EOF) {
                    str1[len1 ++] = static_cast<char>(ch1);
                } else {
                    break;
                }
                ch1 = fgetc(contestantOutputFile);
            }
            str1[len1] = '\0';

            int len2 = 0;
            while (len2 < 10) {
                if (ch2 != ' ' && ch2 != '\t' && ch2 != '\n' && ch2 != '\r' && ch2 != EOF) {
                    str2[len2 ++] = static_cast<char>(ch2);
                } else {
                    break;
                }
                ch2 = fgetc(standardOutputFile);
            }
            str2[len2] = '\0';

            if (len1 != len2 || strcmp(str1, str2) != 0) {
                message = QString("Read %1 but expect %2").arg(str1).arg(str2);
                fclose(contestantOutputFile);
                fclose(standardOutputFile);
                return 0;
            }
            if (ch1 == EOF && ch2 == EOF) break;

            QCoreApplication::processEvents();
        }

        fclose(contestantOutputFile);
        fclose(standardOutputFile);
        return 1;
}

void judgeThread::judge()
{
    qDebug() << "Judging...";
    QList<int> fullScore;
    int n = inputFile.size();
    for (int i = 1; i != n; ++i)
        fullScore.push_back(100 / n);
    fullScore.push_back(100 / n + 100 % n);
    status = 10;
    timeUsed = 0;
    memoryUsed = 0;
    for (int i = 0; i != n; ++i)
    {
        results.append(QJsonObject {
                          {"id", i + 1},
                          {"status", i == 0 ? 1 : 0},
                          {"score", 0},
                          {"full_score", fullScore[i]},
                          {"time_usage", 0},
                          {"memory_usage", 0},
                          {"judger_message", ""},
                          {"checker_message", ""}
                       });
    }
    QFile::copy(":/resources/runner_linux", settings->getTempDir() + "/runner_linux");
    QProcess::execute(QString("chmod +wx \"") + settings->getTempDir() + "/runner_linux" + "\"");
    for (int i = 0; i < n; ++i)
    {
        QJsonObject res = results.at(i).toObject();
        res.remove("status");
        res.insert("status", 1);
        results.replace(i, res);
        db->updateResult(runid, QString(QJsonDocument(results).toJson()));
        db->updateTime(runid, timeUsed);
        db->updateMemory(runid, memoryUsed);
        db->updateScore(runid, score);
        QString input = inputFile[i];
        QString output = input;
        *++output.rbegin() = 'o';
        *output.rbegin() = 'u';
        output.push_back('t');
        QString inputFull = settings->getDataDir() + "/" + QString::number(problem_id) + "/" + input;
        QString outputFull = settings->getDataDir() + "/" + QString::number(problem_id) + "/" + output;
        qDebug() << inputFull;
        QProcess *runner = new QProcess();
        QStringList argumentList;
        argumentList << "./a" << QString::number(timeLimit) << QString::number(memoryLimit * 1024 * 1024) << inputFull << "__tmpout";
        runner->setWorkingDirectory(settings->getTempDir());
        runner->start(settings->getTempDir() + "/runner_linux", argumentList);
        res = QJsonObject {
            {"id", i + 1},
            {"status", 10},
            {"score", 0},
            {"full_score", fullScore[i]},
            {"time_usage", 0},
            {"memory_usage", 0},
            {"judger_message", ""},
            {"checker_message", ""}
         };
        if (!runner->waitForStarted(-1))
        {
            delete runner;
            res.remove("status");
            res.remove("judger_message");
            res.insert("status", 2);
            res.insert("judger_message", "Runner error.");
            if (status == 10)
                status = 2;
            results.replace(i, res);
            continue;
        }
        runner->waitForFinished(-1);
        if (runner->exitCode() != 0)
        {
            delete runner;
            res.remove("status");
            res.remove("judger_message");
            res.insert("status", 2);
            res.insert("judger_message", "Runner error.");
            if (status == 10)
                status = 2;
            results.replace(i, res);
            continue;
        }
        delete runner;
        int curtime, curmemory, curstatus;
        QString curinfo;

        QFile run_info(settings->getTempDir() + "/run.info");
        if (!run_info.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            res.remove("status");
            res.remove("judger_message");
            res.insert("status", 2);
            res.insert("judger_message", "Runner error.");
            if (status == 10)
                status = 2;
            results.replace(i, res);
            continue;
        }
        QTextStream instream(&run_info);
        curtime = instream.readLine().toInt();
        curmemory = instream.readLine().toInt() / 1024;
        curstatus = instream.readLine().toInt();
        curinfo = instream.readLine();
        res.remove("time_usage");
        res.remove("memory_usage");
        res.remove("judger_message");
        res.insert("time_usage", curtime);
        res.insert("memory_usage", curmemory);
        res.insert("judger_message", curinfo);
        run_info.close();
        timeUsed += curtime;
        memoryUsed = std::max(memoryUsed, curmemory);
        if (curstatus != 10)
        {
            if (status == 10)
                status = curstatus;
            res.remove("status");
            res.insert("status", curstatus);
            results.replace(i, res);
            continue;
        }
        QString checker_message;
        int t = compare(settings->getTempDir() + "/__tmpout", outputFull, checker_message);
        if (t == -1)
        {
            res.remove("status");
            res.remove("judger_message");
            res.insert("status", 2);
            res.insert("judger_message", "Checker error.");
            if (status == 10)
                status = 2;
            results.replace(i, res);
            continue;
        }
        if (t == 0)
        {
            curstatus = 5;
            if (status == 10)
                status = curstatus;
            res.remove("status");
            res.insert("status", curstatus);
            res.remove("checker_message");
            res.insert("checker_message", checker_message);
            results.replace(i, res);
            continue;
        }
        res.remove("score");
        res.insert("score", fullScore[i]);
        results.replace(i, res);
        score += fullScore[i];
    }
    db->updateResult(runid, QString(QJsonDocument(results).toJson()));
}

void judgeThread::judgeExtraTest()
{

}

void judgeThread::run()
{
    db->initRecord(runid);
    db->updateStatus(runid, 1);
    if (!compile())
    {
        qDebug() << "Compile error.\n";
        db->updateStatus(runid, 3);
        db->updateCompilation(runid, compileMessage);
        return;
    }
    qDebug() << "Compile success.\n";
    if (!readFile())
    {
        qDebug() << "Read data files error.\n";
        db->updateStatus(runid, 2);
        db->updatesystemMessage(runid, "Data file not found.");
        return;
    }
    if (!checkFile())
    {
        qDebug() << "Answer file not found.\n";
        db->updateStatus(runid, 2);
        db->updatesystemMessage(runid, "Answer file not found.");
        return;
    }
    qDebug() << "Read data files success," << inputFile.size() << "data(s)," << extraTest.size() << "extra test(s).\n";
    judge();
    db->updateStatus(runid, status);
    db->updateTime(runid, timeUsed);
    db->updateMemory(runid, memoryUsed);
    db->updateScore(runid, score);
    qDebug() << "Finished judging.\n";
}
