#pragma once

#include <QString>
#include <QHash>
#include <QList>
#include <QDate>
#include <QPair>
#include <QRegularExpression>

typedef struct amount
{
    float current;
    float max;

    amount()
    {
        current = 0;
        max = 0;
    }

    amount(float value, float threshold = 0)
    {
        current = value;
        max = threshold;
    }

} amount;

typedef struct uncategorized_data
{
    QDate date;
    QString label;
    float amount;

    uncategorized_data()
    {
        date = QDate::currentDate();
        label = "";
        amount = 0.0f;
    }

    uncategorized_data(const QDate & date_in, const QString & str, float value)
    {
        date = QDate(date_in);
        label = str;
        amount = value;
    }

} uncategorized_data;

typedef QHash<QString, amount> expense_categorized;

typedef QHash<QString, QList<uncategorized_data>> expense_categorized_detail;

typedef QList<uncategorized_data> expense_uncategorized;

typedef struct MonthlyExpense
{
    QString dbgLabel;
    QRegularExpression pattern;
    float amount;
} MonthlyExpense;

