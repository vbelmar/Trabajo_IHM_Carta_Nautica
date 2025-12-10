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
    void onCancelClicked();

    void onRegisterClicked();
    void onBackToLogin();

    void onNicknameEditingFinished();
    void onEmailEditingFinished();
    void onPasswordEditingFinished();
    void onPassword2EditingFinished();
    void onBirthdateChanged();
    void onSelectAvatar();
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::AuthWindow *ui;

    bool nicknameValid;
    bool emailValid;
    bool passwordValid;
    bool password2Valid;
    bool birthdateValid;

    QImage avatarImage;
    QString m_loggedUsername;

    void updateRegisterButton();
};

#endif // AUTHWINDOW_H
