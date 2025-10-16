#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <algorithm>
#include <cmath>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextBrowser>


const QStringList baseNames = {"Binary", "Decimal", "Octal", "Hexadecimal", "Base36"};
const QString digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void MainWindow::onHelpClicked() {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Інструкція користувача");

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    QTextBrowser* browser = new QTextBrowser(dialog);

    QFile file("інструкція.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        browser->setHtml(in.readAll());
        file.close();
    } else {
        browser->setText("Не вдалося відкрити інструкція.html");
    }

    layout->addWidget(browser);
    dialog->setLayout(layout);
    dialog->resize(600, 400);
    dialog->exec();

}


int baseFromName(const QString& name) {
    if (name == "Binary") return 2;
    if (name == "Decimal") return 10;
    if (name == "Octal") return 8;
    if (name == "Hexadecimal") return 16;
    if (name == "Base36") return 36;
    return -1;
}

bool isValidForBase(const QString &number, int base) {
    QString allowed = digits.left(base);  // універсально для будь-якої бази <= 36

    bool dotFound = false;
    for (QChar c : number.toUpper()) {  // до upper для порівняння
        if (c == '.') {
            if (dotFound) return false;
            dotFound = true;
            continue;
        }
        if (!allowed.contains(c)) return false;
    }
    return true;
}

double fromBaseFractional(const QString& number, int baseFrom) {
    QString upper = number.toUpper();
    QStringList parts = upper.split('.');
    long long intPart = 0;
    double fracPart = 0.0;

    for (QChar ch : parts[0]) {
        int val = digits.indexOf(ch);
        if (val == -1 || val >= baseFrom)
            throw std::invalid_argument("Invalid digit in integer part.");
        intPart = intPart * baseFrom + val;
    }

    if (parts.size() > 1) {
        QString frac = parts[1];
        for (int i = 0; i < frac.size(); ++i) {
            int val = digits.indexOf(frac[i]);
            if (val == -1 || val >= baseFrom)
                throw std::invalid_argument("Invalid digit in fractional part.");
            fracPart += val / pow(baseFrom, i + 1);
        }
    }

    return intPart + fracPart;
}

QString toBaseFractional(double number, int baseTo, int precision = 6) {
    if (number == 0.0) return "0";
    long long intPart = static_cast<long long>(number);
    double fracPart = number - intPart;

    QString result;
    // integer part
    if (intPart == 0) result = "0";
    else {
        while (intPart > 0) {
            result.prepend(digits[intPart % baseTo]);
            intPart /= baseTo;
        }
    }


    if (fracPart > 0.0) {
        result += '.';
        for (int i = 0; i < precision && fracPart > 0.0; ++i) {
            fracPart *= baseTo;
            int digit = static_cast<int>(fracPart);
            result += digits[digit];
            fracPart -= digit;
        }
    }
    return result;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->from->addItems(baseNames);
    ui->to->addItems(baseNames);

    connect(ui->CONVERT, &QPushButton::clicked, this, &MainWindow::onConvertClicked);
    connect(ui->COPY, &QPushButton::clicked, this, &MainWindow::onCopyClicked);
    connect(ui->FILE_INPUT, &QPushButton::clicked, this, &MainWindow::onFileInputClicked);
    connect(ui->FILE_SAVE, &QPushButton::clicked, this, &MainWindow::onFileSaveClicked);
    connect(ui->RESET, &QPushButton::clicked, this, &MainWindow::onResetClicked);
    connect(ui->SWAP, &QPushButton::clicked, this, &MainWindow::onSwapClicked);
    connect(ui->instruction, &QPushButton::clicked, this, &MainWindow::onHelpClicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onConvertClicked() {
    QString input = ui->inputnumber->text().trimmed();
    if (input.isEmpty()) return;

    int sourceBase = baseFromName(ui->from->currentText());
    int targetBase = baseFromName(ui->to->currentText());

    QStringList inputs = input.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    QStringList results;

    for (const QString& num : inputs) {
        if (!isValidForBase(num, sourceBase)) {
            QMessageBox::warning(this, "Invalid Input", QString("Invalid digit in number '%1' for base %2").arg(num).arg(sourceBase));
            return;
        }

        try {
            double value = fromBaseFractional(num, sourceBase);
            QString converted = toBaseFractional(value, targetBase);
            results << converted;
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Conversion Error", QString("Error converting '%1': %2").arg(num).arg(e.what()));
            return;
        }
    }

    ui->result_line->setText(results.join(" "));
}

void MainWindow::onCopyClicked() {
    QApplication::clipboard()->setText(ui->result_line->text());
}

void MainWindow::onFileInputClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "Text Files (*.txt)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QStringList numbers;
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (!line.isEmpty())
                    numbers.append(line);
            }
            file.close();
            ui->inputnumber->setText(numbers.join(" "));
        }
    }
}

void MainWindow::onFileSaveClicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Result", "", "Text Files (*.txt)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << ui->result_line->text();
            file.close();
        } else {
            QMessageBox::warning(this, "Error", "Cannot save file");
        }
    }
}

void MainWindow::onResetClicked() {
    ui->inputnumber->clear();
    ui->result_line->clear();
    ui->from->setCurrentIndex(0);
    ui->to->setCurrentIndex(0);
}

void MainWindow::onSwapClicked() {
    int fromIndex = ui->from->currentIndex();
    int toIndex = ui->to->currentIndex();
    ui->from->setCurrentIndex(toIndex);
    ui->to->setCurrentIndex(fromIndex);
}
