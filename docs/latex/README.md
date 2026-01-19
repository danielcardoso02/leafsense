# LeafSense Implementation Report - LaTeX Documentation

This folder contains the LaTeX source files for the Implementation chapter of the LeafSense project report.

## Files

| File | Description |
|------|-------------|
| `main-implementation.tex` | Main document file (compile this) |
| `implementation-chapter.tex` | The implementation chapter content |
| `README.md` | This file |

## Compilation

### Using pdflatex

```bash
cd docs/latex
pdflatex main-implementation.tex
pdflatex main-implementation.tex  # Run twice for references/TOC
```

### Using latexmk (recommended)

```bash
cd docs/latex
latexmk -pdf main-implementation.tex
```

### Using Overleaf

1. Upload all `.tex` files to your Overleaf project
2. Set `main-implementation.tex` as the main document
3. Add any required images to an `images/` folder
4. Compile

## Required Images/Prints

The document contains placeholders marked with `% [IMAGE PLACEHOLDER: ...]` comments. Below is a complete list of recommended images to add:

### Section 5.2 - Software Architecture

| Location | Suggested Image | Filename |
|----------|-----------------|----------|
| After Section 5.2.1 | Project directory structure diagram/screenshot | `project-structure.png` |
| After Section 5.2.3 | Middleware architecture diagram | `middleware-architecture.png` |

### Section 5.3 - AI Integration

| Location | Suggested Image | Filename |
|----------|-----------------|----------|
| After Section 5.3 | ONNX Runtime logo | `onnx-runtime-logo.png` |
| After Section 5.3 | Inference pipeline flowchart | `inference-pipeline.png` |

### Section 5.6 - Kernel Module

| Location | Suggested Image | Filename |
|----------|-----------------|----------|
| End of Section 5.6 | LED connected to GPIO 20 (ON/OFF states) | `led-gpio-test.jpg` |

### Section 5.7 - GUI

| Location | Suggested Image | Filename |
|----------|-----------------|----------|
| Login Window | Login screen (Light + Dark mode) | `gui-login-light.png`, `gui-login-dark.png` |
| Dashboard | Main dashboard (Light + Dark mode) | `gui-dashboard-light.png`, `gui-dashboard-dark.png` |
| Logs Window | All 4 tabs in both themes | `gui-logs-*.png` |
| Settings | Settings window (Light + Dark mode) | `gui-settings-light.png`, `gui-settings-dark.png` |
| Info Window | Info screen (Light + Dark mode) | `gui-info-light.png`, `gui-info-dark.png` |
| Dialog | Logout confirmation popup | `gui-dialog-light.png`, `gui-dialog-dark.png` |

### Section 5.8 - Sensor Integration (TO BE COMPLETED)

| Location | Suggested Image | Filename |
|----------|-----------------|----------|
| Sensor connections | Photo of sensors wired to RPi | `sensors-connections.jpg` |
| Sensor readings | GUI screenshot with real readings | `sensors-gui-readings.png` |
| Calibration | Calibration procedure graphs | `sensor-calibration.png` |

### Section 5.9 - Actuator Integration (TO BE COMPLETED)

| Location | Suggested Image | Filename |
|----------|-----------------|----------|
| Actuator connections | Photo of pumps/relays connected | `actuators-connections.jpg` |
| Wiring diagram | Relay module wiring schematic | `actuator-wiring.png` |
| Control demo | Screenshot of actuator control | `actuators-control.png` |

### Section 5.10 - System Validation

| Location | Suggested Image | Filename |
|----------|-----------------|----------|
| End of section | Terminal showing boot/startup logs | `system-validation-terminal.png` |

## Adding Images

To add an image, create an `images/` folder in `docs/latex/` and use:

```latex
\begin{figure}[htbp]
    \centering
    \includegraphics[width=0.8\textwidth]{images/your-image.png}
    \caption{Your caption here}
    \label{fig:your-label}
\end{figure}
```

For side-by-side images (Light/Dark mode comparisons):

```latex
\begin{figure}[htbp]
    \centering
    \begin{minipage}{0.48\textwidth}
        \centering
        \includegraphics[width=\textwidth]{images/gui-login-light.png}
        \caption*{(a) Light Mode}
    \end{minipage}
    \hfill
    \begin{minipage}{0.48\textwidth}
        \centering
        \includegraphics[width=\textwidth]{images/gui-login-dark.png}
        \caption*{(b) Dark Mode}
    \end{minipage}
    \caption{LeafSense Login Window}
    \label{fig:login-window}
\end{figure}
```

## Sections To Complete

The following sections are marked as **[To be completed]** and should be filled in when the corresponding hardware integration is finished:

1. **Section 5.2.2 - Hardware Abstraction Layer (HAL)**
   - Sensors subsection
   - Actuators subsection

2. **Section 5.8 - Sensor Integration**
   - pH Sensor (PH-4502C) implementation
   - Temperature Sensor (DS18B20) 1-Wire code
   - TDS Sensor calibration
   - Camera integration with libcamera
   - ADS1115 ADC I2C code
   - DS3231 RTC implementation

3. **Section 5.9 - Actuator Integration**
   - Peristaltic pump GPIO control
   - Water heater relay control
   - Timing algorithms
   - Safety interlocks

## Dependencies

Required LaTeX packages (included in most TeX distributions):

- `inputenc`, `fontenc`, `lmodern` - Encoding
- `babel` - Language
- `geometry` - Page layout
- `graphicx`, `float` - Images
- `booktabs`, `array`, `longtable` - Tables
- `listings`, `xcolor` - Code listings
- `amsmath`, `amssymb` - Math
- `hyperref` - Links
- `pifont` - Checkmarks
- `fancyhdr` - Headers/footers

## Notes

- The document uses chapter 5 numbering to match the original PDF structure
- Tables are designed to match the format in the Implementation chapter of `leafsense_implementation.pdf`
- Code listings use the `lstlisting` environment with custom styling
- All file paths reference the actual project structure
