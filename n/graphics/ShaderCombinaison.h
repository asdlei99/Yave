/*******************************
Copyright (C) 2013-2015 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/

#ifndef N_GRAPHICS_SHADERCOMBINAISON
#define N_GRAPHICS_SHADERCOMBINAISON

#include "Shader.h"
#include "UniformBuffer.h"
#include "GLContext.h"
#include "TextureBinding.h"
#include <n/core/Map.h>
#include <n/math/Matrix.h>

namespace n {
namespace graphics {

class ShaderCombinaison : core::NonCopyable
{
	public:
		typedef gl::GLint UniformAddr;

		enum StandardVertexShader
		{
			ProjectionShader = 0,
			NoProjectionShader = 1
		};

	private:
		struct UniformInfo
		{
			UniformAddr addr;
			uint size;
		};

		struct BlockInfo
		{
			gl::GLuint addr;
			gl::GLuint slot;
			const typename internal::DynamicBufferBase<GL_UNIFORM_BUFFER> *buffer;
		};

		static Shader<VertexShader> *getDefaultVertexShader(StandardVertexShader type = ProjectionShader) {
			static Shader<VertexShader> **def = 0;
			if(!def) {
					def = new Shader<VertexShader>*[2];
					def[ProjectionShader] = new Shader<VertexShader>("#version 420 core\n"
								"layout(location = 0) in vec3 n_VertexPosition;"
								"layout(location = 1) in vec3 n_VertexNormal;"
								"layout(location = 2) in vec3 n_VertexTangent;"
								"layout(location = 3) in vec2 n_VertexCoord;"
								"uniform mat4 n_ViewProjectionMatrix;"
								"uniform mat4 n_ModelMatrix;"
								"uniform vec3 n_Camera;"

								"out vec3 n_Position;"
								"out vec3 n_Normal;"
								"out vec3 n_Tangent;"
								"out vec3 n_Binormal;"
								"out vec3 n_View;"
								"out vec2 n_TexCoord;"
								"out int n_VertexID;"

								"void main() {"
									"vec4 model = n_ModelMatrix * vec4(n_VertexPosition, 1.0);"
									"gl_Position = n_ViewProjectionMatrix * model;"
									"n_VertexID = gl_VertexID;"
									"n_Position = model.xyz;"
									"n_View = normalize(n_Camera - model.xyz);"
									"n_Normal = mat3(n_ModelMatrix) * n_VertexNormal;"
									"n_Tangent = mat3(n_ModelMatrix) * n_VertexTangent;"
									"n_TexCoord = n_VertexCoord;"
									"n_Binormal = cross(n_Normal, n_Tangent);"
								"}");
					def[NoProjectionShader] = new Shader<VertexShader>("#version 420 core\n"
								"layout(location = 0) in vec3 n_VertexPosition;"
								"layout(location = 1) in vec3 n_VertexNormal;"
								"layout(location = 2) in vec3 n_VertexTangent;"
								"layout(location = 3) in vec2 n_VertexCoord;"

								"out vec3 n_Position;"
								"out vec3 n_Normal;"
								"out vec3 n_Tangent;"
								"out vec3 n_Binormal;"
								"out vec2 n_TexCoord;"
								"out int n_VertexID;"

								"void main() {"
									"gl_Position = vec4(n_Position = n_VertexPosition, 1.0);"
									"n_VertexID = gl_VertexID;"
									"n_Normal = n_VertexNormal;"
									"n_Tangent = n_VertexTangent;"
									"n_TexCoord = n_VertexCoord;"
									"n_Binormal = cross(n_Normal, n_Tangent);"
								"}");
				}
				return def[type];
		}

	public:
		class Uniform : core::NonCopyable
		{
			public:
				Uniform(Uniform &&u) : sh(u.sh), name(u.name) {
				}

				template<typename T>
				T operator=(const T &t) {
					sh->setValue(name, t);
					return t;
				}

				template<typename T>
				const UniformBuffer<T> &operator=(const UniformBuffer<T> &t) {
					sh->setBuffer(name, &t);
					return t;
				}

			private:
				friend class ShaderCombinaison;
				Uniform(const ShaderCombinaison *s, const core::String &a) : sh(s), name(a) {
				}

				const ShaderCombinaison *sh;
				core::String name;
		};

		ShaderCombinaison(Shader<FragmentShader> *frag, Shader<VertexShader> *vert = 0, Shader<GeometryShader> *geom = 0) : handle(0), bindings(0) {
			if(!vert) {
				vert = getDefaultVertexShader();
			}
			shaders[FragmentShader] = frag;
			shaders[VertexShader] = vert;
			shaders[GeometryShader] = geom;
			compile();
		}

		ShaderCombinaison(Shader<FragmentShader> *frag, StandardVertexShader vs) : ShaderCombinaison(frag, getDefaultVertexShader(vs)) {
		}

		~ShaderCombinaison() {
			if(handle) {
				gl::GLuint h = handle;
				GLContext::getContext()->addGLTask([=]() {
					gl::glDeleteProgram(h);
				});
			}
			delete[] bindings;
		}

		bool isValid() const {
			return handle;
		}

		const core::String &getLogs() const {
			return logs;
		}

		void bind() const {
			if(GLContext::getContext()->shader == this) {
				return;
			}
			gl::glUseProgram(handle);
			for(const core::Pair<const core::String, BlockInfo> &i : blocks) {
				const BlockInfo &infos = i._2;
				if(infos.buffer && infos.addr != gl::GLuint(GL_INVALID_INDEX)) {
					infos.buffer->update();
					gl::glBindBufferBase(GL_UNIFORM_BUFFER, infos.slot, infos.buffer->handle);
				}
			}
			for(uint i = 0; i != samplers.size(); i++) {
				bindings[i].bind(i);
			}

			setValue("n_ModelMatrix", GLContext::getContext()->getModelMatrix());
			setValue("n_ProjectionMatrix", GLContext::getContext()->getProjectionMatrix());
			setValue("n_ViewMatrix", GLContext::getContext()->getViewMatrix());
			setValue("n_ViewProjectionMatrix", GLContext::getContext()->getProjectionMatrix() * GLContext::getContext()->getViewMatrix());

			GLContext::getContext()->shader = this;
		}

		UniformAddr getAddr(const core::String &name) const {
			return getInfo(name).addr;
		}

		Uniform operator[](const core::String &name) const {
			return Uniform(this, name);
		}

		template<typename T>
		void setValue(const core::String &name, const T &t) const {
			UniformAddr i = getAddr(name);
			if(i != UniformAddr(GL_INVALID_INDEX)) {
				setValue(i, t);
			}
		}

		void setValue(UniformAddr addr, int a) const {
			gl::glProgramUniform1i(handle, addr, a);
		}

		void setValue(UniformAddr addr, uint a) const {
			gl::glProgramUniform1ui(handle, addr, a);
		}

		void setValue(UniformAddr addr, float f) const {
			gl::glProgramUniform1f(handle, addr, f);
		}

		void setValue(UniformAddr addr, double f) const {
			gl::glProgramUniform1f(handle, addr, f);
		}

		void setValue(UniformAddr addr, const math::Vec2i &v) const {
			gl::glProgramUniform2iv(handle, addr, 1, v.begin());
		}

		void setValue(UniformAddr addr, const math::Vec3i &v) const {
			gl::glProgramUniform3iv(handle, addr, 1, v.begin());
		}

		void setValue(UniformAddr addr, const math::Vec2 &v) const {
			gl::glProgramUniform2fv(handle, addr, 1, v.begin());
		}

		void setValue(UniformAddr addr, const math::Vec3 &v) const {
			gl::glProgramUniform3fv(handle, addr, 1, v.begin());
		}

		void setValue(UniformAddr addr, const math::Vec4 &v) const {
			gl::glProgramUniform4fv(handle, addr, 1, v.begin());
		}

		void setValue(UniformAddr addr, const math::Matrix4<float> &m) const {
			gl::glProgramUniformMatrix4fv(handle, addr, 1, GL_TRUE, m.begin());
		}

		void setValue(UniformAddr addr, const Texture &t) const {
			core::Map<UniformAddr, uint>::const_iterator it = samplers.find(addr);
			if(it != samplers.end()) {
				uint slot = (*it)._2;
				bindings[slot] = t;
				if(isCurrent()) {
					bindings[slot].bind(slot);
				} else {
					t.prepare();
				}
			}
		}

		template<typename T>
		void setBuffer(const core::String &name, const UniformBuffer<T> *buffer) const {
			core::Map<core::String, BlockInfo>::iterator it = blocks.find(name);
			BlockInfo infos{gl::GLuint(GL_INVALID_INDEX), 0, 0};
			if(it == blocks.end()) {
				gl::GLint index = gl::glGetUniformBlockIndex(handle, name.toChar());
				static uint bs = 0;
				blocks[name] = infos = BlockInfo{gl::GLuint(index), bs++, 0};
				gl::glUniformBlockBinding(handle, infos.addr, infos.slot);
			} else {
				infos = (*it)._2;
			}
			(*it)._2.buffer = buffer;
			if(buffer) {
				if(isCurrent() && infos.addr != gl::GLuint(GL_INVALID_INDEX)) {
					buffer->update();
					gl::glBindBufferBase(GL_UNIFORM_BUFFER, infos.slot, buffer->handle);
				}
			}

		}

	private:
		bool isCurrent() const {
			return GLContext::getContext()->shader == this;
		}

		void compile() {
			handle = gl::glCreateProgram();
			bool val = true;
			for(uint i = 0; i != 3; i++) {
				if(shaders[i]) {
					gl::glAttachShader(handle, shaders[i]->handle);
					val &= shaders[i]->isValid();
					if(!shaders[i]->getLogs().isEmpty()) {
						logs += shaders[i]->getLogs() + "\n";
					}
				}
			}
			gl::glLinkProgram(handle);
			int res = 0;
			gl::glGetProgramiv(handle, GL_LINK_STATUS, &res);
			if(!res || !val) {
				int size = 0;
				gl::glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &size);
				char *msg = new char[size + 1];
				gl::glGetProgramInfoLog(handle, size, &res, msg);
				gl::glDeleteProgram(handle);
				handle = 0;
				msg[size] = '\0';
				logs += msg;
				delete[] msg;
			} else {
				getUniforms();
			}
		}

		void getUniforms() {
			const uint max = 512;
			char name[max];
			int uniforms = 0;
			gl::glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &uniforms);
			for(uint i = 0; i != (uint)uniforms; i++) {
				int size = 0;
				gl::GLenum type = GL_NONE;
				gl::glGetActiveUniform(handle, i, max, 0, &size, &type, name);
				core::String uniform = name;
				if(uniform.endWith("[0]")) {
					uniform = uniform.subString(0, uniform.size() - 3);
				}
				UniformInfo info({gl::glGetUniformLocation(handle, name), (uint)size});
				uniformsInfo[uniform] = info;
				if(type == GL_SAMPLER_2D) {
					uint slot = samplers.size();
					setValue(info.addr, int(slot));
					samplers[info.addr] = slot;
				}
			}
			bindings = new internal::TextureBinding[samplers.size()];
		}

		UniformInfo getInfo(const core::String &name) const {
			return uniformsInfo.get(name, UniformInfo{UniformAddr(GL_INVALID_INDEX), 0});
		}

		internal::ShaderBase *shaders[3];
		gl::GLuint handle;
		core::String logs;
		core::Map<core::String, UniformInfo> uniformsInfo;
		core::Map<UniformAddr, uint> samplers;
		internal::TextureBinding *bindings;
		mutable core::Map<core::String, BlockInfo> blocks;

};

}
}

#endif // N_GRAPHICS_SHADERCOMBINAISON

