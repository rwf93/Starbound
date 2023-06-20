#ifndef _STAR_DATABASE_CONNECTOR_HPP_
#define _STAR_DATABASE_CONNECTOR_HPP_

#include "StarAuthenticationKey.hpp"
#include "StarAuthenticationService.hpp"
#include "StarException.hpp"
#include "StarVariant.hpp"
#include "StarRoot.hpp"
#include "StarTcp.hpp"
#include "StarThread.hpp"

namespace Star {

class DatabaseConnectionImpl;

class DatabaseConnector
{
public:
  DatabaseConnector(String const& connectionString);
  ~DatabaseConnector();
  List<VariantMap> fetch(String const& command, VariantList const& arguments = VariantList());
  VariantMap fetchRow(String const& command, VariantList const& arguments = VariantList());
  void exec(String const& command, VariantList const& arguments = VariantList());

private:
  DatabaseConnectionImpl* m_impl;
};

}

#endif
