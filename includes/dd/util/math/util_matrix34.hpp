 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#pragma once

namespace dd::util::math {

    template<typename T> requires std::is_floating_point<T>::value
    class Matrix34RowMajorType {
        public:
            typedef T __attribute__((vector_size(sizeof(T) * 16))) m34t;
            union {
                struct {
                    Vector4Type<T> m_row1;
                    Vector4Type<T> m_row2;
                    Vector4Type<T> m_row3;
                };
                T m_arr[12];
                T m_arr2d[3][4];
            };
        public:
            constexpr Matrix34RowMajorType() : m_arr{} { /*...*/ }
            constexpr Matrix34RowMajorType(const m34t& vec) : m_row1(vec[0], vec[1], vec[2], vec[3]), m_row2(vec[4], vec[5], vec[6], vec[7]), m_row3(vec[8], vec[9], vec[10], vec[11]) {/*...*/}
            constexpr Matrix34RowMajorType(const Vector4Type<T>& row1, const Vector4Type<T>& row2, const Vector4Type<T>& row3) : m_row1(row1), m_row2(row2), m_row3(row3) {/*...*/}

            constexpr Matrix34RowMajorType(const Matrix34RowMajorType& rhs) : m_row1(rhs.m_row1), m_row2(rhs.m_row2), m_row3(rhs.m_row3) {/*...*/}

            constexpr ALWAYS_INLINE Matrix34RowMajorType& operator=(const Matrix34RowMajorType& rhs) {
                m_row1.m_vec = rhs.m_row1.m_vec;
                m_row2.m_vec = rhs.m_row2.m_vec;
                m_row3.m_vec = rhs.m_row3.m_vec;
                return *this;
            }
            
    };

    using Matrix34f = Matrix34RowMajorType<float>;

    static_assert(sizeof(Matrix34f) == sizeof(float) * 12);

    template<typename T>
    constexpr Matrix34RowMajorType<T> ZeroMatrix34 = {};

    template<typename T>
    constexpr Matrix34RowMajorType<T> IdentityMatrix34(Vector4f(1.0, 0.0, 0.0, 0.0), Vector4f(0.0, 1.0, 0.0, 0.0), Vector4f(0.0, 0.0, 1.0, 0.0));

    constexpr bool MakeVectorRotation(Matrix34f *out_rot_matrix, const Vector3f& align, const Vector3f& base) {
        const float dot = base.Dot(align) + 1.0f;

        if (dot <= FloatUlp) {
            *out_rot_matrix = ZeroMatrix34<float>;
            return false;
        }

        const float sqrt2dot = ::sqrt(dot + dot);
        const float k = 1.0f / sqrt2dot;

        const Vector3f kcross = base.Cross(align) * k;

        const float kcross2x               = kcross.x + kcross.x;
        const float kcross2y_kcrossy       = kcross.y * (kcross.y + kcross.y);
        const float kcross2z_kcrossz       = kcross.z * (kcross.z + kcross.z);
        const float kcross2y_kcrossz       = kcross.z * (kcross.y + kcross.y);
        const float oproj_kcross2x_kcrossx = 1.0f - (kcross.x * kcross2x);

        const Vector4f row1( (1.0f - kcross2y_kcrossy) - kcross2y_kcrossz,  (kcross.y * kcross2x) - (kcross.z * sqrt2dot), (kcross.y * sqrt2dot) + (kcross.z * kcross2x), 0.0f );
        const Vector4f row2( (kcross.z * sqrt2dot) + (kcross.y * kcross2x),  oproj_kcross2x_kcrossx - kcross2z_kcrossz,     kcross2z_kcrossz - (kcross.x * sqrt2dot),     0.0f );
        const Vector4f row3( (kcross.z * kcross2x) - (kcross.y - sqrt2dot), (kcross.x * sqrt2dot) + kcross2y_kcrossz,       oproj_kcross2x_kcrossx - kcross2y_kcrossy,    0.0f );

        const Matrix34f out(row1, row2, row3);

        *out_rot_matrix = out;

        return true;
    }

    constexpr void RotateLocalX(Matrix34f *out_rot_matrix, const float theta) {
        const float sin = SampleSin(theta * FloatSample);
        const float cos = SampleCos(theta * FloatSample);
        const float m12 = out_rot_matrix->m_arr2d[0][1];
        const float m22 = out_rot_matrix->m_arr2d[1][1];
        const float m32 = out_rot_matrix->m_arr2d[2][1];
        out_rot_matrix->m_arr2d[0][1] = (m12 * cos) + (out_rot_matrix->m_arr2d[0][2] * sin);
        out_rot_matrix->m_arr2d[0][2] = (out_rot_matrix->m_arr2d[0][2] * cos) - (m12 * sin);
        out_rot_matrix->m_arr2d[1][1] = (m22 * cos) + (out_rot_matrix->m_arr2d[1][2] * sin);
        out_rot_matrix->m_arr2d[1][2] = (out_rot_matrix->m_arr2d[1][2] * cos) - (m22 * sin);
        out_rot_matrix->m_arr2d[2][1] = (m32 * cos) + (out_rot_matrix->m_arr2d[2][2] * sin);
        out_rot_matrix->m_arr2d[2][2] = (out_rot_matrix->m_arr2d[2][2] * cos) - (m32 * sin);
    }

    constexpr void RotateLocalY(Matrix34f *out_rot_matrix, const float theta) {
        const float sin = SampleSin(theta * FloatSample);
        const float cos = SampleCos(theta * FloatSample);
        const float m11 = out_rot_matrix->m_arr2d[0][0];
        const float m21 = out_rot_matrix->m_arr2d[1][0];
        const float m31 = out_rot_matrix->m_arr2d[2][0];
        out_rot_matrix->m_arr2d[0][0] = (m11 * cos) - (out_rot_matrix->m_arr2d[0][2] * sin);
        out_rot_matrix->m_arr2d[0][2] = (out_rot_matrix->m_arr2d[0][2] * cos) + (m11 * sin);
        out_rot_matrix->m_arr2d[1][0] = (m21 * cos) - (out_rot_matrix->m_arr2d[1][2] * sin);
        out_rot_matrix->m_arr2d[1][2] = (out_rot_matrix->m_arr2d[1][2] * cos) + (m21 * sin);
        out_rot_matrix->m_arr2d[2][0] = (m31 * cos) - (out_rot_matrix->m_arr2d[2][2] * sin);
        out_rot_matrix->m_arr2d[2][2] = (out_rot_matrix->m_arr2d[2][2] * cos) + (m31 * sin);
    }

    constexpr void RotateLocalZ(Matrix34f *out_rot_matrix, const float theta) {
        const float sin = std::sin(theta);
        const float cos = std::cos(theta);
        const float m11 = out_rot_matrix->m_arr2d[0][0];
        const float m21 = out_rot_matrix->m_arr2d[1][0];
        const float m31 = out_rot_matrix->m_arr2d[2][0];
        out_rot_matrix->m_arr2d[0][0] = (m11 * cos)                           + (out_rot_matrix->m_arr2d[0][1] * sin);
        out_rot_matrix->m_arr2d[0][1] = (out_rot_matrix->m_arr2d[0][1] * cos) - (m11 * sin);
        out_rot_matrix->m_arr2d[1][0] = (m21 * cos)                           + (out_rot_matrix->m_arr2d[1][1] * sin);
        out_rot_matrix->m_arr2d[1][1] = (out_rot_matrix->m_arr2d[1][1] * cos) - (m21 * sin);
        out_rot_matrix->m_arr2d[2][0] = (m31 * cos)                           + (out_rot_matrix->m_arr2d[2][1] * sin);
        out_rot_matrix->m_arr2d[2][1] = (out_rot_matrix->m_arr2d[2][1] * cos) - (m31 * sin);
    }
}