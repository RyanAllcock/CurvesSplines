#include "shader.hpp"

// buffer

Buffer::Buffer(BufferFrequency frequency, GLvoid const *data, GLuint size){
	glGenBuffers(1, &id);
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferData(GL_ARRAY_BUFFER, size, data, frequency);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Buffer::~Buffer(){
	glDeleteBuffers(1, &id);
}

void Buffer::update(GLvoid const *data, GLsizeiptr size, GLintptr offset) const {
	glBindBuffer(GL_ARRAY_BUFFER, id);
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// index

Index::Index(Buffer const &b,  GLint e, IndexType t, IndexNormal n, GLsizei s, GLvoid *o) : 
	buffer{b.id}, size{e}, type{t}, normal{n}, stride{s}, offset{o} {}

Index::Index(Buffer const &b, IndexType t, GLsizei s, GLvoid *o) : 
	buffer{b.id}, type{t}, stride{s}, offset{o}, size{1}, normal{IndexUnchanged} {}

void Index::bind(GLenum target) const {
	glBindBuffer(target, buffer);
}

void Index::attribute(GLenum target, GLuint index) const {
	bind(target);
	glVertexAttribPointer(index, size, type, normal, stride, offset);
	glEnableVertexAttribArray(index);
}

// shader

Shader::Shader() : id{GL_INVALID_ENUM}, compileStatus{GL_FALSE} {}

Shader::Shader(ShaderType t, std::vector<const char*> src){
	id = glCreateShader(t);
	glShaderSource(id, src.size(), src.data(), 0);
	glCompileShader(id);
	glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus);
}

Shader::~Shader(){
	if(id != GL_INVALID_ENUM) glDeleteShader(id);
}

std::string Shader::getErrorStatus(){
	GLchar infoLog[1024];
	if(!compileStatus){
		glGetShaderInfoLog(id, sizeof(infoLog), NULL, infoLog);
		return "Error: shader compile error" + std::string(infoLog);
	}
	return "No error";
}

Shader& Shader::operator=(Shader&& s){
	id = std::move(s.id);
	s.id = GL_INVALID_ENUM;
	return *this;
}

// data

DataFloat::DataFloat(float d) : data{d} {}

void DataFloat::pass(GLint l) const {
	glUniform1fv(l, 1, (GLfloat*)&data);
}

DataFloat2::DataFloat2(float x1, float x2) : data{x1, x2} {}

void DataFloat2::pass(GLint l) const {
	glUniform2fv(l, 1, (GLfloat*)&data);
}

DataFloat3::DataFloat3(float x1, float x2, float x3) : data{x1, x2, x3} {}

void DataFloat3::pass(GLint l) const {
	glUniform3fv(l, 1, (GLfloat*)&data);
}

DataMatrix4::DataMatrix4(std::array<float, 16> const &x, DataTranspose t) : transpose{t} {
	for(int i = 0; i < 16; i++) data[i] = x[i];
}

void DataMatrix4::pass(GLint l) const {
	glUniformMatrix4fv(l, 1, GL_FALSE, (GLfloat*)&data);
}

// program

Program::Program(std::vector<Shader*> const &s) : linkStatus{GL_FALSE} {
	if((id = glCreateProgram()) == 0) return;
	for(Shader const *shader : s) glAttachShader(id, shader->id);
	glLinkProgram(id);
	glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);
}

Program::~Program(){
	glDeleteProgram(id);
}

std::string Program::getErrorStatus(){
	GLchar infoLog[1024];
	if(id == 0) return "Error: program not created";
	else if(!linkStatus){
		glGetProgramInfoLog(id, sizeof(infoLog), NULL, infoLog);
		return "Error: program not linked: " + std::string(infoLog);
	}
	else if(!uniformStatus.empty()) return "Error: uniform tag not accepted: " + uniformStatus;
	return "No error";
}

void Program::setUniform(const GLchar *tag, Data const &&d){
	GLint location = glGetUniformLocation(id, tag);
	if(location == -1){
		uniformStatus = std::string(tag);
		return;
	}
	glUseProgram(id);
	d.pass(location);
	glUseProgram(0);
}

// draw

DrawArray::DrawArray(DrawMode m, std::vector<Index*> const &ivs, GLsizei n) : mode{m}, count{n} {
	glGenVertexArrays(1, &id);
	glBindVertexArray(id);
	for(int i = 0; i < ivs.size(); i++) ivs[i]->attribute(GL_ARRAY_BUFFER, i);
	glBindVertexArray(0);
}

DrawArray::~DrawArray(){
	glDeleteVertexArrays(1, &id);
}

void DrawArray::recount(GLsizei n){
	count = n;
}

DrawElements::DrawElements(DrawMode m, std::vector<Index*> const &ivs, Index const &ie, GLsizei n) : DrawArray(m, ivs, n), type{ie.type} {
	glBindVertexArray(id);
	ie.bind(GL_ELEMENT_ARRAY_BUFFER);
	glBindVertexArray(0);
}

DrawInstanced::DrawInstanced(GLuint id, std::vector<Index*> const &ivs, std::vector<Index*> const &iis, GLsizei n) : instanceCount{n} {
	glBindVertexArray(id);
	for(int i = 0; i < iis.size(); i++){
		iis[i]->attribute(GL_ARRAY_BUFFER, ivs.size() + i);
		glVertexAttribDivisor(ivs.size() + i, 1);
	}
	glBindVertexArray(0);
}

void DrawInstanced::recountInstance(GLsizei n){
	instanceCount = n;
}

DrawInstancedArray::DrawInstancedArray(DrawMode m, std::vector<Index*> const &ivs, GLsizei in, std::vector<Index*> const &iis, GLsizei n) : 
	DrawArray(m, ivs, n), DrawInstanced(id, ivs, iis, in) {}

DrawInstancedElements::DrawInstancedElements(DrawMode m, std::vector<Index*> const &ivs, Index &ie, GLsizei in, std::vector<Index*> const &iis, GLsizei n) : 
	DrawElements(m, ivs, ie, n), DrawInstanced(id, ivs, iis, in) {}

void DrawArray::call() const {
	glDrawArrays(mode, 0, count);
}

void DrawElements::call() const {
	glDrawElements(mode, count, type, 0);
}

void DrawInstancedArray::call() const {
	glDrawArraysInstanced(mode, 0, instanceCount, count);
}

void DrawInstancedElements::call() const {
	glDrawElementsInstanced(mode, instanceCount, type, 0, count);
}

// renderer

Renderer::Renderer(Program const &p, DrawArray const &d) : program{p.id}, vao{d.id}, draw{d} {}

void Renderer::display() const {
	glUseProgram(program);
	glBindVertexArray(vao);
	draw.call();
	glBindVertexArray(0);
	glUseProgram(0);
}