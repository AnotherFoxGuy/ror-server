/*
This file is part of "Rigs of Rods Server" (Relay mode)

Copyright 2007   Pierre-Michel Ricordel
Copyright 2014+  Rigs of Rods Community

"Rigs of Rods Server" is free software: you can redistribute it
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3
of the License, or (at your option) any later version.

"Rigs of Rods Server" is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with "Rigs of Rods Server". 
If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef WITH_WEBSERVER

#include "webserver.h"

#include "sequencer.h"
#include "master-server.h"
#include "messaging.h"
#include "userauth.h"
#include "logger.h"
#include "config.h"

#include "utils.h"


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

#endif //WITH_WEBSERVER


WebServer::WebServer (const Wt::WEnvironment& env) : Wt::WApplication (env)
{

    auto table = Wt::cpp14::make_unique<Wt::WTable> ();
    table->setHeaderCount (1);
    table->setWidth (Wt::WLength ("100%"));

    table->elementAt (0, 0)->addWidget (Wt::cpp14::make_unique<Wt::WText> ("#"));
    table->elementAt (0, 1)->addWidget (Wt::cpp14::make_unique<Wt::WText> ("First Name"));
    table->elementAt (0, 2)->addWidget (Wt::cpp14::make_unique<Wt::WText> ("Last Name"));
    table->elementAt (0, 3)->addWidget (Wt::cpp14::make_unique<Wt::WText> ("Pay"));

    for (unsigned i = 0; i < 3; ++i) {
        Employee& employee = employees[i];
        int row = i + 1;

        table->elementAt (row, 0)
            ->addWidget (Wt::cpp14::make_unique<Wt::WText> (Wt::WString ("{1}")
                .arg (row)));
        table->elementAt (row, 1)
            ->addWidget (Wt::cpp14::make_unique<Wt::WText> (employee.firstName));
        table->elementAt (row, 2)
            ->addWidget (Wt::cpp14::make_unique<Wt::WText> (employee.lastName));
        table->elementAt (row, 3)
            ->addWidget (Wt::cpp14::make_unique<Wt::WLineEdit> (Wt::WString ("{1}")
                .arg (employee.pay)));
    }

}

WebServer::~WebServer ()
{
}