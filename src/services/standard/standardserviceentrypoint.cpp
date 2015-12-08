// This file is part of RSS Guard.
//
// Copyright (C) 2011-2015 by Martin Rotter <rotter.martinos@gmail.com>
//
// RSS Guard is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RSS Guard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RSS Guard. If not, see <http://www.gnu.org/licenses/>.


#include "services/standard/standardserviceentrypoint.h"

#include "definitions/definitions.h"
#include "miscellaneous/application.h"
#include "services/standard/standardserviceroot.h"

#include <QSqlQuery>


StandardServiceEntryPoint::StandardServiceEntryPoint() {
}

StandardServiceEntryPoint::~StandardServiceEntryPoint() {
}

bool StandardServiceEntryPoint::isSingleInstanceService() {
  return true;
}

QString StandardServiceEntryPoint::name() {
  return QSL("Standard online feeds (RSS/RDF/ATOM)");
}

QString StandardServiceEntryPoint::description() {
  return QSL("This service offers integration with standard online RSS/RDF/ATOM feeds and podcasts.");
}

QString StandardServiceEntryPoint::version() {
  return APP_VERSION;
}

QString StandardServiceEntryPoint::author() {
  return APP_AUTHOR;
}

QIcon StandardServiceEntryPoint::icon() {
  return QIcon(APP_ICON_PATH);
}

QString StandardServiceEntryPoint::code() {
  return SERVICE_CODE_STD_RSS;
}

ServiceRoot *StandardServiceEntryPoint::createNewRoot() {
  // Switch DB.
  QSqlDatabase database = qApp->database()->connection(QSL("StandardServiceEntryPoint"), DatabaseFactory::FromSettings);
  QSqlQuery query(database);

  // First obtain the ID, which can be assigned to this new account.
  if (!query.exec("SELECT max(id) FROM Accounts;") || !query.next()) {
    return NULL;
  }

  int id_to_assing = query.value(0).toInt() + 1;

  if (query.exec(QString("INSERT INTO Accounts (id, type) VALUES (%1, '%2');").arg(QString::number(id_to_assing),
                                                                                   SERVICE_CODE_STD_RSS))) {
    StandardServiceRoot *root = new StandardServiceRoot();
    root->setAccountId(id_to_assing);
    return root;
  }
  else {
    return NULL;
  }
}

QList<ServiceRoot*> StandardServiceEntryPoint::initializeSubtree() {
  // Check DB if standard account is enabled.
  QSqlDatabase database = qApp->database()->connection(QSL("StandardServiceEntryPoint"), DatabaseFactory::FromSettings);
  QSqlQuery query(database);
  QList<ServiceRoot*> roots;

  if (query.exec(QString("SELECT id FROM Accounts WHERE type = '%1';").arg(SERVICE_CODE_STD_RSS))) {
    while (query.next()) {
      StandardServiceRoot *root = new StandardServiceRoot();
      root->setAccountId(query.value(0).toInt());
      roots.append(root);
    }
  }

  return roots;
}
