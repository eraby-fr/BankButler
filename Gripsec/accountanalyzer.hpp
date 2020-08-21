#pragma once

#include <QSettings>
#include <QRegularExpression>
#include <Gripsec/expense.hpp>
#include <Gripsec/constants.hpp>

typedef struct ReGexpEnty
{
    QString category;
    QList<QRegularExpression> patternList;
    float maxBeforeWarning;

    ReGexpEnty()
    {
        category = QString();
        patternList = QList<QRegularExpression>();
        maxBeforeWarning = -1;
    }

    ReGexpEnty(const QString & cat, const QList<QRegularExpression> & regexp, float threshold = -1)
    {
        category = cat;
        patternList = regexp;
        maxBeforeWarning = threshold;
    }
} ReGexpEnty;

class AccountAnalyzer
{
public:
    AccountAnalyzer(const QSettings &config);

    void ParseBankAmount(const QString & inputStr, balances & out_balances, float &out_spendableAmount);
    void ParseBankHistory(const QString & inputSample, expense_categorized & out_categorized, expense_categorized_detail & out_categorized_detail, expense_uncategorized & out_uncategorized);
    const QList<MonthlyExpense> & GetRemainingMonthlyExpense() const;

#ifdef UNIT_TEST
    struct AcccountAnalyzerBackDoor;
    friend AcccountAnalyzerBackDoor;
#endif

private:
    void insertCategorized(const ReGexpEnty& entry, float amount, expense_categorized & categorized);
    float applyMonthlyExpenses(const float & amount);

    QList<ReGexpEnty> m_RegExp;
    QList<MonthlyExpense> m_MonthlyExpense;
    float m_MonthlyAlreadySpent;
    float m_SumOfExpectedMonthlyExpenses;
};
