uniform float iGlobalTime;
uniform vec2 iResolution;
uniform sampler2D iChannel0;
 
void main() {
     gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}