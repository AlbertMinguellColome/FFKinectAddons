varying vec3  texcoord;

void main(void) 
{
    vec4 position = gl_Vertex;
    vec3 normal = gl_Normal;
    float l = sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z );
    position = position + normal * 5 / l;
 	vec4	texcoord0 = gl_ModelViewMatrix * gl_Vertex;
	//texcoord = texcoord0.xyz;
	texcoord = normalize(gl_Vertex.xyz);

    gl_Position    =  gl_ProjectionMatrix * gl_ModelViewMatrix * position;
   
}
