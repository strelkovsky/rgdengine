#pragma once

#include "math/mathTypes3d.h"

namespace render
{
	class CFog
	{
	public:
		enum Type
		{
			None,
			Exp,
			Exp2,
			Linear
		};

		CFog();

		void setMode(Type type);
		void loadFromXML(const std::string& strFileName);
		void loadFromXML(TiXmlElement* node);

	private:
		bool        m_bEnabled;
		math::Color m_color;
		Type		m_type;
		float       m_fStart;
		float       m_fEnd;
	};
}