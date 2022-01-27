#pragma once

#include <QSettings>
#include <Gripsec/constants.hpp>
#include <QMap>

class SavingManager
{
public:
    SavingManager(QSettings &config);

    void ParseFileSystem();

    savings GetSavings();

    void UpdateBalanceWithSaving(balances& realAmount, const savings& saving);

#ifdef UNIT_TEST
    struct SavingManagerBackDoor;
    friend SavingManagerBackDoor;
#endif

private:
    QString m_savingFolderPath;
    QStringList m_filePathList;
    QSettings &m_settings;
};
