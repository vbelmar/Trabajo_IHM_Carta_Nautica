#include "authwindow.h"
#include "ui_authwindow.h"
#include "utils.h"
#include "navigation.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QRegularExpression>
#include <QEvent>
#include <QMouseEvent>

AuthWindow::AuthWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthWindow)
{
    ui->setupUi(this);

    // Permitir que el QDialog tenga botones de min/max y se comporte como ventana
    setWindowFlags(windowFlags()
                   | Qt::Window
                   | Qt::WindowMinMaxButtonsHint
                   | Qt::WindowSystemMenuHint);

    // Quitar cualquier hint de tamaño fijo (si estuviese activo)
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, false);

    // Habilitar el “size grip” y un tamaño mínimo razonable
    setSizeGripEnabled(true);
    setMinimumSize(900, 600);

    nicknameValid = false;
    emailValid = false;
    passwordValid = false;
    password2Valid = false;
    birthdateValid = false;

    avatarImage = QImage(100, 100, QImage::Format_RGB32);
    avatarImage.fill(QColor(200, 200, 200));
    //ui->registerAvatarLabel->setPixmap(QPixmap::fromImage(avatarImage));

    ui->registerBirthdateEdit->setMaximumDate(QDate::currentDate().addYears(-16));
    ui->registerBirthdateEdit->setDate(QDate::currentDate().addYears(-18));

    // ============================================
    // EMPEZAR EN LA PÁGINA DE LOGIN (índice 0)
    // ============================================
    ui->stackedWidget->setCurrentIndex(0);

    // Conectar botones del LOGIN
    connect(ui->loginButton, &QPushButton::clicked,
            this, &AuthWindow::onLoginClicked);
    connect(ui->loginPasswordEdit, &QLineEdit::returnPressed,
            this, &AuthWindow::onLoginClicked);

    ui->goToRegisterLabel->installEventFilter(this);
    ui->goToRegisterLabel->setCursor(Qt::PointingHandCursor);

    // Conectar botones del REGISTRO
    connect(ui->registerButton, &QPushButton::clicked,
            this, &AuthWindow::onRegisterClicked);

    connect(ui->backToLoginButton, &QPushButton::clicked,
            this, &AuthWindow::onBackToLogin);

    //connect(ui->selectAvatarButton, &QPushButton::clicked,
      //      this, &AuthWindow::onSelectAvatar);

    connect(ui->registerNicknameEdit, &QLineEdit::editingFinished,
            this, &AuthWindow::onNicknameEditingFinished);
    connect(ui->registerEmailEdit, &QLineEdit::editingFinished,
            this, &AuthWindow::onEmailEditingFinished);
    connect(ui->registerPasswordEdit, &QLineEdit::editingFinished,
            this, &AuthWindow::onPasswordEditingFinished);
    connect(ui->registerPassword2Edit, &QLineEdit::editingFinished,
            this, &AuthWindow::onPassword2EditingFinished);
    connect(ui->registerBirthdateEdit, &QDateEdit::dateChanged,
            this, &AuthWindow::onBirthdateChanged);

    ui->registerButton->setEnabled(false);
}

AuthWindow::~AuthWindow()
{
    delete ui;
}

bool AuthWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->goToRegisterLabel && event->type() == QEvent::MouseButtonPress) {
        onGoToRegister();
        return true;
    }
    return QDialog::eventFilter(obj, event);
}

// ============================================
// IMPLEMENTACIÓN DE LOS SLOTS
// ============================================

void AuthWindow::onLoginClicked()
{
    QString username = ui->loginUsernameEdit->text().trimmed();
    QString password = ui->loginPasswordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Campos Vacíos",
                             "Por favor, introduce tu usuario y contraseña.");
        return;
    }

    User *user = Navigation::instance().authenticate(username, password);

    if (user != nullptr) {
        m_loggedUsername = username;
        accept();  // Cierra AuthWindow y abre MainWindow
    } else {
        QMessageBox::warning(this, "Error de Autenticación",
                             "Usuario o contraseña incorrectos.");
        ui->loginPasswordEdit->clear();
        ui->loginPasswordEdit->setFocus();
    }
}

void AuthWindow::onGoToRegister()
{
    // ============================================
    // CAMBIAR A LA PÁGINA DE REGISTRO (índice 1)
    // ============================================
    ui->stackedWidget->setCurrentIndex(1);
}

void AuthWindow::onBackToLogin()
{
    // ============================================
    // VOLVER A LA PÁGINA DE LOGIN (índice 0)
    // ============================================
    ui->stackedWidget->setCurrentIndex(0);
}



void AuthWindow::onRegisterClicked()
{
    try {
        User newUser(
            ui->registerNicknameEdit->text().trimmed(),
            ui->registerEmailEdit->text().trimmed(),
            ui->registerPasswordEdit->text(),
            avatarImage,
            ui->registerBirthdateEdit->date()
            );

        Navigation::instance().addUser(newUser);

        QMessageBox::information(this, "Registro Exitoso",
                                 "¡Cuenta creada correctamente!\n"
                                 "Ahora puedes iniciar sesión con tus credenciales.");

        // Limpiar campos
        ui->registerNicknameEdit->clear();
        ui->registerEmailEdit->clear();
        ui->registerPasswordEdit->clear();
        ui->registerPassword2Edit->clear();
        ui->registerBirthdateEdit->setDate(QDate::currentDate().addYears(-18));
        avatarImage.fill(QColor(200, 200, 200));
        //ui->registerAvatarLabel->setPixmap(QPixmap::fromImage(avatarImage));

        nicknameValid = false;
        emailValid = false;
        passwordValid = false;
        password2Valid = false;
        birthdateValid = false;
        updateRegisterButton();

        // ============================================
        // VOLVER AL LOGIN DESPUÉS DE REGISTRARSE
        // ============================================
        ui->stackedWidget->setCurrentIndex(0);

    } catch (const NavDAOException &e) {
        QMessageBox::critical(this, "Error al Registrar",
                              QString("No se pudo registrar el usuario:\n%1").arg(e.what()));
    }
}

/*void AuthWindow::onSelectAvatar()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Seleccionar Avatar", "",
                                                    "Imágenes (*.png *.jpg *.jpeg *.bmp)");

    if (!fileName.isEmpty()) {
        avatarImage.load(fileName);
        if (!avatarImage.isNull()) {
            ui->registerAvatarLabel->setPixmap(QPixmap::fromImage(
                avatarImage.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation)
                ));
        } else {
            QMessageBox::warning(this, "Error",
                                 "No se pudo cargar la imagen.");
        }
    }
}*/

void AuthWindow::onNicknameEditingFinished()
{
    QString nickname = ui->registerNicknameEdit->text().trimmed();

    QRegularExpression regex("^[a-zA-Z0-9_-]{6,15}$");
    if (!regex.match(nickname).hasMatch()) {
        QMessageBox::warning(this, "Error en Usuario",
                             "El usuario debe tener entre 6 y 15 caracteres.\n"
                             "Solo letras, números, guiones (-) y guiones bajos (_).");
        ui->registerNicknameEdit->setFocus();
        nicknameValid = false;
        updateRegisterButton();
        return;
    }

    if (Navigation::instance().findUser(nickname) != nullptr) {
        QMessageBox::warning(this, "Error en Usuario",
                             "Este usuario ya existe. Elige otro.");
        ui->registerNicknameEdit->setFocus();
        nicknameValid = false;
        updateRegisterButton();
        return;
    }

    nicknameValid = true;
    updateRegisterButton();
}

void AuthWindow::onEmailEditingFinished()
{
    QString email = ui->registerEmailEdit->text().trimmed();

    if (!Utils::checkEmail(email)) {
        QMessageBox::warning(this, "Error en Email",
                             "El formato del email no es válido.");
        ui->registerEmailEdit->setFocus();
        emailValid = false;
        updateRegisterButton();
        return;
    }

    emailValid = true;
    updateRegisterButton();
}

void AuthWindow::onPasswordEditingFinished()
{
    QString password = ui->registerPasswordEdit->text();

    if (!Utils::checkPassword(password)) {
        QMessageBox::warning(this, "Error en Contraseña",
                             "La contraseña debe tener:\n"
                             "- Entre 8 y 20 caracteres\n"
                             "- Al menos una mayúscula y una minúscula\n"
                             "- Al menos un número\n"
                             "- Al menos un carácter especial (!@#$%&*()-+=)");
        ui->registerPasswordEdit->setFocus();
        passwordValid = false;
        updateRegisterButton();
        return;
    }

    passwordValid = true;
    updateRegisterButton();
}

void AuthWindow::onPassword2EditingFinished()
{
    if (ui->registerPasswordEdit->text() != ui->registerPassword2Edit->text()) {
        QMessageBox::warning(this, "Error en Confirmación",
                             "Las contraseñas no coinciden.");
        ui->registerPasswordEdit->clear();
        ui->registerPassword2Edit->clear();
        ui->registerPasswordEdit->setFocus();
        passwordValid = false;
        password2Valid = false;
        updateRegisterButton();
        return;
    }

    password2Valid = true;
    updateRegisterButton();
}

void AuthWindow::onBirthdateChanged()
{
    QDate birthdate = ui->registerBirthdateEdit->date();
    int age = birthdate.daysTo(QDate::currentDate()) / 365;

    if (age < 16) {
        QMessageBox::warning(this, "Error en Fecha",
                             "Debes tener al menos 16 años para registrarte.");
        birthdateValid = false;
    } else {
        birthdateValid = true;
    }

    updateRegisterButton();
}

void AuthWindow::updateRegisterButton()
{
    ui->registerButton->setEnabled(
        nicknameValid &&
        emailValid &&
        passwordValid &&
        password2Valid &&
        birthdateValid
        );
}
