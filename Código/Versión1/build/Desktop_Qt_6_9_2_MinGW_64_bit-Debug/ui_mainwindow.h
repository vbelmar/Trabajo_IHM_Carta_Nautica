/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionMoveMode;
    QAction *actionClickMode;
    QAction *actionDrawLine;
    QAction *actionProtractor;
    QWidget *centralwidget;
    QFormLayout *formLayout;
    QMenuBar *menubar;
    QStatusBar *statusbar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        actionMoveMode = new QAction(MainWindow);
        actionMoveMode->setObjectName("actionMoveMode");
        actionMoveMode->setCheckable(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/new/prefix1/resources/icons/mano.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionMoveMode->setIcon(icon);
        actionMoveMode->setMenuRole(QAction::MenuRole::NoRole);
        actionClickMode = new QAction(MainWindow);
        actionClickMode->setObjectName("actionClickMode");
        actionClickMode->setCheckable(true);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/new/prefix1/resources/icons/raton.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        actionClickMode->setIcon(icon1);
        actionClickMode->setMenuRole(QAction::MenuRole::NoRole);
        actionDrawLine = new QAction(MainWindow);
        actionDrawLine->setObjectName("actionDrawLine");
        actionDrawLine->setCheckable(true);
        QIcon icon2(QIcon::fromTheme(QIcon::ThemeIcon::MailMessageNew));
        actionDrawLine->setIcon(icon2);
        actionDrawLine->setMenuRole(QAction::MenuRole::NoRole);
        actionProtractor = new QAction(MainWindow);
        actionProtractor->setObjectName("actionProtractor");
        actionProtractor->setCheckable(true);
        QIcon icon3(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStop));
        actionProtractor->setIcon(icon3);
        actionProtractor->setMenuRole(QAction::MenuRole::NoRole);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        formLayout = new QFormLayout(centralwidget);
        formLayout->setObjectName("formLayout");
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 26));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName("toolBar");
        MainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, toolBar);

        toolBar->addAction(actionMoveMode);
        toolBar->addAction(actionClickMode);
        toolBar->addSeparator();
        toolBar->addAction(actionDrawLine);
        toolBar->addAction(actionProtractor);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionMoveMode->setText(QCoreApplication::translate("MainWindow", "Move", nullptr));
        actionClickMode->setText(QCoreApplication::translate("MainWindow", "Click", nullptr));
        actionDrawLine->setText(QCoreApplication::translate("MainWindow", "DrawLine", nullptr));
        actionProtractor->setText(QCoreApplication::translate("MainWindow", "Protractor", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
