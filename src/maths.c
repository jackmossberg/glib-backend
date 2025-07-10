#include "maths.h"

#define _FL "maths.c" 

#define APIC static

float degs_to_rads(float degrees) {
    return degrees * (PI/180);
}

float rads_to_degs(float rads) {
    return rads * (180/PI);
}

void mapi_Vec2Add(float* v1, float* v2, float* result) {
    if (!v1 || !v2 || !*v1 || !*v2)
        return;
    
    result[0] = v1[0] + v2[0];
    result[1] = v1[1] + v2[1];
}

void mapi_Vec3Add(float* v1, float* v2, float* result) {
    if (!v1 || !v2 || !*v1 || !*v2)
        return;
    
    result[0] = v1[0] + v2[0];
    result[1] = v1[1] + v2[1];
    result[2] = v1[2] + v2[2];
}

void mapi_Vec4Add(float* v1, float* v2, float* result) {
    if (!v1 || !v2 || !*v1 || !*v2)
        return;
    
    result[0] = v1[0] + v2[0];
    result[1] = v1[1] + v2[1];
    result[2] = v1[2] + v2[2];
    result[3] = v1[3] + v2[3];
}

void mapi_Matrix3x3Fill(float* matrix3x3, float val) {
    if (!matrix3x3)
        return;

    for(int i = 0; i < (3 * 3); i++) {
        matrix3x3[i] = val;
    }
}

void mapi_Matrix4x4Fill(float* matrix4x4, float val) {
    if (!matrix4x4)
        return;

    for(int i = 0; i < (4 * 4); i++) {
        matrix4x4[i] = val;
    }
}

void mapi_Matrix5x5Fill(float* matrix5x5, float val) {
    if (!matrix5x5)
        return;

    for(int i = 0; i < (5 * 5); i++) {
        matrix5x5[i] = val;
    }
}

void mapi_MultiMatrix4x4(float* m1, float* m2, float* result) {
    matrix4x4 temp;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            temp[i + j*4] = 0;
            for (int k = 0; k < 4; k++)
                temp[i + j*4] += m1[i + k*4] * m2[k + j*4];
        }
    memcpy(result, temp, 16 * sizeof(float));
}

void mapi_TransformMatrix4x4(float* m4, float* position, float* rotation, float* scale) {
    if (!m4 || !position || !rotation || !scale)
        return;

    matrix4x4 temp = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

    temp[0] = scale[0];
    temp[5] = scale[1];
    temp[10] = scale[2];

    matrix4x4 rotX = {1,0,0,0, 0,cosf(degs_to_rads(rotation[0])),-sinf(degs_to_rads(rotation[0])),0,
                      0,sinf(degs_to_rads(rotation[0])),cosf(degs_to_rads(rotation[0])),0, 0,0,0,1};
    matrix4x4 rotY = {cosf(degs_to_rads(rotation[1])),0,sinf(degs_to_rads(rotation[1])),0, 0,1,0,0,
                      -sinf(degs_to_rads(rotation[1])),0,cosf(degs_to_rads(rotation[1])),0, 0,0,0,1};
    matrix4x4 rotZ = {cosf(degs_to_rads(rotation[2])),-sinf(degs_to_rads(rotation[2])),0,0,
                      sinf(degs_to_rads(rotation[2])),cosf(degs_to_rads(rotation[2])),0,0, 0,0,1,0, 0,0,0,1};

    matrix4x4 tempRot;
    mapi_MultiMatrix4x4(rotZ, rotY, tempRot);
    mapi_MultiMatrix4x4(tempRot, rotX, tempRot);

    matrix4x4 trans = {1,0,0,0, 0,1,0,0, 0,0,1,0, position[0],position[1],position[2],1};

    mapi_MultiMatrix4x4(temp, tempRot, temp);
    mapi_MultiMatrix4x4(trans, temp, m4);
}

void mapi_ViewMatrix4x4(float* m4, float* position, float* rotation) {
    if (!m4 || !position || !rotation)
        return;

    matrix4x4 temp = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

    matrix4x4 rotX = {1,0,0,0, 0,cosf(degs_to_rads(-rotation[0])),-sinf(degs_to_rads(-rotation[0])),0,
                      0,sinf(degs_to_rads(-rotation[0])),cosf(degs_to_rads(-rotation[0])),0, 0,0,0,1};
    matrix4x4 rotY = {cosf(degs_to_rads(-rotation[1])),0,sinf(degs_to_rads(-rotation[1])),0, 0,1,0,0,
                      -sinf(degs_to_rads(-rotation[1])),0,cosf(degs_to_rads(-rotation[1])),0, 0,0,0,1};
    matrix4x4 rotZ = {cosf(degs_to_rads(-rotation[2])),-sinf(degs_to_rads(-rotation[2])),0,0,
                      sinf(degs_to_rads(-rotation[2])),cosf(degs_to_rads(-rotation[2])),0,0, 0,0,1,0, 0,0,0,1};

    matrix4x4 tempRot;
    mapi_MultiMatrix4x4(rotZ, rotY, tempRot);
    mapi_MultiMatrix4x4(tempRot, rotX, tempRot);

    matrix4x4 trans = {1,0,0,0, 0,1,0,0, 0,0,1,0, position[0],-position[1],position[2],1};

    mapi_MultiMatrix4x4(tempRot, trans, m4);
}

void mapi_ProjectionMatrix4x4(float* m4, float fovy, float width, float height) {
    if (!m4)
        return;

    float aspect = width / height;
    float near = 0.01f, far = 1500.0f;
    float f = 1.0f / tanf(degs_to_rads(fovy * 0.5f));

    matrix4x4 proj = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (far + near) / (near - far), -1,
        0, 0, (2 * far * near) / (near - far), 0
    };
    memcpy(m4, proj, 16 * sizeof(float));
}