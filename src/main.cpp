#include <QApplication>
#include <QDialog>
#include <QDebug>
#include <pthread.h>
#include "../include/middleware/MQueueHandler.h"
#include "../include/application/gui/mainwindow.h"
#include "../include/application/gui/login_dialog.h"
#include "../include/application/gui/theme/theme_manager.h"
#include "../include/middleware/MQueueHandler.h"
#include "../include/middleware/dDatabase.h"
#include "../include/middleware/Master.h"

// Global Pointers
Master* systemMaster = nullptr;
dDatabase* dbDaemon = nullptr;
MQueueHandler* mqueueToDB = nullptr;
pthread_t tDatabase;

// Daemon Wrapper
void* dbDaemonFunc(void* arg) {
    ((dDatabase*)arg)->run();
    return NULL;
}

// Cleanup
void performCleanup() {
    qDebug() << "[System] Stopping backend services...";
    if (systemMaster) systemMaster->stop();
    if (dbDaemon) dbDaemon->stop();
    pthread_join(tDatabase, NULL);
    delete systemMaster; 
    delete dbDaemon; 
    delete mqueueToDB;
    qDebug() << "[System] Cleanup done.";
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("LeafSense");

    // 1. Start Backend (Simulation Mode)
    // This starts the threads that generate fake data and write to DB
    mqueueToDB = new MQueueHandler();
    dbDaemon = new dDatabase(mqueueToDB, "leafsense.db"); 
    pthread_create(&tDatabase, NULL, dbDaemonFunc, (void*)dbDaemon);
    
    systemMaster = new Master(mqueueToDB);
    systemMaster->start(); 

    // 2. Start GUI
    ThemeManager::instance().set_theme(ThemeMode::LIGHT);

    while (true) {
        LoginDialog login;
        if (login.exec() == QDialog::Accepted) {
            MainWindow* w = new MainWindow();
            w->set_logged_in_user(login.get_username());
            w->set_login_time(login.get_login_time());
            
            Plant p; p.id=1; p.name="Lettuce"; 
            w->set_selected_plant(p);
            w->show();
            
            int res = app.exec();
            delete w;
            
            if (res != 0) { performCleanup(); return res; }
        } else {
            performCleanup(); return 0;
        }
    }
}