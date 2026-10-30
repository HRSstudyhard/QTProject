#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QLabel>
#include<QStack>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString operand;
    QString opcode;
    QStack<QString>opcodes;
    QStack<QString>operands;

    QString calculation(bool *ok=NULL);

private slots:

    void btnNumClicked();

    void on_btnPeriod_clicked();

    void on_btnDel_clicked();

    void on_btnClear_clicked();

    void on_btnClearAll_clicked();


    void on_btnEqual_clicked();

    void btnBinaryOperatorClicked();
    void btnUnaryOperaterClicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
