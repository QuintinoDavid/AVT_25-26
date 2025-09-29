
/** ----------------------------------------------------------
 *AVT Math Lib
 *
 * Definition of the Graphics Math Utility - gmu Class
 *
 * ALL resulting matrices are in COLUMN ORDER
 *
 // The code comes with no warranties, use it at your own risk.
 // You may use it, or parts of it, wherever you want.
 //
 // Author: João Madeiras Pereira
 //
 *
 ---------------------------------------------------------------*/

#include "mathUtility.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// glPushMatrix implementation
void gmu::pushMatrix(MatrixTypes aType)
{
	float *aux = (float *)malloc(sizeof(float) * 16);
	memcpy(aux, mMatrix[aType], sizeof(float) * 16);
	mMatrixStack[aType].push_back(aux);
}

// glPopMatrix implementation
void gmu::popMatrix(MatrixTypes aType)
{
	float *m = mMatrixStack[aType][mMatrixStack[aType].size() - 1];
	memcpy(mMatrix[aType], m, sizeof(float) * 16);
	mMatrixStack[aType].pop_back();
	free(m);
}

// glLoadIdentity implementation
void gmu::loadIdentity(MatrixTypes aType)
{
	setIdentityMatrix(mMatrix[aType]);
}

// glMultMatrix implementation
void gmu::multMatrix(MatrixTypes aType, float *aMatrix)
{
	float *a, *b, res[16];
	a = mMatrix[aType];
	b = aMatrix;

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			res[j * 4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k)
			{
				res[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
			}
		}
	}

	memcpy(mMatrix[aType], res, 16 * sizeof(float));
}

// aux function resMat = resMat * aMatrix
void gmu::multMatrix(float *resMat, float *aMatrix)
{
	float *a, *b, res[16];
	a = resMat;
	b = aMatrix;

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			res[j * 4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k)
			{
				res[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
			}
		}
	}

	memcpy(a, res, 16 * sizeof(float));
}

// glLoadMatrix implementation
void gmu::loadMatrix(MatrixTypes aType, float *aMatrix)
{
	memcpy(mMatrix[aType], aMatrix, 16 * sizeof(float));
}

// glTranslate implementation with matrix selection
void gmu::translate(MatrixTypes aType, float x, float y, float z)
{
	float mat[16];

	setIdentityMatrix(mat);
	mat[12] = x;
	mat[13] = y;
	mat[14] = z;

	multMatrix(aType, mat);
}

// glScale implementation with matrix selection
void gmu::scale(MatrixTypes aType, float x, float y, float z)
{
	float mat[16];

	setIdentityMatrix(mat, 4);
	mat[0] = x;
	mat[5] = y;
	mat[10] = z;

	multMatrix(aType, mat);
}

// glRotate implementation with matrix selection
void gmu::rotate(MatrixTypes aType, float angle, float x, float y, float z)
{
	float mat[16];
	float v[3];

	v[0] = x;
	v[1] = y;
	v[2] = z;

	float radAngle = DegToRad(angle);
	float co = cos(radAngle);
	float si = sin(radAngle);
	normalize(v);
	float x2 = v[0] * v[0];
	float y2 = v[1] * v[1];
	float z2 = v[2] * v[2];

	//	mat[0] = x2 + (y2 + z2) * co;
	mat[0] = co + x2 * (1 - co); // + (y2 + z2) * co;
	mat[4] = v[0] * v[1] * (1 - co) - v[2] * si;
	mat[8] = v[0] * v[2] * (1 - co) + v[1] * si;
	mat[12] = 0.0f;

	mat[1] = v[0] * v[1] * (1 - co) + v[2] * si;
	//	mat[5] = y2 + (x2 + z2) * co;
	mat[5] = co + y2 * (1 - co);
	mat[9] = v[1] * v[2] * (1 - co) - v[0] * si;
	mat[13] = 0.0f;

	mat[2] = v[0] * v[2] * (1 - co) - v[1] * si;
	mat[6] = v[1] * v[2] * (1 - co) + v[0] * si;
	//	mat[10]= z2 + (x2 + y2) * co;
	mat[10] = co + z2 * (1 - co);
	mat[14] = 0.0f;

	mat[3] = 0.0f;
	mat[7] = 0.0f;
	mat[11] = 0.0f;
	mat[15] = 1.0f;

	multMatrix(aType, mat);
}

// gluLookAt implementation
void gmu::lookAt(float xPos, float yPos, float zPos,
				 float xLook, float yLook, float zLook,
				 float xUp, float yUp, float zUp)
{
	float dir[3], right[3], up[3];

	up[0] = xUp;
	up[1] = yUp;
	up[2] = zUp;

	dir[0] = (xLook - xPos);
	dir[1] = (yLook - yPos);
	dir[2] = (zLook - zPos);
	normalize(dir);

	crossProduct(dir, up, right);
	normalize(right);

	crossProduct(right, dir, up);
	normalize(up);

	float m1[16], m2[16];

	m1[0] = right[0];
	m1[4] = right[1];
	m1[8] = right[2];
	m1[12] = 0.0f;

	m1[1] = up[0];
	m1[5] = up[1];
	m1[9] = up[2];
	m1[13] = 0.0f;

	m1[2] = -dir[0];
	m1[6] = -dir[1];
	m1[10] = -dir[2];
	m1[14] = 0.0f;

	m1[3] = 0.0f;
	m1[7] = 0.0f;
	m1[11] = 0.0f;
	m1[15] = 1.0f;

	setIdentityMatrix(m2, 4);
	m2[12] = -xPos;
	m2[13] = -yPos;
	m2[14] = -zPos;

	multMatrix(MatrixTypes::VIEW, m1);
	multMatrix(MatrixTypes::VIEW, m2);
}

// gluPerspective implementation
void gmu::perspective(float fov, float ratio, float nearp, float farp)
{
	float projMatrix[16];

	float f = 1.0f / tan(fov * (PI_F / 360.0f));

	setIdentityMatrix(projMatrix, 4);

	projMatrix[0] = f / ratio;
	projMatrix[1 * 4 + 1] = f;
	projMatrix[2 * 4 + 2] = (farp + nearp) / (nearp - farp);
	projMatrix[3 * 4 + 2] = (2.0f * farp * nearp) / (nearp - farp);
	projMatrix[2 * 4 + 3] = -1.0f;
	projMatrix[3 * 4 + 3] = 0.0f;

	multMatrix(MatrixTypes::PROJECTION, projMatrix);
}

// glOrtho implementation
void gmu::ortho(float left, float right,
				float bottom, float top,
				float nearp, float farp)
{
	float m[16];

	setIdentityMatrix(m, 4);

	m[0 * 4 + 0] = 2 / (right - left);
	m[1 * 4 + 1] = 2 / (top - bottom);
	m[2 * 4 + 2] = -2 / (farp - nearp);
	m[3 * 4 + 0] = -(right + left) / (right - left);
	m[3 * 4 + 1] = -(top + bottom) / (top - bottom);
	m[3 * 4 + 2] = -(farp + nearp) / (farp - nearp);

	multMatrix(MatrixTypes::PROJECTION, m);
}

// glFrustum implementation
void gmu::frustum(float left, float right,
				  float bottom, float top,
				  float nearp, float farp)
{
	float m[16];

	setIdentityMatrix(m, 4);

	m[0 * 4 + 0] = 2 * nearp / (right - left);
	m[1 * 4 + 1] = 2 * nearp / (top - bottom);
	m[2 * 4 + 0] = (right + left) / (right - left);
	m[2 * 4 + 1] = (top + bottom) / (top - bottom);
	m[2 * 4 + 2] = -(farp + nearp) / (farp - nearp);
	m[2 * 4 + 3] = -1.0f;
	m[3 * 4 + 2] = -2 * farp * nearp / (farp - nearp);
	m[3 * 4 + 3] = 0.0f;

	multMatrix(MatrixTypes::PROJECTION, m);
}

// sets the square matrix mat to the identity matrix,
// size refers to the number of rows (or columns)
void gmu::setIdentityMatrix(float *mat, int size)
{
	// fill matrix with 0s
	for (int i = 0; i < size * size; ++i)
		mat[i] = 0.0f;

	// fill diagonal with 1s
	for (int i = 0; i < size; ++i)
		mat[i + i * size] = 1.0f;
}

// Compute res = M * point
void gmu::multMatrixPoint(MatrixTypes aType, float *point, float *res)
{

	for (int i = 0; i < 4; ++i)
	{

		res[i] = 0.0f;

		for (int j = 0; j < 4; j++)
		{

			res[i] += point[j] * mMatrix[aType][j * 4 + i];
		}
	}
}

void gmu::multMatrixPoint(ComputedMatrixTypes aType, float *point, float *res)
{

	for (int i = 0; i < 4; ++i)
	{

		res[i] = 0.0f;

		for (int j = 0; j < 4; j++)
		{

			res[i] += point[j] * mCompMatrix[aType][j * 4 + i];
		}
	}
}

// res = a cross b;
void gmu::crossProduct(float *a, float *b, float *res)
{

	res[0] = a[1] * b[2] - b[1] * a[2];
	res[1] = a[2] * b[0] - b[2] * a[0];
	res[2] = a[0] * b[1] - b[0] * a[1];
}

// returns a . b
float gmu::dotProduct(float *a, float *b)
{

	float res = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];

	return res;
}

// returns k * a
void gmu::constProduct(float k, float *a, float *res)
{
	res[0] = k * a[0];
	res[1] = k * a[1];
	;
	res[2] = k * a[2];
}

// Normalize a vec3
void gmu::normalize(float *a)
{

	float mag = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);

	a[0] /= mag;
	a[1] /= mag;
	a[2] /= mag;
}

// res = b - a
void gmu::subtract(float *a, float *b, float *res)
{
	res[0] = b[0] - a[0];
	res[1] = b[1] - a[1];
	res[2] = b[2] - a[2];
}

// res = a + b
void gmu::add(float *a, float *b, float *res)
{
	res[0] = b[0] + a[0];
	res[1] = b[1] + a[1];
	res[2] = b[2] + a[2];
}

// returns |a|
float gmu::length(float *a)
{

	return (sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]));
}

// Computes derived matrices
void gmu::computeDerivedMatrix(ComputedMatrixTypes aType)
{
	memcpy(mCompMatrix[ComputedMatrixTypes::VIEW_MODEL], mMatrix[MatrixTypes::VIEW], 16 * sizeof(float));
	multMatrix(mCompMatrix[ComputedMatrixTypes::VIEW_MODEL], mMatrix[MatrixTypes::MODEL]);

	if (aType == ComputedMatrixTypes::PROJ_VIEW_MODEL)
	{
		memcpy(mCompMatrix[ComputedMatrixTypes::PROJ_VIEW_MODEL], mMatrix[MatrixTypes::PROJECTION], 16 * sizeof(float));
		multMatrix(mCompMatrix[ComputedMatrixTypes::PROJ_VIEW_MODEL], mCompMatrix[ComputedMatrixTypes::VIEW_MODEL]);
	}
}

// computes the derived normal matrix - should be used after computeDerivedMatrix
void gmu::computeNormalMatrix3x3()
{
	float mMat3x3[9];

	mMat3x3[0] = mCompMatrix[ComputedMatrixTypes::VIEW_MODEL][0];
	mMat3x3[1] = mCompMatrix[ComputedMatrixTypes::VIEW_MODEL][1];
	mMat3x3[2] = mCompMatrix[ComputedMatrixTypes::VIEW_MODEL][2];

	mMat3x3[3] = mCompMatrix[ComputedMatrixTypes::VIEW_MODEL][4];
	mMat3x3[4] = mCompMatrix[ComputedMatrixTypes::VIEW_MODEL][5];
	mMat3x3[5] = mCompMatrix[ComputedMatrixTypes::VIEW_MODEL][6];

	mMat3x3[6] = mCompMatrix[ComputedMatrixTypes::VIEW_MODEL][8];
	mMat3x3[7] = mCompMatrix[ComputedMatrixTypes::VIEW_MODEL][9];
	mMat3x3[8] = mCompMatrix[ComputedMatrixTypes::VIEW_MODEL][10];

	float det, invDet;

	det = mMat3x3[0] * (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) +
		  mMat3x3[1] * (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) +
		  mMat3x3[2] * (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]);

	invDet = 1.0f / det;

	mNormal3x3[0] = (mMat3x3[4] * mMat3x3[8] - mMat3x3[5] * mMat3x3[7]) * invDet;
	mNormal3x3[1] = (mMat3x3[5] * mMat3x3[6] - mMat3x3[8] * mMat3x3[3]) * invDet;
	mNormal3x3[2] = (mMat3x3[3] * mMat3x3[7] - mMat3x3[4] * mMat3x3[6]) * invDet;
	mNormal3x3[3] = (mMat3x3[2] * mMat3x3[7] - mMat3x3[1] * mMat3x3[8]) * invDet;
	mNormal3x3[4] = (mMat3x3[0] * mMat3x3[8] - mMat3x3[2] * mMat3x3[6]) * invDet;
	mNormal3x3[5] = (mMat3x3[1] * mMat3x3[6] - mMat3x3[7] * mMat3x3[0]) * invDet;
	mNormal3x3[6] = (mMat3x3[1] * mMat3x3[5] - mMat3x3[4] * mMat3x3[2]) * invDet;
	mNormal3x3[7] = (mMat3x3[2] * mMat3x3[3] - mMat3x3[0] * mMat3x3[5]) * invDet;
	mNormal3x3[8] = (mMat3x3[0] * mMat3x3[4] - mMat3x3[3] * mMat3x3[1]) * invDet;
}

// returns a pointer to the requested matrix
float *gmu::get(MatrixTypes aType)
{
	return mMatrix[aType];
}

// returns a pointer to the requested matrix of ComputedMatrixTypes
float *gmu::get(ComputedMatrixTypes aType)
{
	// computeDerivedMatrix(aType);
	return mCompMatrix[aType];
}

// Calculate and return the normal matrix
float *gmu::getNormalMatrix()
{
	return mNormal3x3;
}

// It calculates only the PVM matrix. Just an auxiliary function to be used in billboad demo: it implies that VIEW_MODEL was already calculated
void gmu::computeDerivedMatrix_PVM()
{
	memcpy(mCompMatrix[ComputedMatrixTypes::PROJ_VIEW_MODEL], mMatrix[MatrixTypes::PROJECTION], 16 * sizeof(float));
	multMatrix(mCompMatrix[ComputedMatrixTypes::PROJ_VIEW_MODEL], mCompMatrix[ComputedMatrixTypes::VIEW_MODEL]);
}

// Maps object coordinates to window coordinates: - should be used after computeDerivedMatrix
bool gmu::project(float *objCoord, float *windowCoord, int *m_viewport)
{
	float point_tmp[4];

	// gets point in clipping coordinates
	multMatrixPoint(ComputedMatrixTypes::PROJ_VIEW_MODEL, objCoord, point_tmp);
	// normalize between -1 and 1
	if (point_tmp[3] == 0.0f) // the w value
		return false;
	else
		point_tmp[3] = 1 / point_tmp[3];

	// Perspective division
	point_tmp[0] *= point_tmp[3];
	point_tmp[1] *= point_tmp[3];
	point_tmp[2] *= point_tmp[3];

	// Window coordinates
	// Map x, y to range 0-1
	windowCoord[0] = (point_tmp[0] * 0.5 + 0.5) * m_viewport[2] + m_viewport[0];
	windowCoord[1] = (point_tmp[1] * 0.5 + 0.5) * m_viewport[3] + m_viewport[1];
	// This is only correct when glDepthRange(0.0, 1.0)
	windowCoord[2] = (1.0 + point_tmp[2]) * 0.5; // Between 0 and 1
	return true;
}

void gmu::shadow_matrix(float *m, float *plane, float *light) // planar shadows
{
	float dot = plane[0] * light[0] + plane[1] * light[1] + plane[2] * light[2] + plane[3] * light[3];

	m[0] = dot - light[0] * plane[0];
	m[4] = -light[0] * plane[1];
	m[8] = -light[0] * plane[2];
	m[12] = -light[0] * plane[3];

	m[1] = -light[1] * plane[0];
	m[5] = dot - light[1] * plane[1];
	m[9] = -light[1] * plane[2];
	m[13] = -light[1] * plane[3];

	m[2] = -light[2] * plane[0];
	m[6] = -light[2] * plane[1];
	m[10] = dot - light[2] * plane[2];
	m[14] = -light[2] * plane[3];

	m[3] = -light[3] * plane[0];
	m[7] = -light[3] * plane[1];
	m[11] = -light[3] * plane[2];
	m[15] = dot - light[3] * plane[3];
}
