#include "bankwrapper.hpp"

#include <QProcess>
#include <QString>
#include <QDebug>

BankWrapper::BankWrapper(const QSettings &config)
{
    m_accountID = config.value("Account").toString();
    m_processPath = config.value("BoobankPath").toString();
    m_entryLimit = config.value("BoobankMaxEntries").toInt();
}

bool BankWrapper::RequestAmount(QString &output)
{
    QStringList args;
    args << "list";
    args << "--formatter=csv";
    args << "--no-header";
    args << "--no-keys";

    QProcess boobank;
    qDebug() << "BankWrapper : Start request amounts [" << m_processPath << args.join(" ") << "]";
    boobank.start(m_processPath, args);

    if (!boobank.waitForStarted())
    {
        qFatal("boobank FAILLED to start so we crash the program");
    }

    if (!boobank.waitForFinished(240000))
    {
        qFatal("boobank FAILLED to finished so we crash the program");
    }

    if (boobank.exitCode() != 0)
    {
        qFatal("Exit code not null");
    }

    output = QString(boobank.readAllStandardOutput());

    if (boobank.exitCode() != 0)
    {
        qWarning() << "BankWrapper : Fail to get amount";
        output = QString(boobank.readAllStandardError());
        qDebug() << "BankWrapper error :" << output;
        return false;
    }

    qDebug() << "BankWrapper : End of request amounts.";
    return true;
}


bool BankWrapper::RequestHistory(QString &output, QDate date)
{
    QStringList full_CLI = this->GenerateBooBankHistoryCLI(date);

    QProcess boobank;
    qDebug() << "BankWrapper : Start request history [" << m_processPath << full_CLI.join(" ") << "]";
    boobank.start(m_processPath, full_CLI);

    if (!boobank.waitForStarted())
    {
        qFatal("boobank FAILLED to start so we crash the program");
    }

    if (!boobank.waitForFinished(240000))
    {
        qFatal("boobank FAILLED to finished so we crash the program");
    }

    output = QString(boobank.readAllStandardOutput());


    if (boobank.exitCode() != 0)
    {
        qWarning() << "BankWrapper : Fail to get history";
        output = QString(boobank.readAllStandardError());
        qDebug() << "BankWrapper error :" << output;
        return false;
    }

    qDebug() << "BankWrapper : End of request history.";
    return true;
}

QStringList BankWrapper::GenerateBooBankHistoryCLI(const QDate & date)
{
    QDate conditionDate(date.year(), date.month(), 1);

    conditionDate = conditionDate.addDays(-1);

    QStringList args;
    args << "history";
    args << m_accountID;
    args << "--condition";
    args << QString("date>%3-%4-%5").arg(conditionDate.year()).arg(conditionDate.month()).arg(conditionDate.day());
    args << "-n";
    args << QString::number(m_entryLimit);

    return args;
}
