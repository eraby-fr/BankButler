#include "savingmanager.hpp"
#include <QFileInfo>
#include <QDirIterator>
#include <QDebug>

const QMap<QString, QString> SavingManager::LUT_Hardcoded_Saving2Balance = { {"LDD","Livret Developpement Du... M Etienne Raby"} ,{"CEL","Compte Epargne Logement M Etienne Raby"} };

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

balances SavingManager::GetBalanceWithSavingRemoved(const balances& fullBalance, const savings& saving)
{
    balances out = fullBalance;

    QList<QString> accountNameLst = saving.uniqueKeys();
    for (int i = 0; i < accountNameLst.size(); ++i)
    {
        QString accountName = accountNameLst.at(i);
        QString balanceKey = LUT_Hardcoded_Saving2Balance[accountName];



        //Find the associated Balance



        /* balance :
         * Compte Cheque Mr Ou Mme Raby Etienne : 489.46Eur
         * Compte Epargne Logement M Etienne Raby : 7349.01Eur
         * Plan Epargne Logement M Etienne Raby : 3090.05Eur
         * Livret A Particuliers M Victor Raby : 2761Eur
         * Livret Developpement Du... M Etienne Raby : 7546.36Eur
         * BOURSORAMA BANQUE : 200.9Eur
         */

        /* Saving :
         * LDD
         * CEL
         * */

    }

    return out;
}
