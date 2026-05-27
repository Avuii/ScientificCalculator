#include "calculator.h"
#include "ui_calculator.h"

#include <QSignalBlocker>
#include <QStatusBar>

#include <cmath>

namespace
{
constexpr double PI = 3.14159265358979323846;
constexpr double EPS = 1e-12;

bool endsWithOperator(const QString &text)
{
    return text.endsWith('+')
    || text.endsWith('-')
        || text.endsWith(QChar(0x00D7))   // ×
        || text.endsWith(QChar(0x00F7))   // ÷
        || text.endsWith("mod");
}

class ExpressionParser
{
public:
    explicit ExpressionParser(const QString &input)
        : m_input(input), m_pos(0)
    {
    }

    double parse(bool &ok, QString &error)
    {
        ok = true;
        error.clear();

        double value = parseExpression(ok, error);
        skipSpaces();

        if (ok && m_pos != m_input.size())
        {
            ok = false;
            error = "Niepoprawne wyrażenie.";
            return 0.0;
        }

        return value;
    }

private:
    QString m_input;
    int m_pos;

    void skipSpaces()
    {
        while (m_pos < m_input.size() && m_input[m_pos].isSpace())
        {
            ++m_pos;
        }
    }

    bool match(QChar c)
    {
        skipSpaces();
        if (m_pos < m_input.size() && m_input[m_pos] == c)
        {
            ++m_pos;
            return true;
        }
        return false;
    }

    bool matchWord(const QString &word)
    {
        skipSpaces();
        if (m_input.mid(m_pos, word.size()).compare(word, Qt::CaseInsensitive) == 0)
        {
            m_pos += word.size();
            return true;
        }
        return false;
    }

    double parseExpression(bool &ok, QString &error)
    {
        double left = parseTerm(ok, error);

        while (ok)
        {
            if (match('+'))
            {
                left += parseTerm(ok, error);
            }
            else if (match('-'))
            {
                left -= parseTerm(ok, error);
            }
            else
            {
                break;
            }
        }

        return left;
    }

    double parseTerm(bool &ok, QString &error)
    {
        double left = parseUnary(ok, error);

        while (ok)
        {
            if (match('*') || match(QChar(0x00D7)))
            {
                left *= parseUnary(ok, error);
            }
            else if (match('/') || match(QChar(0x00F7)))
            {
                double right = parseUnary(ok, error);
                if (!ok) return 0.0;

                if (std::abs(right) < EPS)
                {
                    ok = false;
                    error = "Dzielenie przez zero.";
                    return 0.0;
                }

                left /= right;
            }
            else if (matchWord("mod"))
            {
                double right = parseUnary(ok, error);
                if (!ok) return 0.0;

                if (std::abs(right) < EPS)
                {
                    ok = false;
                    error = "Modulo przez zero.";
                    return 0.0;
                }

                left = std::fmod(left, right);
            }
            else
            {
                break;
            }
        }

        return left;
    }

    double parseUnary(bool &ok, QString &error)
    {
        if (match('+'))
        {
            return parseUnary(ok, error);
        }

        if (match('-'))
        {
            return -parseUnary(ok, error);
        }

        return parsePrimary(ok, error);
    }

    double parsePrimary(bool &ok, QString &error)
    {
        skipSpaces();

        if (match('('))
        {
            double value = parseExpression(ok, error);
            if (!ok) return 0.0;

            if (!match(')'))
            {
                ok = false;
                error = "Brak zamykającego nawiasu.";
                return 0.0;
            }

            return value;
        }

        return parseNumber(ok, error);
    }

    double parseNumber(bool &ok, QString &error)
    {
        skipSpaces();

        int start = m_pos;
        bool hasDigit = false;

        while (m_pos < m_input.size() && m_input[m_pos].isDigit())
        {
            hasDigit = true;
            ++m_pos;
        }

        if (m_pos < m_input.size() && m_input[m_pos] == '.')
        {
            ++m_pos;

            while (m_pos < m_input.size() && m_input[m_pos].isDigit())
            {
                hasDigit = true;
                ++m_pos;
            }
        }

        if (!hasDigit)
        {
            ok = false;
            error = "Oczekiwano liczby.";
            return 0.0;
        }

        bool convertOk = false;
        double value = m_input.mid(start, m_pos - start).toDouble(&convertOk);

        if (!convertOk)
        {
            ok = false;
            error = "Niepoprawna liczba.";
            return 0.0;
        }

        return value;
    }
};
}

Calculator::Calculator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Calculator)
{
    ui->setupUi(this);

    setupUiState();
    setupConnections();
}

Calculator::~Calculator()
{
    delete ui;
}

void Calculator::setupUiState()
{
    ui->Display->setReadOnly(true);
    ui->Display->setFocusPolicy(Qt::NoFocus);
    ui->Display->setMaxLength(256);
    ui->Display->setText("0");

    ui->Display->setStyleSheet(
        "QLineEdit {"
        "background-color: #707070;"
        "color: white;"
        "border: 1px solid #8a8a8a;"
        "padding: 10px;"
        "font-size: 24px;"
        "font-weight: bold;"
        "}"
        );

    if (ui->Funkcje->count() <= 1)
    {
        ui->Funkcje->addItems({
            "log",
            "ln",
            "√x",
            "x²",
            "1/x",
            "|x|",
            "n!",
            "π",
            "e"
        });
    }

    statusBar()->showMessage("Kalkulator gotowy.", 2000);
}

void Calculator::setupConnections()
{
    connect(ui->liczba0, &QPushButton::clicked, this, [this]() { appendDigit("0"); });
    connect(ui->liczba1, &QPushButton::clicked, this, [this]() { appendDigit("1"); });
    connect(ui->liczba2, &QPushButton::clicked, this, [this]() { appendDigit("2"); });
    connect(ui->liczba3, &QPushButton::clicked, this, [this]() { appendDigit("3"); });
    connect(ui->liczba4, &QPushButton::clicked, this, [this]() { appendDigit("4"); });
    connect(ui->liczba5, &QPushButton::clicked, this, [this]() { appendDigit("5"); });
    connect(ui->liczba6, &QPushButton::clicked, this, [this]() { appendDigit("6"); });
    connect(ui->liczba7, &QPushButton::clicked, this, [this]() { appendDigit("7"); });
    connect(ui->liczba8, &QPushButton::clicked, this, [this]() { appendDigit("8"); });
    connect(ui->liczba9, &QPushButton::clicked, this, [this]() { appendDigit("9"); });

    connect(ui->dodawanie, &QPushButton::clicked, this, [this]() { appendOperator("+"); });
    connect(ui->minus, &QPushButton::clicked, this, [this]() { appendOperator("-"); });
    connect(ui->mnozenie, &QPushButton::clicked, this, [this]() { appendOperator(QString(QChar(0x00D7))); });
    connect(ui->dzielenie, &QPushButton::clicked, this, [this]() { appendOperator(QString(QChar(0x00F7))); });
    connect(ui->mod, &QPushButton::clicked, this, [this]() { appendOperator("mod"); });

    connect(ui->przecinek, &QPushButton::clicked, this, &Calculator::appendDot);
    connect(ui->nawias_otw, &QPushButton::clicked, this, &Calculator::appendLeftParen);
    connect(ui->nawias_zam, &QPushButton::clicked, this, &Calculator::appendRightParen);

    connect(ui->pi, &QPushButton::clicked, this, [this]() { appendConstant(PI); });
    connect(ui->e, &QPushButton::clicked, this, [this]() { appendConstant(std::exp(1.0)); });

    connect(ui->backspace, &QPushButton::clicked, this, &Calculator::backspace);
    connect(ui->CE, &QPushButton::clicked, this, &Calculator::clearEntry);
    connect(ui->C, &QPushButton::clicked, this, &Calculator::clearDisplay);
    connect(ui->AC, &QPushButton::clicked, this, &Calculator::clearAll);
    connect(ui->rownasie, &QPushButton::clicked, this, &Calculator::calculateResult);

    connect(ui->plusminus, &QPushButton::clicked, this, &Calculator::toggleSign);

    connect(ui->x2, &QPushButton::clicked, this, [this]() {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            Q_UNUSED(error);
            ok = true;
            return x * x;
        });
    });

    connect(ui->pierwiatek, &QPushButton::clicked, this, [this]() {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            if (x < 0.0)
            {
                ok = false;
                error = "Pierwiastek z liczby ujemnej.";
                return 0.0;
            }
            ok = true;
            return std::sqrt(x);
        });
    });

    connect(ui->ulamek, &QPushButton::clicked, this, [this]() {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            if (std::abs(x) < EPS)
            {
                ok = false;
                error = "Nie można dzielić przez zero.";
                return 0.0;
            }
            ok = true;
            return 1.0 / x;
        });
    });

    connect(ui->bezwzgledna, &QPushButton::clicked, this, [this]() {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            Q_UNUSED(error);
            ok = true;
            return std::abs(x);
        });
    });

    connect(ui->log, &QPushButton::clicked, this, [this]() {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            if (x <= 0.0)
            {
                ok = false;
                error = "log jest określony tylko dla x > 0.";
                return 0.0;
            }
            ok = true;
            return std::log10(x);
        });
    });

    connect(ui->silnia, &QPushButton::clicked, this, [this]() {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            if (x < 0.0)
            {
                ok = false;
                error = "Silnia dla liczb ujemnych nie istnieje.";
                return 0.0;
            }

            double rounded = std::round(x);
            if (std::abs(x - rounded) > EPS)
            {
                ok = false;
                error = "Silnia działa tylko dla liczb całkowitych.";
                return 0.0;
            }

            if (rounded > 170.0)
            {
                ok = false;
                error = "Wynik jest za duży.";
                return 0.0;
            }

            unsigned int n = static_cast<unsigned int>(rounded);
            double result = 1.0;
            for (unsigned int i = 2; i <= n; ++i)
            {
                result *= i;
            }

            ok = true;
            return result;
        });
    });

    connect(ui->MC, &QPushButton::clicked, this, &Calculator::memoryClear);
    connect(ui->MR, &QPushButton::clicked, this, &Calculator::memoryRecall);
    connect(ui->MS, &QPushButton::clicked, this, &Calculator::memoryStore);
    connect(ui->Mplus, &QPushButton::clicked, this, &Calculator::memoryAdd);
    connect(ui->Mminus, &QPushButton::clicked, this, &Calculator::memorySubtract);

    connect(ui->Trygonometria, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Calculator::handleTrigSelection);

    connect(ui->Funkcje, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Calculator::handleFunctionSelection);

    connect(ui->nd2, &QPushButton::clicked, this, [this]() {
        statusBar()->showMessage("Tryb 2nd nie jest jeszcze zaimplementowany.", 2500);
    });
}

QString Calculator::displayText() const
{
    return ui->Display->text().trimmed();
}

void Calculator::setDisplayText(const QString &text)
{
    ui->Display->setText(text);
    ui->Display->setCursorPosition(ui->Display->text().size());
}

QString Calculator::formatNumber(double value) const
{
    if (!std::isfinite(value))
    {
        return "Błąd";
    }

    if (std::abs(value) < EPS)
    {
        value = 0.0;
    }

    QString text = QString::number(value, 'g', 15);

    if (text.contains('.') && !text.contains('e') && !text.contains('E'))
    {
        while (text.endsWith('0'))
        {
            text.chop(1);
        }

        if (text.endsWith('.'))
        {
            text.chop(1);
        }
    }

    if (text == "-0")
    {
        text = "0";
    }

    return text;
}

void Calculator::resetIfError()
{
    if (errorState)
    {
        setDisplayText("0");
        errorState = false;
    }
}

void Calculator::showError(const QString &message)
{
    errorState = true;
    setDisplayText("Błąd");
    statusBar()->showMessage(message, 4000);
}

void Calculator::appendDigit(const QString &digit)
{
    resetIfError();

    QString text = displayText();

    if (text == "0")
    {
        if (digit == "0")
            return;

        setDisplayText(digit);
        return;
    }

    if (!text.isEmpty() && text.back() == ')')
    {
        text += QString(QChar(0x00D7));
    }

    text += digit;
    setDisplayText(text);
}

void Calculator::appendOperator(const QString &op)
{
    resetIfError();

    QString text = displayText();

    if (text == "0")
    {
        if (op == "-")
            setDisplayText("-");
        return;
    }

    if (text.isEmpty())
    {
        if (op == "-")
            setDisplayText("-");
        return;
    }

    if (text.endsWith('('))
    {
        if (op == "-")
        {
            text += "-";
            setDisplayText(text);
        }
        return;
    }

    if (endsWithOperator(text))
    {
        if (text.endsWith("mod"))
            text.chop(3);
        else
            text.chop(1);

        text += op;
        setDisplayText(text);
        return;
    }

    if (text.endsWith('.'))
        return;

    text += op;
    setDisplayText(text);
}

void Calculator::appendDot()
{
    resetIfError();

    QString text = displayText();

    if (text == "0")
    {
        setDisplayText("0.");
        return;
    }

    if (!text.isEmpty() && text.back() == ')')
    {
        setDisplayText(text + QString(QChar(0x00D7)) + "0.");
        return;
    }

    if (endsWithOperator(text) || text.endsWith('(') || text == "-")
    {
        setDisplayText(text + "0.");
        return;
    }

    int i = text.size() - 1;
    while (i >= 0)
    {
        QChar c = text[i];
        if (c == '+' || c == '-' || c == QChar(0x00D7) || c == QChar(0x00F7) || c == '(' || c == ')')
            break;

        if (i >= 2 && text.mid(i - 2, 3) == "mod")
            break;

        --i;
    }

    QString currentToken = text.mid(i + 1);
    if (currentToken.contains('.'))
        return;

    text += ".";
    setDisplayText(text);
}

void Calculator::appendLeftParen()
{
    resetIfError();

    QString text = displayText();

    if (text == "0")
    {
        setDisplayText("(");
        return;
    }

    if (!text.isEmpty())
    {
        QChar last = text.back();
        if (last.isDigit() || last == ')' || last == '.')
        {
            text += QString(QChar(0x00D7));
        }
    }

    text += "(";
    setDisplayText(text);
}

void Calculator::appendRightParen()
{
    resetIfError();

    QString text = displayText();

    if (text.isEmpty() || endsWithOperator(text) || text.endsWith('(') || text == "-")
        return;

    int openCount = text.count('(');
    int closeCount = text.count(')');

    if (openCount <= closeCount)
        return;

    text += ")";
    setDisplayText(text);
}

void Calculator::appendConstant(double value)
{
    resetIfError();

    QString token = formatNumber(value);
    QString text = displayText();

    if (text == "0")
    {
        setDisplayText(token);
        return;
    }

    if (!text.isEmpty())
    {
        QChar last = text.back();
        if (last.isDigit() || last == ')' || last == '.')
        {
            text += QString(QChar(0x00D7));
        }
    }

    text += token;
    setDisplayText(text);
}

void Calculator::backspace()
{
    if (errorState)
    {
        clearDisplay();
        return;
    }

    QString text = displayText();

    if (text.size() <= 1)
    {
        setDisplayText("0");
        return;
    }

    if (text.endsWith("mod"))
        text.chop(3);
    else
        text.chop(1);

    if (text.isEmpty() || text == "-")
        text = "0";

    setDisplayText(text);
}

void Calculator::clearEntry()
{
    if (errorState)
    {
        clearDisplay();
        return;
    }

    QString text = displayText();

    if (text == "0")
        return;

    if (text.endsWith("mod"))
    {
        text.chop(3);
    }
    else if (text.endsWith(')'))
    {
        int depth = 0;
        int start = -1;

        for (int i = text.size() - 1; i >= 0; --i)
        {
            if (text[i] == ')')
                ++depth;
            else if (text[i] == '(')
            {
                --depth;
                if (depth == 0)
                {
                    start = i;
                    break;
                }
            }
        }

        if (start >= 0)
            text.remove(start, text.size() - start);
        else
            text = "0";
    }
    else
    {
        int i = text.size() - 1;

        while (i >= 0 && (text[i].isDigit() || text[i] == '.'))
            --i;

        if (i == text.size() - 1)
            text.chop(1);
        else
            text.remove(i + 1, text.size() - (i + 1));
    }

    if (text.isEmpty() || text == "-")
        text = "0";

    setDisplayText(text);
}

void Calculator::clearDisplay()
{
    errorState = false;
    setDisplayText("0");
    statusBar()->clearMessage();
}

void Calculator::clearAll()
{
    errorState = false;
    setDisplayText("0");
    statusBar()->clearMessage();
}

double Calculator::currentValue(bool *ok, QString *error) const
{
    return evaluateExpression(displayText(), ok, error);
}

double Calculator::evaluateExpression(const QString &expr, bool *ok, QString *errorMessage) const
{
    bool localOk = true;
    QString localError;

    QString trimmed = expr.trimmed();
    if (trimmed.isEmpty() || trimmed == "-" || endsWithOperator(trimmed) || trimmed.endsWith('('))
    {
        localOk = false;
        localError = "Niepełne wyrażenie.";
    }
    else
    {
        ExpressionParser parser(trimmed);
        double value = parser.parse(localOk, localError);

        if (ok) *ok = localOk;
        if (errorMessage) *errorMessage = localError;

        if (!localOk)
            return 0.0;

        if (!std::isfinite(value))
        {
            if (ok) *ok = false;
            if (errorMessage) *errorMessage = "Wynik poza zakresem.";
            return 0.0;
        }

        return value;
    }

    if (ok) *ok = localOk;
    if (errorMessage) *errorMessage = localError;
    return 0.0;
}

void Calculator::calculateResult()
{
    bool ok = false;
    QString error;
    double value = currentValue(&ok, &error);

    if (!ok)
    {
        showError(error);
        return;
    }

    setDisplayText(formatNumber(value));
    errorState = false;
    statusBar()->showMessage("Obliczono.", 1500);
}

void Calculator::toggleSign()
{
    applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
        Q_UNUSED(error);
        ok = true;
        return -x;
    });
}

void Calculator::applyUnaryOperation(const std::function<double(double, bool&, QString&)> &op)
{
    bool ok = false;
    QString error;
    double value = currentValue(&ok, &error);

    if (!ok)
    {
        showError(error);
        return;
    }

    double result = op(value, ok, error);

    if (!ok || !std::isfinite(result))
    {
        if (error.isEmpty())
            error = "Nie można wykonać tej operacji.";
        showError(error);
        return;
    }

    setDisplayText(formatNumber(result));
    errorState = false;
}

void Calculator::memoryClear()
{
    memoryValue = 0.0;
    memorySet = false;
    statusBar()->showMessage("Pamięć wyczyszczona.", 1500);
}

void Calculator::memoryRecall()
{
    if (!memorySet)
    {
        statusBar()->showMessage("Pamięć jest pusta.", 1500);
        return;
    }

    errorState = false;
    setDisplayText(formatNumber(memoryValue));
    statusBar()->showMessage("Odczytano z pamięci.", 1500);
}

void Calculator::memoryStore()
{
    bool ok = false;
    QString error;
    double value = currentValue(&ok, &error);

    if (!ok)
    {
        showError(error);
        return;
    }

    memoryValue = value;
    memorySet = true;
    statusBar()->showMessage("Zapisano do pamięci.", 1500);
}

void Calculator::memoryAdd()
{
    bool ok = false;
    QString error;
    double value = currentValue(&ok, &error);

    if (!ok)
    {
        showError(error);
        return;
    }

    if (!memorySet)
    {
        memoryValue = 0.0;
        memorySet = true;
    }

    memoryValue += value;
    statusBar()->showMessage("Dodano do pamięci.", 1500);
}

void Calculator::memorySubtract()
{
    bool ok = false;
    QString error;
    double value = currentValue(&ok, &error);

    if (!ok)
    {
        showError(error);
        return;
    }

    if (!memorySet)
    {
        memoryValue = 0.0;
        memorySet = true;
    }

    memoryValue -= value;
    statusBar()->showMessage("Odjęto od pamięci.", 1500);
}

void Calculator::handleTrigSelection(int index)
{
    if (index <= 0)
        return;

    QString selected = ui->Trygonometria->itemText(index);

    {
        QSignalBlocker blocker(ui->Trygonometria);
        ui->Trygonometria->setCurrentIndex(0);
    }

    applyUnaryOperation([this, selected](double x, bool &ok, QString &error) -> double {
        Q_UNUSED(error);

        double angle = x;
        if (ui->DegRad->currentText() == "DEG")
            angle = angle * PI / 180.0;

        ok = true;

        if (selected == "sin")
            return std::sin(angle);
        if (selected == "cos")
            return std::cos(angle);
        if (selected == "tan")
            return std::tan(angle);

        ok = false;
        error = "Nieobsługiwana funkcja trygonometryczna.";
        return 0.0;
    });
}

void Calculator::handleFunctionSelection(int index)
{
    if (index <= 0)
        return;

    QString selected = ui->Funkcje->itemText(index);

    {
        QSignalBlocker blocker(ui->Funkcje);
        ui->Funkcje->setCurrentIndex(0);
    }

    if (selected == "π")
    {
        appendConstant(PI);
        return;
    }

    if (selected == "e")
    {
        appendConstant(std::exp(1.0));
        return;
    }

    if (selected == "x²")
    {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            Q_UNUSED(error);
            ok = true;
            return x * x;
        });
        return;
    }

    if (selected == "√x")
    {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            if (x < 0.0)
            {
                ok = false;
                error = "Pierwiastek z liczby ujemnej.";
                return 0.0;
            }
            ok = true;
            return std::sqrt(x);
        });
        return;
    }

    if (selected == "1/x")
    {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            if (std::abs(x) < EPS)
            {
                ok = false;
                error = "Nie można dzielić przez zero.";
                return 0.0;
            }
            ok = true;
            return 1.0 / x;
        });
        return;
    }

    if (selected == "|x|")
    {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            Q_UNUSED(error);
            ok = true;
            return std::abs(x);
        });
        return;
    }

    if (selected == "log")
    {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            if (x <= 0.0)
            {
                ok = false;
                error = "log jest określony tylko dla x > 0.";
                return 0.0;
            }
            ok = true;
            return std::log10(x);
        });
        return;
    }

    if (selected == "ln")
    {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            if (x <= 0.0)
            {
                ok = false;
                error = "ln jest określony tylko dla x > 0.";
                return 0.0;
            }
            ok = true;
            return std::log(x);
        });
        return;
    }

    if (selected == "n!")
    {
        applyUnaryOperation([](double x, bool &ok, QString &error) -> double {
            if (x < 0.0)
            {
                ok = false;
                error = "Silnia dla liczb ujemnych nie istnieje.";
                return 0.0;
            }

            double rounded = std::round(x);
            if (std::abs(x - rounded) > EPS)
            {
                ok = false;
                error = "Silnia działa tylko dla liczb całkowitych.";
                return 0.0;
            }

            if (rounded > 170.0)
            {
                ok = false;
                error = "Wynik jest za duży.";
                return 0.0;
            }

            unsigned int n = static_cast<unsigned int>(rounded);
            double result = 1.0;
            for (unsigned int i = 2; i <= n; ++i)
                result *= i;

            ok = true;
            return result;
        });
        return;
    }
}