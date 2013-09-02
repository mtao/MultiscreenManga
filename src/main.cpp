#include "include/mainwindow.h"
#include "include/remotesyncserver.h"
#include <QApplication>
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QStringList args = a.arguments();
    MainWindow w ;
    bool rootSet = false;
    for(auto && it = args.constBegin()+1; it < args.constEnd(); ++it) {
        qWarning() << "Processing line: " << *it;
        if (it->startsWith(QObject::tr("--root-dir"))) {
             w.setRoot(static_cast<QStringList>(
                           it->split(QObject::tr("=")).mid(1)
                           ).join(QObject::tr(""))
                       );
             rootSet = true;
        } else if (*it == "--server-only") {
            RemoteSyncServer serv;
            serv.run();
            return 0;
        } else {
            if (QFileInfo(*it).exists()) {
                w.openRootVolume(*it, !rootSet);
            }
        }
    }
    w.show();

    return a.exec();
}
