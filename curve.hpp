#ifndef	HEADER_CURVE
#define	HEADER_CURVE

#include <vector> // spacial & table data storage
#include <array> // vector manipulation
#include <map> // sampler t-value referencing

#ifndef PI
#define PI 3.14159
#endif

// samplers

struct CurveSampler{
	virtual void setTotal(float _total) = 0;
	virtual void reset() = 0;
	virtual bool isFinished() = 0;
	virtual float get() = 0;
	virtual int capacity() = 0;
	virtual void update(std::vector<float> const &samples){}
	virtual void order(std::vector<float> &samples){}
};

struct CurveSampler_Constant : CurveSampler{
	int resolution;
	int effectiveTotal;
	int total;
	float t, dt;
	int count;
	CurveSampler_Constant(int _resolution, int _total);
	void setTotal(float _total);
	void reset();
	bool isFinished();
	float get();
	int capacity();
	void update(std::vector<float> const &samples);
	void setSpacing();
};

struct CurveSampler_Spatial : CurveSampler{
	float maximumLengthSquared;
	int maximumTotal;
	float t;
	int count;
	std::map<float,int> previousTValues; // used t-value, sample index
	std::multimap<float,float> lengths; // sorted threshold-exceeding length, corresponding segment's lower t-value
	float tValueToRelength;
	CurveSampler_Spatial(float _maximumLength, int _maximumTotal);
	void setTotal(float _total);
	void reset();
	bool isFinished();
	float get();
	int capacity();
	void update(std::vector<float> const &samples);
	void order(std::vector<float> &samples);
};

struct CurveSampler_Curvature : CurveSampler{
	float maximumAngle;
	float maximumDistanceSquared;
	int maximumTotal;
	float t, tCornerStart;
	int count;
	std::map<float,int> previousTValues; // used t-value, sample index
	std::map<float,std::pair<float,bool>> currentAngles; // current angle & subdivision eligibility at each t-value, sorted by t-value
	CurveSampler_Curvature(float maximumAngleDegrees, float maximumDistance, int _maximumTotal);
	void setTotal(float _total);
	void reset();
	bool isFinished();
	float get();
	int capacity();
	void update(std::vector<float> const &samples);
	void order(std::vector<float> &samples);
	void getCornerTValues(float tStart, float (&t)[4]);
	bool isCornerAdequate(float tStart, float &angle, std::vector<float> const &samples);
};

// line generation

inline std::array<float,2> getNormalisedVector2(float x, float y);
inline std::vector<float> lineVectors2(std::vector<float> const &source);

// control data generation

inline std::vector<float> pointVectors2(std::vector<float> const &positions, int increment);

// sample generation

inline std::vector<float> bezierCurve2(float x0, float y0, float x1, float y1, float *controlPoints, int pointCount, CurveSampler &sampler); // bezier curve
inline std::vector<float> bezierCurves2(float *knots, float *controlPoints, const int pieces, const int degree, CurveSampler &sampler); // bezier spline pieces

#endif