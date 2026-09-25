#ifndef FUNCKCION_H
#define FUNCKCION_H

#include <QString>
#include <QStringList>
#include <QRegularExpression>

// Объявление функции: теперь это свободная функция, не метод класса

    uchar calcIntelHexChecksum(const QString &line);
    void checkLine(const QString &line);
    bool checkCrc(const QString &raw);
    QString generateAsciiString(const QString &dataHex);
    QStringList formatHexLineToTwoRows(const QString &line, int dataSize );

#endif // FUNCKCION_H
