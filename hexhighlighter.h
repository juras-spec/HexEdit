#ifndef HEXHIGHLIGHTER_H
#define HEXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class HexHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit HexHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    QColor colorLen;
    QColor colorAddr;
    QColor colorType;
    QColor colorData;
    QColor colorCrc;
    QColor colorError;
    QColor colorCrcBad;
};

#endif
