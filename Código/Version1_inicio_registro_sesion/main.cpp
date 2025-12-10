#include "mainwindow.h"
//AÑADIDO
#include "authwindow.h"
#include "navigation.h"


#include <QApplication>
#include <QMessageBox> //AÑADIDO
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//AÑADIDO
    try {
        Navigation::instance().reload();

    }catch(const NavDAOException &e){
        QMessageBox::critical(nullptr, "Error", e.what());
        return -1;
    }

    AuthWindow authWindow; //mostrar ventana login primero
    authWindow.setWindowState(Qt::WindowMaximized);
    int result = authWindow.exec();

    // ========================================
    // PASO 2: Si login exitoso (accept), abrir mapa
    if (result == QDialog::Accepted) {
        // Obtener el usuario que hizo login
        QString username = authWindow.getLoggedUsername();

        // Crear y mostrar ventana del mapa
        MainWindow *mainWindow = new MainWindow(username);
        mainWindow->showMaximized();

        // Mantener la aplicación corriendo
        return a.exec();
    }

    // Si el usuario canceló el login, salir
    return 0;
    //MainWindow w;
    //w.show();
    //return a.exec();
}
