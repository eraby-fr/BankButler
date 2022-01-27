#include "savingmanager.hpp"
#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>

SavingManager::SavingManager(QSettings &config):
    m_settings(config)
{
    QString filepath = config.fileName();
    QFileInfo fileinfo(filepath);
    m_savingFolderPath = fileinfo.absolutePath();
    m_savingFolderPath.append("/Saving/");
}

void SavingManager::ParseFileSystem()
{
    QDirIterator it(m_savingFolderPath, QStringList() << "*.csv", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        QString path = it.next();
        m_filePathList.append(path);
    }
}

savings SavingManager::GetSavings()
{
    qDebug() << "SavingManager : Start loading files...";
    savings results;

    m_filePathList.sort();

    for(int i=0; i<m_filePathList.size(); ++i)
    {
        QFile currentFile(m_filePathList.at(i));

        if(!currentFile.open(QIODevice::ReadOnly))
        {
            qWarning() << "Savingmanager: unable to open :" << m_filePathList.at(i);
            continue;
        }

        qDebug() << "    -> Reading :" << m_filePathList.at(i);

        QTextStream in(&currentFile);
        in.setCodec("UTF-8");

        float amount = 0.0f;
        while(!in.atEnd())
        {
            QString line = in.readLine();
            if(line.trimmed().startsWith("#"))
            { continue; }

            QStringList contentLst = line.split(";");
            if(contentLst.size() != 3)
            { continue; }

            if(contentLst.at(0).compare("+") == 0)
            {
                amount += contentLst.at(1).toFloat();
                qDebug() << "        -> amount + " << contentLst.at(1) << "lbl:" << contentLst.at(2);
            }
            else if(contentLst.at(0).compare("-") == 0)
            {
                amount -= contentLst.at(1).toFloat();
                qDebug() << "        -> amount - " << contentLst.at(1) << "lbl:" << contentLst.at(2);
            }
            else
            { continue; }
        }
        currentFile.close();

        QFileInfo fileInfo(currentFile);
        QString accountName = fileInfo.dir().dirName();
        QString entryName = fileInfo.baseName();

        results.insert(accountName, QPair<QString, float>(entryName, amount));

        qDebug() << "    -> For" << accountName << "add saving " << entryName << "amount=" << amount;
    }
    qDebug() << "SavingManager : end loading files.";

    return results;
}

void SavingManager::UpdateBalanceWithSaving(balances& fullBalance, const savings& saving)
{
    QList<QString> accountNameLst = saving.uniqueKeys();
    for (int i = 0; i < accountNameLst.size(); ++i)
    {
        QString savingAccountName = accountNameLst.at(i);
        for (int j = 0; j < fullBalance.size(); ++j)
        {
            QString ballanceAccountName = fullBalance.at(j).first;

            if(savingAccountName == ballanceAccountName)
            {
                qDebug() << "SavingManager : UpdateBalanceWithSaving : match for " << savingAccountName;

                QList<QPair<QString,float>> values = saving.values(savingAccountName);
                double alreadySet = 0.0f;
                for (int k = (values.size()-1); k >= 0 ; --k)
                {
                    double value = static_cast<double>(values.at(k).second);
                    if (value > 0.0f)
                    {
                        alreadySet += value;
                    }
                }
                qDebug() << "SavingManager :                         " << savingAccountName << "has" << alreadySet << "Eur already set";
                fullBalance[j].second.second = alreadySet;
            }
        }
    }
}
