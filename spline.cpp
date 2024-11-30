#ifndef HEADER_SPLINE
#define HEADER_SPLINE

#include "spline.hpp"

// spline type methods

std::vector<float> SplineType::points(){
	return std::vector<float>(knots);
}

std::vector<float> SplineType::samplePoints(CurveSampler &sampler){
	return std::vector<float>(knots);
}

int SplineType::getTotalPoints(){
	return knots.size() / 2;
}

void SplineType::clearPoints(){
	knots.clear();
}

void SplineType::getLastPoint(float &x, float &y){
	if(knots.size() < 2){
		x = y = 0;
		return;
	}
	x = knots[knots.size() - 2];
	y = knots[knots.size() - 1];
}

void SplineType::pushKnot(float x, float y){
	knots.push_back(x);
	knots.push_back(y);
}

void SplineType::popKnot(){
	knots.pop_back();
	knots.pop_back();
}

void SplineType::set(std::vector<float> &points){
	clearPoints();
	for(int i = 0; i + 2 <= points.size(); i += 2)
		pushKnot(points[i + 0], points[i + 1]);
}

void SplineType::moveKnot(int index, float x, float y){
	if(knots.size() < 2) return;
	if(index < 0 || index * 2 >= knots.size()) index = knots.size() / 2 - 1;
	knots[index * 2 + 0] = x;
	knots[index * 2 + 1] = y;
}

// spline type bezier methods

std::vector<float> SplineType_Bezier::points(){
	std::vector<float> output;
	for(int i = 0; i * 2 + 2 <= knots.size(); i++){
		output.push_back(knots[i * 2 + 0]);
		output.push_back(knots[i * 2 + 1]);
		if(i >= controlPoints.size()) continue;
		for(int j = 0; j * 2 + 2 <= controlPoints[i].size(); j++){
			output.push_back(controlPoints[i][j * 2 + 0]);
			output.push_back(controlPoints[i][j * 2 + 1]);
		}
	}
	return output;
}

std::vector<float> SplineType_Bezier::samplePoints(CurveSampler &sampler){
	if(knots.size() < 2 * 2) return std::vector<float>(knots);
	return bezierCurve2(knots[0], knots[1], knots[2], knots[3], controlPoints[0].data(), controlPoints[0].size() / 2, sampler);
}

int SplineType_Bezier::getTotalPoints(){
	int total = knots.size() / 2;
	for(std::vector<float> const &piece : controlPoints) total += piece.size() / 2;
	return total;
}

void SplineType_Bezier::clearPoints(){
	controlPoints.clear();
	knots.clear();
}

void SplineType_Bezier::getLastPoint(float &x, float &y){
	if(knots.size() < 1 * 2){
		x = y = 0;
		return;
	}
	if(knots.size() < 2 * 2){
		x = knots[0];
		y = knots[1];
		return;
	}
	x = knots[knots.size() - 2];
	y = knots[knots.size() - 1];
}

void SplineType_Bezier::pushKnot(float x, float y){
	if(knots.size() >= 2 * 2){
		float xPoint, yPoint;
		yPoint = knots.back();
		knots.pop_back();
		xPoint = knots.back();
		knots.pop_back();
		controlPoints.back().push_back(xPoint);
		controlPoints.back().push_back(yPoint);
	}
	else if(knots.size() == 1 * 2) controlPoints.push_back(std::vector<float>{});
	knots.push_back(x);
	knots.push_back(y);
}

void SplineType_Bezier::popKnot(){
	if(knots.size() <= 1 * 2){
		knots.clear();
		return;
	}
	knots.pop_back();
	knots.pop_back();
	if(!controlPoints.empty()){
		if(controlPoints.back().size() >= 1 * 2){
			float x, y;
			y = controlPoints.back().back();
			controlPoints.back().pop_back();
			x = controlPoints.back().back();
			controlPoints.back().pop_back();
			knots.push_back(x);
			knots.push_back(y);
		}
		else controlPoints.pop_back();
	}
}

// spline type bezier constant-degree methods

SplineType_BezierDegree::SplineType_BezierDegree(int _degree, int _continuity) : degree{_degree}, continuity{_continuity} {
	if(continuity > degree + 1) continuity = degree + 1;
}

std::vector<float> SplineType_BezierDegree::points(){
	std::vector<float> output;
	for(int i = 0; i * 2 + 2 <= knots.size(); i++){
		output.push_back(knots[i * 2 + 0]);
		output.push_back(knots[i * 2 + 1]);
		for(int j = 0; j < degree && (i * degree + j) * 2 + 2 <= controlPoints.size(); j++){
			output.push_back(controlPoints[(i * degree + j) * 2 + 0]);
			output.push_back(controlPoints[(i * degree + j) * 2 + 1]);
		}
	}
	return output;
}

std::vector<float> SplineType_BezierDegree::samplePoints(CurveSampler &sampler){
	if(knots.size() < 4) return std::vector<float>{};
	return bezierCurves2(knots.data(), controlPoints.data(), knots.size() / 2 - 1, degree, sampler);
}

int SplineType_BezierDegree::getTotalPoints(){
	return (knots.size() + controlPoints.size()) / 2;
}

void SplineType_BezierDegree::clearPoints(){
	controlPoints.clear();
	knots.clear();
}

void SplineType_BezierDegree::getLastPoint(float &x, float &y){
	if(knots.size() < 1 * 2){
		x = y = 0;
		return;
	}
	if((knots.size() - 1 * 2 + controlPoints.size()) / 2 % (degree + 1) != 0){
		x = controlPoints[controlPoints.size() - 2];
		y = controlPoints[controlPoints.size() - 1];
		return;
	}
	x = knots[knots.size() - 2];
	y = knots[knots.size() - 1];
}

void SplineType_BezierDegree::pushKnot(float x, float y){
	
	// constrained knot
	if(((knots.size() + controlPoints.size()) / 2) % 3 == 0){
		if(knots.size() >= 2 * 2 && degree + 1 <= continuity){ // infinite continuity
			x = 2.f * knots[knots.size() - 2] - knots[knots.size() - 4];
			y = 2.f * knots[knots.size() - 1] - knots[knots.size() - 3];
		}
		knots.push_back(x);
		knots.push_back(y);
		return;
	}
	
	// constrained control point
	if(knots.size() >= 2 * 2 && (controlPoints.size() / 2 % degree) + 1 <= continuity){
		int backward = (controlPoints.size() / 2 % degree) * 2;
		x = 2.f * knots[knots.size() - 2] - controlPoints[controlPoints.size() - backward * 2 - 2];
		y = 2.f * knots[knots.size() - 1] - controlPoints[controlPoints.size() - backward * 2 - 1];
	}
	controlPoints.push_back(x);
	controlPoints.push_back(y);
}

void SplineType_BezierDegree::popKnot(){
	if(((knots.size() + controlPoints.size()) / 2) % 3 == 1){
		knots.pop_back();
		knots.pop_back();
		return;
	}
	controlPoints.pop_back();
	controlPoints.pop_back();
}

// spline data methods

void SplineData::setPoints(std::vector<float> &&_points){
	points = _points;
}

void SplineData::setSamples(std::vector<float> &&_samplePoints){
	samplePoints = _samplePoints;
	sampleVectors = lineVectors2(samplePoints);
}

#endif