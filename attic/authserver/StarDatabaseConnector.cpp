#include "StarDatabaseConnector.hpp"
#include "StarLogging.hpp"

//todo, ifdef for database type or something ?

#include <iostream>
#include "libpq-fe.h"

using namespace std;

namespace Star {

class DatabaseConnectionImpl {
public:
  PGconn* conn;
};

DatabaseConnector::DatabaseConnector(String const& connectionString) {
  m_impl = new DatabaseConnectionImpl;
  m_impl->conn = PQconnectdb(connectionString.utf8Ptr());
  if (PQstatus(m_impl->conn) != CONNECTION_OK) {
    throw StarException(strf("Fail to connect to the database. %s", PQerrorMessage(m_impl->conn)));
  }
}

DatabaseConnector::~DatabaseConnector() {
  PQfinish(m_impl->conn);
  delete m_impl;
}

List<VariantMap> DatabaseConnector::fetch(String const& command, VariantList const& arguments) {
  int nParams = arguments.size();
  List<String> paramValuesBacking(nParams);
  DynamicArray<const char*> paramValues(nParams, (const char*)nullptr);
  for (int i = 0; i < nParams; i++) {
    if (arguments[i].type() == Variant::Type::BOOL)
      paramValuesBacking[i] = arguments[i].toBool()?"t":"f";
    else
      paramValuesBacking[i] = arguments[i].toString();
    paramValues[i] = paramValuesBacking[i].utf8Ptr();
  }
  auto res = PQexecParams(m_impl->conn, command.utf8Ptr(), nParams, NULL, paramValues.ptr(), NULL, NULL, 0);
  if (PQresultStatus(res) == PGRES_COMMAND_OK) {
    PQclear(res);
    return {};
  }
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    auto message = strf("Command failed. Error: (%d) %s", PQresultStatus(res), PQresultErrorMessage(res));
    PQclear(res);
    throw StarException(message);
  }
  List<String> columns;
  int nFields = PQnfields(res);
  for (int i = 0; i < nFields; i++)
    columns.append(PQfname(res, i));

  List<VariantMap> result;
  for (int i = 0; i < PQntuples(res); i++)
      {
    result.append({});
    VariantMap& row = result.last();
    for (int j = 0; j < nFields; j++)
      row.insert(columns[j], PQgetvalue(res, i, j));
  }

  PQclear(res);
  return result;
}

VariantMap DatabaseConnector::fetchRow(String const& command, VariantList const& arguments) {
  auto result = fetch(command, arguments);
  if (result.size() != 1)
    throw StarException(strf("Command failed. Expected single row. Got %s", result.size()));
  return result[0];
}

void DatabaseConnector::exec(String const& command, VariantList const& arguments) {
  auto result = fetch(command, arguments);
  if (result.size() != 0)
    throw StarException(strf("Command failed. Expected no result row. Got %s", result.size()));
}

}
