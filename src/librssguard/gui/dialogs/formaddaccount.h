// For license of this file, see <project-root-folder>/LICENSE.md.

#ifndef FORMADDACCOUNT_H
#define FORMADDACCOUNT_H

#include "ui_formaddaccount.h"

#include <QDialog>

class ServiceEntryPoint;
class FeedsModel;

class FormAddAccount : public QDialog {
    Q_OBJECT

  public:
    explicit FormAddAccount(const QList<ServiceEntryPoint*>& entry_points,
                            FeedsModel* model,
                            QWidget* parent = nullptr);
    virtual ~FormAddAccount();

  private slots:
    void addSelectedAccount();
    void showAccountDetails();

  private:
    ServiceEntryPoint* selectedEntryPoint() const;

    void loadEntryPoints();

    QScopedPointer<Ui::FormAddAccount> m_ui;
    FeedsModel* m_model;
    QList<ServiceEntryPoint*> m_entryPoints;
};

#endif // FORMADDACCOUNT_H
