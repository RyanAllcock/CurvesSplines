
#include "lib/window/window.hpp" // windowing
#include "lib/camera.hpp" // viewport
#include "lib/shader.hpp" // shader program
#include "source/spline.hpp" // curves & splines

#include "util/filemanager.hpp" // shader source

#include <stdio.h> // testing

// window constants
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_PERSEC 60.f
#define INPUT_PERSEC 60.f

// camera constants
#define CAMERA_FOV 70.f
#define PROJECTION_NEAR -1.f
#define PROJECTION_FAR 1.f

// shader constants
#define MAX_VERTICES 10
#define MAX_LINES 100
#define POINT_RADIUS .03f
#define LINE_THICKNESS .01f
#define VECTOR_THICKNESS .005f
#define VECTOR_LENGTH .1f

// sampler constants
#define SAMPLER_CONSTANT_RESOLUTION 5
#define SAMPLER_SPATIAL_MAXLENGTH .05f
#define SAMPLER_CURVATURE_MAXDIST .05f
#define SAMPLER_CURVATURE_MAXANGLE 5.f

// curve constants
#define CURVE_MAXIMUM_SAMPLES 20
#define SPLINE_MAXIMUM_SAMPLES 8

// input constants
#define INPUT_SELECT_RADIUS .075f
#define INPUT_SELECT_RADIUS_SQUARED (INPUT_SELECT_RADIUS * INPUT_SELECT_RADIUS)

// input
enum ProgramInput{
	InputPlace, InputRemove, // control points
	InputSampler, // curve
	InputSpline // spline
};

void displayCurve(std::vector<Renderer*> const renderers, Window const &window){
	window.clear();
	for(int i = 0; i < renderers.size(); i++) renderers[i]->display();
	window.swap();
}

int main(int argc, char *argv[]){
	
	// input
	std::vector<float> initialPoints{
		0, 0, 
		1, 0, 
		0, .5f, 
		.5f, 1, 
		1, .5f
	};
	
	// samplers
	CurveSampler_Constant samplerConstant(SAMPLER_CONSTANT_RESOLUTION, CURVE_MAXIMUM_SAMPLES);
	CurveSampler_Spatial samplerSpatial(SAMPLER_SPATIAL_MAXLENGTH, CURVE_MAXIMUM_SAMPLES);
	CurveSampler_Curvature samplerCurvature(SAMPLER_CURVATURE_MAXANGLE, SAMPLER_CURVATURE_MAXDIST, CURVE_MAXIMUM_SAMPLES);
	std::vector<CurveSampler*> samplers{ &samplerConstant, &samplerSpatial, &samplerCurvature };
	std::vector<CurveSampler*>::iterator currentSampler = samplers.begin();
	
	// splines
	SplineType_Bezier bezierCurve;
	std::vector<float> bezierCubicBasis = bezierBasis(2 + 2);
	SplineType_Basis cubicSpline(std::vector<float>(bezierCubicBasis), 2, 0);
	SplineType_Basis handledSpline(std::vector<float>(bezierCubicBasis), 2, 1);
	SplineType_Basis naturalSpline(std::vector<float>(bezierCubicBasis), 2, 2);
	SplineType_Basis infiniteSpline(std::vector<float>(bezierCubicBasis), 2, 3);
	SplineType_Basis cardinalSpline(std::vector<float>(bezierCubicBasis), 2, 1, true);
	std::vector<SplineType*> splines{ &bezierCurve, &cubicSpline, &handledSpline, &naturalSpline, &infiniteSpline, &cardinalSpline };
	std::vector<SplineType*>::iterator currentSpline = splines.begin();
	
	// input data
	SplineInput splineInput;
	splineInput.points = std::vector<float>(initialPoints);
	(*currentSpline)->constrain(splineInput.points);
	splineInput.setSamples((*currentSpline)->computeSamples(splineInput.points, **currentSampler));
	
	// window
	Window window("Splines", WindowGraphic, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_PERSEC, INPUT_PERSEC);
	InputBind input(window.getMouseMotionHandle(), window.getMousePositionHandle());
	input.bindAll(std::vector<std::pair<int,WindowKey>>{
		{InputSpline, KeyS}, {InputSampler, KeyC}}, window);
	input.bindAll(std::vector<std::pair<int,WindowButton>>{
		{InputPlace, MouseLeftClick}, {InputRemove, MouseRightClick}}, window);
	float viewport[2];
	window.getViewport(viewport[0], viewport[1]);
	
	// camera
	CameraProjection projection(CameraOrthographic, CAMERA_FOV, PROJECTION_NEAR, PROJECTION_FAR);
	std::array<float,16> projectionMatrix = projection.get(window.getAspectRatio());
	
	// shaders
	Shader pointVertexShader(ShaderVertex, std::vector<const char*>{FileManager::get("shaders/pointVertex.glsl").c_str()});
	Shader pointFragmentShader(ShaderFragment, std::vector<const char*>{FileManager::get("shaders/pointFragment.glsl").c_str()});
	Shader vectorVertexShader(ShaderVertex, std::vector<const char*>{FileManager::get("shaders/vectorVertex.glsl").c_str()});
	Shader vectorFragmentShader(ShaderFragment, std::vector<const char*>{FileManager::get("shaders/vectorFragment.glsl").c_str()});
	Shader lineVertexShader(ShaderVertex, std::vector<const char*>{FileManager::get("shaders/lineVertex.glsl").c_str()});
	Shader lineFragmentShader(ShaderFragment, std::vector<const char*>{FileManager::get("shaders/lineFragment.glsl").c_str()});
		
	// renderer instance
	std::vector<float> quad{
		-1,-1,
		 1,-1,
		 1, 1,
		-1,-1,
		 1, 1,
		-1, 1
	};
	Buffer quadBuffer(BufferStatic, quad.data(), sizeof(float) * quad.size());
	Index quadIndex(quadBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 2, 0);
	
	// point renderer
	Buffer pointBuffer(BufferStream, splineInput.points.data(), sizeof(float) * MAX_VERTICES * 2);
	Index pointIndex(pointBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 2, 0);
	Program pointProgram(std::vector<Shader*>{ &pointVertexShader, &pointFragmentShader });
	DrawInstancedArray pointDraw(DrawTriangle, std::vector<Index*>{ &quadIndex }, quad.size() / 2, std::vector<Index*>{ &pointIndex }, splineInput.points.size() / 2);
	
	// line segment renderer
	Buffer linePositionBuffer(BufferStream, splineInput.samplePoints.data(), sizeof(float) * MAX_LINES * 2);
	Buffer lineDirectionBuffer(BufferStream, splineInput.sampleVectors.data(), sizeof(float) * MAX_LINES * 2);
	Index linePosition0Index(linePositionBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 2, 0);
	Index lineDirection0Index(lineDirectionBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 2, 0);
	Index linePosition1Index(linePositionBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 2, (void*)(sizeof(float) * 2));
	Index lineDirection1Index(lineDirectionBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 2, (void*)(sizeof(float) * 2));
	Program lineProgram(std::vector<Shader*>{ &lineVertexShader, &lineFragmentShader });
	DrawInstancedArray lineDraw(DrawTriangle, std::vector<Index*>{ &quadIndex }, quad.size(), 
		std::vector<Index*>{ &linePosition0Index, &lineDirection0Index, &linePosition1Index, &lineDirection1Index }, splineInput.samplePoints.size() / 2 - 1);
	
	// vector renderer
	Index vectorPosition0Index(pointBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 4, 0);
	Index vectorPosition1Index(pointBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 4, (void*)(sizeof(float) * 2));
	Program vectorProgram(std::vector<Shader*>{ &vectorVertexShader, &vectorFragmentShader });
	DrawInstancedArray vectorPointDraw(DrawTriangle, std::vector<Index*>{ &quadIndex }, quad.size() / 2, 
		std::vector<Index*>{ &vectorPosition0Index }, splineInput.points.size() / 4);
	DrawInstancedArray vectorDirectionDraw(DrawTriangle, std::vector<Index*>{ &quadIndex }, quad.size(),
		std::vector<Index*>{ &vectorPosition0Index, &vectorPosition1Index }, splineInput.points.size() / 4);
	
	// renderers
	std::vector<Renderer> renderers{
		Renderer(pointProgram, pointDraw), 
		Renderer(pointProgram, vectorPointDraw), 
		Renderer(vectorProgram, vectorDirectionDraw), 
		Renderer(lineProgram, lineDraw)
	};
	std::vector<std::vector<Renderer*>> currentRenderers{
		std::vector<Renderer*>{ &renderers[0], &renderers[3] }, 
		std::vector<Renderer*>{ &renderers[1], &renderers[2], &renderers[3] }
	};
	
	// shader uniforms
	std::array<float,16> defaultTransform{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1};
	pointProgram.setUniform("view_projection", DataMatrix4(projectionMatrix, DataUnchanged));
	pointProgram.setUniform("point_radius", DataFloat(POINT_RADIUS));
	vectorProgram.setUniform("view_projection", DataMatrix4(projectionMatrix, DataUnchanged));
	vectorProgram.setUniform("vector_thickness", DataFloat(VECTOR_THICKNESS));
	vectorProgram.setUniform("vector_length", DataFloat(VECTOR_LENGTH));
	lineProgram.setUniform("view_projection", DataMatrix4(projectionMatrix, DataUnchanged));
	lineProgram.setUniform("line_thickness", DataFloat(LINE_THICKNESS));
	
	// first display
	displayCurve(currentRenderers[0], window);
	bool isDataOutdated = false;
	
	// loop
	bool isRunning = true;
	while(isRunning){
		
		// window behaviour
		WindowState windowState;
		while((windowState = window.get(), windowState) != WindowDefault){
			switch(windowState){
				case WindowQuit:
					isRunning = false;
					break;
				case WindowFocus:
					input.setActive(true);
					break;
				case WindowUnfocus:
					input.setActive(false);
					break;
			}
		}
		
		// input
		if(window.cap(WindowInput)){
			
			// drag-move selected point
			if(splineInput.selectedPoint != -1){
				if(input.getHold(InputPlace)){
					
					// position
					float placeAt[2];
					input.getMousePosition(placeAt);
					placeAt[0] *= viewport[0];
					placeAt[1] *= viewport[1];
					
					// move
					splineInput.movePoint(splineInput.selectedPoint, placeAt[0], placeAt[1]);
					
					// update
					pointBuffer.update(&placeAt[0], sizeof(float) * 2, sizeof(float) * splineInput.selectedPoint * 2);
					isDataOutdated = true;
				}
				else{
					
					// drop
					(*currentSpline)->constrain(splineInput.selectedPoint, splineInput.points);
					pointBuffer.update(splineInput.points.data(), sizeof(float) * splineInput.points.size(), 0);
					isDataOutdated = true;
					printf("Moved point %i\n", splineInput.selectedPoint);
					splineInput.setSelectedPoint(-1);
				}
			}
			
			// push or select point
			else if(input.getPress(InputPlace)){
				
				// position
				float placeAt[2];
				input.getMousePosition(placeAt);
				placeAt[0] *= viewport[0];
				placeAt[1] *= viewport[1];
					
				// select
				float selectDistanceSquared;
				int selectIndex = splineInput.getClosestPoint(placeAt[0], placeAt[1], selectDistanceSquared);
				if(selectIndex != -1 && selectDistanceSquared < INPUT_SELECT_RADIUS_SQUARED) splineInput.setSelectedPoint(selectIndex);
				
				// add
				else if(splineInput.points.size() / 2 < MAX_VERTICES){
					
					// push
					splineInput.pushPoint(placeAt[0], placeAt[1]);
					(*currentSpline)->constrainPoint(splineInput.points.size() / 2 - 1, -1, splineInput.points);
					pointDraw.recount(splineInput.points.size() / 2);
					vectorPointDraw.recount(splineInput.points.size() / 4);
					
					// update
					pointBuffer.update(&splineInput.points[splineInput.points.size() - 2], sizeof(float) * 2, sizeof(float) * (splineInput.points.size() - 2));
					isDataOutdated = true;
					printf("Added point %i\n", splineInput.points.size() / 2);
				}
			}
			
			// pop point
			if(input.getPress(InputRemove)){
				if(!splineInput.points.empty()){
					splineInput.popPoint();
					pointDraw.recount(splineInput.points.size() / 2);
					vectorPointDraw.recount(splineInput.points.size() / 4);
					isDataOutdated = true;
					printf("Removed point %i\n", splineInput.points.size() / 2);
				}
			}
			
			// toggle curve sampler
			if(input.getPress(InputSampler)){
				currentSampler++;
				if(currentSampler == samplers.end()) currentSampler = samplers.begin();
				isDataOutdated = true;
				printf("Toggled curve sampler\n");
			}
			
			// toggle spline type
			if(input.getPress(InputSpline)){
				currentSpline++;
				if(currentSpline == splines.end()) currentSpline = splines.begin();
				if(currentSpline - splines.begin() == 0) (*currentSampler)->setTotal(CURVE_MAXIMUM_SAMPLES);
				else (*currentSampler)->setTotal(SPLINE_MAXIMUM_SAMPLES);
				(*currentSpline)->constrain(splineInput.points);
				pointBuffer.update(splineInput.points.data(), sizeof(float) * splineInput.points.size(), 0);
				isDataOutdated = true;
				printf("Toggled spline type\n");
			}
			
			// update sample curve
			if(isDataOutdated){
				splineInput.setSamples((*currentSpline)->computeSamples(splineInput.points, **currentSampler));
				lineDraw.recount(splineInput.samplePoints.size() / 2 - 1);
				vectorDirectionDraw.recount(splineInput.points.size() / 4);
				linePositionBuffer.update(splineInput.samplePoints.data(), sizeof(float) * splineInput.samplePoints.size(), 0);
				lineDirectionBuffer.update(splineInput.sampleVectors.data(), sizeof(float) * splineInput.samplePoints.size(), 0);
				displayCurve(currentRenderers[0], window);
				isDataOutdated = false;
			}
		}
	}
	
	return 0;
}