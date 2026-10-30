#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QStack>
#include <QString>

class QKeyEvent;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // 正在输入的操作数
    QString operand;
    // 最近一次按下的运算符（用于表达式显示）
    QString opcode;
    QStack<QString> opcodes;
    QStack<QString> operands;

    // 输入状态：鼠标按键和键盘输入共用同一套状态机
    enum CalcState {
        InputState,     // 正在输入操作数
        OperatorState,  // 刚按下二元运算符，等待第二个操作数
        ResultState,    // 刚按下“=”，等待新的算式
        ErrorState      // 出错（如除数为0），必须先清除才能继续
    };

    QString calculation(bool *ok = NULL);

protected:
    // 键盘事件：所有按键都转换成与鼠标按钮相同的处理函数
    void keyPressEvent(QKeyEvent *event);

private slots:
    // ---- 鼠标按键对应的槽函数（只负责取出按钮文本，再转交给共用函数） ----
    void btnNumClicked();
    void btnBinaryOperatorClicked();
    void btnUnaryOperaterClicked();

    void on_btnPeriod_clicked();
    void on_btnDel_clicked();
    void on_btnClear_clicked();
    void on_btnClearAll_clicked();
    void on_btnEqual_clicked();
    void on_btnZF_clicked();

private:
    // ---- 鼠标与键盘共用的处理逻辑 ----
    void inputDigit(const QString &digit);      // 数字
    void inputPeriod();                         // 小数点
    void backspace();                           // 退格
    void clearEntry();                          // CE：清除当前输入
    void clearAll();                            // C：清除全部
    void toggleSign();                          // ±：正负号
    void inputBinaryOperator(const QString &op);// + - × ÷
    void inputUnaryOperator(const QString &op); // % 1/x x^2 √
    void inputEqual();                          // =

    // ---- 界面辅助函数 ----
    void loadStyleSheet();                      // 加载 qss 样式表
    void updateDisplay(const QString &text);    // 刷新显示框
    void updateExpression();                    // 刷新表达式提示
    void showError(const QString &message);     // 错误提示

    CalcState state;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
