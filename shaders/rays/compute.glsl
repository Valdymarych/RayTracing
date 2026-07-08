#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(rgba32f, binding = 0) writeonly uniform image2D img_output;
layout(binding = 2) uniform sampler2D sky;

layout(std430, binding = 4) buffer Debugger {
    float debugger[];
};

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
struct Source {
    vec4 pos;   //pos.w - meta data "finished" if > 0
    vec4 dir;
    vec4 color;  // color.w - meta data "weight"
};
Source sources[5];
vec3 color_acum = vec3(0.,0.,0.);
int current_source_amount=1;

vec4 getSky(vec3 dir){
    return texture(sky,vec2(atan(dir.x,dir.z)*0.1591+0.5, dir.y*0.5+0.5));
}
const float INF = 1000.;
const float epsilon = .0001;

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
        return min(lambda_1,lambda_2);
    }
    if (obj.type==1){
        vec3 n = cross(obj.param2.xyz, obj.param3.xyz);
        float h = dot(obj.param1.xyz-camera,n);
        float lambda = h/dot(dir,n);
        return lambda;
    }
    if (obj.type==2){
        float h = dot(obj.param1.xyz-camera,obj.param2.xyz);
        float lambda = h/dot(dir,obj.param2.xyz);
        return lambda;
    }
    return -1.;
}

void try_to_add_source(vec4 pos,vec4 dir,vec4 color){
    if(current_source_amount>4){
        if (sources[4].color.w<color.w){
            sources[4].pos=pos;
            sources[4].dir=dir;
            sources[4].color=color;
        }
    } else{
        sources[current_source_amount].color=color;
        sources[current_source_amount].pos=pos;
        sources[current_source_amount].dir=dir;
        current_source_amount+=1;
    }
}

void post_hit(int source_id, vec3 point_of_intersection, int obj_of_intersection){
    Object obj = objs[obj_of_intersection];
    vec3 dir = sources[source_id].dir.xyz;
    vec3 pos = sources[source_id].pos.xyz;
    vec4 color = sources[source_id].color;
    vec3 normal;
    float refractive_index = 1.;
    if (obj.type==0){
        normal = normalize(point_of_intersection - obj.param1.xyz);
        refractive_index = obj.param2.x;
    }
    float movingIn = 1.-step(0., dot(dir,normal))*2.;
    
    vec3 tempNormal = movingIn * normal;   // always points in oppsite direction to dir
    float real_cos_angle = dot(dir,-tempNormal);
    float n1 = (movingIn>0.) ? 1.0 : refractive_index;
    float n2 = (movingIn>0.) ? refractive_index : 1.0;
    float eta = n1 / n2;

    float sin2Theta = eta * eta * (1.0 - real_cos_angle*real_cos_angle);
    float k_r;  // refrection_coeficient
    vec4 transparancy_color;
    float transparancy_lambda;
    vec3 transparancy_dir;

    if (sin2Theta>1.){
        k_r = 1.;  // full reflection
        sources[source_id].pos.xyz=point_of_intersection+tempNormal*epsilon;
        sources[source_id].dir.xyz=dir+tempNormal*2*real_cos_angle;

        if (source_id==0 && gl_GlobalInvocationID.x == 960 && gl_GlobalInvocationID.y == 540){
            debugger[0]=eta;
            debugger[1]=real_cos_angle;
            debugger[2]=sin2Theta;
        }

    } else {
            
        if (movingIn>0.){  // if moving inside the body

            // checking transparancy
            transparancy_dir = (dir+tempNormal*real_cos_angle)/eta - tempNormal*sqrt(1-sin2Theta);
            transparancy_lambda=intersection(point_of_intersection-epsilon*tempNormal,transparancy_dir,obj);
            transparancy_color = color * exp2(-obj.color*transparancy_lambda);
            // calculation of reflection koeficient (Frenell's one) 
            float r_0 = (refractive_index-1.)/(refractive_index+1.);
            r_0=r_0*r_0;
            k_r = r_0 + (1-r_0)*pow(1+dot(transparancy_dir,normal),5.);
            
            transparancy_color.w *= (1-k_r);
 
            if (k_r>0.6){
                try_to_add_source(vec4(point_of_intersection+(transparancy_lambda-epsilon)*transparancy_dir,0.),vec4(transparancy_dir,0.),transparancy_color);
                sources[source_id].pos.xyz=point_of_intersection+normal*epsilon;
                sources[source_id].dir.xyz=dir+tempNormal*2*real_cos_angle; 
                sources[source_id].color.w*=k_r;
            }
            else{
                try_to_add_source(vec4(point_of_intersection+normal*epsilon,0.),vec4(dir+tempNormal*2*real_cos_angle,0.),vec4(color.xyz,color.w*k_r));
                sources[source_id].pos.xyz=point_of_intersection+(transparancy_lambda-epsilon)*transparancy_dir;
                sources[source_id].dir.xyz=transparancy_dir;
                sources[source_id].color=transparancy_color;
            }
            
        } else {

            // checking transparancy
            transparancy_dir = dir+tempNormal*2*real_cos_angle;
            transparancy_lambda=intersection(point_of_intersection+epsilon*tempNormal,transparancy_dir,obj);
            transparancy_color = color * exp2(-obj.color*transparancy_lambda); 
            
            vec3 going_out_dir = (dir+tempNormal*real_cos_angle)/eta - tempNormal*sqrt(1-sin2Theta);
            
            float r_0 = (refractive_index-1.)/(refractive_index+1.);
            r_0=r_0*r_0;
            k_r = r_0 + (1-r_0)*pow(1-dot(going_out_dir,normal),5.);
            transparancy_color.w *= k_r;
            if (k_r>0.5){   // Ok, this ray mostly REFLECTS so I REFLECT IT and pass through the material to determine color loss
                try_to_add_source(vec4(point_of_intersection+normal*epsilon,0.),vec4(going_out_dir,0.),vec4(color.xyz,color.w*(1-k_r)));
                sources[source_id].pos.xyz=point_of_intersection+(transparancy_lambda-epsilon)*transparancy_dir;
                sources[source_id].dir.xyz=transparancy_dir;
                sources[source_id].color=transparancy_color;
            }else{
                try_to_add_source(vec4(point_of_intersection+(transparancy_lambda-epsilon)*transparancy_dir,0.),vec4(transparancy_dir,0.),transparancy_color);
                sources[source_id].pos.xyz=point_of_intersection+normal*epsilon;
                sources[source_id].dir.xyz=going_out_dir; 
                sources[source_id].color.w*=1-k_r;
            }
        }
    }
}

void propagate(int source_id){
    float lambda=100.;
    int obj_of_intersection=-1;
    float is_better;

    vec3 pos = sources[source_id].pos.xyz;
    vec3 dir = sources[source_id].dir.xyz;
    vec3 point_of_intersection;
    
    for( int i=0; i<uf.amount_of_objs; i++){
        float new_lambda=intersection(pos,dir,objs[i]);
        float is_better = step(0.0, new_lambda) * step(new_lambda, lambda);
        lambda=mix(lambda, new_lambda, is_better);
        obj_of_intersection=(is_better>0.)? i:obj_of_intersection;
    }
    if (obj_of_intersection<0.){
        sources[source_id].pos.w = 1.;
        color_acum+=sources[source_id].color.w*sources[source_id].color.xyz*(getSky(dir)).xyz;
    } else{
        point_of_intersection = pos+dir*lambda;
        post_hit(source_id, point_of_intersection, obj_of_intersection);
    }
}

void principal_ray(vec3 camera, vec3 dir){ 
    sources[0].pos=vec4(camera,0.);
    sources[0].dir=vec4(dir,0.);
    sources[0].color=vec4(1.,1.,1.,1.);
    #pragma unroll
    for (int i = 1; i < 5; i++) {
        sources[i].pos=vec4(0.);
        sources[i].dir=vec4(0.);
        sources[i].color=vec4(0.);
    }
    for (int bounce = 0; bounce < 12; bounce++) {
        #pragma unroll
        for (int i = 0; i < 5; i++) {
            if (sources[i].pos.w < 0.5) { // Якщо промінь активний
                propagate(i);
            }
        }
    }
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 img_size = imageSize(img_output);

    if (pixel_coords.x >= img_size.x || pixel_coords.y >= img_size.y) return;

    vec2 uv = (vec2(pixel_coords) + vec2(0.5)) / vec2(img_size);
    uv = uv * 2.0 - vec2(1.0,1.0); 
    uv.x *= uf.display_ratio;
    
    principal_ray(uf.cameraPos.xyz,(normalize(vec4(uv,1.,0.)) * uf.view).xyz);

    imageStore(img_output, pixel_coords, vec4(color_acum.xyz, 1.));
}