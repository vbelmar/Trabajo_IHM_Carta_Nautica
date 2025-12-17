#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QDialog>
#include <QImage>

namespace Ui{
class AuthWindow;
}

class AuthWindow : public QDialog{
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

    QString getLoggedUsername() const { return m_loggedUsername;}

private slots:
    void onLoginClicked();
    void onGoToRegister();

    void onRegisterClicked();
    void onBackToLogin();

    void onLoginUsernameEditingFinished();
    void onLoginPasswordEditingFinished();

    void onNicknameEditingFinished();
    void onEmailEditingFinished();
    void onPasswordEditingFinished();
    void onPassword2EditingFinished();
    void onBirthdateChanged();
    void onSelectAvatar(int index);

    void onToggleLoginPasswordVisibility();
    void onToggleRegisterPasswordVisibility();
    void onToggleRegisterPassword2Visibility();

    void onNicknameTextChanged(const QString &text);
    void onEmailTextChanged(const QString &text);
    void onPasswordTextChanged(const QString &text);
    void onPassword2TextChanged(const QString &text);


protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::AuthWindow *ui;

    bool nicknameValid;
    bool emailValid;
    bool passwordValid;
    bool password2Valid;
    bool birthdateValid;
    bool avatarValid;

    QImage avatarImage;
    QString m_loggedUsername;

    void updateRegisterButton();
    void updateLoginButton();
};

#endif // AUTHWINDOW_H
