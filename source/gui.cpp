#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QProcess>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QFileDialog>
#include <QCommandLineParser>
#include <QStandardPaths>

bool isExecutableAvailable(const QString &exeName) {
    return !QStandardPaths::findExecutable(exeName).isEmpty();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Command-line argument parsing
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Program with Device and Media Type Arguments");
    parser.addHelpOption();

    QCommandLineOption deviceOption("device", "Device argument", "device");
    QCommandLineOption mediaTypeOption("mediaType", "Media Type argument", "mediaType");

    parser.addOption(deviceOption);
    parser.addOption(mediaTypeOption);

    parser.process(app);

    QString device = parser.value(deviceOption);
    QString mediaType = parser.value(mediaTypeOption);

    if (device.isEmpty() || mediaType.isEmpty()) {
        qWarning("Both --device and --mediaType arguments are required.");
        return 1;
    }

    // Main Window
    QWidget window;
    window.setWindowTitle("Autoplay");

    // Layout configuration
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // Labels Layout (Device Left, Media Type Right)
    QHBoxLayout *labelsLayout = new QHBoxLayout;
    labelsLayout->setContentsMargins(8, 8, 8, 8);

    QLabel *deviceLabel = new QLabel("<b>Device:</b> " + device);
    QLabel *mediaTypeLabel = new QLabel("<b>Media Type:</b> " + mediaType);

    deviceLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mediaTypeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    labelsLayout->addWidget(deviceLabel, 1, Qt::AlignLeft);
    labelsLayout->addWidget(mediaTypeLabel, 1, Qt::AlignRight);

    mainLayout->addLayout(labelsLayout);

    auto addButton = [&](const QString &text, const QString &iconPath, const QString &command, QStringList args = {}) {
        if (isExecutableAvailable(command)) {
            QPushButton *button = new QPushButton(text);
            button->setIcon(QIcon(iconPath));
            button->setIconSize(QSize(32, 32));
            button->setMinimumWidth(315);
            button->setMinimumHeight(48);
            button->setStyleSheet("text-align: left; padding-left: 8px;");
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

            mainLayout->addWidget(button);

            QObject::connect(button, &QPushButton::clicked, [=]() {
                QProcess::startDetached(command, args);
                QApplication::quit();
            });
        }
    };

    int optionsDetected = 0;

    if (isExecutableAvailable("vlc")) {
        optionsDetected++;
        QString vlcMediaType = mediaType;
        if (vlcMediaType == "bd") {
            vlcMediaType = "bluray";
        }
        addButton(
            "Play with VLC Media Player",
            "/usr/share/icons/hicolor/256x256/apps/vlc.png",
            "vlc",
            {vlcMediaType + "://" + device}
        );
    }

    if ((mediaType == "cd") && (isExecutableAvailable("sound-juicer"))) {
        optionsDetected++;
        addButton(
            "Open Sound Juicer",
            "/usr/share/icons/hicolor/256x256/apps/org.gnome.SoundJuicer.png",
            "sound-juicer"
        );
    }

    if ((mediaType == "cd" || mediaType == "dvd") &&
        (isExecutableAvailable("brasero"))) {
        optionsDetected++;
        addButton(
            "Open Brasero",
            "/usr/share/icons/hicolor/256x256/apps/brasero.png",
            "brasero"
        );
    }

    if ((mediaType == "dvd" || mediaType == "bd") &&
        (isExecutableAvailable("makemkv"))) {
        optionsDetected++;
        addButton(
            "Open MakeMKV",
            "/usr/share/icons/hicolor/256x256/apps/makemkv.png",
            "makemkv"
        );
    }

    if (optionsDetected == 0) {
        QMessageBox::critical(nullptr, "Autoplay Error", "No software options installed.");
        return 1;
    }

    // Set layout and display window
    window.setLayout(mainLayout);
    window.show();

    return app.exec();
}
