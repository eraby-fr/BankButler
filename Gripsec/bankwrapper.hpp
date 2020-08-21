#pragma once

#include <QSettings>
#include <QDate>
#include <QString>

class BankWrapper
{
public:
    BankWrapper(const QSettings &config);

    bool RequestAmount(QString &output);
    bool RequestHistory(QString &output, QDate date = QDate::currentDate());

#ifdef UNIT_TEST
    struct BankWrapperBackDoor;
    friend BankWrapperBackDoor;
#endif

private:
    QStringList GenerateBooBankHistoryCLI(const QDate & date);

    QString m_accountID;
    QString m_processPath;
    int m_entryLimit;
};
