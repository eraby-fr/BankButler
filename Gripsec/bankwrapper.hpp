#pragma once

#include <QSettings>
#include <QDate>
#include <QString>

class BankWrapper
{
public:
    BankWrapper(const QSettings &config);

    bool RequestAmount(QString &output, bool stubWoob=false);
    bool RequestHistory(QString &output, QDate date = QDate::currentDate(), bool stubWoob=false);

#ifdef UNIT_TEST
    struct BankWrapperBackDoor;
    friend BankWrapperBackDoor;
#endif

private:
    bool InjectHardcodedAmount(QString &output);
    bool InjectHardcodedHistory(QString &output, QDate date);

    QStringList GenerateBooBankHistoryCLI(const QDate & date);

    QString m_accountID;
    QString m_processPath;
    int m_entryLimit;
};
