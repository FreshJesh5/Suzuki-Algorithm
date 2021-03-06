#include "stdafx.h"
#include <string.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>

#define HOLE_BORDER 1
#define OUTER_BORDER 2

using namespace std;

struct Border {
	int seq_num;
	int border_type;
};

struct Point {
	int row;
	int col;

	Point() {
	}

	Point(int r, int c) {
		row = r;
		col = c;
	}

	void setPoint(int r, int c) {
		row = r;
		col = c;
	}

	bool samePoint(Point p) {
		return row == p.row && col == p.col;
	}
};

struct Pixel {
	unsigned char red;
	unsigned char blue;
	unsigned char green;

	Pixel(unsigned char r, unsigned char g, unsigned char b) {
		red = r;
		green = g;
		blue = b;
	}
	void setPixel(unsigned char r, unsigned char g, unsigned char b) {
		red = r;
		green = g;
		blue = b;
	}
};

/*
struct TreeNode {
TreeNode *parent;
Border border;
TreeNode *first_child;
TreeNode *next_sibling;

TreeNode(Border b) {
border = b;
}
};
*/

//struct for storing information on the current border, the first child, next sibling, and the parent.
struct Node {
	int parent;
	int first_child;
	int next_sibling;
	Border border;
	Node(int p, int fc, int ns) {
		parent = p;
		first_child = fc;
		next_sibling = ns;
	}
	void reset() {
		parent = -1;
		first_child = -1;
		next_sibling = -1;
	}
};

//reads a bmp file, stores it in a 2D vector
vector<vector<int>> readFile(string file_name, int &numrows, int &numcols) {
	int row = 0, col = 0;

	ifstream infile(file_name);

	stringstream ss;
	string inputLine = "";

	// First line : version
	//getline(infile, inputLine);
	getline(infile, inputLine);
	if (inputLine.compare("P2") != 0)
		cerr << "Version error" << endl;
	else
	//	cout << "Version : " << inputLine << endl;

	// Secondline : size
	ss << infile.rdbuf();
	ss >> numcols >> numrows;
	//cout << numcols << " columns and " << numrows << " rows" << endl;


	// Third line : comment
	ss >> inputLine;
	//cout << "Comment : " << inputLine << endl;

	vector<vector<int>> image(numrows);

	//int array[numrows][numcols]; not using array

	int input;

	// Following lines : data
	for (row = 0; row < numrows; ++row) {
		for (col = 0; col < numcols; ++col) {
			ss >> input;
			if (input != 0)
				input = 1;
			image[row].push_back(input);
		}
	}

	infile.close();
	return image;
}

//Saves the altered image 2D vector to a txt file. Used for debugging
void saveTextFile(string file_name, vector<vector<int>> image) {
	ofstream myfile;
	myfile.open(file_name);
	for (unsigned int row = 0; row < image.size(); ++row) {
		for (unsigned int col = 0; col < image[0].size(); ++col) {
			myfile << setw(4) << image[row][col];
		}
		myfile << endl;
	}
	myfile.close();
	return;
}

//Given a vector of vectors of contours, draw out the contour specified by seq_num
//contours[n-2] contains the nth contour since the first contour starts at 2
void drawContour(vector<vector<Point>> contours, vector<vector<Pixel>> &color, int seq_num, Pixel pix) {
	int index = seq_num - 2;
	int r, c;
	for (unsigned int i = 0; i < contours[index].size(); i++) {
		r = contours[index][i].row;
		c = contours[index][i].col;
		color[r][c] = pix;
	}
}

//chooses color for a contour based on its seq_num
Pixel chooseColor(int n) {
	switch (n%6) {
		case 0:
			return Pixel(255,0,0);
		case 1:
			return Pixel(255, 127, 0);
		case 2:
			return Pixel(255, 255, 0);
		case 3:
			return Pixel(0, 255, 0);
		case 4:
			return Pixel(0, 0, 255);
		case 5:
			return Pixel(139, 0, 255);
	}
}

//creates a 2D array of struct Pixel, which is the 3 channel image needed to convert the 2D vector contours to a drawn bmp file
//uses DFS to step through the hierarchy tree, can be set to draw only the top 2 levels of contours, for example.
vector<vector<Pixel>> createChannels(int h, int w, vector<Node> hierarchy, vector<vector<Point>> contours) {
	queue<int> myQueue;
	vector<vector<Pixel>> color(h,vector<Pixel> (w, Pixel((unsigned char) 0, (unsigned char)0, (unsigned char)0)));
	int seq_num;
	for (int n = hierarchy[0].first_child; n != -1; n = hierarchy[n - 1].next_sibling) {
		myQueue.push(n);
	}

	while (!myQueue.empty()) {
		seq_num = myQueue.front();
		drawContour(contours, color, seq_num, chooseColor(seq_num));
		myQueue.pop();
		for (int n = hierarchy[seq_num - 1].first_child; n != -1; n = hierarchy[n - 1].next_sibling) {
			myQueue.push(n);
		}
	}
	return color;
}

//save image to bmp
void saveImageFile(const char * file_name, int h, int w, vector<Node> hierarchy, vector<vector<Point>> contours) {
	FILE *f;

	vector<vector<Pixel>> color = createChannels(h,w,hierarchy,contours);
	unsigned char *img = NULL;
	int filesize = 54 + 3 * w*h;  //w is your image width, h is image height, both int

	img = (unsigned char *)malloc(3 * w*h);
	memset(img, 0, 3 * w*h);
	int x, y;
	unsigned char r, g, b;
	for (int i = 0; i<h; i++){
		for (int j = 0; j<w; j++){
			y = (h - 1) - i; x = j;
			r = color[i][j].red;
			g = color[i][j].green;
			b = color[i][j].blue;
			/*	        if (r > 255) r=255;
			if (g > 255) g=255;
			if (b > 255) b=255;*/
			img[(x + y * w) * 3 + 2] = (unsigned char)(r);
			img[(x + y * w) * 3 + 1] = (unsigned char)(g);
			img[(x + y * w) * 3 + 0] = (unsigned char)(b);
		}
	}

	unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
	unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
	unsigned char bmppad[3] = { 0,0,0 };

	bmpfileheader[2] = (unsigned char)(filesize);
	bmpfileheader[3] = (unsigned char)(filesize >> 8);
	bmpfileheader[4] = (unsigned char)(filesize >> 16);
	bmpfileheader[5] = (unsigned char)(filesize >> 24);

	bmpinfoheader[4] = (unsigned char)(w);
	bmpinfoheader[5] = (unsigned char)(w >> 8);
	bmpinfoheader[6] = (unsigned char)(w >> 16);
	bmpinfoheader[7] = (unsigned char)(w >> 24);
	bmpinfoheader[8] = (unsigned char)(h);
	bmpinfoheader[9] = (unsigned char)(h >> 8);
	bmpinfoheader[10] = (unsigned char)(h >> 16);
	bmpinfoheader[11] = (unsigned char)(h >> 24);

	fopen_s(&f, file_name, "wb");
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);
	for (int i = 0; i<h; i++){
		fwrite(img + (w*(i) * 3), 3, w, f);
		fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
	}

	free(img);
	fclose(f);
}

//step around a pixel CCW
void stepCCW(Point &current, Point pivot) {
	if (current.col > pivot.col)
		current.setPoint(pivot.row - 1, pivot.col);
	else if (current.col < pivot.col)
		current.setPoint(pivot.row + 1, pivot.col);
	else if (current.row > pivot.row)
		current.setPoint(pivot.row, pivot.col + 1);
	else if (current.row < pivot.row)
		current.setPoint(pivot.row, pivot.col - 1);
}

//step around a pixel CW
void stepCW(Point &current, Point pivot) {
	if (current.col > pivot.col)
		current.setPoint(pivot.row + 1, pivot.col);
	else if (current.col < pivot.col)
		current.setPoint(pivot.row - 1, pivot.col);
	else if (current.row > pivot.row)
		current.setPoint(pivot.row, pivot.col - 1);
	else if (current.row < pivot.row)
		current.setPoint(pivot.row, pivot.col + 1);
}

//step around a pixel CCW in the 8-connect neighborhood.
void stepCCW8(Point &current, Point pivot) {
	if (current.row == pivot.row && current.col > pivot.col)
		current.setPoint(pivot.row - 1, pivot.col + 1);
	else if (current.col > pivot.col && current.row < pivot.row)
		current.setPoint(pivot.row - 1, pivot.col);
	else if (current.row < pivot.row && current.col == pivot.col)
		current.setPoint(pivot.row - 1, pivot.col - 1);
	else if (current.row < pivot.row && current.col < pivot.col)
		current.setPoint(pivot.row, pivot.col - 1);
	else if (current.row == pivot.row && current.col < pivot.col)
		current.setPoint(pivot.row + 1, pivot.col - 1);
	else if (current.row > pivot.row && current.col < pivot.col)
		current.setPoint(pivot.row + 1, pivot.col);
	else if (current.row > pivot.row && current.col == pivot.col)
		current.setPoint(pivot.row + 1, pivot.col + 1);
	else if (current.row > pivot.row && current.col > pivot.col)
		current.setPoint(pivot.row, pivot.col + 1);
}

//step around a pixel CW in the 8-connect neighborhood.
void stepCW8(Point &current, Point pivot) {
	if (current.row == pivot.row && current.col > pivot.col)
		current.setPoint(pivot.row + 1, pivot.col + 1);
	else if (current.col > pivot.col && current.row < pivot.row)
		current.setPoint(pivot.row, pivot.col + 1);
	else if (current.row < pivot.row && current.col == pivot.col)
		current.setPoint(pivot.row - 1, pivot.col + 1);
	else if (current.row < pivot.row && current.col < pivot.col)
		current.setPoint(pivot.row - 1, pivot.col);
	else if (current.row == pivot.row && current.col < pivot.col)
		current.setPoint(pivot.row - 1, pivot.col - 1);
	else if (current.row > pivot.row && current.col < pivot.col)
		current.setPoint(pivot.row, pivot.col - 1);
	else if (current.row > pivot.row && current.col == pivot.col)
		current.setPoint(pivot.row + 1, pivot.col - 1);
	else if (current.row > pivot.row && current.col > pivot.col)
		current.setPoint(pivot.row + 1, pivot.col);
}

//checks if a given pixel is out of bounds of the image
bool pixelOutOfBounds(Point p, int numrows, int numcols) {
	return (p.col >= numcols || p.row >= numrows || p.col < 0 || p.row < 0);
}

//marks a pixel as examined after passing through
void markExamined(Point mark, Point center, bool checked[4]) {
	//p3.row, p3.col + 1
	int loc = -1;
	//    3
	//  2 x 0
	//    1
	if (mark.col > center.col)
		loc = 0;
	else if (mark.col < center.col)
		loc = 2;
	else if (mark.row > center.row)
		loc = 1;
	else if (mark.row < center.row)
		loc = 3;

	if (loc == -1)
		throw exception("Error: markExamined Failed");

	checked[loc] = true;
	return;
}

//marks a pixel as examined after passing through in the 8-connected case
void markExamined8(Point mark, Point center, bool checked[8]) {
	//p3.row, p3.col + 1
	int loc = -1;
	//  5 6 7
	//  4 x 0
	//  3 2 1
	if (mark.row == center.row && mark.col > center.col)
		loc = 0;
	else if (mark.col > center.col && mark.row < center.row)
		loc = 7;
	else if (mark.row < center.row && mark.col == center.col)
		loc = 6;
	else if (mark.row < center.row && mark.col < center.col)
		loc = 5;
	else if (mark.row == center.row && mark.col < center.col)
		loc = 4;
	else if (mark.row > center.row && mark.col < center.col)
		loc = 3;
	else if (mark.row > center.row && mark.col == center.col)
		loc = 2;
	else if (mark.row > center.row && mark.col > center.col)
		loc = 1;

	if (loc == -1)
		throw exception("Error: markExamined Failed");

	checked[loc] = true;
	return;
}

//checks if given pixel has already been examined
bool isExamined(bool checked[4]) {
	//p3.row, p3.col + 1
	return checked[0];
}

bool isExamined8(bool checked[8]) {
	//p3.row, p3.col + 1
	return checked[0];
}

//prints image in console
void printImage(vector<vector<int>> image, int numrows, int numcols) {
	// Now print the array to see the result
	for (int row = 0; row < numrows; ++row) {
		for (int col = 0; col < numcols; ++col) {
			cout << setw(3) << image[row][col];
		}
		cout << endl;
	}
	cout << endl;
}

//follows a border from start to finish given a starting point
void followBorder(vector<vector<int>> &image, int row, int col, Point p2, Border NBD, vector<vector<Point>> &contours) {
	int numrows = image.size();
	int numcols = image[0].size();
	Point current(p2.row, p2.col);
	Point start(row, col);
	vector<Point> point_storage;

	//(3.1)
	//Starting from (i2, j2), look around clockwise the pixels in the neighborhood of (i, j) and find a nonzero pixel.
	//Let (i1, j1) be the first found nonzero pixel. If no nonzero pixel is found, assign -NBD to fij and go to (4).
	do {
		stepCW(current, start);
		if (current.samePoint(p2)) {
			image[start.row][start.col] = -NBD.seq_num;
			point_storage.push_back(start);
			contours.push_back(point_storage);
			return;
		}
	} while (pixelOutOfBounds(current, numrows, numcols) || image[current.row][current.col] == 0);
	Point p1 = current;
	
	//(3.2)
	//(i2, j2) <- (i1, j1) and (i3, j3) <- (i, j).
	
	Point p3 = start;
	Point p4;
	p2 = p1;
	bool checked[4];
//	bool checked[8];
	while (true) {
		//(3.3)
		//Starting from the next element of the pixel(i2, j2) in the counterclockwise order, examine counterclockwise the pixels in the
		//neighborhood of the current pixel(i3, j3) to find a nonzero pixel and let the first one be(i4, j4).
		current = p2;

		for (int i = 0; i < 4; i++)
			checked[i] = false;

		do {
			markExamined(current, p3, checked);
			stepCCW(current, p3);
		} while (pixelOutOfBounds(current, numrows, numcols) || image[current.row][current.col] == 0);
		p4 = current;

		//Change the value fi3, j3 of the pixel(i3, j3) as follows :
		//	If the pixel(i3, j3 + 1) is a 0 - pixel examined in the substep(3.3) then fi3, j3 <- - NBD.
		//	If the pixel(i3, j3 + 1) is not a 0 - pixel examined in the substep(3.3) and fi3, j3 = 1, then fi3, j3 ←NBD.
		//	Otherwise, do not change fi3, j3.

		if ( (p3.col+1 >= numcols || image[p3.row][p3.col + 1] == 0) && isExamined(checked)) {
			image[p3.row][p3.col] = -NBD.seq_num;
		}
		else if (p3.col + 1 < numcols && image[p3.row][p3.col] == 1) {
			image[p3.row][p3.col] = NBD.seq_num;
		}

		point_storage.push_back(p3);

		//(3.5)
		//If(i4, j4) = (i, j) and (i3, j3) = (i1, j1) (coming back to the starting point), then go to(4);
		//otherwise, (i2, j2) <- (i3, j3), (i3, j3) <- (i4, j4), and go back to(3.3).
		if (p4.samePoint(start) && p3.samePoint(p1)) {
			contours.push_back(point_storage);
			return;
		}

		p2 = p3;
		p3 = p4;
	}
}

//prints the hierarchy list
void printHierarchy(vector<Node> hierarchy) {
	for (unsigned int i = 0; i < hierarchy.size(); i++) {
		cout << setw(2) << i + 1 << ":: parent: " <<setw(3)<< hierarchy[i].parent << " first child: " << setw(3) << hierarchy[i].first_child << " next sibling: " << setw(3) << hierarchy[i].next_sibling << endl;
	}
}

/*
void printTree(TreeNode *root) {
	//DFS traversal of Tree
	if (root != NULL) {
		cout << root->border.seq_num << " ";
		for (TreeNode * n = root->first_child; n != NULL; n = n->next_sibling) {
			printTree(n);
		}
	}
}
*/

int main() {
	int numrows = 0;
	int numcols = 0;
	Border NBD, LNBD;
	//a vector of vectors to store each contour.
	//contour n will be stored in contours[n-2]
	//contour 2 will be stored in contours[0], contour 3 will be stored in contours[1], ad infinitum
	vector<vector<Point>> contours; 

	vector<vector<int>> image = readFile("data1_filter2.pgm", numrows, numcols);


	if (image.empty()) {
		throw exception("Image Error");
	}

	LNBD.border_type = HOLE_BORDER;
	NBD.border_type = HOLE_BORDER;
	NBD.seq_num = 1;

	//hierarchy tree will be stored as an vector of nodes instead of using an actual tree since we need to access a node based on its index
	//see definition for Node
	//-1 denotes NULL
	vector<Node> hierarchy;
	Node temp_node(-1, -1, -1);
	temp_node.border = NBD;
	hierarchy.push_back(temp_node);

	Point p2;
	bool border_start_found;
	for (int r = 0; r < numrows; r++) {
		LNBD.seq_num = 1;
		LNBD.border_type = HOLE_BORDER;
		for (int c = 0; c < numcols; c++) {
			border_start_found = false;
			//Phase 1: Find border
			//If fij = 1 and fi, j-1 = 0, then decide that the pixel (i, j) is the border following starting point
			//of an outer border, increment NBD, and (i2, j2) <- (i, j - 1).
			if ((image[r][c] == 1 && c - 1 < 0) || (image[r][c] == 1 && image[r][c - 1] == 0)) {
				NBD.border_type = OUTER_BORDER;
				NBD.seq_num += 1;
				p2.setPoint(r,c-1);
				border_start_found = true;
			}

			//Else if fij >= 1 and fi,j+1 = 0, then decide that the pixel (i, j) is the border following
			//starting point of a hole border, increment NBD, (i2, j2) ←(i, j + 1), and LNBD ← fij in case fij > 1.
			else if ( c+1 < numcols && (image[r][c] >= 1 && image[r][c + 1] == 0)) {
				NBD.border_type = HOLE_BORDER;
				NBD.seq_num += 1;
				if (image[r][c] > 1) {
					LNBD.seq_num = image[r][c];
					LNBD.border_type = hierarchy[LNBD.seq_num-1].border.border_type;
				}
				p2.setPoint(r, c + 1);
				border_start_found = true;
			}

			if (border_start_found) {
				//Phase 2: Store Parent

//				current = new TreeNode(NBD);
				temp_node.reset();
				if (NBD.border_type == LNBD.border_type) {
					temp_node.parent = hierarchy[LNBD.seq_num - 1].parent;
					temp_node.next_sibling = hierarchy[temp_node.parent - 1].first_child;
					hierarchy[temp_node.parent - 1].first_child = NBD.seq_num;
					temp_node.border = NBD;
					hierarchy.push_back(temp_node);

//					cout << "indirect: " << NBD.seq_num << "  parent: " << LNBD.seq_num <<endl;
				}
				else {
					if (hierarchy[LNBD.seq_num-1].first_child != -1) {
						temp_node.next_sibling = hierarchy[LNBD.seq_num-1].first_child;
					}

					temp_node.parent = LNBD.seq_num;
					hierarchy[LNBD.seq_num-1].first_child = NBD.seq_num;
					temp_node.border = NBD;
					hierarchy.push_back(temp_node);

//					cout << "direct: " << NBD.seq_num << "  parent: " << LNBD.seq_num << endl;
				}

				//Phase 3: Follow border
				followBorder(image, r, c, p2, NBD, contours);
			}

			//Phase 4: Continue to next border
			//If fij != 1, then LNBD <- abs( fij ) and resume the raster scan from the pixel(i, j + 1).
			//The algorithm terminates when the scan reaches the lower right corner of the picture.
			if (abs(image[r][c]) > 1) {
				LNBD.seq_num = abs(image[r][c]);
				LNBD.border_type = hierarchy[LNBD.seq_num - 1].border.border_type;
			}
		}

	}
	printHierarchy(hierarchy);
//	printImage(image, image.size(), image[0].size());
//	printImage(image, image.size(), image[0].size());
	saveTextFile("output.txt", image);
	saveImageFile("data1_filter2.bmp", image.size(), image[0].size(), hierarchy, contours);

}