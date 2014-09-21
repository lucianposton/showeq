/*
 * cgiconv.cpp
 * 
 * ShowEQ Distributed under GPL
 * http://seq.sourceforge.net/
 *
 * Copyright 2001 Zaphod (dohpaz@users.sourceforge.net). All Rights Reserved.
 *
 * Contributed to ShowEQ by Zaphod (dohpaz@users.sourceforge.net) 
 * for use under the terms of the GNU General Public License, 
 * incorporated herein by reference.
 *
 */

#include <stdlib.h>

#include <qregexp.h>
#include <qfile.h>
#include <q3textstream.h>

#include "cgiconv.h"

CGI::CGI()
{
}

void CGI::processCGIData()
{
  QString request_method;


  // retrieve request method
  request_method = getenv("REQUEST_METHOD");

  // what is the request method (POST and GET supported)
  if (request_method == "POST") 
  {
    // determine the length of the data
    QString content_length_str = getenv("CONTENT_LENGTH");
    int content_length = content_length_str.toInt();

    // allocate a buffer to hold the query
    char* query_str = new char[content_length + 1];

    // open the data stream
    Q3TextStream postStream(stdin, QIODevice::ReadOnly);

    // set the encoding (required for readRawBytes to work)
    postStream.setEncoding(Q3TextStream::Latin1);
    
    // read the data into the allocated buffer
    postStream.readRawBytes(query_str, content_length);

    // make sure to NULL terminate the string
    query_str[content_length] = '\0';

    // store the data
    query_string = query_str;

    // delete the temporary buffer
    delete query_str;
  }
  else if (request_method == "GET")
  {
    query_string = getenv("QUERY_STRING");
  }
  else 
    query_string = "";

  // if there is a query string, proces the arguments from it.
  if (!query_string.isEmpty())
  {
    QString name = "";
    QString value = "";
    int index = 0;
    int oldindex = 0;
    
    // frequently used regular expressions
    QRegExp paramEndExp("[&;]");
    QRegExp paramPlusExp("\\+");

    // iterate over the string finding name/value pairs
    do 
    {
      // find the end of the parameter name (ends at '=')
      index = query_string.find('=', oldindex);
      if (index == -1)
	break;

      // extract the name from the query string
      name = query_string.mid(oldindex, (index - oldindex));

      // replace +'s with spaces
      name.replace(paramPlusExp, " ");
      
      // unescape the string
      name = unescapeURL(name);

      // the new place to search from
      oldindex = index + 1;

      // find the end of the parameter pair (ends at '&')
      index = query_string.find(paramEndExp, oldindex);
      
      // extract the value from the query_string
      if (index != -1)
	value = query_string.mid(oldindex, (index - oldindex));
      else
	value = query_string.mid(oldindex, (query_string.length() - oldindex));

      // replace +'s with spaces
      value.replace(paramPlusExp, " ");

      // unescape the string
      value = unescapeURL(value);

      // the new oldindex
      oldindex = index + 1;

      if (value.isNull())
	value = "";

      // insert the parameter into the parameter list
      cgiParams.append(new CGIParam(name, value));
	
    }    
    while (index != -1);  // while not out of parameters
  }
}

void CGI::logCGIData(const QString& filename)
{
  // create a QFile object to do the file I/O
  QFile file(filename);
  
  // open the file
  if (file.open(QIODevice::WriteOnly))
  {
    // create a QTextStream object on the file
    Q3TextStream textFile(&file);

    // get the environment
    textFile << "REQUEST_METHOD=" << getenv("REQUEST_METHOD") << endl;
    textFile << "CONTENT_LENGTH=" << getenv("CONTENT_LENGTH") << endl;

    // write the query string to the file
    textFile << "QUERY_STRING=" << query_string << endl; 

    // write misc. CGI environment pieces
    textFile << "AUTH_TYPE=" << getAuthType() << endl;
    textFile << "GATEWAY_INTERFACE=" << getGatewayInterface() << endl;
    textFile << "HTTP_ACCEPT=" << getHTTPAccept() << endl;
    textFile << "HTTP_ACCEPT_ENCODING=" << getHTTPAcceptEncoding() << endl;
    textFile << "HTTP_ACCEPT_LANGUAGE=" << getHTTPAcceptLanguage() << endl;
    textFile << "HTTP_CONNECTION=" << getHTTPConnection() << endl;
    textFile << "HTTP_REFERER=" << getHTTPReferer() << endl;
    textFile << "HTTP_USER_AGENT=" << getHTTPUserAgent() << endl;
    textFile << "REMOTE_HOST=" << getRemoteHost() << endl;
    textFile << "REMOTE_ADDRESS=" << getRemoteAddress() << endl;
    textFile << "REMOTE_PORT=" << getRemotePort() << endl;
    textFile << "REQUEST_URI=" << getRequestURI() << endl;
    textFile << "SCRIPT_NAME=" << getScriptName() << endl;
    textFile << "SERVER_ADMIN=" << getServerAdmin() << endl;
    textFile << "SERVER_NAME=" << getServerName() << endl;
    textFile << "SERVER_PORT=" << getServerPort() << endl;
    textFile << "SERVER_PROTOCOL=" << getServerProtocol() << endl;
    textFile << "SERVER_SOFTWARE=" << getServerSoftware() << endl;
  }

  // close the file
  file.close();

}

QString CGI::getParamName(int paramNum)
{
  int count = 0;
  
  // iterate over the list of parameters
  for (CGIParam* param = cgiParams.first(); 
       param != 0; 
       param = cgiParams.next(), ++count)
  {
    // if the count matches the parameter number requested, then return it.
    if (count == paramNum)
      return param->getValue();
  }

  return "";
}

int CGI::getParamNameCount(const QString& paramName)
{
  int count = 0;
  
  // iterate over the list of parameters
  for (CGIParam* param = cgiParams.first(); 
       param != 0; 
       param = cgiParams.next())
  {
    // if this is the correct parameter, increment the count
    if (param->getName() == paramName)
      count++;
  }

  // return the number of instances of the parameter in the list
  return count;
}

QString CGI::getParamValue(const QString& paramName, int instance)
{
  int count = 0;
  
  // iterate over the list of parameters
  for (CGIParam* param = cgiParams.first(); 
       param != 0; 
       param = cgiParams.next())
  {
    // is this the correct parameter
    if (param->getName() == paramName)
    {
      // yes, is the count correct, if so return the value
      if (count == instance)
	return param->getValue();

      // otherwise, increment the count
      count++;
    } 
  }
  
  return "";
}

QString CGI::getAuthType()
{
  const char* env = getenv("AUTH_TYPE");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getGatewayInterface()
{
  const char* env = getenv("GATEWAY_INTERFACE");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getHTTPAccept()
{
  const char* env = getenv("HTTP_ACCEPT");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getHTTPAcceptEncoding()
{
  const char* env = getenv("HTTP_ACCEPT_ENCODING");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getHTTPAcceptLanguage()
{
  const char* env = getenv("HTTP_ACCEPT_LANGUAGE");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getHTTPConnection()
{
  const char* env = getenv("HTTP_CONNECTION");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getHTTPReferer()
{
  const char* env = getenv("HTTP_REFERER");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getHTTPUserAgent()
{
  const char* env = getenv("HTTP_USER_AGENT");

  if (env)
    return env;
  else
    return "";
}


QString CGI::getRemoteHost()
{
  const char* env = getenv("REMOTE_HOST");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getRemoteAddress()
{
  const char* env = getenv("REMOTE_ADDRESS");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getRemotePort()
{
  const char* env = getenv("REMOTE_PORT");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getRequestURI()
{
  const char* env = getenv("REQUEST_URI");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getScriptName()
{
  const char* env = getenv("SCRIPT_NAME");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getServerAdmin()
{
  const char* env = getenv("SERVER_ADMIN");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getServerName()
{
  const char* env = getenv("SERVER_NAME");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getServerPort()
{
  const char* env = getenv("SERVER_PORT");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getServerProtocol()
{
  const char* env = getenv("SERVER_PROTOCOL");

  if (env)
    return env;
  else
    return "";
}

QString CGI::getServerSoftware()
{
  const char* env = getenv("SERVER_SOFTWARE");

  if (env)
    return env;
  else
    return "";
}


QString CGI::unescapeURL(QString url)
{
  int index = 0;
  
  // find first % in the string
  index = url.find('%', index);

  // loop over string while %'s exist
  while (index != -1)
  {
    // turn the string from % and the 2 chars following into a hex value
    url.replace(index, 3, QString(QChar((char)url.mid(index + 1, 2).toInt(NULL, 16))));

    // find next occurrence of % in the URL
    index = url.find('%', index + 1);
  }

  return url;
}

#ifdef CGI_TEST_PROG
int main(int argc, char**argv)
{
  extern char **environ;
  char **it;
  CGI cgiconv;

  cgiconv.processCGIData();

  printf("Content-type: text/html\n\n");

  printf("<H1>Arguments: (count = %d)</H1>\n", argc);
  for (it = argv; *it; it++)
    printf("%s ", *it);
  printf("\n");

  printf("<H1>Environment</H1>\n");
  printf("<PRE>\n");
  for (it = environ; *it ; it++)
    printf("%s\n", *it);
  printf("</PRE>\n");

  printf("<H1>CGI Post Data</H1>\n");
  printf("%s\n", (const char*)cgiconv.getQueryString());

#ifdef CGI_TEST_SAVE_DATA
  FILE* test = fopen("/tmp/testdata.txt", "w");
  fprintf(test, "%s", (const char*)cgiconv.getQueryString());
  fclose(test);
#endif

  printf("<H1>CGI Test</H1>\n");
  printf("<PRE>\n");
  int i, count;
  printf("Number of Parameters: %d\n", cgiconv.getParamCount());
  printf("name='%s'\n", (const char*)cgiconv.getParamValue("name"));
  printf("address='%s'\n", (const char*)cgiconv.getParamValue("address"));
  count = cgiconv.getParamNameCount("flavors");
  printf("# of flavors = %d\n", count);
  for (i = 0; i < count; i++)
    printf("flavors[%d]='%s'\n", i, (const char*)cgiconv.getParamValue("flavors", i));
  printf("</PRE>\n");
}
#endif
