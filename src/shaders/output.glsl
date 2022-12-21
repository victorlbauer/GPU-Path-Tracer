#version 430 core
#ifdef COMPILING_VERTEX_SHADER

const vec2 pos[6] = vec2[6] (
    vec2(-1.0, 1.0),
    vec2(-1.0,-1.0),
    vec2( 1.0,-1.0),
    vec2(-1.0, 1.0),
    vec2( 1.0,-1.0),
    vec2( 1.0, 1.0)
);

const vec2 uv[6] = vec2[6] (
	vec2(0.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(0.0, 1.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0)
);

out vec2 texCoords;

void main() {
    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0); 
    texCoords = uv[gl_VertexID];
}
#endif

#ifdef COMPILING_FRAGMENT_SHADER
out vec4 color;
in vec2 texCoords;

uniform sampler2D outputTex;

void main() { 
    color = texture(outputTex, texCoords);
}
#endif
