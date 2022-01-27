#include "gripsec.hpp"

#include <Gripsec/bankwrapper.hpp>
#include <Gripsec/accountanalyzer.hpp>
#include <Gripsec/mailgenerator.hpp>
#include <Gripsec/savingmanager.hpp>
#include <Gripsec/liabilitymanager.hpp>
#include <Gripsec/constants.hpp>

#include <QDebug>
#include <QFile>

GripSec::GripSec() :
    m_conf("StunButler", "GripSec")
{

}

bool GripSec::LoadConfig()
{
    QString filename = m_conf.fileName();
    QFile file(filename);

    if(!file.exists())
    {
        qDebug() << filename << "doesn't exist. Gripsec can't load setting and start...";
        return false;
    }

    qDebug() << "Loading settings from " << filename;
    return true;
}

void GripSec::GenerateInitialConfig()
{
    m_conf.setValue("NumOfRegExp", 2);

    m_conf.beginGroup("RegExp");
    m_conf.beginGroup("0");
    m_conf.setValue("label", "00_Consomation");
    QStringList consoList;
    consoList << ".*INTERMARCHE.*" << ".*PICARD.*";
    m_conf.setValue("regexp", consoList.join(list_separator));
    m_conf.setValue("threshold", 600.0f);
    m_conf.endGroup();
    m_conf.beginGroup("1");
    m_conf.setValue("label", "50_Vetements");
    m_conf.setValue("regexp", ".*ZALANDO.*");
    m_conf.setValue("threshold", -1.0f);
    m_conf.endGroup();
    m_conf.endGroup();

    m_conf.setValue("Account", "myaccount@banquepopulaire");
    m_conf.setValue("BoobankPath", "~/.bin/weboob/bin/boobank");
    m_conf.setValue("BoobankMaxEntries", 4000);

    m_conf.setValue("NumOfMonthlyExpense", 1);

    m_conf.beginGroup("MonthlyExpense");
    m_conf.beginGroup("0");
    m_conf.setValue("label", "Soupape");
    m_conf.setValue("pattern", "IDontWantAnyMatch");
    m_conf.setValue("amount", 500.0f);
    m_conf.endGroup();
    m_conf.endGroup();


    m_conf.setValue("Mail", "foo@bar");
    m_conf.setValue("Mail_CC", "");

    qDebug() << "Gripsec wrote a template to help you at : " << m_conf.fileName();
}

int GripSec::AnalyseAccount(bool sendToCc, bool stubWoob)
{
    enum{NO_ERROR = 0, ERROR = 1};

    QTime startTS = QTime::currentTime();

    BankWrapper      wrapperProcess(m_conf);
    AccountAnalyzer  analyzerProcess(m_conf);
    MailGenerator    senderProcess(m_conf);
    SavingManager    savingsProcess(m_conf);
    LiabilityManager debtProcess(m_conf);

    int              numOfRemainingRetry = 3;
    QString history;
    QString amount;

    bool wrapperSucces = false;

    if (stubWoob)
    {
        m_conf.setValue("Account", "myMainAccount@mybank");
    }

    while ((!wrapperSucces) && (numOfRemainingRetry > 0))
    {
        wrapperSucces = wrapperProcess.RequestHistory(history, QDate::currentDate(), stubWoob);
        if(!wrapperSucces) numOfRemainingRetry--;
    }

    if(wrapperSucces)
    {
        expense_categorized categorized;
        expense_categorized_detail categorizedDetails;
        expense_uncategorized uncategorized;
        analyzerProcess.ParseBankHistory(history, categorized, categorizedDetails, uncategorized);

        wrapperSucces = false;
        while ((!wrapperSucces) && (numOfRemainingRetry > 0))
        {
            wrapperSucces = wrapperProcess.RequestAmount(amount, stubWoob);
            if(!wrapperSucces) numOfRemainingRetry--;
        }

        if(wrapperSucces)
        {
            balances allBalances;
            float spendableAmount = 0.0f;
            analyzerProcess.ParseBankAmount(amount, allBalances, spendableAmount);
            const QList<MonthlyExpense> & remainingMonthlyExpense = analyzerProcess.GetRemainingMonthlyExpense();

            savingsProcess.ParseFileSystem();
            savings savingsToPrint = savingsProcess.GetSavings();

            savingsProcess.UpdateBalanceWithSaving(allBalances, savingsToPrint);

            debtProcess.loadLiabilitiesFromFileSystem();
            debtProcess.computeHistory(history);
            const QList<Liability> & liabilities = debtProcess.getLiabilities();

            senderProcess.ProcessBalances(allBalances, spendableAmount);
            senderProcess.ProcessExpenses(categorized, uncategorized);
            senderProcess.ProcessSaving(savingsToPrint, allBalances);
            senderProcess.ProcessLiabilities(liabilities);
            senderProcess.ProcessRemainingMonthlyExpense(remainingMonthlyExpense);
            senderProcess.ProcessExpensesDetail(categorizedDetails);//expense_categorized_detail categorizedDetails;

            senderProcess.SendMail(startTS, sendToCc);
            return NO_ERROR;
        }

    }

    senderProcess.SendMail(startTS, amount, history);
    return ERROR;





}
