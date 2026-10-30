#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <math.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btnNum0,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum1,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum2,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum3,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum4,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum5,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum6,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum7,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum8,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum9,SIGNAL(clicked()),this,SLOT(btnNumClicked()));

    connect(ui->btnPlus,SIGNAL(clicked()),this,SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnMinus,SIGNAL(clicked()),this,SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnMultiple,SIGNAL(clicked()),this,SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnDivide,SIGNAL(clicked()),this,SLOT(btnBinaryOperatorClicked()));

    connect(ui->btnPercentage,SIGNAL(clicked()),this,SLOT(btnUnaryOperaterClicked()));
    connect(ui->btnInverse,SIGNAL(clicked()),this,SLOT(btnUnaryOperaterClicked()));
    connect(ui->btnSquare,SIGNAL(clicked()),this,SLOT(btnUnaryOperaterClicked()));
    connect(ui->btnSqrt,SIGNAL(clicked()),this,SLOT(btnUnaryOperaterClicked()));

    //注意：on_btnClearAll_clicked()、on_btnClear_clicked()、on_btnDel_clicked()、
    //on_btnPeriod_clicked()、on_btnEqual_clicked()、on_btnZF_clicked() 的命名符合
    //on_<对象名>_<信号名> 规则，ui->setupUi() 内部的 QMetaObject::connectSlotsByName()
    //已经自动完成连接，这里不能再手动 connect()，
    //否则一次点击会进入槽函数两次（原来退格键按一次删两位就是这个原因）。

}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::calculation(bool *ok)
{


    double result =0;
    if(operands.size()==2 && opcodes.size()>0)
    {   //取操作数
        double operand1 = operands.front().toDouble();
        operands.pop_front();
        double operand2 = operands.front().toDouble();
        operands.pop_front();

        //取操作符
        QString op = opcodes.front();
        opcodes.pop_front();

        if(op == "+"){
            result = operand1 + operand2;
        }else if(op == "-"){
            result = operand1 - operand2;
        }else if(op == "×"||op == "*"){
            //原来的写法是 op == "*"||"×"，"×"是非空字符串常量，恒为真，
            //导致后面的除法分支永远进不去，8÷2 会被算成 8×2
            result = operand1 * operand2;
        }else if(op == "÷"||op == "/"){
            //除数为 0 时直接给出提示，避免出现 inf / nan
            if(operand2 == 0.0){
                if(ok){
                    *ok = false;
                }
                operands.clear();
                opcodes.clear();
                operand.clear();
                ui->statusBar->showMessage(QString("除数不能为0"));
                return QString("除数不能为0");
            }
            result = operand1 / operand2;
        }


        operands.push_back(QString::number(result));

        ui->statusBar->showMessage(QString("calculation is in progress: operands is  %1, opcode is  %2").arg(operands.size()).arg(opcodes.size()));
    }
    else
    {
        ui->statusBar->showMessage(QString("operands is  %1, opcode is  %2").arg(operands.size()).arg(opcodes.size()));
    }



    return QString::number(result);
}

void MainWindow::btnNumClicked()
{
    QString digit = qobject_cast<QPushButton *>(sender())->text();

    if(digit == "0" && operand == "0")
        digit = "";

    if(operand == "0" && digit != "0")
        operand = "";

    operand += digit;

    ui->display->setText(operand);
    //ui->statusBar->showMessage(qobject_cast<QPushButton*>(sender())->text() + "btn clicked");
}

void MainWindow::on_btnPeriod_clicked()
{
    if(!operand.contains("."))
        operand += qobject_cast<QPushButton*>(sender())->text();
    ui->display->setText(operand);
}

void MainWindow::on_btnDel_clicked()
{
    operand = operand.left(operand.length() - 1);
    ui->display->setText(operand);
}

void MainWindow::on_btnClear_clicked()
{
    operand.clear();
    ui->display->setText(operand);
}

void MainWindow::on_btnClearAll_clicked()
{
    operands.clear();
    opcodes.clear();
    operand.clear();
    opcode.clear();
    ui->display->setText(operand);

}



void MainWindow::on_btnEqual_clicked()
{
    if(operand != "")
    {
        operands.push_back(operand);
        operand = "";
    }
    QString result = calculation();
    ui->display->setText(result);
}

void MainWindow::btnBinaryOperatorClicked()
{
    ui->statusBar->showMessage("last operand"+operand);
    QString opcode = qobject_cast<QPushButton *>(sender())->text();

    if(operand != "")
    {
        operands.push_back(operand);
        operand = "";
        opcodes.push_back(opcode);

        //只有凑够两个操作数时才真正计算：否则 calculation() 会走到 else 分支
        //返回默认的 0，显示框就被清零了（例如输入 2 后按 + 会显示 0）
        if(operands.size() >= 2 && opcodes.size() > 0)
        {
            QString result = calculation();
            ui->display->setText(result);
        }
        else
        {
            ui->display->setText(operands.back());
        }
    }
}


void MainWindow::btnUnaryOperaterClicked()
{
    if(operand != "")
    {
        double result = operand.toDouble()  ;
        operand = "";

        QString op =qobject_cast<QPushButton*>(sender())->text();

        if(op == "%"){
            result /= 100.0;
        }else if(op == "1/x"){
            result = 1/result;
        }else if(op == "x^2"){
            result *= result;
        }else if(op == "√"){
            result =sqrt(result);
        }

        ui->display->setText(QString::number(result));
    }
}


void MainWindow::on_btnZF_clicked()
{
    //正负号：直接改变当前操作数的符号
    if(operand.isEmpty()){
        return;
    }

    if(operand.startsWith("-")){
        operand.remove(0, 1);
    }else{
        operand.prepend("-");
    }

    ui->display->setText(operand.isEmpty() ? QString("0") : operand);
}