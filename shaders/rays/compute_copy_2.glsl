#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(rgba32f, binding = 0) writeonly uniform image2D img_output;
layout(binding = 2) uniform sampler2D sky;
layout(std140, binding = 0) uniform Uniforms {
    mat4 view;
    mat4 projection;

    vec4 cameraPos;
    int amount_of_objs;
    float display_ratio;
    float pad;
    float pad2;
} uf;

struct Object {
    vec4 color;
    vec4 param1;
    vec4 param2;
    vec4 param3;
    int type;
};

layout(std430, binding = 0) buffer Objects {
    Object objs[];
};

layout(std430, binding = 4) buffer Debugger {
    float debugger[];
};

vec4 getSky(vec3 dir){
    return texture(sky,vec2(atan(dir.x,dir.z)*0.1591+0.5, dir.y*0.5+0.5));
}

const float INF = 1000.;

float intersection(vec3 camera, vec3 dir, Object obj){
    if (obj.type==0){
        vec3 m = camera-obj.param1.xyz;
        float a=dot(dir,dir);
        float b=2.*dot(dir,m);
        float c=dot(m,m)-obj.param1.w*obj.param1.w;
        
        float d=b*b-4*a*c;

        float half_reverse_a = 0.5/a;
        float root = sqrt(max(0.,d));

        float lambda_1 = (-b + root)*half_reverse_a;
        lambda_1 = (step(0.,-d)+step(0.,-lambda_1))*INF+step(0.,lambda_1)*lambda_1;
        float lambda_2 = (-b - root)*half_reverse_a;
        lambda_2 = (step(0.,-d)+step(0.,-lambda_2))*INF+step(0.,lambda_2)*lambda_2;

        //if (gl_GlobalInvocationID.x == 960 && gl_GlobalInvocationID.y == 540){
        //    debugger[0]=dir.z;
        //    debugger[1]=camera.z;
        //    debugger[2]=obj.param1.z;
        //    debugger[3]=b;
        //    debugger[4]=c;
        //    debugger[5]=d;
        //    debugger[6]=root;
        //}

        return min(lambda_1,lambda_2);

    }
    return -1.;
}

vec3 ray(vec3 camera, vec3 dir){ 
    float lambda=100.;
    int obj_of_intersection=-1;
    float is_better;
    for( int i=0; i<uf.amount_of_objs; i++){
        float new_lambda=intersection(camera,dir,objs[i]);
        float is_better = step(0.0, new_lambda) * step(new_lambda, lambda);
        lambda=mix(lambda, new_lambda, is_better);
        obj_of_intersection=(is_better>0.)? i:obj_of_intersection;
    }
    if (obj_of_intersection<0.){
        return (getSky(dir)).xyz;
    }
    else{
        return objs[obj_of_intersection].color.xyz*lambda;
    }
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 img_size = imageSize(img_output);

    if (pixel_coords.x >= img_size.x || pixel_coords.y >= img_size.y) return;

    vec2 uv = (vec2(pixel_coords) + vec2(0.5)) / vec2(img_size);
    uv = uv * 2.0 - vec2(1.0,1.0); 
    uv.x *= uf.display_ratio;
    
    vec3 color = ray(uf.cameraPos.xyz,(normalize(vec4(uv,1.,0.)) * uf.view).xyz);
    

    imageStore(img_output, pixel_coords, vec4(color.xyz, 1.));
}