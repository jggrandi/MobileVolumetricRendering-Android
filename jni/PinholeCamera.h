#ifndef PINHOLECAMERA_INCLUDED
#define PINHOLECAMERA_INCLUDED

#include "esUtils/esUtil.h"

class PinholeCamera
{
public:
	enum ProjectionTypeEnum {
		PERSPECTIVE_PROJECTION,
		ORTHOGONAL_PROJECTION
	};
	enum CameraTypeEnum {
		TRACKBALL_CAMERA,
		FLYBY_CAMERA
	};
	PinholeCamera(void);
	//CPinholeCamera(const CPinholeCamera &pinholeCamera);
	~PinholeCamera(void);
	//CPinholeCamera &operator=(const CPinholeCamera &pinholeCamera);
	bool Create(float leftClippingPlaneCoord, float rightClippingPlaneCoord, float bottomClippingPlaneCoord, float topClippingPlaneCoord, float nearPlaneDepth, float farPlaneDepth, int windowWidth, int windowHeight);
	bool Create(float fieldOfView, float nearPlaneDepth, float farPlaneDepth, int windowWidth, int windowHeight);
	/*float GetLeftClippingPlaneCoord(void) const;
	bool SetLeftClippingPlaneCoord(float coord);
	float GetRightClippingPlaneCoord(void) const;
	bool SetRightClippingPlaneCoord(float coord);
	float GetBottomClippingPlaneCoord(void) const;
	bool SetBottomClippingPlaneCoord(float coord);
	float GetTopClippingPlaneCoord(void) const;
	bool SetTopClippingPlaneCoord(float coord);
	float GetFieldOfView(void) const;
	bool SetFieldOfView(float fieldOfView);
	float GetNearPlaneDepth(void) const;
	bool SetNearPlaneDepth(float depth);
	float GetFarPlaneDepth(void) const;
	bool SetFarPlaneDepth(float depth);
	*/void SetViewport(int x, int y, int width, int height);
	void Pitch(float angle);
	void Yaw(float angle);
	void MoveFront(float step);
	void MoveSide(float step);
	void MoveUp(float step);
	ESMatrix ApplyTransform(void);
	//ProjectionTypeEnum GetProjectionType(void) const;
	//void SetCameraType(CameraTypeEnum type);
	//CameraTypeEnum GetCameraType(void) const;
	void ResetViewMatrix(void);
	//void SetViewMatrix(const float *viewMatrix);
	ESMatrix GetViewMatrix(void);
	ESMatrix GetProjectionMatrix(void);
	//const int *GetViewport(void) const;
	//const float *GetImagePlaneMatrix(void) const;
	//const float *GetCenterOfProjection(void);
	//const bool SaveTransformToFile(const std::string &fileName) const;
	//bool LoadTransformFromFile(const std::string &fileName);
private:
	void Create(void);
	//void Copy(const CPinholeCamera &pinholeCamera);
	//void ResetProjectionMatrix(void);
	void CalculateProjectionMatrix(void);
	//void ResetImagePlaneMatrix(void);
	//void CalculateImagePlaneMatrix(void);
	//bool UpdateProjection(float leftClippingPlaneCoord, float rightClippingPlaneCoord, float bottomClippingPlaneCoord, float topClippingPlaneCoord, float nearPlaneDepth, float farPlaneDepth);
	//bool UpdateProjection(float fieldOfView, float nearPlaneDepth, float farPlaneDepth);
protected:
	enum ProjectionTypeEnum m_projectionType;
	enum CameraTypeEnum m_cameraType;  
	
	float m_leftClippingPlaneCoord;
	float m_rightClippingPlaneCoord;
	float m_bottomClippingPlaneCoord;
	float m_topClippingPlaneCoord;
	float m_fieldOfView;
	float m_nearPlaneDepth;
	float m_farPlaneDepth;
	int m_viewport[4];
	float m_pitchRotation;
	float m_yawRotation;
	float m_eyeVectorDisplacement;
	float m_sideVectorDisplacement;
	float m_upVectorDisplacement;
	ESMatrix m_viewMatrix;
	ESMatrix m_projectionMatrix;
	float m_imagePlaneMatrix[9];
	float m_centerOfProjection[3];
	
};

#endif // PINHOLECAMERA_INCLUDED

