#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <fftw3.h>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum MatrixPrintingMode{
        REAL = 0b01,
        IMAGINARY = 0b10,
        FULL_COMPLEX_NUMBER = 0b11
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    //ładuje zdjęcie i przeprowadza inicjalizację macierzy
    void on_pushButton_openPicture_clicked();
    //wykonuje splot
    void on_pushButton_doFFT_clicked();
    //pozbawia zdjęcie wejściowe kolorów (aktualnie nieużywane w fft)
    void rgbToBW();
    //inicjalizuje macierze i wypełnia je zerami
    void initializeMatrices();
    //wypełnia macierz tab zerami
    void clearMatrice(fftw_complex *mat, int width, int height);
    //przygotowuje plany fft i ifft
    void makePlans();
    //wyświtla wynikowy obraz
    void showResult();
    //ładuje wartości z formularz w oknie programu do macierzy maski
    void loadMaskFromForm();
    //ładuje macierz zdjęcia źródłowego
    void loadSourcePictureToMatrix();
    //zapisuje zdjęcie wynikowe
    void saveResult();
    //mnoży tablicowo macierze
    void multiplicateArrays(fftw_complex* A, fftw_complex* B, int width, int height);
    //ustawia zdjęcie wynikowe jako źródłowe i zwalnia pamięć macierzy poprzednich operacji
    void on_pushButton_swap_clicked();
    //zapisuje macierz w formie tekstowej
    void printMatrixAsTextFile(fftw_complex *tab,int width, int height, QString filename,MatrixPrintingMode mode = REAL, QString valueSeparator =" ", QString lineSeparator = "\n");
    //znajdź największą wartość w tablicy
    double findMax(fftw_complex* tab,int size);
    //tworzy domyślne maski
    void makePredefinedMasks();
    //załaduj maskę z listy
    void loadMaskFromCombobox(int index);
    //ustawia "Własne", w comboboxie z maskami po edycji któregokolwiek z lineEditów Maski
    void maskEditsChanged();

private:
    Ui::MainWindow *ui;
    fftw_complex *in, *out;
    fftw_complex *inMask, *outMask;
    fftw_complex *final;
    QImage SourceImage;
    QImage MaskedImage;
    int OutWidth;
    int OutHeight;
    fftw_plan PlanForIFFT;
    fftw_plan PlanForMainPicture;
    fftw_plan PlanForMask;
    QList<QMap<QString,QString> > PredefinedMasks;
    int CurrentMask;

};

#endif // MAINWINDOW_H
