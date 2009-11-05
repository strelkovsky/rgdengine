#include "precompiled.h"

#include <rgde/math/transform.h>

#include "../base/exception.h"
#include <rgde/base/lexical_cast.h>

#include <rgde/render/lines3d.h>
#include <rgde/render/render_device.h>

namespace math
{
	matrix44f frame::make_transform(const point3f& pos, const Quatf& rot, const vec3f& s)
	{
		math::matrix44f rotation;
		math::setRot(rotation, rot);		

		math::matrix44f translate;
		math::setTrans(translate, pos);

		math::matrix44f scale;
		math::setScale(scale, s);

		return translate*rotation*scale;
	}

	matrix44f frame::make_transform(const point3f& pos, const EulerAngleXYZf& rot, const vec3f& s)
	{
		math::Quatf quat = math::make<Quatf, EulerAngleXYZf>(rot);
		return make_transform(pos, quat, s);
	}

	frame::frame()
		: m_bIsNeedRecompute(false),
		  core::meta_node<frame>("frame"),
		  m_bNeedRecomputeGlobalMatrix(true),
		  m_scale(1.0f,1.0f,1.0f)
	{
		//property_owner::addProperty(new property<math::vec3f>(m_scale, "Scale"));
		//property_owner::addProperty(new property<point3f>(m_position, "Position", "Point"));
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

	void frame::set_position(const point3f& pos)
	{
		m_position = pos;
		m_bIsNeedRecompute = true;
	}

	void frame::set_rot(const Quatf& quat)
	{
		m_rotation = quat;
		m_bIsNeedRecompute = true;
	}

	void frame::look_at(const vec3f& eye, const vec3f& look_at, const vec3f& up_vec)
	{	
		m_position = eye;
		const math::vec3f& up = up_vec;
		math::vec3f at = look_at - eye;

		{	
			using namespace math;
			vec3f z = makeNormal<float>(at);
			vec3f x = makeNormal<float>(makeCross(up, z));
			vec3f y = makeCross<float>(z, x);

			Matrix33f mat = makeAxes<Matrix33f>(x, y, z);
			set(m_rotation, mat); 
		}

		m_bIsNeedRecompute = true;
	}

	void frame::set_scale(const vec3f& s)
	{
		m_scale = s;
		m_bIsNeedRecompute = true;
	}

	const matrix44f & frame::get_local_tm() const
	{
		computeLocalTransform();
		return m_local_tm;
	}

	const matrix44f & frame::get_full_tm() const
	{
        computeFullTransform();
		return m_fullTransform;
	}

	void frame::debug_draw() const
	{
		const float l = 10.5f;
		math::point3f p = get_world_pos();

		math::point3f X = p + l * getLeftGlobal();
		math::point3f Y = p + l * getUpGlobal();
		math::point3f Z = p + l * getAtGlobal();

		render::lines3d& line_manager = render::render_device::get().get_lines3d();
		line_manager.add_line( p, X, math::Red );
		line_manager.add_line( p, Y, math::Green );
		line_manager.add_line( p, Z, math::Blue );
	}

	void frame::computeLocalTransform() const
	{
		if (!m_bIsNeedRecompute)
			return;

		math::matrix44f rotation;
		math::setRot(rotation, m_rotation);		

		math::matrix44f translate;
		math::setTrans(translate, m_position);
		
		math::matrix44f scale;
		math::setScale(scale, m_scale);

		m_local_tm = translate * rotation * scale;

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
			m_fullTransform = get_parent()->get_full_tm() * m_local_tm;
		else
			m_fullTransform = m_local_tm;

		m_bNeedRecomputeGlobalMatrix = false;
	}

	void frame::on_parent_change()
	{
		m_bIsNeedRecompute = true;
	}

	point3f frame::get_world_pos() const 
	{
		computeFullTransform();
		const  matrix44f &m	= m_fullTransform;
		return point3f(m.mData[12], m.mData[13], m.mData[14]);
	}

	// left(right)  up          at
	//xaxis.x     yaxis.x     zaxis.x
	//xaxis.y     yaxis.y     zaxis.y
	//xaxis.z     yaxis.z     zaxis.z
	vec3f frame::getUp() const 
	{
		computeLocalTransform();
		const matrix44f &m= m_local_tm;
		return vec3f(m[1][0], m[1][1], m[1][2]);
	}
	vec3f frame::getAt() const 
	{
		computeLocalTransform();
		const matrix44f &m= m_local_tm;
		return vec3f(m[2][0], m[2][1], m[2][2]);
	}
	vec3f frame::getLeft() const 
	{
		computeLocalTransform();
		const matrix44f &m= m_local_tm;
		return vec3f(m[0][0], m[0][1], m[0][2]);
	}

	vec3f frame::getUpGlobal() const
	{
		computeFullTransform();
		matrix44f &m = m_fullTransform;
		return vec3f(m[1][0], m[1][1], m[1][2]);
	}

	vec3f frame::getAtGlobal() const
	{
		computeFullTransform();
		matrix44f &m = m_fullTransform;
		return vec3f(m[2][0], m[2][1], m[2][2]);
	}

	vec3f frame::getLeftGlobal() const
	{
		computeFullTransform();
		matrix44f &m = m_fullTransform;
		return vec3f(m[0][0], m[0][1], m[0][2]);
	}

	//Neonic: octree
	void frame::update( bool NeedFullUpdate )
	{
		for (math::frame::children_list::const_iterator it = get_children().begin(); it != get_children().end(); it++)
			(*it)->update(NeedFullUpdate);
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