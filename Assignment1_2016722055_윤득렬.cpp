#include <math.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
using namespace std;
#define _USE_MATH_DEFINES
#pragma warning(disable : 4996)

HWND hwnd;
HDC hdc;

enum color {
	BLUE, RED, WHITE, GREEN
};

//Used to print out the correct answer
void Draw(float val, int x_origin, int y_origin, int curx, color c) {				
	if (c == BLUE) {
		SetPixel(hdc, x_origin + curx, y_origin - val, RGB(0, 0, 255));
	}
	else if (c == RED) {	//i used to print Laplace_pdf
		SetPixel(hdc, x_origin + curx, y_origin - val, RGB(255, 0, 0));
	}
	else if (c == GREEN) {	//i used to print Laplace_cdf
		SetPixel(hdc, x_origin + curx, y_origin - val, RGB(0, 255, 0));
	}
	else {
		SetPixel(hdc, x_origin + curx, y_origin - val, RGB(255, 255, 255));
	}
}

//A function that draws a graph using histogram
void DrawHistogram(int histogram[400], int x_origin, int y_origin, int cnt) {			
	MoveToEx(hdc, x_origin, y_origin, 0);
	LineTo(hdc, x_origin + cnt, y_origin);

	MoveToEx(hdc, x_origin, 100, 0);
	LineTo(hdc, x_origin, y_origin);

	for (int CurX = 0; CurX < cnt; CurX++) {
		for (int CurY = 0; CurY < histogram[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			LineTo(hdc, x_origin + CurX, y_origin - histogram[CurX] / 2);
		}
	}
}


//A function that draws a graph using histogram
void DrawHistogram(float histogram[400], int x_origin, int y_origin, int cnt) {
	MoveToEx(hdc, x_origin, y_origin, 0);
	LineTo(hdc, x_origin + cnt, y_origin);

	MoveToEx(hdc, x_origin, y_origin, 0);
	LineTo(hdc, x_origin, y_origin - 100);

	for (int CurX = 0; CurX < cnt; CurX++) {
		for (int CurY = 0; CurY < histogram[CurX]; CurY++) {
			MoveToEx(hdc, x_origin + CurX, y_origin, 0);
			LineTo(hdc, x_origin + CurX, y_origin - histogram[CurX]);
		}
	}
}

//to find the correct answer.(Laplace_pdf function)
float Laplace_pdf(int x, float rate, float  mu = 0, float sigma = 1) {
	float b = sqrt((double)sigma / 2);
	float _exp;
	float result;
	float p_x = x / rate;
	if (p_x < mu) {
		_exp = exp((p_x - mu) / b);
	}
	else if (p_x >= mu) {
		_exp = exp((mu - p_x) / b);	
	}
	result = ((double)1 / (2*b)) * _exp;

	return result;
}

//to find the correct answer.(Laplace_cdf function)
float Laplace_cdf(float x, float rate, float mu = 0, float sigma = 1) {	
	//mu = avg, sigma = 2*b^2
	float b = sqrt(sigma / 2);
	float _x = (double)x / rate;
	float tmp;

	float abs_v;
	if (_x - mu > 0) { tmp = 1; abs_v = mu - _x; }
	else if (_x - mu < 0) { tmp = -1; abs_v = _x - mu; }
	else { tmp = 0; abs_v = 0; }

	float _exp = exp(abs_v / b);
	
	float result = (double)1 / 2 * tmp * (1 - _exp);
	
	
	return (double) 0.5 + result;
}

//Used to convert uniform distribution values ​​to Laplace distribution.(transform function)
float inverse_Laplace_cdf(float p, float mu = 0, float sigma = 1)		
{
	float signum;
	if ((p - 0.5) > 0) { signum = 1; }
	else if ((p - 0.5) == 0) { signum = 0; }
	else if ((p - 0.5) < 0) { signum = -1; }
	else signum = -999;

	float result;
	float b = sqrt(sigma / 2);	// sigma = 2b*b
	if (abs(p - 0.5) == 0.5) { result = mu; }
	else if( abs(p-0.5) != 0.5) { result = mu - b * signum * log(1 - 2 * abs(p -0.5)); }
	return result;
}

//K-S 검증을 위한 table 및 결과
bool KS(double d, double a, int range) {
	double val;
	if (a == 0.001) val = 1.94947;
	else if (a == 0.01) val = 1.62762;
	else if (a == 0.02) val = 1.51743;
	else if (a == 0.05) val = 1.3581;
	else if (a == 0.1) val = 1.22385;
	else if (a == 0.15) val = 1.13795;
	else val = 1.07275;	
	double p = val / sqrt(range);
	if (d > 2 * p) {
		return false;
	}
	return true;
}


int main(void)
{
	system("color F0");
	hwnd = GetForegroundWindow();
	hdc = GetWindowDC(hwnd);

	srand(time(NULL));
	int y_axis = 300;
	int range = 400; 
	int rcnt = 10000; 
	float avg = 0;								//default average
	float sigma = 1;							//default sigma

	int random_variable[401] = { 0, };			//random variable count 

	int random_to_Laplace_pdf[401] = { 0, };	//Laplace Variable count
	float Laplace_pdf_fact[401] = { 0, };			//correct Laplace PDF 

	float random_to_Laplace_cdf[401] = { 0, };	//Laplace distribution cdf
	float Laplace_cdf_fact[401] = { 0, };		//correct Laplace CDF

	cout << "평균: ";						
	cin >> avg;									//user input average
	cout << "표준편차: ";
	cin >> sigma;								//user input sigma
	sigma = pow(sigma, 2);

	/* 1. make random uniform distribution */
	int tmp;
	for (int i = 0; i < rcnt; i++) {			//each number count. (random variable of uniform distribution)
		tmp = rand() % 401;						//range 0~400
		random_variable[tmp]++;
	}


	/* 2. use random uniform. make Laplace distribution */
	for (int i = 0; i < 401; i++) {
		float value = (float)i/400;				//choice number
		int x = inverse_Laplace_cdf(value, avg, sigma) * 20 + 200;		
		//we range is 0~400, but inverse_Laplace_cdf value -10~10. because -10~10 translate to 0~400.
		random_to_Laplace_pdf[x] = random_variable[i] + random_to_Laplace_pdf[x];
	}


	DrawHistogram(random_variable, 30, y_axis - 100, range);	//print uniform distribution
	DrawHistogram(random_to_Laplace_pdf, 500, y_axis, range);	//print Laplace distribution
	/* 3. check Laplace distribution */
	//print correct answer Laplae distribution
	for (int i = 1; i < range; i++) {			
		float result = Laplace_pdf(i - 1 * range / 2, range / 20, avg, sigma);
		Laplace_pdf_fact[i - 1] = result;
		Draw(result * 250, 500, y_axis, i, RED);
	}


	/* 4. use random_to_Laplace_pdf. make Laplace CDF */
	int total = 0;
	for (int i = 0; i < 401; i++) {						
		total += random_to_Laplace_pdf[i];
		random_to_Laplace_cdf[i] = (double) total / rcnt * 100;
	}


	DrawHistogram(random_to_Laplace_cdf, 30, y_axis + 100, range);	//print we Laplace CDF
	/* 5. check Laplace CDF.(correct Laplace CDF)*/
	for (int i = 1; i < range; i++) {							//print correct Laplace CDF
		float result = Laplace_cdf(i - 1 * range / 2, range / 20, avg, sigma);
		Laplace_cdf_fact[i - 1] = result;
		Draw(result*100, 30, y_axis+100, i, GREEN);
	}

	/* K-S 검증*/
	double end_distance = 0;
	for (int i = 0; i < range; i++) {
		double distance = fabs((double)random_to_Laplace_cdf[i] - Laplace_cdf_fact[i]);
		if (distance > end_distance) {
			end_distance = distance;
		}
	}
	if (KS(end_distance, 0.1, range)) printf("대체적으로 값이 유사합니다.");
	else printf("대체적으로 값이 유사하지 못합니다.");

	return 0;
}