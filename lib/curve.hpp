#ifndef	HEADER_CURVE
#define	HEADER_CURVE

#include <vector> // spacial & table data storage
#include <array> // vector manipulation

// auxiliary functions

std::vector<int> binomialRow(int n);
void powerTable(std::vector<float> &powers, float t, int n);

// line generation

std::array<float,2> getNormalisedVector2(float x, float y);
std::vector<float> lineVectors2(std::vector<float> const &source);

// control data generation

std::vector<float> pointVectors2(std::vector<float> const &positions, int increment);

// curve sample generation

std::vector<float> bezierCurve2(std::vector<float> const &P, int samples); // C0 continuity bezier curve
std::vector<float> bezierCurves2(std::vector<float> const &P, const int piecePoints, const int pieceSamples); // C0 continuity bezier spline
std::vector<float> bezierCubic2(std::vector<float> const &P, std::vector<float> const &V, const int pieceSamples); // C1 continuity bezier spline

#endif