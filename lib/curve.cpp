#include "curve.hpp"

#include <cmath> // float absolute value

// auxiliary functions

std::vector<int> binomialRow(int n){
	std::vector<int> binomial;
	binomial.reserve(n);
	int biNum = 1;
	int biDen = 1;
	binomial.push_back(1);
	for(int i = 1; i < n; i++){
		biNum *= n - i;
		biDen *= i;
		binomial.push_back(biNum / biDen);
	}
	return binomial;
}

void powerTable(std::vector<float> &powers, float t, int n){
	powers.push_back(1);
	float value = 1;
	for(int i = 1; i <= n; i++){
		value *= t;
		powers.push_back(value);
	}
}

// control data generation

std::vector<float> pointVectors2(std::vector<float> const &positions, int increment){
	increment *= 2;
	std::vector<float> vectors;
	vectors.reserve((positions.size() / increment) * 2);
	for(int i = 0; i + 4 <= positions.size(); i += increment){
		float x = positions[i + 2] - positions[i + 0];
		float y = positions[i + 3] - positions[i + 1];
		vectors.push_back(x);
		vectors.push_back(y);
	}
	return vectors;
}

// line generation

std::array<float,2> getNormalisedVector2(float x, float y){
	float total = sqrt(x * x + y * y);
	if(total == 0) return std::array<float,2>{0,1};
	return std::array<float,2>{x / total, y / total};
}

std::vector<float> lineVectors2(std::vector<float> const &source){
	std::vector<float> vectors;
	float x0, y0, x1, y1;
	std::array<float,2> v0, v1;
	float root2 = sqrt(2.f); // TODO
	
	// no line
	if(source.size() < 4) return vectors;
	
	// first
	x0 = source[0];
	y0 = source[1];
	x1 = source[2];
	y1 = source[3];
	v1 = getNormalisedVector2(x1 - x0, y1 - y0);
	vectors.push_back(v1[0]);
	vectors.push_back(v1[1]);
	
	for(int i = 2; i + 2 < source.size(); i += 2){
		
		// next
		v0 = v1;
		x0 = source[i + 0];
		y0 = source[i + 1];
		x1 = source[i + 2];
		y1 = source[i + 3];
		v1 = getNormalisedVector2(x1 - x0, y1 - y0);
		
		// thick line vector proof, for code below: 
		// - present vector to trapezoidal corner of two adjacent line segments which preserves unit thickness on each side
		// - leads to many connected & consistently thick line segments
		// (1) sin(angle) = opposite / hypotenuse: sine trigonometric rule
		// (2) a: line 0 basis vector, b: line 1 basis vector, h = 1: thickness of line
		// (3) (a + b) / 2 = c: basis vector bisecting a and b representing corner vector direction
		// (4) |a . c|: minor angle [0 to 90 degrees] between a and c
		// (5) h[opposite] / |a . c|[sin(angle)] = r[hypotenuse]: magnitude representing corner vector length
		// (6) v = r * c: corner vector at point connecting lines 0 and 1
		
		// joint for smoothly connecting line segments
		std::array<float,2> join;
		join[0] = (v0[0] + v1[0]) / 2;
		join[1] = (v0[1] + v1[1]) / 2;
		float dot = v0[0] * join[0] + v0[1] * join[1];
		float length = 1.f / std::fabs(dot);
		vectors.push_back(join[0] * length);
		vectors.push_back(join[1] * length);
	}
	
	// last
	vectors.push_back(v1[0]);
	vectors.push_back(v1[1]);
	
	return vectors;
}

// curve sample generation

std::vector<float> bezierCurve2(std::vector<float> const &P, int samples){
	std::vector<float> B;
	int n = (int)P.size() / 2 - 1;
	B.reserve(samples * 2);
	
	// binomial table
	std::vector<int> binomial = binomialRow(n + 1);
	
	// sample points
	float t = 0;
	float tMinus = 1.f - t;
	float dt = 1.f / (samples - 1);
	for(int j = 0; j < samples; j++){
	
		// power table
		std::vector<float> tPow, tMinusPow;
		tPow.reserve(n + 1);
		tMinusPow.reserve(n + 1);
		powerTable(tPow, t, n);
		powerTable(tMinusPow, tMinus, n);
		
		// solution
		float x = 0;
		float y = 0;
		for(int i = 0; i <= n; i++){
			float alpha = (float)binomial[i] * tMinusPow[n - i] * tPow[i];
			x += alpha * P[i * 2 + 0];
			y += alpha * P[i * 2 + 1];
		}
		B.push_back(x);
		B.push_back(y);
		
		// next sample
		t += dt;
		tMinus = 1.f - t;
	}
	return B;
}

std::vector<float> bezierCurves2(std::vector<float> const &P, const int piecePoints, const int pieceSamples){
	std::vector<float> B;
	const int n = piecePoints - 1;
	if(n <= 0) return B;
	const int pieces = ((int)P.size() / 2 - 1) / n;
	B.reserve((pieces * (pieceSamples - 1) + 1) * 2);
	
	// binomial table
	std::vector<int> binomial = binomialRow(n + 1);
	
	// power table
	std::vector<float> tPow;
	tPow.reserve(piecePoints * pieceSamples);
	float t = 0;
	float dt = 1.f / (pieceSamples - 1);
	for(int j = 0; j < pieceSamples; j++){
		powerTable(tPow, t, n);
		t += dt;
	}
	
	// solution
	int midPiece = 0;
	for(int i = 0; i < pieces; i++){
		for(int j = midPiece; j < pieceSamples; j++){
			float x = 0;
			float y = 0;
			for(int k = 0; k <= n; k++){
				float alpha = binomial[k] * tPow[(pieceSamples - 1 - j) * piecePoints + n - k] * tPow[j * piecePoints + k];
				x += alpha * P[(i * n + k) * 2 + 0];
				y += alpha * P[(i * n + k) * 2 + 1];
			}
			B.push_back(x);
			B.push_back(y);
		}
		midPiece = 1;
	}
	return B;
}

std::vector<float> bezierCubic2(std::vector<float> const &P, std::vector<float> const &V, const int pieceSamples){
	std::vector<float> B;
	const int n = 3;
	const int pieces = (int)P.size() / 4 - 1;
	if(pieces < 0) return B;
	B.reserve((pieces * (pieceSamples - 1) + 1) * 2);
	
	// binomial table
	const int binomial[n + 1] = {1, 3, 3, 1};
	
	// power table
	std::vector<float> tPow;
	tPow.reserve((n + 1) * pieceSamples);
	float t = 0;
	float dt = 1.f / (pieceSamples - 1);
	for(int j = 0; j < pieceSamples; j++){
		powerTable(tPow, t, n);
		t += dt;
	}
	
	// solution
	int midPiece = 0;
	for(int i = 0; i < pieces; i++){
		for(int j = midPiece; j < pieceSamples; j++){
			
			// knots
			float knots[(n + 1) * 2];
			knots[0 * 2 + 0] = P[(i + 0) * 4 + 0];
			knots[0 * 2 + 1] = P[(i + 0) * 4 + 1];
			knots[1 * 2 + 0] = P[(i + 0) * 4 + 0] + V[(i + 0) * 2 + 0];
			knots[1 * 2 + 1] = P[(i + 0) * 4 + 1] + V[(i + 0) * 2 + 1];
			knots[2 * 2 + 0] = P[(i + 1) * 4 + 0] - V[(i + 1) * 2 + 0];
			knots[2 * 2 + 1] = P[(i + 1) * 4 + 1] - V[(i + 1) * 2 + 1];
			knots[3 * 2 + 0] = P[(i + 1) * 4 + 0];
			knots[3 * 2 + 1] = P[(i + 1) * 4 + 1];
			
			// summation
			float x = 0;
			float y = 0;
			for(int k = 0; k <= n; k++){
				float alpha = binomial[k] * tPow[(pieceSamples - 1 - j) * (n + 1) + n - k] * tPow[j * (n + 1) + k];
				x += alpha * knots[k * 2 + 0];
				y += alpha * knots[k * 2 + 1];
			}
			B.push_back(x);
			B.push_back(y);
			
		}
		midPiece = 1;
	}
	return B;
}