#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QTextEdit>
#include <QTextOption>
#include <QVBoxLayout>
#include <QWidget>

#include <exception>
#include <string>

#include "password_generator.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Password Generator");
    auto *layout = new QVBoxLayout(&window);

    layout->addWidget(new QLabel(QString("Enter the desired password length (1-%1):").arg(pwgen::kMaxLength)));

    auto *input = new QLineEdit("16");
    layout->addWidget(input);

    auto *upperBox = new QCheckBox("Include Uppercase Letters");
    auto *lowerBox = new QCheckBox("Include Lowercase Letters");
    auto *digitsBox = new QCheckBox("Include Digits");
    auto *specialBox = new QCheckBox("Include Special Characters");
    auto *ambiguousBox = new QCheckBox("Exclude ambiguous characters (I, l, 1, O, 0)");
    for (auto *box : {upperBox, lowerBox, digitsBox, specialBox}) {
        box->setChecked(true);
        layout->addWidget(box);
    }
    layout->addWidget(ambiguousBox);

    auto *generateButton = new QPushButton("Generate Password");
    layout->addWidget(generateButton);

    auto *status = new QLabel();
    status->setStyleSheet("color: #c0392b;");
    status->setWordWrap(true);
    layout->addWidget(status);

    auto *output = new QTextEdit();
    output->setReadOnly(true);
    output->setWordWrapMode(QTextOption::WrapAnywhere);
    output->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(output);

    auto *copyButton = new QPushButton("Copy to Clipboard");
    copyButton->setEnabled(false);
    layout->addWidget(copyButton);

    QObject::connect(input, &QLineEdit::returnPressed, generateButton, &QPushButton::click);

    QObject::connect(generateButton, &QPushButton::clicked, [=]() {
        bool ok = false;
        const int length = input->text().trimmed().toInt(&ok);
        if (!ok || length < 1 || length > pwgen::kMaxLength) {
            output->clear();
            copyButton->setEnabled(false);
            status->setText(QString("Please enter a whole number from 1 to %1.").arg(pwgen::kMaxLength));
            return;
        }

        pwgen::Options opts;
        opts.length = length;
        opts.upper = upperBox->isChecked();
        opts.lower = lowerBox->isChecked();
        opts.digits = digitsBox->isChecked();
        opts.special = specialBox->isChecked();
        opts.excludeAmbiguous = ambiguousBox->isChecked();

        try {
            output->setPlainText(QString::fromStdString(pwgen::generate(opts)));
            status->clear();
            copyButton->setEnabled(true);
        } catch (const std::exception &e) {
            output->clear();
            copyButton->setEnabled(false);
            status->setText(QString::fromUtf8(e.what()));
        }
    });

    QObject::connect(copyButton, &QPushButton::clicked, [=]() {
        QApplication::clipboard()->setText(output->toPlainText());
    });

    window.resize(420, 480);
    window.show();
    return app.exec();
}
