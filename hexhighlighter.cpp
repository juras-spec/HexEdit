#include "hexhighlighter.h"
#include <QTextCharFormat>
#include <QSyntaxHighlighter>
#include <QColor>

#include <QDebug>

HexHighlighter::HexHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    colorLen    = QColor("#E06C75");  // розовый — длина
    colorAddr   = QColor("#61AFEF");  // голубой — адрес
    colorType   = QColor("#C678DD");  // фиолетовый — тип
    colorData   = QColor("#2E8B57");  // SeaGreen Морской зеленый— данные
    colorCrc    = QColor("#800000");  // Maroon Темно-бордовый — — CRC
    colorError  = QColor("#FF00FF");  // Magenta Пурпурный  — ошибка
    colorCrcBad = QColor("#FF00FF");  // Magenta Пурпурный — CRC не сошёлся
}

void HexHighlighter::highlightBlock(const QString &text)
{
    if (text.isEmpty()) {
        setCurrentBlockState(0);
        return;
    }

    // Вспомогательная лямбда для проверки HEX-символа
    auto isHexChar = [](QChar c) -> bool
    {
        return c.isDigit() || (c.toLower() >= QChar('a') && c.toLower() <= QChar('f'));
    };

    // ============ ПЕРВАЯ СТРОКА (с двоеточием) ============
    if (text[0] == ':')
    {
        bool crcBad = text.contains("ER");
        int dataLenBytes = text.mid(1, 2).toInt(nullptr, 16);

        if (dataLenBytes < 0 || dataLenBytes > 255)
        {
            QTextCharFormat fmt;
            fmt.setForeground(Qt::gray);
            setFormat(0, text.length(), fmt);
            setCurrentBlockState(0);
            return;
        }

        int pos = 1;

        // Длина (LL)
        setFormat(pos, 2, colorLen);
        pos += 2;
        while (pos < text.length() && text[pos] == ' ') pos++;

        // Адрес (AAAA)
        int addrLen = qMin(4, text.length() - pos);
        setFormat(pos, addrLen, colorAddr);
        pos += addrLen;
        while (pos < text.length() && text[pos] == ' ') pos++;

        // Тип (TT)
        int typeLen = qMin(2, text.length() - pos);
        setFormat(pos, typeLen, colorType);
        pos += typeLen;
        while (pos < text.length() && text[pos] == ' ') pos++;

        // Данные: красим сколько есть в этой строке
        int bytesPainted = 0;
        while (pos + 1 < text.length() && bytesPainted < dataLenBytes)
        {
            if (!isHexChar(text[pos])) break;

            setFormat(pos, 2, colorData);
            pos += 2;
            bytesPainted++;

            // Пропускаем пробел после байта
            if (pos < text.length() && text[pos] == ' ') pos++;
        }

        // Считаем сколько байт осталось для второй строки
        int remaining = dataLenBytes - bytesPainted;

        // Если remaining == 0, значит CRC тоже в этой строке
        if (remaining == 0)
        {
            while (pos < text.length() && text[pos] == ' ') pos++;
            if (pos + 1 < text.length())
            {
                QTextCharFormat fmtCrc;
                if (crcBad)
                {

                    fmtCrc.setForeground(colorCrcBad);
                    fmtCrc.setFontWeight(QFont::Bold);
                }
                else
                {
                    fmtCrc.setForeground(colorCrc);
                }
                setFormat(pos, 2, fmtCrc);

                // Суффикс "ER"
                if (crcBad && text.mid(pos + 2, 2) == "ER")
                {
                    setFormat(pos + 2, 4, fmtCrc);
                }
            }
        }

        // СОХРАНЯЕМ остаток для следующей строки
        setCurrentBlockState(remaining);
        return;
    }

    // ============ ВТОРАЯ СТРОКА (без двоеточия) ============
    else
    {
        // Читаем состояние ПРЕДЫДУЩЕЙ строки (а не текущей!)
        QTextBlock prevBlock = currentBlock().previous();
        int remaining = -1;
        if (prevBlock.isValid())
        {
            remaining = prevBlock.userState();
        }

        if (remaining <= 0)
        {
            QTextCharFormat fmt;
            fmt.setForeground(Qt::gray);
            setFormat(0, text.length(), fmt);
            setCurrentBlockState(0);
            return;
        }
        bool crcBad = text.contains("ER");
        int pos = 0;
        while (pos < text.length() && text[pos] == ' ') pos++;
        int bytesPainted = 0;
        while (pos + 1 < text.length() && bytesPainted < remaining)
        {
            QChar c = text[pos];
            if (!c.isDigit() && !(c.toLower() >= QChar('a') && c.toLower() <= QChar('f')))
                break;

            setFormat(pos, 2, colorData);
            pos += 2;
            bytesPainted++;

            if (pos < text.length() && text[pos] == ' ') pos++;
        }

        while (pos < text.length() && text[pos] == ' ') pos++;

        if (pos + 1 < text.length())
        {
            QTextCharFormat fmtCrc;
            if (crcBad)
            {
                fmtCrc.setForeground(colorCrcBad);
                fmtCrc.setFontWeight(QFont::Bold);
            }
            else
            {
                fmtCrc.setForeground(colorCrc);
            }
            setFormat(pos, 2, fmtCrc);

            if (crcBad && text.mid(pos + 2, 2) == "ER")
            {
                setFormat(pos + 2, 4, fmtCrc);
            }
        }

        setCurrentBlockState(0);
        return;
    }
}
