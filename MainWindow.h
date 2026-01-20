#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QScrollArea>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <dcmtk/dcmimgle/dcmimage.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /** 
     * @brief Construtor da janela principal
     * @param parent Widget pai (padrão: nullptr) 
    **/
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

    /** 
     * @brief Carrega um arquivo DICOM e exibe na tela
     * @param filename Caminho completo do arquivo DICOM a carregar
    **/
    void loadDicom(const QString &filename);

protected:
    /** 
     * @brief Manipulador de evento de redimensionamento da janela
     * @param event Evento de redimensionamento
    **/
    void resizeEvent(QResizeEvent *event) override;

private slots:
    /**
     * @brief Slot acionado quando o slider de Window Level muda
     * @param value Novo valor do Window Level
    **/
    void onWindowLevelChanged(int value);

    /**
     * @brief Slot acionado quando o slider de Window Width muda
     * @param value Novo valor do Window Width
    **/
    void onWindowWidthChanged(int value);

    /**
     * @brief Slot para aumentar o zoom da imagem
    **/
    void onZoomIn();

    /**
     * @brief Slot para diminuir o zoom da imagem
    **/
    void onZoomOut();

    /**
     * @brief Slot para resetar o zoom da imagem ao valor padrão
    **/
    void onResetZoom();

private:
    /**
     * @brief Atualiza a exibição da imagem DICOM com os parâmetros atuais
    **/
    void updateDisplay();

    /**
     * @brief Cria o painel de controle com sliders e botões
    **/
    void createControlPanel();

    /**
     * @brief Atualiza o rótulo que exibe o nível de zoom atual
    **/
    void updateZoomLabel();

    Ui::MainWindow *ui;

    QScrollArea *scroll_area;
    QLabel *image_label;

    QImage dicom_image;
    QString current_file;
    
    // Controles Window Level/Width
    QSlider *slider_window_level;
    QSlider *slider_window_width;
    QLabel *label_window_level;
    QLabel *label_window_width;
    QPushButton *resetWLWWButton;

    // Metadados
    int rows = 0;
    int cols = 0;
    int bits_stored = 0;
    int window_width = 0;
    int window_level = 0;
    int default_window_width = 400;
    int default_window_level = 0;
    QString photometric;
    QString modality;
    
    // Zoom
    double zoom_factor = 1.0;
    double default_zoom_factor = 1.0;
    const double zoom_step = 0.1;
    const double min_zoom = 0.1;
    const double max_zoom = 5.0;
    QLabel *label_zoom;
    
    // Imagem original
    DicomImage *current_dicom_image = nullptr;
};

#endif // MAINWINDOW_H