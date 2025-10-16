#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConvertClicked();
    void onCopyClicked();
    void onFileInputClicked();
    void onFileSaveClicked();
    void onResetClicked();
    void onSwapClicked();
    void onHelpClicked();

private:
    Ui::MainWindow *ui;

    QString convertNumber(const QString &numberStr, const QString &fromBase, const QString &toBase);
};

#endif // MAINWINDOW_H
