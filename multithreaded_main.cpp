#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QFontDatabase>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QPalette>
#include <QPushButton>
#include <QSettings>
#include <QString>
#include <QStyle>
#include <QTextEdit>
#include <QTextOption>
#include <QVBoxLayout>
#include <QWidget>
#include <QtConcurrent>

#include <exception>
#include <string>

#include "password_generator.h"

namespace theme {

const char *const kOrg = "IntracellularDark10";
const char *const kApp = "RandomPasswordGenerator";

QPalette build(const QString &mode, const QColor &accent) {
    QPalette p = QApplication::style()->standardPalette();
    if (mode == "Dark") {
        p.setColor(QPalette::Window, QColor(53, 53, 53));
        p.setColor(QPalette::WindowText, Qt::white);
        p.setColor(QPalette::Base, QColor(42, 42, 42));
        p.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
        p.setColor(QPalette::Text, Qt::white);
        p.setColor(QPalette::Button, QColor(53, 53, 53));
        p.setColor(QPalette::ButtonText, Qt::white);
        p.setColor(QPalette::ToolTipBase, QColor(53, 53, 53));
        p.setColor(QPalette::ToolTipText, Qt::white);
        p.setColor(QPalette::Highlight, QColor(42, 130, 218));
        p.setColor(QPalette::HighlightedText, Qt::black);
        p.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
        p.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
        p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    }
    if (accent.isValid()) {
        p.setColor(QPalette::Highlight, accent);
        p.setColor(QPalette::HighlightedText, accent.lightness() > 128 ? Qt::black : Qt::white);
    }
    return p;
}

struct Saved {
    QString mode = "Light";
    QColor accent;  // invalid = theme default
};

Saved load() {
    QSettings s(kOrg, kApp);
    Saved r;
    if (s.value("theme").toString() == "Dark") r.mode = "Dark";
    r.accent = QColor(s.value("accent").toString());
    return r;
}

void save(const Saved &v) {
    QSettings s(kOrg, kApp);
    s.setValue("theme", v.mode);
    s.setValue("accent", v.accent.isValid() ? v.accent.name() : QString());
}

}  // namespace theme

// Changes preview live. "Apply" saves them; "Close" (or the window's X) restores
// the palette that was active when the dialog opened.
class SettingsDialog : public QDialog {
public:
    explicit SettingsDialog(QWidget *parent = nullptr)
        : QDialog(parent), originalPalette_(QApplication::palette()), state_(theme::load()) {
        setWindowTitle("Settings");
        auto *layout = new QVBoxLayout(this);

        layout->addWidget(new QLabel("Select Theme:"));
        auto *themeBox = new QComboBox();
        themeBox->addItems({"Light", "Dark"});
        themeBox->setCurrentText(state_.mode);
        layout->addWidget(themeBox);

        layout->addWidget(new QLabel("Select Accent Color:"));
        auto *colorRow = new QHBoxLayout();
        auto *colorButton = new QPushButton("Choose Color");
        auto *resetButton = new QPushButton("Reset");
        colorRow->addWidget(colorButton);
        colorRow->addWidget(resetButton);
        layout->addLayout(colorRow);

        auto *buttonRow = new QHBoxLayout();
        auto *applyButton = new QPushButton("Apply");
        auto *closeButton = new QPushButton("Close");
        buttonRow->addWidget(applyButton);
        buttonRow->addWidget(closeButton);
        layout->addLayout(buttonRow);

        connect(themeBox, &QComboBox::currentTextChanged, this, [this](const QString &mode) {
            state_.mode = mode;
            preview();
        });
        connect(colorButton, &QPushButton::clicked, this, [this]() {
            const QColor start = state_.accent.isValid() ? state_.accent : palette().highlight().color();
            const QColor chosen = QColorDialog::getColor(start, this, "Select Accent Color");
            if (chosen.isValid()) {
                state_.accent = chosen;
                preview();
            }
        });
        connect(resetButton, &QPushButton::clicked, this, [this]() {
            state_.accent = QColor();
            preview();
        });
        connect(applyButton, &QPushButton::clicked, this, [this]() {
            theme::save(state_);
            accept();
        });
        connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    }

    void reject() override {
        QApplication::setPalette(originalPalette_);
        QDialog::reject();
    }

private:
    void preview() { QApplication::setPalette(theme::build(state_.mode, state_.accent)); }

    QPalette originalPalette_;
    theme::Saved state_;
};

struct Result {
    bool ok = false;
    QString text;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");  // native styles on some platforms ignore palettes
    {
        const theme::Saved saved = theme::load();
        QApplication::setPalette(theme::build(saved.mode, saved.accent));
    }

    QWidget window;
    window.setWindowTitle("Password Generator");
    auto *layout = new QVBoxLayout(&window);

    auto *menuBar = new QMenuBar(&window);
    QMenu *settingsMenu = menuBar->addMenu("Settings");
    QAction *themeAction = settingsMenu->addAction("Customize Appearance");
    layout->setMenuBar(menuBar);

    layout->addWidget(new QLabel(QString("Enter the desired password length (1-%1):").arg(pwgen::kMaxLength)));

    auto *input = new QLineEdit("16");
    layout->addWidget(input);

    auto *upperBox = new QCheckBox("Include Upper-case Letters");
    auto *lowerBox = new QCheckBox("Include Lower-case Letters");
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

    QObject::connect(themeAction, &QAction::triggered, &window, [&window]() {
        SettingsDialog dialog(&window);
        dialog.exec();
    });

    QObject::connect(input, &QLineEdit::returnPressed, generateButton, &QPushButton::click);

    QObject::connect(generateButton, &QPushButton::clicked, &window, [=, &window]() {
        bool ok = false;
        const int length = input->text().trimmed().toInt(&ok);
        if (!ok || length < 1 || length > pwgen::kMaxLength) {
            output->clear();
            copyButton->setEnabled(false);
            status->setStyleSheet("color: #c0392b;");
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

        generateButton->setEnabled(false);
        copyButton->setEnabled(false);
        status->setStyleSheet(QString());
        status->setText("Generating...");

        // Runs on the Qt thread pool; the UI thread stays responsive.
        auto *watcher = new QFutureWatcher<Result>(&window);
        QObject::connect(watcher, &QFutureWatcher<Result>::finished, &window, [=]() {
            const Result r = watcher->result();
            if (r.ok) {
                output->setPlainText(r.text);
                status->clear();
                copyButton->setEnabled(true);
            } else {
                output->clear();
                status->setStyleSheet("color: #c0392b;");
                status->setText(r.text);
            }
            generateButton->setEnabled(true);
            watcher->deleteLater();
        });
        watcher->setFuture(QtConcurrent::run([opts]() {
            Result r;
            try {
                r.text = QString::fromStdString(pwgen::generate(opts));
                r.ok = true;
            } catch (const std::exception &e) {
                r.text = QString::fromUtf8(e.what());
            }
            return r;
        }));
    });

    QObject::connect(copyButton, &QPushButton::clicked, &window, [=]() {
        QApplication::clipboard()->setText(output->toPlainText());
    });

    window.resize(420, 500);
    window.show();
    return app.exec();
}
