#include "hexf.h"

#include <QString>
#include <QRegularExpression>

// Возвращает контрольную сумму в виде uchar (0x00..0xFF)
uchar calcIntelHexChecksum(const QString &line)
{
    QString clean = line.trimmed();
    if (clean.startsWith(':'))
        clean = clean.mid(1);

    clean.remove(QRegularExpression("[^0-9A-Fa-f]"));
    clean = clean.toUpper();

    if (clean.length() < 8)  // минимум: 2(count) + 4(addr) + 2(type) = 8, при byteCount=0
        return 0;


    // Читаем байт-каунт (первые 2 символа)
    bool ok;
    int byteCount = clean.left(2).toInt(&ok, 16);
    if (!ok)
        return 0;

    // Суммируем: 1(каунт) + 2(адрес) + 1(тип) + byteCount(данные) = 4 + byteCount байт
    // В hex-символах это (4 + byteCount) * 2
    int charsToSum = (4 + byteCount) * 2;

    int sum = 0;
    for (int i = 0; i < charsToSum && i + 1 < clean.length(); i += 2) {
        bool ok2;
        int byte = clean.mid(i, 2).toInt(&ok2, 16);
        if (ok2)
            sum += byte;
    }

    uchar checksum = static_cast<uchar>((0x100 - (sum & 0xFF)) & 0xFF);
    return checksum;
}



//*********** Проверка с выводом в статус-бар ******************
/*
void  checkLine(const QString &line)
 {
     QString hexPart = line.startsWith(':') ? line.mid(1) : line;

     QByteArray bytes;
     for (int i = 0; i + 1 < hexPart.length(); i += 2)
         bytes.append(hexPart.mid(i, 2).toUShort(nullptr, 16));

     uint8_t storedCrc = bytes.at(bytes.size() - 1);
     bytes.chop(1);

     uint8_t sum = 0;
     for (int i = 0; i < bytes.size(); ++i)
         sum += bytes[i];

     uint8_t calcCrc = 0x100 - sum;

     if (calcCrc == storedCrc)
         statusBar()->showMessage("Checksum: OK", 3000);
     else
         statusBar()->showMessage("Checksum: ERROR", 3000);
}
*/

//********  Проверка CRC ******************************
bool checkCrc(const QString &raw) {
    int sum = 0;
    int len = raw.length();
    if (len < 2) return false;

    for (int i = 0; i < len - 2; i += 2) {
        bool ok;
        int val = raw.mid(i, 2).toInt(&ok, 16);
        if (!ok) return false;
        sum += val;
    }

    int calc = (-(sum & 0xFF)) & 0xFF;
    bool ok;
    int crc = raw.mid(len - 2, 2).toInt(&ok, 16);
    return ok && (calc == crc);
}

//*********** Создание Ascii строки ***********************************
QString  generateAsciiString(const QString &dataHex)
{
    QString ascii;
    for (int i = 0; i < dataHex.length(); i += 2)
    {
        bool ok;
        int byteVal = dataHex.mid(i, 2).toInt(&ok, 16);
        if (!ok)
        {
            ascii += '.';
            continue;
        }
        if (byteVal > 32 && byteVal <= 126)
            ascii += QChar(byteVal);
        else
            ascii += '.';
    }
    return ascii;
}

//**************  Форматирование НЕХ строки  *******************************

QStringList formatHexLineToTwoRows(const QString &line, int dataSize )
{
    QStringList result;
    if (!line.startsWith(':'))
    {
        result << line;
        return result;
    }

    QString raw = line.mid(1).trimmed();// Строка без ':'
    if (raw.length() < 10 || raw.length() % 2 != 0)
    {
        result << line;
        return result;
    }

//checkLine(line);

    QString sLen  = raw.left(2);// Размер данных 16 или 32
    QString sAddr = raw.mid(2, 4);// Адресс
    QString sType = raw.mid(6, 2);// Тип строки

    bool ok;
    int dataLen = sLen.toInt(&ok, 16);// Перевод из QString sLen в int dataLen
    if (!ok || dataLen <= 0)
    {
        result << line;
        return result;
    }

    int expectedLen = 8 + dataLen * 2 + 2;//(sLen+sAddr+sType)=8 + длина данных*2 + 2 пробела
    if (raw.length() < expectedLen)
    {
        result << line;
        return result;
    }

    QString sData = raw.mid(8, dataLen * 2);// Изымаем данные
    QString sCrc  = raw.mid(8 + dataLen * 2, 2);// Изымаем CRC
    QString CrcBad  = "ER";

    if(dataSize == 32)
    {
  // Разбиваем данные на две половины (по 16 байт для 32-байтных строк)
        int halfLen = dataLen / 2;
        QString firstHalfHex  = sData.left(halfLen * 2);
        QString secondHalfHex = sData.mid(halfLen * 2);

        QString firstHalfAscii  = generateAsciiString(firstHalfHex);
        QString secondHalfAscii = generateAsciiString(secondHalfHex);

        // --- СТРОКА 1 ---
        // Формат: :XX XXXX XX  [HEX данные] [ASCII данные]
        QString hexRow1 = ":" + sLen + " " + sAddr + " " + sType + "  ";
        // Длина заголовка - :XX XXXX XX
        int LenHeading = hexRow1.length();
        // Добавляем HEX-данные с пробелами между байтами
        QString spacedFirst = "";
        for (int i = 0; i < firstHalfHex.length(); i += 2)
        {
            spacedFirst += firstHalfHex.mid(i, 2) + " ";
        }
        // trimmed() уберет последний лишний пробел после последнего байта
        hexRow1 += spacedFirst.trimmed();
        int LenRow1 = hexRow1.length();// Длина данных с пробелами
        int LenStangart = LenHeading + (dataSize/2)*3;
        int spaceCount = 5; // 1, 4, 8 — сколько нужно
        int LenCount = LenStangart - LenRow1 + spaceCount ;

        hexRow1 += QString(LenCount, ' ') + firstHalfAscii;

        // --- СТРОКА 2 ---
        // Формат: [13 пробелов] [HEX данные]  [CRC]  [ASCII данные]
        QString hexRow2 = "";
        QString spacedSecond = "";
        for (int i = 0; i < secondHalfHex.length(); i += 2) {
            spacedSecond += secondHalfHex.mid(i, 2) + " ";
        }
        // trimmed() уберет последний пробел после байтов
        hexRow2 += spacedSecond.trimmed();
        int LenRow2 = hexRow2.length();// Длина данных с пробелами

// qDebug() << "LenStangart"<< LenStangart;
// qDebug() << "LenRow1"<< LenRow2;

        spaceCount = 1;
        LenCount = LenStangart - LenRow2 - LenHeading + spaceCount ;

// qDebug() << "LenHeading"<< LenHeading;
// qDebug() << "LenCount"<< LenCount;



        // Проверка CRC
        if (!checkCrc(raw))
        {
            hexRow2 = QString(LenHeading, ' ') + hexRow2 + QString(LenCount, ' ') + CrcBad +
                    "  " + secondHalfAscii;
        }
        else hexRow2 = QString(LenHeading, ' ') + hexRow2 + QString(LenCount, ' ') + sCrc +
                "  " + secondHalfAscii;

        result << hexRow1;
        result << hexRow2;

   //*********************** Вывод в одну- СТРОКУ   **********************************
    }           // Формат: :XX XXXX XX  [HEX данные] [ASCII данные]
    else
    {


        if(dataSize != 16)
        {
            result << line;
            return result;
        }
        int halfLen = dataLen;
        QString HalfHex  = sData.left(halfLen * 2);
        QString HalfAscii  = generateAsciiString(HalfHex);

        QString hexRow = ":" + sLen + " " + sAddr + " " + sType + "  ";
        // Длина заголовка - :XX XXXX XX
        int LenHeading = hexRow.length();
        // Добавляем HEX-данные с пробелами между байтами
        QString spacedFirst = "";
        // Добавляем HEX-данные с пробелами между байтами
        spacedFirst = "";
        for (int i = 0; i < HalfHex.length(); i += 2)
        {
            spacedFirst += HalfHex.mid(i, 2) + " ";
        }
        // trimmed() уберет последний лишний пробел после последнего байта
        hexRow += spacedFirst.trimmed();
        int LenRow = hexRow.length();// Длина данных с пробелами
        int LenStangart = LenHeading + (dataSize)*3;
        int spaceCount = 1;

        int LenCount = LenStangart - LenRow + spaceCount ;

        hexRow += QString(LenCount, ' ');

        // Проверка CRC
        if (!checkCrc(raw))
        {
            hexRow += CrcBad + "  " + HalfAscii;
        }
        else hexRow += sCrc + "  " + HalfAscii;
        result << hexRow;
  }
    return result;
}







