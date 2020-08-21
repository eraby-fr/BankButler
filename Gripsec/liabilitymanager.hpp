#pragma once

#include <Gripsec/debt.hpp>
#include <QSettings>
#include <QList>
#include <QFile>

class LiabilityManager
{
public:
    LiabilityManager(const QSettings & config);

    void loadLiabilitiesFromFileSystem();

    void computeHistory(const QString & history);

    const QList<Liability> & getLiabilities() const;

#ifdef UNIT_TEST
    struct LiabilityManagerBackDoor;
    friend LiabilityManagerBackDoor;
#endif
private:
    void processLiability(const QString & entry, Liability &debt);

    QList<Liability> m_Liabilities;

    QString m_savingFolderPath;
};
