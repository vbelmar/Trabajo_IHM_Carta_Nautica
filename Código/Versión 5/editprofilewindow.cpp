#include "editprofilewindow.h"
#include "utils.h"
#include "navigation.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QRegularExpression>

EditProfileWindow::EditProfileWindow(User *user, QWidget *parent)
    : QDialog(parent)
    , m_user(user)
    , m_nicknameValid(true)
    , m_emailValid(true)
    , m_passwordValid(true)
    , m_password2Valid(true)
    , m_birthdateValid(true)
    , m_avatarValid(true)
    , m_wasModified(false)
{
    setWindowTitle("Editar Perfil");
    setMinimumSize(550, 700);
    setObjectName("EditProfileWindow");

    setupUI();
    loadUserData();
}

EditProfileWindow::~EditProfileWindow()
{
}

void EditProfileWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 30, 40, 30);
    mainLayout->setSpacing(20);

    QLabel *titleLabel = new QLabel("Editar Perfil", this);
    titleLabel->setObjectName("editProfileTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QVBoxLayout *formLayout = new QVBoxLayout();
    formLayout->setSpacing(12);

    // Nickname (DESHABILITADO - no editable)
    QLabel *nicknameLabel = new QLabel("Usuario:", this);
    formLayout->addWidget(nicknameLabel);

    m_nicknameEdit = new QLineEdit(this);
    m_nicknameEdit->setReadOnly(true);
    m_nicknameEdit->setEnabled(false);
    formLayout->addWidget(m_nicknameEdit);

    m_nicknameError = new QLabel(this);
    m_nicknameError->setObjectName("nicknameError");
    m_nicknameError->setVisible(false);
    m_nicknameError->setMinimumHeight(15);
    formLayout->addWidget(m_nicknameError);

    // Email
    QLabel *emailLabel = new QLabel("Email:", this);
    formLayout->addWidget(emailLabel);

    m_emailEdit = new QLineEdit(this);
    formLayout->addWidget(m_emailEdit);

    m_emailError = new QLabel(this);
    m_emailError->setObjectName("emailError");
    m_emailError->setVisible(false);
    m_emailError->setMinimumHeight(15);
    formLayout->addWidget(m_emailError);

    // Password
    QLabel *passwordLabel = new QLabel("Nueva Contraseña:", this);
    formLayout->addWidget(passwordLabel);

    QHBoxLayout *passwordInputLayout = new QHBoxLayout();
    passwordInputLayout->setSpacing(5);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Dejar vacío para mantener actual");
    m_togglePasswordButton = new QPushButton(this);
    m_togglePasswordButton->setObjectName("togglePasswordButton");
    m_togglePasswordButton->setIcon(QIcon(":/new/prefix1/resources/icons/invisible.png"));
    m_togglePasswordButton->setFlat(true);
    m_togglePasswordButton->setFixedSize(30, 30);
    m_togglePasswordButton->setCursor(Qt::PointingHandCursor);
    passwordInputLayout->addWidget(m_passwordEdit);
    passwordInputLayout->addWidget(m_togglePasswordButton);
    formLayout->addLayout(passwordInputLayout);

    m_passwordError = new QLabel(this);
    m_passwordError->setObjectName("passwordError");
    m_passwordError->setVisible(false);
    m_passwordError->setMinimumHeight(15);
    m_passwordError->setWordWrap(true);
    formLayout->addWidget(m_passwordError);

    // Confirm Password
    QLabel *password2Label = new QLabel("Confirmar:", this);
    formLayout->addWidget(password2Label);

    QHBoxLayout *password2InputLayout = new QHBoxLayout();
    password2InputLayout->setSpacing(5);
    m_password2Edit = new QLineEdit(this);
    m_password2Edit->setEchoMode(QLineEdit::Password);
    m_password2Edit->setPlaceholderText("Confirmar nueva contraseña");
    m_togglePassword2Button = new QPushButton(this);
    m_togglePassword2Button->setObjectName("togglePassword2Button");
    m_togglePassword2Button->setIcon(QIcon(":/new/prefix1/resources/icons/invisible.png"));
    m_togglePassword2Button->setFlat(true);
    m_togglePassword2Button->setFixedSize(30, 30);
    m_togglePassword2Button->setCursor(Qt::PointingHandCursor);
    password2InputLayout->addWidget(m_password2Edit);
    password2InputLayout->addWidget(m_togglePassword2Button);
    formLayout->addLayout(password2InputLayout);

    m_password2Error = new QLabel(this);
    m_password2Error->setObjectName("password2Error");
    m_password2Error->setVisible(false);
    m_password2Error->setMinimumHeight(15);
    formLayout->addWidget(m_password2Error);

    // Birthdate
    QLabel *birthdateLabel = new QLabel("Fecha de Nacimiento:", this);
    formLayout->addWidget(birthdateLabel);

    m_birthdateEdit = new QDateEdit(this);
    m_birthdateEdit->setCalendarPopup(true);
    m_birthdateEdit->setMaximumDate(QDate::currentDate().addYears(-16));
    m_birthdateEdit->setDisplayFormat("dd/MM/yyyy");
    formLayout->addWidget(m_birthdateEdit);

    // Avatar - EXACTAMENTE COMO EN AUTHWINDOW
    QLabel *avatarLabel = new QLabel("Avatar:", this);
    formLayout->addWidget(avatarLabel);

    m_avatarComboBox = new QComboBox(this);
    m_avatarComboBox->setEditable(true);
    m_avatarComboBox->lineEdit()->setReadOnly(true);
    m_avatarComboBox->lineEdit()->setAlignment(Qt::AlignCenter);
    m_avatarComboBox->lineEdit()->clear();
    m_avatarComboBox->setCurrentIndex(-1);
    m_avatarComboBox->setIconSize(QSize(32, 32));
    m_avatarComboBox->setCursor(Qt::PointingHandCursor);

    m_avatarComboBox->setFixedWidth(80);
    m_avatarComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    // Añadir opción vacía primero
    m_avatarComboBox->addItem(QIcon(), "");
    m_avatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar1.png"), "");
    m_avatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar2.png"), "");
    m_avatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar3.png"), "");
    m_avatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar4.png"), "");
    m_avatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar5.png"), "");
    m_avatarComboBox->addItem(QIcon(":/new/prefix1/resources/icons/avatar6.png"), "");

    formLayout->addWidget(m_avatarComboBox);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    m_cancelButton = new QPushButton("Cancelar", this);
    m_cancelButton->setObjectName("cancelButton");
    m_cancelButton->setCursor(Qt::PointingHandCursor);

    m_saveButton = new QPushButton("Guardar Cambios", this);
    m_saveButton->setObjectName("saveButton");
    m_saveButton->setEnabled(false);
    m_saveButton->setCursor(Qt::PointingHandCursor);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(m_emailEdit, &QLineEdit::editingFinished, this, &EditProfileWindow::onEmailEditingFinished);
    connect(m_passwordEdit, &QLineEdit::editingFinished, this, &EditProfileWindow::onPasswordEditingFinished);
    connect(m_password2Edit, &QLineEdit::editingFinished, this, &EditProfileWindow::onPassword2EditingFinished);
    connect(m_birthdateEdit, &QDateEdit::dateChanged, this, &EditProfileWindow::onBirthdateChanged);
    connect(m_avatarComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditProfileWindow::onSelectAvatar);

    connect(m_togglePasswordButton, &QPushButton::clicked, this, &EditProfileWindow::onTogglePasswordVisibility);
    connect(m_togglePassword2Button, &QPushButton::clicked, this, &EditProfileWindow::onTogglePassword2Visibility);

    connect(m_saveButton, &QPushButton::clicked, this, &EditProfileWindow::onSaveChanges);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    connect(m_emailEdit, &QLineEdit::textChanged, this, &EditProfileWindow::updateSaveButton);
    connect(m_passwordEdit, &QLineEdit::textChanged, this, &EditProfileWindow::updateSaveButton);
    connect(m_password2Edit, &QLineEdit::textChanged, this, &EditProfileWindow::updateSaveButton);
}

void EditProfileWindow::loadUserData()
{
    if (!m_user) return;

    m_originalNickname = m_user->nickName();
    m_originalEmail = m_user->email();
    m_originalBirthdate = m_user->birthdate();
    m_originalAvatar = m_user->avatar();

    m_nicknameEdit->setText(m_originalNickname);
    m_emailEdit->setText(m_originalEmail);
    m_birthdateEdit->setDate(m_originalBirthdate);

    // Find matching avatar - ajustar índice porque el 0 es vacío
    for (int i = 1; i <= 6; ++i) {
        QString avatarPath = QString(":/new/prefix1/resources/icons/avatar%1.png").arg(i);
        QImage testAvatar;
        testAvatar.load(avatarPath);
        if (testAvatar == m_originalAvatar) {
            m_avatarComboBox->setCurrentIndex(i);
            break;
        }
    }

    m_newAvatar = m_originalAvatar;
}

void EditProfileWindow::onNicknameEditingFinished()
{
    // No hace nada - el nickname no se puede editar
}

void EditProfileWindow::onEmailEditingFinished()
{
    QString email = m_emailEdit->text().trimmed();

    if (!Utils::checkEmail(email)) {
        m_emailError->setText("Formato de email inválido");
        m_emailError->setVisible(true);
        m_emailValid = false;
        updateSaveButton();
        return;
    }

    m_emailError->setVisible(false);
    m_emailValid = true;
    updateSaveButton();
}

void EditProfileWindow::onPasswordEditingFinished()
{
    QString password = m_passwordEdit->text();

    if (password.isEmpty()) {
        m_passwordError->setVisible(false);
        m_passwordValid = true;
        m_password2Valid = true;
        updateSaveButton();
        return;
    }

    if (!Utils::checkPassword(password)) {
        m_passwordError->setText("Debe tener 8-20 caracteres, mayúscula, minúscula, número y carácter especial (!@#$%&*()-+=)");
        m_passwordError->setVisible(true);
        m_passwordValid = false;
        updateSaveButton();
        return;
    }

    m_passwordError->setVisible(false);
    m_passwordValid = true;
    updateSaveButton();
}

void EditProfileWindow::onPassword2EditingFinished()
{
    if (m_passwordEdit->text().isEmpty()) {
        m_password2Error->setVisible(false);
        m_password2Valid = true;
        updateSaveButton();
        return;
    }

    if (m_passwordEdit->text() != m_password2Edit->text()) {
        m_password2Error->setText("Las contraseñas no coinciden.");
        m_password2Error->setVisible(true);
        m_password2Valid = false;
        updateSaveButton();
        return;
    }

    m_password2Error->setVisible(false);
    m_password2Valid = true;
    updateSaveButton();
}

void EditProfileWindow::onBirthdateChanged()
{
    QDate birthdate = m_birthdateEdit->date();
    int age = birthdate.daysTo(QDate::currentDate()) / 365;

    if (age < 16) {
        QMessageBox::warning(this, "Error en Fecha", "Debes tener al menos 16 años.");
        m_birthdateEdit->setDate(m_originalBirthdate);
        m_birthdateValid = false;
    } else {
        m_birthdateValid = true;
        updateSaveButton();
    }
}

void EditProfileWindow::onSelectAvatar(int index)
{
    // El índice 0 es la opción vacía, los avatares válidos empiezan en 1
    if (index <= 0) {
        m_avatarValid = false;
        m_newAvatar = QImage(100, 100, QImage::Format_RGB32);
        m_newAvatar.fill(QColor(200, 200, 200));
        updateSaveButton();
        return;
    }

    QString avatarPath = QString(":/new/prefix1/resources/icons/avatar%1.png").arg(index);
    m_newAvatar.load(avatarPath);

    if (m_newAvatar.isNull()) {
        m_newAvatar = QImage(100, 100, QImage::Format_RGB32);
        m_newAvatar.fill(QColor(200, 200, 200));
        QMessageBox::warning(this, "Error", "No se pudo cargar la imagen del avatar.");
        m_avatarValid = false;
    } else {
        m_avatarValid = true;
    }

    updateSaveButton();
}

void EditProfileWindow::onSaveChanges()
{
    if (!m_user) return;

    try {
        QString newEmail = m_emailEdit->text().trimmed();
        QString newPassword = m_passwordEdit->text();
        QDate newBirthdate = m_birthdateEdit->date();

        QString currentPassword = m_user->password();
        QString finalPassword = newPassword.isEmpty() ? currentPassword : newPassword;

        bool hasChanges = (newEmail != m_originalEmail) ||
                          (!newPassword.isEmpty()) ||
                          (newBirthdate != m_originalBirthdate) ||
                          (m_newAvatar != m_originalAvatar);

        if (!hasChanges) {
            QMessageBox::information(this, "Sin cambios", "No se han detectado cambios en el perfil.");
            return;
        }

        User updatedUser(
            m_originalNickname,
            newEmail,
            finalPassword,
            m_newAvatar,
            newBirthdate
            );

        updatedUser.setSessions(m_user->sessions());
        updatedUser.setInsertedInDb(true);

        Navigation::instance().updateUser(updatedUser);

        m_wasModified = true;
        QMessageBox::information(this, "Éxito", "Perfil actualizado correctamente.");
        accept();

    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error",
                              QString("Error al actualizar perfil:\n%1").arg(e.what()));
    }
}

void EditProfileWindow::onTogglePasswordVisibility()
{
    const bool isHidden = (m_passwordEdit->echoMode() == QLineEdit::Password);

    if (isHidden) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        m_togglePasswordButton->setIcon(QIcon(":/new/prefix1/resources/icons/ojo.png"));
    } else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_togglePasswordButton->setIcon(QIcon(":/new/prefix1/resources/icons/invisible.png"));
    }
}

void EditProfileWindow::onTogglePassword2Visibility()
{
    const bool isHidden = (m_password2Edit->echoMode() == QLineEdit::Password);

    if (isHidden) {
        m_password2Edit->setEchoMode(QLineEdit::Normal);
        m_togglePassword2Button->setIcon(QIcon(":/new/prefix1/resources/icons/ojo.png"));
    } else {
        m_password2Edit->setEchoMode(QLineEdit::Password);
        m_togglePassword2Button->setIcon(QIcon(":/new/prefix1/resources/icons/invisible.png"));
    }
}

void EditProfileWindow::updateSaveButton()
{
    bool allValid = m_nicknameValid && m_emailValid && m_passwordValid &&
                    m_password2Valid && m_birthdateValid && m_avatarValid;

    bool changed = hasChanges();

    m_saveButton->setEnabled(allValid && changed);
}

bool EditProfileWindow::hasChanges() const
{
    if (m_emailEdit->text().trimmed() != m_originalEmail) return true;
    if (!m_passwordEdit->text().isEmpty()) return true;
    if (m_birthdateEdit->date() != m_originalBirthdate) return true;
    if (m_newAvatar != m_originalAvatar) return true;

    return false;
}
