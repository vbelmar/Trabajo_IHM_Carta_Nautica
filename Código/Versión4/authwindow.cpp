#include "authwindow.h"
#include "ui_authwindow.h"
#include "utils.h"
#include "navigation.h"
#include "navdaoexception.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QRegularExpression>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

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

    // Avatar
    ui->AvatarComboBox->setEditable(true);
    ui->AvatarComboBox->lineEdit()->setReadOnly(true);
    ui->AvatarComboBox->lineEdit()->setAlignment(Qt::AlignCenter);

    // --- NO mostrar nada hasta seleccionar ---
    ui->AvatarComboBox->lineEdit()->clear();
    ui->AvatarComboBox->setCurrentIndex(-1);

    // --- Iconos más pequeños ---
    ui->AvatarComboBox->setIconSize(QSize(32, 32));

    // --- Quitar hueco blanco y ajustar tamaño ---
    ui->AvatarComboBox->setStyleSheet(
        "QComboBox {"
        "    padding: 0px;"
        "    min-width: 40px;"
        "    max-width: 60px;"
        "}"

        "QComboBox QAbstractItemView::item {"
        "    height: 36px;"
        "    padding: 2px;"
        "}"
        "QLineEdit {"
        "    border: none;"
        "    background: transparent;"
        "    qproperty-alignment: AlignCenter;"
        "}"
        );

    // --- Añadir una opción vacía de verdad ---
    ui->AvatarComboBox->addItem(QIcon(), "");

    ui->AvatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar1.png"), "");
    ui->AvatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar2.png"), "");
    ui->AvatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar3.png"), "");
    ui->AvatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar4.png"), "");
    ui->AvatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar5.png"), "");
    ui->AvatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar6.png"), "");

    nicknameValid = false;
    emailValid = false;
    passwordValid = false;
    password2Valid = false;
    birthdateValid = false;
    avatarValid = false;

    //inicializar avatarImage
    avatarImage = QImage(100,100,QImage::Format_RGB32);
    avatarImage.fill(QColor(200, 200, 200));

    //OJO---------------------------------
    // LOGIN
    ui->toggleLoginPasswordButton->setText("");
    ui->toggleLoginPasswordButton->setIcon(QIcon(":/new/prefix1/resources/icons/invisible.png")); // o tu ruta
    ui->toggleLoginPasswordButton->setFlat(true);

    // REGISTRO (pass)
    ui->toggleRegisterPasswordButton->setText("");
    ui->toggleRegisterPasswordButton->setIcon(QIcon(":/new/prefix1/resources/icons/invisible.png"));
    ui->toggleRegisterPasswordButton->setFlat(true);

    // REGISTRO (confirmación)
    ui->toggleRegisterPassword2Button->setText("");
    ui->toggleRegisterPassword2Button->setIcon(QIcon(":/new/prefix1/resources/icons/invisible.png"));
    ui->toggleRegisterPassword2Button->setFlat(true);

    ui->registerBirthdateEdit->setMaximumDate(QDate::currentDate().addYears(-16));
    ui->registerBirthdateEdit->setDate(QDate::currentDate().addYears(-18));

    // Mensajes de error
    ui->nicknameLoginError->setStyleSheet("color: red; font-size: 11px;");
    ui->nicknameLoginError->setVisible(false);
    ui->passwordLoginError->setStyleSheet("color: red; font-size: 11px;");
    ui->passwordLoginError->setVisible(false);
    ui->nicknameRegisterError->setStyleSheet("color: red; font-size: 11px;");
    ui->nicknameRegisterError->setVisible(false);
    ui->emailRegisterError->setStyleSheet("color: red; font-size: 11px");
    ui->emailRegisterError->setVisible(false);
    ui->passwordRegisterError->setStyleSheet("color: red; font-size: 11px;");
    ui->passwordRegisterError->setVisible(false);
    ui->password2RegisterError->setStyleSheet("color: red; font-size: 11px;");
    ui->password2RegisterError->setVisible(false);

    ui->nicknameLoginError->setStyleSheet("color: red; font-size: 11px;");
    ui->nicknameLoginError->setVisible(false);
    ui->passwordLoginError->setStyleSheet("color: red; font-size: 11px");
    ui->passwordLoginError->setVisible(false);
    // ============================================
    // EMPEZAR EN LA PÁGINA DE LOGIN (índice 0)
    // ============================================
    ui->authStack->setCurrentIndex(0);

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

    connect(ui->AvatarComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AuthWindow::onSelectAvatar);

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



    connect(ui->toggleLoginPasswordButton, &QPushButton::clicked,
            this, &AuthWindow::onToggleLoginPasswordVisibility);
    connect(ui->toggleRegisterPasswordButton, &QPushButton::clicked,
            this, &AuthWindow::onToggleRegisterPasswordVisibility);
    connect(ui->toggleRegisterPassword2Button, &QPushButton::clicked,
            this, &AuthWindow::onToggleRegisterPassword2Visibility);

    connect(ui->loginUsernameEdit, &QLineEdit::editingFinished,
            this, &AuthWindow::onLoginUsernameEditingFinished);
    connect(ui->loginPasswordEdit, &QLineEdit::editingFinished,
            this, &AuthWindow::onLoginPasswordEditingFinished);
    ui->registerButton->setEnabled(false);
    //ui->loginButton->setEnabled(false);
    ui->AvatarComboBox->setCurrentIndex(0);
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
        //QMessageBox::warning(this, "Error de Autenticación",
                             //"Usuario o contraseña incorrectos.");
        ui->loginPasswordEdit->clear();
        ui->loginPasswordEdit->setFocus();
    }
}

void AuthWindow::onGoToRegister()
{
    // ============================================
    // CAMBIAR A LA PÁGINA DE REGISTRO (índice 1)
    // ============================================
    ui->authStack->setCurrentIndex(1);
}

void AuthWindow::onBackToLogin()
{
    // ============================================
    // VOLVER A LA PÁGINA DE LOGIN (índice 0)
    // ============================================
    ui->authStack->setCurrentIndex(0);
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
        newUser.setInsertedInDb(false); //AÑADIDO NUEVO¿?
        Navigation::instance().addUser(newUser);

        // Verificar que se guardó
        User *savedUser = Navigation::instance().findUser(newUser.nickName());
        if (savedUser) {
            qDebug() << "✅ Usuario encontrado en memoria después de guardar";
            qDebug() << "Nickname:" << savedUser->nickName();
            qDebug() << "Email:" << savedUser->email();
            qDebug() << "Insertado en DB:" << savedUser->insertedInDb();
        } else {
            qDebug() << "❌ ERROR: Usuario NO encontrado después de guardar";
        }
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

        // Resetear el avatar al índice vacío
        ui->AvatarComboBox->setCurrentIndex(0);
        avatarImage.fill(QColor(200, 200, 200));

        nicknameValid = false;
        emailValid = false;
        passwordValid = false;
        password2Valid = false;
        birthdateValid = false;
        avatarValid = false;
        updateRegisterButton();

        // ============================================
        // VOLVER AL LOGIN DESPUÉS DE REGISTRARSE
        // ============================================
        ui->authStack->setCurrentIndex(0);

    } catch (const NavDAOException &e) {
        QMessageBox::critical(this, "Error al Registrar",
                              QString("No se pudo registrar el usuario:\n%1").arg(e.what()));
    }catch (const std::exception &e) {
        QMessageBox::critical(this, "Error",
                              QString("Error inesperado:\n%1").arg(e.what()));
    }
}

void AuthWindow::onSelectAvatar(int index)
{
    // El índice 0 es la opción vacía, los avatares válidos empiezan en 1
    if (index <= 0) {
        avatarValid = false;
        avatarImage = QImage(100, 100, QImage::Format_RGB32);
        avatarImage.fill(QColor(200, 200, 200));
        updateRegisterButton();
        return;
    }

    QString avatarPath = QString(":/new/prefix1/resources/icons/avatar%1.png").arg(index);
    avatarImage.load(avatarPath);

    if (avatarImage.isNull()) {
        avatarImage = QImage(100, 100, QImage::Format_RGB32);
        avatarImage.fill(QColor(200, 200, 200));
        QMessageBox::warning(this, "Error", "No se pudo cargar la imagen del avatar.");
        avatarValid = false;
    }else {
        avatarValid = true;
    }

    updateRegisterButton();
}

void AuthWindow::onLoginUsernameEditingFinished(){
    QString username = ui->loginUsernameEdit->text().trimmed();
    if (username.isEmpty()) {
        //ui->nicknameLoginError->setText("El usuario no puede estar vacío");
        ui->nicknameLoginError->setVisible(false);
        //loginNicknameValid = false;
        //updateLoginButton();
        return;
    }

    User *user = Navigation::instance().findUser(username);

    if (user == nullptr) {
        ui->nicknameLoginError->setText("Este usuario no existe");
        ui->nicknameLoginError->setVisible(true);
        //updateLoginButton();
    } else {
        ui->nicknameLoginError->setVisible(false);
        //updateLoginButton();
    }
    //updateLoginButton();
}

void AuthWindow::onLoginPasswordEditingFinished()
{
    QString username = ui->loginUsernameEdit->text().trimmed();
    QString password = ui->loginPasswordEdit->text();

    if (password.isEmpty() || username.isEmpty()) {
        ui->passwordLoginError->setVisible(false);
        return;
    }

    User *user = Navigation::instance().authenticate(username, password);

    if (user == nullptr) {
        User *userExists = Navigation::instance().findUser(username);

        if (userExists != nullptr) {
            ui->passwordLoginError->setText("Contraseña incorrecta");
            ui->passwordLoginError->setVisible(true);
        } else {
            ui->passwordLoginError->setVisible(false);
        }
    } else {
        ui->passwordLoginError->setVisible(false);
    }
}

void AuthWindow::onNicknameEditingFinished()
{
    QString nickname = ui->registerNicknameEdit->text().trimmed();

    static const QRegularExpression regex("^[a-zA-Z0-9_-]{6,15}$");
    if (!regex.match(nickname).hasMatch()) {

        ui->nicknameRegisterError->setText("Usuario: 6-15 caracteres, letras, números, -, _");
        ui->nicknameRegisterError->setVisible(true);

        nicknameValid = false;
        qDebug() <<"nickname invalido";
        updateRegisterButton();
        return;
    }

    if (Navigation::instance().findUser(nickname) != nullptr) {

        ui->nicknameRegisterError->setText("Este usuario ya existe");
        ui->nicknameRegisterError->setVisible(true);

        nicknameValid = false;
        updateRegisterButton();
        return;
    }
    ui->nicknameRegisterError->setVisible(false);
    nicknameValid = true;
    updateRegisterButton();
}

void AuthWindow::onEmailEditingFinished()
{
    QString email = ui->registerEmailEdit->text().trimmed();

    if (!Utils::checkEmail(email)) {

        ui->emailRegisterError->setText("Formato de email inválido");
        ui->emailRegisterError->setVisible(true);
        //ui->registerEmailEdit->setFocus();
        emailValid = false;
        updateRegisterButton();
        return;
    }
    ui->emailRegisterError->setVisible(false);
    emailValid = true;
    updateRegisterButton();
}

void AuthWindow::onPasswordEditingFinished()
{
    QString password = ui->registerPasswordEdit->text();

    if (!Utils::checkPassword(password)) {
        ui->passwordRegisterError->setText("Debe tener 8-20 caracteres, mayúscula, minúscula, <br> número y carácter especial (!@#$%&*()-+=)");
        ui->passwordRegisterError->setVisible(true);
        passwordValid = false;
        updateRegisterButton();
        return;
    }
    ui->passwordRegisterError->setVisible(false);
    passwordValid = true;
    updateRegisterButton();
}

void AuthWindow::onPassword2EditingFinished()
{
    if (ui->registerPasswordEdit->text() != ui->registerPassword2Edit->text()) {
        ui->password2RegisterError->setText("Las contraseñas no coinciden.");
        ui->password2RegisterError->setVisible(true);

        password2Valid = false;
        updateRegisterButton();
        return;
    }
    ui->password2RegisterError->setVisible(false);
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
        updateRegisterButton();
    }

    updateRegisterButton();
}

void AuthWindow::updateRegisterButton()
{
    bool allValid = nicknameValid && emailValid && passwordValid &&
                    password2Valid && birthdateValid && avatarValid;

    ui->registerButton->setEnabled(allValid);
}

void AuthWindow::onToggleLoginPasswordVisibility()
{
    const bool isHidden = (ui->loginPasswordEdit->echoMode() == QLineEdit::Password);

    if (isHidden) {
        // Mostrar
        ui->loginPasswordEdit->setEchoMode(QLineEdit::Normal);
        ui->toggleLoginPasswordButton->setIcon(QIcon(":/new/prefix1/resources/icons/ojo.png"));
        ui->toggleLoginPasswordButton->setToolTip("Ocultar contraseña");
    } else {
        // Ocultar
        ui->loginPasswordEdit->setEchoMode(QLineEdit::Password);
        ui->toggleLoginPasswordButton->setIcon(QIcon(":/new/prefix1/resources/icons/invisible.png"));
        ui->toggleLoginPasswordButton->setToolTip("Mostrar contraseña");
    }
}

void AuthWindow::onToggleRegisterPasswordVisibility()
{
    const bool isHidden = (ui->registerPasswordEdit->echoMode() == QLineEdit::Password);

    if (isHidden) {
        // Mostrar
        ui->registerPasswordEdit->setEchoMode(QLineEdit::Normal);
        ui->toggleRegisterPasswordButton->setIcon(QIcon(":/new/prefix1/resources/icons/ojo.png"));
        ui->toggleRegisterPasswordButton->setToolTip("Ocultar contraseña");
    } else {
        // Ocultar
        ui->registerPasswordEdit->setEchoMode(QLineEdit::Password);
        ui->toggleRegisterPasswordButton->setIcon(QIcon(":/new/prefix1/resources/icons/invisible.png"));
        ui->toggleRegisterPasswordButton->setToolTip("Mostrar contraseña");
    }
}

void AuthWindow::onToggleRegisterPassword2Visibility()
{
    const bool isHidden = (ui->registerPassword2Edit->echoMode() == QLineEdit::Password);

    if (isHidden) {
        // Mostrar
        ui->registerPassword2Edit->setEchoMode(QLineEdit::Normal);
        ui->toggleRegisterPassword2Button->setIcon(QIcon(":/new/prefix1/resources/icons/ojo.png"));
        ui->toggleRegisterPassword2Button->setToolTip("Ocultar contraseña");
    } else {
        // Ocultar
        ui->registerPassword2Edit->setEchoMode(QLineEdit::Password);
        ui->toggleRegisterPassword2Button->setIcon(QIcon(":/new/prefix1/resources/icons/invisible.png"));
        ui->toggleRegisterPassword2Button->setToolTip("Mostrar contraseña");
    }
}

void AuthWindow::onNicknameTextChanged(const QString &text)
{
    static const QRegularExpression regex("^[a-zA-Z0-9_-]{6,15}$");

    // Si NO cumple formato → error
    if (!regex.match(text).hasMatch()) {
        ui->nicknameRegisterError->setText("Usuario: 6-15 caracteres, letras, números, -, _");
        ui->nicknameRegisterError->setVisible(true);
        nicknameValid = false;
        updateRegisterButton();
        return;
    }

    // Si existe en DB → error
    if (Navigation::instance().findUser(text) != nullptr) {
        ui->nicknameRegisterError->setText("Este usuario ya existe");
        ui->nicknameRegisterError->setVisible(true);
        nicknameValid = false;
        updateRegisterButton();
        return;
    }

    // Si es válido → ocultar error
    ui->nicknameRegisterError->setVisible(false);
    nicknameValid = true;
    updateRegisterButton();
}

void AuthWindow::onEmailTextChanged(const QString &text)
{
    if (!Utils::checkEmail(text)) {
        ui->emailRegisterError->setText("Formato de email inválido");
        ui->emailRegisterError->setVisible(true);
        emailValid = false;
        updateRegisterButton();
        return;
    }

    ui->emailRegisterError->setVisible(false);
    emailValid = true;
    updateRegisterButton();
}

void AuthWindow::onPasswordTextChanged(const QString &text)
{
    if (!Utils::checkPassword(text)) {
        ui->passwordRegisterError->setText(
            "Debe tener 8-20 caracteres, mayúscula, minúscula, número y carácter especial (!@#$%&*()-+=)"
            );
        ui->passwordRegisterError->setVisible(true);
        passwordValid = false;
        updateRegisterButton();
        return;
    }

    ui->passwordRegisterError->setVisible(false);
    passwordValid = true;
    updateRegisterButton();
}

void AuthWindow::onPassword2TextChanged(const QString &text)
{
    Q_UNUSED(text);

    if (ui->registerPasswordEdit->text() != ui->registerPassword2Edit->text()) {
        ui->password2RegisterError->setText("Las contraseñas no coinciden.");
        ui->password2RegisterError->setVisible(true);
        password2Valid = false;
        updateRegisterButton();
        return;
    }

    ui->password2RegisterError->setVisible(false);
    password2Valid = true;
    updateRegisterButton();
}

