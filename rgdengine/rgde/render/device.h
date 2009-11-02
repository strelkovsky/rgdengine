#pragma once

#include <rgde/math/camera.h>
#include <rgde/render/font.h>

#include <rgde/base/singelton.h>

namespace render
{
	class device_object;

	class device_dx9
	{
	protected:
		device_dx9();
		~device_dx9();

	public:
		void					onLost();
		void					onReset();

		void					addDeviceObject(device_object* pObj);
		void					removeDeviceObject(device_object* pObj);

		void					setCurentCamera(const math::camera_ptr& cam){m_curCam = cam; }
		const math::camera_ptr&	get_curent_camera() const {return m_curCam;}

		void					setShaderFlags(unsigned long flag)	{m_shaderFlags = flag;}
		unsigned long			getShaderFlags() const				{return m_shaderFlags;}

		void					addStatistic( unsigned verts, unsigned tris);
		void					resetStats();

		inline unsigned			getTris()	const {return m_triangles;}
		inline unsigned			getVerts()	const {return m_verts;}

		/// ���������� ������ BackBuffer
		math::Vec2f				getBackBufferSize();		

		void					showWiredFloorGrid(float size, unsigned num = 20, const math::Color& color = math::Green);

		float                   getFPS(float absoluteTime) const;
		void                    showFPS(const font_ptr& font);
		void                    showStatistics(const font_ptr& font);

        //->
        void                    setClearColor (math::Color color) {m_clear_color = color;}
        math::Color             getClearColor () {return m_clear_color;}
        //-<

	protected:
        //->
        math::Color             m_clear_color;
        //-<

		unsigned long			m_shaderFlags;
		math::camera_ptr			m_curCam;

		unsigned				m_verts;
		unsigned				m_triangles;

		typedef std::list<device_object* > DeviceObjects;
		DeviceObjects m_objects;
	};

	typedef base::singelton<device_dx9> TheDevice;

	class device_object
	{
	public:
								device_object();
		virtual					~device_object();

		virtual void			onLostDevice() = 0;
		virtual void			onResetDevice() = 0;
	};
}