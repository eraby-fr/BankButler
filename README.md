# BankButler
C++ Tool dedicated to compute bank amount, analyze it and send summary by mail.

The current status is "dirt"...

I used this tool each day in my self-hosted server but there is some work to make it easy to use for everyone.

### Features
This tool take your current bank amounts and give you :
- The total of all accounts
- The "real" amount you can spent (removing monthly expected spending like tax)
- The total of each spending per category (food, transport, bank, cloths) based on your regular expression
- The total of each assigned value of your savings

![Mail client screenshot](Img/mail.png)


### Example usage
TODO

### Dependencies
- Parse [boobank](https://weboob.org/applications/boobank) output
- Use ```mail``` command line to send mail
- Use [Qt5](https://www.qt.io/) framework with C++ compliant compiler

### Backlog
#### Improvements/CleanUp
- Use Qt [internationalization](https://doc.qt.io/qt-5/internationalization.html) to send mail in defferant language than FR_fr
- Move current files saving from QSettings to XML or Json file
- Update/reintroduce unitary test (still in old svn repo)
- Add GPL commands `show w' and `show c' should show the appropriate parts of the General Public License.
- Add ```--version``` argument

#### Features
- Fill threshold column with dedicated CSS to alert when there is too much expense in a category
- For each bank account, provide total / assigned / remaining

### License
GPL v3
