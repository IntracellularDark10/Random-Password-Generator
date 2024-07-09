#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <random>
#include <string>

std::string generatePassword(int length, bool includeUpper, bool includeLower, bool includeDigits, bool includeSpecial) {
    const std::string upper_case = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string lower_case = "abcdefghijklmnopqrstuvwxyz";
    const std::string digits = "0123456789";
    const std::string special_chars = "!@#$%^&*()-_=+[]{}|;:',.<>?/`~";

    std::string all_chars;
    if (includeUpper) all_chars += upper_case;
    if (includeLower) all_chars += lower_case;
    if (includeDigits) all_chars += digits;
    if (includeSpecial) all_chars += special_chars;

    if (all_chars.empty()) {
        return "Error: No character sets selected.";
    }

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, all_chars.size() - 1);

    std::string password;
    for (int i = 0; i < length; ++i) {
        password += all_chars[distribution(generator)];
    }
    return password;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Password Generator");

    QVBoxLayout *layout = new QVBoxLayout(&window);

    QLabel *label = new QLabel("Enter the desired password length:");
    layout->addWidget(label);

    QLineEdit *input = new QLineEdit();
    layout->addWidget(input);

    QCheckBox *upperCheckBox = new QCheckBox("Include Uppercase Letters");
    upperCheckBox->setChecked(true);
    layout->addWidget(upperCheckBox);

    QCheckBox *lowerCheckBox = new QCheckBox("Include Lowercase Letters");
    lowerCheckBox->setChecked(true);
    layout->addWidget(lowerCheckBox);

    QCheckBox *digitsCheckBox = new QCheckBox("Include Digits");
    digitsCheckBox->setChecked(true);
    layout->addWidget(digitsCheckBox);

    QCheckBox *specialCheckBox = new QCheckBox("Include Special Characters");
    specialCheckBox->setChecked(true);
    layout->addWidget(specialCheckBox);

    QPushButton *button = new QPushButton("Generate Password");
    layout->addWidget(button);

    QTextEdit *output = new QTextEdit();
    output->setReadOnly(true);
    layout->addWidget(output);

    QObject::connect(button, &QPushButton::clicked, [&]() {
        bool ok;
        int length = input->text().toInt(&ok);
        if (ok && length > 0) {
            bool includeUpper = upperCheckBox->isChecked();
            bool includeLower = lowerCheckBox->isChecked();
            bool includeDigits = digitsCheckBox->isChecked();
            bool includeSpecial = specialCheckBox->isChecked();

            std::string password = generatePassword(length, includeUpper, includeLower, includeDigits, includeSpecial);
            output->setText(QString::fromStdString(password));
        } else {
            output->setText("Please enter a valid positive integer.");
        }
    });

    window.show();
    return app.exec();
}

