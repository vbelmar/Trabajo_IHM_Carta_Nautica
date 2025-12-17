#include "mainwindow.h"
#include "authwindow.h"
#include "navigation.h"
#include "navdaoexception.h"

#include <QApplication>
#include <QMessageBox>
#include <QFile>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//cargar estilos
    QFile styleFile(":/new/prefix1/resources/styles.qss");
    styleFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(styleFile.readAll());
    qApp->setStyleSheet(styleSheet);

    //cargar base datos
    try {
        Navigation::instance().reload();
        qDebug() << "Base datos cargada correctamente";
    }catch(const NavDAOException &e){
        QMessageBox::critical(nullptr, "Error de Base de Datos",
                              QString("No se pudo cargar la base de datos:\n%1").arg(e.what()));
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
