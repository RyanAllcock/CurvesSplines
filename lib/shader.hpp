#ifndef HEADER_SHADER
#define HEADER_SHADER

#include "gl/glew.h" // OpenGL & GLSL

#include <vector> // argument handling
#include <array> // data storage & passing

// overview

struct Shader; // shader compilation
struct Program; // program compilation & shader linking
struct Index; // buffer indexing
struct Buffer; // buffer data
struct Data; // uniform data
struct DrawArray; // drawing operation & attribute binding
struct Renderer; // displaying

// data

enum ShaderType{
	ShaderVertex = GL_VERTEX_SHADER, 
	ShaderGeometry = GL_GEOMETRY_SHADER, 
	ShaderFragment = GL_FRAGMENT_SHADER
};

enum BufferFrequency{
	BufferStatic = GL_STATIC_DRAW, 
	BufferStream = GL_STREAM_DRAW, 
	BufferDynamic = GL_DYNAMIC_DRAW
};

enum IndexType{
	IndexFloat = GL_FLOAT, 
	IndexUint = GL_UNSIGNED_INT
};

enum IndexNormal{
	IndexUnchanged = GL_FALSE, 
	IndexNormalised = GL_TRUE
};

enum DataTranspose{
	DataUnchanged = GL_FALSE, 
	DataTransposed = GL_TRUE
};

enum DrawMode{
	DrawPoint = GL_POINTS, 
	DrawLine = GL_LINES, 
	DrawTriangle = GL_TRIANGLES
};

// classes

struct Buffer{
	GLuint id;
	Buffer(BufferFrequency frequency, GLvoid const *data, GLuint size);
	~Buffer();
	void update(GLvoid const *data, GLsizeiptr size, GLintptr offset) const;
};

struct Index{
	GLuint buffer;
	GLint size;
	GLenum type;
	GLboolean normal;
	GLsizei stride;
	GLvoid *offset;
	Index(Buffer const &b, GLint e, IndexType t, IndexNormal n, GLsizei s, GLvoid *o);
	Index(Buffer const &b, IndexType t, GLsizei s, GLvoid *o);
	void bind(GLenum target) const;
	void attribute(GLenum target, GLuint index) const;
};

struct Shader{
	GLuint id;
	GLint compileStatus;
	Shader();
	Shader(ShaderType t, std::vector<const char*> src);
	~Shader();
	std::string getErrorStatus();
	Shader& operator=(Shader&& s);
};

struct Data{
	virtual void pass(GLint l) const = 0;
};
struct DataFloat : Data{
	GLfloat data;
	DataFloat(float d);
	void pass(GLint l) const;
};
struct DataFloat2 : Data{
	GLfloat data[2];
	DataFloat2(float x1, float x2);
	void pass(GLint l) const;
};
struct DataFloat3 : Data{
	GLfloat data[3];
	DataFloat3(float x1, float x2, float x3);
	void pass(GLint l) const;
};
struct DataMatrix4 : Data{
	GLfloat data[16];
	GLboolean transpose;
	DataMatrix4(std::array<float, 16> const &x, DataTranspose t);
	void pass(GLint l) const;
};

struct Program{
	GLuint id;
	GLint linkStatus;
	std::string uniformStatus;
	Program(std::vector<Shader*> const &s);
	~Program();
	std::string getErrorStatus();
	void setUniform(const GLchar *tag, Data const &&d);
};

struct DrawArray{
	GLuint id;
	GLenum mode;
	GLsizei count;
	DrawArray(DrawMode m, std::vector<Index*> const &ivs, GLsizei n);
	~DrawArray();
	virtual void call() const;
	void recount(GLsizei n);
};
struct DrawElements : DrawArray{
	GLenum type;
	DrawElements(DrawMode m, std::vector<Index*> const &ivs, Index const &ie, GLsizei n);
	void call() const;
};
struct DrawInstanced{
	GLsizei instanceCount;
	DrawInstanced(GLuint id, std::vector<Index*> const &ivs, std::vector<Index*> const &iis, GLsizei i);
	void recountInstance(GLsizei n);
};
struct DrawInstancedArray : DrawArray, DrawInstanced{
	DrawInstancedArray(DrawMode m, std::vector<Index*> const &ivs, GLsizei in, std::vector<Index*> const &iis, GLsizei n);
	void call() const;
};
struct DrawInstancedElements : DrawElements, DrawInstanced{
	DrawInstancedElements(DrawMode m, std::vector<Index*> const &ivs, Index &ie, GLsizei in, std::vector<Index*> const &iis, GLsizei n);
	void call() const;
};

struct Renderer{
	GLuint program, vao;
	DrawArray const &draw;
	Renderer(Program const &p, DrawArray const &d);
	void display() const;
};

#endif