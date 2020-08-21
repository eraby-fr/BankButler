#pragma once

#include <QString>
#include <QRegularExpression>

typedef struct Liability
{
    QString name;
    QString description;
    QRegularExpression pattern;
    float   initialAmount;
    float   currentAmount;
    bool    payementDoneForThisMonth;
    QString fileName;
} Liability;
