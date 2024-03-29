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

bool BankWrapper::RequestAmount(QString &output, bool stubWoob)
{
    if(stubWoob)
    {
        return InjectHardcodedAmount(output);
    }

    QStringList args;
    args << "bank";
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


bool BankWrapper::RequestHistory(QString &output, QDate date, bool stubWoob)
{
    if(stubWoob)
    {
        return InjectHardcodedHistory(output, date);
    }

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
    args << "bank";
    args << "history";
    args << m_accountID;
    args << "--condition";
    args << QString("date>%3-%4-%5").arg(conditionDate.year()).arg(conditionDate.month()).arg(conditionDate.day());
    args << "-n";
    args << QString::number(m_entryLimit);

    return args;
}

bool BankWrapper::InjectHardcodedAmount(QString &output)
{
    output = QString("id;url;label;currency;bank_name;type;owner_type;balance;coming;iban;ownership;paydate;paymin;cardlimit;number;valuation_diff;valuation_diff_ratio;company_name;parent;opening_date\n"\
                        "myMainAccount@mybank;None;Compte Cheque Mr Ou Mme Raby Etienne;EUR;NotLoaded;1;NotAvailable;10000.01;NotLoaded;NotLoaded;co-owner;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded\n"\
                        "NotLoaded@banquepopulaire;None;Compte Epargne Logement M Etienne Raby;EUR;NotLoaded;2;NotAvailable;20000.02;NotLoaded;NotAvailable;owner;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded\n"\
                        "NotLoaded@banquepopulaire;None;Plan Epargne Logement M Etienne Raby;EUR;NotLoaded;2;NotAvailable;30000.03;NotLoaded;NotAvailable;owner;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded\n"\
                        "NotLoaded@banquepopulaire;None;Livret Developpement Du... M Etienne Raby;EUR;NotLoaded;2;NotAvailable;40000.04;NotLoaded;NotAvailable;owner;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded;NotLoaded\n"
                        );
    return true;
}
bool BankWrapper::InjectHardcodedHistory(QString &output, QDate date)
{
    output = QString(" Date         Category     Label                                                  Amount\n"\
                        "------------+------------+---------------------------------------------------+-----------\n"\
                        " %1-%2-%3   Card         SNCF DOUAI                                             200.00\n"\
                        " %1   Transfer     HELLO WORLD                                             39.03\n"\
                        " 2022-01-26   Card         BOUTIQ75PARIS                                          -11.60\n"
                        )
                .arg(date.year()).arg(date.month()).arg(date.day())
                ;
    return true;
}