/** ----------------------------------------------------------
 *AVT Math Lib 
 * 
 * Definition of the Graphics Math Utility - gmu Class
 *
 *
 // The code comes with no warranties, use it at your own risk.
 // You may use it, or parts of it, wherever you want.
 // 
 // Author: João Madeiras Pereira
 //
 * 
 ---------------------------------------------------------------*/
#ifndef _mathUtility_
#define _mathUtility_

#include <vector>
#include <string>
#include <GL/glew.h>

// number of settable matrices
#define COUNT_MATRICES 3
// number of derived matrices
#define COUNT_COMPUTED_MATRICES 2

/// Graphics Math Utility - gmu

class gmu {

public:
	/// Enumeration of the matrix types
	enum MatrixTypes {
		MODEL,
		VIEW,
		PROJECTION
	};

	/// Enumeration of derived matrices
	enum ComputedMatrixTypes {
		VIEW_MODEL,
		PROJ_VIEW_MODEL
	};

		
	/******* Some useful math operations ************/

	/** vector cross product res = a x b
		 * Note: memory for the result must be allocatted by the caller
		 *
		 * \param a,b the two input float[3]
		 * \param res the ouput result, a float[3]
	*/
	static void crossProduct(float* a, float* b, float* res);

	/** vector dot product
		 *
		 * \param a,b the two input float[3]
		 * \returns the dot product a.b
	*/
	static float dotProduct(float* a, float* b);

	// multiply a vector by a constant
	static void constProduct(float k, float* a, float* res);

	/// normalize a vec3
	static void normalize(float* a);

	/// vector subtraction res = b - a
	static void subtract(float* a, float* b, float* res);

	/// vector addition res = a + b
	static void add(float* a, float* b, float* res);

	/// vector length
	static float length(float* a);

	/*****    Matrix Transformations  *********************/

	//resMatrix = resMatrix * aMatrix
	static void multMatrix(float* resMatrix, float* aMatrix);

	//Matrix for calculating planar shadows
	static void shadow_matrix(float* mat, float* plane, float* light);   

	//*********Matrix Transformations which involve MODEL, VIEW and PROJECTION stacks*****//

	/** Similar to glTranslate*. 
		 *
		 * \param aType any value from MatrixTypes
		 * \param x,y,z vector to perform the translation
	*/
	void translate(MatrixTypes aType, float x, float y, float z);

	/** Similar to glScale*.
		 *
		 * \param aType any value from MatrixTypes
		 * \param x,y,z scale factors
	*/
	void scale(MatrixTypes aType, float x, float y, float z);

	/** Similar to glRotate*. 
		 *
		 * \param aType any value from MatrixTypes
		 * \param angle rotation angle in degrees
		 * \param x,y,z rotation axis in degrees
	*/
	void rotate(MatrixTypes aType, float angle, float x, float y, float z);

	/** Similar to glLoadIdentity.
		 *
		 * \param aType any value from MatrixTypes
	*/
		void loadIdentity(MatrixTypes aType);

	/** Similar to glMultMatrix.
		 *
		 * \param aType any value from MatrixTypes
		 * \param aMatrix matrix in column major order data, float[16]
	*/
	void multMatrix(MatrixTypes aType, float *aMatrix);


	/** Similar to gLoadMatrix.
		 *
		 * \param aType any value from MatrixTypes
		 * \param aMatrix matrix in column major order data, float[16]
	*/

	void loadMatrix(MatrixTypes aType, float *aMatrix);

	/** Similar to glPushMatrix
		 * 
		 * \param aType any value from MatrixTypes
	*/
	void pushMatrix(MatrixTypes aType);

	/** Similar to glPopMatrix
		 * 
		 * \param aType any value from MatrixTypes
	*/
	void popMatrix(MatrixTypes aType);

	/** Similar to gluLookAt
		 *
		 * \param xPos, yPos, zPos camera position
		 * \param xLook, yLook, zLook point to aim the camera at
		 * \param xUp, yUp, zUp camera's up vector
	*/
	void lookAt(float xPos, float yPos, float zPos,
				float xLook, float yLook, float zLook,
				float xUp, float yUp, float zUp);


	/** Similar to gluPerspective
		 *
		 * \param fov vertical field of view
		 * \param ratio aspect ratio of the viewport or window
		 * \param nearp,farp distance to the near and far planes
	*/
	void perspective(float fov, float ratio, float nearp, float farp);

	/** Similar to glOrtho and gluOrtho2D (just leave the 
		 * last two params blank).
		 *
		 * \param left,right coordinates for the left and right vertical 
		 * clipping planes
		 * \param bottom,top coordinates for the bottom and top horizontal 
		 * clipping planes
		 * \param nearp,farp distance to the near and far planes
	*/
	void ortho(float left, float right, float bottom, float top,
					float nearp=-1.0f, float farp=1.0f);

	/** Similar to glFrustum
		 *
		 * \param left,right coordinates for the left and right vertical 
		 * clipping planes
		 * \param bottom,top coordinates for the bottom and top horizontal 
		 * clipping planes
		 * \param nearp,farp distance to the near and far planes
	*/
	void frustum(float left, float right, float bottom, float top,
					float nearp, float farp);


	void multMatrixPoint(MatrixTypes aType, float *point, float *res);


	/** Computes the multiplication of a computed matrix and a point 
		 *
		 * \param aType any value from ComputedMatrixTypes
		 * \param point a float[4] representing a point
		 * \param res a float[4] res = M * point
	*/
	void multMatrixPoint(ComputedMatrixTypes aType, float *point, float *res);


	/** Set a float* to an identity matrix
		 *
		 * \param a float array with the matrix contents
		 * \param size the order of the matrix
	*/
	void setIdentityMatrix( float *mat, int size=4);

	/// Computes Derived Matrices (4x4)
	void computeDerivedMatrix(ComputedMatrixTypes aType);

	/// Computes the 3x3 normal matrix for use with glUniform
	void computeNormalMatrix3x3();

	/** Similar to glGet
		 *
		 * \param aType any value from MatrixTypes
		 * \returns pointer to the matrix (float[16])
	*/
	float* get(MatrixTypes aType);


	/** Similar to glGet for computed matrices
		 *
		 * \param aType any value from ComputedMatrixTypes
		 * \returns pointer to the matrix (float[16] or float[9])
	*/
	float* get(ComputedMatrixTypes aType);

	/** Similar to glGet for computed matrices
	 *
	 * \returns pointer to the normal matrixfloat[9])
	*/
	float* getNormalMatrix();

	// It calculates only the PVM matrix. Just an auxiliary function to be used in billboad demo: it implies that VIEW_MODEL was already calculated
	void computeDerivedMatrix_PVM();

	// Maps object coordinates to window coordinates: - should be used after computeDerivedMatrix
	bool project(float* objCoord, float* windowCoord, int* m_viewport);

	
private:
	/// The storage for matrices
	float mMatrix[COUNT_MATRICES][16];
	float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

	/// The normal matrix
	float mNormal3x3[9];

	/// Matrix stacks for all matrix types
	std::vector<float*> mMatrixStack[COUNT_MATRICES];
};

#endif