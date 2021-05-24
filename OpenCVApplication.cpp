// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <float.h>


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


double rmseMatrix(Mat_<uchar> first, Mat_<uchar> second) {

	double rmse = 0.0;
	int nrOfElements = first.rows * first.cols;
	int ratio = 1;
	for (int i = 0; i < first.rows; i++)
	{
		for (int j = 0; j < first.cols; j++)
		{
			rmse += (first(i, j) - second(i, j)) * (first(i, j) - second(i, j)) * ratio;
		}
		ratio++;
	}

	rmse /= nrOfElements;
	return sqrt(rmse);
}

std::vector<Mat_<uchar>> getPatches(Mat_<uchar> image, int patchSize) {
	std::vector<Mat_<uchar>> patches;
	Mat_<uchar> emptyLeft(image.rows, patchSize);
	Mat_<uchar> emptyTop(patchSize, image.cols);
	Mat_<uchar> emptyRight(image.rows, patchSize);
	Mat_<uchar> emptyBottom(patchSize, image.cols);
	patches.push_back(emptyLeft);
	patches.push_back(emptyTop);
	patches.push_back(emptyRight);
	patches.push_back(emptyBottom);

	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			if (j < patchSize) {
				patches[0].at<uchar>(i, j) = image.at<uchar>(i, j);
			}

			if (i < patchSize) {
				patches[1].at<uchar>(i, j) = image.at<uchar>(i, j);
			}

			if (j > image.cols - patchSize) {
				patches[2].at<uchar>(i, abs(image.cols - j - patchSize + 1)) = image.at<uchar>(i, j);
			}

			if (i > image.rows - patchSize) {
				patches[3].at<uchar>(abs(image.rows - i - patchSize + 1), j) = image.at<uchar>(i, j);
			}
		}
	}
	cv::rotate(patches[0], patches[0], ROTATE_90_CLOCKWISE);
	cv::rotate(patches[2], patches[2], ROTATE_90_COUNTERCLOCKWISE);
	cv::flip(patches[2], patches[2], 1);
	cv::rotate(patches[3], patches[3], ROTATE_180);
	cv::flip(patches[3], patches[3], 1);
	/*imshow("left", patches[0]);
	imshow("top", patches[1]);
	imshow("right", patches[2]);
	imshow("bot", patches[3]);
	
	waitKey();*/
	return patches;
}

void resolve2x2()
{
	// Create struct with partCols,partRows,leftBest,topBest,rightBest,bottomBest
	// Based on the rows and cols of the part choose where it goes (default 0,0 to top left)
	// Add the rest of the pieces based on the bests starting from 0,0
	// bests type is _Mat<uchar> so the image will use graph logic with 0,0 being the root
	// make imagePieces a matrix so we can use rows and cols to get a specific piece

	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat_<uchar> src;
		src = imread(fname, IMREAD_GRAYSCALE);

		int rowNumber = src.rows / 2;
		int colNumber = src.cols / 2;
		Mat_<uchar> imgLeft(rowNumber, colNumber), imgRight(rowNumber, colNumber);
		std::vector<Mat_<uchar>> imagePieces;
		for (int i = 0; i < 4; i++)
			imagePieces.push_back(Mat_<uchar>(rowNumber, colNumber));
		for (int i = 0; i < src.rows; i++) // Make it possible to split the image in NxN pieces
			for (int j = 0; j < src.cols; j++)
			{
				if (i < rowNumber && j < colNumber)
					imagePieces.at(0)(i, j) = src(i, j);

				if (i < rowNumber && j > colNumber)
					imagePieces.at(1)(i, j - colNumber) = src(i, j);

				if (i > rowNumber && j < colNumber)
					imagePieces.at(2)(i - rowNumber, j) = src(i, j);

				if (i > rowNumber && j > colNumber)
					imagePieces.at(3)(i - rowNumber, j - colNumber) = src(i, j);
			}
		//for (int i = 0; i < imagePieces.size(); i++)
			//imshow(std::to_string(i), imagePieces.at(i));

		/*std::vector<Mat_<uchar>> p0 = getPatches(imagePieces.at(0), 50);
		std::vector<Mat_<uchar>> p1 = getPatches(imagePieces.at(1), 50);

		for (int i = 0; i < 4; i++)
			imshow(std::to_string(i), p0.at(i));


		for (int i = 0; i < 4; i++)
			imshow(std::to_string(i + 5), p1.at(i));

		waitKey();
		exit(0);*/
		for (int i = 0; i < 4; i++)
		{
			if (i == 0)
			{
				printf("TOP LEFT\n");
				std::vector<Mat_<uchar>> p0 = getPatches(imagePieces.at(0), 2);
				double min = FLT_MAX;
				int auxK = 0;
				int auxZ = 0;
				int auxJ = 0;

				for (int j = 1; j < 4; j++)
				{
					std::vector<Mat_<uchar>> p1 = getPatches(imagePieces.at(j), 2);

					for (int k = 0; k < p1.size(); k++)
					{
						double result = rmseMatrix(p0.at(3), p1.at(k));
						if (result < min)
						{
							min = result;
							auxK = k; // numar patch
							auxJ = j; // numar imagine
						}
					}
				}

				double min2 = FLT_MAX;
				int auxK2 = 0;
				int auxZ2 = 0;
				int auxJ2 = 0;
				for (int j = 1; j < 4; j++)
				{
					std::vector<Mat_<uchar>> p1 = getPatches(imagePieces.at(j), 2);

					for (int k = 0; k < p1.size(); k++)
					{
						double result = rmseMatrix(p0.at(2), p1.at(k));
						if (result < min2)
						{
							min2 = result;
							auxK2 = k; // numar patch
							auxJ2 = j; // numar imagine
						}
					}
				}

				printf("Best: Imagine = %d Patch = %d -> RMSE = %lf\n", auxJ, auxK, min); // imaginea din dreapta
				printf("Best: Imagine = %d Patch = %d -> RMSE = %lf\n", auxJ2, auxK2, min2); //imaginea de jos
				//cum e 2x2, mai ramane o singura imagine ramasa, cea din stanga jos.
			}

			if (i == 3)
			{
				printf("BOT RIGHT\n");
				std::vector<Mat_<uchar>> p0 = getPatches(imagePieces.at(3), 2);
				double min = FLT_MAX;
				int auxK = 0;
				int auxZ = 0;
				int auxJ = 0;

				for (int j = 0; j < 3; j++)
				{
					std::vector<Mat_<uchar>> p1 = getPatches(imagePieces.at(j), 2);

					for (int k = 0; k < p1.size(); k++)
					{
						double result = rmseMatrix(p0.at(0), p1.at(k));
						if (result < min)
						{
							min = result;
							auxK = k; // numar patch
							auxJ = j; // numar imagine
						}
					}
				}

				double min2 = FLT_MAX;
				int auxK2 = 0;
				int auxZ2 = 0;
				int auxJ2 = 0;
				for (int j = 0; j < 3; j++)
				{
					std::vector<Mat_<uchar>> p1 = getPatches(imagePieces.at(j), 2);

					for (int k = 0; k < p1.size(); k++)
					{
						double result = rmseMatrix(p0.at(1), p1.at(k));
						if (result < min2)
						{
							min2 = result;
							auxK2 = k; // numar patch
							auxJ2 = j; // numar imagine
						}
					}
				}

				printf("Best: Imagine = %d Patch = %d -> RMSE = %lf\n", auxJ, auxK, min); // imaginea din stanga
				printf("Best: Imagine = %d Patch = %d -> RMSE = %lf\n", auxJ2, auxK2, min2); //imaginea de sus
				//cum e 2x2, mai ramane o singura imagine ramasa, cea din stanga jos.
			}
		}
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
		printf(" 4 - Divide image\n");
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
			resolve2x2();
			break;

		}

	} while (op != 0);
	return 0;
}
