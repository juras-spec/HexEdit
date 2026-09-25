#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "hexf.h"

#include <QMessageBox>

#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
//************************
#include <QFont>
#include <QFontDialog>
#include <QFontInfo>
//************************

//#include <QString>
//#include <QStringList>
//#include <QRegularExpression>
#include <QLabel>
#include <QPlainTextEdit>
//************************
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //***** Фиксированный размкр окна *******************************
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);

    //***** Запркт переноса строк ***********************************
    ui->browser->setWordWrapMode(QTextOption::NoWrap);

    highlighter = new HexHighlighter(ui->browser->document());

    m_currentFilePath = "";
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --- Исправленная функция: парсит по токенам, игнорирует ASCII-дамп ---



//***************** Open File ****************************

void MainWindow::on_actionOpen_triggered()
{
    {
        QString path = QFileDialog::getOpenFileName(
            this, "Открыть HEX-файл", QString(), "HEX (*.hex);;Все файлы (*)");

        if (path.isEmpty()) return;

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            ui->browser->setPlainText("Не удалось открыть файл: " + path);
            return;
        }

        QFileInfo fi(path);
        QTextStream in(&file);
        QString allText;
        int lineCount = 0;
        bool formatDetected = false;
        QString line;
        while (!in.atEnd())
        {
            line = in.readLine().trimmed();
            if (line.isEmpty()) continue;

            if (!formatDetected && line.startsWith(':') && line.length() >= 3)
            {
                QString lenHex = line.mid(1, 2);
                bool ok = false;
                len = lenHex.toInt(&ok, 16);
                dataSize = len;
                if (ok && len > 0)
                {
                   formatDetected = true;
                }
            }

            QStringList rows = formatHexLineToTwoRows(line, dataSize);
            foreach (const QString &row, rows)
            {
                allText += row + "\n";
                lineCount++;
//qWarning() << "lineCount:"<< lineCount;
            }
        }

        ui->browser->setPlainText(allText);

        qint64 sz = fi.size();
        QString sizeStr = QString("%1 байт").arg(sz);

        QString statusText = QString("Файл: %1 | Размер: %2 | Строк: %3")
                .arg(fi.fileName())
                .arg(sizeStr)
                .arg(lineCount);
        //ui->statusLine->setText(statusText);

        // 0 — показывать бесконечно, или поставь мс (например, 3000)
        statusBar()->showMessage(statusText, 0);
    }

}

//************ Ихменение шрифта *************************************
void MainWindow::on_actionFont_triggered()
{
        QFont font;
        // Шаг А: Берем шрифт, который мы только что установили на окно
        QFont currentWindowFont = this->font();

        // Шаг Б: Создаем QFontInfo на основе этого шрифта
        QFontInfo fontInfo(currentWindowFont);

        // Шаг В: Теперь можно безопасно брать и имя, и размер
        QString familyName = fontInfo.family();      // Реальное имя (Courier New и т.д.)
        int size = fontInfo.pointSize();             // Размер (14)

        qDebug() << "Реальное имя:" << familyName << "Size:" << size;

        bool ok;
        font = QFontDialog::getFont(&ok, currentWindowFont, this);//QFont("Consolas", 14)
        if (ok)
        {
            // применить выбранный шрифт, например, к QTextEdit:
            this->setFont(font);
        }

        QFontInfo fontIn(this->font());
        // 3. Формируем и выводим строку
        QString statusText = QString("Имя:%1 Размер:%2")
                             .arg(fontIn.family())
                             .arg(fontIn.pointSize());

       // ui->statusLine->setText(statusText);

        statusBar()->showMessage(statusText, 0);  // 0 — показывать бесконечно, или поставь мс (например, 3000)

}

//**********************************************************************
void MainWindow::on_actionSave_triggered()
{

        QString text = ui->browser->toPlainText();
        if (text.isEmpty()) {
            return;
        }

        // Если путь еще не задан, ведем себя как "Сохранить как"
        if (m_currentFilePath.isEmpty()) {
            on_actionSave_as_triggered();
            return;
        }

        // Пытаемся сохранить по уже известному пути
        QFile file(m_currentFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << text;
            file.close();

            qDebug() << "Файл перезаписан:" << m_currentFilePath;
            // Здесь можно добавить статус-бар: ui->statusBar->showMessage("Сохранено", 2000);
        } else {
            // Если файл заблокирован или нет прав, предлагаем выбрать другой путь
            QMessageBox::warning(this, tr("Ошибка доступа"),
                tr("Не удалось сохранить файл по пути:\n%1\n\nПопробуйте 'Сохранить как...'").arg(m_currentFilePath));
            on_actionSave_as_triggered();
        }
}

//****************************************************************************************

void MainWindow::on_actionSave_as_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Сохранить HEX"), m_currentFilePath, tr("HEX Files (*.hex)")
    );
    if (fileName.isEmpty())
        return;

    QFileInfo fi(fileName);
    if (fi.suffix().isEmpty())
        fileName += ".hex";
    m_currentFilePath = fileName;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Ошибка"), tr("Не удалось сохранить файл"));
        return;
    }
    QTextStream out(&file);

    QTextDocument *doc = ui->browser->document();
    int lines = doc->blockCount();
    qDebug() << "Всего строк в документе:" << lines;

    for (int i = 0; i < lines; i++)
    {
        QTextBlock block = doc->findBlockByNumber(i);
        if (!block.isValid()) continue;

        // Убираем все пробелы
        QString line = block.text().remove(QRegularExpression("\\s+"));

        // Строка должна начинаться с ':'
        if (!line.startsWith(':'))
            continue;

        // Читаем байт-каунт (символы 1-2 после ':')
        bool ok = false;
        int len = line.mid(1, 2).toInt(&ok, 16);
        if (!ok) continue;

        // Формируем строку БЕЗ контрольной суммы и БЕЗ ':'
        int strformat = 2 + 4 + 2 + len * 2;  // count + addr + type + data (в hex-символах)
        QString dataPart = line.mid(1, strformat);  // "100000000A128A11E0..."

        // Считаем контрольную сумму от dataPart (без двоеточия!)
        uchar cs = calcIntelHexChecksum(dataPart);
        QString hexCs = QString("%1").arg(cs, 2, 16, QChar('0')).toUpper();

        // Собираем итоговую строку: ':' + данные + контрольная сумма
        QString fullLine = ":" + dataPart + hexCs;

        qDebug() << "hexCs" << hexCs << "fullLine" << fullLine;

        out << fullLine << "\r\n";
    }

    file.close();
    qDebug() << "Файл сохранён:" << fileName;
}
