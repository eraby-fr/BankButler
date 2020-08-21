#include "liabilitymanager.hpp"
#include <constants.hpp>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QDate>

LiabilityManager::LiabilityManager(const QSettings & config)
{
    QString filepath = config.fileName();
    QFileInfo fileinfo(filepath);
    m_savingFolderPath = fileinfo.absolutePath();
    m_savingFolderPath.append("/Debt/");
}

void LiabilityManager::loadLiabilitiesFromFileSystem()
{
    qDebug() << "LiabilityManager : Start load liabilities from file system...";
    QDirIterator it(m_savingFolderPath, QStringList() << "*.ini", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString path = it.next();

        QSettings debtIni(path, QSettings::IniFormat);

        Liability newDebt;
        newDebt.fileName = path;
        newDebt.name = debtIni.value(QString("Name")).toString();
        newDebt.pattern = QRegularExpression(debtIni.value(QString("Pattern")).toString());
        newDebt.description = debtIni.value(QString("Description")).toString();
        newDebt.initialAmount = debtIni.value(QString("InitialAmount"), -1.0f).toFloat();
        newDebt.currentAmount = debtIni.value(QString("CurrentAmount"), -1.0f).toFloat();
        QString dateStr = debtIni.value(QString("LastPayementDate")).toString();
        QDate lastPayement = QDate::fromString(dateStr, date_format);
        newDebt.payementDoneForThisMonth = lastPayement.month() == QDate::currentDate().month();

        qDebug() << "    -> A new debt file is found :" << path << "values:" << newDebt.name << "initial:" << newDebt.initialAmount << "current:" << newDebt.currentAmount << "last change :" <<dateStr << newDebt.pattern.pattern();
        m_Liabilities.append(newDebt);
    }
    qDebug() << "LiabilityManager : End of load liabilities from file system.";
}
void LiabilityManager::computeHistory(const QString & history)
{
    qDebug() << "LiabilityManager : Start compute history...";
    QStringList list = history.split('\n', QString::SkipEmptyParts);

    for(int i = 0; i < list.size(); ++i)
    {
        QString currentEntry = list.at(i);
        qDebug() << "    -> Processing :" << currentEntry;

        for(int listIndex=0; listIndex < m_Liabilities.size(); ++listIndex )
        {
            QRegularExpressionMatch match = m_Liabilities.at(listIndex).pattern.match(currentEntry);
            if(match.hasMatch())
            {
                qDebug() << "        -> Match with :" << m_Liabilities.at(listIndex).pattern.pattern();
                processLiability(currentEntry, m_Liabilities[listIndex]);
                continue;
            }
        }
    }
    qDebug() << "LiabilityManager : End compute history.";
}

void LiabilityManager::processLiability(const QString & entry, Liability& debt)
{
    if(debt.payementDoneForThisMonth)
    {
        return;
    }

    float amount = entry.right(11).trimmed().toFloat();
    if(amount <0.0f)
    {amount = amount * -1.0f;}

    debt.currentAmount -= amount;
    debt.payementDoneForThisMonth = true;

    //Update ini file
    QSettings debtIni(debt.fileName, QSettings::IniFormat);
    debtIni.setValue(QString("CurrentAmount"), debt.currentAmount);
    QString dateStr = entry.left(11).trimmed();
    debtIni.setValue(QString("LastPayementDate"), dateStr);
    debtIni.sync();
}

const QList<Liability> & LiabilityManager::getLiabilities() const
{
    return m_Liabilities;
}
