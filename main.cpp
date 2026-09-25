#include "mainwindow.h"
#include <QApplication>

#include <QDebug>
#include <cstdio>

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    (void)type;          // <-- подавляет предупреждение
    (void)context;       // если и context не нужен — тоже так можно
    // toLocal8Bit() → CP1251 на русской Windows → то, что ждёт Qt Creator
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
    fflush(stderr);  // <-- критически важно: сбросить буфер немедленно
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageHandler);

    QApplication a(argc, argv);
    qWarning() << "Это файл main.cpp с поддержкой qDebug";

    // 1. Создаем шрифт с подсказкой типа, а не с жестким именем
    QFont appFont;
    appFont.setStyleHint(QFont::TypeWriter); // Подсказка: "дай мне моноширинный шрифт"
    appFont.setPointSize(14);

    // 2.Устанавливаем шрифт для всего приложения
    //   QFont appFont("Consolas", 14);
    a.setFont(appFont);
    qDebug() << "Поддержка шрифтов";


    MainWindow w;
    w.show();
    return a.exec();
}
