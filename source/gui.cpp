#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
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
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    // Button Style
    QSize buttonSize(300, 48);
    QSize iconSize(32, 32);

    if (isExecutableAvailable("vlc")) {
        QPushButton *playButton = new QPushButton("Play with VLC Media Player");
        playButton->setIcon(QIcon("/usr/share/icons/hicolor/256x256/apps/vlc.png"));
        playButton->setIconSize(iconSize);
        playButton->setFixedSize(buttonSize);
        playButton->setStyleSheet("text-align: left; padding-left: 8px;");
        layout->addWidget(playButton);

        QObject::connect(playButton, &QPushButton::clicked, [&]() {
            if (mediaType == "bd") {
                mediaType = "bluray";
            }
            QString vlcArg = mediaType + "://" + device;
            QProcess::startDetached("vlc", {vlcArg});
            QApplication::quit();
        });
    }

    if (isExecutableAvailable("brasero")) {
        QPushButton *braseroButton = new QPushButton("Open Brasero");
        braseroButton->setIcon(QIcon("/usr/share/icons/hicolor/256x256/apps/brasero.png"));
        braseroButton->setIconSize(iconSize);
        braseroButton->setFixedSize(buttonSize);
        braseroButton->setStyleSheet("text-align: left; padding-left: 8px;");
        layout->addWidget(braseroButton);
        
        QObject::connect(braseroButton, &QPushButton::clicked, [&]() {
            QProcess::startDetached("brasero", QStringList());
            QApplication::quit();
        });
    }

    if (isExecutableAvailable("makemkv-backup")) {
        QPushButton *backupButton = new QPushButton("Backup with MakeMKV");
        backupButton->setIcon(QIcon("/usr/share/icons/hicolor/256x256/apps/makemkv.png"));
        backupButton->setIconSize(iconSize);
        backupButton->setFixedSize(buttonSize);
        backupButton->setStyleSheet("text-align: left; padding-left: 8px;");
        layout->addWidget(backupButton);

        QObject::connect(backupButton, &QPushButton::clicked, [&]() {
            QString folderPath = QFileDialog::getSaveFileName(
                nullptr,
                "Select or Create Destination Folder",
                QDir::homePath() + "/NewFolderName",
                QString(),
                nullptr,
                QFileDialog::DontConfirmOverwrite
            );

            if (!folderPath.isEmpty()) {
                QDir dir(folderPath);
                if (!dir.exists()) {
                    dir.mkpath(".");
                }
                QString deviceArg = "--device=" + device;
                QString pathArg = "--path=" + folderPath;
                QProcess::startDetached("makemkv-backup", {deviceArg, pathArg});
                QApplication::quit();
            }
        });
    }

    if (layout->count() == 0) {
        QMessageBox::critical(nullptr, "Error", "Neither VLC, Brasero nor MakeMKV is installed.");
        return 1;
    }

    // Set layout and display window
    window.setLayout(layout);
    window.show();

    return app.exec();
}
