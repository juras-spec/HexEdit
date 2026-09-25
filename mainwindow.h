#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "hexhighlighter.h"
//#include "function.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:


    void on_actionOpen_triggered();

    void on_actionFont_triggered();

    void on_actionSave_triggered();

    void on_actionSave_as_triggered();

private:
    Ui::MainWindow *ui;

    HexHighlighter *highlighter;
    int len = 0;
    int dataSize = 0;
    QString m_currentFilePath; // Хранит путь к текущему файлу


//    uchar calcIntelHexChecksum(const QString &line);
//    void checkLine(const QString &line);
//    bool checkCrc(const QString &raw);
//    QString generateAsciiString(const QString &dataHex);
//    QStringList formatHexLineToTwoRows(const QString &line, int dataSize );


};
#endif // MAINWINDOW_H
