#pragma once

#include <QObject>
#include <QSettings>

class GripSec
{
public:
    GripSec();
    bool LoadConfig();
    void GenerateInitialConfig();
    int AnalyseAccount(bool sendToCc=false);

private:
    QSettings m_conf;
};
