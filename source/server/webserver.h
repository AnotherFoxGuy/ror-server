#pragma once


#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WServer.h>
#include <Wt/WTable.h>
#include <Wt/WTableCell.h>
#include <Wt/WLineEdit.h>

class WebServer : public Wt::WApplication
{
public:
    WebServer (const Wt::WEnvironment& env);
    ~WebServer ();

private:
    Wt::WLineEdit *nameEdit_;
    Wt::WText *greeting_;
    void greet ();
};

#ifdef WITH_WEBSERVER


#endif //WITH_WEBSERVER
