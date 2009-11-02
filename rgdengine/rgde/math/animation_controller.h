#pragma once

#include <rgde/math/transform.h>
#include <rgde/math/interpolyator.h>

namespace math
{

	class frame_anim_controller
	{
		typedef math::interpolator<math::Vec3f> PositionInterpolyator;
		typedef math::interpolator<math::Vec3f> RotationInterpolyator;
		typedef math::interpolator<math::Vec3f> ScaleInterpolyator;

	public:
		frame_anim_controller( TiXmlNode* pXmlNode = 0, math::frame_ptr spFrame = frame_ptr());
		virtual ~frame_anim_controller(){}

		bool load( TiXmlNode* pXmlNode );

		float getWeight() const;
		void  setWeight(float fWeight);

		void atachToFrame( frame_ptr spFrame ) { m_spFrame = spFrame; }
		frame_ptr& getFrame() {return m_spFrame;} 

		inline void setRate( float rate ) { m_fAnimationRate = rate; }
		inline float getRate() const { return m_fAnimationRate; }

		inline void SetAnimationTime( float time ) { m_fAnimationTime = time; }

		void update( float dt );
		void updateMatrix();
		
		void  setCurrentTime(float fCurrentTime) { m_fCurrentTime = fCurrentTime; }
		float get_current_time() const { return m_fCurrentTime; }
		float getAnimationTime() const { return m_fAnimationTime; }

		void stop();
		void pause();
		void start();

		inline void setLooped( bool loop ) { m_bLooped = loop; }

		inline bool isPlaing() const { return m_bPlaying && !m_bPaused; }

		inline bool isPaused() const { return m_bPaused; }
		inline bool isLooped() const { return m_bLooped; }

		inline PositionInterpolyator&	getPosInterpolyator() { return m_PosInterpolyator; }
		inline RotationInterpolyator&	getRotationInterpolyator() { return m_RotationInterpolyator; }
		inline ScaleInterpolyator&		getScaleInterpolyator() { return m_ScaleInterpolyator; }

	private:
		float m_fAnimationTime;
		float m_fCurrentTime;
		float m_fAnimationRate;
		float m_fWeight;

		math::frame_ptr m_spFrame;

		bool m_bPaused;
		bool m_bPlaying;
		bool m_bLooped;

		PositionInterpolyator	m_PosInterpolyator;
		RotationInterpolyator	m_RotationInterpolyator;
		ScaleInterpolyator		m_ScaleInterpolyator;
	};
}