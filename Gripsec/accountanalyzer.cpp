#include "accountanalyzer.hpp"
#include <Gripsec/constants.hpp>

#include <QDebug>
#include <QStringList>
#include <QRegularExpression>
#include <QDate>


const int INDEX_LABEL = 2;
const int INDEX_BALANCE = 7;
//const int INDEX_CURRENCY = 3;

AccountAnalyzer::AccountAnalyzer(const QSettings &config) :
    m_MonthlyAlreadySpent(0.0f),
    m_SumOfExpectedMonthlyExpenses(0.0f)
{
    qDebug() << "AccountAnalyzer : Start loading configuration...";
    int numOfRegExp = config.value("NumOfRegExp", 0).toInt();
    qDebug() << "    -> Number of categories =" << numOfRegExp;

    for(int i=0; i<numOfRegExp; ++i )
    {
        ReGexpEnty newEntry;
        newEntry.category = config.value(QString("RegExp/%1/label").arg(i)).toString();

        QStringList tmpList = config.value(QString("RegExp/%1/regexp").arg(i), "No Corrresponding RegExp").toString().split(list_separator);
        for (int i = 0; i < tmpList.size(); ++i)
        {
            newEntry.patternList.append(QRegularExpression(tmpList.at(i)));
        }
        newEntry.maxBeforeWarning = config.value(QString("RegExp/%1/threshold").arg(i), -1.0f).toFloat();

        qDebug() << "        -> New category [" << newEntry.category << ":" << newEntry.maxBeforeWarning << ":" << tmpList << "]";
        m_RegExp.append(newEntry);
    }

    int numOfMonthlyExpense = config.value("NumOfMonthlyExpense", 0).toInt();
    qDebug() << "    -> Number of monthly expenses =" << numOfMonthlyExpense;
    for(int i=0; i<numOfMonthlyExpense; ++i )
    {
        MonthlyExpense newEntry;
        newEntry.dbgLabel = config.value(QString("MonthlyExpense/%1/label").arg(i)).toString();
        newEntry.pattern = QRegularExpression(config.value(QString("MonthlyExpense/%1/pattern").arg(i)).toString());
        newEntry.amount = config.value(QString("MonthlyExpense/%1/amount").arg(i), -1.0f).toFloat();

        qDebug() << "        -> New MonthlyExpense [" << newEntry.dbgLabel << ":" << newEntry.pattern.pattern() << ":" << newEntry.amount << "]";
        m_SumOfExpectedMonthlyExpenses += newEntry.amount;

        m_MonthlyExpense.append(newEntry);
    }
    qDebug() << "AccountAnalyzer : end loading configuration...";
}

void AccountAnalyzer::ParseBankAmount(const QString & inputStr, balances & out_balances, float & out_spendableAmount)
{
/*
id;url;label;currency;bank_name;type;owner_type;balance;coming;iban;ownership;paydate;paymin;cardlimit;number;valuation_diff;valuation_diff_ratio;parent;opening_date
AXNFI807803518@banquepopulaire;None;Credit Renouvelable Fac... M Etienne Raby;EUR;Not loaded;4;Not loaded;0.00;Not loaded;Not available;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded
CPT10719166171@banquepopulaire;None;Compte Cheque Mr Ou Mme Raby Etienne;EUR;Not loaded;1;Not loaded;1785.44;Not loaded;FR7613807005471071916617188;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded
CPT31107502191@banquepopulaire;None;Compte Epargne Logement M Etienne Raby;EUR;Not loaded;2;Not loaded;11026.73;Not loaded;Not available;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded
CPT31180442638@banquepopulaire;None;Plan Epargne Logement M Etienne Raby;EUR;Not loaded;2;Not loaded;2898.51;Not loaded;Not available;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded
CPT51183453033@banquepopulaire;None;Livret Developpement Du... M Etienne Raby;EUR;Not loaded;2;Not loaded;7579.07;Not loaded;Not available;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded
GPR02679541@banquepopulaire;None;Pret Immobilier Non Eli... Mr Ou Mme Raby Etienne;EUR;Not loaded;4;Not loaded;-18104.70;Not loaded;Not available;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded
GPR02679542@banquepopulaire;None;Pret Immobilier Non Eli... Mr Ou Mme Raby Etienne;EUR;Not loaded;4;Not loaded;-55702.23;Not loaded;Not available;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded
GPR02679543@banquepopulaire;None;Pret Immobilier Non Eli... Mr Ou Mme Raby Etienne;EUR;Not loaded;4;Not loaded;-43404.92;Not loaded;Not available;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded;Not loaded
*/
    qDebug() << "AccountAnalyzer : Start parsing bank amounts to get all balances...";
    QStringList list = inputStr.split('\n', QString::SkipEmptyParts);

    for(int i = 0; i < list.size(); ++i)
    {
        QString currentLine = list.at(i);
        if(currentLine.trimmed().isEmpty()) continue;
        if(currentLine.contains(QRegularExpression("id;url;label;.*"))) continue;

        //Remove credits
        if(currentLine.contains(QRegularExpression(".*Immobilier.*"))) continue;
        if(currentLine.contains(QRegularExpression(".*Credit.*"))) continue;

        QStringList splittedLine = currentLine.split(";");
        if(splittedLine.size() >= 8)
        {
            qDebug() << "    -> Balance:" << splittedLine.at(INDEX_LABEL) << "=" << splittedLine.at(INDEX_BALANCE) << "Eur";
            out_balances.append(QPair<QString, QPair<float, float>>(splittedLine.at(INDEX_LABEL), QPair<float, float>(splittedLine.at(INDEX_BALANCE).toFloat(), 0.0f)));

            if((splittedLine.at(INDEX_LABEL)).contains("Compte Cheque Mr Ou Mme Raby Etienne"))
            {
                float amount = splittedLine.at(INDEX_BALANCE).toFloat();
                out_spendableAmount = applyMonthlyExpenses(amount);
            }
        }
    }
    qDebug() << "AccountAnalyzer : End of parsing bank amounts.";
}

void AccountAnalyzer::ParseBankHistory(const QString & inputSample, expense_categorized & out_categorized, expense_categorized_detail & out_categorized_detail, expense_uncategorized & out_uncategorized)
{
/*

 Date         Category     Label                                                  Amount
------------+------------+---------------------------------------------------+-----------
 2019-09-17   Order        SEPA DIRECTION GENE 220513013124958172 111 NNFR46Z    -104.00
 2019-09-17   Order        SEPA DIRECTION GENE 220513013124958172 111 NNFR46Z     -89.00
 2019-09-16   Transfer     M RABY ETIENNE                                        -200.00
 2019-09-16   Transfer     M RABY ETIENNE                                         744.00
 2019-09-16   Card         AU CHAR A BANCS22PLELO                                -571.40
 2019-09-16   Card         VENTE-PRIVEE.CO93ST DENIS                              -96.93
 2019-09-16   Card         INTERMARCHE 22SAINT-AGATHON                            -73.65
 2019-09-16   Card         LA CHAROLAISE 22PLOUMAGOAR                             -62.96
 2019-09-16   Card         LES GALLO PAINS22PLOUMAGOAR                             -6.60
 2019-09-16   Card         INTERMARCHE NFC22SAINT-AGATHON                          -5.45
 2019-09-16   Card         LES GALLO PAINS22PLOUMAGOAR                             -4.20
 2019-09-16   Order        SEPA French Data Ne 371 - fdn aout 2019 FDN-001-37     -29.00
 2019-09-16   Order        SEPA OVH SAS Payment order 113007350 RE12759-OVH-F      -1.19
 */
    qDebug() << "AccountAnalyzer : Start parsing bank history...";

    QStringList list = inputSample.split('\n', QString::SkipEmptyParts);

    for(int i = 0; i < list.size(); ++i)
    {
        QString currentLine = list.at(i);
        if(currentLine.trimmed().isEmpty()) continue;
        if(currentLine.contains(QRegularExpression("Date.*Category.*Label.*Amount"))) continue;
        if(currentLine.contains(QRegularExpression("\\-+\\+\\-+\\+\\-+\\+\\-+"))) continue;

        qDebug() << "    -> Processing:" << currentLine;
        QString dateStr = currentLine.left(11).trimmed();
        QDate date = QDate::fromString(dateStr, date_format);
        currentLine = currentLine.mid(11);

        currentLine = currentLine.mid(14);

        float amount = currentLine.right(11).trimmed().toFloat();
        currentLine.chop(11);

        QString label = currentLine.trimmed();

        bool entry_found = false;
        for(int regexIndex=0; (regexIndex<m_RegExp.size() && !entry_found ); ++regexIndex)
        {
            const QList<QRegularExpression> & refRegExList = m_RegExp.at(regexIndex).patternList;
            for(int listIndex=0; (listIndex<refRegExList.size() && !entry_found ); ++listIndex)
            {
                QRegularExpressionMatch match = refRegExList.at(listIndex).match(label);
                if(match.hasMatch())
                {
                    qDebug() << "        -> MATCH Category : " << m_RegExp.at(regexIndex).category << "(" << refRegExList.at(listIndex).pattern() << ")" << amount;
                    insertCategorized(m_RegExp.at(regexIndex), amount, out_categorized);
                    out_categorized_detail[m_RegExp.at(regexIndex).category].append(uncategorized_data(date, label, amount));
                    entry_found = true;
                }
            }
        }

        if(!entry_found)
        {
            out_uncategorized.append(uncategorized_data(date, label, amount));
        }

        QMutableListIterator<MonthlyExpense> it(m_MonthlyExpense);
        while (it.hasNext())
        {
            MonthlyExpense & current = it.next();
            QRegularExpressionMatch match = current.pattern.match(label);
            if(match.hasMatch())
            {
                qDebug() << "        -> MATCH MonthlyExpense! : " << current.dbgLabel << "(" << current.pattern.pattern() << ")";
                m_MonthlyAlreadySpent += current.amount;
                it.remove();
            }
        }
    }
    qDebug() << "AccountAnalyzer : End of parsing bank history.";
}

const QList<MonthlyExpense> & AccountAnalyzer::GetRemainingMonthlyExpense() const
{
    return m_MonthlyExpense;
}

void AccountAnalyzer::insertCategorized(const ReGexpEnty& entry, float amount, expense_categorized & categorized)
{
    //List summarize categorized
    if(categorized.contains(entry.category))
    {
        categorized[entry.category].current += amount;
    }
    else
    {
        categorized[entry.category].current = amount;
        categorized[entry.category].max = entry.maxBeforeWarning;
    }
}

float AccountAnalyzer::applyMonthlyExpenses(const float & amount)
{
    return amount - (m_SumOfExpectedMonthlyExpenses - m_MonthlyAlreadySpent);
}

