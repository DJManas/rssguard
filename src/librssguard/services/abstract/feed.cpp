// For license of this file, see <project-root-folder>/LICENSE.md.

#include "services/abstract/feed.h"

#include "database/databasequeries.h"
#include "definitions/definitions.h"
#include "miscellaneous/application.h"
#include "miscellaneous/feedreader.h"
#include "miscellaneous/iconfactory.h"
#include "miscellaneous/mutex.h"
#include "miscellaneous/textfactory.h"
#include "services/abstract/cacheforserviceroot.h"
#include "services/abstract/gui/formfeeddetails.h"
#include "services/abstract/importantnode.h"
#include "services/abstract/labelsnode.h"
#include "services/abstract/recyclebin.h"
#include "services/abstract/serviceroot.h"
#include "services/abstract/unreadnode.h"

Feed::Feed(RootItem* parent)
  : RootItem(parent), m_source(QString()), m_status(Status::Normal), m_statusString(QString()),
    m_autoUpdateType(AutoUpdateType::DefaultAutoUpdate), m_autoUpdateInterval(DEFAULT_AUTO_UPDATE_INTERVAL),
    m_lastUpdated(QDateTime::currentDateTimeUtc()), m_isSwitchedOff(false), m_isQuiet(false),
    m_openArticlesDirectly(false), m_messageFilters(QList<QPointer<MessageFilter>>()) {
  setKind(RootItem::Kind::Feed);
}

Feed::Feed(const QString& title, const QString& custom_id, const QIcon& icon, RootItem* parent) : Feed(parent) {
  setTitle(title);
  setCustomId(custom_id);
  setIcon(icon);
}

Feed::Feed(const Feed& other) : RootItem(other) {
  setKind(RootItem::Kind::Feed);

  setCountOfAllMessages(other.countOfAllMessages());
  setCountOfUnreadMessages(other.countOfUnreadMessages());
  setSource(other.source());
  setStatus(other.status(), other.statusString());
  setAutoUpdateType(other.autoUpdateType());
  setAutoUpdateInterval(other.autoUpdateInterval());
  setLastUpdated(other.lastUpdated());
  setMessageFilters(other.messageFilters());
  setOpenArticlesDirectly(other.openArticlesDirectly());
  setIsSwitchedOff(other.isSwitchedOff());
  setIsQuiet(other.isQuiet());
}

QList<Message> Feed::undeletedMessages() const {
  QSqlDatabase database = qApp->database()->driver()->connection(metaObject()->className());

  return DatabaseQueries::getUndeletedMessagesForFeed(database, customId(), getParentServiceRoot()->accountId());
}

QVariant Feed::data(int column, int role) const {
  switch (role) {
    case HIGHLIGHTED_FOREGROUND_TITLE_ROLE:
      switch (status()) {
        case Status::NewMessages:
          return qApp->skins()->currentSkin().colorForModel(SkinEnums::PaletteColors::FgSelectedNewMessages);

        case Status::Normal:
          if (countOfUnreadMessages() > 0) {
            return qApp->skins()->currentSkin().colorForModel(SkinEnums::PaletteColors::FgSelectedInteresting);
          }
          else {
            return QVariant();
          }

        case Status::NetworkError:
        case Status::ParsingError:
        case Status::AuthError:
        case Status::OtherError:
          return qApp->skins()->currentSkin().colorForModel(SkinEnums::PaletteColors::FgSelectedError);

        default:
          return QVariant();
      }

    case Qt::ItemDataRole::ForegroundRole:
      switch (status()) {
        case Status::NewMessages:
          return qApp->skins()->currentSkin().colorForModel(SkinEnums::PaletteColors::FgNewMessages);

        case Status::Normal:
          if (countOfUnreadMessages() > 0) {
            return qApp->skins()->currentSkin().colorForModel(SkinEnums::PaletteColors::FgInteresting);
          }
          else {
            return QVariant();
          }

        case Status::NetworkError:
        case Status::ParsingError:
        case Status::AuthError:
        case Status::OtherError:
          return qApp->skins()->currentSkin().colorForModel(SkinEnums::PaletteColors::FgError);

        default:
          return QVariant();
      }

    default:
      return RootItem::data(column, role);
  }
}

int Feed::autoUpdateInterval() const {
  return m_autoUpdateInterval;
}

int Feed::countOfAllMessages() const {
  return m_totalCount;
}

int Feed::countOfUnreadMessages() const {
  return m_unreadCount;
}

QVariantHash Feed::customDatabaseData() const {
  return {};
}

void Feed::setCustomDatabaseData(const QVariantHash& data) {
  Q_UNUSED(data)
}

void Feed::setCountOfAllMessages(int count_all_messages) {
  m_totalCount = count_all_messages;
}

void Feed::setCountOfUnreadMessages(int count_unread_messages) {
  if (status() == Status::NewMessages && count_unread_messages < m_unreadCount) {
    setStatus(Status::Normal);
  }

  m_unreadCount = count_unread_messages;
}

bool Feed::canBeEdited() const {
  return true;
}

bool Feed::editViaGui() {
  QScopedPointer<FormFeedDetails> form_pointer(new FormFeedDetails(getParentServiceRoot(), qApp->mainFormWidget()));

  form_pointer->addEditFeed(this);
  return false;
}

void Feed::setAutoUpdateInterval(int auto_update_interval) {
  // If new initial auto-update interval is set, then
  // we should reset time that remains to the next auto-update.
  m_autoUpdateInterval = auto_update_interval;
  m_lastUpdated = QDateTime::currentDateTimeUtc();
}

Feed::AutoUpdateType Feed::autoUpdateType() const {
  return m_autoUpdateType;
}

void Feed::setAutoUpdateType(Feed::AutoUpdateType auto_update_type) {
  m_autoUpdateType = auto_update_type;
}

Feed::Status Feed::status() const {
  return m_status;
}

void Feed::setStatus(Feed::Status status, const QString& status_text) {
  m_status = status;
  m_statusString = status_text;
}

QString Feed::source() const {
  return m_source;
}

void Feed::setSource(const QString& source) {
  m_source = source;
}

bool Feed::openArticlesDirectly() const {
  return m_openArticlesDirectly;
}

void Feed::setOpenArticlesDirectly(bool opn) {
  m_openArticlesDirectly = opn;
}

void Feed::appendMessageFilter(MessageFilter* filter) {
  m_messageFilters.append(QPointer<MessageFilter>(filter));
}

void Feed::updateCounts(bool including_total_count) {
  QSqlDatabase database = qApp->database()->driver()->threadSafeConnection(metaObject()->className());
  int account_id = getParentServiceRoot()->accountId();

  if (including_total_count) {
    setCountOfAllMessages(DatabaseQueries::getMessageCountsForFeed(database, customId(), account_id, true));
  }

  setCountOfUnreadMessages(DatabaseQueries::getMessageCountsForFeed(database, customId(), account_id, false));
}

bool Feed::cleanMessages(bool clean_read_only) {
  return getParentServiceRoot()->cleanFeeds(QList<Feed*>() << this, clean_read_only);
}

bool Feed::markAsReadUnread(RootItem::ReadStatus status) {
  ServiceRoot* service = getParentServiceRoot();
  auto* cache = dynamic_cast<CacheForServiceRoot*>(service);

  if (cache != nullptr) {
    cache->addMessageStatesToCache(service->customIDSOfMessagesForItem(this), status);
  }

  return service->markFeedsReadUnread(QList<Feed*>() << this, status);
}

QString Feed::getAutoUpdateStatusDescription() const {
  QString auto_update_string;

  switch (autoUpdateType()) {
    case AutoUpdateType::DontAutoUpdate:
      //: Describes feed auto-update status.
      auto_update_string = tr("does not use auto-fetching of articles");
      break;

    case AutoUpdateType::DefaultAutoUpdate:
      //: Describes feed auto-update status.
      if (qApp->feedReader()->autoUpdateEnabled()) {
        int secs_to_next =
          QDateTime::currentDateTimeUtc()
            .secsTo(qApp->feedReader()->lastAutoUpdate().addSecs(qApp->feedReader()->autoUpdateInterval()));

        auto_update_string =
          tr("uses global settings (%n minute(s) to next auto-fetch of articles)", nullptr, int(secs_to_next / 60.0));
      }
      else {
        auto_update_string = tr("uses global settings, but global auto-fetching of articles is disabled");
      }

      break;

    case AutoUpdateType::SpecificAutoUpdate:
    default:
      int secs_to_next = QDateTime::currentDateTimeUtc().secsTo(lastUpdated().addSecs(autoUpdateInterval()));

      //: Describes feed auto-update status.
      auto_update_string = tr("uses specific settings (%n minute(s) to next auto-fetching of new articles)",
                              nullptr,
                              int(secs_to_next / 60.0));
      break;
  }

  return auto_update_string;
}

QString Feed::getStatusDescription() const {
  switch (m_status) {
    case Status::Normal:
      return tr("no errors");

    case Status::NewMessages:
      return tr("has new articles");

    case Status::AuthError:
      return tr("authentication error");

    case Status::NetworkError:
      return tr("network error");

    case Status::ParsingError:
      return tr("parsing error");

    default:
      return tr("error");
  }
}

bool Feed::isQuiet() const {
  return m_isQuiet;
}

void Feed::setIsQuiet(bool quiet) {
  m_isQuiet = quiet;
}

QDateTime Feed::lastUpdated() const {
  return m_lastUpdated;
}

void Feed::setLastUpdated(const QDateTime& last_updated) {
  m_lastUpdated = last_updated;
}

bool Feed::isSwitchedOff() const {
  return m_isSwitchedOff;
}

void Feed::setIsSwitchedOff(bool switched_off) {
  m_isSwitchedOff = switched_off;
}

QString Feed::statusString() const {
  return m_statusString;
}

QList<QPointer<MessageFilter>> Feed::messageFilters() const {
  return m_messageFilters;
}

void Feed::setMessageFilters(const QList<QPointer<MessageFilter>>& filters) {
  m_messageFilters = filters;
}

void Feed::removeMessageFilter(MessageFilter* filter) {
  int idx = m_messageFilters.indexOf(filter);

  if (idx >= 0) {
    m_messageFilters.removeAll(filter);
  }
}

QString Feed::additionalTooltip() const {
  QString stat = getStatusDescription();

  if (!m_statusString.simplified().isEmpty()) {
    stat += QSL(" (%1)").arg(m_statusString);
  }

  return tr("Auto-update status: %1\n"
            "Active message filters: %2\n"
            "Status: %3")
    .arg(getAutoUpdateStatusDescription(), QString::number(m_messageFilters.size()), stat);
}

Qt::ItemFlags Feed::additionalFlags() const {
  return Qt::ItemFlag::ItemNeverHasChildren;
}
