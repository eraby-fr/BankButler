#pragma once

#include <QList>
#include <QMultiMap>
#include <QString>
#include <array>

typedef QList<QPair<QString, float>> balances;
typedef QMultiMap<QString, QPair<QString, float>> savings;

const QString list_separator = "||";
const QString date_format = "yyyy-MM-dd";
