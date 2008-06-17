#include "precompiled.h"

#include <rgde/render/particles/particlesMain.h>
#include <rgde/render/particles/particlesParticlesProcessor.h>
#include <rgde/render/particles/particlesParticleTank.h>
#include <rgde/render/particles/particlesAbstractEmitter.h>


namespace particles{

	//-----------------------------------------------------------------------------------
	ParticlesProcessor::ParticlesProcessor(AbstractEmitter* em )   // �����������
	: m_bIsVisible(true), m_pParentEmitter(em), m_bIsGeometric(false), core::XmlClass("ParticlesProcessor")
	{
		//addProperty(new TProperty<bool>(m_bIsAnimTextureUsed, "AnimTextureUse", "bool"));
		m_bIsAnimTextureUsed = false; 
		//addProperty(new TProperty<bool>(m_bIsTexAnimCycled, "TexAnimCycled", "bool"));
		m_bIsTexAnimCycled	= false;
		//addProperty(new TProperty<unsigned>(m_cTexFps, "TexFps", "int"));
		m_cTexFps			= 25;
		//addProperty(new TProperty<int>(m_iRndSeed, "RndSeed", "int"));
		m_iRndSeed			= 0;
		//addProperty(new TProperty<bool>(m_bIsSparks, "IsSparks", "bool"));
		m_bIsSparks			= false;
		//addProperty(new TProperty<bool>(m_bIsGeometric, "IsGeometric", "bool"));
		m_bIsGeometric		= false;
		m_dt				= 0;
		m_fRateAccum		= 0;
		m_bIsFading			= false;
		fistTimeInit();
		m_pParentEmitter	= 0;
		//addProperty(new TProperty<bool>(m_bIsGlobal, "IsGlobal", "bool"));
		m_bIsGlobal			= true;

		m_ngkx = 1.8f;

		m_nRndFrame			 = 0;
		m_bIsPlayTexAnimation = true;

		m_spTank = NULL;

		m_PColorAlpha.addKey (0, math::Color (255, 255, 255, 255));
		m_PColorAlpha.addKey (1, math::Color (0, 0, 0, 0));
		m_PLife.addKey(1, 1.0f);
		m_PSize.addKey(1, 1.0f);
		m_PRate.addKey(1, 10.0f);
		m_PVelSpreadAmplifier.addKey(1, 1.0f);

		//addProperty(new TProperty<math::FloatInterp>(m_PRate, "PRate", "FloatInterp"));
		//addProperty(new TProperty<math::FloatInterp>(m_PResistance, "PResistance", "FloatInterp"));
		//addProperty(new TProperty<math::FloatInterp>(m_PSpin, "PSpin", "FloatInterp"));
		//addProperty(new TProperty<math::FloatInterp>(m_PSpinSpread, "PSpinSpread", "FloatInterp"));
		//addProperty(new TProperty<math::FloatInterp>(m_PLife, "PLife", "FloatInterp"));
		//addProperty(new TProperty<math::FloatInterp>(m_PLifeSpread, "PLifeSpread", "FloatInterp"));
		//addProperty(new TProperty<math::FloatInterp>(m_PSize, "PSize", "FloatInterp"));
		//addProperty(new TProperty<math::ColorInterp>(m_PColorAlpha, "PColorAlpha", "ColorInterp"));
		//addProperty(new TProperty<math::Vec3Interp>(m_PActingForce, "PActingForce", "Vec3Interp"));
		//addProperty(new TProperty<math::Vec3Interp>(m_PVelocity, "PVelocity", "Vec3Interp"));
		//addProperty(new TProperty<math::Vec3Interp>(m_PInitialVelSpread, "PInitialVelSpread", "Vec3Interp"));
		//addProperty(new TProperty<math::FloatInterp>(m_PVelSpreadAmplifier, "PVelSpreadAmplifier", "FloatInterp"));
	
		// public properties:
		//REGISTER_PROPERTY(bIsAnimTextureUsed,	bool)
		//REGISTER_PROPERTY(bIsTexAnimCycled,		bool)
		//REGISTER_PROPERTY(cTexFps,				unsigned int)
		//REGISTER_PROPERTY(iRndSeed,				int)
		//REGISTER_PROPERTY(bIsSparks,			bool)
		//REGISTER_PROPERTY(bIsGeometric,			bool)
		//REGISTER_PROPERTY(bIsGlobal,			bool)

		//REGISTER_PROPERTY(PRate,				math::FloatInterp)
		//REGISTER_PROPERTY(PResistance,			math::FloatInterp)
		//REGISTER_PROPERTY(PSpin,				math::FloatInterp)
		//REGISTER_PROPERTY(PSpinSpread,			math::FloatInterp)
		//REGISTER_PROPERTY(PLife,				math::FloatInterp)
		//REGISTER_PROPERTY(PLifeSpread,			math::FloatInterp)
		//REGISTER_PROPERTY(PSize,				math::FloatInterp)
		//REGISTER_PROPERTY(PColorAlpha,			math::ColorInterp)
		//REGISTER_PROPERTY(PActingForce,			math::Vec3Interp)
		//REGISTER_PROPERTY(PVelocity,			math::Vec3Interp)
		//REGISTER_PROPERTY(PInitialVelSpread,	math::Vec3Interp)
		//REGISTER_PROPERTY(PVelSpreadAmplifier,	math::FloatInterp)
		//REGISTER_PROPERTY(TexName,				std::string)

	}

	//-----------------------------------------------------------------------------------
	ParticlesProcessor::~ParticlesProcessor()
	{
		delete m_spTank;
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::updateParticle(Particle& p)
	{
		// ����������� ������� ������� �� �������� �� 0 �� 1 
		float t = p.time / p.ttl; 
		// time passed from last update
		float dt = p.time - p.m_fOldTime;

		if (t >= 1){
			p.dead = true;
			p.color.a = 0;
			return;
		}

		p.color = m_PColorAlpha(t);

		math::Vec3f velocity = m_PVelocity(t);
		
		// setGlobal acceleration
		//const math::Vec3f &acceleration = m_pParentEmitter->getAcceleration(m_bIsGlobal);
		const math::Vec3f force = m_PActingForce(m_fNormalizedTime);

		p.vel += dt * (/*acceleration + */force * 0.5f / p.mass);
		

		// initial velocity
		velocity += p.vel;
		// setGlobal velocity
		// velocity += m_pParentEmitter->getGlobalVelocity(m_bIsGlobal);

		float resistance = math::length(m_fScaling) * math::length(m_fScaling) * (m_PResistance(t) + 1.0f);
		float vel_spread_amp = m_PVelSpreadAmplifier(t);

		velocity = math::length(m_fScaling) * math::length(m_fScaling) * (p.vel_spread * vel_spread_amp + velocity ) / resistance;
			
		p.sum_vel = velocity * dt;

		p.pos += p.sum_vel;
		        
		p.size = math::length(m_fScaling)*math::length(m_fScaling) * m_PSize(t);

		p.rot_speed = m_PSpin(t);
		p.rotation += p.rot_speed + p.initial_spin;

		// save current update time
		p.m_fOldTime = p.time;

		//m_cTexFps
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::reset()								// ReStart ��������� - �������� ��� ����� �� 0
	{
		m_bIsFading = false;

		unsigned size = static_cast<unsigned>(m_Particles.size());
		Particle* pp = &(m_Particles[0]);//[i]

		for (unsigned i = 0; i < size; ++i)
		{
			pp[i].dead = true;
			pp[i].size = 0.0f;
		}
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::fistTimeInit()
	{
		m_bIsAnimTextureUsed		= false;
		m_bIsGlobal				= true;
		m_bModifiersLoaded		= false;
		m_bIsPtankInited		= false;
		m_bIsVisible			= true;
		m_bIntense				= false;
		
		m_nMaxParticles			= 0;
		m_fNormalizedTime		= 0;
		
		//m_TexName = clx::cstr("p_default.bmp");
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::render()
	{
		if( !m_bIsVisible )
			return;

		formTank();

		if (!m_bIsGeometric)
		{
			m_spTank->render(m_texture, m_pParentEmitter->getTransform());
		}
		else
			geomRender();
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::geomRender()
	{
	}
	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::update(float dt) 
	{
		/*
		if (m_iRndSeed){
			rnd.setSeed(m_iRndSeed);
		}
		*/

		m_dt += dt;

		m_fNormalizedTime = m_pParentEmitter->getTime();

		math::Frame& rParentTransform = m_pParentEmitter->getTransform();
		//m_fScaling = (m_ParentTransform->getScaling()).x;
		m_fScaling = rParentTransform.getScale();

		int m_acting_particles = 0;
		// ����� ���������� ������ ���������
		//for (TParticlesIter it = m_Particles.begin(); it != m_Particles.end(); ++it)
		//	if (!(it->dead) && m_bIsGlobal)
		//	{
		//		it->pos += m_ngkx * m_pParentEmitter->m_vCurDisplacement * (1.0f - (it->time / it->ttl));
		//	}

		if (m_dt > 0.01f)
		{
			// ����� ���������� ������ ���������
			for (TParticlesIter it = m_Particles.begin(); it != m_Particles.end(); ++it)
				if (!(it->dead))// && m_bIsGlobal)
				{
					++m_acting_particles;
					it->time += m_dt;
					updateParticle(*it);
				}

			m_dt = 0;
			// �������� ����� �������� ���� ����(���� ���� ����� � �����)
			// ���� � ���� ��� ���� (���-�� Rate)
			if (!m_bIsFading)
				addNewParticles(m_nMaxParticles - m_acting_particles);
		}
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::addNewParticles(int num2add)
	{
		// ��� ����� ��� ����� ���������
		if(!num2add) 
			return;

		// �������� ��������� ��������� ���=��\�� �������
		float rate = m_PRate.getValue(m_fNormalizedTime) / 25.0f;
		m_fRateAccum += rate;

		int to_add  = (int)m_fRateAccum;

		if (to_add > num2add) 
			to_add = num2add;
		
		int added_num = 0;

		for(int i = 0; i < m_nMaxParticles; ++i)
			if (m_Particles[i].dead && to_add > 0){
				createParticle(m_Particles[i]);
				to_add--;
				added_num++;
			}
			else if(to_add < 1)
				break;
		m_fRateAccum -= added_num;
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::createParticle(Particle& p)
	{
		m_pParentEmitter->getParticle(p);
		p.dead = false;
		
		p.ttl = (m_PLife.getValue(m_fNormalizedTime)
			+ rnd()* m_PLifeSpread.getValue(m_fNormalizedTime));// * 10.0f; // debug

		if (!p.ttl)
			p.dead = true;

		math::Vec3f ivs = m_PInitialVelSpread.getValue(m_fNormalizedTime);

		p.vel_spread = math::Vec3f(ivs[0] * (rnd()*2.0f - 1.0f),
							ivs[1] * (rnd()*2.0f - 1.0f),
							ivs[2] * (rnd()*2.0f - 1.0f));

		p.initial_spin = (rnd() * m_PSpinSpread.getValue(m_fNormalizedTime))
						/(25.0f);
		
		if (m_bIsGlobal){
			math::xform( p.initial_pos, getLTM(), p.initial_pos );
			p.initial_vel = m_pParentEmitter->getSpeed();// / 2.0f;
		}

		p.pos = p.initial_pos;
		
		p.m_fOldTime = 0.0f;

		static math::UnitRandom2k lrnd;

		if (m_bIsPlayTexAnimation)
		{
			float f = (float)m_nRndFrame;
			p.cur_tex_frame = lrnd() * f;

			if (p.cur_tex_frame  > m_ucTexFrames)
				p.cur_tex_frame = (float)m_ucTexFrames;
		}
		else
		{
			p.cur_tex_frame = lrnd() * (float)m_ucTexFrames;
		}

		updateParticle(p);
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::debugDraw()
	{
		if( !m_bIsVisible )
			return;

		math::Matrix44f m = getLTM();

		if (m_bIsGlobal)
			m = math::setTrans( m, math::Vec3f(0,0,0) );

		math::Vec3f center, vel;
		render::Line3dManager& line_manager = render::TheLine3dManager::Get();
		for (TParticlesIter it = m_Particles.begin(); it != m_Particles.end(); ++it)
		{
			if ((*it).dead)
				continue;

			if (!m_bIsGlobal)
			{
				center = m * (math::Point3f)((*it).pos);
				vel = it->pos + m * (math::Point3f)((*it).sum_vel*5.0f);
			}
			else
			{
				center = (*it).pos;
				vel = it->pos + (*it).sum_vel*5.0f;
			}

			line_manager.addQuad( center, math::Vec2f (it->size, it->size), 0 );	
			line_manager.addLine( center, vel, math::Green );
		}
	}
	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::formTank()
	{
		if (m_bIsGeometric)
			return;

		const math::Matrix44f& ltm = getLTM();

		PTank::ParticleArray& array = m_spTank->getParticles();
		array.resize( m_Particles.size() );
		int i = 0;

		for (TParticlesIter it = m_Particles.begin(); it != m_Particles.end(); ++it)
		{
			Particle &p = *it;

			//if (m_bIsGlobal)
			//	//*poss = p.pos;
			//	array[i].pos = p.pos;
			//else{
			//	//*poss = ltm.transform(p.pos);
			//	math::xform( array[i].pos, ltm, p.pos );
			//}
			array[i].pos = p.pos;


			array[i].size = math::Vec2f( p.size, p.size );
			array[i].spin = p.rotation * 3.1415926f/180.0f;
			array[i].color = p.color.color;

			//if (m_bIsAnimTextureUsed)
			//{
			//	agl::AglTexCoords uv1, uv2;

			//	unsigned char frame = 0;

			//	if (m_bIsPlayTexAnimation)
			//	{
			//		if (!m_bIsTexAnimCycled)
			//			frame = (unsigned char)((m_ucTexFrames - 1) * (p.time / p.ttl));
			//		else 
			//			frame = (unsigned char)(p.time * m_cTexFps);
			//	}

			//	frame += (unsigned char)p.cur_tex_frame;

			//	if (frame > m_ucTexFrames)
			//	{
			//		if (!m_bIsTexAnimCycled)
			//			frame = m_ucTexFrames;
			//		else
			//			frame = frame % m_ucTexFrames;
			//	}

			//	int x = (frame - 1) / m_ucCol;
			//	int y = frame - m_ucCol * x;
			//	x += 1;
			//	if (y == 0) y = 1;

			//	// proskakivaet posledniy ryad - ya tormoz

			//	uv1.tu = (x - 1) * 1.0f / m_ucTrow;//m_ucRow;
			//	uv1.tv = (y - 1) * 1.0f / m_ucTcol;//m_ucCol;

			//	uv2.tu = x * 1.0f / m_ucTrow;//m_ucRow;
			//	uv2.tv = y * 1.0f / m_ucTcol;//m_ucCol;

			//	//uv1.tv = (x - 1) * 1.0f / m_ucTcol; uv1.tu = (y - 1) * 1.0f / m_ucTrow;
			//	//uv2.tv = x * 1.0f / m_ucTcol; uv2.tu = y * 1.0f / m_ucTrow;

			//	(*uvs).v[0] = uv1;
			//	(*uvs).v[1] = uv2;
			//}
			//else 
			//{
			//	static agl::AglTexCoords suv[2] = {{0,0},{1,1}};
			//	(*uvs).v[0] = suv[0];
			//	(*uvs).v[1] = suv[1];
			//}				
				i++;
		}
		m_spTank->update();
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::loadTexture()
	{
		//try{
		//	agl::LocalImgPathSync p(pfx::tex_path.c_str());//"Effects\\tx\\");
		//	
		//	m_texture.load(m_TexName);

		//	std::string ext_path = std::string::Concat(
		//		pfx::tex_path.c_str(), pfx::tex_path.length(),
		//		 m_TexName.c_str(), m_TexName.length() - 3);
		//	ext_path += "tnf";

		//	if (base::ResourceMaster::Get()->isResourceExist(ext_path))
		//	{
		//		clx::rstream in(base::ResourceMaster::Get()->getResource(ext_path));//ResourseMaster
		//		//m_texture.load(clx::cstr("anim_explode.png"));
		//		m_bIsAnimTextureUsed = true;
		//		in  >> m_ucRow			// = 4;
		//			>> m_ucCol			// = 4;
		//			>> m_ucTexFrames		// = 16;
		//			>> m_ucTrow
		//			>> m_ucTcol;
		//	}
		//	else 
		//		m_bIsAnimTextureUsed = false;

		//	m_texture.setFilterMode(agl::filter_Linear);
		//}
		//catch (...){
		//	m_TexName.clear();
		//	return;
		//}
		m_texture = render::ITexture::Create(m_TexName);//std::wstring(m_TexName.begin(), m_TexName.end()) );
		return;
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::setTextureName( const std::string& texName )
	{
		m_bIsTexLoaded = false;

		if ( texName.length() != 0 )
		{
			m_TexName = texName;	
			loadTexture();
			if( !m_spTank )
				return;

			m_bIsTexLoaded = true;
		}
	}

	//-----------------------------------------------------------------------------------
	const math::Matrix44f& ParticlesProcessor::getLTM()
	{
		return m_pParentEmitter->getTransform().getFullTransform();
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::load()
	{
		reset();
		initPTank();
		setIntenseMode(m_bIntense);
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::initPTank()
	{
		if (!m_nMaxParticles) return;

		if( m_spTank ) 	delete m_spTank;

		m_spTank = new PTank();


		if( m_TexName.length() )
			// TODO: ���������, ��������� �� ��������
			if( !(m_texture /*&& m_texture->isLoaded()*/ ) )
				loadTexture();

		setIntenseMode(m_bIntense);

		m_bIsPtankInited = true;

		return;
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::assignChilds()
	{
		m_bModifiersLoaded = true;
	}

	//-----------------------------------------------------------------------------------
	void ParticlesProcessor::toStream(io::IWriteStream& wf) const
	{
		wf  << ms_nVersion
			<< m_bIntense
			<< m_fScaling
			<< m_bIsGlobal
			<< m_nMaxParticles
			<< m_TexName
			<< m_bIsSparks
			//<< m_DffName
			<< m_iRndSeed
			<< m_ngkx
			<< m_cTexFps
			<< (m_bIsTexAnimCycled)
			<< (m_PRate)
			<< (m_PResistance)
			<< (m_PSpin)
			<< (m_PSpinSpread)
			<< (m_PLife)
			<< (m_PLifeSpread)
			<< (m_PSize)
			<< (m_PColorAlpha)
			<< (m_PActingForce)
			<< (m_PVelocity)
			<< (m_PInitialVelSpread)
			<< (m_PVelSpreadAmplifier)
			<< m_nRndFrame
			<< m_bIsPlayTexAnimation;
	}

	void ParticlesProcessor::fromStream(io::IReadStream& rf)
	{
		unsigned version;
		rf  >> version;
		if( version != ms_nVersion )
			assert(false);

		rf	>> m_bIntense
			>> m_fScaling
			>> m_bIsGlobal
			>> m_nMaxParticles
			>> m_TexName
			>> m_bIsSparks
			//>> m_DffName
			>> m_iRndSeed
			>> m_ngkx
			>> m_cTexFps
			>> (m_bIsTexAnimCycled)
			>> (m_PRate)
			>> (m_PResistance)
			>> (m_PSpin)
			>> (m_PSpinSpread)
			>> (m_PLife)
			>> (m_PLifeSpread)
			>> (m_PSize)
			>> (m_PColorAlpha)
			>> (m_PActingForce)
			>> (m_PVelocity)
			>> (m_PInitialVelSpread)
			>> (m_PVelSpreadAmplifier)
			>> m_nRndFrame
			>> m_bIsPlayTexAnimation;

		setMaxParticles( m_nMaxParticles );
	}

}