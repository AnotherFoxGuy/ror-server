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

#include "config.h"
#include "sequencer.h"
#include "userauth.h"


#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include <iostream>

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(web_rc);

using namespace Poco;
using namespace Poco::Net;

static bool s_is_advertised;
static int s_trust_level;
static Sequencer *s_sequencer;
static HTTPServer *srv;

class StaticFileHandler: public HTTPRequestHandler
{
  void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
  {
    response.setChunkedTransferEncoding(true);
    response.setContentType("text/html");

    auto efs = cmrc::web_rc::get_filesystem();

    if (request.getURI() == "/") {
      auto index_file = efs.open("index.html");
      response.sendBuffer( index_file.begin(), index_file.size());

    } else if (efs.exists(request.getURI())) {
      auto req_file = efs.open(request.getURI());
      response.sendBuffer( req_file.begin(), req_file.size());
    } else {
      response.send()
          << "<html>"
          << "<head><title>Hello</title></head>"
          << "<body><h1>"
          << request.getURI()
          << " Hello from the POCO Web Server</h1></body>"
          << "</html>";
    }
  }
};

class RequestHandlerFactory : public HTTPRequestHandlerFactory {
  HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &) {
    return new StaticFileHandler;
  }
};

int StartWebserver(int port, Sequencer *sequencer, bool is_advertised,
                   int trust_level) {
  s_is_advertised = is_advertised;
  s_trust_level = trust_level;
  s_sequencer = sequencer;


  srv = new HTTPServer(new RequestHandlerFactory, port);
  srv->start();

  return 0;
}

int StopWebserver() {
  srv->stop();
  return 0;
}
#endif