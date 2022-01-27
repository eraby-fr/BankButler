#pragma once

#include <QSettings>
#include <QTime>

#include <Gripsec/expense.hpp>
#include <Gripsec/constants.hpp>
#include <Gripsec/debt.hpp>

class MailGenerator
{
public:
    MailGenerator(const QSettings & config);

    void ProcessExpenses(const expense_categorized & expenseCategorizedHash, expense_uncategorized expenseUncategorizedList);

    void ProcessBalances(const balances & allBalances, const float & spendableAmount);

    void ProcessSaving(const savings & allSavings, const balances &allBalances);

    void ProcessLiabilities(const QList<Liability> & liabilities);

    void ProcessRemainingMonthlyExpense(const QList<MonthlyExpense> & remainingExpenses);

    void ProcessExpensesDetail(expense_categorized_detail categorizedDetails);

    void SendMail(const QTime & timestampStart, bool sendToCc);

    void SendMail(const QTime & timestampStart, const QString & amount, const QString & history);

#ifdef UNIT_TEST
    struct MailGeneratorBackDoor;
    friend MailGeneratorBackDoor;
#endif

private:
    void Send(const QString & subject, const QString & mailContent, bool sendToCc = false);

    QString generateCSS();
    QString generateWelcome();
    QString generateFooter(const QTime & timestampStart);
    QString generateCategoriesPart();
    QString generateRemaining();
    QString generateSaving();
    QString generateLiability();
    QString generateComingMonthlyExpense();

    QString m_categorizedTable;
    QString m_balances;
    QString m_savings;
    QString m_debts;
    QString m_remaingMonthlyExpenses;
    QString m_categorizedDetailTable;
    QString m_targetMail;
    QString m_ccMail;
    float m_spendableAmount;
};
