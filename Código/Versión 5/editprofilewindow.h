#ifndef EDITPROFILEWINDOW_H
#define EDITPROFILEWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QImage>
#include "navtypes.h"

class EditProfileWindow : public QDialog
{
    Q_OBJECT

public:
    explicit EditProfileWindow(User *user, QWidget *parent = nullptr);
    ~EditProfileWindow();

    bool wasModified() const { return m_wasModified; }

private slots:
    void onNicknameEditingFinished();
    void onEmailEditingFinished();
    void onPasswordEditingFinished();
    void onPassword2EditingFinished();
    void onBirthdateChanged();
    void onSelectAvatar(int index);
    void onSaveChanges();
    void onTogglePasswordVisibility();
    void onTogglePassword2Visibility();

private:
    User *m_user;

    QLineEdit *m_nicknameEdit;
    QLineEdit *m_emailEdit;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_password2Edit;
    QDateEdit *m_birthdateEdit;
    QComboBox *m_avatarComboBox;

    QPushButton *m_saveButton;
    QPushButton *m_cancelButton;
    QPushButton *m_togglePasswordButton;
    QPushButton *m_togglePassword2Button;

    QLabel *m_nicknameError;
    QLabel *m_emailError;
    QLabel *m_passwordError;
    QLabel *m_password2Error;

    bool m_nicknameValid;
    bool m_emailValid;
    bool m_passwordValid;
    bool m_password2Valid;
    bool m_birthdateValid;
    bool m_avatarValid;

    QString m_originalNickname;
    QString m_originalEmail;
    QDate m_originalBirthdate;
    QImage m_originalAvatar;
    QImage m_newAvatar;

    bool m_wasModified;

    void setupUI();
    void loadUserData();
    void updateSaveButton();
    bool hasChanges() const;
};

#endif
