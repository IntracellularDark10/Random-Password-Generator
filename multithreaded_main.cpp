#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QMenuBar>
#include <QThread>
#include <random>
#include <string>
#include <thread>
#include <future>
#include <vector>
#include <QDialog>
#include <QComboBox>
#include <QColorDialog>
#include <QPalette>
#include <QPushButton>

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

void generatePasswordThread(int length, bool includeUpper, bool includeLower, bool includeDigits, bool includeSpecial, std::promise<std::string>&& passwordPromise) {
    std::string password = generatePassword(length, includeUpper, includeLower, includeDigits, includeSpecial);
    passwordPromise.set_value(password);
}

// Settings dialog class for theme and color selection
class SettingsDialog : public QDialog {
    Q_OBJECT  // Added Q_OBJECT macro

public:
    SettingsDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Settings");

        QVBoxLayout *layout = new QVBoxLayout(this);

        QLabel *themeLabel = new QLabel("Select Theme:");
        layout->addWidget(themeLabel);

        // Dropdown for light/dark theme selection
        QComboBox *themeComboBox = new QComboBox();
        themeComboBox->addItem("Light");
        themeComboBox->addItem("Dark");
        layout->addWidget(themeComboBox);

        QLabel *accentColorLabel = new QLabel("Select Accent Color:");
        layout->addWidget(accentColorLabel);

        // Button to open color picker dialog
        QPushButton *colorButton = new QPushButton("Choose Color");
        layout->addWidget(colorButton);

        // Apply and close buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *applyButton = new QPushButton("Apply");
        QPushButton *closeButton = new QPushButton("Close");
        buttonLayout->addWidget(applyButton);
        buttonLayout->addWidget(closeButton);
        layout->addLayout(buttonLayout);

        // Signals and slots for theme and color selection
        connect(themeComboBox, &QComboBox::currentTextChanged, this, [this, themeComboBox]() {
            if (themeComboBox->currentText() == "Dark") {
                applyDarkTheme();
            } else {
                applyLightTheme();
            }
        });

        connect(colorButton, &QPushButton::clicked, this, [this]() {
            QColor chosenColor = QColorDialog::getColor(Qt::white, this, "Select Accent Color");
            if (chosenColor.isValid()) {
                applyAccentColor(chosenColor);
            }
        });

        connect(applyButton, &QPushButton::clicked, this, &QDialog::accept);
        connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    }

private:
    void applyDarkTheme() {
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        QApplication::setPalette(darkPalette);
    }

    void applyLightTheme() {
        QApplication::setPalette(QApplication::style()->standardPalette());
    }

    void applyAccentColor(const QColor &color) {
        QPalette palette = QApplication::palette();
        palette.setColor(QPalette::Highlight, color);
        QApplication::setPalette(palette);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Password Generator");

    QVBoxLayout *layout = new QVBoxLayout(&window);

    // Menu bar for settings
    QMenuBar *menuBar = new QMenuBar(&window);
    QMenu *settingsMenu = menuBar->addMenu("☰ Settings");
    QAction *themeAction = new QAction("Customize Appearance", &window);
    settingsMenu->addAction(themeAction);
    layout->setMenuBar(menuBar);

    QLabel *label = new QLabel("Enter the desired password length:");
    layout->addWidget(label);

    QLineEdit *lengthInput = new QLineEdit();
    layout->addWidget(lengthInput);

    // Checkbox options for password criteria
    QCheckBox *upperCaseCheckBox = new QCheckBox("Include Upper-case Letters");
    QCheckBox *lowerCaseCheckBox = new QCheckBox("Include Lower-case Letters");
    QCheckBox *digitsCheckBox = new QCheckBox("Include Digits");
    QCheckBox *specialCharsCheckBox = new QCheckBox("Include Special Characters");

    layout->addWidget(upperCaseCheckBox);
    layout->addWidget(lowerCaseCheckBox);
    layout->addWidget(digitsCheckBox);
    layout->addWidget(specialCharsCheckBox);

    // Button to generate password
    QPushButton *generateButton = new QPushButton("Generate Password");
    layout->addWidget(generateButton);

    // Text area to display the generated password
    QTextEdit *passwordDisplay = new QTextEdit();
    layout->addWidget(passwordDisplay);

    // Event to open the settings dialog
    QObject::connect(themeAction, &QAction::triggered, [&]() {
        SettingsDialog settingsDialog(&window);
        settingsDialog.exec();
    });

    QObject::connect(generateButton, &QPushButton::clicked, [&]() {
        int length = lengthInput->text().toInt();
        bool includeUpper = upperCaseCheckBox->isChecked();
        bool includeLower = lowerCaseCheckBox->isChecked();
        bool includeDigits = digitsCheckBox->isChecked();
        bool includeSpecial = specialCharsCheckBox->isChecked();

        // Create a promise and future to handle multithreading
        std::promise<std::string> passwordPromise;
        std::future<std::string> passwordFuture = passwordPromise.get_future();

        // Run password generation in a separate thread
        std::thread passwordThread(generatePasswordThread, length, includeUpper, includeLower, includeDigits, includeSpecial, std::move(passwordPromise));
        passwordThread.detach();  // Detach thread to run independently

        // Wait for the result from the future and display it
        passwordDisplay->setText(QString::fromStdString(passwordFuture.get()));
    });

    window.show();
    return app.exec();
}

#include "moc_multithreaded_with_settings_main.cpp"  // Include the MOC generated file

