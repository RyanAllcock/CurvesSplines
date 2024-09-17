
#include "lib/window/window.hpp" // windowing
#include "lib/camera.hpp" // viewport
#include "lib/shader.hpp" // shader program
#include "source/curve.hpp" // curves & splines

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

// bezier curve constants
#define TOTAL_BEZIER_SAMPLES 20
#define CUBIC_BEZIER_SAMPLES 8

// options
enum RenderType{ RenderCurve, RenderCubic };
enum BezierType{ BezierCurve, BezierSpline };

// input
enum ProgramInput{
	InputPlace, InputRemove, // control points
	InputContinuity, InputCurve // spline
};

void recomputeCurve(
		BezierType bezierType, RenderType renderType, // choices
		std::vector<float> const &controlPoints, std::vector<float> &samplePoints, std::vector<float> &sampleVectors, // data
		DrawArray &vectorDirectionDraw, DrawArray &lineDraw){ // drawing
	switch(bezierType){
		case BezierCurve:
			samplePoints = bezierCurve2(controlPoints, TOTAL_BEZIER_SAMPLES);
			break;
		case BezierSpline:
			switch(renderType){
				case RenderCurve:
					samplePoints = bezierCurves2(controlPoints, 4, CUBIC_BEZIER_SAMPLES);
					break;
				case RenderCubic:
					std::vector<float> controlVectors = pointVectors2(controlPoints, 2);
					samplePoints = bezierCubic2(controlPoints, controlVectors, CUBIC_BEZIER_SAMPLES);
					vectorDirectionDraw.recount(controlPoints.size() / 4);
					break;
			}
			break;
	}
	if(samplePoints.size() / 2 > MAX_LINES) samplePoints.resize(MAX_LINES * 2);
	sampleVectors = lineVectors2(samplePoints);
	lineDraw.recount(samplePoints.size() / 2 - 1);
}

void displayCurve(std::vector<Renderer*> const renderers, Window const &window){
	window.clear();
	for(int i = 0; i < renderers.size(); i++) renderers[i]->display();
	window.swap();
}

int main(int argc, char *argv[]){
	
	// options
	RenderType renderType = RenderCurve;
	BezierType bezierType = BezierCurve;
	
	// initial data
	std::vector<float> controlPoints{
		0, 0, 
		1, 0, 
		0, .5f, 
		.5f, 1, 
		1, .5f
	};
	std::vector<float> samplePoints = bezierCurve2(controlPoints, TOTAL_BEZIER_SAMPLES);
	std::vector<float> sampleVectors = lineVectors2(samplePoints);
	
	// window
	Window window("Curves", WindowGraphic, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_PERSEC, INPUT_PERSEC);
	InputBind input(window.getMouseMotionHandle(), window.getMousePositionHandle());
	input.bindAll(std::vector<std::pair<int,WindowKey>>{
		{InputContinuity, KeyC}, {InputCurve, KeyS}}, window);
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
	Buffer pointBuffer(BufferStream, controlPoints.data(), sizeof(float) * MAX_VERTICES * 2);
	Index pointIndex(pointBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 2, 0);
	Program pointProgram(std::vector<Shader*>{ &pointVertexShader, &pointFragmentShader });
	DrawInstancedArray pointDraw(DrawTriangle, std::vector<Index*>{ &quadIndex }, quad.size() / 2, std::vector<Index*>{ &pointIndex }, controlPoints.size() / 2);
	
	// line segment renderer
	Buffer linePositionBuffer(BufferStream, samplePoints.data(), sizeof(float) * MAX_LINES * 2);
	Buffer lineDirectionBuffer(BufferStream, sampleVectors.data(), sizeof(float) * MAX_LINES * 2);
	Index linePosition0Index(linePositionBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 2, 0);
	Index lineDirection0Index(lineDirectionBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 2, 0);
	Index linePosition1Index(linePositionBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 2, (void*)(sizeof(float) * 2));
	Index lineDirection1Index(lineDirectionBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 2, (void*)(sizeof(float) * 2));
	Program lineProgram(std::vector<Shader*>{ &lineVertexShader, &lineFragmentShader });
	DrawInstancedArray lineDraw(DrawTriangle, std::vector<Index*>{ &quadIndex }, quad.size(), 
		std::vector<Index*>{ &linePosition0Index, &lineDirection0Index, &linePosition1Index, &lineDirection1Index }, samplePoints.size() / 2 - 1);
	
	// vector renderer
	Index vectorPosition0Index(pointBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 4, 0);
	Index vectorPosition1Index(pointBuffer, 2, IndexFloat, IndexUnchanged, sizeof(float) * 4, (void*)(sizeof(float) * 2));
	Program vectorProgram(std::vector<Shader*>{ &vectorVertexShader, &vectorFragmentShader });
	DrawInstancedArray vectorPointDraw(DrawTriangle, std::vector<Index*>{ &quadIndex }, quad.size() / 2, 
		std::vector<Index*>{ &vectorPosition0Index }, controlPoints.size() / 4);
	DrawInstancedArray vectorDirectionDraw(DrawTriangle, std::vector<Index*>{ &quadIndex }, quad.size(),
		std::vector<Index*>{ &vectorPosition0Index, &vectorPosition1Index }, controlPoints.size() / 4);
	
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
	displayCurve(currentRenderers[renderType], window);
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
			
			// place control point
			if(input.getPress(InputPlace)){
				
				// position
				float placeAt[2];
				input.getMousePosition(placeAt);
				placeAt[0] *= viewport[0];
				placeAt[1] *= viewport[1];
				
				// update
				if(controlPoints.size() / 2 < MAX_VERTICES){
					controlPoints.push_back(placeAt[0]);
					controlPoints.push_back(placeAt[1]);
					pointDraw.recount(controlPoints.size() / 2);
					vectorPointDraw.recount(controlPoints.size() / 4);
				}
				else{
					controlPoints[(MAX_VERTICES - 1) * 2 + 0] = placeAt[0];
					controlPoints[(MAX_VERTICES - 1) * 2 + 1] = placeAt[1];
				}
				pointBuffer.update(&placeAt, sizeof(float) * 2, sizeof(float) * (controlPoints.size() / 2 - 1) * 2);
				isDataOutdated = true;
				printf("Added control point %i\n", controlPoints.size() / 2);
			}
			
			// remove control point
			if(input.getPress(InputRemove)){
				if(!controlPoints.empty()){
					controlPoints.pop_back();
					controlPoints.pop_back();
					pointDraw.recount(controlPoints.size() / 2);
					vectorPointDraw.recount(controlPoints.size() / 4);
					isDataOutdated = true;
					printf("Removed control point %i\n", controlPoints.size() / 2);
				}
			}
			
			// toggle spline continuity
			if(input.getPress(InputContinuity)){
				renderType = (renderType == RenderCurve ? RenderCubic : RenderCurve);
				isDataOutdated = true;
				printf("Toggled spline continuity\n");
			}
			
			// toggle between curve and spline
			if(input.getPress(InputCurve)){
				bezierType = (bezierType == BezierCurve ? BezierSpline : BezierCurve);
				isDataOutdated = true;
				printf("Toggled between curve and spline\n");
			}
			
			// update curve
			if(isDataOutdated){
				recomputeCurve(bezierType, renderType, controlPoints, samplePoints, sampleVectors, vectorDirectionDraw, lineDraw);
				linePositionBuffer.update(samplePoints.data(), sizeof(float) * samplePoints.size(), 0);
				lineDirectionBuffer.update(sampleVectors.data(), sizeof(float) * samplePoints.size(), 0);
				displayCurve(currentRenderers[renderType], window);
				isDataOutdated = false;
			}
		}
	}
	
	return 0;
}