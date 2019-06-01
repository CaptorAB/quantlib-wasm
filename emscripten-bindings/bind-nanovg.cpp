#include "nanovg.h"

#include <emscripten/bind.h>

#define FUNCTION(RET, ARGS, CODE...) \
  emscripten::optional_override([] ARGS -> RET { CODE })

#include <malloc.h>

emscripten::val get_mallinfo() {
  const auto& i = mallinfo();
  emscripten::val rv(emscripten::val::object());
  rv.set("arena", emscripten::val(i.arena));
  rv.set("ordblks", emscripten::val(i.ordblks));
  rv.set("smblks", emscripten::val(i.smblks));
  rv.set("hblks", emscripten::val(i.hblks));
  rv.set("hblkhd", emscripten::val(i.hblkhd));
  rv.set("usmblks", emscripten::val(i.usmblks));
  rv.set("fsmblks", emscripten::val(i.fsmblks));
  rv.set("uordblks", emscripten::val(i.uordblks));
  rv.set("fordblks", emscripten::val(i.fordblks));
  rv.set("keepcost", emscripten::val(i.keepcost));
  return rv;
}

EMSCRIPTEN_BINDINGS(mallinfo) {
  emscripten::function("mallinfo", &get_mallinfo);
}

#include <GLES2/gl2.h>
// #include <EGL/egl.h>
#define NANOVG_GLES2_IMPLEMENTATION
#include "nanovg_gl.h"
#include "nanovg_gl_utils.h"

template <typename T, T NONE>
class ValueCache {
public:
  emscripten::val array = emscripten::val::array();
  ValueCache() {
    if (0 <= NONE) {
      array.set(NONE, emscripten::val::null());
    }
  }
  T hold(emscripten::val value) {
    if (value.isNull()) { return NONE; }
    T index = array["length"].template as<T>();
    array.set(index, value);
    return index;
  }
  emscripten::val drop(T index) {
    if (index == NONE) { return emscripten::val::null(); }
    emscripten::val value = array[index];
    array.delete_(index);
    return value;
  }
  emscripten::val find(T index) {
    if (index == NONE) { return emscripten::val::null(); }
    return array[index];
  }
  T findIndex(emscripten::val value) {
    if (value.isNull()) { return NONE; }
    for (T index = 0, length = array["length"].template as<T>(); index < length; ++index) {
      if (value.strictlyEquals(array[index])) {
        return index;
      }
    }
    return NONE;
  }
};

class WrapWebGL {
public:
  emscripten::val _gl;
  ValueCache<GLuint, 0> _buffers;
  ValueCache<GLuint, 0> _textures;
  ValueCache<GLuint, 0> _shaders;
  ValueCache<GLuint, 0> _programs;
  ValueCache<GLint, -1> _uniforms;
  ValueCache<GLuint, 0> _framebuffers;
  ValueCache<GLuint, 0> _renderbuffers;
  WrapWebGL(emscripten::val gl): _gl(gl) {}
};

WrapWebGL* wrap_gl = NULL;

static size_t _gl_format_count(GLenum format) {
  switch (format) {
    case GL_RGB: // A ignored
    case GL_RGBA:
    case GL_ALPHA: // RGB ignored
      return 4;
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
    case GL_DEPTH_COMPONENT:
      return 1;
  }
  return 0;
}
static size_t _gl_type_size(GLenum type) {
  switch (type) {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
      return 1;
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    #if defined(GL_HALF_FLOAT)
    case GL_HALF_FLOAT:
    #endif
      return 2;
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FIXED:
    case GL_FLOAT:
      return 4;
    #if defined(GL_DOUBLE)
    case GL_DOUBLE:
      return 8;
    #endif
  }
  return 0;
}
static size_t _gl_format_type_size(GLenum format, GLenum type) {
  return _gl_format_count(format) * _gl_type_size(type);
}

GL_APICALL void GL_APIENTRY glActiveTexture (GLenum texture) {
  wrap_gl->_gl.call<void>("activeTexture", texture);
}
GL_APICALL void GL_APIENTRY glAttachShader (GLuint program, GLuint shader) {
  wrap_gl->_gl.call<void>("attachShader", wrap_gl->_programs.find(program), wrap_gl->_shaders.find(shader));
}
GL_APICALL void GL_APIENTRY glBindAttribLocation (GLuint program, GLuint index, const GLchar *name) {
  wrap_gl->_gl.call<void>("bindAttribLocation", wrap_gl->_programs.find(program), index, emscripten::val(name));
}
GL_APICALL void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer) {
  wrap_gl->_gl.call<void>("bindBuffer", target, wrap_gl->_buffers.find(buffer));
}
GL_APICALL void GL_APIENTRY glBindFramebuffer (GLenum target, GLuint framebuffer) {
  wrap_gl->_gl.call<void>("bindFramebuffer", target, wrap_gl->_framebuffers.find(framebuffer));
}
GL_APICALL void GL_APIENTRY glBindRenderbuffer (GLenum target, GLuint renderbuffer) {
  wrap_gl->_gl.call<void>("bindRenderbuffer", target, wrap_gl->_renderbuffers.find(renderbuffer));
}
GL_APICALL void GL_APIENTRY glBindTexture (GLenum target, GLuint texture) {
  wrap_gl->_gl.call<void>("bindTexture", target, wrap_gl->_textures.find(texture));
}
GL_APICALL void GL_APIENTRY glBlendColor (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
  wrap_gl->_gl.call<void>("blendColor", red, green, blue, alpha);
}
GL_APICALL void GL_APIENTRY glBlendEquation (GLenum mode) {
  wrap_gl->_gl.call<void>("blendEquation", mode);
}
GL_APICALL void GL_APIENTRY glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha) {
  wrap_gl->_gl.call<void>("blendEquationSeparate", modeRGB, modeAlpha);
}
GL_APICALL void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor) {
  wrap_gl->_gl.call<void>("blendFunc", sfactor, dfactor);
}
GL_APICALL void GL_APIENTRY glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
  wrap_gl->_gl.call<void>("blendFuncSeparate", srcRGB, dstRGB, srcAlpha, dstAlpha);
}
GL_APICALL void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage) {
  wrap_gl->_gl.call<void>("bufferData", target, emscripten::typed_memory_view<char>(size, (char*) data), usage);
}
GL_APICALL void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
  wrap_gl->_gl.call<void>("bufferSubData", target, offset, emscripten::typed_memory_view<char>(size, (char*) data));
}
GL_APICALL GLenum GL_APIENTRY glCheckFramebufferStatus (GLenum target) {
  return wrap_gl->_gl.call<GLenum>("checkFramebufferStatus", target);
}
GL_APICALL void GL_APIENTRY glClear (GLbitfield mask) {
  wrap_gl->_gl.call<void>("clear", mask);
}
GL_APICALL void GL_APIENTRY glClearColor (GLclampf r, GLclampf g, GLclampf b, GLclampf a) {
  wrap_gl->_gl.call<void>("clearColor", r, g, b, a);
}
GL_APICALL void GL_APIENTRY glClearDepthf (GLfloat d) {
  wrap_gl->_gl.call<void>("clearDepth", d);
}
GL_APICALL void GL_APIENTRY glClearStencil (GLint s) {
  wrap_gl->_gl.call<void>("clearStencil", s);
}
GL_APICALL void GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
  wrap_gl->_gl.call<void>("colorMask", red, green, blue, alpha);
}
GL_APICALL void GL_APIENTRY glCompileShader (GLuint shader) {
  wrap_gl->_gl.call<void>("compileShader", wrap_gl->_shaders.find(shader));
}
// GL_APICALL void GL_APIENTRY glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
// GL_APICALL void GL_APIENTRY glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
// GL_APICALL void GL_APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
// GL_APICALL void GL_APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GL_APICALL GLuint GL_APIENTRY glCreateProgram () {
  return wrap_gl->_programs.hold(wrap_gl->_gl.call<emscripten::val>("createProgram"));
}
GL_APICALL GLuint GL_APIENTRY glCreateShader (GLenum shaderType) {
  return wrap_gl->_shaders.hold(wrap_gl->_gl.call<emscripten::val>("createShader", shaderType));
}
GL_APICALL void GL_APIENTRY glCullFace (GLenum mode) {
  wrap_gl->_gl.call<void>("cullFace", mode);
}
GL_APICALL void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint * buffers) {
  for (GLsizei i = 0; i < n; ++i) {
    wrap_gl->_gl.call<void>("deleteBuffer", wrap_gl->_buffers.drop(buffers[i]));
  }
}
GL_APICALL void GL_APIENTRY glDeleteFramebuffers (GLsizei n, const GLuint *framebuffers) {
  for (GLsizei i = 0; i < n; ++i) {
    wrap_gl->_gl.call<void>("deleteFramebuffer", wrap_gl->_framebuffers.drop(framebuffers[i]));
  }
}
GL_APICALL void GL_APIENTRY glDeleteProgram (GLuint program) {
  wrap_gl->_gl.call<void>("deleteProgram", wrap_gl->_programs.drop(program));
}
GL_APICALL void GL_APIENTRY glDeleteRenderbuffers (GLsizei n, const GLuint *renderbuffers) {
  for (GLsizei i = 0; i < n; ++i) {
    wrap_gl->_gl.call<void>("deleteRenderbuffer", wrap_gl->_renderbuffers.drop(renderbuffers[i]));
  }
}
GL_APICALL void GL_APIENTRY glDeleteShader (GLuint shader) {
  wrap_gl->_gl.call<void>("deleteShader", wrap_gl->_shaders.drop(shader));
}
GL_APICALL void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint * textures) {
  for (GLsizei i = 0; i < n; ++i) {
    wrap_gl->_gl.call<void>("deleteTexture", wrap_gl->_textures.drop(textures[i]));
  }
}
GL_APICALL void GL_APIENTRY glDepthFunc (GLenum func) {
  wrap_gl->_gl.call<void>("depthFunc", func);
}
GL_APICALL void GL_APIENTRY glDepthMask (GLboolean flag) {
  wrap_gl->_gl.call<void>("depthMask", flag);
}
GL_APICALL void GL_APIENTRY glDepthRangef (GLfloat n, GLfloat f) {
  wrap_gl->_gl.call<void>("depthRange", n, f);
}
GL_APICALL void GL_APIENTRY glDetachShader (GLuint program, GLuint shader) {
  wrap_gl->_gl.call<void>("detachShader", wrap_gl->_programs.find(program), wrap_gl->_shaders.find(shader));
}
GL_APICALL void GL_APIENTRY glDisable (GLenum cap) {
  wrap_gl->_gl.call<void>("disable", cap);
}
GL_APICALL void GL_APIENTRY glDisableVertexAttribArray (GLuint index) {
  wrap_gl->_gl.call<void>("disableVertexAttribArray", index);
}
GL_APICALL void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count) {
  wrap_gl->_gl.call<void>("drawArrays", mode, first, count);
}
GL_APICALL void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const void *indices) {
  wrap_gl->_gl.call<void>("drawElements", mode, count, type, (GLintptr) indices);
}
GL_APICALL void GL_APIENTRY glEnable (GLenum cap) {
  wrap_gl->_gl.call<void>("enable", cap);
}
GL_APICALL void GL_APIENTRY glEnableVertexAttribArray (GLuint index) {
  wrap_gl->_gl.call<void>("enableVertexAttribArray", index);
}
GL_APICALL void GL_APIENTRY glFinish () {
  wrap_gl->_gl.call<void>("finish");
}
GL_APICALL void GL_APIENTRY glFlush (void) {
  wrap_gl->_gl.call<void>("flush");
}
GL_APICALL void GL_APIENTRY glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
  wrap_gl->_gl.call<void>("framebufferRenderbuffer", target, attachment, renderbuffertarget, wrap_gl->_renderbuffers.find(renderbuffer));
}
GL_APICALL void GL_APIENTRY glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
  wrap_gl->_gl.call<void>("framebufferTexture2D", target, attachment, textarget, wrap_gl->_textures.find(texture), level);
}
GL_APICALL void GL_APIENTRY glFrontFace (GLenum mode) {
  wrap_gl->_gl.call<void>("frontFace", mode);
}
GL_APICALL void GL_APIENTRY glGenBuffers (GLsizei n, GLuint * buffers) {
  for (GLsizei i = 0; i < n; ++i) {
    buffers[i] = wrap_gl->_buffers.hold(wrap_gl->_gl.call<emscripten::val>("createBuffer"));
  }
}
GL_APICALL void GL_APIENTRY glGenerateMipmap (GLenum target) {
  wrap_gl->_gl.call<void>("generateMipmap");
}
GL_APICALL void GL_APIENTRY glGenFramebuffers (GLsizei n, GLuint* textures) {
  for (GLsizei i = 0; i < n; ++i) {
    textures[i] = wrap_gl->_framebuffers.hold(wrap_gl->_gl.call<emscripten::val>("createFramebuffer"));
  }
}
GL_APICALL void GL_APIENTRY glGenRenderbuffers (GLsizei n, GLuint* textures) {
  for (GLsizei i = 0; i < n; ++i) {
    textures[i] = wrap_gl->_renderbuffers.hold(wrap_gl->_gl.call<emscripten::val>("createRenderbuffer"));
  }
}
GL_APICALL void GL_APIENTRY glGenTextures (GLsizei n, GLuint* textures) {
  for (GLsizei i = 0; i < n; ++i) {
    textures[i] = wrap_gl->_textures.hold(wrap_gl->_gl.call<emscripten::val>("createTexture"));
  }
}
// GL_APICALL void GL_APIENTRY glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
// GL_APICALL void GL_APIENTRY glGetActiveUniform (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
// GL_APICALL void GL_APIENTRY glGetAttachedShaders (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
GL_APICALL GLint GL_APIENTRY glGetAttribLocation (GLuint program, const GLchar *name) {
  return wrap_gl->_gl.call<GLint>("getAttribLocation", wrap_gl->_programs.find(program), emscripten::val(name));  
}
// GL_APICALL void GL_APIENTRY glGetBooleanv (GLenum pname, GLboolean *data);
// GL_APICALL void GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint *params);
GL_APICALL GLenum GL_APIENTRY glGetError () {
  return wrap_gl->_gl.call<GLenum>("getError");
}
// GL_APICALL void GL_APIENTRY glGetFloatv (GLenum pname, GLfloat *data);
// GL_APICALL void GL_APIENTRY glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetIntegerv (GLenum pname, GLint *data) {
  emscripten::val value = wrap_gl->_gl.call<emscripten::val>("getParameter", pname);
  switch (pname) {
    case GL_FRAMEBUFFER_BINDING:
      *data = value.isNull() ? 0 : value.as<GLint>();
      break;
    case GL_RENDERBUFFER_BINDING:
      *data = value.isNull() ? 0 : value.as<GLint>();
      break;
  }
}
GL_APICALL void GL_APIENTRY glGetProgramiv (GLuint program, GLenum pname, GLint *params) {
  emscripten::val value = wrap_gl->_gl.call<emscripten::val>("getProgramParameter", wrap_gl->_programs.find(program), pname);
  switch (pname) {
    // case GL_DELETE_STATUS: // params returns GL_TRUE if program is currently flagged for deletion, and GL_FALSE otherwise.
    case GL_LINK_STATUS: // params returns GL_TRUE if the last link operation on program was successful, and GL_FALSE otherwise.
      *params = value.as<GLboolean>() ? GL_TRUE : GL_FALSE;
      break;
    // case GL_VALIDATE_STATUS: // params returns GL_TRUE or if the last validation operation on program was successful, and GL_FALSE otherwise.
    // case GL_INFO_LOG_LENGTH: // params returns the number of characters in the information log for program including the null termination character (i.e., the size of the character buffer required to store the information log). If program has no information log, a value of 0 is returned.
    // case GL_ATTACHED_SHADERS: // params returns the number of shader objects attached to program.
    // case GL_ACTIVE_ATTRIBUTES: // params returns the number of active attribute variables for program.
    // case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH: // params returns the length of the longest active attribute name for program, including the null termination character (i.e., the size of the character buffer required to store the longest attribute name). If no active attributes exist, 0 is returned.
    // case GL_ACTIVE_UNIFORMS: // params returns the number of active uniform variables for program.
    // case GL_ACTIVE_UNIFORM_MAX_LENGTH: // params returns the length of the longest active uniform variable name for program, including the null termination character (i.e., the size of the character buffer required to store the longest uniform variable name). If no active uniform variables exist, 0 is returned.
  }
}
GL_APICALL void GL_APIENTRY glGetProgramInfoLog (GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog) {
  std::string log = wrap_gl->_gl.call<emscripten::val>("getProgramInfoLog", wrap_gl->_programs.find(program)).as<std::string>();
  *length = log.length();
  strncpy(infoLog, log.c_str(), maxLength);
}
// GL_APICALL void GL_APIENTRY glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint *params);
GL_APICALL void GL_APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint *params) {
  emscripten::val value = wrap_gl->_gl.call<emscripten::val>("getShaderParameter", wrap_gl->_shaders.find(shader), pname);
  switch (pname) {
    // case GL_SHADER_TYPE: // params returns GL_VERTEX_SHADER if shader is a vertex shader object, and GL_FRAGMENT_SHADER if shader is a fragment shader object.
    // case GL_DELETE_STATUS: // params returns GL_TRUE if shader is currently flagged for deletion, and GL_FALSE otherwise.
    case GL_COMPILE_STATUS: // For implementations that support a shader compiler, params returns GL_TRUE if the last compile operation on shader was successful, and GL_FALSE otherwise.
      *params = value.as<GLboolean>() ? GL_TRUE : GL_FALSE;
      break;
    // case GL_INFO_LOG_LENGTH: // For implementations that support a shader compiler, params returns the number of characters in the information log for shader including the null termination character (i.e., the size of the character buffer required to store the information log). If shader has no information log, a value of 0 is returned.
    // case GL_SHADER_SOURCE_LENGTH: // For implementations that support a shader compiler, params returns the length of the concatenation of the source strings that make up the shader source for the shader, including the null termination character. (i.e., the size of the character buffer required to store the shader source). If no source code exists, 0 is returned.
  }
}
GL_APICALL void GL_APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) {
  std::string log = wrap_gl->_gl.call<emscripten::val>("getShaderInfoLog", wrap_gl->_shaders.find(shader)).as<std::string>();
  *length = log.length();
  strncpy(infoLog, log.c_str(), maxLength);
}
// GL_APICALL void GL_APIENTRY glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
// GL_APICALL void GL_APIENTRY glGetShaderSource (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
// GL_APICALL const GLubyte *GL_APIENTRY glGetString (GLenum name);
// GL_APICALL void GL_APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params);
// GL_APICALL void GL_APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params);
// GL_APICALL void GL_APIENTRY glGetUniformfv (GLuint program, GLint location, GLfloat *params);
// GL_APICALL void GL_APIENTRY glGetUniformiv (GLuint program, GLint location, GLint *params);
GL_APICALL GLint GL_APIENTRY glGetUniformLocation (GLuint program, const GLchar *name) {
  // return wrap_gl->_uniforms.hold(wrap_gl->_gl.call<emscripten::val>("getUniformLocation", wrap_gl->_programs.find(program), emscripten::val(name)));
  emscripten::val value = wrap_gl->_gl.call<emscripten::val>("getUniformLocation", wrap_gl->_programs.find(program), emscripten::val(name));
  if (value.isNull()) { return -1; }
  return wrap_gl->_uniforms.hold(value);
}
// GL_APICALL void GL_APIENTRY glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat *params);
// GL_APICALL void GL_APIENTRY glGetVertexAttribiv (GLuint index, GLenum pname, GLint *params);
// GL_APICALL void GL_APIENTRY glGetVertexAttribPointerv (GLuint index, GLenum pname, void **pointer);
GL_APICALL void GL_APIENTRY glHint (GLenum target, GLenum mode) {
  wrap_gl->_gl.call<void>("hint", target, mode);
}
GL_APICALL GLboolean GL_APIENTRY glIsBuffer (GLuint buffer) {
  return wrap_gl->_gl.call<GLboolean>("isBuffer", wrap_gl->_buffers.find(buffer));
}
GL_APICALL GLboolean GL_APIENTRY glIsEnabled (GLenum cap) {
  return wrap_gl->_gl.call<GLboolean>("isEnabled", cap);
}
GL_APICALL GLboolean GL_APIENTRY glIsFramebuffer (GLuint framebuffer) {
  return wrap_gl->_gl.call<GLboolean>("isFramebuffer", wrap_gl->_framebuffers.find(framebuffer));
}
GL_APICALL GLboolean GL_APIENTRY glIsProgram (GLuint program) {
  return wrap_gl->_gl.call<GLboolean>("isProgram", wrap_gl->_programs.find(program));
}
GL_APICALL GLboolean GL_APIENTRY glIsRenderbuffer (GLuint renderbuffer) {
  return wrap_gl->_gl.call<GLboolean>("isRenderbuffer", wrap_gl->_renderbuffers.find(renderbuffer));
}
GL_APICALL GLboolean GL_APIENTRY glIsShader (GLuint shader) {
  return wrap_gl->_gl.call<GLboolean>("isShader", wrap_gl->_shaders.find(shader));
}
GL_APICALL GLboolean GL_APIENTRY glIsTexture (GLuint texture) {
  return wrap_gl->_gl.call<GLboolean>("isTexture", wrap_gl->_textures.find(texture));
}
GL_APICALL void GL_APIENTRY glLineWidth (GLfloat width) {
  wrap_gl->_gl.call<void>("lineWidth", width);
}
GL_APICALL void GL_APIENTRY glLinkProgram (GLuint program) {
  wrap_gl->_gl.call<void>("linkProgram", wrap_gl->_programs.find(program));
}
GL_APICALL void GL_APIENTRY glPixelStorei (GLenum pname, GLint param) {
  wrap_gl->_gl.call<void>("pixelStorei", pname, param);
}
GL_APICALL void GL_APIENTRY glPolygonOffset (GLfloat factor, GLfloat units) {
  wrap_gl->_gl.call<void>("polygonOffset", factor, units);
}
GL_APICALL void GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) {
  size_t _size = width * height * _gl_format_type_size(format, type);
  wrap_gl->_gl.call<void>("readPixels", x, y, width, height, format, type, emscripten::typed_memory_view<char>(_size, (char*) pixels));
}
GL_APICALL void GL_APIENTRY glReleaseShaderCompiler (void) {
  // not implemented
}
GL_APICALL void GL_APIENTRY glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
  wrap_gl->_gl.call<void>("renderbufferStorage", target, internalformat, width, height);
}
GL_APICALL void GL_APIENTRY glSampleCoverage (GLfloat value, GLboolean invert) {
  wrap_gl->_gl.call<void>("sampleCoverage", value, invert);
}
GL_APICALL void GL_APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height) {
  wrap_gl->_gl.call<void>("scissor", x, y, width, height);
}
GL_APICALL void GL_APIENTRY glShaderBinary (GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length) {
  // not implemented
}
GL_APICALL void GL_APIENTRY glShaderSource (GLuint shader, GLsizei count, const GLchar * const *string, const GLint *length) {
  std::string source;
  for (GLsizei index = 0; index < count; ++index) {
    source += string[index];
  }
  wrap_gl->_gl.call<void>("shaderSource", wrap_gl->_shaders.find(shader), source);
}
GL_APICALL void GL_APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask) {
  wrap_gl->_gl.call<void>("stencilFunc", func, ref, mask);
}
GL_APICALL void GL_APIENTRY glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask) {
  wrap_gl->_gl.call<void>("stencilFuncSeparate", face, func, ref, mask);
}
GL_APICALL void GL_APIENTRY glStencilMask (GLuint mask) {
  wrap_gl->_gl.call<void>("stencilMask", mask);
}
GL_APICALL void GL_APIENTRY glStencilMaskSeparate (GLenum face, GLuint mask) {
  wrap_gl->_gl.call<void>("stencilMaskSeparate", face, mask);
}
GL_APICALL void GL_APIENTRY glStencilOp (GLenum sfail, GLenum dpfail, GLenum dppass) {
  wrap_gl->_gl.call<void>("stencilOp", sfail, dpfail, dppass);
}
GL_APICALL void GL_APIENTRY glStencilOpSeparate (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
  wrap_gl->_gl.call<void>("stencilOpSeparate", face, sfail, dpfail, dppass);
}
GL_APICALL void GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * data) {
  size_t _size = width * height * _gl_format_type_size(format, type);
  emscripten::val _data = (data == NULL) ? emscripten::val::null() : emscripten::val(emscripten::typed_memory_view<unsigned char>(_size, (unsigned char*) data));
  wrap_gl->_gl.call<void>("texImage2D", target, level, internalformat, width, height, border, format, type, _data);
}
GL_APICALL void GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param) {
  wrap_gl->_gl.call<void>("texParameterf", target, pname, param);
}
// GL_APICALL void GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params);
GL_APICALL void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param) {
  wrap_gl->_gl.call<void>("texParameteri", target, pname, param);
}
// GL_APICALL void GL_APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params);
GL_APICALL void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * data) {
  size_t _size = width * height * _gl_format_type_size(format, type);
  emscripten::val _data = (data == NULL) ? emscripten::val::null() : emscripten::val(emscripten::typed_memory_view<unsigned char>(_size, (unsigned char*) data));
  wrap_gl->_gl.call<void>("texSubImage2D", target, level, xoffset, yoffset, width, height, format, type, _data);
}
GL_APICALL void GL_APIENTRY glUniform1f (GLint location, GLfloat v0) {
  wrap_gl->_gl.call<void>("uniform1f", wrap_gl->_uniforms.find(location), v0);
}
GL_APICALL void GL_APIENTRY glUniform1fv (GLint location, GLsizei count, const GLfloat *value) {
  wrap_gl->_gl.call<void>("uniform1fv", wrap_gl->_uniforms.find(location), emscripten::typed_memory_view<float>(count, value));
}
GL_APICALL void GL_APIENTRY glUniform1i (GLint location, GLint v0) {
  wrap_gl->_gl.call<void>("uniform1i", wrap_gl->_uniforms.find(location), v0);
}
GL_APICALL void GL_APIENTRY glUniform1iv (GLint location, GLsizei count, const GLint *value) {
  wrap_gl->_gl.call<void>("uniform1iv", wrap_gl->_uniforms.find(location), emscripten::typed_memory_view<int>(count, value));
}
GL_APICALL void GL_APIENTRY glUniform2f (GLint location, GLfloat v0, GLfloat v1) {
  wrap_gl->_gl.call<void>("uniform2f", wrap_gl->_uniforms.find(location), v0, v1);
}
GL_APICALL void GL_APIENTRY glUniform2fv (GLint location, GLsizei count, const GLfloat *value) {
  wrap_gl->_gl.call<void>("uniform2fv", wrap_gl->_uniforms.find(location), emscripten::typed_memory_view<float>(count*2, value));
}
GL_APICALL void GL_APIENTRY glUniform2i (GLint location, GLint v0, GLint v1) {
  wrap_gl->_gl.call<void>("uniform2i", wrap_gl->_uniforms.find(location), v0, v1);
}
GL_APICALL void GL_APIENTRY glUniform2iv (GLint location, GLsizei count, const GLint *value) {
  wrap_gl->_gl.call<void>("uniform2iv", wrap_gl->_uniforms.find(location), emscripten::typed_memory_view<int>(count*2, value));
}
GL_APICALL void GL_APIENTRY glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
  wrap_gl->_gl.call<void>("uniform3f", wrap_gl->_uniforms.find(location), v0, v1, v2);
}
GL_APICALL void GL_APIENTRY glUniform3fv (GLint location, GLsizei count, const GLfloat *value) {
  wrap_gl->_gl.call<void>("uniform3fv", wrap_gl->_uniforms.find(location), emscripten::typed_memory_view<float>(count*3, value));
}
GL_APICALL void GL_APIENTRY glUniform3i (GLint location, GLint v0, GLint v1, GLint v2) {
  wrap_gl->_gl.call<void>("uniform3i", wrap_gl->_uniforms.find(location), v0, v1, v2);
}
GL_APICALL void GL_APIENTRY glUniform3iv (GLint location, GLsizei count, const GLint *value) {
  wrap_gl->_gl.call<void>("uniform3iv", wrap_gl->_uniforms.find(location), emscripten::typed_memory_view<int>(count*3, value));
}
GL_APICALL void GL_APIENTRY glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
  wrap_gl->_gl.call<void>("uniform4f", wrap_gl->_uniforms.find(location), v0, v1, v2, v3);
}
GL_APICALL void GL_APIENTRY glUniform4fv (GLint location, GLsizei count, const GLfloat *value) {
  wrap_gl->_gl.call<void>("uniform4fv", wrap_gl->_uniforms.find(location), emscripten::typed_memory_view<float>(count*4, value));
}
GL_APICALL void GL_APIENTRY glUniform4i (GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
  wrap_gl->_gl.call<void>("uniform4i", wrap_gl->_uniforms.find(location), v0, v1, v2, v3);
}
GL_APICALL void GL_APIENTRY glUniform4iv (GLint location, GLsizei count, const GLint *value) {
  wrap_gl->_gl.call<void>("uniform4iv", wrap_gl->_uniforms.find(location), emscripten::typed_memory_view<int>(count*4, value));
}
GL_APICALL void GL_APIENTRY glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  wrap_gl->_gl.call<void>("uniformMatrix2fv", wrap_gl->_uniforms.find(location), transpose, emscripten::typed_memory_view<float>(count, value));
}
GL_APICALL void GL_APIENTRY glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  wrap_gl->_gl.call<void>("uniformMatrix3fv", wrap_gl->_uniforms.find(location), transpose, emscripten::typed_memory_view<float>(count, value));
}
GL_APICALL void GL_APIENTRY glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  wrap_gl->_gl.call<void>("uniformMatrix4fv", wrap_gl->_uniforms.find(location), transpose, emscripten::typed_memory_view<float>(count, value));
}
GL_APICALL void GL_APIENTRY glUseProgram (GLuint program) {
  wrap_gl->_gl.call<void>("useProgram", wrap_gl->_programs.find(program));
}
GL_APICALL void GL_APIENTRY glValidateProgram (GLuint program) {
  wrap_gl->_gl.call<void>("validateProgram", wrap_gl->_programs.find(program));
}
GL_APICALL void GL_APIENTRY glVertexAttrib1f (GLuint index, GLfloat x) {
  wrap_gl->_gl.call<void>("vertexAttrib1f", index, x);
}
GL_APICALL void GL_APIENTRY glVertexAttrib1fv (GLuint index, const GLfloat *v) {
  wrap_gl->_gl.call<void>("vertexAttrib1fv", index, emscripten::typed_memory_view<float>(1, v));
}
GL_APICALL void GL_APIENTRY glVertexAttrib2f (GLuint index, GLfloat x, GLfloat y) {
  wrap_gl->_gl.call<void>("vertexAttrib2f", index, x, y);
}
GL_APICALL void GL_APIENTRY glVertexAttrib2fv (GLuint index, const GLfloat *v) {
  wrap_gl->_gl.call<void>("vertexAttrib2fv", index, emscripten::typed_memory_view<float>(2, v));
}
GL_APICALL void GL_APIENTRY glVertexAttrib3f (GLuint index, GLfloat x, GLfloat y, GLfloat z) {
  wrap_gl->_gl.call<void>("vertexAttrib3f", index, x, y, z);
}
GL_APICALL void GL_APIENTRY glVertexAttrib3fv (GLuint index, const GLfloat *v) {
  wrap_gl->_gl.call<void>("vertexAttrib3fv", index, emscripten::typed_memory_view<float>(3, v));
}
GL_APICALL void GL_APIENTRY glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
  wrap_gl->_gl.call<void>("vertexAttrib4f", index, x, y, z, w);
}
GL_APICALL void GL_APIENTRY glVertexAttrib4fv (GLuint index, const GLfloat *v) {
  wrap_gl->_gl.call<void>("vertexAttrib4fv", index, emscripten::typed_memory_view<float>(4, v));
}
GL_APICALL void GL_APIENTRY glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer) {
  wrap_gl->_gl.call<void>("vertexAttribPointer", index, size, type, normalized, stride, (GLintptr) pointer);
}
GL_APICALL void GL_APIENTRY glViewport (GLint x, GLint y, GLuint w, GLuint h) {
  wrap_gl->_gl.call<void>("viewport", x, y, w, h);
}

class WrapNVGcontext {
public:
  NVGcontext* ctx;

  WrapNVGcontext(emscripten::val gl, int flags) {
    wrap_gl = new WrapWebGL(gl);
    ctx = nvgCreateGLES2(flags);
  }

  ~WrapNVGcontext() {
    nvgDeleteGLES2(ctx); ctx = NULL;
    delete wrap_gl; wrap_gl = NULL;
  }
};

EMSCRIPTEN_BINDINGS(WrapNVGcontext) {
    emscripten::class_<WrapNVGcontext>("WrapNVGcontext")
    ;
}

EMSCRIPTEN_BINDINGS(NanoVG_GL) {
  emscripten::function("nvgCreateWebGL", FUNCTION(WrapNVGcontext*, (emscripten::val gl, int flags), {
    return new WrapNVGcontext(gl, flags);
  }), emscripten::allow_raw_pointers());
  emscripten::function("nvgDeleteWebGL", FUNCTION(void, (WrapNVGcontext* wrap), {
    delete wrap;
  }), emscripten::allow_raw_pointers());
  // int nvglCreateImageFromHandleGLES2(NVGcontext* ctx, GLuint textureId, int w, int h, int flags);
  // nvglCreateImageFromHandleWebGL(ctx: NVGcontext, textureId: WebGLTexture | null, w: number, h: number, flags: number): number;
  emscripten::function("nvglCreateImageFromHandleWebGL", FUNCTION(int, (WrapNVGcontext* wrap, emscripten::val textureId, int w, int h, int flags), {
    GLuint index = wrap_gl->_textures.findIndex(textureId);
    return nvglCreateImageFromHandleGLES2(wrap->ctx, index, w, h, flags);
  }), emscripten::allow_raw_pointers());
  // GLuint nvglImageHandleGLES2(NVGcontext* ctx, int image);
  // nvglImageHandleWebGL(ctx: NVGcontext, image: number): WebGLTexture | null;
  emscripten::function("nvglImageHandleWebGL", FUNCTION(emscripten::val, (WrapNVGcontext* wrap, int image), {
    GLuint texture = nvglImageHandleGLES2(wrap->ctx, image);
    return wrap_gl->_textures.find(texture);
  }), emscripten::allow_raw_pointers());
}

class WrapNVGLUframebuffer {
public:
  NVGLUframebuffer* fb;

  WrapNVGLUframebuffer(NVGcontext* ctx, int w, int h, int imageFlags) {
    fb = nvgluCreateFramebuffer(ctx, w, h, imageFlags);
  }

  ~WrapNVGLUframebuffer() {
    nvgluDeleteFramebuffer(fb); fb = NULL;
  }
};

EMSCRIPTEN_BINDINGS(WrapNVGLUframebuffer) {
    emscripten::class_<WrapNVGLUframebuffer>("WrapNVGLUframebuffer")
      // NVGcontext* ctx;
      // GLuint fbo;
      .property("fbo", FUNCTION(emscripten::val, (const WrapNVGLUframebuffer& that), {
        return wrap_gl->_framebuffers.find(that.fb->fbo);
      }))
      // GLuint rbo;
      .property("rbo", FUNCTION(emscripten::val, (const WrapNVGLUframebuffer& that), {
        return wrap_gl->_renderbuffers.find(that.fb->rbo);
      }))
      // GLuint texture;
      .property("texture", FUNCTION(emscripten::val, (const WrapNVGLUframebuffer& that), {
        return wrap_gl->_textures.find(that.fb->texture);
      }))
      // int image;
      .property("image", FUNCTION(int, (const WrapNVGLUframebuffer& that), {
        return that.fb->image;
      }))
    ;
}

EMSCRIPTEN_BINDINGS(NanoVG_GLU) {
  // void nvgluBindFramebuffer(NVGLUframebuffer* fb);
  // nvgluBindFramebuffer(fb: NVGLUframebuffer): void;
  emscripten::function("nvgluBindFramebuffer", FUNCTION(void, (WrapNVGLUframebuffer* wrap), {
    nvgluBindFramebuffer(wrap == NULL ? NULL : wrap->fb);
  }), emscripten::allow_raw_pointers());
  // NVGLUframebuffer* nvgluCreateFramebuffer(NVGcontext* ctx, int w, int h, int imageFlags);
  // nvgluCreateFramebuffer(ctx: NVGcontext, w: number, h: number, imageFlags: NVGimageFlags): NVGLUframebuffer;
  emscripten::function("nvgluCreateFramebuffer", FUNCTION(WrapNVGLUframebuffer*, (WrapNVGcontext* wrap_ctx, int w, int h, int imageFlags), {
    return new WrapNVGLUframebuffer(wrap_ctx->ctx, w, h, imageFlags);
  }), emscripten::allow_raw_pointers());
  // void nvgluDeleteFramebuffer(NVGLUframebuffer* fb);
  // nvgluDeleteFramebuffer(fb: NVGLUframebuffer): void;
  emscripten::function("nvgluDeleteFramebuffer", FUNCTION(void, (WrapNVGLUframebuffer* wrap), {
    delete wrap;
  }), emscripten::allow_raw_pointers());
}


static NVGcolor import_NVGcolor(emscripten::val value) {
  NVGcolor color;
  color.r = value["r"].as<float>();
  color.g = value["g"].as<float>();
  color.b = value["b"].as<float>();
  color.a = value["a"].as<float>();
  return color;
}

static emscripten::val export_NVGcolor(const NVGcolor& color, emscripten::val value) {
  value.set("r", emscripten::val(color.r));
  value.set("g", emscripten::val(color.g));
  value.set("b", emscripten::val(color.b));
  value.set("a", emscripten::val(color.a));
  return value;
}

EMSCRIPTEN_BINDINGS(NVGcolor) {
    emscripten::class_<NVGcolor>("NVGcolor")
      .property("rgba", FUNCTION(emscripten::val, (const NVGcolor& that), {
        return emscripten::val(emscripten::typed_memory_view<float>(4, that.rgba));
      }))
      .property("r", &NVGcolor::r)
      .property("g", &NVGcolor::g)
      .property("b", &NVGcolor::b)
      .property("a", &NVGcolor::a)
    ;
}

static NVGpaint import_NVGpaint(emscripten::val value) {
  NVGpaint paint;
  emscripten::val xform = value["xform"];
  for (int i = 0; i < 6; ++i) {
    paint.xform[i] = xform[i].as<float>();
  }
  emscripten::val extent = value["extent"];
  for (int i = 0; i < 2; ++i) {
    paint.extent[i] = extent[i].as<float>();
  }
  paint.radius = value["radius"].as<int>();
  paint.feather = value["feather"].as<int>();
  paint.innerColor = import_NVGcolor(value["innerColor"]);
  paint.outerColor = import_NVGcolor(value["outerColor"]);
  paint.image = value["image"].as<int>();
  return paint;
}

static emscripten::val export_NVGpaint(const NVGpaint& paint, emscripten::val value) {
  emscripten::val xform = value["xform"];
  for (int i = 0; i < 6; ++i) {
    xform.set(i, emscripten::val(paint.xform[i]));
  }
  emscripten::val extent = value["extent"];
  for (int i = 0; i < 2; ++i) {
    extent.set(i, emscripten::val(paint.extent[i]));
  }
  value.set("radius", emscripten::val(paint.radius));
  value.set("feather", emscripten::val(paint.feather));
  export_NVGcolor(paint.innerColor, value["innerColor"]);
  export_NVGcolor(paint.outerColor, value["outerColor"]);
  value.set("image", emscripten::val(paint.image));
  return value;
}

EMSCRIPTEN_BINDINGS(NVGpaint) {
  emscripten::class_<NVGpaint>("NVGpaint")
    .property("xform", FUNCTION(emscripten::val, (const NVGpaint& that), {
      return emscripten::val(emscripten::typed_memory_view<float>(6, that.xform));
    }))
    .property("extent", FUNCTION(emscripten::val, (const NVGpaint& that), {
      return emscripten::val(emscripten::typed_memory_view<float>(2, that.extent));
    }))
    .property("radius", &NVGpaint::radius)
    .property("feather", &NVGpaint::feather)
    .property("innerColor", &NVGpaint::innerColor)
    .property("outerColor", &NVGpaint::outerColor)
    .property("image", &NVGpaint::image)
  ;
}

EMSCRIPTEN_BINDINGS(NVGcompositeOperationState) {
  emscripten::class_<NVGcompositeOperationState>("NVGcompositeOperationState")
    .property("srcRGB", &NVGcompositeOperationState::srcRGB)
    .property("dstRGB", &NVGcompositeOperationState::dstRGB)
    .property("srcAlpha", &NVGcompositeOperationState::srcAlpha)
    .property("dstAlpha", &NVGcompositeOperationState::dstAlpha)
  ;
}

EMSCRIPTEN_BINDINGS(NVGscissor) {
  emscripten::class_<NVGscissor>("NVGscissor")
    .property("xform", FUNCTION(emscripten::val, (const NVGscissor& that), {
      return emscripten::val(emscripten::typed_memory_view<float>(6, that.xform));
    }))
    .property("extent", FUNCTION(emscripten::val, (const NVGscissor& that), {
      return emscripten::val(emscripten::typed_memory_view<float>(2, that.extent));
    }))
  ;
}

EMSCRIPTEN_BINDINGS(NanoVG) {
  // Begin drawing a new frame
  // Calls to nanovg drawing API should be wrapped in nvgBeginFrame() & nvgEndFrame()
  // nvgBeginFrame() defines the size of the window to render to in relation currently
  // set viewport (i.e. glViewport on GL backends). Device pixel ration allows to
  // control the rendering on Hi-DPI devices.
  // For example, GLFW returns two dimension for an opened window: window size and
  // frame buffer size. In that case you would set windowWidth/Height to the window size
  // devicePixelRatio to: frameBufferWidth / windowWidth.
  // // void nvgBeginFrame(NVGcontext* ctx, float windowWidth, float windowHeight, float devicePixelRatio);
  emscripten::function("nvgBeginFrame", FUNCTION(void, (WrapNVGcontext* wrap, float windowWidth, float windowHeight, float devicePixelRatio), {
    nvgBeginFrame(wrap->ctx, windowWidth, windowHeight, devicePixelRatio);
  }), emscripten::allow_raw_pointers());

  // Cancels drawing the current frame.
  // void nvgCancelFrame(NVGcontext* ctx);
  emscripten::function("nvgCancelFrame", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgCancelFrame(wrap->ctx);
  }), emscripten::allow_raw_pointers());

  // Ends drawing flushing remaining render state.
  // void nvgEndFrame(NVGcontext* ctx);
  emscripten::function("nvgEndFrame", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgEndFrame(wrap->ctx);
  }), emscripten::allow_raw_pointers());

  //
  // Composite operation
  //
  // The composite operations in NanoVG are modeled after HTML Canvas API, and
  // the blend func is based on OpenGL (see corresponding manuals for more info).
  // The colors in the blending state have premultiplied alpha.

  // Sets the composite operation. The op parameter should be one of NVGcompositeOperation.
  // void nvgGlobalCompositeOperation(NVGcontext* ctx, int op);
  emscripten::function("nvgGlobalCompositeOperation", FUNCTION(void, (WrapNVGcontext* wrap, int op), {
    nvgGlobalCompositeOperation(wrap->ctx, op);
  }), emscripten::allow_raw_pointers());

  // Sets the composite operation with custom pixel arithmetic. The parameters should be one of NVGblendFactor.
  // void nvgGlobalCompositeBlendFunc(NVGcontext* ctx, int sfactor, int dfactor);
  emscripten::function("nvgGlobalCompositeBlendFunc", FUNCTION(void, (WrapNVGcontext* wrap, int sfactor, int dfactor), {
    nvgGlobalCompositeBlendFunc(wrap->ctx, sfactor, dfactor);
  }), emscripten::allow_raw_pointers());

  // Sets the composite operation with custom pixel arithmetic for RGB and alpha components separately. The parameters should be one of NVGblendFactor.
  // void nvgGlobalCompositeBlendFuncSeparate(NVGcontext* ctx, int srcRGB, int dstRGB, int srcAlpha, int dstAlpha);
  emscripten::function("nvgGlobalCompositeBlendFuncSeparate", FUNCTION(void, (WrapNVGcontext* wrap, int srcRGB, int dstRGB, int srcAlpha, int dstAlpha), {
    nvgGlobalCompositeBlendFuncSeparate(wrap->ctx, srcRGB, dstRGB, srcAlpha, dstAlpha);
  }), emscripten::allow_raw_pointers());

  //
  // Color utils
  //
  // Colors in NanoVG are stored as unsigned ints in ABGR format.

  // Returns a color value from red, green, blue values. Alpha will be set to 255 (1.0f).
  // NVGcolor nvgRGB(unsigned char r, unsigned char g, unsigned char b);

  // Returns a color value from red, green, blue values. Alpha will be set to 1.0f.
  // NVGcolor nvgRGBf(float r, float g, float b);


  // Returns a color value from red, green, blue and alpha values.
  // NVGcolor nvgRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);

  // Returns a color value from red, green, blue and alpha values.
  // NVGcolor nvgRGBAf(float r, float g, float b, float a);


  // Linearly interpolates from color c0 to c1, and returns resulting color value.
  // NVGcolor nvgLerpRGBA(NVGcolor c0, NVGcolor c1, float u);

  // Sets transparency of a color value.
  // NVGcolor nvgTransRGBA(NVGcolor c0, unsigned char a);

  // Sets transparency of a color value.
  // NVGcolor nvgTransRGBAf(NVGcolor c0, float a);

  // Returns color value specified by hue, saturation and lightness.
  // HSL values are all in range [0..1], alpha will be set to 255.
  // NVGcolor nvgHSL(float h, float s, float l);

  // Returns color value specified by hue, saturation and lightness and alpha.
  // HSL values are all in range [0..1], alpha in range [0..255]
  // NVGcolor nvgHSLA(float h, float s, float l, unsigned char a);

  //
  // State Handling
  //
  // NanoVG contains state which represents how paths will be rendered.
  // The state contains transform, fill and stroke styles, text and font styles,
  // and scissor clipping.

  // Pushes and saves the current render state into a state stack.
  // A matching nvgRestore() must be used to restore the state.
  // void nvgSave(NVGcontext* ctx);
  emscripten::function("nvgSave", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgSave(wrap->ctx);
  }), emscripten::allow_raw_pointers());

  // Pops and restores current render state.
  // void nvgRestore(NVGcontext* ctx);
  emscripten::function("nvgRestore", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgRestore(wrap->ctx);
  }), emscripten::allow_raw_pointers());

  // Resets current render state to default values. Does not affect the render state stack.
  // void nvgReset(NVGcontext* ctx);
  emscripten::function("nvgReset", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgReset(wrap->ctx);
  }), emscripten::allow_raw_pointers());

  //
  // Render styles
  //
  // Fill and stroke render style can be either a solid color or a paint which is a gradient or a pattern.
  // Solid color is simply defined as a color value, different kinds of paints can be created
  // using nvgLinearGradient(), nvgBoxGradient(), nvgRadialGradient() and nvgImagePattern().
  //
  // Current render style can be saved and restored using nvgSave() and nvgRestore().

  // Sets whether to draw antialias for nvgStroke() and nvgFill(). It's enabled by default.
  // void nvgShapeAntiAlias(NVGcontext* ctx, int enabled);
  emscripten::function("nvgShapeAntiAlias", FUNCTION(void, (WrapNVGcontext* wrap, int enabled), {
    nvgShapeAntiAlias(wrap->ctx, enabled);
  }), emscripten::allow_raw_pointers());

  // Sets current stroke style to a solid color.
  // void nvgStrokeColor(NVGcontext* ctx, NVGcolor color);
  emscripten::function("nvgStrokeColor", FUNCTION(void, (WrapNVGcontext* wrap, emscripten::val color), {
    nvgStrokeColor(wrap->ctx, import_NVGcolor(color));
  }), emscripten::allow_raw_pointers());

  // Sets current stroke style to a paint, which can be a one of the gradients or a pattern.
  // void nvgStrokePaint(NVGcontext* ctx, NVGpaint paint);
  emscripten::function("nvgStrokePaint", FUNCTION(void, (WrapNVGcontext* wrap, emscripten::val paint), {
    nvgStrokePaint(wrap->ctx, import_NVGpaint(paint));
  }), emscripten::allow_raw_pointers());

  // Sets current fill style to a solid color.
  // void nvgFillColor(NVGcontext* ctx, NVGcolor color);
  emscripten::function("nvgFillColor", FUNCTION(void, (WrapNVGcontext* wrap, emscripten::val color), {
    nvgFillColor(wrap->ctx, import_NVGcolor(color));
  }), emscripten::allow_raw_pointers());

  // Sets current fill style to a paint, which can be a one of the gradients or a pattern.
  // void nvgFillPaint(NVGcontext* ctx, NVGpaint paint);
  emscripten::function("nvgFillPaint", FUNCTION(void, (WrapNVGcontext* wrap, emscripten::val paint), {
    nvgFillPaint(wrap->ctx, import_NVGpaint(paint));
  }), emscripten::allow_raw_pointers());

  // Sets the miter limit of the stroke style.
  // Miter limit controls when a sharp corner is beveled.
  // void nvgMiterLimit(NVGcontext* ctx, float limit);
  emscripten::function("nvgMiterLimit", FUNCTION(void, (WrapNVGcontext* wrap, float limit), {
    nvgMiterLimit(wrap->ctx, limit);
  }), emscripten::allow_raw_pointers());

  // Sets the stroke width of the stroke style.
  // void nvgStrokeWidth(NVGcontext* ctx, float size);
  emscripten::function("nvgStrokeWidth", FUNCTION(void, (WrapNVGcontext* wrap, float size), {
    nvgStrokeWidth(wrap->ctx, size);
  }), emscripten::allow_raw_pointers());

  // Sets how the end of the line (cap) is drawn,
  // Can be one of: NVG_BUTT (default), NVG_ROUND, NVG_SQUARE.
  // void nvgLineCap(NVGcontext* ctx, int cap);
  emscripten::function("nvgLineCap", FUNCTION(void, (WrapNVGcontext* wrap, int cap), {
    nvgLineCap(wrap->ctx, cap);
  }), emscripten::allow_raw_pointers());

  // Sets how sharp path corners are drawn.
  // Can be one of NVG_MITER (default), NVG_ROUND, NVG_BEVEL.
  // void nvgLineJoin(NVGcontext* ctx, int join);
  emscripten::function("nvgLineJoin", FUNCTION(void, (WrapNVGcontext* wrap, int join), {
    nvgLineJoin(wrap->ctx, join);
  }), emscripten::allow_raw_pointers());

  // Sets the transparency applied to all rendered shapes.
  // Already transparent paths will get proportionally more transparent as well.
  // void nvgGlobalAlpha(NVGcontext* ctx, float alpha);
  emscripten::function("nvgGlobalAlpha", FUNCTION(void, (WrapNVGcontext* wrap, float alpha), {
    nvgGlobalAlpha(wrap->ctx, alpha);
  }), emscripten::allow_raw_pointers());

  //
  // Transforms
  //
  // The paths, gradients, patterns and scissor region are transformed by an transformation
  // matrix at the time when they are passed to the API.
  // The current transformation matrix is a affine matrix:
  //   [sx kx tx]
  //   [ky sy ty]
  //   [ 0  0  1]
  // Where: sx,sy define scaling, kx,ky skewing, and tx,ty translation.
  // The last row is assumed to be 0,0,1 and is not stored.
  //
  // Apart from nvgResetTransform(), each transformation function first creates
  // specific transformation matrix and pre-multiplies the current transformation by it.
  //
  // Current coordinate system (transformation) can be saved and restored using nvgSave() and nvgRestore().

  // Resets current transform to a identity matrix.
  // void nvgResetTransform(NVGcontext* ctx);
  emscripten::function("nvgResetTransform", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgResetTransform(wrap->ctx);
  }), emscripten::allow_raw_pointers());

  // Premultiplies current coordinate system by specified matrix.
  // The parameters are interpreted as matrix as follows:
  //   [a c e]
  //   [b d f]
  //   [0 0 1]
  // void nvgTransform(NVGcontext* ctx, float a, float b, float c, float d, float e, float f);
  emscripten::function("nvgTransform", FUNCTION(void, (WrapNVGcontext* wrap, float a, float b, float c, float d, float e, float f), {
    nvgTransform(wrap->ctx, a, b, c, d, e, f);
  }), emscripten::allow_raw_pointers());

  // Translates current coordinate system.
  // void nvgTranslate(NVGcontext* ctx, float x, float y);
  emscripten::function("nvgTranslate", FUNCTION(void, (WrapNVGcontext* wrap, float x, float y), {
    nvgTranslate(wrap->ctx, x, y);
  }), emscripten::allow_raw_pointers());

  // Rotates current coordinate system. Angle is specified in radians.
  // void nvgRotate(NVGcontext* ctx, float angle);
  emscripten::function("nvgRotate", FUNCTION(void, (WrapNVGcontext* wrap, float angle), {
    nvgRotate(wrap->ctx, angle);
  }), emscripten::allow_raw_pointers());

  // Skews the current coordinate system along X axis. Angle is specified in radians.
  // void nvgSkewX(NVGcontext* ctx, float angle);
  emscripten::function("nvgSkewX", FUNCTION(void, (WrapNVGcontext* wrap, float angle), {
    nvgSkewX(wrap->ctx, angle);
  }), emscripten::allow_raw_pointers());

  // Skews the current coordinate system along Y axis. Angle is specified in radians.
  // void nvgSkewY(NVGcontext* ctx, float angle);
  emscripten::function("nvgSkewY", FUNCTION(void, (WrapNVGcontext* wrap, float angle), {
    nvgSkewY(wrap->ctx, angle);
  }), emscripten::allow_raw_pointers());

  // Scales the current coordinate system.
  // void nvgScale(NVGcontext* ctx, float x, float y);
  emscripten::function("nvgScale", FUNCTION(void, (WrapNVGcontext* wrap, float x, float y), {
    nvgScale(wrap->ctx, x, y);
  }), emscripten::allow_raw_pointers());

  // Stores the top part (a-f) of the current transformation matrix in to the specified buffer.
  //   [a c e]
  //   [b d f]
  //   [0 0 1]
  // There should be space for 6 floats in the return buffer for the values a-f.
  // void nvgCurrentTransform(NVGcontext* ctx, float* xform);
  emscripten::function("nvgCurrentTransform", FUNCTION(void, (WrapNVGcontext* wrap, emscripten::val xform), {
    float _xform[6];
    nvgCurrentTransform(wrap->ctx, _xform);
    xform.call<void>("set", emscripten::typed_memory_view<float>(6, _xform));
  }), emscripten::allow_raw_pointers());


  // The following functions can be used to make calculations on 2x3 transformation matrices.
  // A 2x3 matrix is represented as float[6].

  // Sets the transform to identity matrix.
  // void nvgTransformIdentity(float* dst);
  // emscripten::function("nvgTransformIdentity", FUNCTION(void, (float* dst), {
  // }), emscripten::allow_raw_pointers());

  // Sets the transform to translation matrix matrix.
  // void nvgTransformTranslate(float* dst, float tx, float ty);
  // emscripten::function("nvgTransformTranslate", FUNCTION(void, (float* dst, float tx, float ty), {
  // }), emscripten::allow_raw_pointers());

  // Sets the transform to scale matrix.
  // void nvgTransformScale(float* dst, float sx, float sy);
  // emscripten::function("nvgTransformScale", FUNCTION(void, (float* dst, float sx, float sy), {
  // }), emscripten::allow_raw_pointers());

  // Sets the transform to rotate matrix. Angle is specified in radians.
  // void nvgTransformRotate(float* dst, float a);
  // emscripten::function("nvgTransformRotate", FUNCTION(void, (float* dst, float a), {
  // }), emscripten::allow_raw_pointers());

  // Sets the transform to skew-x matrix. Angle is specified in radians.
  // void nvgTransformSkewX(float* dst, float a);
  // emscripten::function("nvgTransformSkewX", FUNCTION(void, (float* dst, float a), {
  // }), emscripten::allow_raw_pointers());

  // Sets the transform to skew-y matrix. Angle is specified in radians.
  // void nvgTransformSkewY(float* dst, float a);
  // emscripten::function("nvgTransformSkewY", FUNCTION(void, (float* dst, float a), {
  // }), emscripten::allow_raw_pointers());

  // Sets the transform to the result of multiplication of two transforms, of A = A*B.
  // void nvgTransformMultiply(float* dst, const float* src);
  // emscripten::function("nvgTransformMultiply", FUNCTION(void, (float* dst, const float* src), {
  // }), emscripten::allow_raw_pointers());

  // Sets the transform to the result of multiplication of two transforms, of A = B*A.
  // void nvgTransformPremultiply(float* dst, const float* src);
  // emscripten::function("nvgTransformPremultiply", FUNCTION(void, (float* dst, const float* src), {
  // }), emscripten::allow_raw_pointers());

  // Sets the destination to inverse of specified transform.
  // Returns 1 if the inverse could be calculated, else 0.
  // int nvgTransformInverse(float* dst, const float* src);
  // emscripten::function("nvgTransformInverse", FUNCTION(int, (float* dst, const float* src), {
  //   return 0;
  // }), emscripten::allow_raw_pointers());

  // Transform a point by given transform.
  // void nvgTransformPoint(float* dstx, float* dsty, const float* xform, float srcx, float srcy);
  // emscripten::function("nvgTransformPoint", FUNCTION(void, (float* dstx, float* dsty, const float* xform, float srcx, float srcy), {
  // }), emscripten::allow_raw_pointers());

  // Converts degrees to radians and vice versa.
  // float nvgDegToRad(float deg);
  // float nvgRadToDeg(float rad);

  //
  // Images
  //
  // NanoVG allows you to load jpg, png, psd, tga, pic and gif files to be used for rendering.
  // In addition you can upload your own image. The image loading is provided by stb_image.
  // The parameter imageFlags is combination of flags defined in NVGimageFlags.

  // Creates image by loading it from the disk from specified file name.
  // Returns handle to the image.
  // int nvgCreateImage(NVGcontext* ctx, const char* filename, int imageFlags);
  emscripten::function("nvgCreateImage", FUNCTION(int, (WrapNVGcontext* wrap, const char* filename, int imageFlags), {
    return 0;
  }), emscripten::allow_raw_pointers());

  // Creates image by loading it from the specified chunk of memory.
  // Returns handle to the image.
  // int nvgCreateImageMem(NVGcontext* ctx, int imageFlags, unsigned char* data, int ndata);
  emscripten::function("nvgCreateImageMem", FUNCTION(int, (WrapNVGcontext* wrap, int imageFlags, emscripten::val data), {
    std::vector<unsigned char> _data;
    _data.resize(data["length"].as<size_t>());
    emscripten::val(emscripten::typed_memory_view<unsigned char>(_data.size(), _data.data())).call<void>("set", data);
    return nvgCreateImageMem(wrap->ctx, imageFlags, _data.data(), _data.size());
  }), emscripten::allow_raw_pointers());

  // Creates image from specified image data.
  // Returns handle to the image.
  // int nvgCreateImageRGBA(NVGcontext* ctx, int w, int h, int imageFlags, const unsigned char* data);
  emscripten::function("nvgCreateImageRGBA", FUNCTION(int, (WrapNVGcontext* wrap, int w, int h, int imageFlags, emscripten::val data), {
    std::vector<unsigned char> _data;
    _data.resize(data["length"].as<size_t>());
    emscripten::val(emscripten::typed_memory_view<unsigned char>(_data.size(), _data.data())).call<void>("set", data);
    return nvgCreateImageRGBA(wrap->ctx, w, h, imageFlags, _data.data());
  }), emscripten::allow_raw_pointers());

  // Updates image data specified by image handle.
  // void nvgUpdateImage(NVGcontext* ctx, int image, const unsigned char* data);
  emscripten::function("nvgUpdateImage", FUNCTION(void, (WrapNVGcontext* wrap, int image, emscripten::val data), {
    std::vector<unsigned char> _data;
    _data.resize(data["length"].as<size_t>());
    emscripten::val(emscripten::typed_memory_view<unsigned char>(_data.size(), _data.data())).call<void>("set", data);
    nvgUpdateImage(wrap->ctx, image, _data.data());
  }), emscripten::allow_raw_pointers());

  // Returns the dimensions of a created image.
  // void nvgImageSize(NVGcontext* ctx, int image, int* w, int* h);
  emscripten::function("nvgImageSize", FUNCTION(void, (WrapNVGcontext* wrap, int image, emscripten::val w, emscripten::val h), {
    int _w = 0;
    int _h = 0;
    nvgImageSize(wrap->ctx, image, &_w, &_h);
    w.set(0, emscripten::val(_w));
    h.set(0, emscripten::val(_h));
  }), emscripten::allow_raw_pointers());

  // Deletes created image.
  // void nvgDeleteImage(NVGcontext* ctx, int image);
  emscripten::function("nvgDeleteImage", FUNCTION(void, (WrapNVGcontext* wrap, int image), {
    nvgDeleteImage(wrap->ctx, image);
  }), emscripten::allow_raw_pointers());

  //
  // Paints
  //
  // NanoVG supports four types of paints: linear gradient, box gradient, radial gradient and image pattern.
  // These can be used as paints for strokes and fills.

  // Creates and returns a linear gradient. Parameters (sx,sy)-(ex,ey) specify the start and end coordinates
  // of the linear gradient, icol specifies the start color and ocol the end color.
  // The gradient is transformed by the current transform when it is passed to nvgFillPaint() or nvgStrokePaint().
  // NVGpaint nvgLinearGradient(NVGcontext* ctx, float sx, float sy, float ex, float ey, NVGcolor icol, NVGcolor ocol);
  emscripten::function("nvgLinearGradient", FUNCTION(emscripten::val, (WrapNVGcontext* wrap, float sx, float sy, float ex, float ey, emscripten::val icol, emscripten::val ocol, emscripten::val out), {
    return export_NVGpaint(nvgLinearGradient(wrap->ctx, sx, sy, ex, ey, import_NVGcolor(icol), import_NVGcolor(ocol)), out);
  }), emscripten::allow_raw_pointers());

  // Creates and returns a box gradient. Box gradient is a feathered rounded rectangle, it is useful for rendering
  // drop shadows or highlights for boxes. Parameters (x,y) define the top-left corner of the rectangle,
  // (w,h) define the size of the rectangle, r defines the corner radius, and f feather. Feather defines how blurry
  // the border of the rectangle is. Parameter icol specifies the inner color and ocol the outer color of the gradient.
  // The gradient is transformed by the current transform when it is passed to nvgFillPaint() or nvgStrokePaint().
  // NVGpaint nvgBoxGradient(NVGcontext* ctx, float x, float y, float w, float h, float r, float f, NVGcolor icol, NVGcolor ocol);
  emscripten::function("nvgBoxGradient", FUNCTION(emscripten::val, (WrapNVGcontext* wrap, float x, float y, float w, float h, float r, float f, emscripten::val icol, emscripten::val ocol, emscripten::val out), {
    return export_NVGpaint(nvgBoxGradient(wrap->ctx, x, y, w, h, r, f, import_NVGcolor(icol), import_NVGcolor(ocol)), out);
  }), emscripten::allow_raw_pointers());

  // Creates and returns a radial gradient. Parameters (cx,cy) specify the center, inr and outr specify
  // the inner and outer radius of the gradient, icol specifies the start color and ocol the end color.
  // The gradient is transformed by the current transform when it is passed to nvgFillPaint() or nvgStrokePaint().
  // NVGpaint nvgRadialGradient(NVGcontext* ctx, float cx, float cy, float inr, float outr, NVGcolor icol, NVGcolor ocol);
  emscripten::function("nvgRadialGradient", FUNCTION(emscripten::val, (WrapNVGcontext* wrap, float cx, float cy, float inr, float outr, emscripten::val icol, emscripten::val ocol, emscripten::val out), {
    return export_NVGpaint(nvgRadialGradient(wrap->ctx, cx, cy, inr, outr, import_NVGcolor(icol), import_NVGcolor(ocol)), out);
  }), emscripten::allow_raw_pointers());

  // Creates and returns an image patter. Parameters (ox,oy) specify the left-top location of the image pattern,
  // (ex,ey) the size of one image, angle rotation around the top-left corner, image is handle to the image to render.
  // The gradient is transformed by the current transform when it is passed to nvgFillPaint() or nvgStrokePaint().
  // NVGpaint nvgImagePattern(NVGcontext* ctx, float ox, float oy, float ex, float ey, float angle, int image, float alpha);
  emscripten::function("nvgImagePattern", FUNCTION(emscripten::val, (WrapNVGcontext* wrap, float ox, float oy, float ex, float ey, float angle, int image, float alpha, emscripten::val out), {
    return export_NVGpaint(nvgImagePattern(wrap->ctx, ox, oy, ex, ey, angle, image, alpha), out);
  }), emscripten::allow_raw_pointers());

  //
  // Scissoring
  //
  // Scissoring allows you to clip the rendering into a rectangle. This is useful for various
  // user interface cases like rendering a text edit or a timeline.

  // Sets the current scissor rectangle.
  // The scissor rectangle is transformed by the current transform.
  // void nvgScissor(NVGcontext* ctx, float x, float y, float w, float h);
  emscripten::function("nvgScissor", FUNCTION(void, (WrapNVGcontext* wrap, float x, float y, float w, float h), {
    nvgScissor(wrap->ctx, x, y, w, h);
  }), emscripten::allow_raw_pointers());

  // Intersects current scissor rectangle with the specified rectangle.
  // The scissor rectangle is transformed by the current transform.
  // Note: in case the rotation of previous scissor rect differs from
  // the current one, the intersection will be done between the specified
  // rectangle and the previous scissor rectangle transformed in the current
  // transform space. The resulting shape is always rectangle.
  // void nvgIntersectScissor(NVGcontext* ctx, float x, float y, float w, float h);
  emscripten::function("nvgIntersectScissor", FUNCTION(void, (WrapNVGcontext* wrap, float x, float y, float w, float h), {
    nvgIntersectScissor(wrap->ctx, x, y, w, h);
  }), emscripten::allow_raw_pointers());

  // Reset and disables scissoring.
  // void nvgResetScissor(NVGcontext* ctx);
  emscripten::function("nvgResetScissor", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgResetScissor(wrap->ctx);
  }), emscripten::allow_raw_pointers());

  //
  // Paths
  //
  // Drawing a new shape starts with nvgBeginPath(), it clears all the currently defined paths.
  // Then you define one or more paths and sub-paths which describe the shape. The are functions
  // to draw common shapes like rectangles and circles, and lower level step-by-step functions,
  // which allow to define a path curve by curve.
  //
  // NanoVG uses even-odd fill rule to draw the shapes. Solid shapes should have counter clockwise
  // winding and holes should have counter clockwise order. To specify winding of a path you can
  // call nvgPathWinding(). This is useful especially for the common shapes, which are drawn CCW.
  //
  // Finally you can fill the path using current fill style by calling nvgFill(), and stroke it
  // with current stroke style by calling nvgStroke().
  //
  // The curve segments and sub-paths are transformed by the current transform.

  // Clears the current path and sub-paths.
  // void nvgBeginPath(NVGcontext* ctx);
  emscripten::function("nvgBeginPath", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgBeginPath(wrap->ctx);
  }), emscripten::allow_raw_pointers());

  // Starts new sub-path with specified point as first point.
  // void nvgMoveTo(NVGcontext* ctx, float x, float y);
  emscripten::function("nvgMoveTo", FUNCTION(void, (WrapNVGcontext* wrap, float x, float y), {
    nvgMoveTo(wrap->ctx, x, y);
  }), emscripten::allow_raw_pointers());

  // Adds line segment from the last point in the path to the specified point.
  // void nvgLineTo(NVGcontext* ctx, float x, float y);
  emscripten::function("nvgLineTo", FUNCTION(void, (WrapNVGcontext* wrap, float x, float y), {
    nvgLineTo(wrap->ctx, x, y);
  }), emscripten::allow_raw_pointers());

  // Adds cubic bezier segment from last point in the path via two control points to the specified point.
  // void nvgBezierTo(NVGcontext* ctx, float c1x, float c1y, float c2x, float c2y, float x, float y);
  emscripten::function("nvgBezierTo", FUNCTION(void, (WrapNVGcontext* wrap, float c1x, float c1y, float c2x, float c2y, float x, float y), {
    nvgBezierTo(wrap->ctx, c1x, c1y, c2x, c2y, x, y);
  }), emscripten::allow_raw_pointers());

  // Adds quadratic bezier segment from last point in the path via a control point to the specified point.
  // void nvgQuadTo(NVGcontext* ctx, float cx, float cy, float x, float y);
  emscripten::function("nvgQuadTo", FUNCTION(void, (WrapNVGcontext* wrap, float cx, float cy, float x, float y), {
    nvgQuadTo(wrap->ctx, cx, cy, x, y);
  }), emscripten::allow_raw_pointers());

  // Adds an arc segment at the corner defined by the last path point, and two specified points.
  // void nvgArcTo(NVGcontext* ctx, float x1, float y1, float x2, float y2, float radius);
  emscripten::function("nvgArcTo", FUNCTION(void, (WrapNVGcontext* wrap, float x1, float y1, float x2, float y2, float radius), {
    nvgArcTo(wrap->ctx, x1, y1, x2, y2, radius);
  }), emscripten::allow_raw_pointers());

  // Closes current sub-path with a line segment.
  // void nvgClosePath(NVGcontext* ctx);
  emscripten::function("nvgClosePath", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgClosePath(wrap->ctx);
  }), emscripten::allow_raw_pointers());

  // Sets the current sub-path winding, see NVGwinding and NVGsolidity.
  // void nvgPathWinding(NVGcontext* ctx, int dir);
  emscripten::function("nvgPathWinding", FUNCTION(void, (WrapNVGcontext* wrap, int dir), {
    nvgPathWinding(wrap->ctx, dir);
  }), emscripten::allow_raw_pointers());

  // Creates new circle arc shaped sub-path. The arc center is at cx,cy, the arc radius is r,
  // and the arc is drawn from angle a0 to a1, and swept in direction dir (NVG_CCW, or NVG_CW).
  // Angles are specified in radians.
  // void nvgArc(NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int dir);
  emscripten::function("nvgArc", FUNCTION(void, (WrapNVGcontext* wrap, float cx, float cy, float r, float a0, float a1, int dir), {
    nvgArc(wrap->ctx, cx, cy, r, a0, a1, dir);
  }), emscripten::allow_raw_pointers());

  // Creates new rectangle shaped sub-path.
  // void nvgRect(NVGcontext* ctx, float x, float y, float w, float h);
  emscripten::function("nvgRect", FUNCTION(void, (WrapNVGcontext* wrap, float x, float y, float w, float h), {
    nvgRect(wrap->ctx, x, y, w, h);
  }), emscripten::allow_raw_pointers());

  // Creates new rounded rectangle shaped sub-path.
  // void nvgRoundedRect(NVGcontext* ctx, float x, float y, float w, float h, float r);
  emscripten::function("nvgRoundedRect", FUNCTION(void, (WrapNVGcontext* wrap, float x, float y, float w, float h, float r), {
    nvgRoundedRect(wrap->ctx, x, y, w, h, r);
  }), emscripten::allow_raw_pointers());

  // Creates new rounded rectangle shaped sub-path with varying radii for each corner.
  // void nvgRoundedRectVarying(NVGcontext* ctx, float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft);
  emscripten::function("nvgRoundedRectVarying", FUNCTION(void, (WrapNVGcontext* wrap, float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft), {
    nvgRoundedRectVarying(wrap->ctx, x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
  }), emscripten::allow_raw_pointers());

  // Creates new ellipse shaped sub-path.
  // void nvgEllipse(NVGcontext* ctx, float cx, float cy, float rx, float ry);
  emscripten::function("nvgEllipse", FUNCTION(void, (WrapNVGcontext* wrap, float cx, float cy, float rx, float ry), {
    nvgEllipse(wrap->ctx, cx, cy, rx, ry);
  }), emscripten::allow_raw_pointers());

  // Creates new circle shaped sub-path.
  // void nvgCircle(NVGcontext* ctx, float cx, float cy, float r);
  emscripten::function("nvgCircle", FUNCTION(void, (WrapNVGcontext* wrap, float cx, float cy, float r), {
    nvgCircle(wrap->ctx, cx, cy, r);
  }), emscripten::allow_raw_pointers());

  // Fills the current path with current fill style.
  // void nvgFill(NVGcontext* ctx);
  emscripten::function("nvgFill", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgFill(wrap->ctx);
  }), emscripten::allow_raw_pointers());

  // Fills the current path with current stroke style.
  // void nvgStroke(NVGcontext* ctx);
  emscripten::function("nvgStroke", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgStroke(wrap->ctx);
  }), emscripten::allow_raw_pointers());



  //
  // Text
  //
  // NanoVG allows you to load .ttf files and use the font to render text.
  //
  // The appearance of the text can be defined by setting the current text style
  // and by specifying the fill color. Common text and font settings such as
  // font size, letter spacing and text align are supported. Font blur allows you
  // to create simple text effects such as drop shadows.
  //
  // At render time the font face can be set based on the font handles or name.
  //
  // Font measure functions return values in local space, the calculations are
  // carried in the same resolution as the final rendering. This is done because
  // the text glyph positions are snapped to the nearest pixels sharp rendering.
  //
  // The local space means that values are not rotated or scale as per the current
  // transformation. For example if you set font size to 12, which would mean that
  // line height is 16, then regardless of the current scaling and rotation, the
  // returned line height is always 16. Some measures may vary because of the scaling
  // since aforementioned pixel snapping.
  //
  // While this may sound a little odd, the setup allows you to always render the
  // same way regardless of scaling. I.e. following works regardless of scaling:
  //
  //		const char* txt = "Text me up.";
  //		nvgTextBounds(vg, x,y, txt, NULL, bounds);
  //		nvgBeginPath(vg);
  //		nvgRoundedRect(vg, bounds[0],bounds[1], bounds[2]-bounds[0], bounds[3]-bounds[1]);
  //		nvgFill(vg);
  //
  // Note: currently only solid color fill is supported for text.

  // Creates font by loading it from the disk from specified file name.
  // Returns handle to the font.
  // int nvgCreateFont(NVGcontext* ctx, const char* name, const char* filename);
  emscripten::function("nvgCreateFont", FUNCTION(int, (WrapNVGcontext* wrap, std::string name, std::string filename), {
    // return nvgCreateFont(wrap->ctx, name.c_str(), filename.c_str());
    return -1;
  }), emscripten::allow_raw_pointers());

  // Creates font by loading it from the specified memory chunk.
  // Returns handle to the font.
  // int nvgCreateFontMem(NVGcontext* ctx, const char* name, unsigned char* data, int ndata, int freeData);
  emscripten::function("nvgCreateFontMem", FUNCTION(int, (WrapNVGcontext* wrap, std::string name, emscripten::val data), {
    std::vector<unsigned char> _data;
    _data.resize(data["length"].as<size_t>());
    emscripten::val(emscripten::typed_memory_view<unsigned char>(_data.size(), _data.data())).call<void>("set", data);
    // return nvgCreateFontMem(wrap->ctx, name.c_str(), _data.data(), _data.size(), 0);
    unsigned char* __data = (unsigned char*) malloc(_data.size());
    memcpy(__data, _data.data(), _data.size());
    return nvgCreateFontMem(wrap->ctx, name.c_str(), __data, _data.size(), 1);
  }), emscripten::allow_raw_pointers());

  // Finds a loaded font of specified name, and returns handle to it, or -1 if the font is not found.
  // int nvgFindFont(NVGcontext* ctx, const char* name);
  emscripten::function("nvgFindFont", FUNCTION(int, (WrapNVGcontext* wrap, std::string name), {
    return nvgFindFont(wrap->ctx, name.c_str());
  }), emscripten::allow_raw_pointers());

  // Adds a fallback font by handle.
  // int nvgAddFallbackFontId(NVGcontext* ctx, int baseFont, int fallbackFont);
  emscripten::function("nvgAddFallbackFontId", FUNCTION(int, (WrapNVGcontext* wrap, int baseFont, int fallbackFont), {
    return nvgAddFallbackFontId(wrap->ctx, baseFont, fallbackFont);
  }), emscripten::allow_raw_pointers());

  // Adds a fallback font by name.
  // int nvgAddFallbackFont(NVGcontext* ctx, const char* baseFont, const char* fallbackFont);
  emscripten::function("nvgAddFallbackFont", FUNCTION(int, (WrapNVGcontext* wrap, std::string baseFont, std::string fallbackFont), {
    return nvgAddFallbackFont(wrap->ctx, baseFont.c_str(), fallbackFont.c_str());
  }), emscripten::allow_raw_pointers());

  // Sets the font size of current text style.
  // void nvgFontSize(NVGcontext* ctx, float size);
  emscripten::function("nvgFontSize", FUNCTION(void, (WrapNVGcontext* wrap, float size), {
    nvgFontSize(wrap->ctx, size);
  }), emscripten::allow_raw_pointers());

  // Sets the blur of current text style.
  // void nvgFontBlur(NVGcontext* ctx, float blur);
  emscripten::function("nvgFontBlur", FUNCTION(void, (WrapNVGcontext* wrap, float blur), {
    nvgFontBlur(wrap->ctx, blur);
  }), emscripten::allow_raw_pointers());

  // Sets the letter spacing of current text style.
  // void nvgTextLetterSpacing(NVGcontext* ctx, float spacing);
  emscripten::function("nvgTextLetterSpacing", FUNCTION(void, (WrapNVGcontext* wrap, float spacing), {
    nvgTextLetterSpacing(wrap->ctx, spacing);
  }), emscripten::allow_raw_pointers());

  // Sets the proportional line height of current text style. The line height is specified as multiple of font size.
  // void nvgTextLineHeight(NVGcontext* ctx, float lineHeight);
  emscripten::function("nvgTextLineHeight", FUNCTION(void, (WrapNVGcontext* wrap, float lineHeight), {
    nvgTextLineHeight(wrap->ctx, lineHeight);
  }), emscripten::allow_raw_pointers());

  // Sets the text align of current text style, see NVGalign for options.
  // void nvgTextAlign(NVGcontext* ctx, int align);
  emscripten::function("nvgTextAlign", FUNCTION(void, (WrapNVGcontext* wrap, int align), {
    nvgTextAlign(wrap->ctx, align);
  }), emscripten::allow_raw_pointers());

  // Sets the font face based on specified id of current text style.
  // void nvgFontFaceId(NVGcontext* ctx, int font);
  emscripten::function("nvgFontFaceId", FUNCTION(void, (WrapNVGcontext* wrap, int font), {
    nvgFontFaceId(wrap->ctx, font);
  }), emscripten::allow_raw_pointers());

  // Sets the font face based on specified name of current text style.
  // void nvgFontFace(NVGcontext* ctx, const char* font);
  emscripten::function("nvgFontFace", FUNCTION(void, (WrapNVGcontext* wrap, std::string font), {
    nvgFontFace(wrap->ctx, font.c_str());
  }), emscripten::allow_raw_pointers());

  // Draws text string at specified location. If end is specified only the sub-string up to the end is drawn.
  // float nvgText(NVGcontext* ctx, float x, float y, const char* string, const char* end);
  emscripten::function("nvgText", FUNCTION(float, (WrapNVGcontext* wrap, float x, float y, std::string string, int end), {
    return nvgText(wrap->ctx, x, y, string.c_str(), end == 0 ? 0 : &string.c_str()[end]);
  }), emscripten::allow_raw_pointers());

  // Draws multi-line text string at specified location wrapped at the specified width. If end is specified only the sub-string up to the end is drawn.
  // White space is stripped at the beginning of the rows, the text is split at word boundaries or when new-line characters are encountered.
  // Words longer than the max width are slit at nearest character (i.e. no hyphenation).
  // void nvgTextBox(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end);
  emscripten::function("nvgTextBox", FUNCTION(void, (WrapNVGcontext* wrap, float x, float y, float breakRowWidth, std::string string, int end), {
    nvgTextBox(wrap->ctx, x, y, breakRowWidth, string.c_str(), end == 0 ? 0 : &string.c_str()[end]);
  }), emscripten::allow_raw_pointers());

  // Measures the specified text string. Parameter bounds should be a pointer to float[4],
  // if the bounding box of the text should be returned. The bounds value are [xmin,ymin, xmax,ymax]
  // Returns the horizontal advance of the measured text (i.e. where the next character should drawn).
  // Measured values are returned in local coordinate space.
  // float nvgTextBounds(NVGcontext* ctx, float x, float y, const char* string, const char* end, float* bounds);
  emscripten::function("nvgTextBounds", FUNCTION(float, (WrapNVGcontext* wrap, float x, float y, std::string string, int end, emscripten::val bounds), {
    float _bounds[4];
    float ret = nvgTextBounds(wrap->ctx, x, y, string.c_str(), end == 0 ? 0 : &string.c_str()[end], bounds.isNull() ? NULL : _bounds);
    if (!bounds.isNull()) {
      bounds.set(0, _bounds[0]);
      bounds.set(1, _bounds[1]);
      bounds.set(2, _bounds[2]);
      bounds.set(3, _bounds[3]);
    }
    return ret;
  }), emscripten::allow_raw_pointers());

  // Measures the specified multi-text string. Parameter bounds should be a pointer to float[4],
  // if the bounding box of the text should be returned. The bounds value are [xmin,ymin, xmax,ymax]
  // Measured values are returned in local coordinate space.
  // void nvgTextBoxBounds(NVGcontext* ctx, float x, float y, float breakRowWidth, const char* string, const char* end, float* bounds);
  emscripten::function("nvgTextBoxBounds", FUNCTION(void, (WrapNVGcontext* wrap, float x, float y, float breakRowWidth, std::string string, int end, emscripten::val bounds), {
    float _bounds[4];
    nvgTextBoxBounds(wrap->ctx, x, y, breakRowWidth, string.c_str(), end == 0 ? 0 : &string.c_str()[end], _bounds);
    bounds.set(0, _bounds[0]);
    bounds.set(1, _bounds[1]);
    bounds.set(2, _bounds[2]);
    bounds.set(3, _bounds[3]);
  }), emscripten::allow_raw_pointers());

  // Calculates the glyph x positions of the specified text. If end is specified only the sub-string will be used.
  // Measured values are returned in local coordinate space.
  // int nvgTextGlyphPositions(NVGcontext* ctx, float x, float y, const char* string, const char* end, NVGglyphPosition* positions, int maxPositions);
  emscripten::function("nvgTextGlyphPositions", FUNCTION(int, (WrapNVGcontext* wrap, float x, float y, std::string string, int end, emscripten::val positions, int maxPositions), {
    const char* _string = string.c_str();
    const char* _end = end == 0 ? NULL : &_string[end];
    NVGglyphPosition _positions[maxPositions];
    int ret = nvgTextGlyphPositions(wrap->ctx, x, y, _string, _end, _positions, maxPositions);
    for (int i = 0; i < positions["length"].as<int>(); ++i) {
      NVGglyphPosition& _position = _positions[i];
      emscripten::val position = positions[i];
      /*
      struct NVGglyphPosition {
        const char* str;	// Position of the glyph in the input string.
        float x;			// The x-coordinate of the logical glyph position.
        float minx, maxx;	// The bounds of the glyph shape.
      };
      */
      position.set("str", (int)(_position.str - _string));
      position.set("x", _position.x);
      position.set("minx", _position.minx);
      position.set("maxx", _position.maxx);
    }
    return ret;
  }), emscripten::allow_raw_pointers());

  // Returns the vertical metrics based on the current text style.
  // Measured values are returned in local coordinate space.
  // void nvgTextMetrics(NVGcontext* ctx, float* ascender, float* descender, float* lineh);
  emscripten::function("nvgTextMetrics", FUNCTION(void, (WrapNVGcontext* wrap, emscripten::val ascender, emscripten::val descender, emscripten::val lineh), {
    float _ascender = 0.0f;
    float _descender = 0.0f;
    float _lineh = 0.0f;
    nvgTextMetrics(wrap->ctx, ascender.isNull() ? NULL : &_ascender, descender.isNull() ? NULL : &_descender, lineh.isNull() ? NULL : &_lineh);
    if (!ascender.isNull()) { ascender.set(0, _ascender); }
    if (!descender.isNull()) { descender.set(0, _descender); }
    if (!lineh.isNull()) { lineh.set(0, _lineh); }
  }), emscripten::allow_raw_pointers());

  // Breaks the specified text into lines. If end is specified only the sub-string will be used.
  // White space is stripped at the beginning of the rows, the text is split at word boundaries or when new-line characters are encountered.
  // Words longer than the max width are slit at nearest character (i.e. no hyphenation).
  // int nvgTextBreakLines(NVGcontext* ctx, const char* string, const char* end, float breakRowWidth, NVGtextRow* rows, int maxRows);
  emscripten::function("nvgTextBreakLines", FUNCTION(int, (WrapNVGcontext* wrap, std::string string, int end, float breakRowWidth, emscripten::val rows, int maxRows), {
    const char* _string = string.c_str();
    const char* _end = end == 0 ? NULL : &_string[end];
    NVGtextRow _rows[maxRows];
    int ret = nvgTextBreakLines(wrap->ctx, _string, _end, breakRowWidth, _rows, maxRows);
    for (int i = 0; i < rows["length"].as<int>(); ++i) {
      NVGtextRow& _row = _rows[i];
      emscripten::val row = rows[i];
      /*
      struct NVGtextRow {
        const char* start;	// Pointer to the input text where the row starts.
        const char* end;	// Pointer to the input text where the row ends (one past the last character).
        const char* next;	// Pointer to the beginning of the next row.
        float width;		// Logical width of the row.
        float minx, maxx;	// Actual bounds of the row. Logical with and bounds can differ because of kerning and some parts over extending.
      };
      */
      row.set("start", (int)(_row.start - _string));
      row.set("end", (int)(_row.end - _string));
      row.set("next", (int)(_row.next - _string));
      row.set("width", _row.width);
      row.set("minx", _row.minx);
      row.set("maxx", _row.maxx);
    }
    return ret;
  }), emscripten::allow_raw_pointers());

  // Constructor and destructor, called by the render back-end.
  // NVGcontext* nvgCreateInternal(NVGparams* params);
  // emscripten::function("nvgCreateInternal", FUNCTION(WrapNVGcontext*, (emscripten::val params), {
  //   return new WrapNVGcontext(params);
  // }), emscripten::allow_raw_pointers());
  // void nvgDeleteInternal(NVGcontext* ctx);
  // emscripten::function("nvgDeleteInternal", FUNCTION(void, (WrapNVGcontext* wrap), {
  //   delete wrap;
  // }), emscripten::allow_raw_pointers());

  // NVGparams* nvgInternalParams(NVGcontext* ctx);
  // emscripten::function("nvgInternalParams", FUNCTION(emscripten::val, (WrapNVGcontext* wrap), {
  //   return wrap->_params;
  // }), emscripten::allow_raw_pointers());

  // Debug function to dump cached path data.
  // // void nvgDebugDumpPathCache(NVGcontext* ctx);
  emscripten::function("nvgDebugDumpPathCache", FUNCTION(void, (WrapNVGcontext* wrap), {
    nvgDebugDumpPathCache(wrap->ctx);
  }), emscripten::allow_raw_pointers());
}
