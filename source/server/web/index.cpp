#include "index.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTMLForm.h"
#line 3 "E:\\projects\\ror-server\\source\\server\\web\\index.html"

#include "../config.h"
#include "rornet.h"


using namespace std::string_literals;


void IndexHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
	response.setChunkedTransferEncoding(true);
	response.setContentType("text/html"s);

	Poco::Net::HTMLForm form(request, request.stream());
	std::ostream& responseStream = response.send();
	// begin include header.html
	responseStream << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n";
	responseStream << "<html>\n";
	responseStream << "<head>\n";
	responseStream << "    <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n";
	responseStream << "\n";
	responseStream << "    <link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-1BmE4kWBq78iYhFldvKuhfTAU6auU8tT94WrHftjDbrCEXSU1oBoqyl2QvZ6jIW3\" crossorigin=\"anonymous\">\n";
	responseStream << "    <script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-ka7Sk0Gln4gmtz2MlQnikT1wXgYsOg+OMhuP+IlRH9sENBO0LRn5q+8nbTov4+1p\" crossorigin=\"anonymous\"></script>\n";
	responseStream << "\n";
	responseStream << "</head>\n";
	responseStream << "<body>\n";
	responseStream << "\n";
	responseStream << "<nav class=\"navbar navbar-expand-lg navbar-light bg-light\">\n";
	responseStream << "<div class=\"container-fluid\">\n";
	responseStream << "    <a class=\"navbar-brand\" href=\"#\">AAAA</a>\n";
	responseStream << "    <div class=\"collapse navbar-collapse\" id=\"navbarSupportedContent\">\n";
	responseStream << "        <ul class=\"navbar-nav me-auto mb-2 mb-lg-0\">\n";
	responseStream << "            <li class=\"nav-item\">\n";
	responseStream << "                <a class=\"nav-link\"href=\"/\">Home</a>\n";
	responseStream << "            </li>\n";
	responseStream << "            <li class=\"nav-item\">\n";
	responseStream << "                <a class=\"nav-link\" href=\"/playerlist\">Playerlist</a>\n";
	responseStream << "            </li>\n";
	responseStream << "            <li class=\"nav-item\">\n";
	responseStream << "                <a class=\"nav-link disabled\">Disabled</a>\n";
	responseStream << "            </li>\n";
	responseStream << "        </ul>\n";
	responseStream << "    </div>\n";
	responseStream << "</div>\n";
	responseStream << "</nav>\n";
	responseStream << "\n";
	responseStream << "<div class=\"container\">";
	// end include header.html
	responseStream << "\n";
	responseStream << "<table class=\"table table-striped\">\n";
	responseStream << "    <thead>\n";
	responseStream << "    <tr class=\"header\">\n";
	responseStream << "        <th>Server settings</th>\n";
	responseStream << "        <th></th>\n";
	responseStream << "    </tr>\n";
	responseStream << "    </thead>\n";
	responseStream << "    <tbody>\n";
	responseStream << "    <tr>\n";
	responseStream << "        <td>Server Name</td>\n";
	responseStream << "        <td>";
#line 18 "E:\\projects\\ror-server\\source\\server\\web\\index.html"
	responseStream << ( Config::getServerName() );
	responseStream << "</td>\n";
	responseStream << "    </tr>\n";
	responseStream << "    <tr>\n";
	responseStream << "        <td>Max Clients</td>\n";
	responseStream << "        <td>";
#line 22 "E:\\projects\\ror-server\\source\\server\\web\\index.html"
	responseStream << ( Config::getMaxClients() );
	responseStream << "</td>\n";
	responseStream << "    </tr>\n";
	responseStream << "    <tr>\n";
	responseStream << "        <td>Terrain Name</td>\n";
	responseStream << "        <td>";
#line 26 "E:\\projects\\ror-server\\source\\server\\web\\index.html"
	responseStream << ( Config::getTerrainName() );
	responseStream << "</td>\n";
	responseStream << "    </tr>\n";
	responseStream << "    <tr>\n";
	responseStream << "        <td>IP Address</td>\n";
	responseStream << "        <td>";
#line 30 "E:\\projects\\ror-server\\source\\server\\web\\index.html"
	responseStream << ( Config::getIPAddr() == "0.0.0.0" ? "0.0.0.0 (Any)" : Config::getIPAddr() );
	responseStream << "</td>\n";
	responseStream << "    </tr>\n";
	responseStream << "    <tr>\n";
	responseStream << "        <td>Password Protected</td>\n";
	responseStream << "        <td>";
#line 34 "E:\\projects\\ror-server\\source\\server\\web\\index.html"
	responseStream << ( Config::getPublicPassword().empty() ? "No" : "Yes" );
	responseStream << "</td>\n";
	responseStream << "    </tr>\n";
	responseStream << "    <tr>\n";
	responseStream << "        <td>Listening Port</td>\n";
	responseStream << "        <td>";
#line 38 "E:\\projects\\ror-server\\source\\server\\web\\index.html"
	responseStream << ( Config::getListenPort() );
	responseStream << "</td>\n";
	responseStream << "    </tr>\n";
	responseStream << "    <tr>\n";
	responseStream << "        <td>Protocol Version</td>\n";
	responseStream << "        <td>";
#line 42 "E:\\projects\\ror-server\\source\\server\\web\\index.html"
	responseStream << ( RORNET_VERSION );
	responseStream << "</td>\n";
	responseStream << "    </tr>\n";
	responseStream << "    </tbody>\n";
	responseStream << "</table>\n";
	responseStream << "\n";
	// begin include footer.html
	responseStream << "<footer>\n";
	responseStream << "    <a href=\"/api/poweroff?key={{API_AUTH}}\" class=\"btn\" role=\"button\">poweroff</a>\n";
	responseStream << "</footer>\n";
	responseStream << "\n";
	responseStream << "</div>\n";
	responseStream << "</body>\n";
	responseStream << "</html>";
	// end include footer.html
}
