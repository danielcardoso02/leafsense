# LeafSense - Interface Gráfica (Qt5)

## Overview

A interface gráfica do LeafSense é desenvolvida em Qt5 e fornece visualização em tempo real dos dados do sistema, controlo manual e configurações. A interface é otimizada para o display Waveshare 3.5" (480x320) com suporte a touchscreen.

## Display Configuration

### Waveshare 3.5" LCD (C)
| Property | Value |
|----------|-------|
| Resolution | 480x320 |
| Controller | ILI9486 (SPI) |
| Touch | ADS7846 (SPI) |
| Framebuffer | /dev/fb1 |
| Rotation | 90° (landscape) |

### Qt Platform Plugin
```bash
# Environment variables for Waveshare touchscreen
export QT_QPA_PLATFORM=linuxfb
export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/lib/qt5/plugins
export QT_QPA_FONTDIR=/usr/share/fonts
export QT_QPA_MOUSEDRIVER=linuxinput

# Launch command
./LeafSense -platform linuxfb:fb=/dev/fb1
```

### Framebuffer Devices
| Device | Controller | Use |
|--------|-----------|-----|
| /dev/fb0 | BCM2708 (GPU) | HDMI output |
| /dev/fb1 | ILI9486 (SPI) | Waveshare LCD |

## Stack de UI

| Componente | Library |
|------------|------------|
| Framework | Qt 5.15.14 |
| Widgets | Qt5Widgets |
| Charts | Qt5Charts |
| Database | Qt5Sql |
| SVG | Qt5Svg |
| Platform | linuxfb (framebuffer) |

## Estrutura de Ficheiros

```
include/application/gui/
├── mainwindow.h           # Janela principal
├── sensors_display.h      # Display de sensores
├── health_display.h       # Display de saúde (ML)
├── alerts_display.h       # Display de alertas
├── analytics_window.h     # Janela de analytics
├── settings_window.h      # Janela de configurações
├── logs_window.h          # Janela de logs
├── login_dialog.h         # Diálogo de login
├── info_window.h          # Janela de informações
└── leafsense_data_bridge.h # Bridge de dados

src/application/gui/
├── mainwindow.cpp
├── sensors_display.cpp
├── health_display.cpp
├── alerts_display.cpp
├── analytics_window.cpp
├── settings_window.cpp
├── logs_window.cpp
├── login_dialog.cpp
├── info_window.cpp
└── leafsense_data_bridge.cpp
```

## Componentes Principais

### MainWindow
Janela principal com sistema de tabs.

```cpp
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    
private:
    QTabWidget *tabWidget;
    SensorsDisplay *sensorsTab;
    HealthDisplay *healthTab;
    AlertsDisplay *alertsTab;
    
    QMenuBar *menuBar;
    QToolBar *toolBar;
    QStatusBar *statusBar;
    
    void setupUI();
    void setupMenus();
    void setupConnections();
};
```

### SensorsDisplay
Mostra dados de sensores em tempo real.

```cpp
class SensorsDisplay : public QWidget {
    Q_OBJECT
public:
    explicit SensorsDisplay(QWidget *parent = nullptr);
    
public slots:
    void updateTemperature(double value);
    void updatePH(double value);
    void updateEC(double value);
    
private:
    // Gauges/Progress bars
    QProgressBar *tempGauge;
    QProgressBar *phGauge;
    QProgressBar *ecGauge;
    
    // Labels
    QLabel *tempLabel;
    QLabel *phLabel;
    QLabel *ecLabel;
    
    // Charts
    QtCharts::QChart *tempChart;
    QtCharts::QLineSeries *tempSeries;
};
```

### HealthDisplay
Mostra resultados de ML e estado de saúde.

```cpp
class HealthDisplay : public QWidget {
    Q_OBJECT
public:
    explicit HealthDisplay(QWidget *parent = nullptr);
    
public slots:
    void updatePrediction(const QString& className, double confidence);
    void updateImage(const QImage& image);
    
private:
    QLabel *imageLabel;
    QLabel *predictionLabel;
    QLabel *confidenceLabel;
    QProgressBar *confidenceBar;
    
    // History de predições
    QTableWidget *historyTable;
};
```

### AlertsDisplay
Lista e gestão de alertas.

```cpp
class AlertsDisplay : public QWidget {
    Q_OBJECT
public:
    explicit AlertsDisplay(QWidget *parent = nullptr);
    
public slots:
    void addAlert(const QString& type, const QString& severity,
                  const QString& message);
    void markAsRead(int alertId);
    void clearAll();
    
signals:
    void alertClicked(int alertId);
    
private:
    QTableWidget *alertsTable;
    QLabel *unreadCountLabel;
    
    QPushButton *markReadBtn;
    QPushButton *clearBtn;
};
```

### AnalyticsWindow
Charts históricos e análise.

```cpp
class AnalyticsWindow : public QDialog {
    Q_OBJECT
public:
    explicit AnalyticsWindow(QWidget *parent = nullptr);
    
private:
    // Charts
    QtCharts::QChartView *tempChartView;
    QtCharts::QChartView *phChartView;
    QtCharts::QChartView *ecChartView;
    
    // Filtros
    QDateEdit *startDate;
    QDateEdit *endDate;
    QComboBox *periodSelector;
    
    void loadData();
    void createCharts();
};
```

### LeafSenseDataBridge
Ponte entre backend e UI.

```cpp
class LeafSenseDataBridge : public QObject {
    Q_OBJECT
public:
    static LeafSenseDataBridge& instance();
    
signals:
    // Sensor updates
    void sensorDataUpdated(double temp, double ph, double ec);
    void temperatureChanged(double value);
    void phChanged(double value);
    void ecChanged(double value);
    
    // ML updates
    void predictionReady(const QString& className, double confidence);
    void imageUpdated(const QImage& image);
    
    // Alerts
    void alertTriggered(const QString& type, const QString& severity,
                        const QString& message);
    void alertCountChanged(int count);
    
    // Status
    void connectionStatusChanged(bool connected);
    void errorOccurred(const QString& error);
    
public slots:
    void processSensorData(double temp, double ph, double ec);
    void processMLPrediction(const QString& result, double confidence);
    void processAlert(const QString& type, const QString& severity,
                      const QString& message);
};
```

## Temas

### Light Mode (Default)
```cpp
QString lightStyle = R"(
    QMainWindow, QDialog {
        background-color: #f8f8f8;
    }
    QWidget {
        background-color: #f8f8f8;
        color: #212121;
        font-family: "Arial";
        font-size: 9pt;
    }
    QPushButton {
        background-color: #4CAF50;
        color: white;
        border: none;
        border-radius: 4px;
        padding: 4px;
        font-weight: bold;
        min-height: 32px;
    }
    QPushButton:hover {
        background-color: #66BB6A;
    }
    QProgressBar {
        border: 1px solid #66cdaa;
        border-radius: 4px;
        background-color: #ffffff;
    }
    QProgressBar::chunk {
        background-color: #4CAF50;
        border-radius: 3px;
    }
    QTabWidget::pane {
        border: 1px solid #66cdaa;
    }
    QTabBar::tab {
        background: #e6e6e6;
        color: #212121;
        min-width: 140px;
        padding: 8px 0px;
    }
    QTabBar::tab:selected {
        background: #ffffff;
        font-weight: bold;
        border-bottom: 2px solid #4CAF50;
    }
)";
```

### Dark Mode
```cpp
QString darkStyle = R"(
    QMainWindow, QDialog {
        background-color: #1e1e1e;
    }
    QWidget {
        background-color: #1e1e1e;
        color: #e0e0e0;
        font-family: "Arial";
        font-size: 9pt;
    }
    QPushButton {
        background-color: #4CAF50;
        color: white;
        border: none;
        border-radius: 4px;
        padding: 4px;
        font-weight: bold;
    }
    QTabBar::tab {
        background: #2d2d2d;
        color: #e0e0e0;
    }
    QTabBar::tab:selected {
        background: #3d3d3d;
        border-bottom: 2px solid #4CAF50;
    }
)";
```

## Recursos (QRC)

### Ficheiro: `resources/resources.qrc`
```xml
<!DOCTYPE RCC>
<RCC version="1.0">
    <qresource prefix="/">
        <file>images/logo.png</file>
        <file>images/leaf_healthy.png</file>
        <file>images/leaf_disease.png</file>
        <file>images/sensor_temp.svg</file>
        <file>images/sensor_ph.svg</file>
        <file>images/sensor_ec.svg</file>
        <file>images/alert_low.svg</file>
        <file>images/alert_high.svg</file>
        <file>images/alert_critical.svg</file>
    </qresource>
</RCC>
```

### Usage em Código
```cpp
QPixmap logo(":/images/logo.png");
QIcon alertIcon(":/images/alert_critical.svg");
```

## Layouts

### Dashboard Layout
```
┌────────────────────────────────────────────────────────┐
│  Menu Bar                                              │
├────────────────────────────────────────────────────────┤
│  Tool Bar                                              │
├────────────────────────────────────────────────────────┤
│  ┌─────────┬─────────┬──────────┐                      │
│  │ Sensors │ Health  │  Alerts  │  (Tabs)              │
│  └─────────┴─────────┴──────────┘                      │
│  ┌──────────────────────────────────────────────────┐  │
│  │                                                  │  │
│  │                 Tab Content                      │  │
│  │                                                  │  │
│  │  ┌─────────┐  ┌─────────┐  ┌─────────┐         │  │
│  │  │  Temp   │  │   pH    │  │   EC    │         │  │
│  │  │  23.5°C │  │   6.2   │  │  1250   │         │  │
│  │  │  [====] │  │  [====] │  │  [====] │         │  │
│  │  └─────────┘  └─────────┘  └─────────┘         │  │
│  │                                                  │  │
│  │  ┌───────────────────────────────────────────┐  │  │
│  │  │           Chart (Temperature)              │  │  │
│  │  │                                            │  │  │
│  │  └───────────────────────────────────────────┘  │  │
│  │                                                  │  │
│  └──────────────────────────────────────────────────┘  │
├────────────────────────────────────────────────────────┤
│  Status Bar: Connected | Last update: 12:34:56         │
└────────────────────────────────────────────────────────┘
```

## Conexões Signal/Slot

```cpp
// MainWindow::setupConnections()
void MainWindow::setupConnections() {
    auto& bridge = LeafSenseDataBridge::instance();
    
    // Sensor updates
    connect(&bridge, &LeafSenseDataBridge::sensorDataUpdated,
            sensorsTab, &SensorsDisplay::updateAllSensors);
    
    connect(&bridge, &LeafSenseDataBridge::temperatureChanged,
            sensorsTab, &SensorsDisplay::updateTemperature);
    
    // ML updates
    connect(&bridge, &LeafSenseDataBridge::predictionReady,
            healthTab, &HealthDisplay::updatePrediction);
    
    // Alerts
    connect(&bridge, &LeafSenseDataBridge::alertTriggered,
            alertsTab, &AlertsDisplay::addAlert);
    
    connect(&bridge, &LeafSenseDataBridge::alertCountChanged,
            this, &MainWindow::updateAlertBadge);
    
    // Menu actions
    connect(analyticsAction, &QAction::triggered,
            this, &MainWindow::showAnalytics);
    
    connect(settingsAction, &QAction::triggered,
            this, &MainWindow::showSettings);
}
```

## Plataformas Qt

### Desktop (X11/Wayland)
```bash
# Pattern - detecta automaticamente
./LeafSense

# Forçar X11
QT_QPA_PLATFORM=xcb ./LeafSense
```

### Raspberry Pi (HDMI)
```bash
# Framebuffer direto
export QT_QPA_PLATFORM=linuxfb:fb=/dev/fb0
./LeafSense
```

### Raspberry Pi (Sem Display)
```bash
# Modo offscreen (para testes ou headless)
export QT_QPA_PLATFORM=offscreen
./LeafSense
```

### VNC (Acesso Remoto)
```bash
# Servidor VNC integrado
export QT_QPA_PLATFORM=vnc:port=5900
./LeafSense

# Cliente conecta em: vnc://pi-ip:5900
```

## CMake Configuration

```cmake
# Qt5
find_package(Qt5 COMPONENTS 
    Core 
    Gui 
    Widgets 
    Sql 
    Charts 
    Svg 
    REQUIRED
)

# MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Sources
set(GUI_SOURCES
    src/application/gui/mainwindow.cpp
    src/application/gui/sensors_display.cpp
    src/application/gui/health_display.cpp
    src/application/gui/alerts_display.cpp
    src/application/gui/analytics_window.cpp
    src/application/gui/settings_window.cpp
    src/application/gui/logs_window.cpp
    src/application/gui/leafsense_data_bridge.cpp
)

# Resources
set(RESOURCES
    resources/resources.qrc
)

# Link
target_link_libraries(LeafSense PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Charts
    Qt5::Svg
)
```
