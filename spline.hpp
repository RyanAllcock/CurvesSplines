#include "curve.cpp"

struct SplineType{
	std::vector<float> knots;
	
	// data
	virtual std::vector<float> points();
	virtual std::vector<float> samplePoints(CurveSampler &sampler);
	
	// points
	virtual int getTotalPoints();
	virtual void clearPoints();
	virtual void getLastPoint(float &x, float &y);
	
	// input
	virtual void pushKnot(float x, float y);
	virtual void popKnot();
	void set(std::vector<float> &points);
	void moveKnot(int index, float x, float y);
};
	
struct SplineType_Bezier : SplineType{
	std::vector<std::vector<float>> controlPoints;
	
	// data
	std::vector<float> points();
	std::vector<float> samplePoints(CurveSampler &sampler);
	
	// points
	int getTotalPoints();
	void clearPoints();
	void getLastPoint(float &x, float &y);
	
	// input
	void pushKnot(float x, float y);
	void popKnot();
};

struct SplineType_BezierDegree : SplineType{
	int degree;
	int continuity;
	std::vector<float> controlPoints;
	SplineType_BezierDegree(int _degree, int _continuity);
	
	// data
	std::vector<float> points();
	std::vector<float> samplePoints(CurveSampler &sampler);
	
	// points
	int getTotalPoints();
	void clearPoints();
	void getLastPoint(float &x, float &y);
	
	// input
	void pushKnot(float x, float y);
	void popKnot();
};

struct SplineData{
	std::vector<float> points;
	std::vector<float> samplePoints;
	std::vector<float> sampleVectors;
	void setPoints(std::vector<float> &&_points);
	void setSamples(std::vector<float> &&_samplePoints);
};