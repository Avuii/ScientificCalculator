#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QMainWindow>
#include <functional>

QT_BEGIN_NAMESPACE
namespace Ui {
class Calculator;
}
QT_END_NAMESPACE

class Calculator : public QMainWindow
{
    Q_OBJECT

public:
    Calculator(QWidget *parent = nullptr);
    ~Calculator();

private:
    Ui::Calculator *ui;

    double memoryValue = 0.0;
    bool memorySet = false;
    bool errorState = false;

    void setupUiState();
    void setupConnections();

    QString displayText() const;
    void setDisplayText(const QString &text);
    QString formatNumber(double value) const;

    void resetIfError();
    void showError(const QString &message);

    void appendDigit(const QString &digit);
    void appendOperator(const QString &op);
    void appendDot();
    void appendLeftParen();
    void appendRightParen();
    void appendConstant(double value);

    void backspace();
    void clearEntry();
    void clearDisplay();
    void clearAll();

    void calculateResult();
    void toggleSign();

    void applyUnaryOperation(const std::function<double(double, bool&, QString&)> &op);
    double currentValue(bool *ok = nullptr, QString *error = nullptr) const;
    double evaluateExpression(const QString &expr, bool *ok = nullptr, QString *errorMessage = nullptr) const;

    void memoryClear();
    void memoryRecall();
    void memoryStore();
    void memoryAdd();
    void memorySubtract();

    void handleTrigSelection(int index);
    void handleFunctionSelection(int index);
};

#endif