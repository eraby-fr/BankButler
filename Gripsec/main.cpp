#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDateTime>
#include <iostream>
#include "gripsec.hpp"

static bool verbose = false;

void qTraceHandler(QtMsgType type, const QMessageLogContext &context,  const QString &msg)
{
    (void)context;

    //Get the current date
    QString dateStr = QDateTime::currentDateTime().toString(QLatin1String("[dd.MM.yy hh.mm.ss.zzz] " ));
    switch (type)
    {
        case QtDebugMsg:
            if(verbose) {std::cout << dateStr.toStdString() << ": Debug   : " << msg.toStdString() << std::endl;}
            break;
        case QtWarningMsg:
            std::cout << dateStr.toStdString() << ": Warning   : " << msg.toStdString() << std::endl;
            break;
        default: //QtCriticalMsg & QtFatalMsg:
            std::cout << dateStr.toStdString() << ": Fatal   : " << msg.toStdString() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(qTraceHandler);

    //Parse arguments
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("Gripsec");

    QCommandLineParser parser;
    parser.setApplicationDescription("A wonderfull Gringotts goblin");
    parser.addHelpOption();

    QCommandLineOption ccOption("cc", "Send a mail copy to CC");
    parser.addOption(ccOption);
    QCommandLineOption verboseOption("verbose", "Show debug traces");
    parser.addOption(verboseOption);
    QCommandLineOption stubOption("stub", "Stub Woob and inject hardcoded data");
    parser.addOption(stubOption);
    parser.process(app);

    bool sendToCC = parser.isSet(ccOption);
    verbose  = parser.isSet(verboseOption);
    bool stubWoob = parser.isSet(stubOption);


    //Start process
    GripSec mainObject;
    if(!mainObject.LoadConfig())
    {
        mainObject.GenerateInitialConfig();
        return 0;
    }

    return mainObject.AnalyseAccount(sendToCC, stubWoob);
}
