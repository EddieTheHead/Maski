#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qdebug.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    makePredefinedMasks();
    connect(ui->comboBox_predefinieMasks,SIGNAL(currentIndexChanged(int)),this,SLOT(loadMaskFromCombobox(int)));
    connect(ui->lineEdit_a1,SIGNAL(editingFinished()),this,SLOT(maskEditsChanged()));
    connect(ui->lineEdit_a2,SIGNAL(editingFinished()),this,SLOT(maskEditsChanged()));
    connect(ui->lineEdit_a3,SIGNAL(editingFinished()),this,SLOT(maskEditsChanged()));
    connect(ui->lineEdit_b1,SIGNAL(editingFinished()),this,SLOT(maskEditsChanged()));
    connect(ui->lineEdit_b2,SIGNAL(editingFinished()),this,SLOT(maskEditsChanged()));
    connect(ui->lineEdit_b3,SIGNAL(editingFinished()),this,SLOT(maskEditsChanged()));
    connect(ui->lineEdit_c1,SIGNAL(editingFinished()),this,SLOT(maskEditsChanged()));
    connect(ui->lineEdit_c2,SIGNAL(editingFinished()),this,SLOT(maskEditsChanged()));
    connect(ui->lineEdit_c3,SIGNAL(editingFinished()),this,SLOT(maskEditsChanged()));
}

MainWindow::~MainWindow()
{
    fftw_free(in);
    fftw_free(out);
    fftw_free(inMask);
    fftw_free(outMask);
    fftw_free(final);

    fftw_destroy_plan(PlanForMainPicture);
    fftw_destroy_plan(PlanForIFFT);
    fftw_destroy_plan(PlanForMask);
    delete ui;
}

void MainWindow::on_pushButton_openPicture_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Open File"), QDir::currentPath());
    if (!fileName.isEmpty())
    {
        SourceImage = QImage(fileName);
        if (SourceImage.isNull())
        {
            QMessageBox::information(this, tr("Błąd ładowania"),
                                     tr("Nie mogę załadować %1").arg(fileName));
            return;
        }
        ui->label_Picture->setPixmap(QPixmap::fromImage(SourceImage));
        ui->label_Picture->adjustSize();

        OutWidth = SourceImage.width()+2;
        OutHeight = SourceImage.height()+2;
        initializeMatrices();
        makePlans();
    }
}


void MainWindow::on_pushButton_doFFT_clicked()
{
    //kopiowanie danych z głównego okna do rozszerzonej macierzy
    loadMaskFromForm();
    //kopiowanie obrazu do rozszerzenoj maceirzy
    loadSourcePictureToMatrix();
    //fft na rozszerzonych macierzach  
    fftw_execute(PlanForMainPicture);
    fftw_execute(PlanForMask);

    //mnożenie tablicowe
    multiplicateArrays(out,outMask,OutWidth,OutHeight);

    //ifft
    fftw_execute(PlanForIFFT);

    //normalizacja
    double factor = 1.0/(OutWidth*OutHeight);
    for(int i=0; i<OutWidth; ++i)
    {
        for(int k= 0; k<OutHeight; ++k)
        {
            final[i*OutWidth + k][0] *= factor;
            final[i*OutWidth + k][1] *= factor;
            //pominięcie znikomych wartości
            if(final[i*OutWidth + k][0] < 0.01) final[i*OutWidth + k][0] =0;
            if(final[i*OutWidth + k][1] < 0.01) final[i*OutWidth + k][1] =0;
        }
    }

    //wyświetlenie wyniku
    showResult();

//    saveResult();
//    printMatrixAsTextFile(outMask,OutWidth,OutHeight,"maska po fft.txt",FULL_COMPLEX_NUMBER,",",";");
//    printMatrixAsTextFile(final,OutWidth,OutHeight,"finalna macierz.txt",FULL_COMPLEX_NUMBER," ","\n");
}

//nieużywane
void MainWindow::rgbToBW()
{
    for (int i = 0; i < SourceImage.height(); ++i)
    {
        uchar* scan = SourceImage.scanLine(i);
        int depth =4;
        for (int k = 0; k < SourceImage.width(); ++k)
        {

            QRgb* rgbpixel = reinterpret_cast<QRgb*>(scan + k*depth);
            int gray = qGray(*rgbpixel);
            *rgbpixel = QColor(gray, gray, gray).rgb();
        }
    }
}

void MainWindow::initializeMatrices()
{
    //inicjalizaja tablic dla fft zdjęcia
    in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (OutWidth* OutHeight));
    out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (OutWidth * OutHeight ));

    //inicjalizacja tablic dla fft maski
    inMask = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (OutWidth * OutHeight ));
    outMask = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * (OutWidth * OutHeight ));

    //inicjalizacja tablicy dla finalnego zdjęcia
    final = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * OutWidth*OutHeight);

    //zerowanie tablic
    clearMatrice(final,OutWidth,OutHeight);
    clearMatrice(inMask,OutWidth,OutHeight);
    clearMatrice(outMask,OutWidth,OutHeight);
    clearMatrice(in,OutWidth,OutHeight);
    clearMatrice(out,OutWidth,OutHeight);

}

void MainWindow::clearMatrice(fftw_complex* mat,int width,int height)
{
    for(int i=0; i<width; ++i)
    {
        for(int k= 0; k<height; ++k)
        {
            mat[k*width + i][0] = 0;
            mat[k*width + i][1] = 0;
        }
    }
}

void MainWindow::makePlans()
{
    PlanForMainPicture = fftw_plan_dft_2d(OutWidth,OutHeight, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    PlanForMask = fftw_plan_dft_2d(OutWidth,OutHeight, inMask, outMask, FFTW_FORWARD, FFTW_ESTIMATE);
    PlanForIFFT = fftw_plan_dft_2d(OutWidth,OutHeight, out, final, FFTW_BACKWARD, FFTW_ESTIMATE);

}

void MainWindow::showResult()
{


//     MaskedImage = QImage(OutWidth,OutHeight,QImage::Format_Indexed8);
//     int max = findMax(final,OutHeight*OutWidth);
//     for(int i=0; i<max; ++i)
//     {
//         MaskedImage.setColor(i,qRgb(i,i,i));
//     }

    MaskedImage = QImage(OutWidth,OutHeight,QImage::Format_RGB32);
    for(int x = 0; x < OutWidth; x++)
    {
        for(int y = 0; y < OutHeight; y++)
        {
//            double greyValue = final[y * OutWidth + x][0];
//            uint rgb = QColor(greyValue,greyValue,greyValue).rgb();
//            MaskedImage.setPixel(x,y,rgb);

            unsigned int grey = (unsigned int) final[y * OutWidth + x][0];
//            unsigned long int argb = ((0x00) << 24) + (grey<<16) + (grey<<8) + grey;
            unsigned int rgb = grey * 0x00010101;
            //rgb = rgb | 0x11000000;
            MaskedImage.setPixel(x,y,rgb);

            //MaskedImage.setPixel(x,y,(*reinterpret_cast<uint*>(&grey) ));
            //MaskedImage.setPixel(x,y,(*reinterpret_cast<uint*>(&greyValue) | 0xff000000));
        }
    }
    ui->label_FFT->setPixmap(QPixmap::fromImage(MaskedImage));
}

void MainWindow::loadSourcePictureToMatrix()
{
    for(int x = 0; x < SourceImage.width(); x++)
    {
        for(int y = 0; y < SourceImage.height(); y++)
        {
            unsigned int i = SourceImage.pixel(x,y);
            // rgb-> int 0-255
            int Gray = qGray(i);
            // int->complex (double)
            in[y * OutWidth + x][0] = double(Gray);
            in[y * OutWidth + x][1] = 0;

//            QColor color = SourceImage.pixel(x,y);
//            double R = color.redF();
//            double G = color.greenF();
//            double B = color.blueF();
//            double gray = 0.2989 * R + 0.5870 * G + 0.1140 * B;
//            double gray = (R+B+G)/3;
//            in[y * OutWidth + x][0] = *reinterpret_cast<double*>(&i);

        }
    }
}

void MainWindow::loadMaskFromForm()
{

//       y          x      re
//        \          \    /
    inMask[0*OutWidth+0][0] = ui->lineEdit_a1->text().toDouble();
    inMask[0*OutWidth+1][0] = ui->lineEdit_a2->text().toDouble();
    inMask[0*OutWidth+2][0] = ui->lineEdit_a3->text().toDouble();
    inMask[1*OutWidth+0][0] = ui->lineEdit_b1->text().toDouble();
    inMask[1*OutWidth+1][0] = ui->lineEdit_b2->text().toDouble();
    inMask[1*OutWidth+2][0] = ui->lineEdit_b3->text().toDouble();
    inMask[2*OutWidth+0][0] = ui->lineEdit_c1->text().toDouble();
    inMask[2*OutWidth+1][0] = ui->lineEdit_c2->text().toDouble();
    inMask[2*OutWidth+2][0] = ui->lineEdit_c3->text().toDouble();

    printf("Zaladowana maska: \n %f %f %f \n %f %f %f \n %f %f %f \n", inMask[0*OutWidth+0][0],inMask[0*OutWidth+1][0],inMask[0*OutWidth+2][0],
                                                                    inMask[1*OutWidth+0][0],inMask[1*OutWidth+1][0],inMask[1*OutWidth+2][0],
                                                                    inMask[2*OutWidth+0][0],inMask[2*OutWidth+1][0],inMask[2*OutWidth+2][0]);
    //printMatrixAsTextFile(inMask,OutWidth,OutHeight,"maska.txt");
}

void MainWindow::saveResult()
{
    MaskedImage.save("result.png",0,100);
}

void MainWindow::multiplicateArrays(fftw_complex *A, fftw_complex *B,int width,int height)
{
//    for(int i = 0; i< width; ++i)
//    {
//        for(int k = 0; k< height; ++k)
//        {
//            //re
//            double re = A[k*width +i][0]* B[k*width+i][0]
//                                         -A[k*width+i][1]* B[k*width+i][1];
//            //im
//            double im = A[k*width +i][1]* B[k*width+i][0]
//                                        +A[k*width +i][0]* B[k*width+i][1];
//            A[k*width +i][0] = re;
//            A[k*width +i][1] = im;
//        }
//   }

    int length= width*height;
    for(int i = 0; i<length; ++i)
    {
        // (a+bi)(c+di) = (ac-bd)+(ad+bd)i
        double re = A[i][0]*B[i][0] - A[i][1]*B[i][1];
        double im = A[i][0]*B[i][1] + A[i][1]*B[i][0];
        A[i][0] = re;
        A[i][1] = im;
    }
}


void MainWindow::on_pushButton_swap_clicked()
{
    if(MaskedImage.isNull()) return;
    fftw_free(in);
    fftw_free(out);
    fftw_free(inMask);
    fftw_free(outMask);
    fftw_free(final);

    SourceImage = MaskedImage;
    ui->label_Picture->setPixmap(QPixmap::fromImage(SourceImage));
    ui->label_Picture->adjustSize();

    OutWidth = SourceImage.width()+2;
    OutHeight = SourceImage.height()+2;

    fftw_destroy_plan(PlanForMainPicture);
    fftw_destroy_plan(PlanForIFFT);
    fftw_destroy_plan(PlanForMask);

    initializeMatrices();
    makePlans();
}

void MainWindow::printMatrixAsTextFile(fftw_complex *tab,int width, int height, QString filename,MatrixPrintingMode mode, QString valueSeparator, QString lineSeparator)
{
    QFile file(filename);
    if(!file.open(QFile::ReadWrite |QFile::Text)) return;
    QTextStream out(&file);
    for(int y = 0; y < height ; ++y)
    {
        for(int x = 0; x < width; ++x)
        {
            if(mode & REAL) out << tab[y * width + x][0];
            if(mode & IMAGINARY)
            {
                if(tab[y * width + x][1] >= 0) out << "+";
                out << tab[y * width + x][1] << "i";
            }
            out << valueSeparator;
        }
        out << lineSeparator;
    }
    file.flush();
    file.close();
}

double MainWindow::findMax(fftw_complex *tab, int size)
{
    double max = 0;
    for(int i = 0; i<size; ++i)
    {
       if(max < tab[i][0]) max = tab[i][0];
    }
    return max;
}

void MainWindow::makePredefinedMasks()
{
    QMap<QString,QString> edgeDetection;
    edgeDetection.insert("a1","0");
    edgeDetection.insert("a2","-1");
    edgeDetection.insert("a3","0");
    edgeDetection.insert("b1","-1");
    edgeDetection.insert("b2","4");
    edgeDetection.insert("b3","-1");
    edgeDetection.insert("c1","0");
    edgeDetection.insert("c2","-1");
    edgeDetection.insert("c3","0");
    PredefinedMasks.append(edgeDetection);
    ui->comboBox_predefinieMasks->addItem(QStringLiteral("Wykrywanie krawędzi"));
    ui->comboBox_predefinieMasks->addItem(QStringLiteral("Własne"));
}


void MainWindow::loadMaskFromCombobox(int index)
{
    if(index >= PredefinedMasks.size()) return;
    ui->lineEdit_a1->setText(PredefinedMasks[index].find("a1").value());
    ui->lineEdit_a2->setText(PredefinedMasks[index].find("a2").value());
    ui->lineEdit_a3->setText(PredefinedMasks[index].find("a3").value());
    ui->lineEdit_b1->setText(PredefinedMasks[index].find("b1").value());
    ui->lineEdit_b2->setText(PredefinedMasks[index].find("b2").value());
    ui->lineEdit_b3->setText(PredefinedMasks[index].find("b3").value());
    ui->lineEdit_c1->setText(PredefinedMasks[index].find("c1").value());
    ui->lineEdit_c2->setText(PredefinedMasks[index].find("c2").value());
    ui->lineEdit_c3->setText(PredefinedMasks[index].find("c3").value());
}

void MainWindow::maskEditsChanged()
{
    ui->comboBox_predefinieMasks->setCurrentText(QStringLiteral("Własne"));
}
