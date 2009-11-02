#include "precompiled.h"

#include <rgde/math/transform.h>

#include "../base/exception.h"
#include <rgde/base/lexical_cast.h>

#include <rgde/render/lines3d.h>

namespace math
{
	Matrix44f frame::makeTransformMatrix(const Point3f& pos, const Quatf& rot, const Vec3f& s)
	{
		math::Matrix44f rotation;
		math::setRot(rotation, rot);		

		math::Matrix44f translate;
		math::setTrans(translate, pos);

		math::Matrix44f scale;
		math::setScale(scale, s);

		return translate*rotation*scale;
	}

	Matrix44f frame::makeTransformMatrix(const Point3f& pos, const EulerAngleXYZf& rot, const Vec3f& s)
	{
		math::Quatf quat = math::make<Quatf, EulerAngleXYZf>(rot);
		return makeTransformMatrix(pos, quat, s);
	}

	frame::frame()
		: m_bIsNeedRecompute(false),
		  core::meta_node<frame>("frame"),
		  m_bNeedRecomputeGlobalMatrix(true),
		  m_scale(1.0f,1.0f,1.0f)
	{
		//property_owner::addProperty(new property<math::Vec3f>(m_scale, "Scale"));
		//property_owner::addProperty(new property<Point3f>(m_position, "Position", "Point"));
		//property_owner::addProperty(new property<Quatf>(m_rotation, "Rotation", "Quaternion"));
	}

	frame::~frame()
	{
	}

	void frame::findFrames(const std::string& strTemplate, std::vector<frame_ptr>& container)
	{
		const std::string &strFrameName = get_name();

		size_t pos = strFrameName.find_first_of("_");
		if(pos != -1)
		{
			size_t nBegin = strFrameName.find_first_not_of(" ");
			std::string name = strFrameName.substr(nBegin, pos - nBegin);

			if(name == strTemplate)
				container.push_back(this);
		}

		for (math::frame::children_list::const_iterator it = get_children().begin(); it != get_children().end(); it++)
			(*it)->findFrames(strTemplate, container);
	}

	void frame::setPosition(const Point3f& pos)
	{
		m_position = pos;
		m_bIsNeedRecompute = true;
	}

	void frame::setRotation(const Quatf& quat)
	{
		m_rotation = quat;
		m_bIsNeedRecompute = true;
	}

	void frame::lookAt(const Vec3f& vEyePt, const Vec3f& vLookatPt, const Vec3f& vUpVec)
	{	
		m_position = vEyePt;
		const math::Vec3f& up = vUpVec;
		math::Vec3f at = vLookatPt - vEyePt;

		{	
			using namespace math;
			Vec3f z = makeNormal<float>(at);
			Vec3f x = makeNormal<float>(makeCross(up, z));
			Vec3f y = makeCross<float>(z, x);

			Matrix33f mat = makeAxes<Matrix33f>(x, y, z);
			set(m_rotation, mat); 
		}

		m_bIsNeedRecompute = true;
	}

	void frame::setScale(const Vec3f& s)
	{
		m_scale = s;
		m_bIsNeedRecompute = true;
	}

	const Matrix44f & frame::getLocalTransform() const
	{
		computeLocalTransform();
		return m_localTransform;
	}

	const Matrix44f & frame::getFullTransform() const
	{
        computeFullTransform();
		return m_fullTransform;
	}

	void frame::debugDraw() const
	{
		const float l = 10.5f;
		math::Point3f p = getGlobalPosition();

		math::Point3f X = p + l * getLeftGlobal();
		math::Point3f Y = p + l * getUpGlobal();
		math::Point3f Z = p + l * getAtGlobal();

		render::Line3dManager& line_manager = render::TheLine3dManager::get();
		line_manager.addLine( p, X, math::Red );
		line_manager.addLine( p, Y, math::Green );
		line_manager.addLine( p, Z, math::Blue );
	}

	void frame::computeLocalTransform() const
	{
		if (!m_bIsNeedRecompute)
			return;

		math::Matrix44f rotation;
		math::setRot(rotation, m_rotation);		

		math::Matrix44f translate;
		math::setTrans(translate, m_position);
		
		math::Matrix44f scale;
		math::setScale(scale, m_scale);

		m_localTransform = translate * rotation * scale;

		m_bIsNeedRecompute = false;
		m_bNeedRecomputeGlobalMatrix = true;
	}

	void frame::computeFullTransform() const 
	{
		if (m_bIsNeedRecompute)
			computeLocalTransform();

		if (!m_bNeedRecomputeGlobalMatrix)
			return;

		computeLocalTransform();

		if (get_parent())
			m_fullTransform = get_parent()->getFullTransform() * m_localTransform;
		else
			m_fullTransform = m_localTransform;

		m_bNeedRecomputeGlobalMatrix = false;
	}

	void frame::on_parent_change()
	{
		m_bIsNeedRecompute = true;
	}

	Point3f frame::getGlobalPosition() const 
	{
		computeFullTransform();
		const  Matrix44f &m	= m_fullTransform;
		return Point3f(m.mData[12], m.mData[13], m.mData[14]);
	}

	// left(right)  up          at
	//xaxis.x     yaxis.x     zaxis.x
	//xaxis.y     yaxis.y     zaxis.y
	//xaxis.z     yaxis.z     zaxis.z
	Vec3f frame::getUp() const 
	{
		computeLocalTransform();
		const Matrix44f &m= m_localTransform;
		return Vec3f(m[1][0], m[1][1], m[1][2]);
	}
	Vec3f frame::getAt() const 
	{
		computeLocalTransform();
		const Matrix44f &m= m_localTransform;
		return Vec3f(m[2][0], m[2][1], m[2][2]);
	}
	Vec3f frame::getLeft() const 
	{
		computeLocalTransform();
		const Matrix44f &m= m_localTransform;
		return Vec3f(m[0][0], m[0][1], m[0][2]);
	}

	Vec3f frame::getUpGlobal() const
	{
		computeFullTransform();
		Matrix44f &m = m_fullTransform;
		return Vec3f(m[1][0], m[1][1], m[1][2]);
	}

	Vec3f frame::getAtGlobal() const
	{
		computeFullTransform();
		Matrix44f &m = m_fullTransform;
		return Vec3f(m[2][0], m[2][1], m[2][2]);
	}

	Vec3f frame::getLeftGlobal() const
	{
		computeFullTransform();
		Matrix44f &m = m_fullTransform;
		return Vec3f(m[0][0], m[0][1], m[0][2]);
	}

	//Neonic: octree
	void frame::updateTree( bool NeedFullUpdate )
	{
		for (math::frame::children_list::const_iterator it = get_children().begin(); it != get_children().end(); it++)
			(*it)->updateTree(NeedFullUpdate);
	};

	//-----------------------------------------------------------------------------------
	void frame::toStream(io::write_stream& wf) const
	{
		wf	<< m_scale
			<< m_position
			<< m_rotation;

		//// ��������� �������� �������������
		wf << (unsigned int)m_children.size();
		for( children_list::const_iterator it = m_children.begin(); it != m_children.end(); it++ )
			(*it)->toStream( wf );
	}

	//-----------------------------------------------------------------------------------
	void frame::fromStream(io::read_stream& rf)
	{
		rf	>> m_scale
			>> m_position
			>> m_rotation;

		m_bIsNeedRecompute = true;
		
		//// ������ �������� �������������
		unsigned nChildren;
		rf >> nChildren;

		for(unsigned i = 0; i < nChildren; i++)
		{
			frame_ptr child = new frame;
			child->fromStream( rf );
			add( child );
		}
	}

	std::ostream& operator<<(std::ostream& out, const math::frame& f)
	{
		return out;
	}

	std::istream& operator>>(std::istream& in, math::frame& f)
	{
		return in;
	}
}