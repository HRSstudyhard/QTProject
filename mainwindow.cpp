#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QKeyEvent>
#include <QPushButton>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ---------- 数字键 ----------
    connect(ui->btnNum0, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->btnNum1, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->btnNum2, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->btnNum3, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->btnNum4, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->btnNum5, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->btnNum6, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->btnNum7, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->btnNum8, SIGNAL(clicked()), this, SLOT(btnNumClicked()));
    connect(ui->btnNum9, SIGNAL(clicked()), this, SLOT(btnNumClicked()));

    // ---------- 二元运算符 ----------
    connect(ui->btnPlus, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnMinus, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnMultiple, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnDivide, SIGNAL(clicked()), this, SLOT(btnBinaryOperatorClicked()));

    // ---------- 一元运算符 ----------
    connect(ui->btnPercentage, SIGNAL(clicked()), this, SLOT(btnUnaryOperaterClicked()));
    connect(ui->btnInverse, SIGNAL(clicked()), this, SLOT(btnUnaryOperaterClicked()));
    connect(ui->btnSquare, SIGNAL(clicked()), this, SLOT(btnUnaryOperaterClicked()));
    connect(ui->btnSqrt, SIGNAL(clicked()), this, SLOT(btnUnaryOperaterClicked()));

    // 注意：on_btnPeriod_clicked()、on_btnDel_clicked()、on_btnClear_clicked()、
    // on_btnClearAll_clicked()、on_btnEqual_clicked()、on_btnZF_clicked() 的命名
    // 符合 on_<对象名>_<信号名> 规则，ui->setupUi() 内部的
    // QMetaObject::connectSlotsByName() 已经自动完成连接，这里不能再手动 connect()，
    // 否则一次点击会进入槽函数两次（原来的退格键按一次删两位就是这个原因）。

    // 显示框只用来显示，不接收焦点，保证键盘事件统一由窗口的 keyPressEvent 处理
    ui->display->setReadOnly(true);
    ui->display->setFocusPolicy(Qt::NoFocus);

    // 所有按键都不抢焦点，这样鼠标点完按钮后还能直接用键盘继续输入
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for(QPushButton *button : buttons)
    {
        button->setFocusPolicy(Qt::NoFocus);
    }
    setFocusPolicy(Qt::StrongFocus);

    loadStyleSheet();

    state = InputState;
    updateDisplay("");
    updateExpression();
    ui->statusBar->showMessage(QString("就绪：可以直接用鼠标按键或键盘输入"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 从资源文件加载样式表，集中管理界面外观
void MainWindow::loadStyleSheet()
{
    QFile styleFile(":/style.qss");
    if(styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }
}

// 刷新显示框，空字符串统一显示为 0
void MainWindow::updateDisplay(const QString &text)
{
    ui->display->setText(text.isEmpty() ? QString("0") : text);
}

// 刷新上方的表达式提示，例如 “12.5 ×”
void MainWindow::updateExpression()
{
    if(opcodes.isEmpty() || state == ErrorState)
    {
        ui->lblExpression->clear();
        return;
    }
    QString left = operands.isEmpty() ? QString("0") : operands.back();
    ui->lblExpression->setText(QString("%1 %2").arg(left).arg(opcodes.back()));
}

// 统一错误出口：显示提示并进入错误状态，必须按 C/CE 或输入数字才能继续
void MainWindow::showError(const QString &message)
{
    ui->display->setText(message);
    ui->lblExpression->clear();
    ui->statusBar->showMessage(message);
    state = ErrorState;
}

/*!
 * \brief 弹出两个操作数和一个运算符，计算后把结果压回栈顶
 * \param ok 出错时置为 false（如除数为0）
 * \return 计算结果（或错误提示文本）
 */
QString MainWindow::calculation(bool *ok)
{
    if(ok)
    {
        *ok = true;
    }

    double result = 0;

    if(operands.size() >= 2 && opcodes.size() > 0)
    {
        // 取操作数
        double operand1 = operands.front().toDouble();
        operands.pop_front();
        double operand2 = operands.front().toDouble();
        operands.pop_front();

        // 取操作符
        QString op = opcodes.front();
        opcodes.pop_front();

        if(op == "+")
        {
            result = operand1 + operand2;
        }
        else if(op == "-")
        {
            result = operand1 - operand2;
        }
        else if(op == "×" || op == "*")
        {
            result = operand1 * operand2;
        }
        else if(op == "÷" || op == "/")
        {
            // 除数为 0 属于异常输入：明确提示，并清空现场，避免出现 inf / nan
            if(operand2 == 0.0)
            {
                if(ok)
                {
                    *ok = false;
                }
                operands.clear();
                opcodes.clear();
                operand.clear();
                opcode.clear();
                showError(QString("除数不能为0"));
                return QString("除数不能为0");
            }
            result = operand1 / operand2;
        }
        else
        {
            if(ok)
            {
                *ok = false;
            }
            return ui->display->text();
        }

        // 用 'g' 格式只保留有效数字，避免 0.1+0.2 显示成 0.30000000000000004
        operands.push_back(QString::number(result, 'g', 12));
        ui->statusBar->showMessage(QString("计算结果：%1").arg(QString::number(result, 'g', 12)));
    }

    return QString::number(result, 'g', 12);
}

// ---------------- 鼠标按键对应的槽函数：取出按钮文本后交给共用函数 ----------------

void MainWindow::btnNumClicked()
{
    inputDigit(qobject_cast<QPushButton *>(sender())->text());
}

void MainWindow::on_btnPeriod_clicked()
{
    inputPeriod();
}

void MainWindow::on_btnDel_clicked()
{
    backspace();
}

void MainWindow::on_btnClear_clicked()
{
    clearEntry();
}

void MainWindow::on_btnClearAll_clicked()
{
    clearAll();
}

void MainWindow::on_btnEqual_clicked()
{
    inputEqual();
}

void MainWindow::on_btnZF_clicked()
{
    toggleSign();
}

void MainWindow::btnBinaryOperatorClicked()
{
    inputBinaryOperator(qobject_cast<QPushButton *>(sender())->text());
}

void MainWindow::btnUnaryOperaterClicked()
{
    inputUnaryOperator(qobject_cast<QPushButton *>(sender())->text());
}

// ---------------- 鼠标与键盘共用的处理逻辑 ----------------

void MainWindow::inputDigit(const QString &digit)
{
    // 出错后、或已经按过“=”之后，再输入数字都表示开始一条新的算式
    if(state == ErrorState || state == ResultState)
    {
        clearAll();
    }

    // 刚按完运算符，开始输入右操作数
    if(state == OperatorState)
    {
        operand.clear();
        state = InputState;
    }

    if(operand == "0")
    {
        if(digit == "0")
        {
            return;     // “0”后面继续按 0，仍然是 0
        }
        operand.clear();// 去掉无意义的前导 0
    }
    else if(operand == "-0")
    {
        operand = "-";
    }

    operand += digit;
    updateDisplay(operand);
}

void MainWindow::inputPeriod()
{
    if(state == ErrorState || state == ResultState)
    {
        clearAll();
    }

    if(state == OperatorState)
    {
        operand.clear();
        state = InputState;
    }

    // 直接判断“.”在 operand 中是否已经存在，保证一个操作数只有一个小数点
    if(operand.contains("."))
    {
        return;
    }

    // 以小数点开头时补上前导 0，保证 toDouble() 正确
    if(operand.isEmpty() || operand == "-")
    {
        operand += "0";
    }

    operand += ".";
    updateDisplay(operand);
}

void MainWindow::backspace()
{
    // 结果状态和错误状态下不允许退格，避免破坏已有的计算状态
    if(state == ResultState || state == ErrorState)
    {
        return;
    }

    if(operand.isEmpty())
    {
        return;
    }

    operand.chop(1);
    updateDisplay(operand);
}

void MainWindow::clearEntry()
{
    // 计算完成或出错后按 CE，直接回到初始状态
    if(state == ResultState || state == ErrorState)
    {
        clearAll();
        return;
    }

    // 只清除当前正在输入的操作数，已经压栈的运算符保留（与系统计算器的 CE 一致）
    operand.clear();
    state = InputState;
    updateDisplay("");
    ui->statusBar->showMessage(QString("已清除当前输入"));
}

void MainWindow::clearAll()
{
    operands.clear();
    opcodes.clear();
    operand.clear();
    opcode.clear();
    state = InputState;
    updateDisplay("");
    updateExpression();
    ui->statusBar->showMessage(QString("已清空，可以重新计算"));
}

void MainWindow::toggleSign()
{
    if(state == ErrorState)
    {
        return;
    }

    if(operand.isEmpty())
    {
        // 结果状态下取反：直接修改栈顶的结果值
        if(!operands.isEmpty())
        {
            double value = -operands.back().toDouble();
            operands.pop_back();
            operands.push_back(QString::number(value, 'g', 12));
            updateDisplay(operands.back());
        }
        return;
    }

    if(operand.startsWith("-"))
    {
        operand.remove(0, 1);
    }
    else
    {
        operand.prepend("-");
    }
    updateDisplay(operand);
}

void MainWindow::inputBinaryOperator(const QString &op)
{
    if(state == ErrorState)
    {
        return;
    }

    if(state == OperatorState)
    {
        // 连续按运算符：只替换上一个运算符，不产生新的计算
        if(!opcodes.isEmpty())
        {
            opcodes.pop_back();
            opcodes.push_back(op);
        }
        opcode = op;
        updateExpression();
        ui->statusBar->showMessage(QString("当前运算符：%1").arg(op));
        return;
    }

    if(state == ResultState)
    {
        // 用上一次的结果继续运算，结果已经在 operands 栈顶
        opcodes.push_back(op);
        opcode = op;
        state = OperatorState;
        updateExpression();
        ui->statusBar->showMessage(QString("用结果继续运算，当前运算符：%1").arg(op));
        return;
    }

    // 正常输入完一个操作数后按运算符
    operands.push_back(operand);
    operand.clear();
    opcodes.push_back(op);
    opcode = op;
    state = OperatorState;

    if(operands.size() >= 2)
    {
        // 连续运算：栈中已经有两个操作数，立即算出中间结果
        bool ok = true;
        updateDisplay(calculation(&ok));
        if(!ok)
        {
            return;
        }
    }
    else
    {
        // 只有左操作数时显示左操作数（原来这里会被 calculation() 的默认返回值 0 覆盖）
        updateDisplay(operands.back());
    }

    state = OperatorState;
    updateExpression();
    ui->statusBar->showMessage(QString("当前运算符：%1").arg(op));
}

void MainWindow::inputUnaryOperator(const QString &op)
{
    if(state == ErrorState)
    {
        return;
    }

    // 没有正在输入的操作数时，对当前显示的值做运算
    QString current = operand;
    if(current.isEmpty())
    {
        current = operands.isEmpty() ? QString("0") : operands.back();
    }

    double result = current.toDouble();

    if(op == "%")
    {
        result = result / 100.0;
    }
    else if(op == "1/x")
    {
        if(result == 0.0)
        {
            operands.clear();
            opcodes.clear();
            operand.clear();
            showError(QString("除数不能为0"));
            return;
        }
        result = 1.0 / result;
    }
    else if(op == "x^2")
    {
        result = result * result;
    }
    else if(op == "√")
    {
        if(result < 0)
        {
            operands.clear();
            opcodes.clear();
            operand.clear();
            showError(QString("无效输入"));
            return;
        }
        result = std::sqrt(result);
    }
    else
    {
        return;
    }

    operand = QString::number(result, 'g', 12);

    // 结果状态下做一元运算后，要把新值同步回栈顶，保证后面按运算符时用的是新值
    if(state == ResultState && !operands.isEmpty())
    {
        operands.pop_back();
        operands.push_back(operand);
    }

    updateDisplay(operand);
    ui->statusBar->showMessage(QString("%1(%2) = %3").arg(op).arg(current).arg(operand));
}

void MainWindow::inputEqual()
{
    if(state == ErrorState)
    {
        return;
    }

    // 没有输入任何操作数（例如一上来就按“=”），保持显示不变
    if(operand.isEmpty())
    {
        if(!operands.isEmpty())
        {
            updateDisplay(operands.back());
        }
        else
        {
            updateDisplay("");
        }
        opcodes.clear();
        updateExpression();
        state = ResultState;
        return;
    }

    operands.push_back(operand);
    operand.clear();

    if(operands.size() < 2 || opcodes.isEmpty())
    {
        // 只有一个操作数时，结果就是它本身
        updateDisplay(operands.back());
        opcodes.clear();
        updateExpression();
        state = ResultState;
        return;
    }

    bool ok = true;
    QString result = calculation(&ok);
    if(!ok)
    {
        return;     // 计算过程中已经给出错误提示
    }

    updateDisplay(result);
    opcodes.clear();
    updateExpression();
    ui->statusBar->showMessage(QString("计算完成：%1").arg(result));
    state = ResultState;
}

// ---------------- 键盘事件 ----------------

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();

    // 数字键（主键盘和小键盘的数字都映射到 Key_0 ~ Key_9）
    if(key >= Qt::Key_0 && key <= Qt::Key_9)
    {
        inputDigit(QString::number(key - Qt::Key_0));
        return;
    }

    switch(key)
    {
    case Qt::Key_Period:
        inputPeriod();                          // .
        return;
    case Qt::Key_Backspace:
        backspace();                            // 退格
        return;
    case Qt::Key_Delete:
    case Qt::Key_Escape:
        clearAll();                             // Del / Esc 相当于 C
        return;
    case Qt::Key_Plus:
        inputBinaryOperator(QString("+"));      // +
        return;
    case Qt::Key_Minus:
        inputBinaryOperator(QString("-"));      // -
        return;
    case Qt::Key_Asterisk:
        inputBinaryOperator(QString("×"));      // * 映射为按钮上的 ×
        return;
    case Qt::Key_Slash:
        inputBinaryOperator(QString("÷"));      // / 映射为按钮上的 ÷
        return;
    case Qt::Key_Equal:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        inputEqual();                           // = 或回车
        return;
    case Qt::Key_Percent:
        inputUnaryOperator(QString("%"));       // %
        return;
    case Qt::Key_C:
        clearAll();                             // 字母 C 相当于 C
        return;
    default:
        break;
    }

    // 其余按键交回父类处理
    QMainWindow::keyPressEvent(event);
}
