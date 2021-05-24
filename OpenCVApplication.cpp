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

std::string sides[] = {"left", "top", "right", "bottom"};

bool rectContains(Rect vector[], Rect element, int size) {
	for (int i = 0; i < size; i++) {
		if (vector[i] == element) return true;
	}
	return false;
}
void resolveNxN(int N)
{
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat_<uchar> src;
		src = imread(fname, IMREAD_GRAYSCALE);
		
		double width = src.cols;
		double height = src.rows;
		double GRID_WIDTH = width / N;
		double GRID_HEIGHT = height / N;

		Rect cellsVector[9];
		int k = 0;
		Mat_<uchar> solution[7][7];
		solution[0][0] = src(cellsVector[0]);
		for (int y = 0; y <= height - GRID_HEIGHT ; y += GRID_HEIGHT) {
			for (int x = 0; x <= width - GRID_WIDTH; x += GRID_WIDTH) {
				Rect grid_rect(x, y, GRID_WIDTH, GRID_HEIGHT);
				cellsVector[k] = grid_rect;
				k++;
				rectangle(src, grid_rect, Scalar(0, 255, 0), 1);
				imshow("src", src);
				//imshow(format("grid x:%d y:%d", x, y), src(grid_rect));
				//Uncomment to see how the grid looks over the source image
			}
		}

		double minRMSE;
		Mat_<uchar> bestPiece;
		int bestPieceSide;
		bool rowChange = false;
		Rect usedPieces[9];

		int solutionPositions[9];
		solutionPositions[0] = 0;

		int currentPiece = 0;
		for (int i = 0; i < N * N; i++)
		{
			std::vector<Mat_<uchar>> p1 = getPatches(src(cellsVector[currentPiece]), 5);

			minRMSE = FLT_MAX;
			bestPiece = Mat_<uchar>();
			bestPieceSide = 0;
			int bestPiecePosition = 0;


			if (currentPiece == 0) {
				for (int j = 1; j < N * N; j++)
				{
					std::vector<Mat_<uchar>> p2 = getPatches(src(cellsVector[j]), 5);
					for (int k = 0; k < 4; k++) {
						float rmse = rmseMatrix(p1[2], p2[k]);
						std::cout << "Piece " << currentPiece << " right with piece " << j << " " << sides[k] << " : " << rmse << ".\n";
						if (minRMSE > rmse) {
							minRMSE = rmse;
							bestPiece = Mat(src, cellsVector[j]);
							bestPieceSide = k;
							bestPiecePosition = j;
						}
					}
				}
				currentPiece = bestPiecePosition;
				usedPieces[bestPiecePosition] = cellsVector[bestPiecePosition];
				solutionPositions[i+1] = bestPiecePosition;
			}
			else {
				for (int j = i + 1; j < N * N; j++)
				{
					if (!rectContains(usedPieces, cellsVector[j], N * N) && cellsVector[j] != cellsVector[currentPiece]) {
						std::vector<Mat_<uchar>> p2 = getPatches(src(cellsVector[j]), 5);
						if (i % N != N - 1 && !rowChange) {
							for (int k = 0; k < 4; k++) {
								float rmse = rmseMatrix(p1[2], p2[k]);
								std::cout << "Piece " << currentPiece << " right with piece " << j << " " << sides[k] << " : " << rmse << ".\n";
								if (minRMSE > rmse) {
									minRMSE = rmse;
									bestPiece = Mat(src, cellsVector[j]);
									bestPieceSide = k;
									bestPiecePosition = j;
								}
							}
						}
						else if (i % N != N - 1 && rowChange) {
							for (int k = 0; k < 4; k++) {
								float rmse = rmseMatrix(p1[0], p2[k]);
								std::cout << "Piece " << currentPiece << " left with piece " << j << " " << sides[k] << " : " << rmse << ".\n";
								if (minRMSE > rmse) {
									minRMSE = rmse;
									bestPiece = Mat(src, cellsVector[j]);
									bestPieceSide = k;
									bestPiecePosition = j;
								}
							}
						}
						else {
							for (int k = 0; k < 4; k++) {
								float rmse = rmseMatrix(p1[3], p2[k]);
								std::cout << "Piece " << currentPiece << " bottom with piece " << j << " " << sides[k] << " : " << rmse << ".\n";
								if (minRMSE > rmse) {
									minRMSE = rmse;
									bestPiece = Mat(src, cellsVector[j]);
									bestPieceSide = k;
									bestPiecePosition = j;
								}
							}
							rowChange = !rowChange;
						}
						//switch (bestPieceSide) {
						//	case 0:

						//	case 1:

						//	case 2:

						//	case 3:
						//}
						solution[j / N][j % N] = bestPiece;
						solutionPositions[i + 1] = bestPiecePosition;
						currentPiece = bestPiecePosition;
					}
				}
			}
		}

		for (int i = 0; i < N * N; i++) {
			printf("%d", solutionPositions[i]);
			if (i % N == N - 1) printf("\n");
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
			resolveNxN(3);
			break;
		}

	} while (op != 0);
	return 0;
}
