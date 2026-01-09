#include "liabilitymanager.hpp"
#include <constants.hpp>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QDate>

const int INDEX_HISTORY_AMOUNT = 10;
const int INDEX_HISTORY_DATE = 2;

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
    QStringList list = history.split('\n', Qt::SkipEmptyParts);

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
        qDebug() << "    -> LiabilityManager drop because payement already done";
        return;
    }

    QStringList splittedLine = entry.split(";");
    if(splittedLine.size() < INDEX_HISTORY_AMOUNT)
    {
        qDebug() << "    -> DROP Line : because splittedLine.size() < INDEX_HISTORY_AMOUNT";
    }

    QDate date = QDate::fromString(splittedLine.at(INDEX_HISTORY_DATE).split(" ")[0], date_format);
    float amount = splittedLine.at(INDEX_HISTORY_AMOUNT).toFloat();

    if(amount <0.0f)
    {amount = amount * -1.0f;}

    debt.currentAmount -= amount;
    debt.payementDoneForThisMonth = true;

    qDebug() << "    -> Update liability: new date:" << date.toString(date_format) << "amount applied:" << amount << "New remaining:"<<debt.currentAmount;
    //Update ini file
    QSettings debtIni(debt.fileName, QSettings::IniFormat);
    debtIni.setValue(QString("CurrentAmount"), debt.currentAmount);
    debtIni.setValue(QString("LastPayementDate"), date.toString(date_format));
    debtIni.sync();
}

const QList<Liability> & LiabilityManager::getLiabilities() const
{
    return m_Liabilities;
}
