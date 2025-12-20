#include "utils.h"
#include <QRegularExpression>

Utils::Utils() {}

bool Utils::checkEmail(const QString &email)
{
    QRegularExpression regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return regex.match(email).hasMatch();
}

bool Utils::checkPassword(const QString &password)
{
    // Validar longitud: 8-20 caracteres
    if (password.length() < 8 || password.length() > 20) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    QString specialChars = "!@#$%&*()-+=";
    for (const QChar &c : password) {
        if (c.isUpper()) {
            hasUpper = true;
        } else if (c.isLower()) {
            hasLower = true;
        } else if (c.isDigit()) {
            hasDigit = true;
        } else if (specialChars.contains(c)) {
            hasSpecial = true;
        }
    }

    // Debe tener al menos una de cada tipo
    return hasUpper && hasLower && hasDigit && hasSpecial;
}
