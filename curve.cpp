#include "curve.hpp"

#include <cmath> // float absolute value & squared number

// generation auxiliary functions

inline float getAngleFromLines(float x1, float y1, float x2, float y2){
	float angle1 = (y1 != 0 ? fabs(atan(x1 / y1)) : PI / 2.f);
	float angle2 = (y2 != 0 ? fabs(atan(x2 / y2)) : PI / 2.f);
	if(y1 < 0) angle1 = PI - angle1;
	if(y2 < 0) angle2 = PI - angle2;
	if(x1 < 0) angle1 = 2.f * PI - angle1;
	if(x2 < 0) angle2 = 2.f * PI - angle2;
	float angle = fabs(angle1 - angle2);
	if(angle > PI) angle = 2.f * PI - angle;
	return angle;
}

inline std::vector<int> binomialRow(int n){
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

inline std::vector<float> bezierWeightTable(float t, int n){
	std::vector<float> weights;
	weights.reserve(n + 1);
	weights.push_back(1);
	float value = 1.f;
	for(int i = 1; i <= n; i++){
		value *= t;
		weights.push_back(value);
	}
	float tMinus = 1.f - t;
	value = 1;
	for(int i = n - 1; i >= 0; i--){
		value *= tMinus;
		weights[i] *= value;
	}
	return weights;
}

inline std::vector<float> bezierWeightTables(int n, int samples){
	float t = 0;
	float dt = 1.f / (samples - 1);
	std::vector<float> weights;
	weights.reserve((n + 1) * samples);
	for(int j = 0; j < samples; j++){
		weights.push_back(1);
		float value = 1.f;
		for(int i = 1; i <= n; i++){
			value *= t;
			weights.push_back(value);
		}
		float tMinus = 1.f - t;
		value = 1;
		for(int i = n - 1; i >= 0; i--){
			value *= tMinus;
			weights[j * (n + 1) + i] *= value;
		}
		t += dt;
	}
	return weights;
}

// spline sampler constant total methods

inline CurveSampler_Constant::CurveSampler_Constant(int _resolution, int _total) : resolution{_resolution}, total{_total}, t{0}, count{0} {
	setSpacing();
}

inline void CurveSampler_Constant::setTotal(float _total){
	total = _total;
	setSpacing();
}

inline void CurveSampler_Constant::reset(){
	t = 0;
	count = 0;
}

inline bool CurveSampler_Constant::isFinished(){
	return count >= effectiveTotal;
}

inline float CurveSampler_Constant::get(){
	count++;
	return t;
}

inline int CurveSampler_Constant::capacity(){
	return effectiveTotal;
}

inline void CurveSampler_Constant::update(std::vector<float> const &samples){
	t += dt;
}

inline void CurveSampler_Constant::setSpacing(){
	int effectiveResolution = resolution;
	while((effectiveTotal = 1 << effectiveResolution - 1 + 2, effectiveTotal) > total){
		if(effectiveResolution <= 0) break;
		effectiveResolution--;
	}
	dt = 1.f / (effectiveTotal - 1);
}

// spline sampler constant spacing methods

inline CurveSampler_Spatial::CurveSampler_Spatial(float _maximumLength, int _maximumTotal) : maximumTotal{_maximumTotal}, t{0}, count{0} {
	maximumLengthSquared = _maximumLength * _maximumLength;
}

inline void CurveSampler_Spatial::setTotal(float _total){
	maximumTotal = _total;
}

inline void CurveSampler_Spatial::reset(){
	t = 0;
	count = 0;
	previousTValues.clear();
}

inline bool CurveSampler_Spatial::isFinished(){
	return t < 0 || count >= maximumTotal;
}

inline float CurveSampler_Spatial::get(){
	previousTValues.insert(std::pair<float,int>{t, count});
	count++;
	return t;
}

inline int CurveSampler_Spatial::capacity(){
	return 2;
}

inline void CurveSampler_Spatial::update(std::vector<float> const &samples){
	float tNewlyAdded = t;
	t = -1;
	
	// enforce sample count upper limit
	if(count >= maximumTotal) return;
	
	// ensure minimum t-values are set
	if(previousTValues.size() == 0){
		t = 0;
		return;
	}
	else if(previousTValues.size() == 1){
		t = 1;
		return;
	}
	
	// compute new segment lengths
	else if(previousTValues.size() == 2){
		std::map<float,int>::iterator tValueIterator = previousTValues.begin();
		float x0 = samples[tValueIterator->second * 2 + 0];
		float y0 = samples[tValueIterator->second * 2 + 1];
		tValueIterator++;
		float x1 = samples[tValueIterator->second * 2 + 0];
		float y1 = samples[tValueIterator->second * 2 + 1];
		float length = pow(x1 - x0, 2) + pow(y1 - y0, 2);
		if(length > maximumLengthSquared) lengths.insert(std::pair<float,float>{length, previousTValues.begin()->first});
	}
	else{
		std::map<float,int>::iterator tValueIterator = previousTValues.find(tValueToRelength);
		int i1 = tValueIterator->second;
		tValueIterator++;
		int i2 = tValueIterator->second;
		tValueIterator++;
		int i3 = tValueIterator->second;
		float x1 = samples[i1 * 2 + 0];
		float y1 = samples[i1 * 2 + 1];
		float x2 = samples[i2 * 2 + 0];
		float y2 = samples[i2 * 2 + 1];
		float x3 = samples[i3 * 2 + 0];
		float y3 = samples[i3 * 2 + 1];
		float l1 = pow(x2 - x1, 2) + pow(y2 - y1, 2);
		float l2 = pow(x3 - x2, 2) + pow(y3 - y2, 2);
		if(l1 > maximumLengthSquared) lengths.insert(std::pair<float,float>{l1, tValueToRelength});
		if(l2 > maximumLengthSquared) lengths.insert(std::pair<float,float>{l2, tNewlyAdded});
	}
	
	// all lengths below threshold
	if(lengths.empty()) return;
	
	// remove length
	std::map<float,float>::iterator tValueLength = lengths.end();
	tValueLength--;
	tValueToRelength = tValueLength->second;
	lengths.erase(tValueLength);
	
	// compute bisecting t-value
	std::map<float,int>::iterator tValueIterator = previousTValues.find(tValueToRelength);
	tValueIterator++;
	float tNext = tValueIterator->first;
	t = (tValueToRelength + tNext) / 2.f;
}

inline void CurveSampler_Spatial::order(std::vector<float> &samples){
	std::vector<float> oldSamples(samples);
	int i = 0;
	for(std::map<float,int>::iterator it = previousTValues.begin(); it != previousTValues.end(); it++, i++){
		samples[i * 2 + 0] = oldSamples[it->second * 2 + 0];
		samples[i * 2 + 1] = oldSamples[it->second * 2 + 1];
	}
}
// spline sampler curvature dependent methods

inline CurveSampler_Curvature::CurveSampler_Curvature(float maximumAngleDegrees, float maximumDistance, int _maximumTotal) : maximumTotal{_maximumTotal}, t{0}, count{0} {
	maximumAngle = PI * maximumAngleDegrees / 180.f;
	maximumDistanceSquared = maximumDistance * maximumDistance;
}

inline void CurveSampler_Curvature::setTotal(float _total){
	maximumTotal = _total;
}

inline void CurveSampler_Curvature::reset(){
	t = 0;
	count = 0;
	previousTValues.clear();
	currentAngles.clear();
}

inline bool CurveSampler_Curvature::isFinished(){
	return t < 0 || count >= maximumTotal;
}

inline float CurveSampler_Curvature::get(){
	previousTValues.insert(std::pair<float,int>{t, count});
	count++;
	return t;
}

inline int CurveSampler_Curvature::capacity(){
	return 2;
}

inline void CurveSampler_Curvature::update(std::vector<float> const &samples){
	
	// enforce sample count upper limit
	if(count >= maximumTotal){
		t = -1;
		return;
	}
	
	// set minimim t-values for angles & tangent distances to be obtainable
	if(previousTValues.size() == 0){
		t = 0;
		return;
	}
	else if(previousTValues.size() == 1){
		t = 1;
		return;
	}
	else if(previousTValues.size() == 2){
		std::map<float,int>::iterator tValueIterator = previousTValues.begin();
		float t0 = tValueIterator->first;
		tValueIterator++;
		float t1 = tValueIterator->first;
		t = (t0 + t1) / 2.f;
		tCornerStart = t0;
		currentAngles.insert(std::pair<float,std::pair<float,bool>>{t0, {0, false}});
		currentAngles.insert(std::pair<float,std::pair<float,bool>>{t1, {-1, false}});
		return;
	}
	
	// compute & enter new corner's angle & distance
	float tParts[4];
	getCornerTValues(tCornerStart, tParts);
	float angle;
	bool isAdequate = isCornerAdequate(tParts[1], angle, samples);
	if(tParts[0] >= 0.f){
		float newAngle;
		bool isNewAdequate = isCornerAdequate(tParts[0], newAngle, samples);
		currentAngles[tParts[0]] = std::pair<float,bool>{newAngle, !isNewAdequate};
	}
	currentAngles[tParts[1]] = std::pair<float,bool>{angle, !isAdequate};
	currentAngles.insert(std::pair<float,std::pair<float,bool>>{tParts[2], {angle, !isAdequate}});
	if(tParts[3] < 1.f){
		float newAngle;
		bool isNewAdequate = isCornerAdequate(tParts[3], newAngle, samples);
		currentAngles[tParts[3]] = std::pair<float,bool>{newAngle, !isNewAdequate};
	}
	
	// select t-value to compute next
	t = -1;
	while(true){
		
		// select largest flagged angle
		std::map<float,std::pair<float,bool>>::iterator currentAngleIterator = currentAngles.begin(), selectedAngle;
		float currentLargestAngle = -1;
		while(currentAngleIterator != currentAngles.end()){
			if(currentLargestAngle < currentAngleIterator->second.first && currentAngleIterator->second.second){
				selectedAngle = currentAngleIterator;
				currentLargestAngle = currentAngleIterator->second.first;
			}
			currentAngleIterator++;
		}
		if(currentLargestAngle < 0) return;
		
		// get bisected t-value
		float t1 = selectedAngle->first;
		selectedAngle++;
		float t2 = selectedAngle->first;
		t = (t1 + t2) / 2.f;
		tCornerStart = t1;
		break;
	}
}

inline void CurveSampler_Curvature::order(std::vector<float> &samples){
	std::vector<float> oldSamples(samples);
	int i = 0;
	for(std::map<float,int>::iterator it = previousTValues.begin(); it != previousTValues.end(); it++, i++){
		samples[i * 2 + 0] = oldSamples[it->second * 2 + 0];
		samples[i * 2 + 1] = oldSamples[it->second * 2 + 1];
	}
}

inline void CurveSampler_Curvature::getCornerTValues(float tStart, float (&t)[4]){
	std::map<float,int>::iterator tValueIterator = previousTValues.find(tStart);
	t[0] = -1;
	if(tValueIterator != previousTValues.begin()){
		tValueIterator--;
		t[0] = tValueIterator->first;
		tValueIterator++;
	}
	t[1] = tValueIterator->first;
	tValueIterator++;
	t[2] = tValueIterator->first;
	tValueIterator++;
	t[3] = tValueIterator->first;
}

inline bool CurveSampler_Curvature::isCornerAdequate(float tStart, float &angle, std::vector<float> const &samples){
	std::map<float,int>::iterator tValueIterator = previousTValues.find(tStart);
	int i1 = tValueIterator->second;
	tValueIterator++;
	int i2 = tValueIterator->second;
	tValueIterator++;
	int i3 = tValueIterator->second;
	float x1 = samples[i1 * 2 + 0];
	float y1 = samples[i1 * 2 + 1];
	float x2 = samples[i2 * 2 + 0];
	float y2 = samples[i2 * 2 + 1];
	float x3 = samples[i3 * 2 + 0];
	float y3 = samples[i3 * 2 + 1];
	angle = getAngleFromLines(x2 - x1, y2 - y1, x3 - x2, y3 - y2);
	if(angle > maximumAngle) return false;
	float distanceSquared = pow(sin(angle), 2) * (pow(x2 - x1, 2) + pow(y2 - y1, 2));
	if(distanceSquared > maximumDistanceSquared) return false;
	return true;
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

// generation functions

std::vector<float> bezierCurve2(float x0, float y0, float x1, float y1, float *controlPoints, int controlPointCount, CurveSampler &sampler){
	std::vector<float> samples;
	int n = controlPointCount + 2 - 1;
	
	// binomial table
	std::vector<int> binomial = binomialRow(n + 1);
	
	// sample points
	samples.reserve(sampler.capacity() * 2);
	while(!sampler.isFinished()){
		float t = sampler.get();
		
		// weight table
		std::vector<float> weights = bezierWeightTable(t, n);
		
		// solution
		float x = weights[0] * x0 + weights[n] * x1;
		float y = weights[0] * y0 + weights[n] * y1;
		for(int i = 1; i < n; i++){
			float alpha = (float)binomial[i] * weights[i];
			x += alpha * controlPoints[(i - 1) * 2 + 0];
			y += alpha * controlPoints[(i - 1) * 2 + 1];
		}
		samples.push_back(x);
		samples.push_back(y);
		
		// next sample
		sampler.update(samples);
	}
	samples.shrink_to_fit();
	
	// finalise samples
	sampler.order(samples);
	sampler.reset();
	
	return samples;
}

std::vector<float> bezierCurves2(float *knots, float *controlPoints, const int pieces, const int degree, CurveSampler &sampler){
	std::vector<float> samples;
	const int piecePoints = degree + 2;
	const int n = piecePoints - 1;
	if(n <= 0) return samples;
	samples.reserve((pieces * (sampler.capacity() - 1) + 1) * 2);
	
	// precompute tables
	std::vector<int> binomial = binomialRow(n + 1);
	
	// solution
	for(int j = 0; j < pieces; j++){
		std::vector<float> newSamples;
		newSamples.reserve(sampler.capacity() * 2);
		while(!sampler.isFinished()){
			float t = sampler.get();
			
			// weight table
			std::vector<float> weights = bezierWeightTable(t, n);
			
			// solution
			float x = weights[0] * knots[j * 2 + 0] + weights[n] * knots[j * 2 + 2];
			float y = weights[0] * knots[j * 2 + 1] + weights[n] * knots[j * 2 + 3];
			for(int i = 1; i < n; i++){
				float alpha = (float)binomial[i] * weights[i];
				x += alpha * controlPoints[(j * degree + i - 1) * 2 + 0];
				y += alpha * controlPoints[(j * degree + i - 1) * 2 + 1];
			}
			newSamples.push_back(x);
			newSamples.push_back(y);
			
			// next sample
			sampler.update(newSamples);
		}
		sampler.order(newSamples);
		sampler.reset();
		if(j + 1 < pieces){
			newSamples.pop_back();
			newSamples.pop_back();
		}
		samples.insert(samples.end(), std::make_move_iterator(newSamples.begin()), std::make_move_iterator(newSamples.end()));
	}
	samples.shrink_to_fit();
	return samples;
}