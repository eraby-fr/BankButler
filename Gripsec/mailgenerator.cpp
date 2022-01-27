#include "mailgenerator.hpp"
#include <QDebug>
#include <QDate>
#include <QProcess>
#include <QStringList>

static bool compare(const uncategorized_data& first, const uncategorized_data& second)
{
    return first.date < second.date;
}

MailGenerator::MailGenerator(const QSettings &config) :
    m_categorizedTable(),
    m_balances(),
    m_categorizedDetailTable(),
    m_spendableAmount(0.0f)
{
    m_targetMail = config.value("Mail").toString();
    m_ccMail = config.value("Mail_CC").toString();
}

void MailGenerator::SendMail(const QTime & timestampStart, const QString & amount, const QString & history)
{
    qDebug() << "MailGenerator : Start sending error mail...";

    QString mailContent = QString("%1\n%2\n%3\n%4\n%5\n%6\n%7\n%8\n%9")
            .arg(generateCSS())
            .arg(generateWelcome())
            .arg("        <p>Malheureusement, l'analyse n'a pas pu être réalisée...</p>\n")
            .arg("        <p>Voici la sortie de boobank list : <br>\n")
            .arg(amount)
            .arg("        </p><p>Voici la sortie de boobank history : <br>\n")
            .arg(history)
            .arg("        </p>")
            .arg(generateFooter(timestampStart));

    QString subject = QString("[Gripsec] Echec de l'analyse du %1").arg(QDate::currentDate().toString("dd.MM.yyyy"));
    this->Send(subject, mailContent);
    qDebug() << "MailGenerator : End sending error mail...";
}

void MailGenerator::SendMail(const QTime & timestampStart, bool sendToCc)
{
    qDebug() << "MailGenerator : Start sending mail...";
    QString mailContent = QString("%1\n%2\n%3\n%4\n%5\n%6\n%7\n%8\n%9")
            .arg(generateCSS())
            .arg(generateWelcome())
            .arg(generateRemaining())
            .arg(generateCategoriesPart())
            .arg(generateSaving())
            .arg(generateLiability())
            .arg(generateComingMonthlyExpense())
            .arg(generateFooter(timestampStart))
            .arg(m_categorizedDetailTable);

    QString subject = QString("[Gripsec %2€] Analyse du %1").arg(QDate::currentDate().toString("dd.MM.yyyy")).arg(double(m_spendableAmount));
    this->Send(subject, mailContent, sendToCc);
    qDebug() << "MailGenerator : End sending mail...";
}

void MailGenerator::Send(const QString & subject, const QString & mailContent, bool sendToCc)
{
    QProcess mail;

    QString binName = "mail";
    QStringList args;
    args << "-a" << "MIME-Version: 1.0" << "-a" << "Content-Type: text/html; charset=utf8"  << "-s" << subject;

    if(!m_ccMail.isEmpty() && sendToCc)
    {
        args << "-c" << m_ccMail;
    }

    args << m_targetMail;

    qDebug() << "    -> Start external process :" << binName << args.join(" ");

    mail.start(binName, args);

    if (!mail.waitForStarted())
    {
        qFatal("mail FAILLED to start so we crash the program");
    }

    mail.write(mailContent.toUtf8() );
    mail.closeWriteChannel();

    qDebug() << "    -> Mail written! Closing channel...";

    if (!mail.waitForFinished())
    {
        qFatal("mail FAILLED to finished so we crash the program");
    }
    qDebug() << "    -> Channel closed...";

/*    QByteArray baErr = mail.readAllStandardError();
    qDebug() << "ERR : " << QString(baErr);

    QByteArray ba = mail.readAllStandardOutput();
    qDebug() << ba;*/
}

QString MailGenerator::generateCSS()
{
    return QString(
    "<style type=\"text/css\">\n" \
    "  h1 {color:#b48608;font-size:36px;font-weight:400;font-style:italic;line-height:44px;margin: 0 0 12px;text-align:left;}\n" \
    "  h2 {color:#b48608;font-size:25px;font-weight:400;line-height:24px;text-align:left;}\n" \
    "  .pAmount {font-size:25px}\n" \
    "  .pAmountAlert {color:red;font-size:25px}\n" \
    "  strong {color: red;}\n" \
    "  .gripsecTable {width: 100%;table-layout: fixed;border-collapse:collapse;border:1px solid;}\n" \
    "  .gripsecTable td{padding:5px 5px;border-style:solid;border-width:0px;overflow:hidden;word-break:normal;border-top-width:1px;border-bottom-width:1px;border-color:#aaa;color:#333;background-color:#fff;}\n" \
    "  .gripsecTable th{padding:5px 5px;border-style:solid;border-width:0px;overflow:hidden;word-break:normal;border-top-width:1px;border-bottom-width:1px;border-color:#aaa;color:#fff;background-color:#f38630;}\n" \
    "  .gripsecTable .white{text-align:left;vertical-align:top;}\n" \
    "  .gripsecTable .whiteCenter{text-align:center;vertical-align:top;}\n" \
    "  .gripsecTable .colored{text-align:left;vertical-align:topb;ackground-color:#FCFBE3;}\n" \
    "  .gripsecTable .coloredCenter{text-align:center;vertical-align:top;background-color:#FCFBE3;}\n" \
    "  .gripsecTable .alert{text-align:left;vertical-align:top;background-color:#fe0000;color:#ffffff;font-weight:bold;}\n" \
    "  .gripsecTable .alertCenter{text-align:center;vertical-align:top;background-color:#fe0000;color:#ffffff;font-weight:bold;}\n" \
    "  .gripsecUnknowTable {width: 100%;table-layout: fixed;border-collapse:collapse;border:1px solid;}\n" \
    "  .gripsecUnknowTable td{padding:0px 5px;border-style:solid;border-width:0px;overflow:hidden;word-break:normal;border-top-width:1px;border-bottom-width:1px;border-color:#aaa;color:#333;background-color:#fff;}\n" \
    "  .gripsecUnknowTable th{padding:5px 5px;border-style:solid;border-width:0px;overflow:hidden;word-break:normal;border-top-width:1px;border-bottom-width:1px;border-color:#aaa;color:#fff;background-color:#f38630;}\n" \
    "  .gripsecUnknowTable .whiteCenter{text-align:center;vertical-align:top;}\n" \
    "  .gripsecUnknowTable .coloredCenter{text-align:center;vertical-align:top;background-color:#FCFBE3;}\n" \
    "  .tdRight{text-align:right;vertical-align:center;padding-right:8px;}\n" \
    "  .tdRightRed{color:red;text-align:right;vertical-align:center;padding-right:8px;}\n" \
    "  table{margin: 0px 0px 0px 20px;}\n" \
    "  tr:nth-child(even) {background-color: #E38630;}\n" \
    "</style>\n");
}

void MailGenerator::ProcessExpenses(const expense_categorized & expenseCategorizedHash, expense_uncategorized expenseUncategorizedList)
{
    if((expenseCategorizedHash.size() == 0) && (expenseUncategorizedList.size() == 0))
    {
        m_categorizedTable = "        <p>Incroyable, aucune dépense ce mois ci!</p>\n";
        return;
    }

    m_categorizedTable = QString();

    if(expenseCategorizedHash.size() == 0)
    {
        m_categorizedTable.append("        <p>Malheureusement, aucune dépense n'a pu être classée ce mois ci...</p>\n");
    }
    else
    {
        m_categorizedTable.append(""\
                                  "        <h1>Dépenses</h1>\n"\
                                  "        <p>\n"\
                                  "        <table class=\"gripsecTable\">\n"\
                                  "          <colgroup>\n"\
                                  "            <col>\n"\
                                  "            <col style=\"width:22%\">\n"\
                                  "            <col style=\"width:22%\">\n"\
                                  "          </colgroup>\n"\
                                  "          <tr><th>Catégorie</th><th>Montant</th><th>Seuil d'alerte</th></tr>\n");

        QStringList keys = expenseCategorizedHash.keys();
        keys.sort();
        for (int i = 0; i < keys.size(); ++i)
        {
            QString key = keys.at(i);

            m_categorizedTable.append(QString("          <tr><td class=\"white\"><a href=\"#detail_%1\">%1</a></td><td class=\"coloredCenter\">%2</td><td class=\"whiteCenter\">%3</td></tr>\n")
                                      .arg(key)
                                      .arg(QString::number( static_cast<double>(expenseCategorizedHash[key].current), 'f', 2))
                                      .arg((expenseCategorizedHash[key].max < 0.0f) ? QString::number( static_cast<double>(expenseCategorizedHash[key].max), 'f', 2) : ""));
        }

        m_categorizedTable.append(QString("%1\n%2\n")
                                  .arg("        </table>")
                                  .arg("        </p>"));

    }

    if(expenseUncategorizedList.size() == 0)
    {
        m_categorizedTable.append("        <p>Bonne nouvelle, il n'y a pas de dépenses non classées</p>\n");
    }
    else
    {
        m_categorizedTable.append(""\
                                  "        <p>Voici les dépenses que je n'ai pas pu classer :\n"\
                                  "        <table class=\"gripsecUnknowTable\">\n"\
                                  "          <colgroup>\n"\
                                  "            <col style=\"width:22%\">\n"\
                                  "            <col style=\"width:22%\">\n"\
                                  "            <col>\n"\
                                  "          </colgroup>\n"\
                                  "          <tr><th>Date</th><th>Montant</th><th>Libellé</th></tr>\n");

        std::sort(expenseUncategorizedList.begin(), expenseUncategorizedList.end(), compare);

        for(int i=0; i<expenseUncategorizedList.size(); ++i)
        {
            const uncategorized_data & current = expenseUncategorizedList.at(i);
            m_categorizedTable.append(QString("          <tr><td class=\"white\">%1</td><td class=\"coloredCenter\">%2</td><td class=\"whiteCenter\">%3</td></tr>\n")
                                      .arg(current.date.toString("dd/MM/yyyy"))
                                      .arg(QString::number( static_cast<double>(current.amount), 'f', 2))
                                      .arg(current.label));
        }

        m_categorizedTable.append(QString("%1\n%2\n")
                                  .arg("        </table>")
                                  .arg("        </p>"));

    }
}

void MailGenerator::ProcessBalances(const balances & fullBalances, const float & spendableAmount)
{
    m_spendableAmount = spendableAmount;

    m_balances = QString("        <h1>Trésorerie</h1>\n");
    if(fullBalances.size() == 0)
    {
        return;
    }

    m_balances.append("        <table>\n"\
                         "            <colgroup>\n"\
                         "                <col style=\"width:400px\">\n"\
                         "                <col>\n"\
                         "                <col>\n"\
                         "                <col>\n"\
                         "            </colgroup>\n"\
                         "            <tr><th>  </th><th>Montant réel</th><th>Affecté</th><th>Epargne pure</th></tr>\n");

    for(int i=0; i<fullBalances.size(); ++i)
    {
        double full_amount = double(fullBalances.at(i).second.first);
        double set_ammount = double(fullBalances.at(i).second.second);
        double free_amount = full_amount - set_ammount;

        m_balances.append(QString("            <tr><td class=\"tdRight\">%1</td><td class=\"tdRight\">%2 Eur</td><td class=\"tdRight\">%3 Eur</td><td class=\"tdRight\">%4 Eur</td></tr>\n")
                            .arg(fullBalances.at(i).first)
                            .arg(full_amount)
                            .arg(set_ammount)
                            .arg(free_amount)
                            );
    }
    m_balances.append("        </table>\n        </p>\n");
}

void MailGenerator::ProcessSaving(const savings & allSavings, const balances &allBalances)
{
    (void)allBalances;
    m_savings = QString("        <h1>Sommes affectées</h1>\n");

    if(allSavings.size() == 0)
    {
        m_savings.append("        <p>Aucune somme affectée sur un plan d'épargne n'est disponible actuellement (et ça craint), désolé...</p>\n");
        return;
    }
    m_savings.append("        <p>Voici la liste des sommes déjà affectées sur les plans d'épargne:\n");

    QList<QString> accountNameLst = allSavings.uniqueKeys();
    for (int i = 0; i < accountNameLst.size(); ++i)
    {
        QString accountName = accountNameLst.at(i);
        m_savings.append(QString("        <h2>%1</h2>\n").arg(accountName));
        m_savings.append("        <table>\n"\
                         "            <colgroup>\n"\
                         "                <col style=\"width:200px\">\n"\
                         "                <col>\n"\
                         "            </colgroup>\n");

        QList<QPair<QString,float>> values = allSavings.values(accountName);
        for (int k = (values.size()-1); k >= 0 ; --k)
        {
            double value = static_cast<double>(values.at(k).second);
            m_savings.append(QString("            <tr><td class=\"tdRight\">%1</td><td class=\"%2\">%3Eur</td></tr>\n")
                             .arg(values.at(k).first.mid(3))
                             .arg((value<0.0)?"tdRightRed":"tdRight")
                             .arg(QString::number(value , 'f', 2)));
        }
        m_savings.append("        </table>\n");
    }
}

void MailGenerator::ProcessLiabilities(const QList<Liability> & liabilities)
{
    m_debts = QString("        <h1>Dettes</h1>\n");
    if(liabilities.size() == 0)
    {
        m_debts.append("        <p>Super, il n'y a aucune dette en cours!</p>\n");
        return;
    }

    for(int iList=0; iList<liabilities.size(); ++iList)
    {
        QString debtStr;
        const Liability & debt = liabilities.at(iList);

        if(debt.currentAmount <= 0.0f)
        {
            debtStr = QString("        <p>Dette %1 [%2] de %3Eur remboursée :-) !</p>\n")
                .arg(debt.name)
                .arg(debt.description)
                .arg(QString::number(static_cast<double>(debt.initialAmount), 'f', 2))
                .arg(QString::number(static_cast<double>(debt.currentAmount), 'f', 2));
        }
        else
        {
            if(debt.payementDoneForThisMonth)
            {
                debtStr = QString("        <p>Mensualité reçue pour %1 [%2] Initial=%3Eur<br>Il reste encore %4Eur</p>\n")
                        .arg(debt.name)
                        .arg(debt.description)
                        .arg(QString::number(static_cast<double>(debt.initialAmount), 'f', 2))
                        .arg(QString::number(static_cast<double>(debt.currentAmount), 'f', 2));
            }
            else
            {
                debtStr = QString("        <p><strong>En attente de la mensualité pour %1</strong> [%2] Initial=%3Eur<br>Il reste encore %4Eur !!!</p>\n")
                        .arg(debt.name)
                        .arg(debt.description)
                        .arg(QString::number(static_cast<double>(debt.initialAmount), 'f', 2))
                        .arg(QString::number(static_cast<double>(debt.currentAmount), 'f', 2));
            }
        }
        m_debts.append(debtStr);
    }
}

void MailGenerator::ProcessRemainingMonthlyExpense(const QList<MonthlyExpense> & remainingExpenses)
{
    m_remaingMonthlyExpenses = QString("        <h1>Dépenses mensuelles à venir</h1>\n");
    if(remainingExpenses.size() == 0)
    {
        m_remaingMonthlyExpenses.append("        <p>A priori, aucune! Bonne nouvelle.</p>");
        return;
    }

    m_remaingMonthlyExpenses.append("        <p>\n            <ul>\n");
    for(int i=0; i<remainingExpenses.size(); ++i)
    {
        m_remaingMonthlyExpenses.append(QString("              <li>%1 : %2Eur<br></li>\n")
                              .arg(remainingExpenses.at(i).dbgLabel)
                              .arg(double(remainingExpenses.at(i).amount))
                              );
    }
    m_remaingMonthlyExpenses.append("            </ul>\n        </p>\n");
}

void MailGenerator::ProcessExpensesDetail(expense_categorized_detail categorizedDetails)
{
    QStringList keys = categorizedDetails.keys();
    keys.sort();
    for (int i = 0; i < keys.size(); ++i)
    {
        QString key = keys.at(i);
        m_categorizedDetailTable.append("<br> <br> <br> <br> <br> <br> <br> <br> <br> <br>\n");
        m_categorizedDetailTable.append(QString("%1%2%3")
                    .arg("        <a name=\"detail_")
                    .arg(key)
                    .arg("\"></a>\n"));

        m_categorizedDetailTable.append(QString("%1%2%3")
                    .arg("        <h2>Détail ")
                    .arg(key)
                    .arg("</h2>\n"));

        m_categorizedDetailTable.append(""\
                                  "        <table class=\"gripsecUnknowTable\">\n"\
                                  "          <colgroup>\n"\
                                  "            <col style=\"width:22%\">\n"\
                                  "            <col style=\"width:22%\">\n"\
                                  "            <col>\n"\
                                  "          </colgroup>\n"\
                                  "          <tr><th>Date</th><th>Montant</th><th>Libellé</th></tr>\n");

        QList<uncategorized_data> category = categorizedDetails[key];
        std::sort(category.begin(), category.end(), compare);

        for(int k = 0; k < category.size(); ++k)
        {
            const uncategorized_data & current = category.at(k);
            m_categorizedDetailTable.append(QString("          <tr><td class=\"white\">%1</td><td class=\"coloredCenter\">%2</td><td class=\"whiteCenter\">%3</td></tr>\n")
                                      .arg(current.date.toString("dd/MM/yyyy"))
                                      .arg(QString::number( static_cast<double>(current.amount), 'f', 2))
                                      .arg(current.label));
        }

        m_categorizedDetailTable.append(QString("%1\n%2\n")
                                  .arg("        </table>")
                                  .arg("        </p>"));
    }
}

QString MailGenerator::generateWelcome()
{
    return QString("%1\n%2\n%3\n%4\n%5\n%6")
            .arg("<html>")
            .arg("    <head>")
            .arg("        <meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">")
            .arg("    </head>")
            .arg("    <body>")
            .arg("        <p>Bonjour ! Voici l'analyse des comptes du mois!</p>");
}

QString MailGenerator::generateCategoriesPart()
{
    return m_categorizedTable;
}

QString MailGenerator::generateRemaining()
{
    return m_balances;
}

QString MailGenerator::generateSaving()
{
    return m_savings;
}

QString MailGenerator::generateLiability()
{
    return m_debts;
}

QString MailGenerator::generateComingMonthlyExpense()
{
    return m_remaingMonthlyExpenses;
}

QString MailGenerator::generateFooter(const QTime & timestampStart)
{
    return QString("%1\n%2\n%3%4%5\n%6\n%7")
            .arg("        <p><br>Cordialement,</p>")
            .arg("        <p>Gripsec</p>")
            .arg("        <p>Mail généré en ")
            .arg(timestampStart.msecsTo(QTime::currentTime()))
            .arg(" ms par Gripsec V1.6.</p>")
            .arg("    </body>")
            .arg("</html>");
}
