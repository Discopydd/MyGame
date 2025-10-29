#include "MyMath.h"
namespace Math {
    Matrix4x4 Math::MakeScaleMatrix(const Vector3& scale) {
        Matrix4x4 result;

        result.m[0][0] = scale.x;
        result.m[0][1] = 0.0f;
        result.m[0][2] = 0.0f;
        result.m[0][3] = 0.0f;

        result.m[1][0] = 0.0f;
        result.m[1][1] = scale.y;
        result.m[1][2] = 0.0f;
        result.m[1][3] = 0.0f;

        result.m[2][0] = 0.0f;
        result.m[2][1] = 0.0f;
        result.m[2][2] = scale.z;
        result.m[2][3] = 0.0f;

        result.m[3][0] = 0.0f;
        result.m[3][1] = 0.0f;
        result.m[3][2] = 0.0f;
        result.m[3][3] = 1.0f;

        return result;
    }

    Matrix4x4 Math::MakeRotateZMatrix(float radian) {

        Matrix4x4 result;
        result.m[0][0] = cosf(radian);
        result.m[0][1] = sinf(radian);
        result.m[0][2] = 0.0f;
        result.m[0][3] = 0.0f;
        result.m[1][0] = -sinf(radian);
        result.m[1][1] = cosf(radian);
        result.m[1][2] = 0.0f;
        result.m[1][3] = 0.0f;
        result.m[2][1] = 0.0f;
        result.m[2][2] = 1.0f;
        result.m[2][3] = 0.0f;
        result.m[3][0] = 0.0f;
        result.m[3][1] = 0.0f;
        result.m[3][2] = 0.0f;
        result.m[3][3] = 1.0f;

        return result;
    }

    Matrix4x4 Math::MakeTranslateMatrix(const Vector3& translate) {
        Matrix4x4 matrix;

        matrix.m[0][0] = 1.0f;
        matrix.m[0][1] = 0.0f;
        matrix.m[0][2] = 0.0f;
        matrix.m[0][3] = 0.0f;

        matrix.m[1][0] = 0.0f;
        matrix.m[1][1] = 1.0f;
        matrix.m[1][2] = 0.0f;
        matrix.m[1][3] = 0.0f;

        matrix.m[2][0] = 0.0f;
        matrix.m[2][1] = 0.0f;
        matrix.m[2][2] = 1.0f;
        matrix.m[2][3] = 0.0f;

        matrix.m[3][0] = translate.x;
        matrix.m[3][1] = translate.y;
        matrix.m[3][2] = translate.z;
        matrix.m[3][3] = 1.0f;

        return matrix;
    }


    Matrix4x4 Math::MakeRotateYMatrix(float radian) {

        Matrix4x4 result;
        result.m[0][0] = cosf(radian);
        result.m[0][1] = 0.0f;
        result.m[0][2] = -sinf(radian);
        result.m[0][3] = 0.0f;
        result.m[1][0] = 0.0f;
        result.m[1][1] = 1.0f;
        result.m[1][2] = 0.0f;
        result.m[1][3] = 0.0f;
        result.m[2][0] = sinf(radian);
        result.m[2][1] = 0.0f;
        result.m[2][2] = cosf(radian);
        result.m[2][3] = 0.0f;
        result.m[3][0] = 0.0f;
        result.m[3][1] = 0.0f;
        result.m[3][2] = 0.0f;
        result.m[3][3] = 1.0f;

        return result;
    }

    Matrix4x4 Math::MakeRotateXMatrix(float radian) {
        Matrix4x4 result;
        float c = float(cos(radian));
        float s = float(sin(radian));

        result.m[0][0] = 1.0f;
        result.m[0][1] = 0.0f;
        result.m[0][2] = 0.0f;
        result.m[0][3] = 0.0f;

        result.m[1][0] = 0.0f;
        result.m[1][1] = c;
        result.m[1][2] = s;
        result.m[1][3] = 0.0f;

        result.m[2][0] = 0.0f;
        result.m[2][1] = -s;
        result.m[2][2] = c;
        result.m[2][3] = 0.0f;

        result.m[3][0] = 0.0f;
        result.m[3][1] = 0.0f;
        result.m[3][2] = 0.0f;
        result.m[3][3] = 1.0f;

        return result;
    }
    Matrix4x4 Math::Multiply(const Matrix4x4& matrix1, const Matrix4x4& matrix2) {
        Matrix4x4 result = {};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                for (int k = 0; k < 4; ++k) {
                    result.m[i][j] += matrix1.m[i][k] * matrix2.m[k][j];
                }
            }
        }
        return result;
    }

    Matrix4x4 Math::MakeAffineMatrix(const Vector3& scale, const Vector3& rotation, const Vector3& translation) {
        // Scale
        Matrix4x4 Scale = { 0 };
        Scale.m[0][0] = scale.x;
        Scale.m[1][1] = scale.y;
        Scale.m[2][2] = scale.z;
        Scale.m[3][3] = 1;
        // Rotation
        Matrix4x4 RotationZ = { 0 };
        RotationZ.m[0][0] = cosf(rotation.z);
        RotationZ.m[0][1] = sinf(rotation.z);
        RotationZ.m[1][0] = -sinf(rotation.z);
        RotationZ.m[1][1] = cosf(rotation.z);
        RotationZ.m[2][2] = RotationZ.m[3][3] = 1;
        Matrix4x4 RotationX = { 0 };
        RotationX.m[1][1] = cosf(rotation.x);
        RotationX.m[1][2] = sinf(rotation.x);
        RotationX.m[2][1] = -sinf(rotation.x);
        RotationX.m[2][2] = cosf(rotation.x);
        RotationX.m[0][0] = RotationX.m[3][3] = 1;
        Matrix4x4 RotationY = { 0 };
        RotationY.m[0][0] = cosf(rotation.y);
        RotationY.m[2][0] = sinf(rotation.y);
        RotationY.m[0][2] = -sinf(rotation.y);
        RotationY.m[2][2] = cosf(rotation.y);
        RotationY.m[1][1] = RotationY.m[3][3] = 1;
        Matrix4x4 Rotation = Multiply(RotationX, Multiply(RotationY, RotationZ));
        // Translation
        Matrix4x4 Translation = { 0 };
        Translation.m[0][0] = Translation.m[1][1] = Translation.m[2][2] = Translation.m[3][3] = 1;
        Translation.m[3][0] = translation.x;
        Translation.m[3][1] = translation.y;
        Translation.m[3][2] = translation.z;

        return Multiply(Scale, Multiply(Rotation, Translation));
    }

    Matrix4x4 Math::Inverse(const Matrix4x4& m) {
        float determinant =
            +m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3]
            + m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1]
            + m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]
            - m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1]
            - m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]
            - m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]
            - m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3]
            - m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1]
            - m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]
            + m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1]
            + m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3]
            + m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]
            + m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3]
            + m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1]
            + m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]
            - m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1]
            - m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3]
            - m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]
            - m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0]
            - m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0]
            - m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]
            + m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0]
            + m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0]
            + m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

        Matrix4x4 result = {};
        float recpDeterminant = 1.0f / determinant;

        result.m[0][0] = (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] +
            m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][3] * m.m[2][2] * m.m[3][1] -
            m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
        result.m[0][1] = (-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] -
            m.m[0][3] * m.m[2][1] * m.m[3][2] + m.m[0][3] * m.m[2][2] * m.m[3][1] +
            m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
        result.m[0][2] = (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] +
            m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][3] * m.m[1][2] * m.m[3][1] -
            m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]) * recpDeterminant;
        result.m[0][3] = (-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] -
            m.m[0][3] * m.m[1][1] * m.m[2][2] + m.m[0][3] * m.m[1][2] * m.m[2][1] +
            m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]) * recpDeterminant;

        result.m[1][0] = (-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] -
            m.m[1][3] * m.m[2][0] * m.m[3][2] + m.m[1][3] * m.m[2][2] * m.m[3][0] +
            m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
        result.m[1][1] = (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] +
            m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][3] * m.m[2][2] * m.m[3][0] -
            m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
        result.m[1][2] = (-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] -
            m.m[0][3] * m.m[1][0] * m.m[3][2] + m.m[0][3] * m.m[1][2] * m.m[3][0] +
            m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]) * recpDeterminant;
        result.m[1][3] = (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] +
            m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][3] * m.m[1][2] * m.m[2][0] -
            m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]) * recpDeterminant;

        result.m[2][0] = (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] +
            m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][3] * m.m[2][1] * m.m[3][0] -
            m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]) * recpDeterminant;
        result.m[2][1] = (-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] -
            m.m[0][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[2][1] * m.m[3][0] +
            m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]) * recpDeterminant;
        result.m[2][2] = (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] +
            m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][3] * m.m[1][1] * m.m[3][0] -
            m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]) * recpDeterminant;
        result.m[2][3] = (-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] -
            m.m[0][3] * m.m[1][0] * m.m[2][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] +
            m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]) * recpDeterminant;

        result.m[3][0] = (-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] -
            m.m[1][2] * m.m[2][0] * m.m[3][1] + m.m[1][2] * m.m[2][1] * m.m[3][0] +
            m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]) * recpDeterminant;
        result.m[3][1] = (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] +
            m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[2][1] * m.m[3][0] -
            m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]) * recpDeterminant;
        result.m[3][2] = (-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] -
            m.m[0][2] * m.m[1][0] * m.m[3][1] + m.m[0][2] * m.m[1][1] * m.m[3][0] +
            m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]) * recpDeterminant;
        result.m[3][3] = (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] +
            m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] -
            m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]) * recpDeterminant;

        return result;
    }

    Vector3 Math::Transform(const Vector3& vector, const Matrix4x4& matrix) {
        Vector3 result;
        result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + matrix.m[3][0];
        result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + matrix.m[3][1];
        result.z = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + matrix.m[3][2];
        float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + matrix.m[3][3];

        assert(w != 0.0f);

        result.x /= w;
        result.y /= w;
        result.z /= w;

        return result;
    }
    Vector3 Math::TransformCoordLocal(const Vector3& v, const Matrix4x4& m)
    {
        // 当作 (x,y,z,1) * m
        float x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + 1.0f * m.m[3][0];
        float y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + 1.0f * m.m[3][1];
        float z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + 1.0f * m.m[3][2];
        float w = v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + 1.0f * m.m[3][3];
        if (w != 0.0f) { x /= w; y /= w; z /= w; }
        return { x, y, z };
    }

    Matrix4x4 Math::MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
        Matrix4x4 perspectiveMatrix;
        perspectiveMatrix.m[0][0] = 1.0f / float((aspectRatio * tan(fovY / 2.0f)));
        perspectiveMatrix.m[0][1] = 0;
        perspectiveMatrix.m[0][2] = 0;
        perspectiveMatrix.m[0][3] = 0;
        perspectiveMatrix.m[1][0] = 0;
        perspectiveMatrix.m[1][1] = 1.0f / float(tan(fovY / 2.0f));
        perspectiveMatrix.m[1][2] = 0;
        perspectiveMatrix.m[1][3] = 0;
        perspectiveMatrix.m[2][0] = 0;
        perspectiveMatrix.m[2][1] = 0;
        perspectiveMatrix.m[2][2] = farClip / (farClip - nearClip);
        perspectiveMatrix.m[2][3] = 1;
        perspectiveMatrix.m[3][0] = 0;
        perspectiveMatrix.m[3][1] = 0;
        perspectiveMatrix.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
        perspectiveMatrix.m[3][3] = 0;
        return perspectiveMatrix;
    }

    Matrix4x4 Math::MakeOrthographicMatrix(float left, float right, float top, float bottom, float nearClip, float farClip) {
        Matrix4x4 result;


        result.m[0][0] = 2.0f / (right - left);
        result.m[0][1] = 0.0f;
        result.m[0][2] = 0.0f;
        result.m[0][3] = 0.0f;

        result.m[1][0] = 0.0f;
        result.m[1][1] = 2.0f / (top - bottom);
        result.m[1][2] = 0.0f;
        result.m[1][3] = 0.0f;

        result.m[2][0] = 0.0f;
        result.m[2][1] = 0.0f;
        result.m[2][2] = 1.0f / (farClip - nearClip);
        result.m[2][3] = 0.0f;

        result.m[3][0] = (right + left) / (left - right);
        result.m[3][1] = (top + bottom) / (bottom - top);
        result.m[3][2] = nearClip / (nearClip - farClip);
        result.m[3][3] = 1.0f;

        return result;
    }

    Matrix4x4 Math::MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
        Matrix4x4 result;

        result.m[0][0] = width / 2.0f;
        result.m[0][1] = 0.0f;
        result.m[0][2] = 0.0f;
        result.m[0][3] = 0.0f;

        result.m[1][0] = 0.0f;
        result.m[1][1] = -height / 2.0f;
        result.m[1][2] = 0.0f;
        result.m[1][3] = 0.0f;

        result.m[2][0] = 0.0f;
        result.m[2][1] = 0.0f;
        result.m[2][2] = maxDepth - minDepth;
        result.m[2][3] = 0.0f;

        result.m[3][0] = left + width / 2.0f;
        result.m[3][1] = top + height / 2.0f;
        result.m[3][2] = minDepth;
        result.m[3][3] = 1.0f;

        return result;
    }

    Matrix4x4 Math::MakeIdentity4x4() {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
        return result;
    }


    float Math::Length(const Vector3& vec) {
        return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    }

    Vector3 Math::Normalize(const Vector3& vec) {
        float length = Length(vec);
        if (length != 0) {
            return { vec.x / length, vec.y / length, vec.z / length };
        }
        return { 0, 0, 0 };
    }

    Vector3 Math::Add(const Vector3& v1, const Vector3& v2) {
        return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
    }

    Vector3 Math::Subtract(const Vector3& v1, const Vector3& v2) {
        return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
    }
    Vector3 Multiply(const Vector3& vec, float scalar)
    {
        return { vec.x * scalar, vec.y * scalar, vec.z * scalar };
    }
    float Math::Lerp(float a, float b, float t) {
        return a * (1.0f - t) + b * t;
    }
    Vector3 Math::Lerp(const Vector3& a, const Vector3& b, float t) {
        return { a.x * (1.0f - t) + b.x * t,
                 a.y * (1.0f - t) + b.y * t,
                 a.z * (1.0f - t) + b.z * t };
    }
    Vector4 Math::Lerp(const Vector4& a, const Vector4& b, float t) {
        return { a.x * (1.0f - t) + b.x * t,
                 a.y * (1.0f - t) + b.y * t,
                 a.z * (1.0f - t) + b.z * t,
                 a.w * (1.0f - t) + b.w * t };
    }
    float Math::ToRadian(float degrees) {
        // 使用你头文件里已定义的 PI 常量
        return degrees * (PI / 180.0f);
    }

    Quaternion Math::MakeAxisAngleQuaternion(const Vector3& axis, float angleRad) {
        // 允许 axis 非单位向量；零向量则返回单位四元数
        float len = Math::Length(axis);
        if (len <= 1e-8f) {
            return Quaternion{ 0.0f, 0.0f, 0.0f, 1.0f }; // Identity (x,y,z,w)
        }

        Vector3 n = Math::Normalize(axis);
        float half = 0.5f * angleRad;
        float s = sinf(half);
        float c = cosf(half);

        // 返回 (x, y, z, w) —— 向量部 = 轴 * sin(θ/2)，标量部 = cos(θ/2)
        return Quaternion{ n.x * s, n.y * s, n.z * s, c };
    }
    // ------------- 工具：长度与单位化（四元数为 x,y,z,w） -------------
    static inline float QDot(const Quaternion& q1, const Quaternion& q2) {
        return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
    }
    static inline float QLen(const Quaternion& q) {
        return std::sqrt(QDot(q, q));
    }
    static inline Quaternion QNormalize(const Quaternion& q) {
        float l = QLen(q);
        if (l <= 1e-8f) return Quaternion{ 0,0,0,1 }; // identity
        float inv = 1.0f / l;
        return Quaternion{ q.x * inv, q.y * inv, q.z * inv, q.w * inv };
    }

    // 线性插值（单位化后的 Lerp = Nlerp）
    Quaternion Math::Lerp(const Quaternion& a, const Quaternion& b, float t, bool shortestPath) {
        // 走最短弧：若点积为负，则取 -b
        Quaternion bb = b;
        float d = QDot(a, b);
        if (shortestPath && d < 0.0f) {
            bb = Quaternion{ -b.x, -b.y, -b.z, -b.w };
        }
        // 线性插值并单位化
        Quaternion q{
            a.x * (1.0f - t) + bb.x * t,
            a.y * (1.0f - t) + bb.y * t,
            a.z * (1.0f - t) + bb.z * t,
            a.w * (1.0f - t) + bb.w * t
        };
        return QNormalize(q);
    }

    // 球面线性插值（等角速度）
    Quaternion Math::Slerp(const Quaternion& a, const Quaternion& b, float t, float eps) {
        float d = QDot(a, b);
        Quaternion bb = b;
        // 最短弧
        if (d < 0.0f) {
            d = -d;
            bb = Quaternion{ -b.x, -b.y, -b.z, -b.w };
        }

        // 非常接近时退化为 Nlerp（避免除以很小的数）
        if (1.0f - d < eps) {
            return Lerp(a, bb, t, /*shortestPath=*/false);
        }

        // 标准 Slerp
        d = std::clamp(d, -1.0f, 1.0f);
        float theta = std::acos(d);
        float s0 = std::sinf((1.0f - t) * theta);
        float s1 = std::sinf(t * theta);
        float invSin = 1.0f / std::sinf(theta);

        Quaternion q{
            (a.x * s0 + bb.x * s1) * invSin,
            (a.y * s0 + bb.y * s1) * invSin,
            (a.z * s0 + bb.z * s1) * invSin,
            (a.w * s0 + bb.w * s1) * invSin
        };
        return QNormalize(q);
    }
    static inline float Clampf(float v, float lo, float hi) {
        return (v < lo) ? lo : (v > hi) ? hi : v;
    }

    // 约定：Quaternion 按 (x,y,z,w) 存储
    Vector3 Math::QuaternionToEuler(const Quaternion& q)
    {
        const float x = q.x, y = q.y, z = q.z, w = q.w;

        // q -> 3x3 旋转矩阵（行主序）
        const float xx = x + x, yy = y + y, zz = z + z;
        const float xy = x * yy, xz = x * zz, yz = y * zz;
        const float wx = w * xx, wy = w * yy, wz = w * zz;
        const float xx2 = x * xx, yy2 = y * yy, zz2 = z * zz;

        float m00 = 1.0f - (yy2 + zz2);
        float m01 = xy + wz;
        float m02 = xz - wy;

        float m10 = xy - wz;
        float m11 = 1.0f - (xx2 + zz2);
        float m12 = yz + wx;

        float m20 = xz + wy;
        float m21 = yz - wx;
        float m22 = 1.0f - (xx2 + yy2);

        // YXZ 分解得到 (yawY, pitchX, rollZ)
        // pitchX = asin(clamp(-m21, -1, 1))
        float sy = Clampf(-m21, -1.0f, 1.0f);
        float pitchX = std::asin(sy);

        float yawY, rollZ;
        if (std::fabs(sy) < 0.999999f) {
            // 常规情况
            yawY = std::atan2(m20, m22);
            rollZ = std::atan2(m01, m11);
        }
        else {
            // 近奇异：合并一个角
            yawY = 0.0f;
            rollZ = std::atan2(-m10, m00);
        }
        return { yawY, pitchX, rollZ };
    }

    // （可选）度数版
    Vector3 Math::QuaternionToEulerDeg(const Quaternion& q)
    {
        Vector3 r = QuaternionToEuler(q);
        const float rad2deg = 180.0f / PI; // PI 常量在 MyMath.h 已定义
        return { r.x * rad2deg, r.y * rad2deg, r.z * rad2deg };
    }
}
