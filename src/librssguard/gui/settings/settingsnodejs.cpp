// For license of this file, see <project-root-folder>/LICENSE.md.

#include "gui/settings/settingsnodejs.h"

#include "definitions/definitions.h"
#include "exceptions/applicationexception.h"
#include "miscellaneous/application.h"
#include "miscellaneous/nodejs.h"

#include <QDir>
#include <QFileDialog>

SettingsNodejs::SettingsNodejs(Settings* settings, QWidget* parent) : SettingsPanel(settings, parent) {
  m_ui.setupUi(this);

  m_ui.m_helpInfo->setHelpText(tr("What is Node.js?"),
                               tr("Node.js is asynchronous event-driven JavaScript runtime, designed to build "
                                  "scalable network applications.\n\n"
                                  "%1 integrates Node.js to bring some modern features like Adblock.\n\n"
                                  "Note that usually all required Node.js tools should be available via your \"PATH\" "
                                  "environment variable, so you do not have to specify full paths.").arg(APP_NAME),
                               false);

  m_ui.m_helpPackages->setHelpText(tr("%1 automatically installs some Node.js packages so that you do not have to. %1 does not "
                                      "use global package folder because that requires administrator rights, therefore by default "
                                      "it uses subfolder placed in your \"user data\" folder.").arg(APP_NAME),
                                   false);

  connect(m_ui.m_tbNodeExecutable->lineEdit(), &BaseLineEdit::textChanged,
          this, &SettingsNodejs::testNodejs);
  connect(m_ui.m_tbNpmExecutable->lineEdit(), &BaseLineEdit::textChanged,
          this, &SettingsNodejs::testNpm);
  connect(m_ui.m_tbPackageFolder->lineEdit(), &BaseLineEdit::textChanged,
          this, &SettingsNodejs::testPackageFolder);

  connect(m_ui.m_tbNodeExecutable->lineEdit(), &BaseLineEdit::textChanged,
          this, &SettingsNodejs::dirtifySettings);
  connect(m_ui.m_tbNpmExecutable->lineEdit(), &BaseLineEdit::textChanged,
          this, &SettingsNodejs::dirtifySettings);
  connect(m_ui.m_tbPackageFolder->lineEdit(), &BaseLineEdit::textChanged,
          this, &SettingsNodejs::dirtifySettings);

  connect(m_ui.m_btnPackageFolder, &QPushButton::clicked, this, [this]() {
    changeFileFolder(m_ui.m_tbPackageFolder, true);
  });
  connect(m_ui.m_btnNodeExecutable, &QPushButton::clicked, this, [this]() {
    changeFileFolder(m_ui.m_tbPackageFolder, false, QSL("Node.js (node*)"));
  });
  connect(m_ui.m_btnNpmExecutable, &QPushButton::clicked, this, [this]() {
    changeFileFolder(m_ui.m_tbPackageFolder, false, QSL("NPM (npm*)"));
  });
}

void SettingsNodejs::changeFileFolder(LineEditWithStatus* tb, bool directory_select, const QString& file_filter) {
  QFileDialog d(this);

  d.setFileMode(directory_select ? QFileDialog::FileMode::Directory : QFileDialog::FileMode::ExistingFile);

  if (directory_select) {
    d.setOption(QFileDialog::Option::ShowDirsOnly);
  }
  else {
    d.setNameFilter(file_filter);
  }

  QString current = qApp->replaceDataUserDataFolderPlaceholder(tb->lineEdit()->text());

  d.selectFile(current);

  if (d.exec() == QDialog::DialogCode::Accepted && !d.selectedFiles().isEmpty()) {
    tb->lineEdit()->setText(QDir::toNativeSeparators(d.selectedFiles().at(0)));
  }
}

QString SettingsNodejs::title() const {
  return QSL("Node.js");
}

void SettingsNodejs::loadSettings() {
  onBeginLoadSettings();

  m_ui.m_tbNodeExecutable->lineEdit()->setText(qApp->nodejs()->nodeJsExecutable());
  m_ui.m_tbNpmExecutable->lineEdit()->setText(qApp->nodejs()->npmExecutable());
  m_ui.m_tbPackageFolder->lineEdit()->setText(qApp->nodejs()->packageFolder());

  onEndLoadSettings();
}

void SettingsNodejs::saveSettings() {
  onBeginSaveSettings();

  qApp->nodejs()->setNodeJsExecutable(m_ui.m_tbNodeExecutable->lineEdit()->text());
  qApp->nodejs()->setNpmExecutable(m_ui.m_tbNpmExecutable->lineEdit()->text());
  qApp->nodejs()->setPackageFolder(m_ui.m_tbPackageFolder->lineEdit()->text());

  onEndSaveSettings();
}

void SettingsNodejs::testNodejs() {
  try {
    QString node_version = qApp->nodejs()->nodejsVersion(m_ui.m_tbNodeExecutable->lineEdit()->text());

    m_ui.m_tbNodeExecutable->setStatus(WidgetWithStatus::StatusType::Ok,
                                       tr("Node.js has version %1.").arg(node_version));
  }
  catch (const ApplicationException& ex) {
    m_ui.m_tbNodeExecutable->setStatus(WidgetWithStatus::StatusType::Error,
                                       tr("Node.js: %1.").arg(ex.message()));
  }
}

void SettingsNodejs::testNpm() {
  try {
    QString npm_version = qApp->nodejs()->npmVersion(m_ui.m_tbNpmExecutable->lineEdit()->text());

    m_ui.m_tbNpmExecutable->setStatus(WidgetWithStatus::StatusType::Ok,
                                      tr("NPM has version %1.").arg(npm_version));
  }
  catch (const ApplicationException& ex) {
    m_ui.m_tbNpmExecutable->setStatus(WidgetWithStatus::StatusType::Error,
                                      tr("NPM: %1.").arg(ex.message()));
  }
}

void SettingsNodejs::testPackageFolder() {
  QString folder = qApp->replaceDataUserDataFolderPlaceholder(m_ui.m_tbPackageFolder->lineEdit()->text());

  m_ui.m_tbPackageFolder->setStatus(WidgetWithStatus::StatusType::Ok,
                                    QDir().exists(folder)
                                    ? tr("Package folder is OK.")
                                    : tr("Package folder will be created!"));
}