#ifndef UTILS_H
#define UTILS_H

#include <QString>
class Utils
{
public:
    Utils();
    static bool checkEmail(const QString &email);
    static bool checkPassword(const QString &password);
};

#endif // UTILS_H
