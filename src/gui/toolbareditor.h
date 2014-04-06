#ifndef TOOLBAREDITOR_H
#define TOOLBAREDITOR_H

#include <QWidget>

#include "ui_toolbareditor.h"


namespace Ui {
  class ToolBarEditor;
}

class BaseToolBar;

// TODO: dialog pro úpravu prirazeneho toolbaru.
class ToolBarEditor : public QWidget {
    Q_OBJECT

  public:
    // Constructors and destructors.
    explicit ToolBarEditor(QWidget *parent = 0);
    virtual ~ToolBarEditor();

    // Toolbar operations.
    void loadFromToolBar(BaseToolBar *tool_bar);
    void saveToolBar();

  private slots:
    void insertSpacer();
    void insertSeparator();

  private:
    Ui::ToolBarEditor *m_ui;
    BaseToolBar *m_toolBar;
};

#endif // TOOLBAREDITOR_H
