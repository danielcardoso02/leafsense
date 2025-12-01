#include <QApplication>
#include <QDebug>
#include "../include/application/gui/login_dialog.h"
#include "../include/application/gui/mainwindow.h"
#include "../include/application/gui/theme/theme_manager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("LeafSense");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("University of Minho");

    ThemeManager& theme_mgr = ThemeManager::instance();
    theme_mgr.set_theme(ThemeMode::LIGHT);

    qDebug() << "\n========================================";
    qDebug() << "LeafSense";
    qDebug() << "Version: 1.0.0";
    qDebug() << "Developers: Daniel Cardoso and Marco Costa";
    qDebug() << "========================================\n";

    MainWindow* main_window = nullptr;

    while (true) {
        LoginDialog login_dialog;

        if (login_dialog.exec() == QDialog::Accepted) {
            qDebug() << "Login successful";
            QString logged_in_user = login_dialog.get_username();
            QString login_time = login_dialog.get_login_time();

            qDebug() << "User:" << logged_in_user;
            qDebug() << "Login Time:" << login_time << "\n";

            // Default plant (Lettuce)
            Plant default_plant;
            default_plant.id = 1;
            default_plant.name = "Lettuce";
            default_plant.location = "";
            default_plant.is_active = true;
            default_plant.status_color = QColor(76, 175, 80);
            default_plant.ideal_ph_min = 6.5;
            default_plant.ideal_ph_max = 7.5;
            default_plant.ideal_temp_min = 20.0;
            default_plant.ideal_temp_max = 25.0;
            default_plant.ideal_ec_min = 1400;
            default_plant.ideal_ec_max = 2000;

            qDebug() << "Plant:" << default_plant.name;
            qDebug() << "Plant ID:" << default_plant.id;
            qDebug() << "Active:" << (default_plant.is_active ? "Yes" : "No") << "\n";

            // Create main window
            main_window = new MainWindow();

            main_window->set_logged_in_user(logged_in_user);
            main_window->set_login_time(login_time);
            main_window->set_selected_plant(default_plant);

            main_window->show();

            qDebug() << "Main monitoring window displayed for plant:" << default_plant.name;
            qDebug() << "========================================\n";

            // Run application (only ONCE, at top level)
            int result = app.exec();

            qDebug() << "Returned from main window";
            qDebug() << "Current theme:" << (theme_mgr.get_current_theme() == ThemeMode::LIGHT ? "LIGHT" : "DARK");
            qDebug() << "Returning to login window...\n";

            // Clean up main window
            if (main_window) {
                delete main_window;
                main_window = nullptr;
            }

            // If result is not 0, user wants to exit completely
            if (result != 0) {
                qDebug() << "Application terminated";
                return result;
            }

        } else {
            qDebug() << "Application closed - login cancelled";
            qDebug() << "Application terminated";
            return 0;
        }
    }

    return 0;
}
