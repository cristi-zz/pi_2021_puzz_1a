// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"


void testOpenImage()
{
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat src;
		src = imread(fname);
		imshow("opened image", src);
		waitKey();
	}
}

void testOpenImagesFld()
{
	char folderName[MAX_PATH];
	if (openFolderDlg(folderName) == 0)
		return;
	char fname[MAX_PATH];
	FileGetter fg(folderName, "bmp");
	while (fg.getNextAbsFile(fname))
	{
		Mat src;
		src = imread(fname);
		imshow(fg.getFoundFileName(), src);
		if (waitKey() == 27) //ESC pressed
			break;
	}
}

void testColor2Gray()
{
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat_<Vec3b> src = imread(fname, IMREAD_COLOR);

		int height = src.rows;
		int width = src.cols;

		Mat_<uchar> dst(height, width);

		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				Vec3b v3 = src(i, j);
				uchar b = v3[0];
				uchar g = v3[1];
				uchar r = v3[2];
				dst(i, j) = (r + g + b) / 3;
			}
		}

		imshow("original image", src);
		imshow("gray image", dst);
		waitKey();
	}
}

double rmseVector(std::vector<uchar>& m0, std::vector<uchar>& m1) {
	double result = 0;
	int size = m0.size();

	for (int i = 0; i < size; i++) {
		uchar p0 = m0[i];
		uchar p1 = m1[i];
		int c = abs(p0 - p1);
		result += sqrt(c * c);
	}

	result /= size;
	return result;
}

double rmseMatrix(Mat_<uchar>& m0, Mat_<uchar>& m1) {
    double result = 0;
    int height = m0.rows;
    int width = m0.cols;

	std::vector<uchar> p0;
	std::vector<uchar> p1;

	if (width < height)
	{
		for (int i = 0; i < width; i++) {
			p0.clear();
			p1.clear();
			for (int j = 0; j < height; j++) {
				p0.push_back(m0.at<uchar>(j,width-i));
				p1.push_back(m1.at<uchar>(j,i));
			}
			result += rmseVector(p0, p1);
		}
	}
	else {
		for (int i = 0; i < height; i++) {
			p0.clear();
			p1.clear();
			for (int j = 0; j < width; j++) {
				p0.push_back(m0.at<uchar>(height - i, j));
				p1.push_back(m1.at<uchar>(i, j));
			}
			result += rmseVector(p0, p1);
		}
	}


    result /= width;
    return result;
}

void divideImage()
{
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat_<Vec3b> src;
		src = imread(fname);
		printf("%d %d", src.rows, src.cols);
		int rowNumber = src.rows / 2;
		int colNumber = src.cols / 2;
		Mat_<Vec3b> imgLeft(rowNumber, colNumber), imgRight(rowNumber, colNumber);
		std::vector<Mat_<Vec3b>> imagePieces;
		for (int i = 0; i < 4; i++)
			imagePieces.push_back(Mat_<Vec3b>(rowNumber, colNumber));
		for(int i = 0; i< src.rows; i++)
			for (int j = 0; j < src.cols; j++)
			{
				if (i < rowNumber && j < colNumber)
					imagePieces.at(0)(i,j) = src(i, j);

				if (i < rowNumber && j > colNumber)
					imagePieces.at(1)(i, j-colNumber) = src(i, j);

				if (i > rowNumber && j < colNumber)
					imagePieces.at(2)(i-rowNumber, j) = src(i, j);
				if (i > rowNumber && j > colNumber)
					imagePieces.at(3)(i-rowNumber,j - colNumber) = src(i, j);
			}
		for (int i = 0; i < imagePieces.size(); i++)
			imshow(std::to_string(i), imagePieces.at(i));
		waitKey();
	}
}
int main()
{
	int op;
	do
	{
		system("cls");
		destroyAllWindows();
		printf("Menu:\n");
		printf(" 1 - Basic image opening...\n");
		printf(" 2 - Open BMP images from folder\n");
		printf(" 3 - Color to Gray\n");
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d", &op);
		switch (op)
		{
		case 1:
			testOpenImage();
			break;
		case 2:
			testOpenImagesFld();
			break;
		case 3:
			testColor2Gray();
			break;
		case 4:
			divideImage();
			break;
		}
		
	} 	while (op != 0);
	return 0;
}
